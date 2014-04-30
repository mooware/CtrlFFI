#include <FFIExternHdl.hxx>

#include <Controller.hxx>

#include <AnyTypeVar.hxx>
#include <BitVar.hxx>
#include <CharVar.hxx>
#include <IntegerVar.hxx>
#include <UIntegerVar.hxx>
#include <LongVar.hxx>
#include <ULongVar.hxx>
#include <FloatVar.hxx>
#include <TextVar.hxx>
#include <MappingVar.hxx>
#include <Resources.hxx>
#include <SharedLib.hxx>

#include <PVSSMacros.hxx>

#include <memory>
#include <cstring>


/// Enum of the functions implemented in this Ctrl extension
enum
{
  // basic interaction with functions
  F_fiiDeclareFunction = 0,
  F_ffiCallFunction,
  F_ffiGetAllFunctions,
  F_ffiGetTypeSize,
  F_ffiGetTypeName,
  // allocation
  F_ffiAllocBuffer,
  F_ffiFreeBuffer,
  // copy from raw memory to various structures
  F_ffiBufferToString,
  F_ffiBufferToStruct,
  F_ffiBufferToDyn,
  // copy from various structures to raw memory
  F_ffiFillBufferWithString,
  F_ffiFillBufferWithStruct,
  F_ffiFillBufferWithDyn,
  // direct memory access
  F_ffiReadFromPointer,
  F_ffiWriteToPointer
};

static FunctionListRec fnList[] =
{
//  return type, function name, parameter list, thread safe
//------------------------------------------------------------------------------
  { UINTEGER_VAR,   "ffiDeclareFunction",      "(string libPath, string name [, int returntype [, int paramtype1, ...] ] )", false },
  { BIT_VAR,        "ffiCallFunction",         "(uint funcId, anytype &returnvalue, anytype &paramvalue1, ...)", false },
  { DYNMAPPING_VAR, "ffiGetAllFunctions",      "", false },
  { UINTEGER_VAR,   "ffiGetTypeSize",          "(int type)", false },
  { TEXT_VAR,       "ffiGetTypeName",          "(int type)", false },

  { ULONG_VAR,      "ffiAllocBuffer",          "(ulong bytes, bool setzero = true)", false },
  { NO_VAR,         "ffiFreeBuffer",           "(ulong ptr)", false },

  { TEXT_VAR,       "ffiBufferToString",       "(ulong ptr [, int strlen] )", false },
  { DYN_VAR,        "ffiBufferToStruct",       "(ulong ptr, dyn_int fieldtypes)", false },
  { DYN_VAR,        "ffiBufferToDyn",          "(ulong ptr, int itemtype, uint itemcount)", false },

  { NO_VAR,         "ffiFillBufferWithString", "(ulong ptr, string text)", false },
  { NO_VAR,         "ffiFillBufferWithStruct", "(ulong ptr, dyn_int fieldtypes, dyn_anytype fieldvalues)", false },
  { NO_VAR,         "ffiFillBufferWithDyn",    "(ulong ptr, int itemtype, dyn_anytype itemvalues)", false },

  { ANYTYPE_VAR,    "ffiReadFromPointer",      "(ulong ptr, int type)", false },
  { NO_VAR,         "ffiWriteToPointer",       "(ulong ptr, int type, anytype value)", false }
};

CTRL_EXTENSION(FFIExternHdl, fnList);


// debug flag "-dbg CTRLFFI"
PVSSshort FFIExternHdl::dbgFlag = -1;

