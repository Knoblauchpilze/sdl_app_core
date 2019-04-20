#ifndef    SDLAPPLICATION_HXX
# define   SDLAPPLICATION_HXX

# include "SdlApplication.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace app {

    inline
    SdlApplication::~SdlApplication() {
      stop();
    }

    inline
    const std::string&
    SdlApplication::getTitle() const noexcept {
      return m_title;
    }

    inline
    core::engine::Engine&
    SdlApplication::getEngine() {
      if (m_engine == nullptr) {
        error(
          std::string("Cannot return null engine from application \"") + getName() + "\"",
          std::string("Engine not set")
        );
      }

      return *m_engine;
    }

    inline
    void
    SdlApplication::setIcon(const std::string& icon) {
      m_engine->setWindowIcon(m_window, icon);
    }

    inline
    void
    SdlApplication::run() {
      // The `run` method needs to start several process inside the application.
      // Events need to be handling to be able to process user's requests.
      // In the meantime, we should maintain a steady framerate as described by
      // the `m_framerate` attribute.
      // Finally children widgets should be rendered whenever needed.
      // This application creates the engine to use to interact with the low level
      // library allowing rendering and thus we should guarantee that all children
      // widgets also gain access to the internal engine.
      // We chose to do that by providing a root canvas onto which widgets will be
      // drawn and to which each one will be attached.
      // This allows to benefit from the automatic transfer of the engine of a
      // widget to its children.
      // Also to maintain a steady framerate, we chose to perform the widgets'
      // rendering and then perform a short sleep whenever the rendering took less
      // time than allowed, or printing a warning message in case the rendering
      // time took too long.

      // First, start the event handling routine.
      m_eventsDispatcher->run();

      // Start main loop to render the root canvas.
      performRendering();
    }

    inline
    void
    SdlApplication::addWidget(sdl::core::SdlWidgetShPtr widget) {
      if (widget == nullptr) {
        error(std::string("Cannot add null widget"));
      }

      widget->setEngine(m_engine);
      widget->setEventsQueue(m_eventsDispatcher.get());

      m_widgets[widget->getName()] = widget;
    }

    inline
    void
    SdlApplication::removeWidget(sdl::core::SdlWidgetShPtr widget) {
      if (widget == nullptr) {
        error(std::string("Cannot remove null widget"));
      }

      m_widgets.erase(widget->getName());
    }

    inline
    void
    SdlApplication::stop() {
      // Stop the events handler.
      if (m_eventsDispatcher->isRunning()) {
        m_eventsDispatcher->stop();
      }

      // The widget's rendering is not a concern here: either it has not
      // started in which case there's nothing to worry about or it has
      // been started and the only solution for ending up here is that
      // the infinite loop started for the rendering has been stopped by
      // some other means (typically through a user request).
      // So nothing to be done in here.
    }

    inline
    utils::Boxf
    SdlApplication::getCachedSize() noexcept {
      std::lock_guard<std::mutex> guard(m_renderLocker);
      return m_cachedSize;
    }

  }
}

#endif    /* SDLAPPLICATION_HXX */
