#ifndef    SDL_APPLICATION_HXX
# define   SDL_APPLICATION_HXX

# include "SdlApplication.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace app {

    inline
    SdlApplication::~SdlApplication() {
      stop();

      // Clear widgets.
      if (m_menuBar != nullptr) {
        delete m_menuBar;
      }
      if (m_toolBar != nullptr) {
        delete m_toolBar;
      }
      if (m_topArea != nullptr) {
        delete m_topArea;
      }
      if (m_leftArea != nullptr) {
        delete m_leftArea;
      }
      if (m_rightArea != nullptr) {
        delete m_rightArea;
      }
      if (m_centralWidget != nullptr) {
        delete m_centralWidget;
      }
      if (m_bottomArea != nullptr) {
        delete m_bottomArea;
      }
      if (m_statusBar != nullptr) {
        delete m_statusBar;
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
    void
    SdlApplication::shareDataWithWidget(core::SdlWidget* widget)
    {
      // Check degenrate cases.
      if (widget == nullptr) {
        error(std::string("Cannot add null widget"));
      }

      // Set up the widget with internal elements.
      registerToSameQueue(widget);
      widget->setEngine(m_engine);
    }

    inline
    graphic::TabWidget*
    SdlApplication::getTabFromArea(const DockWidgetArea& area) {
      // Return the corresponding tab widget from the input `area`. If no area
      // can be matched, an error is raised.
      switch (area) {
        case DockWidgetArea::TopArea:
          return m_topArea;
        case DockWidgetArea::LeftArea:
          return m_leftArea;
        case DockWidgetArea::RightArea:
          return m_rightArea;
        case DockWidgetArea::BottomArea:
          return m_bottomArea;
        default:
          break;
      }

      error(
        std::string("Could not retrieve tab widget from application for area \"") + areaToName(area) + "\"",
        std::string("Invalid area")
      );

      // Silent the compiler.
      return nullptr;
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

      // Assign its events queue so that it is consistent with
      // the internal queue of the application.
      registerToSameQueue(m_layout.get());

      // Provide the current size of the application to the layout.
      invalidate();
    }

    inline
    void
    SdlApplication::invalidate() {
      // Post a new geometry update event.
      postEvent(std::make_shared<core::engine::Event>(core::engine::Event::Type::GeometryUpdate, this));
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

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::quitEvent(e);
    }

  }
}

#endif    /* SDL_APPLICATION_HXX */
