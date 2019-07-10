
# include "SdlApplication.hh"
# include <thread>
# include <core_utils/Chrono.hh>
# include <sdl_engine/Color.hh>
# include <sdl_engine/SdlEngine.hh>
# include <sdl_engine/PaintEvent.hh>
# include <sdl_graphic/TabWidget.hh>

namespace sdl {
  namespace app {

    SdlApplication::SdlApplication(const std::string& name,
                                   const std::string& title,
                                   const std::string& icon,
                                   const utils::Sizei& size,
                                   const bool resizable,
                                   const float& framerate,
                                   const float& eventsFramerate):
      core::engine::EngineObject(name),

      m_title(title),

      m_framerate(std::max(0.1f, framerate)),
      m_frameDuration(1000.0f / m_framerate),

      m_executionLocker(),
      m_renderingRunning(false),

      m_eventsDispatcher(nullptr),
      m_engine(nullptr),

      m_layout(nullptr),

      m_menuBar(nullptr),
      m_toolBar(nullptr),
      m_topArea(nullptr),
      m_leftArea(nullptr),
      m_rightArea(nullptr),
      m_centralWidget(nullptr),
      m_bottomArea(nullptr),
      m_statusBar(nullptr),

      m_widgets(),

      m_renderLocker(),
      m_cachedSize(),
      m_window(),
      m_canvas(),
      m_palette(core::engine::Palette::fromButtonColor(core::engine::Color::NamedColor::Cyan))
    {
      setService("app");

      // Create the engine and the window.
      create(size, eventsFramerate, resizable);

      // Assign the desired icon.
      setIcon(icon);
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
      // multi-threading, we chose to use an hybrid system where the rendering is
      // not completely done using the events system.
      // Basically the main thread (i.e. the one calling this function) periodically
      // wakes up to traverse the entire hierarchy of widgets in order to perform
      // the redraw. Each widget is then responsible to handle caching at its own
      // level and to provide the necessary locks so that this step does not access
      // concurrently data say if an event is processed at the same time.
      //
      // This system is cool but not perfect: indeed upon calling this function,
      // no events have been processed yet so all widgets which use the events
      // system to be rendered did not have any chance to perform the first paint
      // operation and it will become problematic very quickly.
      // Indeed we will start the events dispatcher and then start the rendering
      // right away.
      // So most probably some widgets will have been created but not all of them:
      // it all depends on how fast the events dispatcher can process the events
      // already queued. In the case of a large UI it is a lost cause.
      // We figured some kind of workaround by providing a way for the events
      // dispatcher to notify this application that at least one round of events
      // has been processed.

      // Start the event handling routine in order to launch the main event loop.
      m_eventsDispatcher->run();

      // Wait for the first events round to be processed.
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
    SdlApplication::setMenuBar(core::SdlWidget* item) {
      // Lock this app to prevent data races.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Share data with this widget.
      shareDataWithWidget(item);

      // Insert it into the layout if any.
      if (m_layout != nullptr) {
        m_layout->setMenuBar(item);
      }

      // Register it in the internal variable. Do not
      // forget to release memory used by previous
      // iterations.
      if (m_menuBar != nullptr) {
        delete m_menuBar;
      }
      m_menuBar = item;
    }

    void
    SdlApplication::addToolBar(core::SdlWidget* item) {
      // Lock this app to prevent data races.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Share data with this widget.
      shareDataWithWidget(item);

      // We need to insert this item into the corresponding
      // tab widget. We should also set the tab widget to
      // visible if needed so that it gets space upon calling
      // the layout's recompute method.
      if (m_toolBar == nullptr) {
        error(
          std::string("Could not add tool bar \"") + item->getName() + "\"",
          std::string("Invalid tab widget")
        );
      }

      m_toolBar->insertTab(m_toolBar->getTabsCount(), item);
      // Make the tab widget visible if needed.
      if (!m_toolBar->isVisible()) {
        m_toolBar->setVisible(true);

        // Trigger a layout recomputation.
        if (m_layout != nullptr) {
          m_layout->invalidate();
        }
      }
    }

    void
    SdlApplication::setCentralWidget(core::SdlWidget* item) {
      // Lock this app to prevent data races.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Share data with this widget.
      shareDataWithWidget(item);

      // Insert it into the layout if any.
      if (m_layout != nullptr) {
        m_layout->setCentralWidget(item);
      }

      // Register it in the internal variable. Do not
      // forget to release memory used by previous
      // iterations.
      if (m_centralWidget != nullptr) {
        delete m_centralWidget;
      }
      m_centralWidget = item;
    }

    void
    SdlApplication::addDockWidget(core::SdlWidget* item,
                                  const DockWidgetArea& area)
    {
      // Lock this app to prevent data races.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Share data with this widget.
      shareDataWithWidget(item);

      // We need to insert this item into the corresponding
      // tab widget. We should also set the tab widget to
      // visible if needed so that it gets space upon calling
      // the layout's recompute method.
      graphic::TabWidget* tab = getTabFromArea(area);

      if (tab == nullptr) {
        error(
          std::string("Could not add dock widget \"") + item->getName() + "\" as dock widget in area \"" + areaToName(area) + "\"",
          std::string("Invalid tab widget")
        );
      }

      tab->insertTab(tab->getTabsCount(), item);
      // Make the tab widget visible if needed.
      if (!tab->isVisible()) {
        tab->setVisible(true);

        // Trigger a layout recomputation.
        if (m_layout != nullptr) {
          m_layout->invalidate();
        }
      }

      m_widgets[item->getName()] = area;
    }

    void
    SdlApplication::setStatusBar(core::SdlWidget* item) {
      // Lock this app to prevent data races.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Share data with this widget.
      shareDataWithWidget(item);

      // Insert it into the layout if any.
      if (m_layout != nullptr) {
        m_layout->setStatusBar(item);
      }

      // Register it in the internal variable. Do not
      // forget to release memory used by previous
      // iterations.
      if (m_statusBar != nullptr) {
        delete m_statusBar;
      }
      m_statusBar = item;
    }

    void
    SdlApplication::removeToolBar(core::SdlWidget* item) {
      // Lock this app to prevent data races.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      if (item == nullptr) {
        error(
          std::string("Could not remove null tool bar from application"),
          std::string("Invalid null item")
        );
      }

      // Remove the item from the corresponding tab widget.
      if (m_toolBar == nullptr) {
        error(
          std::string("Could not remove took bar \"") + item->getName() + "\"",
          std::string("Invalid tab widget")
        );
      }

      // Remove the item from the tab widget.
      m_toolBar->removeTab(item);

      // Hide the tab widget if needed.
      if (m_toolBar->getTabsCount() == 0) {
        m_toolBar->setVisible(false);

        // Trigger a layout recomputation.
        if( m_layout != nullptr) {
          m_layout->invalidate();
        }
      }
    }

