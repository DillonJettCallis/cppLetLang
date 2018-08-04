//
// Created by Dillon on 1/26/2018.
//

#include "Parser.h"

#include <memory>
#include <utility>
#include <fstream>
#include "Utils.h"

typedef std::istreambuf_iterator<char> CharStream;

Location::Location(std::string _sourceFile, int _x, int _y) {
    sourceFile = std::move(_sourceFile);
    x = _x;
    y = _y;
}

std::string Location::pretty() {
    return "file: " + sourceFile + ", line: " + std::to_string(y) + ", col: " + std::to_string(x);
}

Token::Token(Location loc, std::string value, TokenType type): loc(std::move(loc)), word(std::move(value)), type(type) {

};

std::string Token::expected(std::string expected) {
    return "Expected " + expected + " at " + loc.pretty() + " but found '" + word + "'";
}

enum TypeTokenKind {
    named,
    unknown
};

class TypeToken {
public:

    TypeTokenKind kind;

    TypeToken(TypeTokenKind kind): kind(kind) {}

};

class NamedTypeToken : public TypeToken {
public:

    std::string id;

    NamedTypeToken(std::string id) : TypeToken(TypeTokenKind::named), id(std::move(id)) {}

};

class UnknownTypeToken : public TypeToken {
public:

    UnknownTypeToken(): TypeToken(TypeTokenKind::unknown)  {}

};

enum ExpressionKind {
    assignment,
    numberLiteral
};

class Expression {
public:

    ExpressionKind kind;
    Location _loc;

    Expression(ExpressionKind kind, Location _loc): kind(kind), _loc(std::move(_loc)) {

    }

    Location loc() {
        return _loc;
    }

};

class Assignment : public Expression {
public:

    std::string id;
    std::unique_ptr<TypeToken> type;
    std::unique_ptr<Expression> body;

    Assignment(Location location, std::unique_ptr<TypeToken> type, std::string id, std::unique_ptr<Expression> body) :
            Expression(ExpressionKind::assignment, std::move(location)),
            type(std::move(type)),
            id(std::move(id)),
            body(std::move(body)) {

    }

};

class NumberLiteral : public Expression {
public:

    double value;

    NumberLiteral(Location location, double value) : Expression(ExpressionKind::numberLiteral, std::move(location)), value(value) {}

};

const std::string whiteSpace = " \n\t";
const std::string singleTokens = ";";
const std::string mergeTokens = ":=";

class Tokenizer {

private:

    std::string sourceFile;
    int x = 0;
    int y = 0;

public:
    explicit Tokenizer(std::string _sourceFile) {
        sourceFile = std::move(_sourceFile);
    }

    std::vector<Token> parse() {
        std::ifstream inputStream(sourceFile);

        if (!inputStream.is_open()) {
            throw std::runtime_error("Failed to open input file!");
        }

        CharStream start(inputStream);
        CharStream end;

        return readFile(start, end);
    }

private:

    Location point () {
        return {sourceFile, x, y};
    }

    void eatWhitespace(CharStream& in, CharStream end) {
        while (in != end) {
            char next = *in;

            if (whiteSpace.find(next) != std::string::npos) {
                if (next == '\n') {
                    x = 0;
                    y++;
                    in++;
                } else {
                    x++;
                    in++;
                }
            } else {
                return;
            }
        }
    }

    std::string readWord(CharStream& in, CharStream end) {
        std::string word;

        while (in != end) {
            char next = *in;

            if (isalpha(next)) {
                x++;
                word += next;
                in++;
            } else {
                return word;
            }
        }
    }

    std::string readMergedSymbol(CharStream& in, CharStream end) {
        std::string word;

        while (in != end) {
            char next = *in;

            if (mergeTokens.find(next) != std::string::npos) {
                x++;
                word += next;
                in++;
            } else {
                return word;
            }
        }
    }

    std::string readNumber(CharStream& in, CharStream end) {
        std::string word;

        while (in != end) {
            char next = *in;

            if (isdigit(next) || next == '.') {
                x++;
                word += next;
                in++;
            } else {
                return word;
            }
        }
    }

