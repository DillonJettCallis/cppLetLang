//
// Created by Dillon on 1/26/2018.
//

#ifndef TYPEDLETLANG_PARSER_H
#define TYPEDLETLANG_PARSER_H

#include <vector>
#include "Tokens.h"
#include "Ast.h"


std::vector<Token> parseFile(std::string _sourceFile);

std::unique_ptr<Module> lex(std::vector<Token> tokens);

void printModule(Module& module, std::string dest);

#endif //TYPEDLETLANG_PARSER_H