    void
    SdlApplication::removeDockWidget(core::SdlWidget* item) {
      // Lock this app to prevent data races.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      if (item == nullptr) {
        error(
          std::string("Could not remove null dock widget from application"),
          std::string("Invalid null item")
        );
      }

      // We need to remove the dock widget from its associated area.
      // To do so we first need to retrieve it.
      WidgetsMap::const_iterator area = m_widgets.find(item->getName());
      if (area == m_widgets.cend()) {
        error(
          std::string("Could not remove dock widget \"") + item->getName() + "\" from application",
          std::string("No such widget")
        );
      }

      // Now retrieve the tab associated to this area.
      graphic::TabWidget* tab = getTabFromArea(area->second);

      if (tab == nullptr) {
        error(
          std::string("Could not add dock widget \"") + item->getName() + "\" as dock widget in area \"" + areaToName(area->second) + "\"",
          std::string("Invalid tab widget")
        );
      }

      // Remove the item from the tab widget.
      tab->removeTab(item);

      // Hide the tab widget if needed.
      if (tab->getTabsCount() == 0) {
        tab->setVisible(false);

        // Trigger a layout recomputation.
        if( m_layout != nullptr) {
          m_layout->invalidate();
        }
      }
    }

    void
    SdlApplication::create(const utils::Sizei& size,
                           const float eventsFramerate,
                           const bool resizable)
    {
      // Create the engine to use to perform rendering.
      core::engine::SdlEngineShPtr engine = std::make_shared<core::engine::SdlEngine>();

      // Use the engine to create the window.
      m_window = engine->createWindow(size, resizable, getTitle());

      if (!m_window.valid()) {
        error(std::string("Could not create window with size " + size.toString()));
      }

      // Create a basic canvas which will be used as basis for the rendering.
      m_canvas = engine->createTexture(m_window, size.toType<float>(), core::engine::Palette::ColorRole::Background);
      if (!m_canvas.valid()) {
        error(std::string("Could not create window's canvas with size " + size.toString()));
      }

      // Cache the current size of this window.
      m_cachedSize = utils::Boxf::fromSize(size, true);

      // Finally create the engine decorator which will use the newly created
      // window and canvases.
      m_engine = std::make_shared<AppDecorator>(engine, m_canvas, m_palette, m_window);

      // Create the event listener and register this application as listener.
      m_eventsDispatcher = std::make_shared<core::engine::EventsDispatcher>(eventsFramerate, m_engine, true);

      // Set the queue for this application so that it can post events.
      setEventsQueue(m_eventsDispatcher.get());

      // Trigger the `build`method so that dock widgets are created.
      build();
    }

