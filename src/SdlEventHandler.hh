#ifndef    SDLEVENTHANDLER_HH
# define   SDLEVENTHANDLER_HH

# include <memory>
# include <vector>
# include <thread>
# include <mutex>
# include <SDL2/SDL.h>

# include <sdl_core/EventListener.hh>

namespace sdl {
  namespace app {

    class SdlEventHandler {
      public:

        explicit
        SdlEventHandler(const float& eventHandlingRate = 30.0f,
                        const bool exitOnEscape = true);

        ~SdlEventHandler();

        void
        run();

        void
        stop();

        bool
        isRunning();

        void
        addListener(sdl::core::EventListener* listener);

        void
        removeListener(sdl::core::EventListener* listener);

      private:

        void
        lock();

        void
        unlock();

        void
        handleEvents();

        void
        processEvents();

        void
        processSingleEvent(const SDL_Event& event);

        void
        processKeyPressedEvent(const SDL_KeyboardEvent& event);

        void
        processKeyReleasedEvent(const SDL_KeyboardEvent& event);

        void
        processMouseMotionEvent(const SDL_MouseMotionEvent& event);

        void
        processMouseButtonPressedEvent(const SDL_MouseButtonEvent& event);

        void
        processMouseButtonReleasedEvent(const SDL_MouseButtonEvent& event);

        void
        processMouseWheelEvent(const SDL_MouseWheelEvent& event);

        void
        processQuitEvent(const SDL_QuitEvent& event);

      private:

        static const char* sk_serviceName;

        float m_framerate;
        float m_frameDuration;
        bool m_exitOnEscape;

        bool m_eventsRunning;
        std::mutex m_locker;
        std::shared_ptr<std::thread> m_executionThread;

        std::vector<sdl::core::EventListener*> m_listeners;
        std::mutex m_listenersLocker;
    };

    using SdlEventHandlerShPtr = std::shared_ptr<SdlEventHandler>;
  }
}

# include "SdlEventHandler.hxx"

#endif    /* SDLEVENTHANDLER_HH */
