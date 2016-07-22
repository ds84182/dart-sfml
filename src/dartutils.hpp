#pragma once

#include "dart_api.h"

// C++ Helpers to aid in Dart interactions

class DartHandle {
public:
  DartHandle(Dart_Handle handle) : handle(handle) {
    checkError();
  }
  DartHandle(const std::string &str) : handle(Dart_NewStringFromCString(str.c_str())) {
    checkError();
  }
  DartHandle(const char *str) : handle(Dart_NewStringFromCString(str)) {
    checkError();
  }
  DartHandle(uint64_t val) : handle(Dart_NewIntegerFromUint64(val)) {
    checkError();
  }
  DartHandle(int64_t val) : handle(Dart_NewInteger(val)) {
    checkError();
  }
  template <typename T>
  DartHandle(T *ptr) : handle(Dart_NewIntegerFromUint64(reinterpret_cast<uintptr_t>(ptr))) {
    checkError();
  }
  Dart_Handle handle;

  operator Dart_Handle() const {return handle;}

  uint64_t asUInt() {
    uint64_t temp;
    DartHandle(Dart_IntegerToUint64(handle, &temp));
    return temp;
  }

  operator uint64_t() {return asUInt();}

  int64_t asInt() {
    int64_t temp;
    DartHandle(Dart_IntegerToInt64(handle, &temp));
    return temp;
  }

  operator int64_t() {return asInt();}

  bool asBool() {
    bool b;
    DartHandle(Dart_BooleanValue(handle, &b));
    return b;
  }

  operator bool() {return asBool();}

  const std::string asString() {
    const char *str;
    Dart_StringToCString(handle, &str);
    return std::string(str);
  }

  template <typename T>
  T *asPointer() {
    return reinterpret_cast<T*>(asUInt());
  }

  DartHandle getField(DartHandle field) {
    return Dart_GetField(handle, field);
  }

  void setField(DartHandle field, DartHandle value) {
    DartHandle(Dart_SetField(handle, field, value));
  }

  void checkError() {
    if (Dart_IsError(handle)) {
      printf("%s\n", Dart_GetError(handle));
      abort();
    }
  }
};

class DartTypedData {
public:
  DartHandle handle;
  Dart_TypedData_Type typ;
  void *data = nullptr;
  size_t size = 0;

  DartTypedData(DartHandle handle) : handle(handle) {
    Dart_TypedDataAcquireData(handle, &typ, &data, (intptr_t*)&size);
    switch (typ) {
			case Dart_TypedData_kInt16:
			case Dart_TypedData_kUint16:
				size *= 2;
				break;
			case Dart_TypedData_kInt32:
			case Dart_TypedData_kUint32:
			case Dart_TypedData_kFloat32:
				size *= 4;
				break;
			case Dart_TypedData_kInt64:
			case Dart_TypedData_kUint64:
			case Dart_TypedData_kFloat64:
				size *= 8;
				break;
			case Dart_TypedData_kFloat32x4:
				size *= 16;
		}
  }

  ~DartTypedData() {
    Dart_TypedDataReleaseData(handle);
  }
};

class DartArgs {
public:
  DartArgs(Dart_NativeArguments args) : args(args) {}
  Dart_NativeArguments args;

  DartHandle operator[](int index) {
    return Dart_GetNativeArgument(args, index);
  }
};
