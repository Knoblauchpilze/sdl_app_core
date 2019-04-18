#ifndef    SDLAPPLICATION_HH
# define   SDLAPPLICATION_HH

# include <mutex>
# include <thread>
# include <memory>
# include <unordered_map>
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

        void
        create(const utils::Sizei& size);

        void
        stop();

        void
        performRendering();

        void
        repaint();

        // Returns the number of milliseconds elapsed when executing this function.
        int
        render();

      private:

        using WidgetsMap = std::unordered_map<std::string, sdl::core::SdlWidgetShPtr>;

        std::string m_title;

        float m_framerate;
        float m_frameDuration;

        std::mutex m_locker;
        bool m_renderingRunning;

        core::engine::EventsDispatcherShPtr m_eventsDispatcher;

        WidgetsMap m_widgets;

        AppDecoratorShPtr m_engine;
        utils::Uuid m_window;
        utils::Uuid m_canvas;
        core::engine::Palette m_palette;
    };

    using SdlApplicationShPtr = std::shared_ptr<SdlApplication>;
  }
}

# include "SdlApplication.hxx"

#endif    /* SDLAPPLICATION_HH */
