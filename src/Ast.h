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
    binaryOp,
    numberLiteral
};

class Expression {
public:

    ExpressionKind kind;
    Location _loc;

    Expression(ExpressionKind kind, Location _loc);

    Location loc();

};

class Assignment : public Expression {
public:

    std::string id;
    std::unique_ptr<TypeToken> type;
    std::unique_ptr<Expression> body;

    Assignment(Location location, std::unique_ptr<TypeToken> type, std::string id, std::unique_ptr<Expression> body);

};

class BinaryOp : public Expression {
public:

    std::unique_ptr<TypeToken> type;
    std::string op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryOp(Location location, std::unique_ptr<TypeToken> type, std::string op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);

};

class NumberLiteral : public Expression {
public:

    double value;

    NumberLiteral(Location location, double value);

};

#endif //TYPEDLETLANG_AST_H
