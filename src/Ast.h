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
    ifEx,
    binaryOp,
    block,
    variable,
    listLiteral,
    numberLiteral,
    booleanLiteral,
    nullLiteral
};

class Expression {

    Location _loc;

public:

    ExpressionKind kind;
    std::unique_ptr<TypeToken> _type;

    Expression(ExpressionKind kind, Location _loc, std::unique_ptr<TypeToken> _type);

    Location& loc();
    TypeToken& type();

};

class Assignment : public Expression {
public:

    std::string id;
    std::unique_ptr<Expression> body;

    Assignment(Location location, std::unique_ptr<TypeToken> type, std::string id, std::unique_ptr<Expression> body);

};

class Function : public Expression {
public:

    std::string id;
    std::vector<std::string> params;
    std::unique_ptr<Expression> body;

    Function(Location location, std::string id, std::vector<std::string> params, std::unique_ptr<TypeToken> type, std::unique_ptr<Expression> body);

};

class Call : public Expression {
public:

    std::unique_ptr<Expression> source;
    std::vector<std::unique_ptr<Expression>> args;

    Call(Location location, std::unique_ptr<TypeToken> type, std::unique_ptr<Expression> source, std::vector<std::unique_ptr<Expression>> args);

};

class If : public Expression {
public:

    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> thenEx;
    std::unique_ptr<Expression> elseEx;

    If(Location location, std::unique_ptr<TypeToken> type, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> thenEx, std::unique_ptr<Expression> elseEx);

};

class BinaryOp : public Expression {
public:

    std::string op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryOp(Location location, std::unique_ptr<TypeToken> type, std::string op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);

};

class Block : public Expression {
public:

    std::vector<std::unique_ptr<Expression>> body;

    Block(Location location, std::unique_ptr<TypeToken> type, std::vector<std::unique_ptr<Expression>> body);

};

class Variable : public Expression {
public:

    std::string id;

    Variable(Location location, std::string id, std::unique_ptr<TypeToken> type);

};

class ListLiteral : public Expression {
public:

    std::vector<std::unique_ptr<Expression>> values;

    ListLiteral(Location location, std::unique_ptr<TypeToken> type, std::vector<std::unique_ptr<Expression>> values);

};

class NumberLiteral : public Expression {
public:

    double value;

    NumberLiteral(Location location, double value);

};

class BooleanLiteral : public Expression {
public:
    bool value;

    BooleanLiteral(Location location, bool value);

};

class NullLiteral : public Expression {
public:

    explicit NullLiteral(Location location);

};

class Module {
public:

    std::vector<std::unique_ptr<Function>> functions;

    explicit Module(std::vector<std::unique_ptr<Function>> functions);

};

#endif //TYPEDLETLANG_AST_H
