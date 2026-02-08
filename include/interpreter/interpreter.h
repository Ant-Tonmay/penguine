#pragma once

#include "parser/ast.h"
#include "interpreter/environment.h"
#include "interpreter/expr_evaluator.h"
#include "interpreter/stmt_executor.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    void executeProgram(const Program* program);

    // Public API for evaluators
    Value evaluateExpr(const Expr* expr, Environment* env);
    void executeStmt(const Stmt* stmt, Environment* env);
    void executeBlock(const Block* block, Environment* env);

    Value callFunctionByName(const std::string& name, const std::vector<Value>& args);

private:
    Environment* globals { nullptr };
    std::unordered_map<std::string, Function*> userFunctions;
    
    // Components
    std::unique_ptr<ExprEvaluator> evaluator;
    std::unique_ptr<StmtExecutor> executor;
    
    // Helpers
    Value callUserFunction(Function* fn, const std::vector<Value>& args);
    Value deepCopyIfNeeded(const Value& v);
};
