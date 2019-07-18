#ifndef    SDL_APPLICATION_HH
# define   SDL_APPLICATION_HH

# include <mutex>
# include <thread>
# include <memory>
# include <unordered_map>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <sdl_core/SdlWidget.hh>
# include <sdl_engine/Window.hh>
# include <sdl_engine/Palette.hh>
# include <sdl_engine/Event.hh>
# include <sdl_engine/EventsDispatcher.hh>
# include <sdl_graphic/TabWidget.hh>
# include "AppDecorator.hh"
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    class SdlApplication : public core::engine::EngineObject {
      public:

        explicit
        SdlApplication(const std::string& name,
                       const std::string& title,
                       const std::string& icon,
                       const utils::Sizei& size = utils::Sizei(640, 480),
                       const bool resizable = true,
                       const float& framerate = 60.0f,
                       const float& eventsFramerate = 30.0f);

        virtual ~SdlApplication();

        const std::string&
        getTitle() const noexcept;

        void
        setIcon(const std::string& icon);

        void
        run();

        void
        setMenuBar(core::SdlWidget* item);

        void
        addToolBar(core::SdlWidget* item);

        void
        setCentralWidget(core::SdlWidget* item);

        void
        addDockWidget(core::SdlWidget* item,
                      const DockWidgetArea& area);

        void
        setStatusBar(core::SdlWidget* item);

        void
        removeToolBar(core::SdlWidget* item);

        void
        removeDockWidget(core::SdlWidget* item);

      private:

        void
        startRendering() noexcept;

        bool
        isRendering() noexcept;

        void
        stopRendering() noexcept;

        void
        shareDataWithWidget(core::SdlWidget* widget);

        /**
         * @brief - Returns one of the internal tab widget variable based on the input `area`.
         *          This allows to easily manipulate dock widget area instead of always having
         *          to rely on a switch.
         *          Note that if the `area` does not match any tab widget an error is raised.
         * @param area - the area for which the corresponding tab widget should be returned.
         * @return - the tab widget corresponding to the input `area`.
         */
        graphic::TabWidget*
        getTabFromArea(const DockWidgetArea& area);

        utils::Boxf
        getCachedSize() noexcept;

        void
        setLayout(MainWindowLayoutShPtr layout);

        void
        invalidate();

        void
        stop();

        /**
         * @brief - Used to create basic properties of the application such as the
         *          engine, the general canvas which will be used to render widgets
         *          and the events queue.
         *          Also triggers a call to the `build` method.
         * @param size - the size of the window to create.
         * @param eventsFramerate - the framrerate to maintain while processing
         *                          events, expressed in fps.
         * @param resizable - true if the window should be made resizable, false
         *                    otherwise.
         */
        void
        create(const utils::Sizei& size,
               const float eventsFramerate,
               const bool resizable);

        /**
         * @brief - Creates the dock widgets related to each area and hide each one
         *          of them. They will be revealed if needed when the user adds items
         *          inside them along the way.
         *          It also creates the layout to use to position widgets inside the
         *          area available for this application.
         */
        void
        build();

        /**
         * @brief - Used to perform the rendering of the offscreen canvas to the
         *          displayed canvas.
         *          Returns the elapsed time for the copy operation.
         * @return - a floating point value representing the elapsed time to perform
         *           render of the offscreen canvas onto the visible one.
         *           The duration is expressed in milliseconds which is convenient
         *           to compare it to the internal `m_frameDuration` for example.
         */
        float
        renderCanvas();

        bool
        geometryUpdateEvent(const core::engine::Event& e) override;

        bool
        repaintEvent(const core::engine::PaintEvent& e) override;

        bool
        windowLeaveEvent(const core::engine::WindowEvent& e) override;

        bool
        windowResizeEvent(const core::engine::WindowEvent& e) override;

        bool
        quitEvent(const core::engine::QuitEvent& e) override;

        /**
         * @brief - Used to draw the input `widget` assuming it is not null.
         *          No checks are performed to determine whether it is actually
         *          not null so use with care.
         * @param widget - the widget to draw.
         */
        void
        drawWidget(core::SdlWidget* widget);

      private:

        using WidgetsMap = std::unordered_map<std::string, DockWidgetArea>;

        std::string m_title;

        float m_framerate;
        float m_frameDuration;

        std::mutex m_executionLocker;
        bool m_renderingRunning;

        core::engine::EventsDispatcherShPtr m_eventsDispatcher;
        AppDecoratorShPtr m_engine;

        MainWindowLayoutShPtr m_layout;
        core::SdlWidget* m_menuBar;
        graphic::TabWidget* m_toolBar;
        graphic::TabWidget* m_topArea;
        graphic::TabWidget* m_leftArea;
        graphic::TabWidget* m_rightArea;
        core::SdlWidget* m_centralWidget;
        graphic::TabWidget* m_bottomArea;
        core::SdlWidget* m_statusBar;

        WidgetsMap m_widgets;

        std::mutex m_renderLocker;
        utils::Boxf m_cachedSize;
        utils::Uuid m_window;
        utils::Uuid m_canvas;
        core::engine::Palette m_palette;
    };

    using SdlApplicationShPtr = std::shared_ptr<SdlApplication>;
  }
}

# include "SdlApplication.hxx"

#endif    /* SDL_APPLICATION_HH */
