#ifndef    SDLAPPLICATION_HXX
# define   SDLAPPLICATION_HXX

# include "SdlApplication.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace app {

    inline
    SdlApplication::~SdlApplication() {
      stop();

      // Clear widgets.
      for (WidgetsMap::const_iterator widget = m_widgets.cbegin() ;
           widget != m_widgets.cend() ;
           ++widget)
      {
        if (widget->second != nullptr) {
          delete widget->second;
        }
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
    SdlApplication::addWidget(core::SdlWidget* widget) {
      // Check degenrate cases.
      if (widget == nullptr) {
        error(std::string("Cannot add null widget"));
      }

      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Set up the widget with internal elements.
      widget->setEventsQueue(m_eventsDispatcher.get());
      widget->setEngine(m_engine);

      // Add the widget to the internal layout if any.
      m_layout->setCentralWidget(widget);

      // Add this widget to the internal table.
      m_widgets[widget->getName()] = widget;
    }

    inline
    void
    SdlApplication::removeWidget(core::SdlWidget* widget) {
      // Check degenrate cases.
      if (widget == nullptr) {
        error(std::string("Cannot remove null widget"));
      }

      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Erase the widget and display the result.
      std::size_t nbErased = m_widgets.erase(widget->getName());
      if (nbErased != 1) {
        log(
          std::string("Could not remove widget \"") + widget->getName() + "\" from window, not found",
          utils::Level::Warning
        );
        return;
      }

      // Remove the widget from the layout if any.
      if (m_layout != nullptr) {
        m_layout->removeItem(widget);
      }
    }

    inline
    void
    SdlApplication::startRendering() noexcept {
      std::lock_guard<std::mutex> guard(m_executionLocker);
      m_renderingRunning = true;
    }

    inline
    bool
    SdlApplication::isRendering() noexcept {
      std::lock_guard<std::mutex> guard(m_executionLocker);
      return m_renderingRunning;
    }

    inline
    void
    SdlApplication::stopRendering() noexcept {
      // Stop events processing.
      std::lock_guard<std::mutex> guard(m_executionLocker);
      m_renderingRunning = false;
    }

    inline
    utils::Boxf
    SdlApplication::getCachedSize() noexcept {
      std::lock_guard<std::mutex> guard(m_renderLocker);
      return m_cachedSize;
    }

    inline
    void
    SdlApplication::setLayout(MainWindowLayoutShPtr layout) {
      // Lock this application.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Assign the new layout.
      m_layout = layout;

      // Provide the current size of the application to the layout.
      invalidate(m_cachedSize);
    }

    inline
    void
    SdlApplication::invalidate(const utils::Boxf& area) {
      // Assign the cached size to the internal layout if any.
      if (m_layout != nullptr) {
        m_layout->setArea(area);
      }

      // Post a new geometry update event.
      postEvent(
        std::make_shared<core::engine::Event>(
          core::engine::Event::Type::GeometryUpdate,
          this
        )
      );
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
    bool
    SdlApplication::refreshEvent(const core::engine::PaintEvent& e) {
      log(std::string("Should handle refresh event"), utils::Level::Warning);
      return core::engine::EngineObject::refreshEvent(e);
    }

    inline
    bool
    SdlApplication::quitEvent(const core::engine::QuitEvent& e) {
      // Stop rendering.
      stopRendering();

      // Mark the event as accepted if it is directed only through this
      // object.
      if (isReceiver(e)) {
        e.accept();
      }

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::quitEvent(e);
    }

  }
}

#endif    /* SDLAPPLICATION_HXX */
