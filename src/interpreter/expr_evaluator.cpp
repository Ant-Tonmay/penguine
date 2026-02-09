#include "interpreter/expr_evaluator.h"
#include "interpreter/interpreter.h"
#include <iostream>

ExprEvaluator::ExprEvaluator(Interpreter* interpreter) : interpreter(interpreter) {}

Value ExprEvaluator::evaluate(const Expr* expr, Environment* env) {
    if (auto num = dynamic_cast<const NumberExpr*>(expr)) {
        return visit(num);
    } else if (auto str = dynamic_cast<const StringExpr*>(expr)) {
        return visit(str);
    } else if (auto var = dynamic_cast<const VarExpr*>(expr)) {
        return visit(var, env);
    } else if (auto arr = dynamic_cast<const ArrayExpr*>(expr)) {
        return visit(arr, env);
    } else if (auto idx = dynamic_cast<const IndexExpr*>(expr)) {
        return visit(idx, env);
    } else if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        return visit(call, env);
    } else if (auto bin = dynamic_cast<const BinaryExpr*>(expr)) {
        return visit(bin, env);
    } else if (auto un = dynamic_cast<const UnaryExpr*>(expr)) {
        return visit(un, env);
    } else if (auto boolean = dynamic_cast<const BoolExpr*>(expr)) {
        return visit(boolean);
    }
    return std::monostate{};
}

Value ExprEvaluator::visit(const BoolExpr* expr) {
    return expr->value;
}

Value ExprEvaluator::visit(const NumberExpr* expr) {
    if (expr->value.find('.') != std::string::npos) {
        return std::stod(expr->value);
    }
    return std::stoi(expr->value);
}

Value ExprEvaluator::visit(const StringExpr* expr) {
    return expr->value;
}

Value ExprEvaluator::visit(const VarExpr* expr, Environment* env) {
    return env->get(expr->name);
}

Value ExprEvaluator::visit(const ArrayExpr* expr, Environment* env) {
    std::vector<Value> elements;
    for (const auto& el : expr->elements) {
        elements.push_back(evaluate(el.get(), env));
    }

    
    size_t n = elements.size();
    
    // Auto-unwrap single-element arrays if the element is an array
    if (n == 1 && std::holds_alternative<ArrayObject*>(elements[0])) {
        return elements[0];
    }
    
    ArrayObject* obj = new ArrayObject;
    obj->length = n;
    obj->capacity = n;
    obj->data = new Value[n];
    for(size_t i=0; i<n; ++i) obj->data[i] = elements[i];
    
    return obj;
}

Value ExprEvaluator::visit(const IndexExpr* expr, Environment* env) {
    Value base = evaluate(expr->array.get(), env);
    Value idx = evaluate(expr->index.get(), env);
    
    if (!std::holds_alternative<ArrayObject*>(base)) {
         throw std::runtime_error("Index operation expects an array.");
    }
    if (!std::holds_alternative<int>(idx)) {
         throw std::runtime_error("Index must be an integer.");
    }
    
    ArrayObject* arr = std::get<ArrayObject*>(base);
    int i = std::get<int>(idx);
    
    if (i < 0 || (size_t)i >= arr->length) {
        throw std::runtime_error("Index out of bounds.");
    }
    
    return arr->data[i];
}

Value ExprEvaluator::visit(const CallExpr* expr, Environment* env) {
    
    std::string name;
    std::vector<Value> args;

    if (auto member = dynamic_cast<const MemberExpr*>(expr->callee.get())) {
        name = member->name;
      
        args.push_back(evaluate(member->object.get(), env));
    } else if (auto var = dynamic_cast<const VarExpr*>(expr->callee.get())) {
        name = var->name;
    } else {
       
        return std::monostate{};
    }

   
    for(auto& arg : expr->arguments) {
        args.push_back(evaluate(arg.get(), env));
    }
    
    return interpreter->callFunctionByName(name, args);
}

Value ExprEvaluator::visit(const BinaryExpr* expr, Environment* env) {
    Value left = evaluate(expr->left.get(), env);
    Value right = evaluate(expr->right.get(), env);
    
    if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
        int l = std::get<int>(left);
        int r = std::get<int>(right);
        if (expr->op == "+") return l + r;
        if (expr->op == "-") return l - r;
        if (expr->op == "*") return l * r;
        if (expr->op == "/") return r != 0 ? l / r : 0; // split by zero check?
        if (expr->op == "<") return l < r;
        if (expr->op == ">") return l > r;
        if (expr->op == "<=") return l <= r;
        if (expr->op == ">=") return l >= r;
        if (expr->op == "==") return l == r;
        if (expr->op == "!=") return l != r;
        if (expr->op == "%") return l % r;
        if (expr->op == "|") return l | r;
        if (expr->op == "&") return l & r;
        if (expr->op == "^") return l ^ r;
        if (expr->op == "<<") return l << r;
        if (expr->op == ">>") return l >> r;
        if (expr->op == "&&") return (l != 0) && (r != 0);
        if (expr->op == "||") return (l != 0) || (r != 0);
    }
    
    if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
    {
        if (expr->op == "+")
            return std::get<std::string>(left) + std::get<std::string>(right);
        if (expr->op == "==")
            return std::get<std::string>(left) == std::get<std::string>(right);
    }

    if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
        bool l = std::get<bool>(left);
        bool r = std::get<bool>(right);
        if (expr->op == "&&") return l && r;
        if (expr->op == "||") return l || r;
        if (expr->op == "==") return l == r;
        if (expr->op == "!=") return l != r;
    }
    
    return std::monostate{};
}

Value ExprEvaluator::visit(const UnaryExpr* expr, Environment* env) {
    Value val = evaluate(expr->right.get(), env);
    if (expr->op == "!") {
        if (std::holds_alternative<bool>(val)) return !std::get<bool>(val);
    }
    return std::monostate{};
}
