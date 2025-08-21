#ifndef HYPESCRIPT_VALUE_H
#define HYPESCRIPT_VALUE_H

#include <stdbool.h>

typedef enum {
    VAL_NULL = 0,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool as_bool;
        double as_number;
        char* as_string;
    } data;
} Value;

Value value_null();
Value value_bool(bool b);
Value value_number(double n);
Value value_string(const char* s);

void value_free(Value* v);
bool value_is_truthy(const Value* v);
Value value_clone(const Value* v);

#endif