// an array to map from the enum IntegralType to an appropriate type name
static const char * const TYPE_NAMES[CTRLFFI_MAX_VALUE] = {
  0,                 // CTRLFFI_FIRST_VALUE_TYPE

  "FFI_UCHAR",       // CTRLFFI_UCHAR
  "FFI_CHAR",        // CTRLFFI_CHAR
  "FFI_USHORT",      // CTRLFFI_USHORT
  "FFI_SHORT",       // CTRLFFI_SHORT
  "FFI_UINT",        // CTRLFFI_UINT
  "FFI_INT",         // CTRLFFI_INT
  "FFI_ULONG",       // CTRLFFI_ULONG
  "FFI_LONG",        // CTRLFFI_LONG
  "FFI_FLOAT",       // CTRLFFI_FLOAT
  "FFI_DOUBLE",      // CTRLFFI_DOUBLE

  "FFI_UINT8",       // CTRLFFI_UINT8
  "FFI_INT8",        // CTRLFFI_INT8
  "FFI_UINT16",      // CTRLFFI_UINT16
  "FFI_INT16",       // CTRLFFI_INT16
  "FFI_UINT32",      // CTRLFFI_UINT32
  "FFI_INT32",       // CTRLFFI_INT32
  "FFI_UINT64",      // CTRLFFI_UINT64
  "FFI_INT64",       // CTRLFFI_INT64

  0,                 // CTRLFFI_LAST_VALUE_TYPE
  0,                 // CTRLFFI_FIRST_PTR

  "FFI_UCHAR_PTR",   // CTRLFFI_UCHAR_PTR
  "FFI_CHAR_PTR",    // CTRLFFI_CHAR_PTR
  "FFI_USHORT_PTR",  // CTRLFFI_USHORT_PTR
  "FFI_SHORT_PTR",   // CTRLFFI_SHORT_PTR
  "FFI_UINT_PTR",    // CTRLFFI_UINT_PTR
  "FFI_INT_PTR",     // CTRLFFI_INT_PTR
  "FFI_ULONG_PTR",   // CTRLFFI_ULONG_PTR
  "FFI_LONG_PTR",    // CTRLFFI_LONG_PTR
  "FFI_FLOAT_PTR",   // CTRLFFI_FLOAT_PTR
  "FFI_DOUBLE_PTR",  // CTRLFFI_DOUBLE_PTR

  "FFI_UINT8_PTR",   // CTRLFFI_UINT8_PTR
  "FFI_INT8_PTR",    // CTRLFFI_INT8_PTR
  "FFI_UINT16_PTR",  // CTRLFFI_UINT16_PTR
  "FFI_INT16_PTR",   // CTRLFFI_INT16_PTR
  "FFI_UINT32_PTR",  // CTRLFFI_UINT32_PTR
  "FFI_INT32_PTR",   // CTRLFFI_INT32_PTR
  "FFI_UINT64_PTR",  // CTRLFFI_UINT64_PTR
  "FFI_INT64_PTR",   // CTRLFFI_INT64_PTR

  0,                 // CTRLFFI_LAST_PTR

  "FFI_POINTER",     // CTRLFFI_POINTER
  "FFI_VOID",        // CTRLFFI_VOID
  "FFI_STRING",      // CTRLFFI_STRING
};

//------------------------------------------------------------------------------

FFIExternHdl::FFIExternHdl(BaseExternHdl *nextHdl, PVSSulong funcCount, FunctionListRec fnList[])
  : BaseExternHdl(nextHdl, funcCount, fnList)
{
  if (dbgFlag == -1)
  {
    dbgFlag = Resources::registerDbgFlag("CTRLFFI", "Foreign function interface declarations and calls");
  }

  // HACK: add all types as global vars. i don't know whether this has any
  // issues, but it seems a lot better than changing the Ctrl interpreter code
  // because of a Ctrl extension.

  for (unsigned int i = 0; i < (sizeof(TYPE_NAMES) / sizeof(*TYPE_NAMES)); ++i)
  {
    const char *typeName = TYPE_NAMES[i];
    if (typeName != 0)
    {
      CtrlVar *typeVar = new CtrlVar(new UIntegerVar(i));
      typeVar->setName(typeName);
      Controller::thisPtr->addGlobal(typeVar);
    }
  }
}

//------------------------------------------------------------------------------

FFIExternHdl::~FFIExternHdl()
{
}

//------------------------------------------------------------------------------

