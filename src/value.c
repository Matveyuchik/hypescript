#include <stdlib.h>
#include <string.h>

#include "value.h"

Value value_null() {
    Value v; v.type = VAL_NULL; return v;
}

Value value_bool(bool b) {
    Value v; v.type = VAL_BOOL; v.data.as_bool = b; return v;
}

Value value_number(double n) {
    Value v; v.type = VAL_NUMBER; v.data.as_number = n; return v;
}

Value value_string(const char* s) {
    Value v; v.type = VAL_STRING; 
    if (s) {
        size_t len = strlen(s);
        v.data.as_string = (char*)malloc(len + 1);
        memcpy(v.data.as_string, s, len + 1);
    } else {
        v.data.as_string = NULL;
    }
    return v;
}

void value_free(Value* v) {
    if (!v) return;
    if (v->type == VAL_STRING && v->data.as_string) {
        free(v->data.as_string);
        v->data.as_string = NULL;
    }
    v->type = VAL_NULL;
}

bool value_is_truthy(const Value* v) {
    if (!v) return false;
    switch (v->type) {
        case VAL_NULL: return false;
        case VAL_BOOL: return v->data.as_bool;
        case VAL_NUMBER: return v->data.as_number != 0.0;
        case VAL_STRING: return v->data.as_string && v->data.as_string[0] != '\0';
    }
    return false;
}

Value value_clone(const Value* v) {
    if (!v) return value_null();
    switch (v->type) {
        case VAL_NULL: return value_null();
        case VAL_BOOL: return value_bool(v->data.as_bool);
        case VAL_NUMBER: return value_number(v->data.as_number);
        case VAL_STRING: return value_string(v->data.as_string ? v->data.as_string : "");
    }
    return value_null();
}


