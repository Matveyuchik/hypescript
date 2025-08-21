#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"

static void advance(Parser* p) {
    token_free(&p->previous);
    p->previous = p->current;
    p->current = lexer_next(&p->lexer);
}

static int check(Parser* p, TokenType t) { return p->current.type == t; }

static int match(Parser* p, TokenType t) {
    if (check(p, t)) { advance(p); return 1; }
    return 0;
}

static void consume(Parser* p, TokenType t, const char* msg) {
    if (!match(p, t)) {
        fprintf(stderr, "Parse error at line %d col %d: %s\n", p->current.line, p->current.column, msg);
        p->had_error = 1;
    }
}

static Expr* parse_expression(Parser* p);
static Stmt* parse_statement(Parser* p);
static Stmt* parse_declaration(Parser* p);

void parser_init(Parser* p, const char* source) {
    lexer_init(&p->lexer, source);
    p->current.type = TOK_ERROR;
    p->current.lexeme = NULL;
    p->previous.type = TOK_ERROR;
    p->previous.lexeme = NULL;
    p->had_error = 0;
    advance(p);
}

// Pratt parser precedence
static Expr* parse_primary(Parser* p) {
    if (match(p, TOK_NUMBER)) return expr_literal(value_number(p->previous.number));
    if (match(p, TOK_STRING)) return expr_literal(value_string(p->previous.lexeme));
    if (match(p, TOK_IDENTIFIER)) return expr_variable(p->previous.lexeme);
    if (match(p, TOK_KW_PECHAT)) return expr_variable("pechat");
    if (match(p, TOK_KW_VHOD)) return expr_variable("vhod");
    if (match(p, TOK_KW_SON)) return expr_variable("son");
    if (match(p, TOK_KW_CHISLO)) return expr_variable("chislo");
    if (match(p, TOK_KW_STROKA)) return expr_variable("stroka");
    if (match(p, TOK_KW_LOGIKA)) return expr_variable("logika");
    if (match(p, TOK_KW_ISTINA)) return expr_literal(value_bool(true));
    if (match(p, TOK_KW_LOZH)) return expr_literal(value_bool(false));
    if (match(p, TOK_KW_NICHTO)) return expr_literal(value_null());
    fprintf(stderr, "Unexpected token at %d:%d\n", p->current.line, p->current.column);
    p->had_error = 1;
    return expr_literal(value_null());
}

static Expr* parse_call(Parser* p) {
    Expr* expr = parse_primary(p);
    // Only support built-in calls with syntax: identifier '(' args? ')'
    while (check(p, TOK_LPAREN)) {
        if (expr->type != EXPR_VARIABLE) break;
        advance(p); // consume '('
        // Collect args
        Expr** args = NULL; int count = 0; int capacity = 0;
        if (!check(p, TOK_RPAREN)) {
            do {
                if (count == capacity) {
                    capacity = capacity < 4 ? 4 : capacity * 2;
                    args = (Expr**)realloc(args, sizeof(Expr*) * capacity);
                }
                args[count++] = parse_expression(p);
            } while (match(p, TOK_COMMA));
        }
        consume(p, TOK_RPAREN, ") expected after arguments");
        Expr* call = expr_call(expr->as.variable.name, args, count);
        // free temp variable node but keep the name used in call (duplicate there)
        free(expr->as.variable.name);
        free(expr);
        expr = call;
    }
    return expr;
}

static Expr* parse_unary(Parser* p) {
    if (match(p, TOK_BANG) || match(p, TOK_MINUS)) {
        int op = p->previous.type;
        Expr* right = parse_unary(p);
        return expr_unary(op, right);
    }
    return parse_call(p);
}

static Expr* parse_factor(Parser* p) {
    Expr* expr = parse_unary(p);
    while (match(p, TOK_STAR) || match(p, TOK_SLASH) || match(p, TOK_PERCENT)) {
        int op = p->previous.type;
        Expr* right = parse_unary(p);
        expr = expr_binary(op, expr, right);
    }
    return expr;
}

static Expr* parse_term(Parser* p) {
    Expr* expr = parse_factor(p);
    while (match(p, TOK_PLUS) || match(p, TOK_MINUS)) {
        int op = p->previous.type;
        Expr* right = parse_factor(p);
        expr = expr_binary(op, expr, right);
    }
    return expr;
}

static Expr* parse_comparison(Parser* p) {
    Expr* expr = parse_term(p);
    while (match(p, TOK_GREATER) || match(p, TOK_GREATER_EQUAL) || match(p, TOK_LESS) || match(p, TOK_LESS_EQUAL)) {
        int op = p->previous.type;
        Expr* right = parse_term(p);
        expr = expr_binary(op, expr, right);
    }
    return expr;
}

static Expr* parse_equality(Parser* p) {
    Expr* expr = parse_comparison(p);
    while (match(p, TOK_EQUAL_EQUAL) || match(p, TOK_BANG_EQUAL)) {
        int op = p->previous.type;
        Expr* right = parse_comparison(p);
        expr = expr_binary(op, expr, right);
    }
    return expr;
}

static Expr* parse_logic(Parser* p) {
    Expr* expr = parse_equality(p);
    while (match(p, TOK_AND_AND) || match(p, TOK_OR_OR)) {
        int op = p->previous.type;
        Expr* right = parse_equality(p);
        expr = expr_binary(op, expr, right);
    }
    return expr;
}