const Variable *FFIExternHdl::execute(ExecuteParamRec &param)
{
  static UIntegerVar returnUInt;
  static ULongVar returnULong;
  static BitVar returnBool;
  static TextVar returnText;
  static AnyTypeVar returnAny;

  switch (param.funcNum)
  {
    case F_fiiDeclareFunction: returnUInt.setValue(ffiDeclareFunction(param)); return &returnUInt;
    case F_ffiCallFunction:    returnBool.setValue(ffiCallFunction(param)); return &returnBool;
    case F_ffiGetAllFunctions: returnAny.setVar(ffiGetAllFunctions(param)); return &returnAny;
    case F_ffiGetTypeSize:     returnUInt.setValue(ffiGetTypeSize(param)); return &returnUInt;
    case F_ffiGetTypeName:     returnText.setValue(ffiGetTypeName(param)); return &returnText;

    case F_ffiAllocBuffer:     returnULong.setValue(ffiAllocBuffer(param)); return &returnULong;
    case F_ffiFreeBuffer:      ffiFreeBuffer(param); returnBool.setValue(PVSS_TRUE); return &returnBool;

    case F_ffiBufferToString:  returnText.setValuePtr(ffiBufferToString(param)); return &returnText;
    case F_ffiBufferToStruct:  returnAny.setVar(ffiBufferToStruct(param)); return &returnAny;
    case F_ffiBufferToDyn:     returnAny.setVar(ffiBufferToDyn(param)); return &returnAny;

    case F_ffiFillBufferWithString: ffiFillBufferWithString(param); returnBool.setValue(PVSS_TRUE); return &returnBool;
    case F_ffiFillBufferWithStruct: ffiFillBufferWithStruct(param); returnBool.setValue(PVSS_TRUE); return &returnBool;
    case F_ffiFillBufferWithDyn:    ffiFillBufferWithDyn(param); returnBool.setValue(PVSS_TRUE); return &returnBool;

    case F_ffiReadFromPointer: returnAny.setVar(ffiReadFromPointer(param)); return &returnAny;
    case F_ffiWriteToPointer:  ffiWriteToPointer(param); returnBool.setValue(PVSS_TRUE); return &returnBool;

    default:
    {
      ErrClass err(ErrClass::PRIO_SEVERE, ErrClass::ERR_CONTROL, ErrClass::UNDEFD_FUNC,
                   param.thread->getLocation(), param.funcName);
      ErrHdl::error(err);
      param.thread->appendLastError(err);

      break;
    }
  }

  returnBool.setValue(PVSS_FALSE);
  return &returnBool;
}

//------------------------------------------------------------------------------

// Ctrl: unsigned int ffiDeclareFunction(string libPath, string name [, int returntype [, int paramtype1, ... ] ] )
unsigned int FFIExternHdl::ffiDeclareFunction(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 2)
  {
    // TODO: error
    return 0;
  }

  // get arguments
  TextVar paramLibPath;
  paramLibPath = *(param.args->getFirst()->evaluate(param.thread));

  TextVar paramFuncName;
  paramFuncName = *(param.args->getNext()->evaluate(param.thread));

  // load the library
  if (! SharedLib::load(paramLibPath.getValue()))
  {
    // TODO: error. lib not found.
    return 0;
  }

  SharedLibraryFunction fn = SharedLib::getFuncPtr(paramLibPath.getValue(), paramFuncName.getValue(), true);
  if (! fn)
  {
    // TODO: error. function not found in lib.
    return 0;
  }

  // create the function declaration
  std::auto_ptr<FFIFunction> newFunc(new FFIFunction());
  newFunc->returnType = CTRLFFI_VOID;
  ffi_type *returnType = &ffi_type_void;
  ffi_type **argTypes = 0;
  unsigned int argCount = 0;

  // get the return type
  if (param.args->getNumberOfItems() > 2)
  {
    IntegerVar paramReturnType;
    paramReturnType = *(param.args->getNext()->evaluate(param.thread));

    // CTRLFFI_<FOO>_PTR cannot be used as return type.
    if (paramReturnType.getValue() >= CTRLFFI_FIRST_PTR &&
        paramReturnType.getValue() <= CTRLFFI_LAST_PTR)
    {
      // TODO: error. invalid type for return value.
      return 0;
    }

    returnType = getFFIType(paramReturnType.getValue());
    if (! returnType)
    {
      // TODO: error
      return 0;
    }

    newFunc->returnType = static_cast<IntegralType>(paramReturnType.getValue());

    // get the argument types
    if (param.args->getNumberOfItems() > 3)
    {
      argCount = param.args->getNumberOfItems() - 3;
      newFunc->argTypes.reserve(argCount);

      argTypes = new ffi_type *[argCount];

      for (unsigned int i = 0; i < argCount; ++i)
      {
        IntegerVar paramArgType;
        paramArgType = *(param.args->getNext()->evaluate(param.thread));

        ffi_type *argType = getFFIType(paramArgType.getValue());
        if (! argType)
        {
          // TODO: error
          delete[] argTypes;
          return 0;
        }

        argTypes[i] = argType;
        newFunc->argTypes.push_back(static_cast<IntegralType>(paramArgType.getValue()));
      }
    }
  }

  // take the cif from the function object to avoid leaks
  ffi_cif *cif = &(newFunc->callInterface);
  ffi_status res = ffi_prep_cif(cif, FFI_DEFAULT_ABI, argCount, returnType, argTypes);

  if (res == FFI_OK)
  {
    newFunc->libName = paramLibPath.getString();
    newFunc->funcName = paramFuncName.getString();
    newFunc->funcPtr = reinterpret_cast<VoidFunction>(fn);

    DEBUG_PRINT(dbgFlag, "Declared function " << newFunc->funcName << " from library " << newFunc->libName);

    functions.append(newFunc.release());

    // the function id is a one-based index in the list
    // (because zero is already used to indicate failure)
    return functions.getNumberOfItems();
  }

  return 0;
}

