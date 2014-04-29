#uses "CtrlFFI"

// path to the C runtime library
string clibPath;

//------------------------------------------------------------------------------

int GetLastError_func = 0;

ulong GetLastError()
{
  // DWORD WINAPI GetLastError(void);
  if (! GetLastError_func)
  {
    GetLastError_func = ffiDeclareFunction("kernel32.dll", "GetLastError", FFI_UINT32);
  }

  ulong errorcode = 0;
  ffiCallFunction(GetLastError_func, errorcode);

  return errorcode;
}

//------------------------------------------------------------------------------

int GetSystemTime_func = 0;

mapping GetSystemTime()
{
  /*
  void WINAPI GetSystemTime(__out  LPSYSTEMTIME lpSystemTime);

  typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
  } SYSTEMTIME, *PSYSTEMTIME;
  */

  dyn_int SYSTEMTIME = makeDynInt(
    FFI_UINT16, // WORD wYear
    FFI_UINT16, // WORD wMonth
    FFI_UINT16, // WORD wDayOfWeek
    FFI_UINT16, // WORD wDay
    FFI_UINT16, // WORD wHour
    FFI_UINT16, // WORD wMinute
    FFI_UINT16, // WORD wSecond
    FFI_UINT16  // WORD wMilliseconds
  );

  // declare function
  if (! GetSystemTime_func)
  {
    GetSystemTime_func = ffiDeclareFunction("kernel32.dll", "GetSystemTime", FFI_VOID, FFI_POINTER);
  }

  // prepare struct
  int structsize = ffiGetTypeSize(FFI_UINT16) * dynlen(SYSTEMTIME);
  int systime_ptr = ffiAllocBuffer(structsize);

  // call function
  ffiCallFunction(GetSystemTime_func, 0, systime_ptr);

  // use results
  dyn_int values = ffiBufferToStruct(systime_ptr, SYSTEMTIME);

  // delete struct
  ffiFreeBuffer(systime_ptr);

  // test some other functions
  systime_ptr = ffiAllocBuffer(structsize);

  ffiFillBufferWithStruct(systime_ptr, SYSTEMTIME, values);
  dyn_int copy = ffiBufferToStruct(systime_ptr, SYSTEMTIME);

  DebugN("copied struct correct?", copy == values);

  DebugN("read from address", ffiReadFromPointer(systime_ptr + ffiGetTypeSize(FFI_UINT16), FFI_UINT16));

  ffiWriteToPointer(systime_ptr + ffiGetTypeSize(FFI_UINT16), FFI_UINT16, 17);
  DebugN("read from address after change", ffiReadFromPointer(systime_ptr + ffiGetTypeSize(FFI_UINT16), FFI_UINT16));

  dyn_int changedstruct = ffiBufferToStruct(systime_ptr, SYSTEMTIME);
  DebugN("check changed struct field", changedstruct[2]);

  ffiFreeBuffer(systime_ptr);

  // create a descriptive mapping as result
  dyn_string description = makeDynString(
    "wYear",
    "wMonth",
    "wDayOfWeek",
    "wDay",
    "wHour",
    "wMinute",
    "wSecond",
    "wMilliseconds"
  );

  mapping result;
  for (int i = 1; i <= dynlen(description); ++i)
  {
    result[description[i]] = values[i];
  }

  return result;
}

//------------------------------------------------------------------------------

int GetCommandLine_func = 0;

string GetCommandLine()
{
  // LPTSTR WINAPI GetCommandLine(void);

  if (! GetCommandLine_func)
  {
    GetCommandLine_func = ffiDeclareFunction("kernel32.dll", "GetCommandLineA", FFI_STRING);
  }

  string returnvalue;
  ffiCallFunction(GetCommandLine_func, returnvalue);

  return returnvalue;
}

//------------------------------------------------------------------------------

int malloc_func = 0;
int free_func = 0;

