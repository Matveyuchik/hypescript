#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "interp.h"
#include "token.h"

static Value eval_expr(Interpreter* in, Env* env, Expr* e);
static void exec_stmt(Interpreter* in, Env* env, Stmt* s);

static char* hs_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

void interpreter_init(Interpreter* in) {
    in->globals = env_create(NULL);
    in->signaled_break = 0;
    in->signaled_continue = 0;
    funcs_init(&in->functions);
}

void interpreter_free(Interpreter* in) {
    env_free(in->globals);
    funcs_free(&in->functions);
}

static Value builtin_pechat(int argc, Value* argv) {
    for (int i = 0; i < argc; i++) {
        if (i) printf(" ");
        switch (argv[i].type) {
            case VAL_NULL: printf("null"); break;
            case VAL_BOOL: printf(argv[i].data.as_bool ? "true" : "false"); break;
            case VAL_NUMBER: printf("%g", argv[i].data.as_number); break;
            case VAL_STRING: printf("%s", argv[i].data.as_string ? argv[i].data.as_string : ""); break;
        }
    }
    printf("\n");
    return value_null();
}

static Value builtin_vhod(int argc, Value* argv) {
    (void)argv; // unused
    if (argc > 0) {
        // optional prompt: print first arg without newline
        if (argv[0].type == VAL_STRING && argv[0].data.as_string) {
            fputs(argv[0].data.as_string, stdout);
            fflush(stdout);
        }
    }
    // Portable line reader
    size_t capacity = 128;
    size_t length = 0;
    char* buffer = (char*)malloc(capacity);
    if (!buffer) return value_null();
    int ch;
    while ((ch = fgetc(stdin)) != EOF) {
        if (ch == '\n') break;
        if (length + 1 >= capacity) {
            capacity *= 2;
            char* newbuf = (char*)realloc(buffer, capacity);
            if (!newbuf) { free(buffer); return value_null(); }
            buffer = newbuf;
        }
        buffer[length++] = (char)ch;
    }
    if (length == 0 && ch == EOF) { free(buffer); return value_null(); }
    buffer[length] = '\0';
    Value v = value_string(buffer);
    free(buffer);
    return v;
}

static Value builtin_son(int argc, Value* argv) {
    // sleep in milliseconds if provided
    long ms = 0;
    if (argc >= 1) {
        if (argv[0].type == VAL_NUMBER) ms = (long)(argv[0].data.as_number);
        else if (argv[0].type == VAL_STRING && argv[0].data.as_string) ms = strtol(argv[0].data.as_string, NULL, 10);
    }
    if (ms > 0) {
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (long)(ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);
    }
    return value_null();
}

static Value to_number(const Value v) {
    switch (v.type) {
        case VAL_NUMBER: return value_number(v.data.as_number);
        case VAL_BOOL: return value_number(v.data.as_bool ? 1 : 0);
        case VAL_STRING: return value_number(v.data.as_string ? strtod(v.data.as_string, NULL) : 0);
        case VAL_NULL: return value_number(0);
    }
    return value_number(0);
}

static Value to_string(const Value v) {
    switch (v.type) {
        case VAL_STRING: return value_string(v.data.as_string ? v.data.as_string : "");
        case VAL_NUMBER: {
            char buf[64]; snprintf(buf, sizeof(buf), "%g", v.data.as_number); return value_string(buf);
        }
        case VAL_BOOL: return value_string(v.data.as_bool ? "istina" : "lozh");
        case VAL_NULL: return value_string("NICHTO");
    }
    return value_string("");
}

static Value to_bool(const Value v) {
    return value_bool(value_is_truthy(&v));
}

static int is_equal(Value a, Value b) {
    if (a.type != b.type) return 0;
    switch (a.type) {
        case VAL_NULL: return 1;
        case VAL_BOOL: return a.data.as_bool == b.data.as_bool;
        case VAL_NUMBER: return a.data.as_number == b.data.as_number;
        case VAL_STRING:
            if (a.data.as_string == NULL && b.data.as_string == NULL) return 1;
            if (!a.data.as_string || !b.data.as_string) return 0;
            return strcmp(a.data.as_string, b.data.as_string) == 0;
    }
    return 0;
}

