
# include "SdlApplication.hh"
# include <sdl_engine/SdlEngine.hh>

namespace sdl {
  namespace app {

    SdlApplication::SdlApplication(const std::string& name,
                                   const std::string& title,
                                   const std::string& icon,
                                   const utils::Sizei& size,
                                   const float& framerate,
                                   const float& eventFramerate):
      sdl::core::EventListener(name, EventListener::Interaction::FullInteraction),

      m_title(title),

      m_framerate(std::max(0.1f, framerate)),
      m_frameDuration(1000.0f / m_framerate),

      m_locker(),
      m_renderingRunning(false),

      m_eventsHandler(eventFramerate, true),

      m_widgetsLocker(),
      m_widgets(),

      m_engine(nullptr),
      m_window(nullptr),
      m_canvas(nullptr)
    {
      setService("app");

      create(size);
      setIcon(icon);
      m_eventsHandler.addListener(this);
    }

    void
    SdlApplication::create(const utils::Sizei& size) {
      // Create the engine to use to perform rendering.
      log(std::string("Creating engine"));
      m_engine = std::make_shared<core::engine::SdlEngine>();

      // Use the engine to create the window.
      log(std::string("Creating window"));
      if (m_engine == nullptr) {
        log(std::string("WHAT?"));
      }
      const core::engine::Window::UUID uuid = m_engine->createWindow(size, getTitle());
      m_window = std::make_shared<core::engine::Window::UUID>(uuid);

      // Set it as the active window.
      log(std::string("Creating active window"));
      m_engine->setActiveWindow(*m_window);

      // Create a basic canvas which will be used as basis for the rendering.
      // TODO.
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
