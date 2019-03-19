#ifndef    SDLAPPLICATION_HXX
# define   SDLAPPLICATION_HXX

# include "SdlApplication.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace app {

    inline
    SdlApplication::~SdlApplication() {
      if (m_eventsHandler.isRunning()) {
        m_eventsHandler.stop();
      }

      if (m_window != nullptr) {
        m_engine->destroyWindow(*m_window);
      }
    }

    inline
    const std::string&
    SdlApplication::getTitle() const noexcept {
      return m_title;
    }

    inline
    void
    SdlApplication::setIcon(const std::string& icon) {
      if (m_window == nullptr) {
        error(std::string("Could not set icon for invalid sdl window"));
      }

      // Load this icon.
      m_engine->setWindowIcon(*m_window, icon);
    }

    inline
    void
    SdlApplication::run() {
      // Start event handling.
      m_eventsHandler.run();

      // Start rendering.
      performRendering();
    }

    inline
    void
    SdlApplication::onQuitEvent(const SDL_QuitEvent& /*event*/) {
      std::lock_guard<std::mutex> guard(m_locker);
      m_renderingRunning = false;
    }

    inline
    void
    SdlApplication::addWidget(sdl::core::SdlWidgetShPtr widget) {
      if (widget == nullptr) {
        error(std::string("Cannot add null widget"));
      }

      std::lock_guard<std::mutex> guard(m_widgetsLocker);
      m_widgets[widget->getName()] = widget;
      m_eventsHandler.addListener(widget.get());
    }

    inline
    void
    SdlApplication::removeWidget(sdl::core::SdlWidgetShPtr widget) {
      if (widget == nullptr) {
        error(std::string("Cannot remove null widget"));
      }

      std::lock_guard<std::mutex> guard(m_widgetsLocker);
      m_widgets.erase(widget->getName());
      m_eventsHandler.removeListener(widget.get());
    }

    inline
    void
    SdlApplication::lock() {
      m_locker.lock();
    }

    inline
    void
    SdlApplication::unlock() {
      m_locker.unlock();
    }

    inline
    void
    SdlApplication::render() {
      const unsigned int startingRenderingTime = SDL_GetTicks();

      renderWidgets();

      const unsigned int renderingDuration = SDL_GetTicks() - startingRenderingTime;

      if (renderingDuration > m_frameDuration) {
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

    inline
    void
    SdlApplication::renderWidgets() {
      std::lock_guard<std::mutex> guard(m_widgetsLocker);

      // Clear screen content.
      m_engine->clearWindow(*m_window);

      std::shared_ptr<core::engine::Engine> engine = m_engine;

      for (WidgetsMap::iterator widgetIt = m_widgets.begin() ;
          widgetIt != m_widgets.end() ;
          ++widgetIt)
      {
        sdl::core::SdlWidgetShPtr widget = widgetIt->second;

        // Draw this object (caching is handled by the object itself).
        withSafetyNet(
          [widget, engine]() {
            core::engine::Texture::UUID texture = widget->draw();
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
      m_engine->renderWindow(*m_window);
    }

  }
}

#endif    /* SDLAPPLICATION_HXX */
