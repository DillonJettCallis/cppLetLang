//
// Created by Dillon on 2018-08-12.
//

#include <memory>
#include "Types.h"

TypeToken::TypeToken(TypeTokenKind kind): kind(kind) {}

NamedTypeToken::NamedTypeToken(std::string id) : TypeToken(TypeTokenKind::named), id(std::move(id)) {}

BasicFunctionTypeToken::BasicFunctionTypeToken(std::vector<std::unique_ptr<TypeToken>> params, std::unique_ptr<TypeToken> result) :
        TypeToken(TypeTokenKind::basicFunction),
        params(std::move(params)),
        result(std::move(result)) {

}


UnknownTypeToken::UnknownTypeToken(): TypeToken(TypeTokenKind::unknown)  {}

