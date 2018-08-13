//
// Created by Dillon on 2018-08-12.
//

#ifndef TYPEDLETLANG_TYPES_H
#define TYPEDLETLANG_TYPES_H

#include <memory>

enum TypeTokenKind {
    named,
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

class UnknownTypeToken : public TypeToken {
public:

    UnknownTypeToken();

};

#endif //TYPEDLETLANG_TYPES_H
