//
// Created by Dillon on 2018-08-12.
//

#ifndef TYPEDLETLANG_TYPES_H
#define TYPEDLETLANG_TYPES_H

#include <memory>
#include <vector>

enum TypeTokenKind {
    named,
    basicFunction,
    unknown
};

class TypeToken {
public:

    TypeTokenKind kind;

    TypeToken(TypeTokenKind kind);

};

class NamedTypeToken : public TypeToken {
public:

    std::string id;

    NamedTypeToken(std::string id);

};

class BasicFunctionTypeToken : public TypeToken {
public:

    std::vector<std::unique_ptr<TypeToken>> params;
    std::unique_ptr<TypeToken> result;

    BasicFunctionTypeToken(std::vector<std::unique_ptr<TypeToken>> params, std::unique_ptr<TypeToken> result);

};

class UnknownTypeToken : public TypeToken {
public:

    UnknownTypeToken();

};

#endif //TYPEDLETLANG_TYPES_H
