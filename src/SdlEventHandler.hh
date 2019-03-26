#ifndef    SDLEVENTHANDLER_HH
# define   SDLEVENTHANDLER_HH

# include <memory>
# include <vector>
# include <thread>
# include <mutex>
# include <core_utils/CoreObject.hh>
# include <sdl_engine/Engine.hh>
# include <sdl_engine/EventListener.hh>

namespace sdl {
  namespace app {

    class SdlEventHandler: public utils::CoreObject {
      public:

        explicit
        SdlEventHandler(const float& eventHandlingRate = 30.0f,
                        core::engine::EngineShPtr engine = nullptr,
                        const bool exitOnEscape = true,
                        const std::string& name = std::string("event_handler"));

        ~SdlEventHandler();

        void
        run();

        void
        stop();

        bool
        isRunning();

        void
        addListener(core::engine::EventListener* listener);

        void
        removeListener(core::engine::EventListener* listener);

      private:

        void
        lock();

        void
        unlock();

        void
        handleEvents();

        int
        processEvents();

        void
        processSingleEvent(const core::engine::EventShPtr event);

        void
        processKeyPressedEvent(const core::engine::KeyEvent& event);

        void
        processKeyReleasedEvent(const core::engine::KeyEvent& event);

        void
        processMouseMotionEvent(const core::engine::MouseEvent& event);

        void
        processMouseButtonPressedEvent(const core::engine::MouseEvent& event);

        void
        processMouseButtonReleasedEvent(const core::engine::MouseEvent& event);

        void
        processMouseWheelEvent(const core::engine::MouseEvent& event);

        void
        processQuitEvent(const core::engine::QuitEvent& event);

      private:

        float m_framerate;
        float m_frameDuration;
        bool m_exitOnEscape;

        core::engine::EngineShPtr m_engine;

        bool m_eventsRunning;
        std::mutex m_locker;
        std::shared_ptr<std::thread> m_executionThread;

        std::vector<core::engine::EventListener*> m_listeners;
        std::mutex m_listenersLocker;
    };

    using SdlEventHandlerShPtr = std::shared_ptr<SdlEventHandler>;
  }
}

# include "SdlEventHandler.hxx"

#endif    /* SDLEVENTHANDLER_HH */