uint allocate(uint size)
{
  // void *malloc(size_t size);
  if (! malloc_func)
  {
    malloc_func = ffiDeclareFunction(clibPath, "malloc", FFI_POINTER, FFI_UINT32);
  }

  // void free(void *ptr);
  if (! free_func)
  {
    free_func = ffiDeclareFunction(clibPath, "free", FFI_VOID, FFI_POINTER);
  }

  int newaddr = 0;
  ffiCallFunction(malloc_func, newaddr, size);
  // free() returns void, so we need a dummy param (0)
  // TODO: maybe a call to a function returning void just shouldn't take a
  // return value argument
  ffiCallFunction(free_func, 0, newaddr);

  return newaddr;
}

//------------------------------------------------------------------------------

int printf_int_func = 0;

int printfToHex(int value)
{
  // int printf(char *format, int value);

  if (! printf_int_func)
  {
    printf_int_func = ffiDeclareFunction(clibPath, "printf", FFI_INT, FFI_STRING, FFI_INT);
  }

  int returnvalue = 0;
  ffiCallFunction(printf_int_func, returnvalue, "hex: %X\n", value);

  return returnvalue;
}

//------------------------------------------------------------------------------

int Beep_func = 0;

bool Beep(ulong frequencyHz, ulong durationMs)
{
  // BOOL WINAPI Beep(__in  DWORD dwFreq, __in  DWORD dwDuration);

  if (! Beep_func)
  {
    Beep_func = ffiDeclareFunction("kernel32.dll", "Beep", FFI_INT, FFI_UINT32, FFI_UINT32);
  }

  int returnvalue = 0;
  ffiCallFunction(Beep_func, returnvalue, frequencyHz, durationMs);

  return returnvalue;
}

//------------------------------------------------------------------------------

int time_func = 0;

int unix_time()
{
  // time_t time(time_t *time);

  if (! time_func)
  {
    if (_WIN32)
    {
      time_func = ffiDeclareFunction(clibPath, "_time64", FFI_INT64, FFI_INT64_PTR);
    }
    else
    {
      time_func = ffiDeclareFunction(clibPath, "time", FFI_INT, FFI_INT_PTR);
    }
  }

  long returnvalue = 0;
  long timevalue = 0;
  ffiCallFunction(time_func, returnvalue, timevalue);

  if (returnvalue != timevalue)
  {
    // just so we can be sure it doesn't happen
    DebugN("call to time() failed, return value differs from out param");
  }

  return timevalue;
}

//------------------------------------------------------------------------------

int localtime_func = 0;

mapping localtime(long timestamp)
{
  /*
  struct tm {
    int tm_sec;     // seconds after the minute - [0,59]
    int tm_min;     // minutes after the hour - [0,59]
    int tm_hour;    // hours since midnight - [0,23]
    int tm_mday;    // day of the month - [1,31]
    int tm_mon;     // months since January - [0,11]
    int tm_year;    // years since 1900
    int tm_wday;    // days since Sunday - [0,6]
    int tm_yday;    // days since January 1 - [0,365]
    int tm_isdst;   // daylight savings time flag
  };
  */

  dyn_int tm = makeDynInt(
    FFI_INT, // int tm_sec
    FFI_INT, // int tm_min
    FFI_INT, // int tm_hour
    FFI_INT, // int tm_mday
    FFI_INT, // int tm_mon
    FFI_INT, // int tm_year
    FFI_INT, // int tm_wday
    FFI_INT, // int tm_yday
    FFI_INT  // int tm_isdst
  );

  // struct tm * localtime(time_t *time);

  if (! localtime_func)
  {
    localtime_func = ffiDeclareFunction(clibPath, (_WIN32 ? "_localtime64" : "localtime"), FFI_POINTER, FFI_INT64_PTR);
  }

  ulong tm_ptr = 0;
  ffiCallFunction(localtime_func, tm_ptr, timestamp);

  dyn_int values = ffiBufferToStruct(tm_ptr, tm);

  dyn_string description = makeDynString(
    "seconds after the minute - [0,59]",
    "minutes after the hour - [0,59]",
    "hours since midnight - [0,23]",
    "day of the month - [1,31]",
    "months since January - [0,11]",
    "years since 1900",
    "days since Sunday - [0,6]",
    "days since January 1 - [0,365]",
    "daylight savings time flag"
  );

  mapping result;
  for (int i = 1; i <= dynlen(description); ++i)
  {
    result[description[i]] = values[i];
  }

  return result;
}

