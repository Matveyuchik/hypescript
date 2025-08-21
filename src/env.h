#ifndef HYPESCRIPT_ENV_H
#define HYPESCRIPT_ENV_H

#include "value.h"

// Forward declaration to avoid circular include
typedef struct Stmt Stmt;

typedef struct VarEntry {
    char* name;
    Value value;
    struct VarEntry* next;
} VarEntry;

typedef struct Env {
    VarEntry* head;
    struct Env* parent;
} Env;

Env* env_create(Env* parent);
void env_free(Env* env);

bool env_set(Env* env, const char* name, Value value);
bool env_assign(Env* env, const char* name, Value value);
bool env_get(Env* env, const char* name, Value* out);

// Function registry
typedef struct FunctionDef {
    char* name;
    char** params;
    int param_count;
    Stmt* body;
    struct FunctionDef* next;
} FunctionDef;

typedef struct Functions {
    FunctionDef* head;
} Functions;

void funcs_init(Functions* f);
void funcs_free(Functions* f);
void funcs_register(Functions* f, const char* name, char** params, int param_count, Stmt* body);
FunctionDef* funcs_lookup(Functions* f, const char* name);

#endif


