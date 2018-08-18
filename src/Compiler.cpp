//
// Created by Dillon on 2018-08-12.
//

#include "Compiler.h"
#include "Ast.h"
#include <fstream>
#include <memory>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"

class Compiler {

    llvm::LLVMContext con;
    llvm::Module mod;
    llvm::IRBuilder<> builder;

    std::map<std::string, llvm::AllocaInst*> context;

public:
    Compiler() : mod(llvm::Module("main", con)), builder(con) {

    }

    llvm::Value* compile(Expression *expression) {
        ExpressionKind kind = expression->kind;

        switch (kind) {
            case ExpressionKind::assignment: {
                auto ex = (Assignment *) expression;
                auto created = builder.CreateAlloca(llvm::Type::getDoubleTy(con), nullptr, ex->id);
                builder.CreateStore(compile(ex->body.get()), created);
                context[ex->id] = created;
                return builder.CreateLoad(created);
            }
            case ExpressionKind::function: {
                auto ex = (Function *) expression;

                // TODO: Gen real arg types.
                std::vector<llvm::Type*> argTypes;

                // TODO: Get actual result type.
                llvm::FunctionType* functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(con), argTypes, false);

                llvm::Function* func = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, ex->id, &mod);

                // TODO: Name arguments in function. Not required, but makes debugging IR easier.

                llvm::BasicBlock* body = llvm::BasicBlock::Create(con, "body", func);
                builder.SetInsertPoint(body);

                // TODO: Collect params from function

                auto rawBody = &ex->body;

                if (rawBody->empty()) {
                    builder.CreateRetVoid();
                } else {
                    llvm::Value *result = nullptr;

                    for (auto &next : *rawBody) {
                        result = compile(next.get());
                    }

                    // TODO: Handle return values.
//                    builder.CreateRet(result);
                    builder.CreateRetVoid();
                }

                std::string errorMessage;
                llvm::raw_string_ostream errorStream(errorMessage);
                auto hasErrors = llvm::verifyFunction(*func, &errorStream);

                if (hasErrors) {
                    throw std::runtime_error("Generated invalid function: " + errorMessage);
                }

                return func;
            }
            case ExpressionKind::variable: {
                auto ex = (Variable*) expression;
                auto var = context[ex->id];

                if (var == nullptr) {
                    throw std::runtime_error("Variable " + ex->id + " is not declared at " + ex->loc().pretty());
                }

                return builder.CreateLoad(var, ex->id);
            }
            case ExpressionKind::binaryOp: {
                auto ex = (BinaryOp *) expression;

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
                } else {
                    throw std::runtime_error("Unknown binary operator: " + op + " at " + ex->loc().pretty());
                }
            }
            case ExpressionKind::numberLiteral: {
                auto ex = (NumberLiteral *) expression;
                return llvm::ConstantFP::get(con, llvm::APFloat(ex->value));
            }
            default:
                throw std::runtime_error("Unknown Expression type");
        }
    }

    void output(const std::string &fileName) {
        std::ofstream innerOut(fileName);
        llvm::raw_os_ostream outStream(innerOut);
        mod.print(outStream, nullptr);
    }

};


void compile(std::string dest, Expression *expression) {
    Compiler compiler;
    compiler.compile(expression);
    compiler.output(dest);
}
