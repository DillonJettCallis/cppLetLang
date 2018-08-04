#include "src/Utils.h"
#include "src/Parser.h"


int main() {
    println("Hello, World!");

    try {
        auto tokens = parseFile(R"(C:\Users\Dillon\Projects\typedLetLang\test\basic.let)");

        std::cout << "{";

        for (auto next : tokens) {
            std::cout << "'" << next.word << "', ";
        }

        std::cout << "'EOF'}" << std::endl;

        lexAndPrint(tokens);

        return 0;

    } catch (std::runtime_error& e) {
        std::cout << "Failed parse with error: " << e.what() << std::endl;
        return 1;
    }
}