//------------------------------------------------------------------------------

// Ctrl: bool ffiCallFunction(unsigned int funcId [, anytype &returnvalue [, anytype &paramvalue1, ... ] ] )
bool FFIExternHdl::ffiCallFunction(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 1)
  {
    // TODO: error
    return false;
  }

  // find the function declaration
  UIntegerVar paramFuncId;
  paramFuncId = *(param.args->getFirst()->evaluate(param.thread));

  if (paramFuncId.getValue() < 1 || paramFuncId.getValue() > functions.getNumberOfItems())
  {
    // TODO: error. invalid function id.
    return false;
  }

  FFIFunction *func = functions.getAt(paramFuncId.getValue() - 1);
  if (! func)
  {
    // TODO: error. shouldn't happen, we already checked the id.
    return false;
  }

  // check the number of params
  size_t argCount = func->argTypes.size();
  size_t expectedParams = 1;

  if (argCount > 0) { expectedParams += (argCount + 1); }
  else if (func->returnType != CTRLFFI_VOID) { ++expectedParams; }

  if (param.args->getNumberOfItems() < expectedParams)
  {
    // TODO: error
    return false;
  }

  // prepare return value storage
  SimplePtrArray<FFIValue> storedValues;
  std::auto_ptr<FFIValue> returnValueStorage(FFIValue::allocateValue(func->returnType));

  if (! returnValueStorage.get())
  {
    // TODO: error. shouldn't happen.
    return false;
  }

  void *returnValue = returnValueStorage->getPtr();
  storedValues.append(returnValueStorage.release());

  // skip return value param, we don't need it now
  param.args->getNext();

  // prepare function args
  void **argValues = new void *[argCount];

  for (size_t i = 0; i < argCount; ++i)
  {
    int argType = func->argTypes.at(i);
    std::auto_ptr<FFIValue> argValueStorage(FFIValue::allocateValue(argType));

    if (! argValueStorage.get())
    {
      // TODO: error. shouldn't happen.
      delete[] argValues;
      return false;
    }

    const Variable *paramArgVar = param.args->getNext()->evaluate(param.thread);
    if (! paramArgVar) // TODO: can this be null?
    {
      // TODO: error
      delete[] argValues;
      return false;
    }

    argValueStorage->setValue(*paramArgVar);
    argValues[i] = argValueStorage->getPtr();
    storedValues.append(argValueStorage.release());
  }

  // actual function call
  DEBUG_PRINT(dbgFlag, "Calling function " << func->funcName << " from library " << func->libName);

  ffi_call(&(func->callInterface), func->funcPtr, returnValue, argValues);

  // convert args and return value back

  // reset param list
  param.args->getFirst();

  // NOTE: we will not throw an error if a ctrl param is not assignable.
  // this allows using literals as params.
  // TODO: maybe throw a warning?

  // index 0 is the return value, index 1 to <argCount> are the arguments
  for (size_t i = 0; i <= argCount; ++i)
  {
    Variable *target = param.args->getNext()->getTarget(param.thread);
    if (! target) // TODO: can this be null?
    {
      // TODO: error
      return false;
    }

    storedValues.getAt((unsigned int) i)->getValue(*target);
  }

  return true;
}

