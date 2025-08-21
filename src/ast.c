#include <stdlib.h>
#include <string.h>

#include "ast.h"

static char* str_dup(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    memcpy(out, s, len + 1);
    return out;
}

Expr* expr_literal(Value v) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_LITERAL;
    e->as.literal.value = v;
    return e;
}

Expr* expr_variable(const char* name) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_VARIABLE;
    e->as.variable.name = str_dup(name);
    return e;
}

Expr* expr_assign(const char* name, Expr* value) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_ASSIGN;
    e->as.assign.name = str_dup(name);
    e->as.assign.value = value;
    return e;
}

Expr* expr_binary(int op, Expr* left, Expr* right) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_BINARY;
    e->as.binary.op = op;
    e->as.binary.left = left;
    e->as.binary.right = right;
    return e;
}

Expr* expr_unary(int op, Expr* expr) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_UNARY;
    e->as.unary.op = op;
    e->as.unary.expr = expr;
    return e;
}

Expr* expr_call(const char* name, Expr** args, int count) {
    Expr* e = (Expr*)malloc(sizeof(Expr));
    e->type = EXPR_CALL;
    e->as.call.callee = str_dup(name);
    e->as.call.args = args;
    e->as.call.arg_count = count;
    return e;
}

Stmt* stmt_expr(Expr* expr) {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_EXPR;
    s->as.expr.expr = expr;
    return s;
}

Stmt* stmt_block(StmtList* stmts) {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_BLOCK;
    s->as.block.statements = stmts;
    return s;
}

Stmt* stmt_if(Expr* cond, Stmt* thenb, Stmt* elseb) {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_IF;
    s->as.ifstmt.condition = cond;
    s->as.ifstmt.then_branch = thenb;
    s->as.ifstmt.else_branch = elseb;
    return s;
}

Stmt* stmt_while(Expr* cond, Stmt* body) {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_WHILE;
    s->as.whilestmt.condition = cond;
    s->as.whilestmt.body = body;
    return s;
}

Stmt* stmt_for(Stmt* init, Expr* cond, Expr* inc, Stmt* body) {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_FOR;
    s->as.forstmt.init = init;
    s->as.forstmt.condition = cond;
    s->as.forstmt.increment = inc;
    s->as.forstmt.body = body;
    return s;
}

Stmt* stmt_break() {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_BREAK;
    return s;
}

Stmt* stmt_continue() {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_CONTINUE;
    return s;
}

StmtList* stmt_list_append(StmtList* list, Stmt* stmt) {
    StmtList* node = (StmtList*)malloc(sizeof(StmtList));
    node->stmt = stmt;
    node->next = NULL;
    if (!list) return node;
    StmtList* cur = list;
    while (cur->next) cur = cur->next;
    cur->next = node;
    return list;
}

Stmt* stmt_func(const char* name, char** params, int param_count, Stmt* body) {
    Stmt* s = (Stmt*)malloc(sizeof(Stmt));
    s->type = STMT_FUNC;
    s->as.func.name = str_dup(name);
    s->as.func.params = params;
    s->as.func.param_count = param_count;
    s->as.func.body = body;
    return s;
}

void expr_free(Expr* e) {
    if (!e) return;
    switch (e->type) {
        case EXPR_LITERAL:
            value_free(&e->as.literal.value);
            break;
        case EXPR_VARIABLE:
            free(e->as.variable.name);
            break;
        case EXPR_ASSIGN:
            free(e->as.assign.name);
            expr_free(e->as.assign.value);
            break;
        case EXPR_BINARY:
            expr_free(e->as.binary.left);
            expr_free(e->as.binary.right);
            break;
        case EXPR_UNARY:
            expr_free(e->as.unary.expr);
            break;
        case EXPR_CALL:
            free(e->as.call.callee);
            for (int i = 0; i < e->as.call.arg_count; i++) expr_free(e->as.call.args[i]);
            free(e->as.call.args);
            break;
    }
    free(e);
}

void stmt_list_free(StmtList* list) {
    while (list) {
        StmtList* next = list->next;
        stmt_free(list->stmt);
        free(list);
        list = next;
    }
}

void stmt_free(Stmt* s) {
    if (!s) return;
    switch (s->type) {
        case STMT_EXPR:
            expr_free(s->as.expr.expr);
            break;
        case STMT_BLOCK:
            stmt_list_free(s->as.block.statements);
            break;
        case STMT_IF:
            expr_free(s->as.ifstmt.condition);
            stmt_free(s->as.ifstmt.then_branch);
            stmt_free(s->as.ifstmt.else_branch);
            break;
        case STMT_WHILE:
            expr_free(s->as.whilestmt.condition);
            stmt_free(s->as.whilestmt.body);
            break;
        case STMT_FOR:
            stmt_free(s->as.forstmt.init);
            expr_free(s->as.forstmt.condition);
            expr_free(s->as.forstmt.increment);
            stmt_free(s->as.forstmt.body);
            break;
        case STMT_BREAK:
        case STMT_CONTINUE:
            break;
        case STMT_FUNC:
            free(s->as.func.name);
            for (int i = 0; i < s->as.func.param_count; i++) free(s->as.func.params[i]);
            free(s->as.func.params);
            stmt_free(s->as.func.body);
            break;
    }
    free(s);
}