    std::vector<Token> readFile(CharStream start, CharStream end) {
        std::vector<Token> out = std::vector<Token>();
        CharStream in = start;

        while (in != end) {
            char next = *in;

            if (whiteSpace.find(next) != std::string::npos) {
                eatWhitespace(in, end);
            } else if (isalpha(next)) {
                out.emplace_back(point(), readWord(in, end), TokenType::Identifier);
            } else if (isdigit(next)) {
                out.emplace_back(point(), readNumber(in, end), TokenType::Number);
            } else if (singleTokens.find(next) != std::string::npos) {
                std::string word;
                word += next;
                in++;
                out.emplace_back(point(), word, TokenType::Symbol);
            } else if (mergeTokens.find(next) != std::string::npos) {
                out.emplace_back(point(), readMergedSymbol(in, end), TokenType::Symbol);
            }
        }

        return out;
    }
};

std::vector<Token> parseFile(std::string _sourceFile) {
    return Tokenizer(std::move(_sourceFile)).parse();
}


class Lexer {

    std::vector<Token> tokens;
    int index = 0;
public:
    explicit Lexer(std::vector<Token> tokens): tokens(std::move(tokens)) {

    }

    std::unique_ptr<Expression> readStatement() {
        auto firstWord = next();

        if ("let" == firstWord.word) {
            return readAssignment(firstWord.loc);
        }

        throw std::runtime_error(firstWord.expected("statement"));

    }

private:

    Token next() {
        return tokens[index++];
    }

    Token peek() {
        return tokens[index];
    }

    Token prev() {
        return tokens[--index];
    }

    std::unique_ptr<Expression> readExpression() {
        auto first = next();

        if (first.type == TokenType::Number) {
            double value = std::stod(first.word);
            return std::make_unique<NumberLiteral>(first.loc, value);
        }

        throw std::runtime_error(first.expected("expression"));
    }

    std::unique_ptr<TypeToken> readMaybeType() {
        auto maybeColon = peek();

        if (maybeColon.word == ":") {
            // We have an explicit type.
            index++;
            auto typeName = next();

            if (typeName.type != TokenType::Identifier) {
                throw std::runtime_error(typeName.expected("type identifier"));
            }

            std::unique_ptr<TypeToken> type = std::make_unique<NamedTypeToken>(maybeColon.word);
            return type;
        } else {
            // Type must be implicit
            std::unique_ptr<TypeToken> type = std::make_unique<UnknownTypeToken>();
            return type;
        }
    }

    std::unique_ptr<Expression> readAssignment(Location loc) {
        auto id = next();

        if (id.type != TokenType::Identifier) {
            throw std::runtime_error(id.expected("identifier"));
        }

        auto type = readMaybeType();

        auto equals = next();

        if (equals.word != "=") {
            throw std::runtime_error(equals.expected("="));
        }

        auto body = readExpression();

        return std::make_unique<Assignment>(loc, std::move(type), id.word, std::move(body));
    }

};

class JsonPrinter {
public:

    void print(Expression *expression) {
        ExpressionKind kind = expression->kind;

        switch (kind) {
            case ExpressionKind::assignment: {
                auto ex = (Assignment *) expression;
                std::cout << "{id: '" << ex->id << "', type: '" << typeName(ex->type.get()) << "', body: ";
                print(ex->body.get());
                std::cout << "}";
                break;
            }
            case ExpressionKind::numberLiteral: {
                auto ex = (NumberLiteral *) expression;
                std::cout << ex->value;
                break;
            }
            default:
                throw std::runtime_error("Unknown Expression type");
        }
    }

    std::string typeName(TypeToken* typeToken) {
        TypeTokenKind kind = typeToken->kind;

        switch (kind) {
            case TypeTokenKind::named: {
                auto token = (NamedTypeToken *) typeToken;
                return token->id;
            }
            case TypeTokenKind::unknown:
                return "<unknown>";
            default:
                throw std::runtime_error("Unknown TypeToken type");
        }
    }

};

void lexAndPrint(std::vector<Token> tokens) {
    auto ex = Lexer(std::move(tokens)).readStatement();

    JsonPrinter().print(ex.get());

    println("");
    println("Parsed and lexed");
}
