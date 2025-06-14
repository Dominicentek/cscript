# CScript

A single-file scripting engine written in C that runs C designed with convenience, flexibility and minimal boilerplate in mind.

> [!WARNING]
> This engine only supports x86_64 architectures with either System V or WinABI interfaces.

## Usage

1. Create the scripting context
```c
CScriptContext* context = cscript_create_context();
```
2. Run some code
```c
const char* code = "...";
cscript_exec(context, code);
```
3. When you're done, destroy the context
```c
cscript_destroy_context(context);
```

## Reading variables

You can read from any variable in the global scope.

This can be done with the `cscript_get` function. It returns `true` if the operation is successful and `false` if not.

A simple variable read looks like this:

```c
// script code:
// int value = 5;

int number;
cscript_get(context, "value", &number);
printf("%d\n", number); // 5
```
This also works with pointers, arrays, or structs.
```c
// script code:
// int array[5] = { 1, 2, 3, 4, 5 };
// struct {
//     float a;
//     int b;
// } structure = { 1.5f, 7 };

int* array;
struct {
    float a;
    int b;
} structure;
cscript_get(context, "array", &array);
cscript_get(context, "structure", &structure);
printf("%d\n", array[2]); // 3
printf("%g\n", structure.a); // 1.5
```
In order to call script functions, you just get the function and run it.
> [!NOTE]
> Calling a script returned function after destroying the context results in UB.
```c
// script code:
// int fact(int x) {
//     if (x <= 1) return x;
//     return x * fact(x - 1);
// }

int(*fact)(int x);
cscript_get(context, "fact", &fact);
printf("%d\n", fact(4)); // 24
```

## Writing variables

Simiarly, you can also write to variables in the global scope. To create a new variable, you can run a script that does so in the global scope within the same context.

This can be done with the `cscript_set` function. It returns `true` if the operation is successful and `false` if not.

A simple variable modification looks like this:
```c
int value;

cscript_exec(context, "int value = 5;");

cscript_get(context, "value", &value);
printf("%d\n", value); // 5

int new_value = 10;
cscript_set(context, "value", &new_value);

cscript_get(context, "value", &value);
printf("%d\n", value); // 10
```
C rules still apply, so you can't modify a constant, an array or a function.

Calling native functions within the script also works seamlessly:
```c
// script code:
// int(*fact)(int x);
// int result;

int fact(int x) {
    if (x <= 1) return x;
    return x * fact(x - 1);
}

int result;
cscript_set(context, "fact", &fact);
cscript_exec(context, "result = fact(4);");
cscript_get(context, "result", &result);
printf("%d\n", &result); // 24
```

## Symbol visibility

Scripts will see all symbols that are visible to the linker. This means you can call any function that isn't marked as `static` or included in a linked library. This doesn't apply to just functions, global non-static variables are also visible.
```c
int some_value = 5;
int some_function(int a, int b) {
    return a + b;
}

const char* script = "some_value = some_function(1, 2);";
cscript_exec(context, script);
printf("%d\n", some_value); // 3
```

## Error checking

CScript provides 2 functions for error checking:
* `cscript_any_errors`: Returns `true` if there's an error in the queue.
* `cscript_error`: Returns `true` if there's an error in the queue, pops it and writes it to the parameter.

Errors can be checked with the following code
```c
cscript_exec(context, "...");

if (cscript_any_errors(context)) {
    CScriptError* error;
    while (cscript_error(context, &error)) {
        printf("(%d:%d) %s\n", error->row, error->col, error->msg);
    }
}
else printf("Script executed successfully\n");
```
