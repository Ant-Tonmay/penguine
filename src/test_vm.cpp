#include <iostream>
#include <vector>
#include <cassert>
#include "vm/vm.h"
#include "vm/chunk.h"
#include "vm/opcode.h"
#include "vm/value.h"

// Helper to create a simple chunk
void test_basic_arithmetic() {
    std::cout << "Testing Basic Arithmetic..." << std::endl;
    
    vm::FunctionObject fn("test", 0);
    vm::VM vm;

    // 1.2 + 3.4
    int c1 = fn.chunk.addConstant(1.2);
    int c2 = fn.chunk.addConstant(3.4);

    fn.chunk.write(vm::OP_CONSTANT);
    fn.chunk.write(c1);

    fn.chunk.write(vm::OP_CONSTANT);
    fn.chunk.write(c2);

    fn.chunk.write(vm::OP_ADD);
    fn.chunk.write(vm::OP_PRINT);
    fn.chunk.write(vm::OP_HALT);

    vm.run(&fn);
    
    std::cout << "Basic Arithmetic Test Passed (Clean Run)" << std::endl;
}

int main() {
    test_basic_arithmetic();
    return 0;
}

