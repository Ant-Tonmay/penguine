#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer/lexer.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: penguin <file.pg>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Could not open file\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    Lexer lexer(buffer.str());
    auto tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << token.toString() << "\n";
    }

    return 0;
}