static Value eval_binary(int op, Value l, Value r) {
    switch (op) {
        case TOK_PLUS:
            if (l.type == VAL_STRING || r.type == VAL_STRING) {
                // Convert both sides to strings and concatenate
                char tmp[64];
                char *ls, *rs;
                if (l.type == VAL_STRING) {
                    ls = l.data.as_string ? hs_strdup(l.data.as_string) : hs_strdup("");
                } else if (l.type == VAL_NUMBER) {
                    int n = snprintf(tmp, sizeof(tmp), "%g", l.data.as_number);
                    ls = (char*)malloc((size_t)n + 1); snprintf(ls, (size_t)n + 1, "%g", l.data.as_number);
                } else if (l.type == VAL_BOOL) {
                    ls = hs_strdup(l.data.as_bool ? "true" : "false");
                } else {
                    ls = hs_strdup("null");
                }
                if (r.type == VAL_STRING) {
                    rs = r.data.as_string ? hs_strdup(r.data.as_string) : hs_strdup("");
                } else if (r.type == VAL_NUMBER) {
                    int n = snprintf(tmp, sizeof(tmp), "%g", r.data.as_number);
                    rs = (char*)malloc((size_t)n + 1); snprintf(rs, (size_t)n + 1, "%g", r.data.as_number);
                } else if (r.type == VAL_BOOL) {
                    rs = hs_strdup(r.data.as_bool ? "true" : "false");
                } else {
                    rs = hs_strdup("null");
                }
                size_t llen = strlen(ls), rlen = strlen(rs);
                char* out = (char*)malloc(llen + rlen + 1);
                memcpy(out, ls, llen); memcpy(out + llen, rs, rlen); out[llen + rlen] = '\0';
                free(ls); free(rs);
                Value vv = value_string(out); free(out); return vv;
            } else {
                double v = (l.type == VAL_NUMBER ? l.data.as_number : 0) + (r.type == VAL_NUMBER ? r.data.as_number : 0);
                return value_number(v);
            }
        case TOK_MINUS: return value_number((l.type==VAL_NUMBER?l.data.as_number:0) - (r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_STAR: return value_number((l.type==VAL_NUMBER?l.data.as_number:0) * (r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_SLASH: return value_number((l.type==VAL_NUMBER?l.data.as_number:0) / (r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_PERCENT: return value_number((long)(l.type==VAL_NUMBER?l.data.as_number:0) % (long)(r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_GREATER: return value_bool((l.type==VAL_NUMBER?l.data.as_number:0) > (r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_GREATER_EQUAL: return value_bool((l.type==VAL_NUMBER?l.data.as_number:0) >= (r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_LESS: return value_bool((l.type==VAL_NUMBER?l.data.as_number:0) < (r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_LESS_EQUAL: return value_bool((l.type==VAL_NUMBER?l.data.as_number:0) <= (r.type==VAL_NUMBER?r.data.as_number:0));
        case TOK_EQUAL_EQUAL: return value_bool(is_equal(l, r));
        case TOK_BANG_EQUAL: return value_bool(!is_equal(l, r));
        case TOK_AND_AND: return value_bool(value_is_truthy(&l) && value_is_truthy(&r));
        case TOK_OR_OR: return value_bool(value_is_truthy(&l) || value_is_truthy(&r));
    }
    return value_null();
}

static Value eval_unary(int op, Value v) {
    switch (op) {
        case TOK_MINUS: return value_number(-(v.type == VAL_NUMBER ? v.data.as_number : 0));
        case TOK_BANG: return value_bool(!value_is_truthy(&v));
    }
    return value_null();
}

static Value eval_expr(Interpreter* in, Env* env, Expr* e) {
    switch (e->type) {
        case EXPR_LITERAL:
            return value_clone(&e->as.literal.value);
        case EXPR_VARIABLE: {
            Value out; if (!env_get(env, e->as.variable.name, &out)) return value_null(); return out; }
        case EXPR_ASSIGN: {
            Value v = eval_expr(in, env, e->as.assign.value);
            if (!env_assign(env, e->as.assign.name, v)) {
                env_set(env, e->as.assign.name, v);
            }
            return v;
        }
        case EXPR_BINARY: {
            Value l = eval_expr(in, env, e->as.binary.left);
            Value r = eval_expr(in, env, e->as.binary.right);
            return eval_binary(e->as.binary.op, l, r);
        }
        case EXPR_UNARY: {
            Value v = eval_expr(in, env, e->as.unary.expr);
            return eval_unary(e->as.unary.op, v);
        }
        case EXPR_CALL: {
            int argc = e->as.call.arg_count;
            Value* argv = (Value*)malloc(sizeof(Value) * argc);
            for (int i = 0; i < argc; i++) argv[i] = eval_expr(in, env, e->as.call.args[i]);
            Value result = value_null();
            if (strcmp(e->as.call.callee, "pechat") == 0) result = builtin_pechat(argc, argv);
            else if (strcmp(e->as.call.callee, "vhod") == 0) result = builtin_vhod(argc, argv);
            else if (strcmp(e->as.call.callee, "son") == 0) result = builtin_son(argc, argv);
            else if (strcmp(e->as.call.callee, "chislo") == 0) result = argc>0 ? to_number(argv[0]) : value_number(0);
            else if (strcmp(e->as.call.callee, "stroka") == 0) result = argc>0 ? to_string(argv[0]) : value_string("");
            else if (strcmp(e->as.call.callee, "logika") == 0) result = argc>0 ? to_bool(argv[0]) : value_bool(false);
            else if (strcmp(e->as.call.callee, "ukazatel") == 0) {
                if (argc>0 && argv[0].type==VAL_STRING && argv[0].data.as_string) {
                    // Pointer as a tagged string: "&name"
                    size_t len = strlen(argv[0].data.as_string);
                    char* p = (char*)malloc(len + 2);
                    p[0] = '&'; memcpy(p+1, argv[0].data.as_string, len+1);
                    result = value_string(p); free(p);
                }
            } else if (strcmp(e->as.call.callee, "znach") == 0) {
                if (argc>0 && argv[0].type==VAL_STRING && argv[0].data.as_string && argv[0].data.as_string[0]=='&') {
                    const char* name = argv[0].data.as_string + 1;
                    Value out; if (env_get(env, name, &out)) result = out; else result = value_null();
                }
            } else if (strcmp(e->as.call.callee, "prisvoit") == 0) {
                if (argc>1 && argv[0].type==VAL_STRING && argv[0].data.as_string && argv[0].data.as_string[0]=='&') {
                    const char* name = argv[0].data.as_string + 1;
                    if (!env_assign(env, name, argv[1])) env_set(env, name, argv[1]);
                    result = argv[1];
                }
            } else {
                FunctionDef* def = funcs_lookup(&in->functions, e->as.call.callee);
                if (def) {
                    Env* local = env_create(in->globals);
                    int n = argc < def->param_count ? argc : def->param_count;
                    for (int i = 0; i < n; i++) env_set(local, def->params[i], argv[i]);
                    exec_stmt(in, local, def->body);
                    env_free(local);
                    result = value_null();
                }
            }
            free(argv);
            return result;
        }
    }
    return value_null();
}

static void exec_stmt_list(Interpreter* in, Env* env, StmtList* list) {
    for (StmtList* it = list; it; it = it->next) {
        exec_stmt(in, env, it->stmt);
        if (in->signaled_break || in->signaled_continue) return;
    }
}

static void exec_stmt(Interpreter* in, Env* env, Stmt* s) {
    switch (s->type) {
        case STMT_EXPR: {
            Value v = eval_expr(in, env, s->as.expr.expr); (void)v; break; }
        case STMT_BLOCK: {
            Env* local = env_create(env);
            exec_stmt_list(in, local, s->as.block.statements);
            env_free(local);
            break;
        }
        case STMT_IF: {
            Value cond = eval_expr(in, env, s->as.ifstmt.condition);
            if (value_is_truthy(&cond)) exec_stmt(in, env, s->as.ifstmt.then_branch);
            else if (s->as.ifstmt.else_branch) exec_stmt(in, env, s->as.ifstmt.else_branch);
            break;
        }
        case STMT_WHILE: {
            while (1) {
                Value cond = eval_expr(in, env, s->as.whilestmt.condition);
                if (!value_is_truthy(&cond)) break;
                in->signaled_continue = 0;
                exec_stmt(in, env, s->as.whilestmt.body);
                if (in->signaled_break) { in->signaled_break = 0; break; }
            }
            break;
        }
        case STMT_FOR: {
            Env* local = env_create(env);
            if (s->as.forstmt.init) exec_stmt(in, local, s->as.forstmt.init);
            while (1) {
                if (s->as.forstmt.condition) {
                    Value c = eval_expr(in, local, s->as.forstmt.condition);
                    if (!value_is_truthy(&c)) break;
                }
                in->signaled_continue = 0;
                exec_stmt(in, local, s->as.forstmt.body);
                if (in->signaled_break) { in->signaled_break = 0; break; }
                if (s->as.forstmt.increment) { Value inc = eval_expr(in, local, s->as.forstmt.increment); (void)inc; }
            }
            env_free(local);
            break;
        }
        case STMT_FUNC: {
            funcs_register(&in->functions, s->as.func.name, s->as.func.params, s->as.func.param_count, s->as.func.body);
            s->as.func.params = NULL; s->as.func.param_count = 0; s->as.func.body = NULL;
            break;
        }
        case STMT_BREAK:
            in->signaled_break = 1; break;
        case STMT_CONTINUE:
            in->signaled_continue = 1; break;
    }
}

void interpret(Interpreter* in, StmtList* program) {
    exec_stmt_list(in, in->globals, program);
}


