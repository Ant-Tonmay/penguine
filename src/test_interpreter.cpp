#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

struct ScopedRedirect {
    std::stringstream buffer;
    std::streambuf* old;

    ScopedRedirect() : old(std::cout.rdbuf(buffer.rdbuf())) {}
    ~ScopedRedirect() { std::cout.rdbuf(old); }

    std::string getString() const { return buffer.str(); }
};

bool runTest(const std::string& name, const std::string& source, const std::string& expectedOutput) {
    std::cout << "Running test: " << name << " ... ";
    
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);

    std::unique_ptr<Program> program;
    try {
        program = parser.parse();
    } catch (const std::exception& e) {
        std::cout << "FAILED (Parse Error: " << e.what() << ")" << std::endl;
        return false;
    }

    if (!program) {
        std::cout << "FAILED (Parse Error: null program)" << std::endl;
        return false;
    }

    ScopedRedirect capture;
    Interpreter interpreter;
    
    try {
        interpreter.executeProgram(program.get());
    } catch (const std::exception& e) {
        std::cout << "FAILED (Runtime Error: " << e.what() << ")" << std::endl;
        return false;
    }

    std::string output = capture.getString();
    
    if (output == expectedOutput) {
        std::cout << "PASSED" << std::endl;
        return true;
    } else {
        std::cout << "FAILED" << std::endl;
        std::cout << "  Expected: " << expectedOutput << "__END__" << std::endl;
        std::cout << "  Actual:   " << output << "__END__" << std::endl;
        return false;
    }
}

int main() {
    int passed = 0;
    int total = 0;

    // Test 1: Simple Print
    total++;
    if (runTest("Print Number", 
        "{ main() { print(42); } }", 
        "42\n"
    )) passed++;

    // Test 2: Arithmetic
    total++;
    if (runTest("Arithmetic", 
        "{ main() { print(10 + 32); } }", 
        "42\n"
    )) passed++;

    // Test 3: Variable Assignment and Use
    total++;
    if (runTest("Variables", 
        "{ main() { x = 10; y = 20; print(x + y); } }", 
        "30\n"
    )) passed++;

    // Test 4: For Loop
    total++;
    if (runTest("For Loop", 
        "{ main() { for (i = 0; i < 3; i = i + 1) { print(i); } } }", 
        "0\n1\n2\n"
    )) passed++;
    
    // Test 5: String Concatenation
    total++;
    if (runTest("String Concat",
        "{ main() { print(\"Hello \" + \"World\"); } }", 
        "Hello World\n"
    )) passed++;

    std::cout << "\n---------------------------------------------------" << std::endl;
    std::cout << "Test Summary: " << passed << "/" << total << " passed." << std::endl;
    if (passed == total) {
        std::cout << "SUCCESS: All tests passed!" << std::endl;
    } else {
        std::cout << "FAILURE: Some tests failed." << std::endl;
        return 1;
    }

    return 0;
}
