#include <iostream>
#include <fstream>
#include <sstream>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

static void printInfo() {
    std::cout << "Hello i am penguin , A brand new programming language !!\n";
    std::cout << "Version: 0.1.0\n";
    std::cout << "Meet my creator Tonmay Sardar !!\n";
    std::cout << "Usage: penguin <file.pg>\n";
}

static void printVersion() {
    std::cout << "Penguin Programming Language\n";
    std::cout << "Version: 0.1.0\n";
}

int main(int argc, char* argv[]) {

        // ---- No arguments ----
    if (argc == 1) {
        std::cerr << "Error: no input file\n";
        std::cerr << "Use: penguin <file.pg> or penguin --info\n";
        return 1;
    }

    // ---- Handle flags ----
    std::string arg1 = argv[1];

    if (arg1 == "--info") {
        printInfo();
        return 0;
    }

    if (arg1 == "--help" || arg1 == "-h") {
        printInfo();
        return 0;
    }

    if (arg1 == "--version" || arg1 == "-v") {
        printVersion();
        return 0;
    }



    if (argc != 2) {
        std::cerr << "Usage: penguin <file.pg>\n";
        return 1;
    }

    // 1. Read source file
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: could not open file " << argv[1] << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    try {
        // 2. Lex
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // 3. Parse
        Parser parser(tokens);
        auto program = parser.parse();

        // 4. Interpret
        Interpreter interpreter;
        interpreter.executeProgram(program.get());
    }
    catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
