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
    base,
    unknown
};

enum BasicTypeTokenKind {
    Float,
    Unit
};

class TypeToken {
public:

    TypeTokenKind kind;

    explicit TypeToken(TypeTokenKind kind);

    virtual bool operator==(TypeToken& other) = 0;

    virtual bool operator!=(TypeToken& other);

    virtual std::unique_ptr<TypeToken> clone() = 0;

    virtual std::string pretty() = 0;

};

class NamedTypeToken : public TypeToken {
public:

    std::string id;

    NamedTypeToken(std::string id);

    bool operator==(TypeToken& other) override;

     std::unique_ptr<TypeToken> clone() override;

    std::string pretty() override;

};

class BasicFunctionTypeToken : public TypeToken {
public:

    std::vector<std::unique_ptr<TypeToken>> params;
    std::unique_ptr<TypeToken> result;

    BasicFunctionTypeToken(std::vector<std::unique_ptr<TypeToken>> params, std::unique_ptr<TypeToken> result);

    bool operator==(TypeToken& other) override;

    std::unique_ptr<TypeToken> clone() override;

    std::string pretty() override;

};

class BaseTypeToken : public TypeToken {
public:

    BasicTypeTokenKind base;

    explicit BaseTypeToken(BasicTypeTokenKind base);

    bool operator==(TypeToken& other) override;

    std::unique_ptr<TypeToken> clone() override;

    std::string pretty() override;

};

class UnknownTypeToken : public TypeToken {
public:

    UnknownTypeToken();

    bool operator==(TypeToken& other) override;

    std::unique_ptr<TypeToken> clone() override;

    std::string pretty() override;

};

#endif //TYPEDLETLANG_TYPES_H
