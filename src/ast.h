#ifndef HYPESCRIPT_AST_H
#define HYPESCRIPT_AST_H

#include <stdbool.h>
#include "value.h"

typedef enum {
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_ASSIGN,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL,
} ExprType;

typedef struct Expr Expr;
typedef struct Stmt Stmt;

typedef struct {
    Value value; // literal
} ExprLiteral;

typedef struct {
    char* name;
} ExprVariable;

typedef struct {
    char* name;
    Expr* value;
} ExprAssign;

typedef struct {
    int op; // TokenType but avoid header dependency loops
    Expr* left;
    Expr* right;
} ExprBinary;

typedef struct {
    int op; // TokenType
    Expr* expr;
} ExprUnary;

typedef struct {
    char* callee; // only built-ins like pechat, vhod
    Expr** args;
    int arg_count;
} ExprCall;

struct Expr {
    ExprType type;
    union {
        ExprLiteral literal;
        ExprVariable variable;
        ExprAssign assign;
        ExprBinary binary;
        ExprUnary unary;
        ExprCall call;
    } as;
};

typedef enum {
    STMT_EXPR,
    STMT_BLOCK,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_FUNC
} StmtType;

typedef struct StmtList {
    Stmt* stmt;
    struct StmtList* next;
} StmtList;

typedef struct {
    Expr* expr;
} StmtExpr;

typedef struct {
    StmtList* statements;
} StmtBlock;

typedef struct {
    Expr* condition;
    Stmt* then_branch;
    Stmt* else_branch; // nullable
} StmtIf;

typedef struct {
    Stmt* init;     // nullable
    Expr* condition;// nullable
    Expr* increment;// nullable
    Stmt* body;
} StmtFor;

typedef struct {
    Expr* condition;
    Stmt* body;
} StmtWhile;

typedef struct {
    char* name;
    char** params;
    int param_count;
    Stmt* body;
} StmtFunc;

struct Stmt {
    StmtType type;
    union {
        StmtExpr expr;
        StmtBlock block;
        StmtIf ifstmt;
        StmtWhile whilestmt;
        StmtFor forstmt;
        StmtFunc func;
    } as;
};

// constructors and helpers
Expr* expr_literal(Value v);
Expr* expr_variable(const char* name);
Expr* expr_assign(const char* name, Expr* value);
Expr* expr_binary(int op, Expr* left, Expr* right);
Expr* expr_unary(int op, Expr* expr);
Expr* expr_call(const char* name, Expr** args, int count);

Stmt* stmt_expr(Expr* expr);
Stmt* stmt_block(StmtList* stmts);
Stmt* stmt_if(Expr* cond, Stmt* thenb, Stmt* elseb);
Stmt* stmt_while(Expr* cond, Stmt* body);
Stmt* stmt_for(Stmt* init, Expr* cond, Expr* inc, Stmt* body);
Stmt* stmt_break();
Stmt* stmt_continue();
Stmt* stmt_func(const char* name, char** params, int param_count, Stmt* body);

StmtList* stmt_list_append(StmtList* list, Stmt* stmt);

void expr_free(Expr* e);
void stmt_free(Stmt* s);
void stmt_list_free(StmtList* list);

#endif


