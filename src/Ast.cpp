//
// Created by Dillon on 2018-08-12.
//

#include "Ast.h"
#include "Utils.h"
#include "Tokens.h"
#include "Types.h"

using namespace std;

Expression::Expression(ExpressionKind kind, Location _loc, std::unique_ptr<TypeToken> _type): kind(kind), _loc(move(_loc)), _type(move(_type)) {

}

Location& Expression::loc() {
    return _loc;
}

TypeToken& Expression::type() {
    return *_type;
}

Assignment::Assignment(Location location, unique_ptr<TypeToken> type, string id, unique_ptr<Expression> body) :
        Expression(ExpressionKind::assignment, move(location), move(type)),
        id(move(id)),
        body(move(body)) {

}

Function::Function(Location location, string id, vector<string> params, unique_ptr<TypeToken> type, unique_ptr<Expression> body) :
        Expression(ExpressionKind::function, move(location), move(type)),
        id(move(id)),
        params(move(params)),
        body(move(body)) {

}

Call::Call(Location location, unique_ptr<TypeToken> type, unique_ptr<Expression> source, vector<unique_ptr<Expression>> args) :
    Expression(ExpressionKind::call, move(location), move(type)),
    source(move(source)), 
    args(move(args)) {
    
}

If::If(Location location, std::unique_ptr<TypeToken> type, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> thenEx, std::unique_ptr<Expression> elseEx) :
    Expression(ExpressionKind::ifEx, move(location), move(type)),
    condition(move(condition)),
    thenEx(move(thenEx)),
    elseEx(move(elseEx)) {}

BinaryOp::BinaryOp(Location location, unique_ptr<TypeToken> type, string op, unique_ptr<Expression> left, unique_ptr<Expression> right) :
        Expression(ExpressionKind::binaryOp, move(location), move(type)),
        op(move(op)),
        left(move(left)),
        right(move(right)) {

}

Block::Block(Location location, std::unique_ptr<TypeToken> type, std::vector<std::unique_ptr<Expression>> body) :
    Expression(ExpressionKind::block, move(location), move(type)),
    body(move(body)) {}

Variable::Variable(Location location, string id, unique_ptr<TypeToken> type) :
    Expression(ExpressionKind::variable, move(location), move(type)),
    id(move(id)) {

}

ListLiteral::ListLiteral(Location location, unique_ptr<TypeToken> type, vector<unique_ptr<Expression>> values) :
    Expression(ExpressionKind::listLiteral, move(location), move(type)), values(move(values)) {}

NumberLiteral::NumberLiteral(Location location, double value) : Expression(ExpressionKind::numberLiteral, move(location), make_unique<BaseTypeToken>(BasicTypeTokenKind::Float)), value(value) {}

BooleanLiteral::BooleanLiteral(Location location, bool value) : Expression(ExpressionKind::booleanLiteral, move(location), make_unique<BaseTypeToken>(BasicTypeTokenKind::Boolean)), value(value) {}

NullLiteral::NullLiteral(Location location) : Expression(ExpressionKind::nullLiteral, move(location), make_unique<BaseTypeToken>(BasicTypeTokenKind::Unit)) {}

Module::Module(vector<unique_ptr<Function>> functions) : functions(move(functions)) {}
