#ifndef _FFIVALUE_H_
#define _FFIVALUE_H_

// forward declarations
class Variable;


/// Stores arguments and return value of an ffi_call, and deletes them
class FFIValue
{
public:
  virtual ~FFIValue() { }

  /// Read from a Ctrl Variable into the internal value
  virtual void setValue(const Variable &var) = 0;

  /// Write the internal value to a Ctrl Variable
  virtual void getValue(Variable &var) const = 0;

  /// Write the given Ctrl Variable to the given address as its native C type
  virtual void writeValueToRawMemory(const Variable &var, void *rawMemory) const = 0;

  /// Write from the given address as its native C type into the given Variable
  virtual void readValueFromRawMemory(Variable &var, const void *rawMemory) const = 0;

  /// Allocates a new Variable with a type appropriate for the current FFIValue
  virtual Variable *allocateCtrlVar() const = 0;

  /// Returns a void pointer to the internal value. To be used for ffi_call()
  virtual void *getPtr() = 0;

  /// Factory method for creating a new FFIValue for the given type
  static FFIValue *allocateValue(int type);

};

#endif // _FFIVALUE_H_
