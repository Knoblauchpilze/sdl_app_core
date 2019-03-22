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
    }

    inline
    const std::string&
    SdlApplication::getTitle() const noexcept {
      return m_title;
    }

    inline
    void
    SdlApplication::setIcon(const std::string& icon) {
      m_engine->setWindowIcon(m_window, icon);
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

  }
}

#endif    /* SDLAPPLICATION_HXX */
