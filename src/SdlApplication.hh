#ifndef    SDLAPPLICATION_HH
# define   SDLAPPLICATION_HH

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

namespace sdl {
  namespace app {

    class SdlApplication : public core::engine::EngineObject {
      public:

        explicit
        SdlApplication(const std::string& name,
                       const std::string& title,
                       const std::string& icon,
                       const utils::Sizei& size = utils::Sizei(640, 480),
                       const float& framerate = 60.0f,
                       const float& eventFramerate = 30.0f);

        virtual ~SdlApplication();

        const std::string&
        getTitle() const noexcept;

        core::engine::Engine&
        getEngine();

        void
        setIcon(const std::string& icon);

        void
        run();

        void
        addWidget(sdl::core::SdlWidgetShPtr widget);

        void
        removeWidget(sdl::core::SdlWidgetShPtr widget);

      private:

        bool
        handleEvent(core::engine::EventShPtr e) override;

        bool
        refreshEvent(const core::engine::PaintEvent& e) override;

        bool
        repaintEvent(const core::engine::PaintEvent& e) override;

        bool
        windowEnterEvent(const core::engine::WindowEvent& e) override;

        bool
        windowLeaveEvent(const core::engine::WindowEvent& e) override;

        bool
        windowResizeEvent(const core::engine::WindowEvent& e) override;

        bool
        quitEvent(const core::engine::QuitEvent& e) override;

        void
        create(const utils::Sizei& size);

        void
        stop();

        void
        performRendering();

        utils::Boxf
        getCachedSize() noexcept;

      private:

        using WidgetsMap = std::unordered_map<std::string, sdl::core::SdlWidgetShPtr>;

        std::string m_title;

        float m_framerate;
        float m_frameDuration;

        std::mutex m_renderingLocker;
        bool m_renderingRunning;

        core::engine::EventsDispatcherShPtr m_eventsDispatcher;

        WidgetsMap m_widgets;


        std::mutex m_renderLocker;
        AppDecoratorShPtr m_engine;
        utils::Boxf m_cachedSize;
        utils::Uuid m_window;
        // TODO: Should use double buffering as well.
        utils::Uuid m_canvas;
        core::engine::Palette m_palette;
    };

    using SdlApplicationShPtr = std::shared_ptr<SdlApplication>;
  }
}

# include "SdlApplication.hxx"

#endif    /* SDLAPPLICATION_HH */
