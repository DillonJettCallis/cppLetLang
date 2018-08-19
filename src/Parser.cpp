//
// Created by Dillon on 1/26/2018.
//

#include "Parser.h"

#include <memory>
#include <utility>
#include <fstream>
#include <set>
#include "Utils.h"
#include "Tokens.h"
#include "Ast.h"

using namespace std;

typedef istreambuf_iterator<char> CharStream;


const string whiteSpace = " \n\t";
const string singleTokens = "(){},";
const string mergeTokens = ":=+-*/";

class Tokenizer {

private:

    string sourceFile;
    int x = 1;
    int y = 1;

public:
    explicit Tokenizer(string _sourceFile) {
        sourceFile = move(_sourceFile);
    }

    vector<Token> parse() {
        ifstream inputStream(sourceFile);

        if (!inputStream.is_open()) {
            throw runtime_error("Failed to open input file!");
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

            if (whiteSpace.find(next) != string::npos) {
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

    string readWord(CharStream& in, CharStream end) {
        string word;

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

        return word;
    }

    string readMergedSymbol(CharStream& in, CharStream end) {
        string word;

        while (in != end) {
            char next = *in;

            if (mergeTokens.find(next) != string::npos) {
                x++;
                word += next;
                in++;
            } else {
                return word;
            }
        }

        return word;
    }

    string readNumber(CharStream& in, CharStream end) {
        string word;

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

        return word;
    }

    vector<Token> readFile(CharStream start, CharStream end) {
        vector<Token> out = vector<Token>();
        CharStream in = start;

        while (in != end) {
            char next = *in;

            if (whiteSpace.find(next) != string::npos) {
                eatWhitespace(in, end);
            } else if (isalpha(next)) {
                out.emplace_back(point(), readWord(in, end), TokenType::Identifier);
            } else if (isdigit(next)) {
                out.emplace_back(point(), readNumber(in, end), TokenType::Number);
            } else if (singleTokens.find(next) != string::npos) {
                string word;
                word += next;
                in++;
                out.emplace_back(point(), word, TokenType::Symbol);
            } else if (mergeTokens.find(next) != string::npos) {
                out.emplace_back(point(), readMergedSymbol(in, end), TokenType::Symbol);
            } else {
                throw runtime_error("Illegal symbol: " + string(1, next));
            }
        }

        out.emplace_back(point(), "<EOF>", TokenType::Eof);
        return out;
    }
};

vector<Token> parseFile(string _sourceFile) {
    return Tokenizer(move(_sourceFile)).parse();
}


class Lexer {

    const set<string> sumOps = {"+", "-"};
    const set<string> productOps = {"*", "/", "**"};

    vector<Token> tokens;
    int index = 0;
public:
    explicit Lexer(vector<Token> tokens): tokens(move(tokens)) {

    }

    unique_ptr<Expression> readStatement() {
        auto firstWord = peek();

        if ("let" == firstWord.word) {
            skip();
            return readAssignment(firstWord.loc);
        } else if ("fun" == firstWord.word) {
            skip();
            return readFunction(firstWord.loc);
        } else {
            return readExpression();
        }
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

    void skip() {
        index++;
    }

    unique_ptr<Expression> readExpression() {
        return readCall();
    }
    
    unique_ptr<Expression> readCall() {
        auto left = readSum();

        auto maybeParen = peek();

        if (maybeParen.word == "(") {
            index++;

            vector<unique_ptr<Expression>> args;

            while(peek().word != ")") {
                args.push_back(readExpression());

                if (peek().word == ",") {
                    index++;
                }
            }

            index++;

            return make_unique<Call>(left->loc(), move(left), move(args));
        } else {
            return left;
        }
    }

    /**
     * Looks for + or - operators
     * @return
     */
    unique_ptr<Expression> readSum() {
        auto left = readProduct();

        auto maybeSymbol = peek();

        if (sumOps.count(maybeSymbol.word) == 1) {
            index++;

            auto right = readProduct();

            return make_unique<BinaryOp>(maybeSymbol.loc, make_unique<UnknownTypeToken>(), maybeSymbol.word, move(left), move(right));
        } else {
            return left;
        }
    }

    /**
     * Looks for * / or ** operators
     * @return
     */
    unique_ptr<Expression> readProduct() {
        auto left = readTerm();

        auto maybeSymbol = peek();

        if (productOps.count(maybeSymbol.word) == 1) {
            index++;

            auto right = readTerm();

            return make_unique<BinaryOp>(maybeSymbol.loc, make_unique<UnknownTypeToken>(), maybeSymbol.word, move(left), move(right));
        } else {
            return left;
        }
    }

    /**
     * Looks for a literal or a variable.
     * @return
     */
    unique_ptr<Expression> readTerm() {
        auto first = next();

        if (first.type == TokenType::Number) {
            double value = stod(first.word);
            return make_unique<NumberLiteral>(first.loc, value);
        } else if (first.type == TokenType::Identifier) {
            return make_unique<Variable>(first.loc, first.word, make_unique<UnknownTypeToken>());
        }

        throw runtime_error(first.expected("expression"));
    }

    unique_ptr<TypeToken> readMaybeType() {
        auto maybeColon = peek();

        if (maybeColon.word == ":") {
            // We have an explicit type.
            index++;
            auto typeName = next();

            if (typeName.type != TokenType::Identifier) {
                throw runtime_error(typeName.expected("type identifier"));
            }

            unique_ptr<TypeToken> type = make_unique<NamedTypeToken>(typeName.word);
            return type;
        } else {
            // Type must be implicit
            unique_ptr<TypeToken> type = make_unique<UnknownTypeToken>();
            return type;
        }
    }

    unique_ptr<Expression> readAssignment(Location loc) {
        auto id = next();

        if (id.type != TokenType::Identifier) {
            throw runtime_error(id.expected("identifier"));
        }

        auto type = readMaybeType();

        auto equals = next();

        if (equals.word != "=") {
            throw runtime_error(equals.expected("="));
        }

        auto body = readExpression();

        return make_unique<Assignment>(loc, move(type), id.word, move(body));
    }

    unique_ptr<Expression> readFunction(const Location &loc) {
        auto id = next();

        if (id.type != TokenType::Identifier) {
            throw runtime_error(id.expected("identifier"));
        }

        //TODO: Handle generics later
        auto openParen = next();

        if (openParen.word != "(") {
            throw runtime_error(openParen.expected("("));
        }

        vector<string> paramNames;
        vector<unique_ptr<TypeToken>> paramTypes;

        while (peek().word != ")") {
            auto paramId = next();

            if (paramId.type != TokenType::Identifier) {
                throw runtime_error(paramId.expected("identifier"));
            }

            auto colon = next();

            if (colon.word != ":") {
                throw runtime_error(colon.expected(":"));
            }

            auto paramType = next();

            if (paramType.type != TokenType::Identifier) {
                throw runtime_error(paramType.expected("type"));
            }

            paramNames.push_back(paramId.word);
            unique_ptr<TypeToken> type = make_unique<NamedTypeToken>(paramType.word);
            paramTypes.push_back(move(type));

            auto maybeComma = peek();

            if (maybeComma.word == ",") {
                index++;
            }
        }

        index++;

        auto colon = next();

        if (colon.word != ":") {
            throw runtime_error(colon.expected(":"));
        }

        auto resultType = next();

        if (resultType.type != TokenType::Identifier) {
            throw runtime_error(resultType.expected("type"));
        }

        auto resultToken = make_unique<NamedTypeToken>(resultType.word);

        unique_ptr<TypeToken> functionType = make_unique<BasicFunctionTypeToken>(move(paramTypes), move(resultToken));

        auto openBlock = next();

        if (openBlock.word != "{") {
            throw runtime_error(openBlock.expected("{"));
        }

        vector<unique_ptr<Expression>> body;

        while (peek().word != "}") {
            body.emplace_back(readStatement());
        }

        index++;

        return make_unique<Function>(loc, move(id.word), move(paramNames), move(functionType), move(body));
    }

};

class JsonPrinter {

    ofstream out;

public:

    JsonPrinter(const string &dest) {
        out.open(dest);
    }

    void println(const string &message) {
        out << message << endl;
    }

    void print(Expression *expression) {
        ExpressionKind kind = expression->kind;

        switch (kind) {
            case ExpressionKind::assignment: {
                auto ex = (Assignment *) expression;
                out << "{kind: 'assignment', id: '" << ex->id << "', type: " << typeName(ex->type.get()) << ", body: ";
                print(ex->body.get());
                out << "}";
                break;
            }
            case ExpressionKind::function: {
                auto ex = (Function *) expression;
                out << "{kind: 'function', id: '" << ex->id << "', type: " << typeName(ex->type.get()) << ", params: [";

                auto params = ex->params;

                if (!params.empty()) {
                    out << params[0];
                    for (int i = 1; i < params.size(); i++) {
                        out << ", " << params[i];
                    }
                }

                out << "], body: [";
                vector<unique_ptr<Expression>>& body = ex->body;

                if (!body.empty()) {
                    print(body[0].get());
                    for (int i = 1; i < body.size(); i++) {
                        out << ", ";
                        print(body[i].get());
                    }
                }
                out << "]}";
                break;
            }
            case ExpressionKind::binaryOp: {
                auto ex = (BinaryOp *) expression;
                out << "{kind: 'binaryOp', type: " << typeName(ex->type.get()) << ", op: '" << ex->op << "', left: ";
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
                throw runtime_error("Unknown Expression type");
        }
    }

    string typeName(TypeToken* typeToken) {
        TypeTokenKind kind = typeToken->kind;

        switch (kind) {
            case TypeTokenKind::named: {
                auto token = (NamedTypeToken *) typeToken;
                return "'" + token->id + "'";
            }
            case TypeTokenKind::basicFunction: {
                auto token = (BasicFunctionTypeToken *) typeToken;

                string result = "{params: [";

                vector<unique_ptr<TypeToken>>& params = token->params;

                if (!params.empty()) {
                    result += typeName(params[0].get());
                    for (int i = 1; i < params.size(); i++) {
                        result += ", ";
                        result += typeName(params[i].get());
                    }
                }

                result += "], result: ";
                result += typeName(token->result.get());
                result += "}";

                return result;
            }
            case TypeTokenKind::unknown:
                return "'<unknown>'";
            default:
                throw runtime_error("Unknown TypeToken type");
        }
    }

};

unique_ptr<Expression> lex(vector<Token> tokens) {
    auto lexer = Lexer(move(tokens));

    return move(lexer.readStatement());
}

void lexAndPrint(vector<Token> tokens) {


    auto lexer = Lexer(move(tokens));

    JsonPrinter printer("/home/dillon/projects/cppLetLang/build/ast.js");

    printer.println("const ast = [");

    while(!lexer.isDone()) {
        auto ex = lexer.readStatement();

        printer.print(ex.get());
        printer.println(",");
    }

    printer.println("];");
    println("Parsed and lexed");
}