    void
    SdlApplication::build() {
      // Create the layout for this window and assign it.
      setLayout(std::make_shared<MainWindowLayout>(5.0f));

      // Create dock widget for relevant areas and add them to the
      // layout as non visible items.

      // Toolbar
      m_toolBar = new graphic::TabWidget(
        std::string("toolbar_tabwidget"),
        nullptr,
        graphic::TabWidget::TabPosition::North
      );
      shareDataWithWidget(m_toolBar);
      m_toolBar->setVisible(false);

      m_layout->addToolBar(m_toolBar);

      // Dock widgets for each area.
      m_topArea = new graphic::TabWidget(
        std::string("top_dock_tabwidget"),
        nullptr,
        graphic::TabWidget::TabPosition::North
      );
      shareDataWithWidget(m_topArea);
      m_topArea->setVisible(false);

      m_layout->addDockWidget(m_topArea, DockWidgetArea::TopArea);

      m_leftArea = new graphic::TabWidget(
        std::string("left_dock_tabwidget"),
        nullptr,
        graphic::TabWidget::TabPosition::North
      );
      shareDataWithWidget(m_leftArea);
      m_leftArea->setVisible(false);

      m_layout->addDockWidget(m_leftArea, DockWidgetArea::LeftArea);

      m_rightArea = new graphic::TabWidget(
        std::string("right_dock_tabwidget"),
        nullptr,
        graphic::TabWidget::TabPosition::North
      );
      shareDataWithWidget(m_rightArea);
      m_rightArea->setVisible(false);

      m_layout->addDockWidget(m_rightArea, DockWidgetArea::RightArea);

      m_bottomArea = new graphic::TabWidget(
        std::string("bottom_dock_tabwidget"),
        nullptr,
        graphic::TabWidget::TabPosition::North
      );
      shareDataWithWidget(m_bottomArea);
      m_bottomArea->setVisible(false);

      m_layout->addDockWidget(m_bottomArea, DockWidgetArea::BottomArea);

    }

    float
    SdlApplication::renderCanvas() {
      // Start time measurement as we have to measure the duration of
      // this function.
      auto start = std::chrono::steady_clock::now();

      // Perform the rendering for the widgets registered as children of
      // this application.
      repaintEvent(core::engine::PaintEvent(getCachedSize(), this));

      // Compute the elapsed time and return it as a floating point value.
      auto end = std::chrono::steady_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

      // auto duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

      // log("Rendering took " + std::to_string(duration2/1000) + " microseconds");

      return static_cast<float>(duration);
    }

