#ifndef HYPESCRIPT_TOKEN_H
#define HYPESCRIPT_TOKEN_H

#include <stdbool.h>

typedef enum {
    TOK_EOF = 0,
    TOK_ERROR,

    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,

    // Keywords
    TOK_KW_HYPE,
    TOK_KW_PECHAT,
    TOK_KW_ESLI,
    TOK_KW_INACHE,
    TOK_KW_DLYA,
    TOK_KW_POKA,
    TOK_KW_SLOMAT,
    TOK_KW_PRODOLZHIT,
    TOK_KW_VHOD,
    TOK_KW_ISTINA,   // true
    TOK_KW_LOZH,     // false
    TOK_KW_NICHTO,   // null
    TOK_KW_SON,      // sleep
    TOK_KW_CHISLO,   // to number
    TOK_KW_STROKA,   // to string
    TOK_KW_LOGIKA,   // to boolean
    TOK_KW_PRIKOL,   // function definition

    // Operators and punctuation
    TOK_LPAREN,     // (
    TOK_RPAREN,     // )
    TOK_LBRACE,     // {
    TOK_RBRACE,     // }
    TOK_COMMA,      // ,
    TOK_DOT,        // .
    TOK_SEMICOLON,  // ;
    TOK_PLUS,       // +
    TOK_MINUS,      // -
    TOK_STAR,       // *
    TOK_SLASH,      // /
    TOK_PERCENT,    // %
    TOK_BANG,       // !
    TOK_BANG_EQUAL, // !=
    TOK_EQUAL,      // =
    TOK_EQUAL_EQUAL,// ==
    TOK_GREATER,    // >
    TOK_GREATER_EQUAL, // >=
    TOK_LESS,       // <
    TOK_LESS_EQUAL, // <=
    TOK_AND_AND,    // &&
    TOK_OR_OR       // ||
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;    // Owned string for identifiers/strings; NULL otherwise
    double number;   // For number literals
    int line;
    int column;
} Token;

static inline bool token_is_eof(TokenType t) { return t == TOK_EOF; }

#endif


