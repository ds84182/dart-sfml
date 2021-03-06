#include "graphics.hpp"
#include "render_thread.hpp"
#include "shader.hpp"
#include "vertex_array.hpp"
#include "vertex_buffer.hpp"
#include "index_buffer.hpp"

#include "command/bind_shader.hpp"
#include "command/bind_vertex_array.hpp"
#include "command/clear.hpp"
#include "command/draw_arrays.hpp"
#include "command/draw_elements.hpp"
#include "command/capability.hpp"
#include "command/set_uniform.hpp"

#include <algorithm>
#include <cstring>

#include "dartutils.hpp"
#include "dart_native_api.h"
#include <SFML/Graphics.hpp>

namespace Graphics {

	// RenderContext

	static Dart_Port renderThreadPort = ILLEGAL_PORT;

	void RenderThreadMessageHandler(Dart_Port dest_port_id, Dart_CObject *message);

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

	// Command

	static void Command_dirty(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto elem = *args[0].getField("_ptr").asPointer<std::shared_ptr<GenericRenderCommandElement>>();
		elem->dirty();
	}

	// ClearCommand

	static void _ClearCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		ClearCommandElement *command = new ClearCommandElement();
		auto spp = new std::shared_ptr<ClearCommandElement>(command);

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(ClearCommandElement), MakeDeleter(spp));
	}

	static void _ClearCommand_setColor(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto elem = *args[0].getField("_ptr").asPointer<std::shared_ptr<ClearCommandElement>>();
		DartTypedData color = args[1];
		memcpy(&elem->data.color, color, sizeof(float)*4);
	}

	static void _ClearCommand_setMask(Dart_NativeArguments _args) {
		DartArgs args = _args;

		uint64_t mask = args[1];

		GLbitfield maskGL = 0;

		if (mask & 1) {
			maskGL |= GL_COLOR_BUFFER_BIT;
		}
		if (mask & 2) {
			maskGL |= GL_DEPTH_BUFFER_BIT;
		}
		if (mask & 4) {
			maskGL |= GL_STENCIL_BUFFER_BIT;
		}

		auto elem = *args[0].getField("_ptr").asPointer<std::shared_ptr<ClearCommandElement>>();
		elem->data.mask = maskGL;
	}

	// Shader

	static void _Shader(Dart_NativeArguments _args) {
		DartArgs args = _args;

		Shader *shader = new Shader();
		auto spp = new std::shared_ptr<Shader>(shader);

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(Shader), MakeDeleter(spp));
	}

	// BindShaderCommand

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

	// VertexArray

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

	// VertexBuffer

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

	// IndexBuffer

	enum class IndexBufferElementType {
		Int, Short, Byte
	};

	static void _IndexBuffer(Dart_NativeArguments _args) {
		DartArgs args = _args;

		IndexBuffer *ibo = new IndexBuffer();
		auto spp = new std::shared_ptr<IndexBuffer>(ibo);

		char *dataCopy;
		size_t size;
		{
			DartTypedData typedData = args[2];
			dataCopy = new char[typedData.size];
			size = typedData.size;
			memcpy(dataCopy, reinterpret_cast<char*>(typedData.data), typedData.size);
		}

		IndexBufferElementType type = static_cast<IndexBufferElementType>(args[3].asUInt());
		GLenum typeEnum;
		switch (type) {
			case IndexBufferElementType::Int:
				typeEnum = GL_UNSIGNED_INT;
				break;
			case IndexBufferElementType::Short:
				typeEnum = GL_UNSIGNED_SHORT;
				break;
			case IndexBufferElementType::Byte:
				typeEnum = GL_UNSIGNED_BYTE;
				break;
			default:
				printf("???\n");
				abort();
		}

		BufferUsage usage = static_cast<BufferUsage>(args[4].asUInt());
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
				printf("???\n");
				abort();
		}

		RenderThread *rt = args[1].asPointer<RenderThread>();

		rt->enqueue([=] {
			ibo->init(dataCopy, size, typeEnum, usageEnum);
			delete dataCopy;
			// TODO: Give resource to RenderThread
		});

		args[0].setField("_ptr", spp);
		GCHandle(args[0], sizeof(IndexBuffer), MakeDeleter(spp));
	}

	// BindVertexArrayCommand

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

	// DrawArraysCommand

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

	// DrawElementsCommand

	static void _DrawElementsCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		DrawElementsCommandElement *command = new DrawElementsCommandElement();
		auto spp = new std::shared_ptr<DrawElementsCommandElement>(command);

		auto object = args[0];
		object.setField("_ptr", spp);
		GCHandle(object, sizeof(DrawElementsCommandElement), MakeDeleter(spp));
	}

	static void _DrawElementsCommand_setData(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto spp = *args[0].getField("_ptr").asPointer<std::shared_ptr<DrawElementsCommandElement>>();

		spp->data.ibo = *args[1].asPointer<std::shared_ptr<IndexBuffer>>();

		auto mode = static_cast<PrimitiveMode>(args[2].asUInt());
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
		spp->data.first = args[3].asInt();
		spp->data.count = args[4].asUInt();
	}

	// CapabilityCommand

	static void _CapabilityCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		CapabilityCommandElement *command = new CapabilityCommandElement();
		auto spp = new std::shared_ptr<CapabilityCommandElement>(command);
		command->data.enable = args[1];

		auto object = args[0];
		object.setField("_ptr", spp);
		GCHandle(object, sizeof(CapabilityCommandElement), MakeDeleter(spp));
	}

	enum class RenderCapability {
		Blend,
	  CullFace,
	  DepthTest,
	  PolygonOffset,
	  ScissorTest,
	  StencilTest
	};

	static void _CapabilityCommand_set(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto spp = *args[0].getField("_ptr").asPointer<std::shared_ptr<CapabilityCommandElement>>();
		auto mode = static_cast<RenderCapability>(args[1].asUInt());

		GLenum capEnum;

		switch (mode) {
			case RenderCapability::Blend:
				capEnum = GL_BLEND;
				break;
			case RenderCapability::CullFace:
				capEnum = GL_CULL_FACE;
				break;
			case RenderCapability::DepthTest:
				capEnum = GL_DEPTH_TEST;
				break;
			case RenderCapability::PolygonOffset:
				capEnum = GL_POLYGON_OFFSET_FILL;
				break;
			case RenderCapability::ScissorTest:
				capEnum = GL_SCISSOR_TEST;
				break;
			case RenderCapability::StencilTest:
				capEnum = GL_STENCIL_TEST;
				break;
			default:
				printf("???\n");
				abort();
		}

		spp->data.cap = capEnum;
	}

	// SetUniformCommand

	static void _SetUniformCommand(Dart_NativeArguments _args) {
		DartArgs args = _args;

		SetUniformCommandElement *command = new SetUniformCommandElement();
		auto spp = new std::shared_ptr<SetUniformCommandElement>(command);

		auto object = args[0];
		object.setField("_ptr", spp);
		GCHandle(object, sizeof(SetUniformCommandElement), MakeDeleter(spp));
	}

	static void _SetUniformCommand_update(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto spp = *args[0].getField("_ptr").asPointer<std::shared_ptr<SetUniformCommandElement>>();
		std::string name = args[1];
		auto type = static_cast<UniformType>(args[2].asUInt());
		auto data = args[3];
		auto count = args[4].asUInt();

		DartTypedData typedData = data;
		typedData.copyInto(spp->data.data);
		typedData.release();

		spp->data.name = name;
		spp->data.type = type;

		spp->data.count = count;
	}

	// CommandBuffer

	static void _CommandBuffer(Dart_NativeArguments _args) {
		DartArgs args = _args;

		CommandBuffer *cmdbuf = new CommandBuffer();

		auto object = args[0];
		object.setField("_ptr", cmdbuf);
		GCHandle(object, sizeof(CommandBuffer), MakeDeleter(cmdbuf));
	}

	static void _CommandBuffer_update(Dart_NativeArguments _args) {
		DartArgs args = _args;

		auto cmdbuf = args[0].getField("_ptr").asPointer<CommandBuffer>();
		DartList list = args[1];

		cmdbuf->commands.clear();
		cmdbuf->commands.reserve(list.length());
		for (intptr_t i=0; i<list.length(); i++) {
			auto p = list.get(i).getField("_ptr");
			cmdbuf->commands.emplace_back(*p.asPointer<std::shared_ptr<GenericRenderCommandElement>>());
		}
	}

	FunctionMap functions = {
		{"RenderContext", &RenderContext},

		{"Command::dirty", &Command_dirty},

		{"BindShaderCommand", &_BindShaderCommand},
		{"BindShaderCommand::setShader", &_BindShaderCommand_setShader},

		{"ClearCommand", &_ClearCommand},
		{"ClearCommand::setColor", &_ClearCommand_setColor},
		{"ClearCommand::setMask", &_ClearCommand_setMask},

		{"BindVertexArrayCommand", &_BindVertexArrayCommand},
		{"BindVertexArrayCommand::setVertexArray", &_BindVertexArrayCommand_setVertexArray},

		{"DrawArraysCommand", &_DrawArraysCommand},
		{"DrawArraysCommand::setData", &_DrawArraysCommand_setData},

		{"DrawElementsCommand", &_DrawElementsCommand},
		{"DrawElementsCommand::setData", &_DrawElementsCommand_setData},

		{"CapabilityCommand", &_CapabilityCommand},
		{"CapabilityCommand::set", &_CapabilityCommand_set},

		{"SetUniformCommand", &_SetUniformCommand},
		{"SetUniformCommand::update", &_SetUniformCommand_update},

		{"Shader", &_Shader},
		{"VertexArray", &_VertexArray},
		{"VertexArray::enableAndBind", &_VertexArray_enableAndBind},
		{"VertexBuffer", &_VertexBuffer},
		// TODO: {"VertexBuffer::update", &_VertexBuffer_update},
		{"IndexBuffer", &_IndexBuffer},
		// TODO: {"IndexBuffer::update", &_IndexBuffer_update},
		{"CommandBuffer", &_CommandBuffer},
		{"CommandBuffer::update", &_CommandBuffer_update},
	};

}
