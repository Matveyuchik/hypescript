#include <stdlib.h>
#include <string.h>

#include "env.h"

static char* str_dup(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    memcpy(out, s, len + 1);
    return out;
}

Env* env_create(Env* parent) {
    Env* e = (Env*)malloc(sizeof(Env));
    e->head = NULL;
    e->parent = parent;
    return e;
}

void env_free(Env* env) {
    if (!env) return;
    VarEntry* cur = env->head;
    while (cur) {
        VarEntry* next = cur->next;
        free(cur->name);
        value_free(&cur->value);
        free(cur);
        cur = next;
    }
    free(env);
}

bool env_set(Env* env, const char* name, Value value) {
    VarEntry* e = (VarEntry*)malloc(sizeof(VarEntry));
    e->name = str_dup(name);
    e->value = value;
    e->next = env->head;
    env->head = e;
    return true;
}

bool env_assign(Env* env, const char* name, Value value) {
    for (Env* e = env; e; e = e->parent) {
        for (VarEntry* v = e->head; v; v = v->next) {
            if (strcmp(v->name, name) == 0) {
                value_free(&v->value);
                v->value = value;
                return true;
            }
        }
    }
    return false;
}

bool env_get(Env* env, const char* name, Value* out) {
    for (Env* e = env; e; e = e->parent) {
        for (VarEntry* v = e->head; v; v = v->next) {
            if (strcmp(v->name, name) == 0) {
                if (out) *out = v->value;
                return true;
            }
        }
    }
    return false;
}

void funcs_init(Functions* f) { f->head = NULL; }

void funcs_free(Functions* f) {
    FunctionDef* cur = f->head;
    while (cur) {
        FunctionDef* next = cur->next;
        free(cur->name);
        for (int i = 0; i < cur->param_count; i++) free(cur->params[i]);
        free(cur->params);
        // body is owned by AST program; do not free here to avoid double free
        cur->body = NULL;
        free(cur);
        cur = next;
    }
}

void funcs_register(Functions* f, const char* name, char** params, int param_count, Stmt* body) {
    FunctionDef* def = (FunctionDef*)malloc(sizeof(FunctionDef));
    def->name = str_dup(name);
    def->params = params;
    def->param_count = param_count;
    def->body = body;
    def->next = f->head;
    f->head = def;
}

FunctionDef* funcs_lookup(Functions* f, const char* name) {
    for (FunctionDef* d = f->head; d; d = d->next) if (strcmp(d->name, name) == 0) return d;
    return NULL;
}


