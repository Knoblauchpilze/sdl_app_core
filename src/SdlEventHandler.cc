
# include <iostream>
# include "SdlEventHandler.hh"

namespace sdl {
  namespace core {

    SdlEventHandler::SdlEventHandler(const float& eventHandlingRate):
      m_framerate(std::max(0.1f, eventHandlingRate)),
      m_frameDuration(1000.0f / m_framerate),

      m_eventsRunning(false),
      m_locker(),
      m_executionThread(nullptr),

      m_listeners(),
      m_listenersLocker()
    {}

    SdlEventHandler::~SdlEventHandler() {
      stop();
    }

    void
    SdlEventHandler::handleEvents() {
      // Start the event handling.
      lock();
      m_eventsRunning = true;
      unlock();

      bool stillRunning = true;
      while (stillRunning) {
        lock();
        stillRunning = m_eventsRunning;
        unlock();

        if (!stillRunning) {
          break;
        }

        processEvents();
      }
    }

    void
    SdlEventHandler::processEvents() {
      // Poll events until we deplete the queue.
      SDL_Event event;
      bool eventsStillInQueue = true;

      const unsigned int startingHandlingTime = SDL_GetTicks();
      // std::cout << "[EVN] Performing events handling" << std::endl;

      const unsigned int handlingDuration = SDL_GetTicks() - startingHandlingTime;

      while (eventsStillInQueue) {
        eventsStillInQueue = SDL_PollEvent(nullptr) != 0;
        if (!eventsStillInQueue) {
          break;
        }

        eventsStillInQueue = SDL_PollEvent(&event);
        processSingleEvent(event);
      }

      if (handlingDuration > m_frameDuration) {
        std::cerr << "Event handling took " << handlingDuration << "ms which is greater than the " << m_frameDuration << "ms authorized to maintain " << m_framerate << "fps" << std::endl;
      }
      else {
        const unsigned int remainingDuration = m_frameDuration - handlingDuration;
        if (remainingDuration > 3u) {
          SDL_Delay(remainingDuration);
        }
      }
    }

    void
    SdlEventHandler::processSingleEvent(const SDL_Event& event) {
      // Check the event type.
      switch (event.type) {
        case SDL_KEYDOWN:
          processKeyPressedEvent(event.key);
          break;
        case SDL_KEYUP:
          processKeyReleasedEvent(event.key);
          break;
        case SDL_MOUSEMOTION:
          processMouseMotionEvent(event.motion);
          break;
        case SDL_MOUSEBUTTONDOWN:
          processMouseButtonPressedEvent(event.button);
          break;
        case SDL_MOUSEBUTTONUP:
          processMouseButtonReleasedEvent(event.button);
          break;
        case SDL_MOUSEWHEEL:
          processMouseWheelEvent(event.wheel);
          break;
        case SDL_QUIT:
          processQuitEvent(event.quit);
          break;
        default:
          break;
      }
    }

  }
}
