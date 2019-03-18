
# include "SdlApplication.hh"
# include <sdl_engine/EngineLocator.hh>

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
      m_icon(),
      m_framerate(std::max(0.1f, framerate)),
      m_frameDuration(1000.0f / m_framerate),
      m_window(nullptr),
      m_eventsHandler(eventFramerate, true),

      m_renderingRunning(false),
      m_locker(),

      m_widgets(),
      m_widgetsLocker()
    {
      setService("app");

      createWindow(size);
      setIcon(icon);
      m_eventsHandler.addListener(this);
    }

    void
    SdlApplication::createWindow(const utils::Sizei& size) {
      // Use the engine to create the window.
      const core::engine::Window::UUID uuid = core::engine::EngineLocator::getEngine().createWindow(
        size,
        getTitle()
      );
      m_window = std::make_shared<core::engine::Window::UUID>(uuid);

      // Set it as the active window.
      core::engine::EngineLocator::getEngine().setActiveWindow(*m_window);
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
