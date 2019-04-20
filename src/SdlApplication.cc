
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

      m_executionLocker(),
      m_renderingRunning(false),

      m_eventsDispatcher(nullptr),
      m_engine(nullptr),

      m_widgets(),

      m_renderLocker(),
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
      // Also there's an additional limitation that any call to `SDL_RenderPresent`
      // should be performed from the main thread.
      //
      // The last part forces us to keep some processing in the main thread, i.e.
      // in this function.
      // In order to still benefit from the event system and from some sort of
      // multi-threading, we chose to use a programmatic double buffering system
      // to still account for calls to `SDL_XYZ` function in other threads.
      // The `SdlApplication` class defines two canvases: one lives in the main
      // thread and does not really do anything apart from being being used
      // periodically to copy the second canvas, which is used to perform off-screen
      // rendering by children widgets.

      // Start the event handling routine in order to launch the main event loop.
      m_eventsDispatcher->run();

      // Notify that the rendering loop is now running.
      startRendering();

      // While we are not asked to stop, continue rendering.
      bool stillRunning = true;
      while (stillRunning) {

        // Update running status and break if needed.
        stillRunning = isRendering();
        if (!stillRunning) {
          break;
        }

        // We need to perform the rendering to display the content of the window
        // to the user. Most API prevents GUI operations to be performed into the
        // main thread.
        // In order to do this, we use the `m_offscreenCanvas` as an offscreen
        // buffer to perform children widgets rendering. This canvas is then
        // preiodically copied to the real `m_canvas` in the main thread so that
        // the API detect this as a valid call.
        // We have to use a locker to perform the copy as another asynchronous
        // loop is running (the one started by launching the `m_eventsDispatcher->run()`
        // method) and could trigger conflicts while accessing to the texture.
        // This is not really a problem though because the offscreen canvas is
        // not locked during extensive periods of time: it is only unavailable
        // whenever a children widget is actually performing a rendering onto it.
        // In addition to that, we have to keep track of time in this method as
        // we only want to perform a certain amount of repaint every second.

        // Actually perform the copy of the offscreen canvas into the one displayed
        // on screen.
        const float frameDuration = renderCanvas();

        // Check whether the rendering time is compatible with the desired framerate.
        if (frameDuration > m_frameDuration) {
          // Log this problem.
          log(
            std::string("Repaint took ") + std::to_string(frameDuration) + "ms " +
            "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
            " authorized to maintain " + std::to_string(m_framerate) + "fps",
            utils::Level::Warning
          );

          // Move on to the next frame.
          continue;
        }

        // Sleep for the remaining time to complete a frame if there's enough time left.
        const unsigned int remainingDuration = static_cast<unsigned>(m_frameDuration - frameDuration);
        if (remainingDuration > 3u) {
          std::this_thread::sleep_for(std::chrono::milliseconds(remainingDuration));
        }
      }

      log(std::string("Exiting rendering thread"), utils::Level::Notice);
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
      // window and canvases.
      m_engine = std::make_shared<AppDecorator>(engine, m_canvas, m_palette, m_window);
    }

    float
    SdlApplication::renderCanvas() {
      // Start time measurement as we have to measure the duration of
      // this function.
      auto start = std::chrono::steady_clock::now();

      // Perform the rendering for the widgets registered as children of
      // this application.
      repaintEvent(core::engine::PaintEvent(getCachedSize()));

      // Compute the elapsed time and return it as a floating point value.
      auto end = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      return static_cast<float>(duration);
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

      log("Handling event of type " + std::to_string(static_cast<int>(e->getType())));

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
    SdlApplication::repaintEvent(const core::engine::PaintEvent& e) {
      // Rendering widgets include building a valid `m_canvas` textures by successfully
      // drawing each child widget onto it.
      // Building the `m_canvas` relies on 4 operations:
      // 1) Clear the canvas from existing content.
      // 2) Render each child widget on the `m_canvas`.
      // 3) Render the `m_canvas` to the screen.
      // 4) Update the windows to reveal the modifications.

      // Acquire the lock protecting the canvas so that we can guarantee that no other
      // rendering will take place simultaneously.
      std::lock_guard<std::mutex> guard(m_renderLocker);

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

      // Now render the content of the window and make it visible to the user.
      engine->renderWindow(m_window);

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::repaintEvent(e);
    }

  }
}
