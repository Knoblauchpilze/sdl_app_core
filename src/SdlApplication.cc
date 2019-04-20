
# include "SdlApplication.hh"
# include <chrono>
# include <thread>
# include <sdl_engine/Color.hh>
# include <sdl_engine/SdlEngine.hh>
# include <sdl_engine/PaintEvent.hh>

namespace sdl {
  namespace app {

    SdlApplication::SdlApplication(const std::string& name,
                                   const std::string& title,
                                   const std::string& icon,
                                   const utils::Sizei& size,
                                   const float& framerate,
                                   const float& eventFramerate):
      core::engine::EngineObject(name),

      m_title(title),

      m_framerate(std::max(0.1f, framerate)),
      m_frameDuration(1000.0f / m_framerate),

      m_renderingLocker(),
      m_renderingRunning(false),

      m_eventsDispatcher(nullptr),

      m_widgets(),

      m_renderLocker(),
      m_engine(nullptr),
      m_cachedSize(),
      m_window(),
      m_canvas(),
      m_palette(core::engine::Palette::fromButtonColor(core::engine::Color::NamedColor::Cyan))
    {
      setService("app");

      // Create the engine and the window.
      create(size);

      // Assign the desired icon.
      setIcon(icon);

      // Create the event listener and register this application as listener.
      m_eventsDispatcher = std::make_shared<core::engine::EventsDispatcher>(eventFramerate, m_engine, true);
      m_eventsDispatcher->addListener(this);
    }

    bool
    SdlApplication::handleEvent(core::engine::EventShPtr e) {
      // This function is made to react to all the events produced by
      // the system and also to events posted by objects in the
      // hierarchy.
      // We try to make sense of some events in here and to add some
      // context for specific events. For example, we can try to make
      // sense of `MouseEvent` to produce `EnterEvent` and notify the
      // adequate widget of such an event.
      // This cannot be done by the `EventQueue` itself because it
      // has no information whatsoever about the widgets and the
      // hierarchy of elements in the application.
      // TODO: Add it at some point ?
      //
      // In addition to that, the `QuitEvent`s should be processed
      // in here to stop the application.

      // Handle this event using the base handler.
      const bool recognized = core::engine::EngineObject::handleEvent(e);

      // Check whether the event has been accepted before dispatching
      // to children widgets.
      if (!e->isAccepted()) {
        // We need to trnasmit the event to the widgets added to the
        // window if any.
        for (WidgetsMap::iterator widgetIt = m_widgets.begin() ;
            widgetIt != m_widgets.end() ;
            ++widgetIt)
        {
          core::SdlWidgetShPtr widget = widgetIt->second;

          // Perform event handling for this widget using the input event `e`.
          withSafetyNet(
            [widget, e]() {
              widget->event(e);
            },
            std::string("widget_event")
          );
        }
      }

      return recognized;
    }

    bool
    SdlApplication::refreshEvent(const core::engine::PaintEvent& e) {
      // TODO: Should handle the update region for the event.
      log(std::string("Should handle the update region of ") + e.getUpdateRegion().toString(), utils::Level::Warning);

      // Refresh the window.
      m_engine->renderWindow(m_window);

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::refreshEvent(e);
    }

    bool
    SdlApplication::repaintEvent(const core::engine::PaintEvent& e) {
      // Rendering widgets include building a valid `m_canvas` textures by successfully
      // drawing each child widget onto it.
      // Building the `m_canvas` relies on 4 operations:
      // 1) Clear the canvas from existing content.
      // 2) Render each child widget on the `m_canvas`.
      // 3) Render the `m_canvas` to the screen.
      // 4) Update the windows to reveal the modifications.
      // We also need to perform some kind of time measurement to check whether we're too
      // slow to perform rendering: even though we're too deep into the process to do
      // anything to improve things it might still be nice to know that the repaint event
      // actually took too much time to keep a steady framerate.

      auto start = std::chrono::steady_clock::now();

      // Keep a local reference to the engine to be able to pass it through the
      // lambda expression.
      std::shared_ptr<core::engine::Engine> engine = m_engine;

      // Clear the window.
      // TODO: Should handle only the region to update.
      engine->clearWindow(m_window);

      // Draw each child widget.
      for (WidgetsMap::iterator widgetIt = m_widgets.begin() ;
          widgetIt != m_widgets.end() ;
          ++widgetIt)
      {
        core::SdlWidgetShPtr widget = widgetIt->second;

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

      // Perform time measurement and display a warning if the repaint took took long.
      const float elapsed = 1.0f * std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
      ).count();

      // Check whether the rendering time is compatible with the desired framerate.
      if (elapsed > m_frameDuration) {
        // Log this problem.
        log(
          std::string("Repaint repaint took ") + std::to_string(elapsed) + "ms " +
          "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
          " authorized to maintain " + std::to_string(m_framerate) + "fps",
          utils::Level::Warning
        );
      }

      // Use the dedicated handler to refresh the region to update.
      // return refreshEvent(e);
      return core::engine::EngineObject::repaintEvent(e);
    }

