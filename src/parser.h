#ifndef HYPESCRIPT_PARSER_H
#define HYPESCRIPT_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    int had_error;
} Parser;

void parser_init(Parser* p, const char* source);
StmtList* parse_program(Parser* p);

#endif


