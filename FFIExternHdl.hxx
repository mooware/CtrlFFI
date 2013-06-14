#ifndef _FFIEXTERNHDL_H_
#define _FFIEXTERNHDL_H_

#include <FFITypes.hxx>
#include <FFIValue.hxx>

#include <BaseExternHdl.hxx>
#include <SimplePtrArray.hxx>
#include <Types.hxx>

#include <ffi.h>

#include <vector>

// forward declarations
class Variable;

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
  unsigned int ffiDeclareFunction(ExecuteParamRec &param);

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