//------------------------------------------------------------------------------

int backtrace_func = 0;
int backtrace_symbols_func = 0;

dyn_string backtrace()
{
  int maxFrameCount = 30;
  ulong frameArr = ffiAllocBuffer(maxFrameCount * ffiGetTypeSize(FFI_POINTER));

  if (! backtrace_func)
  {
    backtrace_func = ffiDeclareFunction(clibPath, "backtrace", FFI_INT, FFI_POINTER, FFI_INT);
    backtrace_symbols_func = ffiDeclareFunction(clibPath, "backtrace_symbols", FFI_POINTER, FFI_POINTER, FFI_INT);
  }

  // get the stack frame pointers
  int frameCount = 0;
  ffiCallFunction(backtrace_func, frameCount, frameArr, maxFrameCount);

  // get textual descriptions for the frames
  ulong symbolArr = 0;
  ffiCallFunction(backtrace_symbols_func, symbolArr, frameArr, frameCount);

  // convert to a dyn_string
  dyn_string result;
  for (int i = 0; i < frameCount; ++i)
  {
    ulong symbolPtr = symbolArr + (i * ffiGetTypeSize(FFI_POINTER));
    string line = ffiBufferToString(symbolPtr);
    dynAppend(result, line);
  }

  ffiFreeBuffer(symbolArr);
  ffiFreeBuffer(frameArr);

  return result;
}

//------------------------------------------------------------------------------

void printDeclaredFunctions()
{
  dyn_mapping functions = ffiGetAllFunctions();

  for (uint i = 1; i <= dynlen(functions); ++i)
  {
    mapping func = functions[i];
    string funcDecl = ffiGetTypeName(func["returntype"]) + " " + func["name"] + "(";

    dyn_uint argtypes = func["argtypes"];
    for (uint argidx = 1; argidx <= dynlen(argtypes); ++argidx)
    {
      if (argidx > 1)
      {
        funcDecl += ", ";
      }
      
      funcDecl += ffiGetTypeName(argtypes[argidx]);
    }
    
    funcDecl += ");";

    DebugN(func["id"], func["library"], funcDecl);
  }
}

//------------------------------------------------------------------------------

main()
{
  if (_WIN32)
  {
    clibPath = "msvcr90.dll";
  }
  else
  {
    clibPath = "libc.so.6";
  }

  ulong ptr = allocate(256);
  DebugN("ptr from malloc", ptr);
  DebugN("hex ptr from malloc", printfToHex(ptr));
  
  DebugN("hex word", printfToHex(0xCAFE));
  
  DebugN("size of int", ffiGetTypeSize(FFI_INT));
  DebugN("size of double", ffiGetTypeSize(FFI_DOUBLE));
  DebugN("size of int64", ffiGetTypeSize(FFI_INT64));

  int timestamp = unix_time();
  DebugN("unix time()", timestamp);
  DebugN("unix localtime()", localtime(timestamp));

  if (_WIN32)
  {
    DebugN("GetCommandLine()", GetCommandLine());
    DebugN("GetLastError()", GetLastError());
  
    DebugN("Beep()", Beep(750, 300));
    DebugN("GetLastError()", GetLastError());
  
    DebugN("current date and time", GetSystemTime());
  }
  else
  {
    DebugN("backtrace()", backtrace());
  }

  printDeclaredFunctions();
}
