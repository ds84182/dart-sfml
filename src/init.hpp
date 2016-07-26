#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <string>

#include <dart_api.h>

using FunctionMap = std::map<std::string, Dart_NativeFunction>;

extern thread_local Dart_PersistentHandle library;

void GCHandle(Dart_Handle handle, size_t size, std::function<void()> func);

static inline Dart_Handle $(Dart_Handle handle) {
	if (Dart_IsError(handle)) Dart_PropagateError(handle);
	return handle;
}

// ;)
class SneekyPointer {
public:
	void *ptr;
	template <typename T>
	SneekyPointer(T *p) : ptr(reinterpret_cast<void*>(p)) {}
	template <typename T>
	operator T*() {return reinterpret_cast<T*>(ptr);}
	template <typename T>
	operator const T*() const {return reinterpret_cast<const T*>(ptr);}
};

template <typename T>
static inline void SetPointer(Dart_Handle handle, T *ptr) {
	Dart_SetField(handle,
		Dart_NewStringFromCString("_ptr"),
		Dart_NewIntegerFromUint64(reinterpret_cast<uintptr_t>(ptr)));
}

template <typename T>
static inline T *GetPointer(Dart_Handle handle) {
	uint64_t integer;
	Dart_IntegerToUint64(Dart_GetField(handle, Dart_NewStringFromCString("_ptr")), &integer);
	return reinterpret_cast<T*>(integer);
}
