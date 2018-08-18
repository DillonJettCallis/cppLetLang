//
// Created by Dillon on 2018-08-12.
//

#include "Ast.h"
#include "Utils.h"
#include "Tokens.h"
#include "Types.h"

Expression::Expression(ExpressionKind kind, Location _loc): kind(kind), _loc(std::move(_loc)) {

}

Location& Expression::loc() {
    return _loc;
}

Assignment::Assignment(Location location, std::unique_ptr<TypeToken> type, std::string id, std::unique_ptr<Expression> body) :
        Expression(ExpressionKind::assignment, std::move(location)),
        type(std::move(type)),
        id(std::move(id)),
        body(std::move(body)) {

}

Function::Function(Location location, std::string id, std::vector<std::string> params, std::unique_ptr<TypeToken> type, std::vector<std::unique_ptr<Expression>> body) :
        Expression(ExpressionKind::function, std::move(location)),
        id(std::move(id)),
        params(std::move(params)),
        type(std::move(type)),
        body(std::move(body)) {

}

BinaryOp::BinaryOp(Location location, std::unique_ptr<TypeToken> type, std::string op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) :
        Expression(ExpressionKind::binaryOp, std::move(location)),
        type(std::move(type)),
        op(std::move(op)),
        left(std::move(left)),
        right(std::move(right)) {

}

Variable::Variable(Location location, std::string id, std::unique_ptr<TypeToken> type) :
    Expression(ExpressionKind::variable, std::move(location)),
    id(std::move(id)),
    type(std::move(type)) {

}

NumberLiteral::NumberLiteral(Location location, double value) : Expression(ExpressionKind::numberLiteral, std::move(location)), value(value) {}

Module::Module(std::vector<std::unique_ptr<Function>> functions) : functions(std::move(functions)) {}
