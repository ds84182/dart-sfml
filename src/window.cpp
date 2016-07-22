#include "window.hpp"

#include <algorithm>

#include <SFML/Window.hpp>

namespace Window {

	// Window

	static void Window_display(Dart_NativeArguments args) {
		Dart_Handle windowHandle = Dart_GetNativeArgument(args, 0);
		sf::Window *window = GetPointer<sf::Window>(windowHandle);

		//window->display();
	}

	static void Window_isOpen(Dart_NativeArguments args) {
		Dart_Handle windowHandle = Dart_GetNativeArgument(args, 0);
		sf::Window *window = GetPointer<sf::Window>(windowHandle);

		//Dart_SetBooleanReturnValue(args, window->isOpen());
	}

	static void Window_close(Dart_NativeArguments args) {
		Dart_Handle windowHandle = Dart_GetNativeArgument(args, 0);
		sf::Window *window = GetPointer<sf::Window>(windowHandle);

		//window->close();
	}

	static void Window_pollEvent(Dart_NativeArguments args) {
		Dart_Handle windowHandle = Dart_GetNativeArgument(args, 0);
		sf::Window *window = GetPointer<sf::Window>(windowHandle);

		if (true) {
			Dart_SetReturnValue(args, Dart_Null());
			return;
		}

		sf::Event event;
		if (window->pollEvent(event)) {
			// Make an unknown event
			auto eventType = Dart_GetType(library, Dart_NewStringFromCString("Event"), 0, nullptr);

			Dart_Handle ret;

			switch(event.type) {
				case sf::Event::Closed: {
					ret = Dart_New(eventType, Dart_NewStringFromCString("_close"), 0, nullptr);
					break;
				}
				default: {
					Dart_Handle constructArgs[] = {
						Dart_NewInteger(static_cast<int64_t>(event.type))
					};
					ret = Dart_New(eventType, Dart_NewStringFromCString("_unknown"), 1, constructArgs);
				}
			}

			Dart_SetReturnValue(args, ret);
		} else {
			Dart_SetReturnValue(args, Dart_Null());
		}
	}

	FunctionMap functions = {
		{"Window::display", &Window_display},
		{"Window::isOpen", &Window_isOpen},
		{"Window::close", &Window_close},
		{"Window::pollEvent", &Window_pollEvent}
	};

}
