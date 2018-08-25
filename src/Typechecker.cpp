//
// Created by Dillon on 2018-08-12.
//

#include "Typechecker.h"
#include <set>
#include <map>

using namespace std;

class Typechecker {

    map<string, BasicTypeTokenKind> knownBasicTypes{{"Float",   BasicTypeTokenKind::Float},
                                                    {"Boolean", BasicTypeTokenKind::Boolean},
                                                    {"Unit",    BasicTypeTokenKind::Unit}};
    vector<map<string, unique_ptr<TypeToken>>> contextStack{};

public:
    void check(Module &module) {

        // TODO: Real standard library descriptions.
        setupLibrary();

        // Pre-declare all module functions so that their order doesn't matter.
        map<string, unique_ptr<TypeToken>> context;

        for (auto &fun : module.functions) {
            auto funcType = fillTypes((BasicFunctionTypeToken &) fun->type());

            context.insert({fun->id, move(funcType)});
        }

        contextStack.push_back(move(context));

        for (auto &fun : module.functions) {
            checkFunction(*fun);
        }
    }

private:
    void checkFunction(Function &func) {
        auto funcType = fillTypes((BasicFunctionTypeToken &) func.type());

        map<string, unique_ptr<TypeToken>> context;

        for (int i = 0; i < func.params.size(); i++) {
            auto key = func.params[i];
            auto &value = funcType->params[i];

            context.insert({key, move(value->clone())});
        }

        func._type = move(funcType);
        contextStack.push_back(move(context));

        checkExpression(*func.body);

        contextStack.pop_back();

        contextStack.back()[func.id] = func._type->clone();
    }