    bool
    SdlApplication::windowEnterEvent(const core::engine::WindowEvent& e) {
      log(std::string("Should handle window enter event"), utils::Level::Warning);
      return core::engine::EngineObject::windowEnterEvent(e);
    }

    bool
    SdlApplication::windowLeaveEvent(const core::engine::WindowEvent& e) {
      log(std::string("Should handle window leave event"), utils::Level::Warning);
      return core::engine::EngineObject::windowLeaveEvent(e);
    }

    bool
    SdlApplication::windowResizeEvent(const core::engine::WindowEvent& e) {
      log(std::string("Should handle window resize event"), utils::Level::Warning);
      return core::engine::EngineObject::windowResizeEvent(e);
    }

    bool
    SdlApplication::quitEvent(const core::engine::QuitEvent& e) {
      std::lock_guard<std::mutex> guard(m_renderingLocker);
      m_renderingRunning = false;

      // Mark the event as accepted.
      e.accept();

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::quitEvent(e);
    }

    void
    SdlApplication::create(const utils::Sizei& size) {
      // Create the engine to use to perform rendering.
      core::engine::SdlEngineShPtr engine = std::make_shared<core::engine::SdlEngine>();

      // Use the engine to create the window.
      m_window = engine->createWindow(size, getTitle());

      if (!m_window.valid()) {
        error(std::string("Could not create window with size " + size.toString()));
      }

      // Create a basic canvas which will be used as basis for the rendering.
      m_canvas = engine->createTexture(m_window, size, core::engine::Palette::ColorRole::Background);

      if (!m_canvas.valid()) {
        error(std::string("Could not create window's canvas with size " + size.toString()));
      }

      // Cache the current size of this window.
      m_cachedSize = utils::Boxf::fromSize(size);

      // Finally create the engine decorator which will use the newly created
      // window and canvas
      m_engine = std::make_shared<AppDecorator>(engine, m_canvas, m_palette, m_window);
    }

    void
    SdlApplication::performRendering() {
      // Notify that the rendering for this application has started.
      m_renderingLocker.lock();
      m_renderingRunning = true;
      m_renderingLocker.unlock();

      // While we are not asked to stop, continue rendering.
      bool stillRunning = true;
      while (stillRunning) {
        m_renderingLocker.lock();
        stillRunning = m_renderingRunning;
        m_renderingLocker.unlock();

        if (!stillRunning) {
          break;
        }

        // Post a repaint event in the event queue. This event will be processed
        // at the most suited moment by the dispatcher and we will reenter in the
        // correct method.
        // Usually we should not have too many difficulties pushing the event so
        // we can theoretically maintain a steady framerate at this step: we post
        // the event and then sleep for the remaining duration.

        // Start time measurement.
        auto start = std::chrono::steady_clock::now();

        postEvent(std::make_shared<core::engine::PaintEvent>(getCachedSize()));

        const float elapsed = 1.0f * std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start
        ).count();

        // Check whether the rendering time is compatible with the desired framerate.
        if (elapsed > m_frameDuration) {
          // Log this problem.
          log(
            std::string("Posting repaint took ") + std::to_string(elapsed) + "ms " +
            "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
            " authorized to maintain " + std::to_string(m_framerate) + "fps",
            utils::Level::Warning
          );

          // Move on to the next frame.
        }

        m_engine->renderWindow(m_window);

        // Sleep for the remaining time to complete a frame if there's enough time left.
        const unsigned int remainingDuration = static_cast<unsigned>(m_frameDuration - elapsed);
        if (remainingDuration > 3u) {
          std::this_thread::sleep_for(std::chrono::milliseconds(remainingDuration));
        }
      }
    }

  }
}
