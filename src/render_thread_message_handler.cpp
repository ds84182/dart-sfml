#include "render_thread.hpp"
#include "shader.hpp"

#include "dart_native_api.h"

namespace Graphics {

  static inline void CheckArray(Dart_CObject *object) {
		if (object->type != Dart_CObject_kArray) {
			printf("Abort: Object is not an Array\n");
			abort();
		}
	}

	static inline void CheckArrayLength(Dart_CObject *object, intptr_t length) {
		if (object->value.as_array.length < length) {
			printf("Abort: Object Array is not long enough %ld < %ld\n", object->value.as_array.length, length);
			abort();
		}
	}

	static inline Dart_CObject *GetArray(Dart_CObject *object, intptr_t index) {
		return object->value.as_array.values[index];
	}

	static inline int64_t CheckInt(Dart_CObject *object) {
		if (object->type != Dart_CObject_kInt32 && object->type != Dart_CObject_kInt64) {
			printf("Abort: Object is not an Integer\n");
			abort();
		}
		if (object->type == Dart_CObject_kInt32) return object->value.as_int32;
		return object->value.as_int64;
	}

	static inline std::string CheckString(Dart_CObject *object) {
		if (object->type != Dart_CObject_kString) {
			printf("Abort: Object is not a String\n");
			abort();
		}
		return object->value.as_string;
	}

	static inline Dart_Port CheckPort(Dart_CObject *object) {
		if (object->type != Dart_CObject_kSendPort) {
			printf("Abort: Object is not a SendPort\n");
			abort();
		}
		return object->value.as_send_port.id;
	}

	enum class RenderThreadMessageType {
		Init, // To Render Thread
		StartFrame, // To Dart Thread
		RenderFrame, // To Render Thread
		SetCommands, // To Render Thread (Format of [CommandBufPtr...])
		NewShader, // To Render Thread (ptr, vertexShaderStr, fragmentShaderStr, attributeLayout). We get a reply back with the ptr and the status
		ShaderResult, // To Dart Thread (ptr, errorLog as string if failed or null)
	};

	void RenderThreadMessageHandler(Dart_Port dest_port_id, Dart_CObject *message) {
		CheckArray(message);
		CheckArrayLength(message, 2);

		RenderThread *rt = reinterpret_cast<RenderThread*>(CheckInt(GetArray(message, 0)));
		RenderThreadMessageType type = static_cast<RenderThreadMessageType>(CheckInt(GetArray(message, 1)));

		switch(type) {
			case RenderThreadMessageType::Init:
				// Add an initialization message to the render thread
				rt->enqueue(std::bind([rt](Dart_Port port) {
					rt->replyPort = port;
				}, CheckPort(GetArray(message, 2))));
				break;
			case RenderThreadMessageType::RenderFrame:
				// Add a message to kick off the render
				rt->enqueue([rt]{
					rt->render = true;
				});
				break;
			case RenderThreadMessageType::SetCommands:
				// Add a message to do proper sync
				CheckArrayLength(message, 3);
				{
					Dart_CObject *list = GetArray(message, 2);
					std::vector<CommandBuffer> commandBuffers;
					auto len = list->value.as_array.length;

					commandBuffers.reserve(len);
					for (int i=0; i<len; i++) {
						commandBuffers.emplace_back(*reinterpret_cast<CommandBuffer*>(CheckInt(GetArray(list, i))));
					}

					rt->enqueue([rt, commandBuffers] {
						rt->commands = std::move(commandBuffers);
					});
				}
				break;
			case RenderThreadMessageType::NewShader:
				CheckArrayLength(message, 6);
				{
					int64_t shaderPointer = CheckInt(GetArray(message, 2));
					std::shared_ptr<Shader> shader = *reinterpret_cast<std::shared_ptr<Shader>*>(shaderPointer);
					std::string vs = CheckString(GetArray(message, 3));
					std::string fs = CheckString(GetArray(message, 4));
					std::unordered_map<GLuint, std::string> attributeLocations;
					Dart_CObject *attributeLocArray = GetArray(message, 5);

					if (attributeLocArray->type != Dart_CObject_kNull) {
						GLuint current = 0;
						while (current < attributeLocArray->value.as_array.length) {
							attributeLocations[CheckInt(GetArray(attributeLocArray, current+1))] = CheckString(GetArray(attributeLocArray, current));
							current += 2;
						}
					}

					rt->enqueue([=] {
						Dart_CObject reply;
						Dart_CObject replyValues[3];
						Dart_CObject *replyValuesPtr[3] = {&replyValues[0], &replyValues[1], &replyValues[2]};

						reply.type = Dart_CObject_kArray;
						reply.value.as_array.length = 3;
						reply.value.as_array.values = replyValuesPtr;

						replyValues[0].type = Dart_CObject_kInt32;
						replyValues[0].value.as_int32 = static_cast<int32_t>(RenderThreadMessageType::ShaderResult);

						replyValues[1].type = Dart_CObject_kInt64;
						replyValues[1].value.as_int64 = shaderPointer;

						if (!shader->compile(vs, fs, attributeLocations)) {
							printf("Shader error: %s\n", shader->errorLog.c_str());

							replyValues[2].type = Dart_CObject_kString;
							replyValues[2].value.as_string = const_cast<char*>(shader->errorLog.c_str());
						} else {
							replyValues[2].type = Dart_CObject_kNull;
						}

						Dart_PostCObject(rt->replyPort, &reply);
						// TODO: Add the shader as a resource!
					});
				}
				break;
			default:
				printf("Unknown message type!\n");
				abort();
		}
	}

}