    bool
    SdlApplication::geometryUpdateEvent(const core::engine::Event& e) {
      // We need to handle the recomputation of the internal layout if any.

      // Acquire the lock on this application.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Assign the cached size to the internal layout if any.
      if (m_layout != nullptr) {
        postEvent(std::make_shared<core::engine::ResizeEvent>(m_cachedSize, m_layout->getRenderingArea(), m_layout.get()));
      }

      // Use base handle to determine whether the event was recognized.
      return core::engine::EngineObject::geometryUpdateEvent(e);
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
      utils::Sizef dims = m_cachedSize.toSize();

      // Clear the window.
      engine->clearWindow(m_window);

      // Draw each child widget.
      if (m_menuBar != nullptr && m_menuBar->isVisible()) {
        drawWidget(m_menuBar);
      }

      if (m_toolBar != nullptr && m_toolBar->isVisible()) {
        drawWidget(m_toolBar);
      }

      if (m_topArea != nullptr && m_topArea->isVisible()) {
        drawWidget(m_topArea);
      }

      if (m_leftArea != nullptr && m_leftArea->isVisible()) {
        drawWidget(m_leftArea);
      }

      if (m_centralWidget != nullptr && m_centralWidget->isVisible()) {
        drawWidget(m_centralWidget);
      }

      if (m_rightArea != nullptr && m_rightArea->isVisible()) {
        drawWidget(m_rightArea);
      }

      if (m_bottomArea != nullptr && m_bottomArea->isVisible()) {
        drawWidget(m_bottomArea);
      }

      if (m_statusBar != nullptr && m_statusBar->isVisible()) {
        drawWidget(m_statusBar);
      }

      // Now render the content of the window and make it visible to the user.
      engine->renderWindow(m_window);

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::repaintEvent(e);
    }

    bool
    SdlApplication::windowLeaveEvent(const core::engine::WindowEvent& e) {
      // We need to trigger a global leave event so that no widget stays selected
      // or in highlight mode when the mouse is not in the window anymore.
      postEvent(std::make_shared<core::engine::Event>(core::engine::Event::Type::Leave));

      // Use base handle to determine whether the event was recognized.
      return core::engine::EngineObject::windowLeaveEvent(e);
    }

    bool
    SdlApplication::windowResizeEvent(const core::engine::WindowEvent& e) {
      // We need to handle the resize of the canvas, and possibly the size of the
      // inserted widgets.

      // Acquire the lock on this application.
      std::lock_guard<std::mutex> guard(m_renderLocker);

      // Update the size of the internal canvas if any.
      if (m_canvas.valid()) {
        m_engine->destroyTexture(m_canvas);
        m_canvas.invalidate();
      }

      // Creata a new texture with the required dimensions.
      m_canvas = m_engine->createTexture(m_window, e.getSize(), core::engine::Palette::ColorRole::Background);
      if (!m_canvas.valid()) {
        error(std::string("Could not create window's canvas with size " + e.getSize().toString()));
      }

      // Assign the new canvas texture.
      m_engine->setDrawingCanvas(m_canvas);

      // Assign the cached size.
      m_cachedSize = utils::Boxf::fromSize(e.getSize());

      // And request an update of the layout.
      invalidate();

      // Use base handler to determine whether the event was recognized.
      return core::engine::EngineObject::windowResizeEvent(e);
    }

    void
    SdlApplication::drawWidget(core::SdlWidget* widget) {
      // Retrieve drawing variables.
      std::shared_ptr<core::engine::Engine> engine = m_engine;
      const utils::Sizef dims = m_cachedSize.toSize();

      // Surround with safety net and proceed to draw the widget.
      withSafetyNet(
        [widget, engine, dims]() {
          utils::Uuid texture = widget->draw();
          utils::Boxf render = widget->getDrawingArea();

          render.x() += (dims.w() / 2.0f);
          render.y() = (dims.h() / 2.0f) - render.y();

          engine->drawTexture(
            texture,
            nullptr,
            nullptr,
            &render
          );
        },
        std::string("drawWidget(") + widget->getName() + ")"
       );
    }

  }
}
