//
// Created by Dillon on 2018-08-12.
//

#include <memory>
#include "Types.h"

TypeToken::TypeToken(TypeTokenKind kind): kind(kind) {}

NamedTypeToken::NamedTypeToken(std::string id) : TypeToken(TypeTokenKind::named), id(std::move(id)) {}

UnknownTypeToken::UnknownTypeToken(): TypeToken(TypeTokenKind::unknown)  {}

