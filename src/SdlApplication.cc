
# include "SdlApplication.hh"

namespace sdl {
  namespace core {

    SdlApplication::SdlApplication(const std::string& title,
                                   const std::string& icon,
                                   const int& width,
                                   const int& height,
                                   const float& framerate,
                                   const float& eventFramerate,
                                   const bool exitOnEscape):
      SdlEventListener(SdlEventListener::Interaction::FullInteraction),
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
      m_widgetsLocker()
    {
      createWindow(width, height);
      setIcon(icon);
      m_eventsHandler.addListener(this);
    }

    void
    SdlApplication::createWindow(const int& width, const int& height) {
      // Initialize sdl lib.
      initializeSdlLib();

      // Create the main renderer.
      m_window = SDL_CreateWindow(
        getTitle().c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
      );
      if (m_window == nullptr) {
        throw SdlException(std::string("Could not create main window \"") + getTitle() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED
      );
      if (m_renderer == nullptr) {
        throw SdlException(std::string("Could not create main renderer for \"") + getTitle() + "\" (err: \"" + SDL_GetError() + "\")");
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
