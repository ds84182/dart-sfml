#include "graphics.hpp"
#include "render_thread.hpp"
#include "shader.hpp"
#include "vertex_array.hpp"
#include "vertex_buffer.hpp"

#include "command/bind_shader.hpp"
#include "command/bind_vertex_array.hpp"
#include "command/clear.hpp"
#include "command/draw_arrays.hpp"

#include <algorithm>
#include <cstring>

#include "dartutils.hpp"
#include "dart_native_api.h"
#include <SFML/Graphics.hpp>

namespace Graphics {

	// RenderContext

	Dart_Port renderThreadPort = ILLEGAL_PORT;

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

	enum class UniformType {
		Float, Float2, Float3, Float4,
	  Int, Int2, Int3, Int4,
	  Matrix2, Matrix3, Matrix4
	};

	enum class RenderThreadMessageType {
		Init, // To Render Thread
		StartFrame, // To Dart Thread
		RenderFrame, // To Render Thread
		SetCommands, // To Render Thread (Format of [RenderTargetPtr, [CommandPtr, ...], ...])
		NewShader, // To Render Thread (ptr, vertexShaderStr, fragmentShaderStr, attributeLayout). We get a reply back with the ptr and the status
		ShaderResult, // To Dart Thread (ptr, errorLog as string if failed or null)
		SendUniform, // To Render Thread([ptr, uniformName, uniformType, data, count])
	};

