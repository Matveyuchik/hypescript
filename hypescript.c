// HypeScript interpreter main
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/parser.h"
#include "src/interp.h"

#define VERSION "0.1.0"

static char* read_all(FILE* f) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)size + 1);
    if (!buf) return NULL;
    size_t n = fread(buf, 1, (size_t)size, f);
    buf[n] = '\0';
    return buf;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: hypescript [filename]\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Target file doesn't exists!\n");
        return 1;
    }

    char* src = read_all(file);
    fclose(file);
    if (!src) { fprintf(stderr, "Failed to read file\n"); return 1; }

    Parser p; parser_init(&p, src);
    StmtList* program = parse_program(&p);
    if (p.had_error) { free(src); stmt_list_free(program); return 1; }

    Interpreter in; interpreter_init(&in);
    interpret(&in, program);

    // cleanup
    interpreter_free(&in);
    stmt_list_free(program);
    free(src);
    return 0;
}