//
// Created by Dillon on 1/26/2018.
//

#ifndef TYPEDLETLANG_PARSER_H
#define TYPEDLETLANG_PARSER_H

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
    Number
};

class Token {
public:

    Location loc;
    std::string word;
    TokenType type;

    Token(Location loc, std::string word, TokenType type);

    std::string expected(std::string expected);
};

std::vector<Token> parseFile(std::string _sourceFile);

void lexAndPrint(std::vector<Token> tokens);

#endif //TYPEDLETLANG_PARSER_H
