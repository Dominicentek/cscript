#ifndef CSCRIPT_H
#define CSCRIPT_H

typedef void CScriptContext;
typedef struct {
    int row, col, len;
    char* msg;
} CScriptError;

CScriptContext* cscript_create_context();
void cscript_destroy_context(CScriptContext* context);
bool cscript_run(CScriptContext* context, const char* code);
bool cscript_get(CScriptContext* context, const char* name, void* out);
bool cscript_set(CScriptContext* context, const char* name, void* in);
bool cscript_any_errors(CScriptContext* context);
bool cscript_error(CScriptContext* context, CScriptError** error);

#endif
