//
// Created by Dillon on 2018-08-12.
//

#include <memory>
#include "Types.h"

using namespace std;

TypeToken::TypeToken(TypeTokenKind kind): kind(kind) {}

bool TypeToken::operator!=(TypeToken& other) {
    return !(*this == other);
}

NamedTypeToken::NamedTypeToken(string id) : TypeToken(TypeTokenKind::named), id(move(id)) {}

bool NamedTypeToken::operator==(TypeToken& other) {
    return other.kind == TypeTokenKind::named && ((NamedTypeToken&) other).id == id;
}

unique_ptr<TypeToken> NamedTypeToken::clone() {
    return make_unique<NamedTypeToken>(id);
}

string NamedTypeToken::pretty() {
    return "<" + id + ">";
}

TypeConstructorTypeToken::TypeConstructorTypeToken(string base, int size) :
    TypeToken(TypeTokenKind::constructor),
    base(move(base)),
    size(size) {
}

bool TypeConstructorTypeToken::operator==(TypeToken& other) {
    if (other.kind == TypeTokenKind::constructor) {
        auto &realOther = (TypeConstructorTypeToken&) other;

        return base == realOther.base;
    } else {
        return false;
    }
}

unique_ptr<TypeToken> TypeConstructorTypeToken::clone() {
    return make_unique<TypeConstructorTypeToken>(base, size);
}

string TypeConstructorTypeToken::pretty() {
    string result = base + "<?";

    for (int i = 1; i < size; i++) {
        result += ", ?";
    }

    result += ">";

    return result;
}

GenericTypeToken::GenericTypeToken(unique_ptr<TypeConstructorTypeToken> parent, vector<unique_ptr<TypeToken>> typeParams) :
    TypeToken(TypeTokenKind::generic),
    parent(move(parent)),
    typeParams(move(typeParams)) {

}

bool GenericTypeToken::operator==(TypeToken& other) {
    if (other.kind == TypeTokenKind::generic) {
        auto &realOther = (GenericTypeToken&) other;

        return *parent == *realOther.parent && equal(typeParams.begin(), typeParams.end(), realOther.typeParams.begin(), [](const unique_ptr<TypeToken>& left, const unique_ptr<TypeToken>& right){ return *left == *right;});
    } else {
        return false;
    }
}

unique_ptr<TypeToken> GenericTypeToken::clone() {
    vector<unique_ptr<TypeToken>> newParams;
    newParams.reserve(typeParams.size());

    for (auto &param : typeParams) {
        newParams.push_back(move(param->clone()));
    }

    auto *parentClone = dynamic_cast<TypeConstructorTypeToken*>(parent->clone().release());

    return make_unique<GenericTypeToken>(move(unique_ptr<TypeConstructorTypeToken>(parentClone)), move(newParams));
}

string GenericTypeToken::pretty() {
    return parent->base + "<" + typeName(typeParams) + ">";
}

BasicFunctionTypeToken::BasicFunctionTypeToken(vector<unique_ptr<TypeToken>> params, unique_ptr<TypeToken> result) :
        TypeToken(TypeTokenKind::basicFunction),
        params(move(params)),
        result(move(result)) {

}

bool BasicFunctionTypeToken::operator==(TypeToken& other) {
    if (other.kind == TypeTokenKind::basicFunction) {
        auto &realOther = (BasicFunctionTypeToken&) other;

        return params == realOther.params && result == realOther.result;
    } else {
        return false;
    }
}

unique_ptr<TypeToken> BasicFunctionTypeToken::clone() {
    vector<unique_ptr<TypeToken>> newParams;
    newParams.reserve(params.size());

    for (auto &param : params) {
        newParams.push_back(param->clone());
    }

    return make_unique<BasicFunctionTypeToken>(move(newParams), move(result->clone()));
}

string BasicFunctionTypeToken::pretty() {
    string ans = "(";

    if (!params.empty()) {
        ans += params[0]->pretty();
        for (int i = 1; i < params.size(); i++) {
            ans += ", ";
            ans += params[0]->pretty();
        }
    }

    ans += ") => ";
    ans += result->pretty();

    return ans;
}

BaseTypeToken::BaseTypeToken(BasicTypeTokenKind base) : TypeToken(TypeTokenKind::base), base(base) {

}

bool BaseTypeToken::operator==(TypeToken& other) {
    return other.kind == TypeTokenKind::base && ((BaseTypeToken&) other).base == base;
}

unique_ptr<TypeToken> BaseTypeToken::clone() {
    return make_unique<BaseTypeToken>(base);
}

string BaseTypeToken::pretty() {
    switch(base) {
        case BasicTypeTokenKind::Float:
            return "Float";
        case BasicTypeTokenKind::Boolean:
            return "Boolean";
        case BasicTypeTokenKind::Unit:
            return "Unit";
        default:
            throw runtime_error("Unknown base type token");
    }
}

UnknownTypeToken::UnknownTypeToken(): TypeToken(TypeTokenKind::unknown)  {}

bool UnknownTypeToken::operator==(TypeToken& other) {
    return other.kind == TypeTokenKind::unknown;
}

unique_ptr<TypeToken> UnknownTypeToken::clone() {
    return make_unique<UnknownTypeToken>();
}

string UnknownTypeToken::pretty() {
    return "<Unknown>";
}

string typeName(TypeToken& type) {
    return type.pretty();
}

string typeName(vector<unique_ptr<TypeToken>>& type) {
    string result;

    if (!type.empty()) {
        result += typeName(*type[0].get());
        for (int i = 1; i < type.size(); i++) {
            result += ", ";
            result += typeName(*type[i].get());
        }
    }

    return result;
}

