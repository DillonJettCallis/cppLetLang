//
// Created by Dillon on 2018-08-12.
//

#include "Tokens.h"

Location::Location(std::string _sourceFile, int _x, int _y) {
    sourceFile = std::move(_sourceFile);
    x = _x;
    y = _y;
}

std::string Location::pretty() {
    return "file: " + sourceFile + ", line: " + std::to_string(y) + ", col: " + std::to_string(x);
}

Token::Token(Location loc, std::string value, TokenType type): loc(std::move(loc)), word(std::move(value)), type(type) {

};

std::string Token::expected(std::string expected) {
    return "Expected " + expected + " at " + loc.pretty() + " but found '" + word + "'";
}
