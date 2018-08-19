#include "src/Utils.h"
#include "src/Parser.h"
#include "src/Typechecker.h"
#include "src/Compiler.h"

int main() {
    println("Starting parse");

    try {
        auto tokens = parseFile("/home/dillon/projects/cppLetLang/test/basic.let");

        println("Parsed, doing lex");

        auto ast = lex(tokens);

        println("Done lex, doing typecheck");

        typeCheck(*ast);

        println("Done typecheck, doing compile");

        compile("/home/dillon/projects/cppLetLang/build/basic.ll", ast.get());

        println("Done compile");

        return 0;

    } catch (std::runtime_error& e) {
        std::cout << "Failed parse with error: " << e.what() << std::endl;
        return 1;
    }
}