    void checkExpression(Expression &expression) {
        switch (expression.kind) {
            case ExpressionKind::assignment: {
                auto &ex = (Assignment &) expression;

                checkExpression(*ex.body);

                if (ex.type().kind == TypeTokenKind::unknown) {
                    ex._type = ex.body->type().clone();
                } else {
                    ex._type = fillTypes(ex.type());

                    if (ex.type() != ex.body->type()) {
                        throw runtime_error(
                                "Incompatible types in assignment. Declared " + ex.type().pretty() + " found " +
                                ex.body->type().pretty() + " at " + ex.loc().pretty());
                    }
                }

                map<string, unique_ptr<TypeToken>> &context = contextStack.back();

                if (context.find(ex.id) != context.end()) {
                    throw runtime_error("Attempt to reassign variable: " + ex.id + " at " + ex.loc().pretty());
                }

                context[ex.id] = ex.type().clone();

                break;
            }
            case ExpressionKind::function: {
                checkFunction((Function &) expression);
                break;
            }
            case ExpressionKind::call: {
                auto &ex = (Call &) expression;

                checkExpression(*ex.source);

                if (ex.source->type().kind == TypeTokenKind::basicFunction) {
                    auto &funcType = (BasicFunctionTypeToken &) ex.source->type();

                    if (funcType.params.size() != ex.args.size()) {
                        throw runtime_error(
                                "Wrong number of parameters passed to function at " + ex.source->loc().pretty());
                    }

                    for (int i = 0; i < ex.args.size(); i++) {
                        checkExpression(*ex.args[i]);

                        // TODO: polymorphism.
                        if (ex.args[i]->type() != *funcType.params[i]) {
                            throw runtime_error(
                                    "Invalid parameters passed to function. Expected " + funcType.pretty() + " found " +
                                    ex.args[i]->type().pretty() + " at position " + to_string(i) + " at " +
                                    ex.source->loc().pretty());
                        }
                    }

                    ex._type = funcType.result->clone();
                } else {
                    throw runtime_error("Attempt to call not-callable:  at " + ex.source->loc().pretty());
                }

                break;
            }
            case ExpressionKind::ifEx: {
                auto &ex = (If &) expression;

                checkExpression(*ex.condition);

                if (BaseTypeToken(BasicTypeTokenKind::Boolean) != ex.condition->type()) {
                    throw runtime_error("Condition of if statement is not a boolean " + ex.loc().pretty());
                }

                checkExpression(*ex.thenEx);
                checkExpression(*ex.elseEx);

                if (ex.thenEx->type() == ex.elseEx->type() || BaseTypeToken(BasicTypeTokenKind::Unit) == ex.elseEx->type()) {
                    ex._type = ex.thenEx->type().clone();
                } else {
                    // TODO: Handle polymorphism.
                    throw runtime_error("if expression then and else blocks do not return the same type. Then type: " + ex.thenEx->type().pretty() + " else type: " + ex.elseEx->type().pretty());
                }

                break;
            }
            case ExpressionKind::binaryOp: {
                auto &ex = (BinaryOp &) expression;

                checkExpression(*ex.left);
                checkExpression(*ex.right);

                /* TODO: More complex type checking here. Maybe reuse function code.
                         This will become very acute when the math operators get overloads. */
                auto &opFunc = (BasicFunctionTypeToken &) lookupType(ex.op);

                if (ex.left->type() != *opFunc.params[0]) {
                    throw runtime_error("Invalid use of " + ex.op + " operator. Left hand expression is of type " +
                                        ex.left->type().pretty() + ", expected type " + opFunc.params[0]->pretty());
                }

                if (ex.right->type() != *opFunc.params[1]) {
                    throw runtime_error("Invalid use of " + ex.op + " operator. Right hand expression is of type " +
                                        ex.right->type().pretty() + ", expected type " + opFunc.params[1]->pretty());
                }

                ex._type = opFunc.result->clone();
                break;
            }
            case ExpressionKind::block: {
                auto &ex = (Block &) expression;

                if (ex.body.empty()) {
                    ex._type = make_unique<BaseTypeToken>(BasicTypeTokenKind::Unit);
                    break;
                }

                for (auto &e : ex.body) {
                    checkExpression(*e);
                }

                ex._type = ex.body.back().get()->type().clone();
                break;
            }
            case ExpressionKind::variable: {
                auto &ex = (Variable &) expression;
                ex._type = lookupType(ex.id).clone();
                break;
            }
            case ExpressionKind::listLiteral: {
                auto &ex = (ListLiteral &) expression;

                if (ex.values.empty()) {
                    vector<unique_ptr<TypeToken>> genericListType;
                    genericListType.push_back(move(make_unique<BaseTypeToken>(BasicTypeTokenKind::Unit)));

                    ex._type = make_unique<GenericTypeToken>(move(make_unique<TypeConstructorTypeToken>("List", 1)), move(genericListType));
                } else {
                    unique_ptr<TypeToken> listType = nullptr;

                    for (auto &next : ex.values) {
                        checkExpression(*next);
                        auto nextType = next->type().clone();

                        if (listType == nullptr) {
                            listType = move(nextType);
                        } else {
                            if (*listType != *nextType) {
                                throw runtime_error("List contains more than one type: " + next->loc().pretty());
                            }
                        }
                    }

                    vector<unique_ptr<TypeToken>> genericListType;
                    genericListType.push_back(move(listType));

                    ex._type = make_unique<GenericTypeToken>(move(make_unique<TypeConstructorTypeToken>("List", 1)), move(genericListType));
                }
                break;
            }
            case ExpressionKind::numberLiteral:
            case ExpressionKind::booleanLiteral:
            case ExpressionKind::nullLiteral:
                break;
            default:
                throw runtime_error("Unknown expression kind");
        }
    }

    unique_ptr<TypeToken> fillTypes(TypeToken &source) {
        switch (source.kind) {
            case TypeTokenKind::named: {
                auto name = ((NamedTypeToken &) source).id;

                if (knownBasicTypes.find(name) != knownBasicTypes.end()) {
                    return make_unique<BaseTypeToken>(knownBasicTypes[name]);
                } else {
                    throw runtime_error("Unknown type: " + name);
                }
            }
            case TypeTokenKind::basicFunction: {
                auto &type = (BasicFunctionTypeToken &) source;
                return fillTypes(type);
            }
            case TypeTokenKind::base: {
                auto &base = (BaseTypeToken &) source;

                return make_unique<BaseTypeToken>(base.base);
            }
            default:
                throw runtime_error("Unknown type token kind");
        }
    }

