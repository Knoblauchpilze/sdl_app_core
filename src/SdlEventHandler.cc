
# include "SdlEventHandler.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace app {

    SdlEventHandler::SdlEventHandler(const float& eventHandlingRate,
                                     core::engine::EngineShPtr engine,
                                     const bool exitOnEscape,
                                     const std::string& name):
      utils::CoreObject(name),
      m_framerate(std::max(0.1f, eventHandlingRate)),
      m_frameDuration(1000.0f / m_framerate),
      m_exitOnEscape(exitOnEscape),

      m_engine(engine),

      m_eventsRunning(false),
      m_locker(),
      m_executionThread(nullptr),

      m_listeners(),
      m_listenersLocker()
    {
      setService("events");

      if (m_engine == nullptr) {
        error(std::string("Cannot create event handler with null engine"));
      }
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

        // Process events.
        int processingDuration = processEvents();

        // Check whether the rendering time is compatible with the desired framerate.
        if (1.0f * processingDuration > m_frameDuration) {
          // Log this problem.
          log(
            std::string("Event handling took ") + std::to_string(processingDuration) + "ms " +
            "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
            " authorized to maintain " + std::to_string(m_framerate) + "fps",
            utils::Level::Warning
          );

          // Move on to the next frame.
          return;
        }

        // Sleep for the remaining time to complete a frame if there's enough time left.
        const unsigned int remainingDuration = m_frameDuration - processingDuration;
        if (remainingDuration > 3u) {
          std::this_thread::sleep_for(std::chrono::milliseconds(remainingDuration));
        }
      }
    }

    int
    SdlEventHandler::processEvents() {
      // Poll events until we deplete the queue.
      bool eventsStillInQueue = true;

      // Start time measurement.
      auto start = std::chrono::steady_clock::now();

      while (eventsStillInQueue) {
        core::engine::EventShPtr event = m_engine->pollEvent(eventsStillInQueue);

        if (eventsStillInQueue && event != nullptr) {
          withSafetyNet(
            [&event, this]() {
              processSingleEvent(event);
            },
            std::string("process_single_event")
          );
        }
      }

      // Return the elapsed time.
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    }

    void
    SdlEventHandler::processSingleEvent(const core::engine::EventShPtr event) {
      // Check the event type.
      switch (event->getType()) {
        case core::engine::Event::Type::KeyPress:
          processKeyPressedEvent(*std::dynamic_pointer_cast<core::engine::KeyEvent>(event));
          break;
        case core::engine::Event::Type::KeyRelease:
          processKeyReleasedEvent(*std::dynamic_pointer_cast<core::engine::KeyEvent>(event));
          break;
        case core::engine::Event::Type::MouseMove:
          processMouseMotionEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(event));
          break;
        case core::engine::Event::Type::MouseButtonPress:
          processMouseButtonPressedEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(event));
          break;
        case core::engine::Event::Type::MouseButtonRelease:
          processMouseButtonReleasedEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(event));
          break;
        case core::engine::Event::Type::MouseWheel:
          processMouseWheelEvent(*std::dynamic_pointer_cast<core::engine::MouseEvent>(event));
          break;
        case core::engine::Event::Type::Quit:
          processQuitEvent(*std::dynamic_pointer_cast<core::engine::QuitEvent>(event));
          break;
        default:
          break;
      }
    }

  }
}
