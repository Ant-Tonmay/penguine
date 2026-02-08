#pragma once

#include "parser/ast.h"
#include "interpreter/runtime_value.h"
#include "interpreter/environment.h"

class Interpreter; // Forward declaration

class ExprEvaluator {
public:
    ExprEvaluator(Interpreter* interpreter);
    
    Value evaluate(const Expr* expr, Environment* env);

private:
    Interpreter* interpreter;

    Value visit(const NumberExpr* expr);
    Value visit(const StringExpr* expr);
    Value visit(const VarExpr* expr, Environment* env);
    Value visit(const ArrayExpr* expr, Environment* env);

    Value visit(const IndexExpr* expr, Environment* env);
    Value visit(const CallExpr* expr, Environment* env);
    Value visit(const BinaryExpr* expr, Environment* env);
    Value visit(const UnaryExpr* expr, Environment* env);
};
