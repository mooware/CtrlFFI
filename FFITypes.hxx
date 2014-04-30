#ifndef _FFITYPES_H_
#define _FFITYPES_H_

// HACK: cannot include stdint.h, since it conflicts with Types.hxx.
// try to define uintptr_t based on the size of a void pointer.

template <unsigned int Size> struct GetUIntPtrType;
template <> struct GetUIntPtrType<4> { typedef unsigned int Type; };
template <> struct GetUIntPtrType<8> { typedef unsigned long long Type; };

typedef GetUIntPtrType<sizeof(void *)>::Type uintptr_t;

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
  CTRLFFI_POINTER,
  CTRLFFI_VOID,
  CTRLFFI_STRING,

  // this value terminates the enum. it must always be the last one.
  CTRLFFI_MAX_VALUE
};

#endif // _FFITYPES_H_
