//
// Created by Dillon on 2018-08-12.
//

#include "Compiler.h"
#include <fstream>
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "llvm/Support/raw_os_ostream.h"

using namespace std;

class Compiler {

    llvm::LLVMContext con;
    llvm::Module mod;
    llvm::IRBuilder<> builder;

    vector<map<string, llvm::Value *>> contextStack;
    vector<vector<llvm::AllocaInst*>> declaredArraysStack;
    map<string, llvm::Type *> types;

public:
    Compiler() : mod(llvm::Module("main", con)), builder(con) {

    }

    void compileModule(Module* ex) {
        setupLibrary();

        map<string, llvm::Value*> context;

        for (auto &fun : ex->functions) {
            context[fun->id] = getOrCreateFunction(fun.get());
        }

        contextStack.push_back(move(context));
        declaredArraysStack.emplace_back();

        for (auto &fun : ex->functions) {
            compileFunction(fun.get());
        }

        std::string errorMessage;
        llvm::raw_string_ostream errorStream(errorMessage);
        auto hasErrors = llvm::verifyModule(mod, &errorStream);

        if (hasErrors) {
            throw std::runtime_error("Generated invalid module: " + errorMessage);
        }
    }

    void output(const std::string &fileName) {
        std::ofstream innerOut(fileName);
        llvm::raw_os_ostream outStream(innerOut);
        mod.print(outStream, nullptr);
    }

private:
    llvm::Value *compile(Expression *expression) {
        ExpressionKind kind = expression->kind;

        switch (kind) {
            case ExpressionKind::assignment: {
                auto ex = (Assignment *) expression;
                auto value = compile(ex->body.get());
                contextStack.back()[ex->id] = value;
                return value;
            }
            case ExpressionKind::function: {
                auto ex = (Function *) expression;

                return compileFunction(ex);
            }
            case ExpressionKind::call: {
                auto ex = (Call *) expression;

                auto func = compile(ex->source.get());

                auto rawArgs = &ex->args;

                llvm::Value *args[rawArgs->size()];

                for (unsigned int i = 0; i < rawArgs->size(); i++) {
                    args[i] = compile((*rawArgs)[i].get());
                }

                return builder.CreateCall(func, *args);
            }
            case ExpressionKind::ifEx: {
                auto ex = (If *) expression;

                //create condition
                auto condition = compile(ex->condition.get());

                auto ifBlock = builder.CreateICmpEQ(condition, llvm::ConstantInt::getTrue(con), "ifCondition");

                // create blocks
                auto currentFunction = builder.GetInsertBlock()->getParent();

                auto thenBlock = llvm::BasicBlock::Create(con, "thenBlock", currentFunction);
                auto elseBlock = llvm::BasicBlock::Create(con, "elseBlock");
                auto mergeBlock = llvm::BasicBlock::Create(con, "ifContinued");

                builder.CreateCondBr(ifBlock, thenBlock, elseBlock);

                // Create then block
                builder.SetInsertPoint(thenBlock);
                auto thenResult = compile(ex->thenEx.get());
                builder.CreateBr(mergeBlock);
                thenBlock = builder.GetInsertBlock();

                // Create else block
                currentFunction->getBasicBlockList().push_back(elseBlock);
                builder.SetInsertPoint(elseBlock);
                auto elseResult = compile(ex->elseEx.get());
                builder.CreateBr(mergeBlock);
                elseBlock = builder.GetInsertBlock();

                // Create merge block
                currentFunction->getBasicBlockList().push_back(mergeBlock);
                builder.SetInsertPoint(mergeBlock);

                // Merge resulting values
                auto phi = builder.CreatePHI(mapTypes(ex->type()), 2, "ifTemp");
                phi->addIncoming(thenResult, thenBlock);
                phi->addIncoming(elseResult, elseBlock);
                return phi;
            }
            case ExpressionKind::binaryOp: {
                auto ex = (BinaryOp *) expression;

                return compileBinaryOp(ex);
            }
            case ExpressionKind::block: {
                auto ex = (Block *) expression;

                contextStack.emplace_back();
                declaredArraysStack.emplace_back();

                // TODO: Think about Unit better.
                llvm::Value *last = llvm::ConstantFP::get(con, llvm::APFloat(0.0));

                for (auto &next : ex->body) {
                    last = compile(next.get());
                }

                contextStack.pop_back();
                auto lastDelaredScope = declaredArraysStack.back();
                declaredArraysStack.pop_back();

                auto destroyArray = lookupValue("destroyArray");

                for (auto &next : lastDelaredScope) {
                    builder.CreateCall(destroyArray, { next });
                }

                return last;
            }
            case ExpressionKind::variable: {
                auto ex = (Variable *) expression;
                auto var = lookupValue(ex->id);

                if (var == nullptr) {
                    throw std::runtime_error("Variable " + ex->id + " is not declared at " + ex->loc().pretty());
                }

                return var;
            }
            case ExpressionKind::listLiteral: {
                auto ex = (ListLiteral *) expression;
                auto size = ex->values.size();

                // TODO: List types besides Float
                auto createArray = lookupValue("createArray");
                auto intType = llvm::IntegerType::get(con, 32);
                auto sizeConst = llvm::ConstantInt::get(intType,  size, false);
                auto doubleConst = llvm::ConstantInt::get(intType,  sizeof(double), false);

                auto array = builder.CreateAlloca(types["arrayRefType"], nullptr, "tempArray");
                declaredArraysStack.back().push_back(array);

                builder.CreateCall(createArray, { array, sizeConst, sizeConst, doubleConst });

                auto doubleInsert = lookupValue("mutableInsertArrayDouble");

                unsigned int index = 0;
                for (auto &nextEx : ex->values) {
                    auto nextValue = compile(nextEx.get());
                    builder.CreateCall(doubleInsert, {array, llvm::ConstantInt::get(intType,  index++, false), nextValue} );
                }

                return array;
            }
            case ExpressionKind::numberLiteral: {
                auto ex = (NumberLiteral *) expression;
                return llvm::ConstantFP::get(con, llvm::APFloat(ex->value));
            }
            case ExpressionKind::booleanLiteral: {
                auto ex = (BooleanLiteral *) expression;
                if (ex->value) {
                    return llvm::ConstantInt::getTrue(con);
                } else {
                    return llvm::ConstantInt::getFalse(con);
                }
            }
            case ExpressionKind::nullLiteral: {
                // TODO: Handle nulls somehow?
                return llvm::ConstantFP::get(con, llvm::APFloat(0.0));
            }
            default:
                throw std::runtime_error("Unknown Expression type");
        }
    }

