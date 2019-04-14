
# include "SdlApplication.hh"
# include <chrono>
# include <thread>
# include <sdl_engine/Color.hh>
# include <sdl_engine/SdlEngine.hh>
# include <sdl_core/UserInputFilter.hh>

namespace sdl {
  namespace app {

    SdlApplication::SdlApplication(const std::string& name,
                                   const std::string& title,
                                   const std::string& icon,
                                   const utils::Sizei& size,
                                   const float& framerate,
                                   const float& eventFramerate):
      core::engine::EventListener(name),

      m_title(title),

      m_framerate(std::max(0.1f, framerate)),
      m_frameDuration(1000.0f / m_framerate),

      m_locker(),
      m_renderingRunning(false),

      m_eventsHandler(nullptr),

      m_widgetsLocker(),
      m_widgets(),

      m_engine(nullptr),
      m_window(),
      m_canvas(),
      m_palette(core::engine::Palette::fromButtonColor(core::engine::Color::NamedColor::Cyan))
    {
      setService("app");

      // Create the engine and the window.
      create(size);

      // Assign the desired icon.
      setIcon(icon);

      // Install an event filter to select specifically quit events.
      installEventFilter(core::UserInputFilter::createExclusionFilterFromMask(core::UserInputFilter::Interaction::Quit));

      // Create the event listener and register this application as listener.
      m_eventsHandler = std::make_shared<SdlEventHandler>(eventFramerate, m_engine, true);
      m_eventsHandler->addListener(this);
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

      // Finally create the engine decorator which will use the newly created
      // window and canvas
      m_engine = std::make_shared<AppDecorator>(engine, m_canvas, m_palette, m_window);
    }

    void
    SdlApplication::performRendering() {
      // Notify that the rendering for this application has started.
      m_locker.lock();
      m_renderingRunning = true;
      m_locker.unlock();

      // While we are not asked to stop, continue rendering.
      bool stillRunning = true;
      while (stillRunning) {
        m_locker.lock();
        stillRunning = m_renderingRunning;
        m_locker.unlock();

        if (!stillRunning) {
          break;
        }

        repaint();
      }
    }

    void
    SdlApplication::repaint() {
      // Render widgets.
      int renderingDuration = render();

      // Check whether the rendering time is compatible with the desired framerate.
      if (1.0f * renderingDuration > m_frameDuration) {
        // Log this problem.
        log(
          std::string("Frame took ") + std::to_string(renderingDuration) + "ms " +
          "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
          " authorized to maintain " + std::to_string(m_framerate) + "fps",
          utils::Level::Warning
        );

        // Move on to the next frame.
        return;
      }

      // Sleep for the remaining time to complete a frame if there's enough time left.
      const unsigned int remainingDuration = m_frameDuration - renderingDuration;
      if (remainingDuration > 3u) {
        std::this_thread::sleep_for(std::chrono::milliseconds(remainingDuration));
      }
    }

    int
    SdlApplication::render() {
      std::lock_guard<std::mutex> guard(m_widgetsLocker);

      // Start time measurement.
      auto start = std::chrono::steady_clock::now();

      std::shared_ptr<core::engine::Engine> engine = m_engine;


      // Rendering widgets include building a valid `m_canvas` textures by successfully
      // drawing each child widget onto it.
      // Building the `m_canvas` relies on 4 operations:
      // 1) Clear the canvas from existing content.
      // 2) Render each child widget on the `m_canvas`.
      // 3) Render the `m_canvas` to the screen.
      // 4) Update the windows to reveal the modifications.
      m_engine->clearWindow(m_window);

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

      m_engine->renderWindow(m_window);

      // Return the elapsed time.
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    }

  }
}
