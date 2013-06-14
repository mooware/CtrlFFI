#include <FFIValue.hxx>

#include <FFITypes.hxx>

#include <Variable.hxx>
#include <CharVar.hxx>
#include <FloatVar.hxx>
#include <IntegerVar.hxx>
#include <UIntegerVar.hxx>
#include <LongVar.hxx>
#include <ULongVar.hxx>
#include <TextVar.hxx>

//------------------------------------------------------------------------------
// helper class template FFIScalarValue

/// An implementation of FFIValue for scalar types
template <typename CType, typename CtrlType>
class FFIScalarValue : public FFIValue
{
public:

  virtual void setValue(const Variable &var)
  {
    writeValueToRawMemory(var, &value);
  }

  virtual void getValue(Variable &var) const
  {
    readValueFromRawMemory(var, &value);
  }

  virtual void writeValueToRawMemory(const Variable &var, void *rawMemory) const
  {
    // convert any Ctrl type to expected Ctrl type
    CtrlType tmpVar;
    tmpVar = var;

    // convert the Ctrl Type to its native peer
    CType *nativeValue = static_cast<CType *>(rawMemory);
    *nativeValue = static_cast<CType>(tmpVar.getValue());
  }

  virtual void readValueFromRawMemory(Variable &var, const void *rawMemory) const
  {
    // cast the void pointer to its expected native type
    const CType *nativeValue = static_cast<const CType *>(rawMemory);

    // store the native value in the corresponding Ctrl type
    CtrlType tmpVar;
    tmpVar.setValue(*nativeValue);

    var = tmpVar;
  }

  virtual Variable *allocateCtrlVar() const { return new CtrlType; };

  virtual void *getPtr() { return static_cast<void *>(&value); }

private:
  CType value;
};

//------------------------------------------------------------------------------
// helper class template FFIPointerToScalarValue

/**
 * An implementation of FFIValue for a pointer to a scalar value.
 *
 * Functions using this value are not allowed to delete the pointer or to
 * allocate something new instead of it.
 * The value behind the pointer can be freely changed though.
 */
template <typename CType, typename CtrlType>
class FFIPointerToScalarValue : public FFIScalarValue<CType, CtrlType>
{
public:
  /// Type of the base class
  typedef FFIScalarValue<CType, CtrlType> BaseClass;

  virtual void *getPtr()
  {
    // make sure that the ptr is set correctly
    ptr = BaseClass::getPtr();
    return static_cast<void *>(&ptr);
  }

private:
  // this member only exists because libffi wants the address of a pointer
  void *ptr;
};

//------------------------------------------------------------------------------
// helper class FFIVoidValue

/// An implementation of FFIValue for type void. Doesn't store or do anything.
class FFIVoidValue : public FFIValue
{
public:
  virtual void setValue(const Variable &) { }
  virtual void getValue(Variable &) const { }
  virtual void writeValueToRawMemory(const Variable &, void *) const { }
  virtual void readValueFromRawMemory(Variable &, const void *) const { }
  virtual Variable *allocateCtrlVar() const { return 0; };
  virtual void *getPtr() { return 0; }
};

//------------------------------------------------------------------------------
// helper class FFIPointerValue

/**
 * An implementation of FFIValue for pointers which we don't own.
 * Converts them from and to ULongVar, or to a TextVar if one was given.
 */
class FFIPointerValue : public FFIValue
{
public:
  virtual void setValue(const Variable &var)
  {
    writeValueToRawMemory(var, static_cast<void *>(&value));
  }

  virtual void getValue(Variable &var) const
  {
    readValueFromRawMemory(var, static_cast<const void *>(&value));
  }

  virtual void writeValueToRawMemory(const Variable &var, void *rawMemory) const
  {
    // convert any Ctrl type to expected Ctrl type
    ULongVar tmpVar;
    tmpVar = var;
    uintptr_t ptrValue = static_cast<uintptr_t>(tmpVar.getValue());

    // convert the Ctrl Type to its native peer
    void **nativeValue = static_cast<void **>(rawMemory);
    *nativeValue = reinterpret_cast<void *>(ptrValue);
  }

  virtual void readValueFromRawMemory(Variable &var, const void *rawMemory) const
  {
    // cast the void pointer to its expected native type
    const uintptr_t *ptrValue = static_cast<const uintptr_t *>(rawMemory);

    // store the native value in the corresponding Ctrl type
    ULongVar tmpVar;
    tmpVar.setValue(*ptrValue);

    var = tmpVar;
  }

  virtual Variable *allocateCtrlVar() const { return new ULongVar; };

  virtual void *getPtr() { return static_cast<void *>(&value); }

private:
  void *value;
};

//------------------------------------------------------------------------------
// helper class FFICharPointerValue

/**
 * An implementation of FFIValue that stores and owns a char array.
 *
 * Functions using this value are not allowed to change or delete the text.
 * If the text was not set before, the function can set it to a meaningful
 * value so that it can be copied to a TextVar.
 *
 * This value can either be used to give a read-only string to a function (e.g. printf()),
 * or to receive a read-only string as return value (e.g. getenv()).
 */
class FFICharPointerValue : public FFIValue
{
public:
  FFICharPointerValue() : value(0) { }

