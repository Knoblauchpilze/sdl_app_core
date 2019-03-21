
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
      m_window(),
      m_canvas()
    {
      setService("app");

      create(size);
      setIcon(icon);
      m_eventsHandler.addListener(this);
    }

    void
    SdlApplication::create(const utils::Sizei& size) {
      // Create the engine to use to perform rendering.
      m_engine = std::make_shared<core::engine::SdlEngine>();

      // Use the engine to create the window.
      m_window = m_engine->createWindow(size, getTitle());

      if (!m_window.valid()) {
        error(std::string("Could not create window with size " + size.toString()));
      }

      // Set it as the active window.
      m_engine->setActiveWindow(m_window);

      // Create a basic canvas which will be used as basis for the rendering.
      // TODO.
    }

    void
    SdlApplication::performRendering() {
      // Start the event handling.
      m_locker.lock();
      m_renderingRunning = true;
      m_locker.unlock();

      bool stillRunning = true;
      while (stillRunning) {
        m_locker.lock();
        stillRunning = m_renderingRunning;
        m_locker.unlock();

        if (!stillRunning) {
          break;
        }

        render();
      }
    }

    void
    SdlApplication::render() {
      auto start = std::chrono::steady_clock::now();

      renderWidgets();

      int renderingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

      if (1.0f * renderingDuration > m_frameDuration) {
        log(
          std::string("Frame took ") + std::to_string(renderingDuration) + "ms " +
          "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
          " authorized to maintain " + std::to_string(m_framerate) + "fps",
          utils::Level::Warning
        );
      }
      else {
        const unsigned int remainingDuration = m_frameDuration - renderingDuration;
        if (remainingDuration > 3u) {
          SDL_Delay(remainingDuration);
        }
      }
    }

    void
    SdlApplication::renderWidgets() {
      std::lock_guard<std::mutex> guard(m_widgetsLocker);

      // Clear screen content.
      m_engine->clearWindow(m_window);

      std::shared_ptr<core::engine::Engine> engine = m_engine;

      for (WidgetsMap::iterator widgetIt = m_widgets.begin() ;
          widgetIt != m_widgets.end() ;
          ++widgetIt)
      {
        sdl::core::SdlWidgetShPtr widget = widgetIt->second;

        // Draw this object (caching is handled by the object itself).
        withSafetyNet(
          [widget, engine]() {
            utils::Uuid texture = widget->draw();
            utils::Boxf render = widget->getRenderingArea();

            engine->drawTexture(
              texture,
              nullptr,
              &render
            );
          },
          std::string("draw_widget")
        );
      }

      // Display the changes.
      m_engine->renderWindow(m_window);
    }

  }
}