//------------------------------------------------------------------------------

// Ctrl: dyn_mapping ffiGetAllFunctions()
DynVar *FFIExternHdl::ffiGetAllFunctions(ExecuteParamRec &param)
{
  DynVar *result = new DynVar(MAPPING_VAR);

  for (unsigned int i = 0; i < functions.getNumberOfItems(); ++i)
  {
    const FFIFunction *function = functions.getAt(i);

    MappingVar *funcDesc = new MappingVar();
    funcDesc->setAt(new TextVar("id"),         new UIntegerVar(i + 1));
    funcDesc->setAt(new TextVar("name"),       new TextVar(function->funcName));
    funcDesc->setAt(new TextVar("library"),    new TextVar(function->libName));
    funcDesc->setAt(new TextVar("returntype"), new UIntegerVar(function->returnType));

    DynVar *argTypes = new DynVar(UINTEGER_VAR);
    for (std::vector<IntegralType>::const_iterator it = function->argTypes.begin();
         it != function->argTypes.end(); ++it)
    {
      argTypes->append(new UIntegerVar(*it));
    }

    funcDesc->setAt(new TextVar("argtypes"), argTypes);

    result->append(funcDesc);
  }

  return result;
}

//------------------------------------------------------------------------------

// Ctrl: uint ffiGetTypeSize(int type)
unsigned int FFIExternHdl::ffiGetTypeSize(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 1)
  {
    // TODO: error
    return 0;
  }

  IntegerVar paramType;
  paramType = *(param.args->getFirst()->evaluate(param.thread));

  // ffi_type_void has size 1, but 0 seems more sensible
  if (paramType.getValue() == CTRLFFI_VOID)
  {
    return 0;
  }

  ffi_type *type = getFFIType(paramType.getValue());
  if (type)
  {
    return (unsigned int) type->size;
  }

  return 0;
}

//------------------------------------------------------------------------------

// Ctrl: string ffiGetTypeName(int type)
const char *FFIExternHdl::ffiGetTypeName(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 1)
  {
    // TODO: error. too few arguments.
    return "";
  }

  IntegerVar paramType;
  paramType = *(param.args->getFirst()->evaluate(param.thread));

  int typeArraySize = (int) sizeof(TYPE_NAMES) / sizeof(*TYPE_NAMES);
  if (paramType.getValue() < 0 || paramType.getValue() >= typeArraySize)
  {
    // TODO: error. out of range.
    return "";
  }

  return TYPE_NAMES[paramType.getValue()];
}

//------------------------------------------------------------------------------

// Ctrl: ulong ffiAllocBuffer(ulong bytes, bool setzero = true)
PVSSulonglong FFIExternHdl::ffiAllocBuffer(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 1)
  {
    // TODO: error. too few arguments.
    return 0;
  }

  // get args
  ULongVar paramBytes;
  paramBytes = *(param.args->getFirst()->evaluate(param.thread));

  if (paramBytes.getValue() == 0)
  {
    // TODO: no error?
    return 0;
  }

  bool setzero = true;
  if (param.args->getNumberOfItems() > 1)
  {
    BitVar paramZero;
    paramZero = *(param.args->getNext()->evaluate(param.thread));
    setzero = paramZero.isTrue();
  }

  // allocate the buffer.
  // we use malloc() since libffi is only meant to call C libraries,
  // which probably use free() to delete this buffer.
  void *buffer = malloc(static_cast<size_t>(paramBytes.getValue()));

  // if requested, clear the buffer
  if (setzero)
  {
    memset(buffer, 0, static_cast<size_t>(paramBytes.getValue()));
  }

  uintptr_t ptrValue = reinterpret_cast<uintptr_t>(buffer);
  return static_cast<PVSSulonglong>(ptrValue);
}

//------------------------------------------------------------------------------

