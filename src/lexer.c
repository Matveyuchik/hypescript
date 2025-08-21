#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "lexer.h"

static Token make_token(Lexer* l, TokenType type, const char* start, size_t length) {
    Token t;
    t.type = type;
    t.lexeme = NULL;
    t.number = 0.0;
    t.line = l->line;
    t.column = l->column;
    if (type == TOK_IDENTIFIER || type == TOK_STRING) {
        t.lexeme = (char*)malloc(length + 1);
        memcpy(t.lexeme, start, length);
        t.lexeme[length] = '\0';
    }
    return t;
}

static void skip_whitespace_and_comments(Lexer* l) {
    for (;;) {
        char c = *l->current;
        switch (c) {
            case ' ': case '\r': case '\t':
                l->current++; l->column++;
                break;
            case '\n':
                l->current++; l->line++; l->column = 1;
                break;
            case '/':
                if (l->current[1] == '/') {
                    while (*l->current && *l->current != '\n') l->current++;
                } else if (l->current[1] == '*') {
                    l->current += 2;
                    while (*l->current && !(l->current[0]=='*' && l->current[1]=='/')) {
                        if (*l->current == '\n') { l->line++; l->column = 1; }
                        else { l->column++; }
                        l->current++;
                    }
                    if (*l->current) { l->current += 2; }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static TokenType identifier_type(const char* start, size_t length) {
    // Keywords in Russian transliteration + special !HYPE!
    if (length == 6 && strncmp(start, "pechat", 6) == 0) return TOK_KW_PECHAT;
    if (length == 4 && strncmp(start, "esli", 4) == 0) return TOK_KW_ESLI;
    if (length == 6 && strncmp(start, "inache", 6) == 0) return TOK_KW_INACHE;
    if (length == 4 && strncmp(start, "dlya", 4) == 0) return TOK_KW_DLYA;
    if (length == 4 && strncmp(start, "poka", 4) == 0) return TOK_KW_POKA;
    if (length == 6 && strncmp(start, "slomat", 6) == 0) return TOK_KW_SLOMAT;
    if (length == 10 && strncmp(start, "prodolzhit", 10) == 0) return TOK_KW_PRODOLZHIT;
    if (length == 5 && strncmp(start, "vhod", 5) == 0) return TOK_KW_VHOD;
    if (length == 6 && strncmp(start, "istina", 6) == 0) return TOK_KW_ISTINA;
    if (length == 4 && strncmp(start, "lozh", 4) == 0) return TOK_KW_LOZH;
    if (length == 6 && strncmp(start, "NICHTO", 6) == 0) return TOK_KW_NICHTO;
    if (length == 3 && strncmp(start, "son", 3) == 0) return TOK_KW_SON;
    if (length == 6 && strncmp(start, "chislo", 6) == 0) return TOK_KW_CHISLO;
    if (length == 6 && strncmp(start, "stroka", 6) == 0) return TOK_KW_STROKA;
    if (length == 6 && strncmp(start, "logika", 6) == 0) return TOK_KW_LOGIKA;
    if (length == 6 && strncmp(start, "prikol", 6) == 0) return TOK_KW_PRIKOL;
    return TOK_IDENTIFIER;
}

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
}

static Token string(Lexer* l) {
    // Opening quote already consumed by caller
    const char* start = l->current;
    int startCol = l->column;
    while (*l->current && *l->current != '"') {
        if (*l->current == '\\' && l->current[1]) l->current++; // skip escape next
        if (*l->current == '\n') { l->line++; l->column = 1; }
        else { l->column++; }
        l->current++;
    }
    size_t length = (size_t)(l->current - start);
    if (*l->current == '"') { l->current++; l->column++; }
    Token t = make_token(l, TOK_STRING, start, length);
    t.column = startCol;
    return t;
}

static Token number(Lexer* l) {
    const char* start = l->current;
    int col = l->column;
    while (isdigit(*l->current)) { l->current++; l->column++; }
    if (*l->current == '.') {
        l->current++; l->column++;
        while (isdigit(*l->current)) { l->current++; l->column++; }
    }
    Token t = make_token(l, TOK_NUMBER, start, (size_t)(l->current - start));
    t.number = strtod(start, NULL);
    t.column = col;
    return t;
}

static Token identifier_or_hype(Lexer* l) {
    const char* start = l->current;
    int col = l->column;
    while (isalnum(*l->current) || *l->current == '_') { l->current++; l->column++; }
    size_t length = (size_t)(l->current - start);
    TokenType type = identifier_type(start, length);
    Token t = make_token(l, type, start, length);
    t.column = col;
    return t;
}

static Token symbol(Lexer* l) {
    char c = *l->current++;
    l->column++;
    switch (c) {
        case '(': return make_token(l, TOK_LPAREN, &c, 1);
        case ')': return make_token(l, TOK_RPAREN, &c, 1);
        case '{': return make_token(l, TOK_LBRACE, &c, 1);
        case '}': return make_token(l, TOK_RBRACE, &c, 1);
        case ',': return make_token(l, TOK_COMMA, &c, 1);
        case '.': return make_token(l, TOK_DOT, &c, 1);
        case ';': return make_token(l, TOK_SEMICOLON, &c, 1);
        case '+': return make_token(l, TOK_PLUS, &c, 1);
        case '-': return make_token(l, TOK_MINUS, &c, 1);
        case '*': return make_token(l, TOK_STAR, &c, 1);
        case '%': return make_token(l, TOK_PERCENT, &c, 1);
        case '!':
            if (*l->current == '=') { l->current++; l->column++; return make_token(l, TOK_BANG_EQUAL, "!=", 2);} 
            return make_token(l, TOK_BANG, &c, 1);
        case '=':
            if (*l->current == '=') { l->current++; l->column++; return make_token(l, TOK_EQUAL_EQUAL, "==", 2);} 
            return make_token(l, TOK_EQUAL, &c, 1);
        case '>':
            if (*l->current == '=') { l->current++; l->column++; return make_token(l, TOK_GREATER_EQUAL, ">=", 2);} 
            return make_token(l, TOK_GREATER, &c, 1);
        case '<':
            if (*l->current == '=') { l->current++; l->column++; return make_token(l, TOK_LESS_EQUAL, "<=", 2);} 
            return make_token(l, TOK_LESS, &c, 1);
        case '&':
            if (*l->current == '&') { l->current++; l->column++; return make_token(l, TOK_AND_AND, "&&", 2);} 
            break;
        case '|':
            if (*l->current == '|') { l->current++; l->column++; return make_token(l, TOK_OR_OR, "||", 2);} 
            break;
        case '"':
            // current points to the char after '"' right now, but our string()
            // expects to start at the first character inside quotes.
            // We already advanced past '"' above, so just parse.
            return string(l);
        case '/': return make_token(l, TOK_SLASH, &c, 1);
    }
    Token t = make_token(l, TOK_ERROR, &c, 1);
    return t;
}

Token lexer_next(Lexer* l) {
    skip_whitespace_and_comments(l);
    if (!*l->current) {
        Token t; t.type = TOK_EOF; t.lexeme = NULL; t.number = 0; t.line = l->line; t.column = l->column; return t;
    }

    // Special !HYPE! marker
    if (l->current[0] == '!' && strncmp(l->current, "!HYPE!", 6) == 0) {
        Token t = make_token(l, TOK_KW_HYPE, l->current, 6);
        l->current += 6; l->column += 6;
        return t;
    }

    char c = *l->current;
    if (isalpha(c) || c == '_') return identifier_or_hype(l);
    if (isdigit(c)) return number(l);
    return symbol(l);
}

void token_free(Token* token) {
    if (!token) return;
    if (token->lexeme) { free(token->lexeme); token->lexeme = NULL; }
}


