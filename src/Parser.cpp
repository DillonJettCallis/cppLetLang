//
// Created by Dillon on 1/26/2018.
//

#include "Parser.h"

#include <memory>
#include <utility>
#include <fstream>
#include <fstream>
#include <set>
#include "Utils.h"
#include "Tokens.h"
#include "Ast.h"

typedef std::istreambuf_iterator<char> CharStream;


const std::string whiteSpace = " \n\t";
const std::string singleTokens = ";";
const std::string mergeTokens = ":=+-*/";

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

        out.emplace_back(point(), "<EOF>", TokenType::Eof);
        return out;
    }
};

std::vector<Token> parseFile(std::string _sourceFile) {
    return Tokenizer(std::move(_sourceFile)).parse();
}


class Lexer {

    const std::set<std::string> sumOps = {"+", "-"};
    const std::set<std::string> productOps = {"*", "/", "**"};

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

    bool isDone()  {
        return tokens[index].type == TokenType::Eof;
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
        return readSum();
    }

    /**
     * Looks for + or - operators
     * @return
     */
    std::unique_ptr<Expression> readSum() {
        auto left = readProduct();

        auto maybeSymbol = peek();

        if (sumOps.count(maybeSymbol.word) == 1) {
            index++;

            auto right = readProduct();

            return std::make_unique<BinaryOp>(maybeSymbol.loc, std::make_unique<UnknownTypeToken>(), maybeSymbol.word, std::move(left), std::move(right));
        } else {
            return left;
        }
    }

    /**
     * Looks for * / or ** operators
     * @return
     */
    std::unique_ptr<Expression> readProduct() {
        auto left = readValue();

        auto maybeSymbol = peek();

        if (productOps.count(maybeSymbol.word) == 1) {
            index++;

            auto right = readValue();

            return std::make_unique<BinaryOp>(maybeSymbol.loc, std::make_unique<UnknownTypeToken>(), maybeSymbol.word, std::move(left), std::move(right));
        } else {
            return left;
        }
    }

    /**
     * Looks for a literal or a variable.
     * @return
     */
    std::unique_ptr<Expression> readValue() {
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

            std::unique_ptr<TypeToken> type = std::make_unique<NamedTypeToken>(typeName.word);
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

    std::ofstream out;

public:

    JsonPrinter(const std::string &dest) {
        out.open(dest);
    }

    void println(const std::string &message) {
        out << message << std::endl;
    }

    void print(Expression *expression) {
        ExpressionKind kind = expression->kind;

        switch (kind) {
            case ExpressionKind::assignment: {
                auto ex = (Assignment *) expression;
                out << "{kind: 'assignment', id: '" << ex->id << "', type: '" << typeName(ex->type.get()) << "', body: ";
                print(ex->body.get());
                out << "}";
                break;
            }
            case ExpressionKind::binaryOp: {
                auto ex = (BinaryOp *) expression;
                out << "{kind: 'binaryOp', type: '" << typeName(ex->type.get()) << "', op: '" << ex->op << "', left: ";
                print(ex->left.get());
                out << ", right: ";
                print(ex->right.get());
                out << "}";
                break;
            }
            case ExpressionKind::numberLiteral: {
                auto ex = (NumberLiteral *) expression;
                out << ex->value;
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


    auto lexer = Lexer(std::move(tokens));

    JsonPrinter printer("./build/ast.js");

    printer.println("const ast = [");

    while(!lexer.isDone()) {
        auto ex = lexer.readStatement();

        printer.print(ex.get());
        printer.println(",");
    }

    printer.println("];");
    println("Parsed and lexed");
}