    unique_ptr<BasicFunctionTypeToken> fillTypes(BasicFunctionTypeToken &type) {
        auto &params = type.params;
        auto &result = type.result;

        vector<unique_ptr<TypeToken>> newParams;

        newParams.reserve(params.size());
        for (auto &param : params) {
            newParams.push_back(fillTypes(*param));
        }

        auto newResult = fillTypes(*result);

        return make_unique<BasicFunctionTypeToken>(move(newParams), move(newResult));
    }

    TypeToken &lookupType(const string &id) {

        for (auto i = contextStack.size(); i > 0; --i) {
            auto &context = contextStack[i - 1];

            if (context.find(id) != context.end()) {
                return *context[id];
            }
        }

        throw runtime_error("Undefined identifier " + id);
    }

    void setupLibrary() {
        map<string, unique_ptr<TypeToken>> context;


        BaseTypeToken unitType(BasicTypeTokenKind::Unit);

        vector<unique_ptr<TypeToken>> printdParams;
        printdParams.push_back(move(make_unique<BaseTypeToken>(BasicTypeTokenKind::Float)));

        context["printd"] = make_unique<BasicFunctionTypeToken>(move(printdParams), move(unitType.clone()));

        vector<unique_ptr<TypeToken>> printdsParams;
        vector<unique_ptr<TypeToken>> printdsListType;
        printdsListType.push_back(move(make_unique<BaseTypeToken>(BasicTypeTokenKind::Float)));
        printdsParams.push_back(move(make_unique<GenericTypeToken>(move(make_unique<TypeConstructorTypeToken>("List", 1)), move(printdsListType))));

        context["printds"] = make_unique<BasicFunctionTypeToken>(move(printdsParams), move(unitType.clone()));

        context["+"] = move(makeNumberOp());
        context["-"] = move(makeNumberOp());
        context["*"] = move(makeNumberOp());
        context["/"] = move(makeNumberOp());

        context["=="] = move(makeCompareOp());
        context["!="] = move(makeCompareOp());
        context[">="] = move(makeCompareOp());
        context[">"] = move(makeCompareOp());
        context["<"] = move(makeCompareOp());
        context["<="] = move(makeCompareOp());

        context["&&"] = move(makeBooleanOp());
        context["||"] = move(makeBooleanOp());

        contextStack.clear();
        contextStack.push_back(move(context));
    }

    unique_ptr<BasicFunctionTypeToken> makeNumberOp() {
        vector<unique_ptr<TypeToken>> params;
        params.emplace_back(make_unique<BaseTypeToken>(BasicTypeTokenKind::Float));
        params.emplace_back(make_unique<BaseTypeToken>(BasicTypeTokenKind::Float));

        return make_unique<BasicFunctionTypeToken>(
                move(params),
                move(make_unique<BaseTypeToken>(BasicTypeTokenKind::Float)));
    }

    unique_ptr<BasicFunctionTypeToken> makeCompareOp() {
        vector<unique_ptr<TypeToken>> params;
        params.emplace_back(make_unique<BaseTypeToken>(BasicTypeTokenKind::Float));
        params.emplace_back(make_unique<BaseTypeToken>(BasicTypeTokenKind::Float));

        return make_unique<BasicFunctionTypeToken>(
                move(params),
                make_unique<BaseTypeToken>(BasicTypeTokenKind::Boolean));
    }

    unique_ptr<BasicFunctionTypeToken> makeBooleanOp() {
        vector<unique_ptr<TypeToken>> params;
        params.emplace_back(make_unique<BaseTypeToken>(BasicTypeTokenKind::Boolean));
        params.emplace_back(make_unique<BaseTypeToken>(BasicTypeTokenKind::Boolean));

        return make_unique<BasicFunctionTypeToken>(
                move(params),
                move(make_unique<BaseTypeToken>(BasicTypeTokenKind::Boolean)));

    }
};


void typeCheck(Module &module) {
    Typechecker checker;
    checker.check(module);
}