    llvm::Function* getOrCreateFunction(Function* ex) {
        auto lookedUp = lookupValue(ex->id);

        if (lookedUp == nullptr) {
            auto *functionType = mapTypes((BasicFunctionTypeToken &) ex->type());

            return llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, ex->id, &mod);
        } else {
            return (llvm::Function*) lookedUp;
        }
    }

    llvm::Function* compileFunction(Function* ex) {
        auto *func = getOrCreateFunction(ex);

        auto *body = llvm::BasicBlock::Create(con, "body", func);
        auto initStartPoint = builder.GetInsertBlock();
        builder.SetInsertPoint(body);

        auto rawBody = &ex->body;


        map<string, llvm::Value*> context;

        {
            // Block to keep i out of scope.
            int i = 0;
            for (auto &arg : func->args()) {
                auto name = ex->params[i++];
                arg.setName(name);
                context[name] = &arg;
            }
        }

        contextStack.push_back(move(context));
        llvm::Value *result = compile(rawBody->get());


        if (func->getReturnType()->isVoidTy()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(result);
        }

        contextStack.pop_back();

        builder.SetInsertPoint(initStartPoint);

        return func;
    }

    llvm::Value* compileBinaryOp(BinaryOp *ex) {
        auto left = compile(ex->left.get());
        auto right = compile(ex->right.get());

        auto op = ex->op;

        // TODO: Handle types besides Float
        if (op == "+") {
            return builder.CreateFAdd(left, right, "addTemp");
        } else if (op == "-") {
            return builder.CreateFSub(left, right, "subTemp");
        } else if (op == "*") {
            return builder.CreateFMul(left, right, "mulTemp");
        } else if (op == "/") {
            return builder.CreateFDiv(left, right, "divTemp");
        } else if (op == "==") {
            return builder.CreateFCmpOEQ(left, right, "equalTemp");
        } else if (op == "!=") {
            return builder.CreateFCmpONE(left, right, "notEqualTemp");
        } else if (op == ">=") {
            return builder.CreateFCmpOGE(left, right, "greaterOrEqualTemp");
        } else if (op == ">") {
            return builder.CreateFCmpOGT(left, right, "greaterTemp");
        } else if (op == "<") {
            return builder.CreateFCmpOLT(left, right, "lessTemp");
        } else if (op == "<=") {
            return builder.CreateFCmpOLE(left, right, "lessOrEqualTemp");
        } else if (op == "&&") {
            return builder.CreateAnd(left, right, "andTemp");
        } else if (op == "||") {
            return builder.CreateOr(left, right, "orTemp");
        } else {
            throw std::runtime_error("Unknown binary operator: " + op + " at " + ex->loc().pretty());
        }
    }

    llvm::Value* lookupValue(const string &id) {

        for (auto i = contextStack.size(); i > 0; --i) {
            auto &context = contextStack[i - 1];

            if (context.find(id) != context.end()) {
                return context[id];
            }
        }

        return nullptr;
    }

    llvm::Type* mapTypes(TypeToken& raw) {
        switch (raw.kind) {
            case TypeTokenKind::base: {
                return mapTypes((BaseTypeToken&) raw);
            }
            case TypeTokenKind::basicFunction: {
                return mapTypes((BasicFunctionTypeToken&) raw);
            }
            default:
                throw runtime_error("Unknown type token kind");
        }
    }

    llvm::Type* mapTypes(BaseTypeToken& raw) {
        switch (raw.base) {
            case BasicTypeTokenKind::Float:
                return llvm::Type::getDoubleTy(con);
            case BasicTypeTokenKind::Unit:
                return llvm::Type::getVoidTy(con);
            default:
                throw runtime_error("Unknown base type token kind");
        }
    }

    llvm::FunctionType* mapTypes(BasicFunctionTypeToken& token) {
        vector<llvm::Type*> paramTypes;
        paramTypes.reserve(token.params.size());

        for (auto &param : token.params) {
            paramTypes.push_back(mapTypes(*param));
        }

        return llvm::FunctionType::get(mapTypes(*token.result), paramTypes, false);
    }

    void setupLibrary() {
        map<string, llvm::Value*> context;

        auto voidType = llvm::Type::getVoidTy(con);
        auto intType = llvm::IntegerType::get(con, 32);

        llvm::ArrayRef<llvm::Type *> arrayRefMembers = {llvm::PointerType::getInt8PtrTy(con), intType, intType, intType};
        auto arrayRefType = llvm::StructType::create(con, arrayRefMembers, "ArrayRef");
        auto arrayRefPointerType = llvm::PointerType::get(arrayRefType, 0);
        types["arrayRefType"] = arrayRefType;
        types["arrayRefPointerType"] = arrayRefPointerType;

        llvm::ArrayRef<llvm::Type *> args = {llvm::Type::getDoubleTy(con)};
        auto printdType = llvm::FunctionType::get(voidType, args, false);
        mod.getOrInsertFunction("printd", printdType);
        context["printd"] = mod.getFunction("printd");

        llvm::ArrayRef<llvm::Type *> printdsArgs = { arrayRefPointerType };
        auto printdsType = llvm::FunctionType::get(voidType, printdsArgs, false);
        mod.getOrInsertFunction("printds", printdsType);
        context["printds"] = mod.getFunction("printds");


        llvm::ArrayRef<llvm::Type*> createArrayArgs = { arrayRefPointerType, intType, intType, intType};
        auto createArrayType = llvm::FunctionType::get(voidType, createArrayArgs, false);
        mod.getOrInsertFunction("createArray", createArrayType);
        context["createArray"] = mod.getFunction("createArray");

        llvm::ArrayRef<llvm::Type*> destroyArrayArgs = { arrayRefPointerType };
        auto destroyArrayType = llvm::FunctionType::get(voidType, destroyArrayArgs, false);
        mod.getOrInsertFunction("destroyArray", destroyArrayType);
        context["destroyArray"] = mod.getFunction("destroyArray");

        llvm::ArrayRef<llvm::Type*> mutableInsertArrayDoubleArgs = { arrayRefPointerType, intType, llvm::Type::getDoubleTy(con) };
        auto mutableInsertArrayDoubleType = llvm::FunctionType::get(voidType, mutableInsertArrayDoubleArgs, false);
        mod.getOrInsertFunction("mutableInsertArrayDouble", mutableInsertArrayDoubleType);
        context["mutableInsertArrayDouble"] = mod.getFunction("mutableInsertArrayDouble");

        contextStack.clear();
        contextStack.push_back(move(context));
    }

};


void compile(const std::string &dest, Module *mod) {
    Compiler compiler;
    compiler.compileModule(mod);
    compiler.output(dest);
}