	static void RenderThreadMessageHandler(Dart_Port dest_port_id, Dart_CObject *message) {
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
					std::vector<RenderCommandList> commandLists;
					Dart_CObject *list = GetArray(message, 2);
					for (int i=0; i<list->value.as_array.length; i += 2) {
						auto rtarget = reinterpret_cast<sf::RenderTarget*>(CheckInt(GetArray(list, i)));
						std::vector<std::shared_ptr<GenericRenderCommandElement>> commands;
						if (reinterpret_cast<intptr_t>(rtarget) == reinterpret_cast<intptr_t>(rt)) {
							rtarget = nullptr;
						}
						Dart_CObject *commandList = GetArray(list, i+1);
						for (int j=0; j<commandList->value.as_array.length; j++) {
							commands.push_back(*reinterpret_cast<std::shared_ptr<GenericRenderCommandElement>*>(CheckInt(GetArray(commandList, j))));
						}
						commandLists.emplace_back(rtarget, std::move(commands));
					}
					rt->enqueue([rt, commandLists]{
						rt->commands = std::move(commandLists);
						for (auto &commandList : rt->commands) {
							if (commandList.target == nullptr) commandList.target = rt->window;
						}
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
			case RenderThreadMessageType::SendUniform:
				CheckArrayLength(message, 7); // shaderptr, name, type, data, count
				{
					int64_t shaderPointer = CheckInt(GetArray(message, 2));
					std::shared_ptr<Shader> shader = *reinterpret_cast<std::shared_ptr<Shader>*>(shaderPointer);
					std::string name = CheckString(GetArray(message, 3));
					UniformType type = static_cast<UniformType>(CheckInt(GetArray(message, 4)));
					Dart_CObject *dataObject = GetArray(message, 5);
					int64_t count = CheckInt(GetArray(message, 6));

					// Copy typed data to a new buffer
					size_t length = dataObject->value.as_typed_data.length;
					char *_data = new char[length];
					SneekyPointer data = _data;
					memcpy(_data, dataObject->value.as_typed_data.values, length);

					rt->enqueue([=] {
						shader->bind();
						GLint location = shader->getUniformLocation(name);
						switch (type) {
							case UniformType::Float:
								glUniform1fv(location, count, data);
								break;
							case UniformType::Float2:
								glUniform2fv(location, count, data);
								break;
							case UniformType::Float3:
								glUniform3fv(location, count, data);
								break;
							case UniformType::Float4:
								glUniform4fv(location, count, data);
								break;

							case UniformType::Int:
								glUniform1iv(location, count, data);
								break;
							case UniformType::Int2:
								glUniform2iv(location, count, data);
								break;
							case UniformType::Int3:
								glUniform3iv(location, count, data);
								break;
							case UniformType::Int4:
								glUniform4iv(location, count, data);
								break;

							case UniformType::Matrix2:
								glUniformMatrix2fv(location, count, false, data);
								break;
							case UniformType::Matrix3:
								glUniformMatrix3fv(location, count, false, data);
								break;
							case UniformType::Matrix4:
								glUniformMatrix4fv(location, count, false, data);
								break;
						}
						delete _data;
					});
				}
				break;
			default:
				printf("Unknown message type!\n");
				abort();
		}
	}

	static void RenderContext_destroy(RenderThread *rt) {
		rt->enqueue([rt]{
			rt->stop = true;
		});
		rt->join();
		delete rt;
	}

	static void RenderContext(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto videoMode = args[1];

		uint64_t width, height, bitsPerPixel;
		std::string title;
		width = videoMode.getField("width");
		height = videoMode.getField("height");
		bitsPerPixel = videoMode.getField("bitsPerPixel");
		title = args[2].asString();

		if (renderThreadPort == ILLEGAL_PORT) {
			renderThreadPort = Dart_NewNativePort("RenderThreadServicePort", &RenderThreadMessageHandler, true);
		}

		DartHandle handle = Dart_NewSendPort(renderThreadPort);

		RenderThread *window = new RenderThread(sf::VideoMode(width, height, bitsPerPixel), title);

		DartHandle self = args[0];

		self.setField("_ptr", window);
		GCHandle(self, sizeof(RenderThread)+sizeof(sf::RenderWindow), std::bind(&RenderContext_destroy, window));
		self.setField("_port", handle);
	}

	static void Command_dirty(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto elem = *args[0].getField("_ptr").asPointer<std::shared_ptr<GenericRenderCommandElement>>();
		elem->dirty();
	}

	template <typename T>
	static std::function<void()> MakeDeleter(T *obj) {
		return [=]() {delete obj;};
	}

	static void _ClearCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		uint64_t color = args[1];

		ClearCommandElement *command = new ClearCommandElement();
		command->data.color = sf::Color(color);
		auto spp = new std::shared_ptr<ClearCommandElement>(command);

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(ClearCommandElement), MakeDeleter(spp));
	}

	static void _ClearCommand_setColor(Dart_NativeArguments _args) {
		DartArgs args = _args;

		uint64_t color = args[1];

		auto elem = *args[0].getField("_ptr").asPointer<std::shared_ptr<ClearCommandElement>>();
		elem->data.color = sf::Color(color);
	}

	static void _Shader(Dart_NativeArguments _args) {
		DartArgs args = _args;

		Shader *shader = new Shader();
		auto spp = new std::shared_ptr<Shader>(shader);

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(Shader), MakeDeleter(spp));
	}

	static void _BindShaderCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		BindShaderCommandElement *command = new BindShaderCommandElement();
		auto spp = new std::shared_ptr<BindShaderCommandElement>(command);

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(BindShaderCommandElement), MakeDeleter(spp));
	}

	static void _BindShaderCommand_setShader(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto elem = *args[0].getField("_ptr").asPointer<std::shared_ptr<BindShaderCommandElement>>();
		elem->data.shader = *args[1].asPointer<std::shared_ptr<Shader>>();
	}

	static void _VertexArray(Dart_NativeArguments _args) {
		DartArgs args = _args;

		VertexArray *vao = new VertexArray();
		auto spp = new std::shared_ptr<VertexArray>(vao);

		RenderThread *rt = args[1].asPointer<RenderThread>();
		rt->enqueue([=] {
			vao->init();
			// TODO: Give resource to RenderThread
		});

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(VertexArray), MakeDeleter(spp));
	}

	enum class BufferUsage {
		StreamDraw, StreamRead, StreamCopy,
	  StaticDraw, StaticRead, StaticCopy,
	  DynamicDraw, DynamicRead, DynamicCopy
	};

	static void _VertexBuffer(Dart_NativeArguments _args) {
		DartArgs args = _args;

		VertexBuffer *vbo = new VertexBuffer();
		auto spp = new std::shared_ptr<VertexBuffer>(vbo);

		char *dataCopy;
		size_t size;
		{
			DartTypedData typedData = args[2];
			dataCopy = new char[typedData.size];
			size = typedData.size;
			memcpy(dataCopy, reinterpret_cast<char*>(typedData.data), typedData.size);
		}

		uint64_t usageInt = args[3];
		BufferUsage usage = static_cast<BufferUsage>(usageInt);
		GLenum usageEnum;
		switch (usage) {
			case BufferUsage::StreamDraw:
				usageEnum = GL_STREAM_DRAW;
				break;
			case BufferUsage::StreamRead:
				usageEnum = GL_STREAM_READ;
				break;
			case BufferUsage::StreamCopy:
				usageEnum = GL_STREAM_COPY;
				break;
			case BufferUsage::StaticDraw:
				usageEnum = GL_STATIC_DRAW;
				break;
			case BufferUsage::StaticRead:
				usageEnum = GL_STATIC_READ;
				break;
			case BufferUsage::StaticCopy:
				usageEnum = GL_STATIC_COPY;
				break;
			case BufferUsage::DynamicDraw:
				usageEnum = GL_DYNAMIC_DRAW;
				break;
			case BufferUsage::DynamicRead:
				usageEnum = GL_DYNAMIC_READ;
				break;
			case BufferUsage::DynamicCopy:
				usageEnum = GL_DYNAMIC_COPY;
				break;
			default:
				printf("??? %lu\n", usageInt);
				abort();
		}

		RenderThread *rt = args[1].asPointer<RenderThread>();

		rt->enqueue([=] {
			vbo->init(dataCopy, size, usageEnum);
			delete dataCopy;
			// TODO: Give resource to RenderThread
		});

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(VertexBuffer), MakeDeleter(spp));
	}

	enum class AttributeType {
	  Byte, UnsignedByte, Short, UnsignedShort,
	  Int, UnsignedInt, Float
	};

	static void _VertexArray_enableAndBind(Dart_NativeArguments _args) {
		DartArgs args = _args;
		auto vao = *args[0].getField("_ptr").asPointer<std::shared_ptr<VertexArray>>();
		auto rt = args[1].asPointer<RenderThread>();
		GLuint index = args[2].asUInt();
		auto vbo = *args[3].asPointer<std::shared_ptr<VertexBuffer>>();
		AttributeType type = static_cast<AttributeType>(args[4].asUInt());
		GLint size = args[5].asInt();
		bool normalized = args[6].asBool();
		GLsizei stride = args[7].asInt();
		GLintptr offset = args[8].asInt();

		GLenum typeEnum;
		switch (type) {
			case AttributeType::Byte:
				typeEnum = GL_BYTE;
				break;
			case AttributeType::UnsignedByte:
				typeEnum = GL_UNSIGNED_BYTE;
				break;
			case AttributeType::Short:
				typeEnum = GL_SHORT;
				break;
			case AttributeType::UnsignedShort:
				typeEnum = GL_UNSIGNED_SHORT;
				break;
			case AttributeType::Int:
				typeEnum = GL_INT;
				break;
			case AttributeType::UnsignedInt:
				typeEnum = GL_UNSIGNED_INT;
				break;
			case AttributeType::Float:
				typeEnum = GL_FLOAT;
				break;
			default:
				printf("???\n");
				abort();
		}

		rt->enqueue([=] {
			glBindVertexArray(vao->id);
			glBindBuffer(GL_ARRAY_BUFFER, vbo->id);

			glVertexAttribPointer(index, size, typeEnum, normalized, stride, reinterpret_cast<void*>(offset));
			glEnableVertexAttribArray(index);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		});
	}

	static void _BindVertexArrayCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		BindVertexArrayCommandElement *command = new BindVertexArrayCommandElement();
		auto spp = new std::shared_ptr<BindVertexArrayCommandElement>(command);

		auto object = args[0];
		object.setField("_ptr", spp);
		GCHandle(object, sizeof(BindVertexArrayCommandElement), MakeDeleter(spp));
	}

	static void _BindVertexArrayCommand_setVertexArray(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto spp = *args[0].getField("_ptr").asPointer<std::shared_ptr<BindVertexArrayCommandElement>>();
		auto vao = *args[1].asPointer<std::shared_ptr<VertexArray>>();

		spp->data.vao = vao;
	}

	static void _DrawArraysCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		DrawArraysCommandElement *command = new DrawArraysCommandElement();
		auto spp = new std::shared_ptr<DrawArraysCommandElement>(command);

		auto object = args[0];
		object.setField("_ptr", spp);
		GCHandle(object, sizeof(DrawArraysCommandElement), MakeDeleter(spp));
	}

	enum class PrimitiveMode {
		Points, LineStrip, LineLoop, Lines,
	  TriangleStrip, TriangleFan, Triangles
	};

	static void _DrawArraysCommand_setData(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto spp = *args[0].getField("_ptr").asPointer<std::shared_ptr<DrawArraysCommandElement>>();
		auto mode = static_cast<PrimitiveMode>(args[1].asUInt());

		GLenum modeEnum;

		switch (mode) {
			case PrimitiveMode::Points:
				modeEnum = GL_POINTS;
				break;
			case PrimitiveMode::LineStrip:
				modeEnum = GL_LINE_STRIP;
				break;
			case PrimitiveMode::LineLoop:
				modeEnum = GL_LINE_LOOP;
				break;
			case PrimitiveMode::Lines:
				modeEnum = GL_LINES;
				break;
			case PrimitiveMode::TriangleStrip:
				modeEnum = GL_TRIANGLE_STRIP;
				break;
			case PrimitiveMode::TriangleFan:
				modeEnum = GL_TRIANGLE_FAN;
				break;
			case PrimitiveMode::Triangles:
				modeEnum = GL_TRIANGLES;
				break;
			default:
				printf("???\n");
				abort();
		}

		spp->data.mode = modeEnum;
		spp->data.first = args[2].asInt();
		spp->data.count = args[3].asUInt();
	}

	FunctionMap functions = {
		{"RenderContext", &RenderContext},

		{"Command::dirty", &Command_dirty},

		{"BindShaderCommand", &_BindShaderCommand},
		{"BindShaderCommand::setShader", &_BindShaderCommand_setShader},

		{"ClearCommand", &_ClearCommand},
		{"ClearCommand::setColor", &_ClearCommand_setColor},

		{"BindVertexArrayCommand", &_BindVertexArrayCommand},
		{"BindVertexArrayCommand::setVertexArray", &_BindVertexArrayCommand_setVertexArray},

		{"DrawArraysCommand", &_DrawArraysCommand},
		{"DrawArraysCommand::setData", &_DrawArraysCommand_setData},

		{"Shader", &_Shader},
		{"VertexArray", &_VertexArray},
		{"VertexArray::enableAndBind", &_VertexArray_enableAndBind},
		{"VertexBuffer", &_VertexBuffer},
	};

}
