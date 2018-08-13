//
// Created by Dillon on 1/26/2018.
//

#ifndef TYPEDLETLANG_PARSER_H
#define TYPEDLETLANG_PARSER_H

#include <vector>
#include "Tokens.h"


std::vector<Token> parseFile(std::string _sourceFile);

void lexAndPrint(std::vector<Token> tokens);

#endif //TYPEDLETLANG_PARSER_H
