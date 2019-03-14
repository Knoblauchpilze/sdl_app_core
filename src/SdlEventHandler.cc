
# include "SdlEventHandler.hh"

# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace app {

    SdlEventHandler::SdlEventHandler(const float& eventHandlingRate,
                                     const bool exitOnEscape,
                                     const std::string& name):
      utils::CoreObject(name),
      m_framerate(std::max(0.1f, eventHandlingRate)),
      m_frameDuration(1000.0f / m_framerate),
      m_exitOnEscape(exitOnEscape),

      m_eventsRunning(false),
      m_locker(),
      m_executionThread(nullptr),

      m_listeners(),
      m_listenersLocker()
    {
      setService("events");
    }

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

      const unsigned int handlingDuration = SDL_GetTicks() - startingHandlingTime;

      while (eventsStillInQueue) {
        eventsStillInQueue = SDL_PollEvent(nullptr) != 0;
        if (!eventsStillInQueue) {
          break;
        }

        eventsStillInQueue = SDL_PollEvent(&event);

        withSafetyNet(
          [&event, this]() {
            processSingleEvent(event);
          },
          std::string("process_single_event")
        );
      }

      if (handlingDuration > m_frameDuration) {
        log(
          std::string("Event handling took ") + std::to_string(handlingDuration) + "ms " +
          "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
          " authorized to maintain " + std::to_string(m_framerate) + "fps",
          utils::Level::Warning
        );
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
