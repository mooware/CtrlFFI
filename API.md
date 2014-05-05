# CtrlFFI API

## Data Types

CtrlFFI defines a number of constants that can be used to specify types of return values and parameters for functions.
Each constant is associated with a native C datatype and a Ctrl datatype. When calling a function, arguments will be converted to the associated Ctrl type and then to the associated C type; return values will be converted to the associated Ctrl type after the call.

### Scalar types

These are the typical datatypes of C, passed or received "by value", or as a copy.

The C equivalent would be:
```c
int param, result;

param = 0;
result = myfunc(param);
```

  * `FFI_UCHAR` (C: `unsigned char`, Ctrl: `uint`)
  * `FFI_CHAR` (C: `char`, Ctrl: `char`)
  * `FFI_USHORT` (C: `unsigned short`, Ctrl: `uint`)
  * `FFI_SHORT` (C: `short`, Ctrl: `int`)
  * `FFI_UINT` (C: `unsigned int`, Ctrl: `uint`)
  * `FFI_INT` (C: `int`, Ctrl: `int`)
  * `FFI_ULONG` (C: `unsigned long`, Ctrl: `ulong`)
  * `FFI_LONG` (C: `long`, Ctrl: `long`)
  * `FFI_FLOAT` (C: `float`, Ctrl: `float`)
  * `FFI_DOUBLE` (C: `double`, Ctrl: `float`)

### C99 typedefs with fixed bit size

Basically the same as above, but with the `(u)int_<bits>_t` datatypes.

  * `FFI_UINT8` (C: `uint8_t`, Ctrl: `uint`)
  * `FFI_INT8` (C: `int8_t`, Ctrl: `int`)
  * `FFI_UINT16` (C: `uint16_t`, Ctrl: `uint`)
  * `FFI_INT16` (C: `int16_t`, Ctrl: `int`)
  * `FFI_UINT32` (C: `uint32_t`, Ctrl: `uint`)
  * `FFI_INT32` (C: `int32_t`, Ctrl: `int`)
  * `FFI_UINT64` (C: `uint64_t`, Ctrl: `ulong`)
  * `FFI_INT64` (C: `int64_t`, Ctrl: `long`)

### Scalar types passed as pointers

Analogous to the types above, these are values passed or received "by reference". This can be used for output parameters.

The C equivalent would be:
```c
int param, result;
int *tmp;

tmp = myfunc(&param);
result = *tmp;
```

  * `FFI_UCHAR_PTR` (C: ``unsigned char *``, Ctrl: `uint`)
  * `FFI_CHAR_PTR` (C: ``char *``, Ctrl: `char`)
  * `FFI_USHORT_PTR` (C: ``unsigned short *``, Ctrl: `uint`)
  * `FFI_SHORT_PTR` (C: `short *`, Ctrl: `int`)
  * `FFI_UINT_PTR` (C: `unsigned int *`, Ctrl: `uint`)
  * `FFI_INT_PTR` (C: `int *`, Ctrl: `int`)
  * `FFI_ULONG_PTR` (C: `unsigned long *`, Ctrl: `ulong`)
  * `FFI_LONG_PTR` (C: `long *`, Ctrl: `long`)
  * `FFI_FLOAT_PTR` (C: `float *`, Ctrl: `float`)
  * `FFI_DOUBLE_PTR` (C: `double *`, Ctrl: `float`)

### C99 typedefs as pointers

Same as above, but with the `(u)int_<bits>_t` datatypes.

  * `FFI_UINT8_PTR` (C: `uint8_t *`, Ctrl: `uint`)
  * `FFI_INT8_PTR` (C: `int8_t *`, Ctrl: `int`)
  * `FFI_UINT16_PTR` (C: `uint16_t *`, Ctrl: `uint`)
  * `FFI_INT16_PTR` (C: `int16_t *`, Ctrl: `int`)
  * `FFI_UINT32_PTR` (C: `uint32_t *`, Ctrl: `uint`)
  * `FFI_INT32_PTR` (C: `int32_t *`, Ctrl: `int`)
  * `FFI_UINT64_PTR` (C: `uint64_t *`, Ctrl: `ulong`)
  * `FFI_INT64_PTR` (C: `int64_t *`, Ctrl: `long`)

### Special types

  * `FFI_POINTER` (C: any kind of pointer, Ctrl: `ulong`)

This is a raw pointer, e.g. `void *` or `char *`. In Ctrl, it is represented by the numerical value of the pointer. This can be used for structs, arrays, dynamically allocated strings, or any kind of manually managed memory. See the `ffi*Buffer*` functions for details.

  * `FFI_STRING` (C: `char *`, Ctrl: `string`)

This is a convenience type for immutable strings. It cannot be used to transfer ownership of a string to or from a C function, and C functions also should not change the contents of the string.
It can be used for passing the format string or arguments to `printf`, and to receive the return value of `getenv`.

* `FFI_VOID` (C: `void`, Ctrl: nothing)

The only meaningful use for this type is for the return type of functions which do not return anything, i.e. which return `void`.

## Functions

*TODO*
