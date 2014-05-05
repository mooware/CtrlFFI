CtrlFFI
=========

A foreign function interface for the CONTROL scripting language of WinCC OA.

It allows direct calls from CONTROL scripts to shared library functions written in C, without having to write a new CONTROL extension.

Usage
=====

```c
// register the function with its signature
uint funcId = ffiDeclareFunction("libc.so.6", "time", FFI_INT, FFI_INT_PTR);

// call the function
long returnvalue = 0;
long timevalue = 0;
ffiCallFunction(funcId, returnvalue, timevalue);
```

The [example.ctl](example.ctl) script contains a few examples for Windows and Linux. For a full description of the API, see [API.md](API.md).

Build
=====

Don't forget to check out the libffi submodule before building CtrlFFI.
On Windows, building libffi is quite difficult, so I wrote a script that you have to call before running the Visual Studio build.

There are pre-built binaries for WinCC OA 3.12 in the Releases tab.

License
=======

Licensed under the MIT License, see the [LICENSE](LICENSE) file included in the repository.
