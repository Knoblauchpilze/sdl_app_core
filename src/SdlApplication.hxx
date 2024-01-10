#ifndef    SDL_APPLICATION_HXX
# define   SDL_APPLICATION_HXX

# include "SdlApplication.hh"

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
      const std::lock_guard guard(m_executionLocker);
      m_renderingRunning = true;
    }

    inline
    bool
    SdlApplication::isRendering() noexcept {
      const std::lock_guard guard(m_executionLocker);
      return m_renderingRunning;
    }

    inline
    void
    SdlApplication::stopRendering() noexcept {
      // Stop events processing.
      const std::lock_guard guard(m_executionLocker);
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
    void
    SdlApplication::setLayout(MainWindowLayoutShPtr layout) {
      // Lock this application.
      const std::lock_guard guard(m_renderLocker);

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
      // However we have to unregister ourselves from the events queue.
      // Indeed as the queue is owned by this widget if we don't do that
      // here's what will happen:
      // - the destructor of the `SdlApplication` will be called.
      // - each attribute will be destroyed.
      // - including the events queue itself.
      // - we will destroy this object.
      // - this object inherits from `EngineObject` so its destructor will
      //   be called.
      // - this object has a valid queue and the destructor will try to
      //   unregister from it.
      // - segfault.
      //
      // So let's unregister right now so that we don't go in trouble.
      unregisterFromQueue();
    }

    inline
    bool
    SdlApplication::quitEvent(const core::engine::QuitEvent& e) {
      // Stop rendering.
      stopRendering();

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::quitEvent(e);
    }

    inline
    float
    SdlApplication::fetchSystemEvents() {
      // Use the engine to fetch the events: this allows for easy modification
      // of the actual API used to fetch system events. Note that as we assume
      // that the locker for this application is already locked we can safely
      // go ahead and use the `m_engine` pointer.
      auto start = std::chrono::steady_clock::now();

      std::vector<core::engine::EventShPtr> events = m_engine->pollEvents();

      // Populate the events dispatcher with the events.
      m_eventsDispatcher->pumpEvents(events);

      auto end = std::chrono::steady_clock::now();

      auto nanoDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
      verbose("Events pumping took " + std::to_string(nanoDuration/1000) + "µs");

      return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

  }
}

#endif    /* SDL_APPLICATION_HXX */
