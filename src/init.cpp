#include "init.hpp"
#include "graphics.hpp"
#include "window.hpp"

#include <array>

const std::array<const FunctionMap*, 2> maps = {
	&Graphics::functions,
	&Window::functions,
};

void GCHandle(Dart_Handle handle, size_t size, std::function<void()> func) {
	Dart_NewWeakPersistentHandle(handle, reinterpret_cast<void*>(new std::function<void()>(func)), size, [](void *, Dart_WeakPersistentHandle handle, void *func_v) {
		std::function<void()> *funcptr = reinterpret_cast<std::function<void()>*>(func_v);
		(*funcptr)();
		delete funcptr;
	});
}

static Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool* auto_setup_scope);

thread_local Dart_PersistentHandle library;

DART_EXPORT Dart_Handle sfml_Init(Dart_Handle parent_library) {
	if (Dart_IsError(parent_library)) return parent_library;

	library = Dart_NewPersistentHandle(parent_library);

	Dart_Handle result_code = Dart_SetNativeResolver(parent_library, ResolveName, nullptr);
	if (Dart_IsError(result_code)) return result_code;

	return Dart_Null();
}

Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool* auto_setup_scope) {
	// If we fail, we return nullptr, and Dart throws an exception.
	if (!Dart_IsString(name)) return nullptr;
	const char* cname;
	$(Dart_StringToCString(name, &cname));

	for (auto map : maps) {
		if (map->count(cname)) {
			return map->at(cname);
		}
	}

	return nullptr;
}
