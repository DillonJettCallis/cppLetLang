//
// Created by Dillon on 2018-08-12.
//

#ifndef TYPEDLETLANG_AST_H
#define TYPEDLETLANG_AST_H

#include "Ast.h"
#include "Utils.h"
#include "Tokens.h"
#include "Types.h"

enum ExpressionKind {
    assignment,
    function,
    call,
    binaryOp,
    variable,
    numberLiteral
};

class Expression {
public:

    ExpressionKind kind;
    Location _loc;

    Expression(ExpressionKind kind, Location _loc);

    Location& loc();

};

class Assignment : public Expression {
public:

    std::string id;
    std::unique_ptr<TypeToken> type;
    std::unique_ptr<Expression> body;

    Assignment(Location location, std::unique_ptr<TypeToken> type, std::string id, std::unique_ptr<Expression> body);

};

class Function : public Expression {
public:

    std::string id;
    std::vector<std::string> params;
    std::unique_ptr<TypeToken> type;
    std::vector<std::unique_ptr<Expression>> body;

    Function(Location location, std::string id, std::vector<std::string> params, std::unique_ptr<TypeToken> type, std::vector<std::unique_ptr<Expression>> body);

};

class Call : public Expression {
public:

    std::unique_ptr<Expression> source;
    std::vector<std::unique_ptr<Expression>> args;

    Call(Location location, std::unique_ptr<Expression> source, std::vector<std::unique_ptr<Expression>> args);

};

class BinaryOp : public Expression {
public:

    std::unique_ptr<TypeToken> type;
    std::string op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryOp(Location location, std::unique_ptr<TypeToken> type, std::string op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);

};

class Variable : public Expression {
public:

    std::string id;
    std::unique_ptr<TypeToken> type;

    Variable(Location location, std::string id, std::unique_ptr<TypeToken> type);

};

class NumberLiteral : public Expression {
public:

    double value;

    NumberLiteral(Location location, double value);

};

class Module {
public:

    std::vector<std::unique_ptr<Function>> functions;

    explicit Module(std::vector<std::unique_ptr<Function>> functions);

};

#endif //TYPEDLETLANG_AST_H