  virtual void setValue(const Variable &var)
  {
    textStorage = var;
    value = textStorage.getValue();
  }

  virtual void getValue(Variable &var) const
  {
    TextVar tmpVar;
    tmpVar.setValue(value);
    var = tmpVar;
  }

  virtual void writeValueToRawMemory(const Variable &var, void *rawMemory) const
  {
    // TODO: error?
  }

  virtual void readValueFromRawMemory(Variable &var, const void *rawMemory) const
  {
    // TODO: error?
  }

  virtual Variable *allocateCtrlVar() const { return new TextVar; };

  virtual void *getPtr() { return static_cast<void *>(&value); }

private:
  const char *value;
  TextVar textStorage;
};


//------------------------------------------------------------------------------
// FFIValue factory method

FFIValue *FFIValue::allocateValue(int type)
{
  switch (type)
  {
    // non-fixed length types
    case CTRLFFI_UCHAR:  return new FFIScalarValue<unsigned char,  UIntegerVar>();
    case CTRLFFI_CHAR:   return new FFIScalarValue<char,           CharVar>();
    case CTRLFFI_USHORT: return new FFIScalarValue<unsigned short, UIntegerVar>();
    case CTRLFFI_SHORT:  return new FFIScalarValue<short,          IntegerVar>();
    case CTRLFFI_UINT:   return new FFIScalarValue<unsigned int,   UIntegerVar>();
    case CTRLFFI_INT:    return new FFIScalarValue<int,            IntegerVar>();
    case CTRLFFI_ULONG:  return new FFIScalarValue<unsigned long,  ULongVar>();
    case CTRLFFI_LONG:   return new FFIScalarValue<long,           LongVar>();
    case CTRLFFI_FLOAT:  return new FFIScalarValue<float,          FloatVar>();
    case CTRLFFI_DOUBLE: return new FFIScalarValue<double,         FloatVar>();
    // fixed length types
    case CTRLFFI_UINT8:  return new FFIScalarValue<uint8_t,  UIntegerVar>();
    case CTRLFFI_INT8:   return new FFIScalarValue<int8_t,   IntegerVar>();
    case CTRLFFI_UINT16: return new FFIScalarValue<uint16_t, UIntegerVar>();
    case CTRLFFI_INT16:  return new FFIScalarValue<int16_t,  IntegerVar>();
    case CTRLFFI_UINT32: return new FFIScalarValue<uint32_t, UIntegerVar>();
    case CTRLFFI_INT32:  return new FFIScalarValue<int32_t,  IntegerVar>();
    case CTRLFFI_UINT64: return new FFIScalarValue<uint64_t, ULongVar>();
    case CTRLFFI_INT64:  return new FFIScalarValue<int64_t,  LongVar>();
    // pointer types
    case CTRLFFI_UCHAR_PTR:  return new FFIPointerToScalarValue<unsigned char,  UIntegerVar>();
    case CTRLFFI_CHAR_PTR:   return new FFIPointerToScalarValue<char,           CharVar>();
    case CTRLFFI_USHORT_PTR: return new FFIPointerToScalarValue<unsigned short, UIntegerVar>();
    case CTRLFFI_SHORT_PTR:  return new FFIPointerToScalarValue<short,          IntegerVar>();
    case CTRLFFI_UINT_PTR:   return new FFIPointerToScalarValue<unsigned int,   UIntegerVar>();
    case CTRLFFI_INT_PTR:    return new FFIPointerToScalarValue<int,            IntegerVar>();
    case CTRLFFI_ULONG_PTR:  return new FFIPointerToScalarValue<unsigned long,  ULongVar>();
    case CTRLFFI_LONG_PTR:   return new FFIPointerToScalarValue<long,           LongVar>();
    case CTRLFFI_FLOAT_PTR:  return new FFIPointerToScalarValue<float,          FloatVar>();
    case CTRLFFI_DOUBLE_PTR: return new FFIPointerToScalarValue<double,         FloatVar>();

    case CTRLFFI_UINT8_PTR:  return new FFIPointerToScalarValue<uint8_t,  UIntegerVar>();
    case CTRLFFI_INT8_PTR:   return new FFIPointerToScalarValue<int8_t,   IntegerVar>();
    case CTRLFFI_UINT16_PTR: return new FFIPointerToScalarValue<uint16_t, UIntegerVar>();
    case CTRLFFI_INT16_PTR:  return new FFIPointerToScalarValue<int16_t,  IntegerVar>();
    case CTRLFFI_UINT32_PTR: return new FFIPointerToScalarValue<uint32_t, UIntegerVar>();
    case CTRLFFI_INT32_PTR:  return new FFIPointerToScalarValue<int32_t,  IntegerVar>();
    case CTRLFFI_UINT64_PTR: return new FFIPointerToScalarValue<uint64_t, ULongVar>();
    case CTRLFFI_INT64_PTR:  return new FFIPointerToScalarValue<int64_t,  LongVar>();
    // special types
    case CTRLFFI_POINTER:    return new FFIPointerValue();
    case CTRLFFI_VOID:       return new FFIVoidValue();
    case CTRLFFI_STRING:     return new FFICharPointerValue();
  }

  return 0;
}
