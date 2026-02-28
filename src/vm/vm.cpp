#include "vm/vm.h"
#include <iostream>

namespace vm {

void VM::push(Value v){
    stack.push_back(v);
}

Value VM::pop(){
    Value v = stack.back();
    stack.pop_back();
    return v;
}

void VM::run(Chunk& chunk){

    frames.push_back({&chunk,0,0});

    while(true){
        auto& frame = frames.back();
        uint8_t instruction = frame.chunk->code[frame.ip++];

        switch(instruction){

            case OP_CONSTANT:{
                uint8_t idx = frame.chunk->code[frame.ip++];
                push(frame.chunk->constants[idx]);
                break;
            }

            case OP_ADD:{
                double b = std::get<double>(pop());
                double a = std::get<double>(pop());
                push(a+b);
                break;
            }

            case OP_SUB:{
                double b = std::get<double>(pop());
                double a = std::get<double>(pop());
                push(a-b);
                break;
            }

            case OP_MUL:{
                double b = std::get<double>(pop());
                double a = std::get<double>(pop());
                push(a*b);
                break;
            }

            case OP_DIV:{
                double b = std::get<double>(pop());
                double a = std::get<double>(pop());
                push(a/b);
                break;
            }

            case OP_NEGATE:{
                double v = std::get<double>(pop());
                push(-v);
                break;
            }

            case OP_PRINT:{
                auto v = pop();
                std::cout << std::get<double>(v) << std::endl;
                break;
            }

            case OP_RETURN:{
                return;
            }

            case OP_HALT:
                return;
        }
    }
}

}
