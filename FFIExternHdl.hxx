#ifndef _FFIEXTERNHDL_H_
#define _FFIEXTERNHDL_H_

#include <BaseExternHdl.hxx>

#include <SimplePtrArray.hxx>
#include <Types.hxx>

#include <ffi.h>

#include <vector>

// forward declarations
class Variable;


/// Integral types supported by CtrlFFI
enum IntegralType
{
  // FIRST_VALUE_TYPE and LAST_VALUE_TYPE enclose the non-pointer, non-special
  // types to allow easy checking and easy mapping.

  // non-fixed size types
  CTRLFFI_FIRST_VALUE_TYPE = 0,
  CTRLFFI_UCHAR = 1,
  CTRLFFI_CHAR,
  CTRLFFI_USHORT,
  CTRLFFI_SHORT,
  CTRLFFI_UINT,
  CTRLFFI_INT,
  CTRLFFI_ULONG,
  CTRLFFI_LONG,
  CTRLFFI_FLOAT,
  CTRLFFI_DOUBLE,
  // fixed size types
  CTRLFFI_UINT8,
  CTRLFFI_INT8,
  CTRLFFI_UINT16,
  CTRLFFI_INT16,
  CTRLFFI_UINT32,
  CTRLFFI_INT32,
  CTRLFFI_UINT64,
  CTRLFFI_INT64,
  CTRLFFI_LAST_VALUE_TYPE,
  // pointer types. i rely on these being in the same order as the integral
  // types, so that mapping to them is easier.
  // FIRST_PTR and LAST_PTR enclose the pointer types to allow easy checking
  // for pointer types and easy mapping. they are not valid types.
  CTRLFFI_FIRST_PTR,
  CTRLFFI_UCHAR_PTR,
  CTRLFFI_CHAR_PTR,
  CTRLFFI_USHORT_PTR,
  CTRLFFI_SHORT_PTR,
  CTRLFFI_UINT_PTR,
  CTRLFFI_INT_PTR,
  CTRLFFI_ULONG_PTR,
  CTRLFFI_LONG_PTR,
  CTRLFFI_FLOAT_PTR,
  CTRLFFI_DOUBLE_PTR,
  CTRLFFI_UINT8_PTR,
  CTRLFFI_INT8_PTR,
  CTRLFFI_UINT16_PTR,
  CTRLFFI_INT16_PTR,
  CTRLFFI_UINT32_PTR,
  CTRLFFI_INT32_PTR,
  CTRLFFI_UINT64_PTR,
  CTRLFFI_INT64_PTR,
  CTRLFFI_LAST_PTR,
  // special types
  CTRLFFI_RAW_POINTER,
  CTRLFFI_VOID,
  CTRLFFI_STRING,

  // this value terminates the enum. it must always be the last one.
  CTRLFFI_MAX_VALUE
};


//------------------------------------------------------------------------------

class FFIExternHdl : public BaseExternHdl
{
public:
  /// Function pointer type as used by libffi
  typedef void (*VoidFunction)(void);

  /// Stores a function declaration
  struct FFIFunction
  {
    /// Constructor. Necessary to recognize an empty object in the dtor.
    FFIFunction() : funcPtr(0)
    {
      // set just enough to be able to recognize an empty cif
      callInterface.nargs = 0;
      callInterface.arg_types = 0;
    }

    /// Destructor. Necessary because the ffi_cif contains a heap allocated array.
    ~FFIFunction()
    {
      // Delete the arg_types array, which should be the only heap allocated
      // memory here
      if (callInterface.arg_types)
      {
        delete[] callInterface.arg_types;
      }
    }

    /// Name of the function (e.g. "memset")
    CharString funcName;
    /// Filename of the library containing the function
    CharString libName;
    /// Function pointer to call the function
    VoidFunction funcPtr;

    /// Return type of the function
    IntegralType returnType;
    /// List of argument types for the function
    std::vector<IntegralType> argTypes;

    /// libffi call interface definition
    ffi_cif callInterface;
  };

// boilerplate stuff
  FFIExternHdl(BaseExternHdl *nextHdl, PVSSulong funcCount, FunctionListRec fnList[]);

  virtual ~FFIExternHdl();

  virtual const Variable *execute(ExecuteParamRec &param);

private:
// ctrl functions
  unsigned int fiiDeclareFunction(ExecuteParamRec &param);

  bool ffiCallFunction(ExecuteParamRec &param);

  DynVar *ffiGetAllFunctions(ExecuteParamRec &param);

  unsigned int ffiGetTypeSize(ExecuteParamRec &param);

  const char *ffiGetTypeName(ExecuteParamRec &param);
  
  PVSSulonglong ffiAllocBuffer(ExecuteParamRec &param);
  
  void ffiFreeBuffer(ExecuteParamRec &param);
  
  char *ffiBufferToString(ExecuteParamRec &param);

  DynVar *ffiBufferToStruct(ExecuteParamRec &param);

  DynVar *ffiBufferToDyn(ExecuteParamRec &param);

  void ffiFillBufferWithString(ExecuteParamRec &param);

  void ffiFillBufferWithStruct(ExecuteParamRec &param);

  void ffiFillBufferWithDyn(ExecuteParamRec &param);

  Variable *ffiReadFromPointer(ExecuteParamRec &param);

  void ffiWriteToPointer(ExecuteParamRec &param);

// helpers
  /// Returns the ffi_type struct to be used for an IntegralType
  static ffi_type *getFFIType(int type);

  /// Returns true if the given type is valid for readAddress and writeAddress
  static bool isValidForRawMemoryOperation(int type);

  /// Reads from a pointer to a new Ctrl var
  static Variable *readAddress(int type, const char *buffer);

  /// Writes from a Ctrl var to a pointer
  static void writeAddress(int type, char *buffer, const Variable &var);

// members
  /// List of the declared functions
  SimplePtrArray<FFIFunction> functions;

  /// The number of the CTRLFFI -dbg flag
  static PVSSshort dbgFlag;
};

#endif // _FFIEXTERNHDL_H_