static Expr* parse_assignment(Parser* p) {
    Expr* expr = parse_logic(p);
    if (match(p, TOK_EQUAL)) {
        if (expr->type != EXPR_VARIABLE) {
            fprintf(stderr, "Invalid assignment target at %d:%d\n", p->previous.line, p->previous.column);
            p->had_error = 1; return expr;
        }
        char* name = expr->as.variable.name;
        Expr* value = parse_assignment(p);
        free(expr);
        return expr_assign(name, value);
    }
    return expr;
}

static Expr* parse_expression(Parser* p) { return parse_assignment(p); }

static Stmt* parse_block(Parser* p) {
    StmtList* list = NULL;
    while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF)) {
        Stmt* s = parse_declaration(p);
        list = stmt_list_append(list, s);
    }
    consume(p, TOK_RBRACE, "} expected after block");
    return stmt_block(list);
}

static Stmt* parse_statement(Parser* p) {
    if (match(p, TOK_LBRACE)) return parse_block(p);
    if (match(p, TOK_KW_PRIKOL)) {
        // prikol name(params) { ... }
        if (!match(p, TOK_IDENTIFIER)) {
            fprintf(stderr, "Function name expected after 'prikol' at %d:%d\n", p->current.line, p->current.column);
            p->had_error = 1; return stmt_expr(expr_literal(value_null()));
        }
        char* fname = (char*)malloc(strlen(p->previous.lexeme)+1); strcpy(fname, p->previous.lexeme);
        consume(p, TOK_LPAREN, "( expected after function name");
        char** params = NULL; int count = 0; int cap = 0;
        if (!check(p, TOK_RPAREN)) {
            do {
                if (!match(p, TOK_IDENTIFIER)) { fprintf(stderr, "Parameter name expected at %d:%d\n", p->current.line, p->current.column); p->had_error = 1; break; }
                if (count == cap) { cap = cap < 4 ? 4 : cap * 2; params = (char**)realloc(params, sizeof(char*) * cap); }
                params[count] = (char*)malloc(strlen(p->previous.lexeme)+1); strcpy(params[count], p->previous.lexeme); count++;
            } while (match(p, TOK_COMMA));
        }
        consume(p, TOK_RPAREN, ") expected after parameters");
        consume(p, TOK_LBRACE, "{ expected to start function body");
        // Parse block body assuming LBRACE already consumed
        // Reuse parse_block by expecting it consumes until RBRACE
        // We need to construct a block body statement
        // Create body as a block
        // parse_block expects LBRACE already matched; we matched it, so call parse_block
        Stmt* body = parse_block(p);
        Stmt* fn = stmt_func(fname, params, count, body);
        free(fname);
        return fn;
    }
    if (match(p, TOK_KW_POKA)) {
        consume(p, TOK_LPAREN, "( expected after 'poka'");
        Expr* cond = parse_expression(p);
        consume(p, TOK_RPAREN, ") expected after while condition");
        Stmt* body = parse_statement(p);
        Stmt* w = (Stmt*)malloc(sizeof(Stmt));
        w->type = STMT_WHILE; w->as.whilestmt.condition = cond; w->as.whilestmt.body = body;
        return w;
    }
    if (match(p, TOK_KW_SLOMAT)) { consume(p, TOK_SEMICOLON, "; expected after 'slomat'" ); return stmt_break(); }
    if (match(p, TOK_KW_PRODOLZHIT)) { consume(p, TOK_SEMICOLON, "; expected after 'prodolzhit'" ); return stmt_continue(); }
    if (match(p, TOK_KW_ESLI)) {
        consume(p, TOK_LPAREN, "( expected after 'esli'");
        Expr* cond = parse_expression(p);
        consume(p, TOK_RPAREN, ") expected after condition");
        Stmt* thenb = parse_statement(p);
        Stmt* elseb = NULL;
        if (match(p, TOK_KW_INACHE)) elseb = parse_statement(p);
        return stmt_if(cond, thenb, elseb);
    }
    if (match(p, TOK_KW_DLYA)) {
        consume(p, TOK_LPAREN, "( expected after 'dlya'");
        Stmt* init = NULL;
        if (!check(p, TOK_SEMICOLON)) {
            Expr* initExpr = parse_expression(p); 
            init = stmt_expr(initExpr);
        }
        consume(p, TOK_SEMICOLON, "; expected after for init");
        Expr* cond = NULL;
        if (!check(p, TOK_SEMICOLON)) cond = parse_expression(p);
        consume(p, TOK_SEMICOLON, "; expected after for condition");
        Expr* inc = NULL;
        if (!check(p, TOK_RPAREN)) inc = parse_expression(p);
        consume(p, TOK_RPAREN, ") expected after for clauses");
        Stmt* body = parse_statement(p);
        return stmt_for(init, cond, inc, body);
    }
    if (match(p, TOK_SEMICOLON)) {
        return stmt_expr(expr_literal(value_null()));
    }
    Expr* e = parse_expression(p);
    consume(p, TOK_SEMICOLON, "; expected after expression");
    return stmt_expr(e);
}

static Stmt* parse_declaration(Parser* p) {
    return parse_statement(p);
}

StmtList* parse_program(Parser* p) {
    // Optional leading !HYPE!
    match(p, TOK_KW_HYPE);
    StmtList* list = NULL;
    while (!check(p, TOK_EOF)) {
        Stmt* s = parse_declaration(p);
        list = stmt_list_append(list, s);
    }
    return list;
}


