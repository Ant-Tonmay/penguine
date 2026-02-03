#include "symbol_table/symbol_table.h"
#include "symbol_table/value.h"
#include "parser/ast.h"


class Interpreter {
public:
    Interpreter();

    void executeProgram(const Program* program);

private:
    SymbolTable* globals;
    SymbolTable* current;

    Value evaluate(const Expr* expr);
    void execute(const Stmt* stmt);
    void executeBlock(const Block* block);
};
