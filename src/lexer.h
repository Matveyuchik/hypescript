#ifndef HYPESCRIPT_LEXER_H
#define HYPESCRIPT_LEXER_H

#include "token.h"

typedef struct {
    const char* source;
    const char* current;
    int line;
    int column;
} Lexer;

void lexer_init(Lexer* lexer, const char* source);
Token lexer_next(Lexer* lexer);
void token_free(Token* token);

#endif


