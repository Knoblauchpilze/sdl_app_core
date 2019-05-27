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
                       const float& eventFramerate = 30.0f);

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
        addWidget(core::SdlWidget* widget);

        void
        removeWidget(core::SdlWidget* widget);

        utils::Boxf
        getCachedSize() noexcept;

        void
        setLayout(MainWindowLayoutShPtr layout);

        void
        invalidate();

        void
        stop();

        void
        create(const utils::Sizei& size,
               const bool resizable);

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
        handleEvent(core::engine::EventShPtr e) override;

        bool
        geometryUpdateEvent(const core::engine::Event& e) override;

        bool
        refreshEvent(const core::engine::PaintEvent& e) override;

        bool
        repaintEvent(const core::engine::PaintEvent& e) override;

        bool
        windowLeaveEvent(const core::engine::WindowEvent& e) override;

        bool
        windowResizeEvent(const core::engine::WindowEvent& e) override;

        bool
        quitEvent(const core::engine::QuitEvent& e) override;

      private:

        using WidgetsMap = std::unordered_map<std::string, core::SdlWidget*>;

        std::string m_title;

        float m_framerate;
        float m_frameDuration;

        std::mutex m_executionLocker;
        bool m_renderingRunning;

        core::engine::EventsDispatcherShPtr m_eventsDispatcher;
        AppDecoratorShPtr m_engine;

        MainWindowLayoutShPtr m_layout;
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
