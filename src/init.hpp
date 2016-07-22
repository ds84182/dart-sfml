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

class AbstractContainer {
public:
	AbstractContainer() {}
	AbstractContainer(const AbstractContainer&) = delete;
	AbstractContainer(AbstractContainer&&) = delete;
};

// The container class is (hackishly) used to let us pass references to values around
template <typename T>
class Container : AbstractContainer {
public:
	T value;

	template <typename... Args>
	Container(Args... args) : value(std::move(args)...) {}
};

static_assert(offsetof(Container<void*>, value) == 0, "Container value offset is not 0");

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
