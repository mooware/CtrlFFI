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

  * `uint ffiDeclareFunction(string libPath, string name [, int returntype [, int paramtype1, ...] ])`

Registers a function in CtrlFFI.

Tries to load the shared library at *libPath*, and retrieves the function with the given *name* from it.
The library will be searched with the OS-dependent dynamic loading mechanism (Windows: `LoadLibrary`, Linux: `dlopen`).

Following after the *name* parameter are the return type of the function, and the types of the parameters (from the list of type constants above).

If successful, it will return an ID that can be used with `ffiCallFunction` to call the registered function.

  * `bool ffiCallFunction(uint funcId [, anytype &returnvalue [, anytype &paramvalue1, ...] ])`

Calls a registered function.

*returnvalue* will receive the return value of the function, if the return type is not `FFI_VOID`. The following parameters will be given as arguments to the called function, and can be changed by the function, depending on the parameter type.

Note that for a function with parameters, but without a return type (= void), there still has to be a *returnvalue* parameter, but in that case it can be anything (even a literal like `0` should work).

Returns `true` if the function was successfully called, otherwise `false`.

  * `dyn_mapping ffiGetAllFunctions()`

Returns descriptions of all registered functions.

The result is a list of mappings, with one entry per function. Each entry is a mapping with the keys "id", "name", "library", "returntype" and "argtypes".

  * `uint ffiGetTypeSize(int type)`

Returns the native size in bytes of the given type.

For example `ffiGetTypeSize(FFI_INT64)` returns `8`. This can be used for manual memory management, like structs and arrays.

  * `string ffiGetTypeName(int type)`

Returns the name of the given type. For example, `ffiGetTypeName(FFI_VOID)` returns "FFI_VOID".

  * `ulong ffiAllocBuffer(ulong bytes, bool setzero = true)`

Equivalent to `malloc()` in C. Allocates the given number of bytes. The allocated memory will be zeroed if *setzero* is `true`.

  * `void ffiFreeBuffer(ulong ptr)`

Equivalent to `free()` in C. Deallocates a piece of dynamically allocated memory.

  * `string ffiBufferToString(ulong ptr [, int strlen])`

Reads a string from the given address. If *strlen* is specified, only the given number of bytes will be read, otherwise the string will be read up to the first null-byte.

  * `dyn_anytype ffiBufferToStruct(ulong ptr, dyn_int fieldtypes)`

Reads a struct from the given address.

Based on the list of types given in *fieldtypes*, it will read those types from the memory starting at the given *ptr*, and return them in a dyn. This can be used to read values from a struct.

Note that memory alignment will not be handled automatically. Instead, the padding bytes introduced for alignment have to be explicitly included in *fieldtypes*.

  * `dyn_anytype ffiBufferToDyn(ulong ptr, int itemtype, uint itemcount)`

Reads an array from the given address.

Similar to `ffiBufferToStruct`, it will interpret the memory starting at the given *ptr* as an array with *itemcount* elements of type *itemtype*. The result will be returned in a dyn.

  * `void ffiFillBufferWithString(ulong ptr, string text)`

Writes a string to the given address. This is the inverse of `ffiBufferToString`.

Note that there have to be at least `strlen(text) + 1` bytes writable at the address given in *ptr*.

  * `void ffiFillBufferWithStruct(ulong ptr, dyn_int fieldtypes, dyn_anytype fieldvalues)`

Writes a struct to the given address. This is the inverse of `ffiBufferToStruct`. See its description for details.

  * `void ffiFillBufferWithDyn(ulong ptr, int itemtype, dyn_anytype itemvalues)`

Writes a dyn (an array) to the given address. This is the inverse of `ffiBufferToDyn`. See its description for details.

  * `anytype ffiReadFromPointer(ulong ptr, int type)`

Reads a value of the given type from the given address.

  * `void ffiWriteToPointer(ulong ptr, int type, anytype value)`

Writes a value as the given type to the given address.

## Notes

TODO: calling convention, structs, varargs

