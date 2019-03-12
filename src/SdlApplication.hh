#ifndef    SDLAPPLICATION_HH
# define   SDLAPPLICATION_HH

# include <mutex>
# include <memory>
# include <unordered_map>

# include <SDL2/SDL.h>

# include <maths_utils/Size.hh>
# include <sdl_core/SdlWidget.hh>
# include <sdl_core/EventListener.hh>

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

        int
        getWidth() const;

        int
        getHeight() const;

        const std::string&
        getTitle() const noexcept;

        void
        setIcon(const std::string& icon);

        std::string
        getName() const noexcept;

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
        initializeSdlLib() const;

        void
        createWindow(const utils::Sizei& size);

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

        void
        log(const std::string& message,
            const utils::Level& level = utils::Level::Debug) const noexcept;

      private:

        static const char* sk_serviceName;

        std::string m_name;
        std::string m_title;
        std::string m_icon;
        float m_framerate;
        float m_frameDuration;
        SDL_Window* m_window;
        SDL_Renderer* m_renderer;
        SdlEventHandler m_eventsHandler;

        bool m_renderingRunning;
        std::mutex m_locker;

        std::unordered_map<std::string, sdl::core::SdlWidgetShPtr> m_widgets;
        std::mutex m_widgetsLocker;
    };

    using SdlApplicationShPtr = std::shared_ptr<SdlApplication>;
  }
}

# include "SdlApplication.hxx"

#endif    /* SDLAPPLICATION_HH */
