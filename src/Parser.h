//
// Created by Dillon on 1/26/2018.
//

#ifndef TYPEDLETLANG_PARSER_H
#define TYPEDLETLANG_PARSER_H

#include <vector>
#include "Tokens.h"
#include "Ast.h"


std::vector<Token> parseFile(std::string _sourceFile);

std::unique_ptr<Expression> lex(std::vector<Token> tokens);

void lexAndPrint(std::vector<Token> tokens);

#endif //TYPEDLETLANG_PARSER_H