// Ctrl: void ffiFreeBuffer(ulong ptr)
void FFIExternHdl::ffiFreeBuffer(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 1)
  {
    // TODO: error
    return;
  }

  // get args
  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));

  if (paramPtr.getValue() == 0)
  {
    return;
  }

  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());
  void *buffer = reinterpret_cast<void *>(ptrValue);

  // we use free() since libffi is only meant to call C libraries,
  // which probably used malloc() to allocate this buffer.
  free(buffer);
}

//------------------------------------------------------------------------------

// Ctrl: string ffiBufferToString(ulong ptr [, int strlen] )
char *FFIExternHdl::ffiBufferToString(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 1)
  {
    // TODO: error
    return 0;
  }

  ULongVar ptr;
  ptr = *(param.args->getFirst()->evaluate(param.thread));

  uintptr_t ptrValue = static_cast<uintptr_t>(ptr.getValue());
  const char *buffer = reinterpret_cast<const char *>(ptrValue);

  if (buffer == 0)
  {
    // TODO: error?
    return 0;
  }

  size_t length = 0;
  if (param.args->getNumberOfItems() > 1)
  {
    // length of string given

    IntegerVar paramStrlen;
    paramStrlen = *(param.args->getNext()->evaluate(param.thread));

    if (paramStrlen.getValue() < 0)
    {
      // TODO: error
      return 0;
    }

    length = (size_t) paramStrlen.getValue();
  }
  else
  {
    // no explicit length of string given

    length = strlen(buffer);
  }

  // copy the memory through CharString
  return CharString(buffer, length).cutCharPtr();
}

//------------------------------------------------------------------------------

// Ctrl: dyn_anytype ffiBufferToStruct(ulong ptr, dyn_int fieldtypes)
DynVar *FFIExternHdl::ffiBufferToStruct(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 2)
  {
    // TODO: error
    return 0;
  }

  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));

  if (paramPtr.getValue() == 0)
  {
    // TODO: error. null pointer.
    return 0;
  }

  // HACK: iterate through the memory by incrementing an uintptr_t and
  // casting to a pointer when necessary.
  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());

  std::auto_ptr<DynVar> result(new DynVar());

  DynVar paramFields;
  paramFields = *(param.args->getNext()->evaluate(param.thread));

  for (const Variable *field = paramFields.getFirst(); field != 0;
                       field = paramFields.getNext())
  {
    IntegerVar typeVal;
    typeVal = *field;
    if (typeVal.getValue() == CTRLFFI_VOID) // TODO: use isvalid...
    {
      // TODO: error. void cannot be a struct field.
      return 0;
    }

    ffi_type *type = getFFIType(typeVal.getValue());
    if (type == 0)
    {
      // TODO: error. invalid type.
      return 0;
    }

    const char *buffer = reinterpret_cast<const char *>(ptrValue);
    Variable *fieldValue = readAddress(typeVal.getValue(), buffer);
    if (! fieldValue)
    {
      // TODO: error. failed to read field?
      return 0;
    }

    result->append(fieldValue);

    // advance to next field
    ptrValue += type->size;
  }

  return result.release();
}

//------------------------------------------------------------------------------

// Ctrl: dyn_anytype ffiBufferToDyn(ulong ptr, int itemtype, uint itemcount)
DynVar *FFIExternHdl::ffiBufferToDyn(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 3)
  {
    // TODO: error
    return 0;
  }

  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));

  // HACK: iterate through the memory by incrementing an uintptr_t and
  // casting to a pointer when necessary.
  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());

  std::auto_ptr<DynVar> result(new DynVar());

  IntegerVar paramItemType;
  paramItemType = *(param.args->getNext()->evaluate(param.thread));

  UIntegerVar paramItemCount;
  paramItemCount = *(param.args->getNext()->evaluate(param.thread));

  if (! isValidForRawMemoryOperation(paramItemType.getValue()))
  {
    // TODO: error. invalid type.
    return 0;
  }

  ffi_type *type = getFFIType(paramItemType.getValue());
  if (type == 0)
  {
    // TODO: error. invalid type.
    return 0;
  }

  for (unsigned int i = 0; i < paramItemCount.getValue(); ++i)
  {
    const char *buffer = reinterpret_cast<const char *>(ptrValue);
    Variable *fieldValue = readAddress(paramItemType.getValue(), buffer);
    if (! fieldValue)
    {
      // TODO: error. failed to read field?
      return 0;
    }

    result->append(fieldValue);

    // advance to next field
    ptrValue += type->size;
  }

  return result.release();
}

