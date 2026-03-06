#include "vm/compiler.h"
#include <string>
#include "lexer/lexer.h"
#include "parser/parser.h"

namespace vm {


Chunk& Compiler::currentChunk() {
    return currentFunction->chunk;
}

void Compiler::emit(uint8_t byte) {
    currentChunk().write(byte);
}

void Compiler::emitConstant(Value v) {
    int idx = currentChunk().addConstant(v);
    emit(OP_CONSTANT);
    emit(idx);
}


int Compiler::emitJump(uint8_t instruction) {
    emit(instruction);
    emit(0xff);
    emit(0xff);
    return currentChunk().code.size() - 2;
}

void Compiler::patchJump(int offset) {
    int jump = currentChunk().code.size() - offset - 2;
    if (jump > UINT16_MAX) {
        // Technically this should throw a compiler error
    }
    currentChunk().code[offset] = (jump >> 8) & 0xff;
    currentChunk().code[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart) {
    emit(OP_LOOP);
    int offset = currentChunk().code.size() - loopStart + 2;
    if (offset > UINT16_MAX) {
        // Technically this should throw a compiler error
    }
    emit((offset >> 8) & 0xff);
    emit(offset & 0xff);
}


void Compiler::beginScope() {
    scopeDepth++;
}

void Compiler::endScope() {
    scopeDepth--;
    while (locals.size() > 0 && locals.back().depth > scopeDepth) {
        emit(OP_POP);
        locals.pop_back();
    }
}

void Compiler::addLocal(const std::string& name) {
    locals.push_back({name, scopeDepth});
}

int Compiler::resolveLocal(const std::string& name) {
    for (int i = locals.size() - 1; i >= 0; i--) {
        if (locals[i].name == name) {
            return i;
        }
    }
    return -1;
}


void Compiler::compileFunction(Function* func) {
    // Create a new FunctionObject for this function
    auto* fnObj = new FunctionObject(func->name, func->params.size());

    // Save current compiler state
    FunctionObject* enclosingFunction = currentFunction;
    std::vector<Local> enclosingLocals = std::move(locals);
    int enclosingScopeDepth = scopeDepth;

    // Switch to the new function context
    currentFunction = fnObj;
    locals.clear();
    scopeDepth = 0;

    // Begin scope for function body
    beginScope();

    addLocal(""); // Reserve slot 0 for callee

    for (const auto& param : func->params) {
        addLocal(param.name);
    }

    for (const auto& stmt : func->body->statements) {
        compileStmt(stmt.get());
    }

    emit(OP_NULL);
    emit(OP_RETURN);

    currentFunction = enclosingFunction;
    locals = std::move(enclosingLocals);
    scopeDepth = enclosingScopeDepth;

    compiledFunctions.push_back(fnObj);
}

FunctionObject* Compiler::compile(ASTNode* node) {
    if (auto* program = dynamic_cast<Program*>(node)) {
       
        for (const auto& func : program->functions) {
            if (func->name != "main") {
                compileFunction(func.get());
            }
        }

       
        auto* scriptFn = new FunctionObject("__script__", 0);
        currentFunction = scriptFn;
        locals.clear();
        scopeDepth = 0;

        for (const auto& cls : program->classes) {
            compileStmt(cls.get());
        }

        bool foundMain = false;
        for (const auto& func : program->functions) {
            if (func->name == "main") {
                beginScope();
                for (const auto& stmt : func->body->statements) {
                    compileStmt(stmt.get());
                }
                endScope();
                foundMain = true;
                break;
            }
        }

        emit(OP_HALT);
        return scriptFn;
    }

    auto* scriptFn = new FunctionObject("__script__", 0);
    currentFunction = scriptFn;
    compileStmt(node);
    emit(OP_RETURN);
    return scriptFn;
}

void Compiler::compileExpr(ASTNode* node){
    if (auto* num = dynamic_cast<NumberExpr*>(node)) {
        try {
            double v = std::stod(num->value);
            emitConstant(v);
        } catch (...) {}
    }
    else if (auto* b = dynamic_cast<BoolExpr*>(node)) {
        if (b->value) emit(OP_TRUE);
        else emit(OP_FALSE);
    }
    else if (auto* s = dynamic_cast<StringExpr*>(node)) {
        const std::string& str = s->value;
        bool hasInterpolation = (str.find('{') != std::string::npos);
        if (!hasInterpolation) {
            emitConstant(str);
        } else {
            int partCount = 0;
            size_t i = 0;
            while (i < str.length()) {
                if (str[i] == '{') {
                    size_t j = i + 1;
                    while (j < str.length() && str[j] != '}') j++;
                    if (j < str.length()) {
                        std::string exprStr = str.substr(i + 1, j - i - 1);
                        // Lex and parse the expression, then compile it
                        Lexer lexer(exprStr);
                        auto tokens = lexer.tokenize();
                        Parser parser(tokens);
                        auto expr = parser.parseExpression();
                        compileExpr(expr.get());
                        partCount++;
                        i = j + 1;
                        continue;
                    }
                }
                // Collect literal segment
                std::string literal;
                while (i < str.length() && str[i] != '{') {
                    literal += str[i];
                    i++;
                }
                if (!literal.empty()) {
                    emitConstant(literal);
                    partCount++;
                }
            }
            // Concatenate all parts with OP_ADD
            for (int p = 1; p < partCount; p++) {
                emit(OP_ADD);
            }
        }
    }
    else if (auto* var = dynamic_cast<VarExpr*>(node)) {
        int arg = resolveLocal(var->name);
        if (arg != -1) {
            emit(OP_GET_LOCAL);
            emit(arg);
        } else {
    
            int thisArg = resolveLocal("this");
            Value nameVal = var->name;
            int idx = currentChunk().addConstant(nameVal);
            
            if (thisArg != -1) {
                emit(OP_GET_LOCAL);
                emit(thisArg);
                emit(OP_GET_PROPERTY_OR_GLOBAL);
                emit(idx);
            } else {
                emit(OP_GET_GLOBAL);
                emit(idx);
            }
        }
    }
    else if (auto* call = dynamic_cast<CallExpr*>(node)) {
        // Check for built-in: fixed(size) or fixed(size, init)
        if (auto* calleeName = dynamic_cast<VarExpr*>(call->callee.get())) {
            if (calleeName->name == "fixed") {
                for (const auto& arg : call->arguments) {
                    compileExpr(arg.get());
                }
                emit(OP_FIXED_ARRAY);
                emit(static_cast<uint8_t>(call->arguments.size()));
                return;
            }
            if (calleeName->name == "push") {
                for (const auto& arg : call->arguments) {
                    compileExpr(arg.get());
                }
                emit(OP_ARRAY_PUSH);
                return;
            }
            if (calleeName->name == "length") {
                for (const auto& arg : call->arguments) {
                    compileExpr(arg.get());
                }
                emit(OP_ARRAY_LENGTH);
                return;
            }
        }

        // Check for built-in: arr.push(val)
        if (auto* mem = dynamic_cast<MemberExpr*>(call->callee.get())) {
            if (mem->name == "push") {
                compileExpr(mem->object.get());  // push the array
                for (const auto& arg : call->arguments) {
                    compileExpr(arg.get());
                }
                emit(OP_ARRAY_PUSH);
                return;
            }
            
            // Method call on an object
            compileExpr(mem->object.get()); // push the object
            int nameIdx = currentChunk().addConstant(mem->name);
            emit(OP_GET_PROPERTY);
            emit(nameIdx);
            
            // push arguments
            for (const auto& arg : call->arguments) {
                compileExpr(arg.get());
            }

            emit(OP_CALL);
            emit(static_cast<uint8_t>(call->arguments.size()));
            return;
        }

        // Generic function call
        compileExpr(call->callee.get());

        for (const auto& arg : call->arguments) {
            compileExpr(arg.get());
        }

        emit(OP_CALL);
        emit(static_cast<uint8_t>(call->arguments.size()));
    }
    else if (auto* arr = dynamic_cast<ArrayExpr*>(node)) {
        for (const auto& el : arr->elements) {
            compileExpr(el.get());
        }
        emit(OP_NEW_ARRAY);
        emit(static_cast<uint8_t>(arr->elements.size()));
    }
    else if (auto* idx = dynamic_cast<IndexExpr*>(node)) {
        compileExpr(idx->array.get());
        compileExpr(idx->index.get());
        emit(OP_INDEX_GET);
    }
    else if (auto* bin = dynamic_cast<BinaryExpr*>(node)) {

        // Short-circuiting for && and ||
        if (bin->op == "&&") {
            compileExpr(bin->left.get());
            int endJump = emitJump(OP_JUMP_IF_FALSE);
            emit(OP_POP); // Pop the left operand if it's true, because we need the right operand's value
            compileExpr(bin->right.get());
            patchJump(endJump);
            return;
        } 
        else if (bin->op == "||") {
            compileExpr(bin->left.get());
            // If left is true, jump to the end and keep 'true' (on the stack).
            // If false, jump to evaluate right. 
            int elseJump = emitJump(OP_JUMP_IF_FALSE);
            int endJump = emitJump(OP_JUMP);
            
            patchJump(elseJump);
            emit(OP_POP); // Left is false, pop it
            compileExpr(bin->right.get());
            
            patchJump(endJump);
            return;
        }

        compileExpr(bin->left.get());
        compileExpr(bin->right.get());

        if (bin->op == "+") emit(OP_ADD);
        else if (bin->op == "-") emit(OP_SUB);
        else if (bin->op == "*") emit(OP_MUL);
        else if (bin->op == "/") emit(OP_DIV);
        else if (bin->op == "%") emit(OP_MOD);
        else if (bin->op == ">") emit(OP_GREATER);
        else if (bin->op == ">=") emit(OP_GREATER_EQUAL);
        else if (bin->op == "<") emit(OP_LESSER);
        else if (bin->op == "<=") emit(OP_LESSER_EQUAL);
        else if (bin->op == ">>") emit(OP_RIGHT_SHIFT);
        else if (bin->op == "<<") emit(OP_LEFT_SHIFT);
        else if (bin->op == "|") emit(OP_BITWISE_OR);
        else if (bin->op == "&") emit(OP_BITWISE_AND);
        else if (bin->op == "==") emit(OP_EQUAL);
        else if (bin->op == "!=") emit(OP_NOT_EQUAL);
        else if (bin->op == "^") emit(OP_XOR);
        else if (bin->op == "+=") emit(OP_PLUS_EQUAL);
        else if (bin->op == "-=") emit(OP_MINUS_EQUAL);
        else if (bin->op == "*=") emit(OP_MULTIPLY_EQUAL);
        else if (bin->op == "/=") emit(OP_DIVIDE_EQUAL);
        else if (bin->op == "%=") emit(OP_MODULO_EQUAL);
        else if (bin->op == "<<=") emit(OP_LEFT_SHIFT_EQUAL);
        else if (bin->op == ">>=") emit(OP_RIGHT_SHIFT_EQUAL);
        else if (bin->op == "&=") emit(OP_BITWISE_AND_EQUAL);
        else if (bin->op == "|=") emit(OP_BITWISE_OR_EQUAL);
        else if (bin->op == "^=") emit(OP_XOR_EQUAL);
    }
    else if (auto* mem = dynamic_cast<MemberExpr*>(node)) {
        compileExpr(mem->object.get());
        int nameIdx = currentChunk().addConstant(mem->name);
        emit(OP_GET_PROPERTY);
        emit(nameIdx);
    }
}

void Compiler::compileStmt(ASTNode* node) {
    if (auto* printStmt = dynamic_cast<PrintStmt*>(node)) {
        compileExpr(printStmt->expression.get());
        emit(OP_PRINT);
    }
    else if (auto* printlnStmt = dynamic_cast<PrintlnStmt*>(node)) {
        compileExpr(printlnStmt->expression.get());
        emit(OP_PRINTLN);
    }
    else if (auto* exprStmt = dynamic_cast<ExprStmt*>(node)) {
        compileExpr(exprStmt->expression.get());
        emit(OP_POP);
    }
    else if (auto* returnStmt = dynamic_cast<ReturnStmt*>(node)) {
        if (returnStmt->value) {
            compileExpr(returnStmt->value.get());
        } else {
            emit(OP_NULL);
        }
        emit(OP_RETURN);
    }
    else if (auto* ifStmt = dynamic_cast<IfStmt*>(node)) {
        compileExpr(ifStmt->condition.get());
        int thenJump = emitJump(OP_JUMP_IF_FALSE);
        emit(OP_POP); 
        
        compileStmt(ifStmt->thenBranch.get());
        
        int elseJump = emitJump(OP_JUMP);
        patchJump(thenJump);
        emit(OP_POP); // Pop condition when condition was false!
        
        if (ifStmt->elseBranch) {
            compileStmt(ifStmt->elseBranch.get());
        }
        
        patchJump(elseJump);
    }
    else if (auto* whileStmt = dynamic_cast<WhileStmt*>(node)) {
        int loopStart = currentChunk().code.size();
        
        loopStack.push_back({loopStart, {}});
        
        compileExpr(whileStmt->condition.get());
        int exitJump = emitJump(OP_JUMP_IF_FALSE);
        emit(OP_POP); // Pop condition if we enter the loop
        
        compileStmt(whileStmt->body.get());
        emitLoop(loopStart);
        
        patchJump(exitJump);
        emit(OP_POP); // Pop condition if we just exited the loop
        
        // Patch all break jumps to here (after the loop)
        for (int bj : loopStack.back().breakJumps) {
            patchJump(bj);
        }
        loopStack.pop_back();
    }
    else if (auto* forStmt = dynamic_cast<ForStmt*>(node)) {
        beginScope();
        
        if (forStmt->init) {
            compileStmt(forStmt->init.get());
        }
        
        int loopStart = currentChunk().code.size();
        int exitJump = -1;
        
        if (forStmt->condition) {
            compileExpr(forStmt->condition.get());
            exitJump = emitJump(OP_JUMP_IF_FALSE);
            emit(OP_POP); // Pop condition if we enter the loop
        }
        
        // loopStart = -1 signals that continue needs forward jumps (patched to increment)
        loopStack.push_back({-1, {}, {}});
        
        compileStmt(forStmt->body.get());
        
        // Patch all continue jumps to here (the increment section)
        for (int cj : loopStack.back().continueJumps) {
            patchJump(cj);
        }
        
        if (forStmt->increment) {
            compileStmt(forStmt->increment.get());
        }
        
        emitLoop(loopStart);
        
        if (exitJump != -1) {
            patchJump(exitJump);
            emit(OP_POP); // Pop condition if we just exited the loop
        }
        
        // Patch all break jumps to here (after the loop)
        for (int bj : loopStack.back().breakJumps) {
            patchJump(bj);
        }
        loopStack.pop_back();
        
        endScope();
    }
    else if (auto* assignStmt = dynamic_cast<AssignmentStmt*>(node)) {
        for (const auto& assign : assignStmt->assignments) {
            if (auto* var = dynamic_cast<VarExpr*>(assign.target.get())) {
                int arg = resolveLocal(var->name);
                bool isLocal = true;
                bool isNewLocal = false;

                if (arg == -1 && scopeDepth > 0) {
                     int thisArg = resolveLocal("this");
                     if (thisArg != -1) {
                        
                         Value nameVal = var->name;
                         int idx = currentChunk().addConstant(nameVal);
                         
                         compileExpr(assign.value.get());
                         
                         if (assign.op != TokenType::EQUAL) {
                              
                              emit(OP_GET_LOCAL);
                              emit(thisArg);
                              emit(OP_GET_PROPERTY_OR_GLOBAL);
                              emit(idx);
                              
                              switch(assign.op) {
                                  case TokenType::PLUS_EQUAL: emit(OP_PLUS_EQUAL); break;
                                  case TokenType::MINUS_EQUAL: emit(OP_MINUS_EQUAL); break;
                                  case TokenType::STAR_EQUAL: emit(OP_MULTIPLY_EQUAL); break;
                                  case TokenType::SLASH_EQUAL: emit(OP_DIVIDE_EQUAL); break;
                                  case TokenType::MOD_OP_EQUAL: emit(OP_MODULO_EQUAL); break;
                                  case TokenType::BITWISE_AND_EQUAL: emit(OP_BITWISE_AND_EQUAL); break;
                                  case TokenType::BITWISE_OR_EQUAL: emit(OP_BITWISE_OR_EQUAL); break;
                                  case TokenType::XOR_EQUAL: emit(OP_XOR_EQUAL); break;
                                  default: break;
                              }
                         }

                         emit(OP_GET_LOCAL);
                         emit(thisArg);
                         emit(OP_SET_PROPERTY_OR_LOCAL);
                         emit(idx);
                         
                         // Not a new local, handled exclusively via property binding. Continue to next assignment
                         continue;
                     }
                     
                     // Inside a function, any assignment to an unknown variable becomes a new local
                     addLocal(var->name);
                     arg = locals.size() - 1;
                     isNewLocal = true;
                } else if (arg == -1 && scopeDepth == 0) {
                     // Global scope
                     isLocal = false;
                     Value nameVal = var->name;
                     arg = currentChunk().addConstant(nameVal);
                }

                if (assign.op != TokenType::EQUAL) {
                    if (isLocal) {
                        emit(OP_GET_LOCAL);
                        emit(arg);
                    } else {
                        emit(OP_GET_GLOBAL);
                        emit(arg);
                    }
                    compileExpr(assign.value.get());
                    
                    switch(assign.op) {
                        case TokenType::PLUS_EQUAL: emit(OP_PLUS_EQUAL); break;
                        case TokenType::MINUS_EQUAL: emit(OP_MINUS_EQUAL); break;
                        case TokenType::STAR_EQUAL: emit(OP_MULTIPLY_EQUAL); break;
                        case TokenType::SLASH_EQUAL: emit(OP_DIVIDE_EQUAL); break;
                        case TokenType::MOD_OP_EQUAL: emit(OP_MODULO_EQUAL); break;
                        case TokenType::BITWISE_AND_EQUAL: emit(OP_BITWISE_AND_EQUAL); break;
                        case TokenType::BITWISE_OR_EQUAL: emit(OP_BITWISE_OR_EQUAL); break;
                        case TokenType::XOR_EQUAL: emit(OP_XOR_EQUAL); break;
                        default: break; // Or OP_ADD
                    }
                } else {
                    compileExpr(assign.value.get());
                }
                
                if (isNewLocal) {
                    // Eat Five star , Do Nothing for now
                } else if (isLocal) {
                    emit(OP_SET_LOCAL);
                    emit(arg);
                    emit(OP_POP);
                } else {
                    emit(OP_SET_GLOBAL);
                    emit(arg);
                    emit(OP_POP);
                }
            }
            else if (auto* idx = dynamic_cast<IndexExpr*>(assign.target.get())) {
                compileExpr(idx->array.get());
                compileExpr(idx->index.get());
                compileExpr(assign.value.get());
                emit(OP_INDEX_SET);
            }
            else if (auto* mem = dynamic_cast<MemberExpr*>(assign.target.get())) {
                compileExpr(mem->object.get()); // The object
                compileExpr(assign.value.get()); // The value
                int nameIdx = currentChunk().addConstant(mem->name);
                
                if (assign.op != TokenType::EQUAL) {
                    // Eat Five star , Do Nothing for now (For now)
                }

                emit(OP_SET_PROPERTY);
                emit(nameIdx);
            }
        }
    }
    else if (auto* block = dynamic_cast<Block*>(node)) {
        beginScope();
        for (const auto& stmt : block->statements) {
            compileStmt(stmt.get());
        }
        endScope();
    }
    else if (dynamic_cast<BreakStmt*>(node)) {
        if (!loopStack.empty()) {
            int breakJump = emitJump(OP_JUMP);
            loopStack.back().breakJumps.push_back(breakJump);
        }
    }
    else if (dynamic_cast<ContinueStmt*>(node)) {
        if (!loopStack.empty()) {
            auto& loop = loopStack.back();
            if (loop.loopStart >= 0) {
                // While loop: jump back to condition
                emitLoop(loop.loopStart);
            } else {
                // For loop: forward jump to increment (patched later)
                int continueJump = emitJump(OP_JUMP);
                loop.continueJumps.push_back(continueJump);
            }
        }
    }
    else if (dynamic_cast<Expr*>(node)) {
        compileExpr(node);
        emit(OP_POP);
    }
    else if (auto* classStmt = dynamic_cast<ClassStmt*>(node)) {
        int nameIdx = currentChunk().addConstant(classStmt->name);
        emit(OP_CLASS);
        emit(nameIdx);

        // Define class locally or globally
        int arg = resolveLocal(classStmt->name);
        bool isLocal = true;
        if (arg == -1 && scopeDepth > 0) {
            addLocal(classStmt->name);
            arg = locals.size() - 1;
        } else if (arg == -1 && scopeDepth == 0) {
            isLocal = false;
            arg = currentChunk().addConstant(classStmt->name);
        }

        if (isLocal) {
            emit(OP_SET_LOCAL);
            emit(arg);
        } else {
            emit(OP_SET_GLOBAL);
            emit(arg);
        }
        // DO NOT POP the class from the stack yet, we need it to bind methods
        
        for (const auto& section : classStmt->sections) {
            for (const auto& member : section->members) {
                if (auto* method = dynamic_cast<MethodDef*>(member.get())) {
                    
                    Function func(method->name, method->params, std::make_unique<Block>());
                    
                    auto* fnObj = new FunctionObject(method->name, method->params.size() + 1, true);
                    FunctionObject* enclosingFunction = currentFunction;
                    std::vector<Local> enclosingLocals = std::move(locals);
                    int enclosingScopeDepth = scopeDepth;
                    
                    currentFunction = fnObj;
                    locals.clear();
                    scopeDepth = 0;
                    
                    beginScope();
                    addLocal("this"); // implicit first parameter
                    for (const auto& param : method->params) {
                        addLocal(param.name);
                    }
                    
                    for (const auto& stmt : method->body->statements) {
                        compileStmt(stmt.get());
                    }
                    
                    // For constructors, we strictly want to return `this`
                    if (method->name == classStmt->name) {
                        emit(OP_GET_LOCAL);
                        emit(0); // `this` is slot 0
                    } else {
                        emit(OP_NULL); // Standard methods return null implicitly
                    }
                    emit(OP_RETURN);
                    
                    currentFunction = enclosingFunction;
                    locals = std::move(enclosingLocals);
                    scopeDepth = enclosingScopeDepth;
                    
                    compiledFunctions.push_back(fnObj);
                    
                    // Now bind it
                    emitConstant(fnObj); // push the method function
                    int methodNameIdx = currentChunk().addConstant(method->name);
                    emit(OP_METHOD);
                    emit(methodNameIdx);
                }
            }
        }
        
        emit(OP_POP); // Pop the class after binding all methods
    }
}

}
