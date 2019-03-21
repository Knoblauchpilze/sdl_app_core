#ifndef    SDLAPPLICATION_HH
# define   SDLAPPLICATION_HH

# include <mutex>
# include <memory>
# include <unordered_map>
# include <SDL2/SDL.h>
# include <maths_utils/Size.hh>
# include <sdl_core/SdlWidget.hh>
# include <sdl_core/EventListener.hh>
# include <sdl_engine/Engine.hh>
# include <sdl_engine/Window.hh>

# include "SdlEventHandler.hh"

namespace sdl {
  namespace app {

    class SdlApplication : public sdl::core::EventListener {
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

        void
        setIcon(const std::string& icon);

        void
        run();

        void
        onQuitEvent(const SDL_QuitEvent& event) override;

        void
        addWidget(sdl::core::SdlWidgetShPtr widget);

        void
        removeWidget(sdl::core::SdlWidgetShPtr widget);

      private:

        void
        create(const utils::Sizei& size);

        void
        lock();

        void
        unlock();

        void
        performRendering();

        void
        render();

        void
        renderWidgets();

      public:

        using WidgetsMap = std::unordered_map<std::string, sdl::core::SdlWidgetShPtr>;

        std::string m_title;

        float m_framerate;
        float m_frameDuration;

        std::mutex m_locker;
        bool m_renderingRunning;

        SdlEventHandler m_eventsHandler;

        std::mutex m_widgetsLocker;
        WidgetsMap m_widgets;

        core::engine::EngineShPtr m_engine;
        std::shared_ptr<core::engine::Window::UUID> m_window;
        utils::Uuid m_canvas;
    };

    using SdlApplicationShPtr = std::shared_ptr<SdlApplication>;
  }
}

# include "SdlApplication.hxx"

#endif    /* SDLAPPLICATION_HH */