//------------------------------------------------------------------------------

// Ctrl: void ffiFillBufferWithString(ulong ptr, string text)
void FFIExternHdl::ffiFillBufferWithString(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 2)
  {
    // TODO: error
    return;
  }

  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));
  if (! paramPtr.isTrue())
  {
    // TODO: error. null pointer.
    return;
  }

  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());
  char *buffer = reinterpret_cast<char *>(ptrValue);

  TextVar paramText;
  paramText = *(param.args->getNext()->evaluate(param.thread));

  strcpy(buffer, paramText.getValue());
}

//------------------------------------------------------------------------------

// Ctrl: void ffiFillBufferWithStruct(ulong ptr, dyn_int fieldtypes, dyn_anytype fieldvalues)
void FFIExternHdl::ffiFillBufferWithStruct(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 3)
  {
    // TODO: error
    return;
  }

  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));

  // HACK: iterate through the memory by incrementing an uintptr_t and
  // casting to a pointer when necessary.
  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());

  DynVar paramFieldTypes;
  paramFieldTypes = *(param.args->getNext()->evaluate(param.thread));

  DynVar paramFieldValues;
  paramFieldValues = *(param.args->getNext()->evaluate(param.thread));

  if (paramFieldTypes.getNumberOfItems() != paramFieldValues.getNumberOfItems())
  {
    // TODO: error. arrays do not have the same number of items.
    return;
  }

  for (unsigned int i = 1; i <= paramFieldTypes.getNumberOfItems(); ++i)
  {
    IntegerVar fieldType;
    fieldType = *(paramFieldTypes[i]);
    if (fieldType.getValue() == CTRLFFI_VOID) // TODO: use isvalid...
    {
      // TODO: error. void cannot be a struct field.
      return;
    }

    ffi_type *type = getFFIType(fieldType.getValue());
    if (type == 0)
    {
      // TODO: error. invalid type.
      return;
    }

    char *buffer = reinterpret_cast<char *>(ptrValue);
    writeAddress(fieldType.getValue(), buffer, *(paramFieldValues[i]));

    // advance to next field
    ptrValue += type->size;
  }
}

//------------------------------------------------------------------------------

// Ctrl: void ffiFillBufferWithDyn(ulong ptr, int itemtype, dyn_anytype itemvalues)
void FFIExternHdl::ffiFillBufferWithDyn(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 3)
  {
    // TODO: error
    return;
  }

  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));

  // HACK: iterate through the memory by incrementing an uintptr_t and
  // casting to a pointer when necessary.
  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());

  IntegerVar paramItemType;
  paramItemType = *(param.args->getNext()->evaluate(param.thread));

  if (paramItemType.getValue() == CTRLFFI_VOID) // use isvalid...
  {
    // TODO: error. void cannot be a struct field.
    return;
  }

  ffi_type *type = getFFIType(paramItemType.getValue());
  if (type == 0)
  {
    // TODO: error. invalid type.
    return;
  }

  DynVar paramItemValues;
  paramItemValues = *(param.args->getNext()->evaluate(param.thread));

  for (unsigned int i = 1; i <= paramItemValues.getNumberOfItems(); ++i)
  {
    char *buffer = reinterpret_cast<char *>(ptrValue);
    writeAddress(paramItemType.getValue(), buffer, *(paramItemValues[i]));

    // advance to next field
    ptrValue += type->size;
  }
}

//------------------------------------------------------------------------------

// Ctrl: anytype ffiReadFromPointer(ulong ptr, int type)
Variable *FFIExternHdl::ffiReadFromPointer(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 2)
  {
    // TODO: error
    return 0;
  }

  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));

  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());
  const char *buffer = reinterpret_cast<const char *>(ptrValue);

  IntegerVar paramType;
  paramType = *(param.args->getNext()->evaluate(param.thread));

  Variable *newValue = readAddress(paramType.getValue(), buffer);
  if (! newValue)
  {
    // TODO: error. failed to read from address? invalid type?
    return 0;
  }

  return newValue;
}

