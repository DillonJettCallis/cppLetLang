//
// Created by Dillon on 2018-08-12.
//

#include "Ast.h"
#include "Utils.h"
#include "Tokens.h"
#include "Types.h"

using namespace std;

Expression::Expression(ExpressionKind kind, Location _loc): kind(kind), _loc(move(_loc)) {

}

Location& Expression::loc() {
    return _loc;
}

Assignment::Assignment(Location location, unique_ptr<TypeToken> type, string id, unique_ptr<Expression> body) :
        Expression(ExpressionKind::assignment, move(location)),
        type(move(type)),
        id(move(id)),
        body(move(body)) {

}

Function::Function(Location location, string id, vector<string> params, unique_ptr<TypeToken> type, vector<unique_ptr<Expression>> body) :
        Expression(ExpressionKind::function, move(location)),
        id(move(id)),
        params(move(params)),
        type(move(type)),
        body(move(body)) {

}

Call::Call(Location location, unique_ptr<Expression> source, vector<unique_ptr<Expression>> args) :
    Expression(ExpressionKind::call, move(location)),
    source(move(source)), 
    args(move(args)) {
    
}

BinaryOp::BinaryOp(Location location, unique_ptr<TypeToken> type, string op, unique_ptr<Expression> left, unique_ptr<Expression> right) :
        Expression(ExpressionKind::binaryOp, move(location)),
        type(move(type)),
        op(move(op)),
        left(move(left)),
        right(move(right)) {

}

Variable::Variable(Location location, string id, unique_ptr<TypeToken> type) :
    Expression(ExpressionKind::variable, move(location)),
    id(move(id)),
    type(move(type)) {

}

NumberLiteral::NumberLiteral(Location location, double value) : Expression(ExpressionKind::numberLiteral, move(location)), value(value) {}

Module::Module(vector<unique_ptr<Function>> functions) : functions(move(functions)) {}
