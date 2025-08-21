#ifndef HYPESCRIPT_INTERP_H
#define HYPESCRIPT_INTERP_H

#include "ast.h"
#include "env.h"

typedef struct {
    Env* globals;
    int signaled_break;
    int signaled_continue;
    Functions functions;
} Interpreter;

void interpreter_init(Interpreter* in);
void interpreter_free(Interpreter* in);

void interpret(Interpreter* in, StmtList* program);

#endif