//------------------------------------------------------------------------------

// Ctrl: void ffiWriteToPointer(ulong ptr, int type, anytype value)
void FFIExternHdl::ffiWriteToPointer(ExecuteParamRec &param)
{
  if (param.args->getNumberOfItems() < 2)
  {
    // TODO: error
    return;
  }

  ULongVar paramPtr;
  paramPtr = *(param.args->getFirst()->evaluate(param.thread));

  uintptr_t ptrValue = static_cast<uintptr_t>(paramPtr.getValue());
  char *buffer = reinterpret_cast<char *>(ptrValue);

  IntegerVar paramType;
  paramType = *(param.args->getNext()->evaluate(param.thread));

  const Variable *paramValue = param.args->getNext()->evaluate(param.thread);

  writeAddress(paramType.getValue(), buffer, *paramValue);
}

//------------------------------------------------------------------------------
// helper functions:

ffi_type *FFIExternHdl::getFFIType(int type)
{
  // all CTRLFFI_<FOO>_PTR types are just pointers for libffi
  if (type > CTRLFFI_FIRST_PTR && type < CTRLFFI_LAST_PTR)
  {
    return &ffi_type_pointer;
  }

  switch (type)
  {
    // normal types
    case CTRLFFI_UCHAR:   return &ffi_type_uchar;
    case CTRLFFI_CHAR:    return &ffi_type_schar;
    case CTRLFFI_USHORT:  return &ffi_type_ushort;
    case CTRLFFI_SHORT:   return &ffi_type_sshort;
    case CTRLFFI_UINT:    return &ffi_type_uint;
    case CTRLFFI_INT:     return &ffi_type_sint;
    case CTRLFFI_ULONG:   return &ffi_type_ulong;
    case CTRLFFI_LONG:    return &ffi_type_slong;
    case CTRLFFI_FLOAT:   return &ffi_type_float;
    case CTRLFFI_DOUBLE:  return &ffi_type_double;
    // fixed length types
    case CTRLFFI_UINT8:   return &ffi_type_uint8;
    case CTRLFFI_INT8:    return &ffi_type_sint8;
    case CTRLFFI_UINT16:  return &ffi_type_uint16;
    case CTRLFFI_INT16:   return &ffi_type_sint16;
    case CTRLFFI_UINT32:  return &ffi_type_uint32;
    case CTRLFFI_INT32:   return &ffi_type_sint32;
    case CTRLFFI_UINT64:  return &ffi_type_uint64;
    case CTRLFFI_INT64:   return &ffi_type_sint64;
    // special types
    case CTRLFFI_STRING: // fall through
    case CTRLFFI_POINTER: return &ffi_type_pointer;
    case CTRLFFI_VOID:    return &ffi_type_void;

    default: break; // TODO: error. invalid type.
  }

  return 0;
}

//------------------------------------------------------------------------------

bool FFIExternHdl::isValidForRawMemoryOperation(int type)
{
  return (type > CTRLFFI_FIRST_VALUE_TYPE && type < CTRLFFI_LAST_VALUE_TYPE) ||
          type == CTRLFFI_POINTER;
}

//------------------------------------------------------------------------------

Variable *FFIExternHdl::readAddress(int type, const char *buffer)
{
  std::auto_ptr<FFIValue> conversionObj;
  conversionObj.reset(FFIValue::allocateValue(type));

  if (conversionObj.get())
  {
    Variable *resultVar = conversionObj->allocateCtrlVar();

    if (resultVar)
    {
      conversionObj->readValueFromRawMemory(*resultVar, buffer);
      return resultVar;
    }
  }

  return 0;
}

//------------------------------------------------------------------------------

/// Writes from a Ctrl var to a pointer
void FFIExternHdl::writeAddress(int type, char *buffer, const Variable &var)
{
  std::auto_ptr<FFIValue> conversionObj;
  conversionObj.reset(FFIValue::allocateValue(type));

  if (conversionObj.get())
  {
    conversionObj->writeValueToRawMemory(var, buffer);
  }
}
