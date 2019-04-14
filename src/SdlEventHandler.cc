
# include "SdlEventHandler.hh"
# include <core_utils/CoreWrapper.hh>
# include <sdl_engine/KeyEvent.hh>
# include <sdl_engine/QuitEvent.hh>

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
      // This function basically just transmit the `event` to all the registered
      // listeners.
      // We only have one special case which is when the `Escape` key is pressed
      // and the internal `m_exitOnEscape` status is ticked: in this case we want
      // to bypass the regular event processing and allow the creation of a quit
      // event and process it as usual.

      // Check for key released.
      if (event->getType() == core::engine::Event::Type::KeyRelease) {
        // Check the key which was pressed.
        std::shared_ptr<core::engine::KeyEvent> keyEvent = std::dynamic_pointer_cast<core::engine::KeyEvent>(event);

        // If the conversion was successful and that it corresponds to the `Escape`
        // key, we need to check the internal status to determine the next action.
        if (keyEvent != nullptr && keyEvent->isEscape() && m_exitOnEscape) {
          // Replace the input event with a quit event.
          dispatchEvent(std::make_shared<core::engine::QuitEvent>());

          // All is well.
          return;
        }

        // Continue to standard processing.
      }

      // Transmit the event to all listeners.
      dispatchEvent(event);
    }

  }
}
