#pragma once

#include <dart_native_api.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

// Tiny thread that manages communications with a sf::RenderWindow

// Each render target associated with the RenderThread has a list of drawables paired with render
// states and a flag that says to dispatch rendering
// This includes the RenderWindow itself

class RenderThread;

class GenericRenderCommand {
public:
  virtual ~GenericRenderCommand() = default;
  virtual void render(RenderThread *thread) = 0;
  uint64_t frame = 0;
};

class GenericRenderCommandElement {
public:
  virtual ~GenericRenderCommandElement() = default;
  virtual std::unique_ptr<GenericRenderCommand> get(RenderThread *thread) = 0;

  void dirty() {
    _dirty.store(true);
  }

  bool isDirty() {
    return _dirty.load();
  }

  bool clean() {
    return _dirty.exchange(true);
  }

private:
  std::atomic<bool> _dirty {true};
};

class CommandBuffer {
public:
  std::vector<std::shared_ptr<GenericRenderCommandElement>> commands;

  void execute(RenderThread *thread);
};

// RenderResources are cleaned up when their reference count reaches one
// (the only reference is the RenderThread's Resource list)
class RenderResource {
public:
  virtual ~RenderResource() = default;
};

class RenderThread : public std::thread {
public:
  RenderThread(sf::VideoMode videoMode, sf::String title) :
    std::thread(std::mem_fn(&RenderThread::main), this, videoMode, title) {}

  void enqueue(std::function<void()> &&func) {
    {
      std::lock_guard<std::mutex> guard(messageMutex);
      messages.emplace_back(std::move(func));
    }
    messageCV.notify_one();
  }

  Dart_Port replyPort = ILLEGAL_PORT;
  std::vector<CommandBuffer> commands;
  std::unordered_map<std::shared_ptr<GenericRenderCommandElement>, std::unique_ptr<GenericRenderCommand>> commandMap;
  uint64_t currentFrame = 0;
  std::shared_ptr<class Shader> shader;
  sf::RenderWindow *window;
  bool render = false;
  bool stop = false;
private:

  void main(sf::VideoMode videoMode, sf::String title);
  std::deque<std::function<void()>> messages;
  std::mutex messageMutex;
  std::condition_variable messageCV;
};

// You CANNOT modify data while the RenderCommandElement is dirty
template <typename Data, typename Command>
class RenderCommandElement : public GenericRenderCommandElement {
public:
  Data data;

  virtual ~RenderCommandElement() {}

  virtual std::unique_ptr<GenericRenderCommand> get(RenderThread *thread) {
    return std::unique_ptr<GenericRenderCommand>(make());
  }

protected:
  Command *make() {
    return new Command(&data, this);
  }
};

// Render commands are deleted when their data is nullptr
template <typename Data>
class RenderCommand : public GenericRenderCommand {
  using Self = RenderCommand<Data>;
  using Element = RenderCommandElement<Data, Self>;
public:
  RenderCommand(Data *data, GenericRenderCommandElement *element) : dataRef(data), element(element) {}
  virtual ~RenderCommand() = default;

  virtual void render(RenderThread *thread) {
    bool justUpdated;
    if (justUpdated = element->clean()) {
      data = *dataRef; // Copy
    }

    doRender(thread, justUpdated);
  }

  bool isValid() {return data;}
protected:
  Data data;
  virtual void doRender(RenderThread *thread, bool justUpdated) = 0;
private:
  Data *dataRef;
  GenericRenderCommandElement *element;
};
