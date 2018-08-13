//
// Created by Dillon on 2018-08-12.
//

#ifndef TYPEDLETLANG_TOKENS_H
#define TYPEDLETLANG_TOKENS_H

#include <iostream>
#include <vector>
#include <memory>

class Location {
public:

    std::string sourceFile;
    int x;
    int y;

    Location(std::string _sourceFile, int _x, int _y);

    std::string pretty();
};

enum TokenType {
    Identifier,
    Symbol,
    Number,
    Eof
};

class Token {
public:

    Location loc;
    std::string word;
    TokenType type;

    Token(Location loc, std::string word, TokenType type);

    std::string expected(std::string expected);
};

#endif //TYPEDLETLANG_TOKENS_H
