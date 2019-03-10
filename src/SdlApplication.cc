
# include "SdlApplication.hh"

namespace sdl {
  namespace app {

    SdlApplication::SdlApplication(const std::string& name,
                                   const std::string& title,
                                   const std::string& icon,
                                   const utils::maths::Size<int>& size,
                                   const float& framerate,
                                   const float& eventFramerate,
                                   const bool exitOnEscape,
                                   utils::core::LoggerShPtr logger):
      sdl::core::EventListener(EventListener::Interaction::FullInteraction),
      m_name(name),
      m_title(title),
      m_icon(),
      m_framerate(std::max(0.1f, framerate)),
      m_frameDuration(1000.0f / m_framerate),
      m_window(nullptr),
      m_renderer(nullptr),
      m_eventsHandler(eventFramerate, exitOnEscape),

      m_renderingRunning(false),
      m_locker(),

      m_widgets(),
      m_widgetsLocker(),

      m_logger(logger)
    {
      createWindow(size);
      setIcon(icon);
      m_eventsHandler.addListener(this);

      // Initialize the logger.
      // TODO: Refactor the logger to use a so called `LoggingDevice` so that all
      // loggers share the same device and we can use several loggers with various names.
      if (logger != nullptr) {
        m_logger->setName(getName());
      }
    }

    void
    SdlApplication::createWindow(const utils::maths::Size<int>& size) {
      // Initialize sdl lib.
      initializeSdlLib();

      // Create the main renderer.
      m_window = SDL_CreateWindow(
        getTitle().c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        size.w(),
        size.h(),
        SDL_WINDOW_SHOWN
      );
      if (m_window == nullptr) {
        throw AppException(std::string("Could not create main window \"") + getTitle() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED
      );
      if (m_renderer == nullptr) {
        throw AppException(std::string("Could not create main renderer for \"") + getTitle() + "\" (err: \"" + SDL_GetError() + "\")");
      }
    }

    void
    SdlApplication::performRendering() {
      // Start the event handling.
      lock();
      m_renderingRunning = true;
      unlock();

      bool stillRunning = true;
      while (stillRunning) {
        lock();
        stillRunning = m_renderingRunning;
        unlock();

        if (!stillRunning) {
          break;
        }

        render();
      }
    }

  }
}
