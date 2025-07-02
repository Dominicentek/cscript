# CScript

A single-file scripting engine written in C that runs cimple, a minimalist C-like language.

Designed with convenience, flexibility and minimal boilerplate in mind.

> [!WARNING]
> This engine only supports x86_64 architectures with either System V or WinABI interfaces.

## C vs cimple

One thing that's different is mainly the type system. This project started as a C scripting language similar to [picoC](https://github.com/jpoirier/picoc), but it quickly became something different because of how abysmal the C type system is to parse.

The overall syntax of the language is the same, although several modifiers aren't recognized. Those include:

- `inline`
- `volatile`
- `register`
- `restrict`
- `_Atomic`
- `typedef`

Keywords like `goto`, `typeof` and `_Generic` are also not recognized.

You might be thinking: "How can I break multiple loops without `goto`?" Well, `break` now takes a numeric argument indicating how many layers of loops to break:
```c
while (true) {
    while (true) {
        break 2; // from here
    }
}
// to here
```

The C preprocessor is also absent, because there's no need. Instead of `#include`, you can use `include "file.cmpl";`

### Type system

The base types are clear and simple. Instead of `char`, `short`, `int`, `long`, `signed`, `unsigned`, `float` or `double`, you instead have these:

- `s8` - signed 8-bit integer
- `s16` - signed 16-bit integer
- `s32` - signed 32-bit integer
- `s64` - signed 64-bit integer
- `u8` - unsigned 8-bit integer
- `u16` - unsigned 16-bit integer
- `u32` - unsigned 32-bit integer
- `u64` - unsigned 64-bit integer
- `f32` - 32-bit floating point
- `f64` - 64-bit floating point
- `bool` - same as `s8`

`const` has to be strictly at the left side of the type, or after a pointer: `const s32* const`. `s32 const` isn't valid.

Arrays are after the *type* not the *identifier*. Instead of writing this: `s32 array[5]` you write `s32[5] array`. This makes declaring pointers to arrays easier, as instead of `s32 (*array)[5]` you can write `s32[5]* array`. Similarly, this applies to functions as well. The argument list is placed between the return type and the identifier, like this: `void(s32 a, s32 b)* function`.

Because of this, function definitions look a bit weird:
```c
s32(s32 x) fact {
    if (x <= 1) return x;
    return x * fact(x - 1);
}
```

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
// s32 value = 5;

int number;
cscript_get(context, "value", &number);
printf("%d\n", number); // 5
```
This also works with pointers, arrays, or structs.
```c
// script code:
// s32[5] array = { 1, 2, 3, 4, 5 };
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
// s32(s32 x) fact {
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

cscript_exec(context, "s32 value = 5;");

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
// extern s32(s32 x)* fact;
// s32 result;

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
s32 some_value = 5;
s32(s32 a, s32 b) some_function {
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
