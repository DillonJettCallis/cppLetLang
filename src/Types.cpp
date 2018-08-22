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

NamedTypeToken::NamedTypeToken(std::string id) : TypeToken(TypeTokenKind::named), id(move(id)) {}

bool NamedTypeToken::operator==(TypeToken& other) {
    return other.kind == TypeTokenKind::named && ((NamedTypeToken&) other).id == id;
}

unique_ptr<TypeToken> NamedTypeToken::clone() {
    return make_unique<NamedTypeToken>(id);
}

string NamedTypeToken::pretty() {
    return "<" + id + ">";
}

BasicFunctionTypeToken::BasicFunctionTypeToken(std::vector<std::unique_ptr<TypeToken>> params, std::unique_ptr<TypeToken> result) :
        TypeToken(TypeTokenKind::basicFunction),
        params(std::move(params)),
        result(std::move(result)) {

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
