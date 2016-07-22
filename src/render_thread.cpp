#include "render_thread.hpp"

void RenderCommandList::execute(RenderThread *thread) {
  for (auto command : commands) {
    if (!thread->commandMap.count(command)) {
      thread->commandMap[command] = command->get(thread);
    }

    auto &realCommand = thread->commandMap[command];
    realCommand->render(target, thread);
    realCommand->frame = thread->currentFrame;
  }
}

void RenderThread::main(sf::VideoMode videoMode, sf::String title) {
  sf::RenderWindow window(videoMode, title, sf::Style::Default, sf::ContextSettings(0, 0, 0, 3, 2));
  window.setFramerateLimit(0); // It is up to Dart to tell us when to present
  this->window = &window;

  while (!stop) {
    sf::Event event;
    while (window.pollEvent(event));

    if (render && replyPort != ILLEGAL_PORT) {
      // Send the ready message so that Dart can work on the next command list
      Dart_PostInteger(replyPort, 1);

      currentFrame++;
      for (auto &commandList : commands) {
        commandList.execute(this);
      }
      for (auto &pair : commandMap) {
        auto frameDiff = currentFrame-pair.second->frame;
        if (frameDiff > 3) {
          printf("Collect %lu %p\n", frameDiff, pair.second.get());
        }
      }
      window.display();
      render = false;
    }

    std::unique_lock<std::mutex> lock(messageMutex);

    if (messages.empty()) {
      messageCV.wait(lock, [this]{return !messages.empty();});
    }

    for (auto &message : messages) {
      message();
    }
    messages.clear();
  }
}
