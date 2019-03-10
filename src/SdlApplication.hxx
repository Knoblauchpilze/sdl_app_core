#ifndef    SDLAPPLICATION_HXX
# define   SDLAPPLICATION_HXX

# include "SdlApplication.hh"

# include <core_utils/CoreLogger.hh>
# include <core_utils/CoreWrapper.hh>
# include <sdl_core/SdlException.hh>

namespace sdl {
  namespace app {

    inline
    SdlApplication::~SdlApplication() {
      if (m_eventsHandler.isRunning()) {
        m_eventsHandler.stop();
      }

      if (m_window != nullptr) {
        SDL_DestroyWindow(m_window);
      }
    }

    inline
    int
    SdlApplication::getWidth() const {
      if (m_window == nullptr) {
        throw AppException(std::string("Cannot retrieve height for invalid sdl window \"") + getTitle() + "\"");
      }

      int width = 0;
      SDL_GetWindowSize(m_window, &width, nullptr);
      return width;
    }

    inline
    int
    SdlApplication::getHeight() const {
      if (m_window == nullptr) {
        throw AppException(std::string("Cannot retrieve height for invalid sdl window \"") + getTitle() + "\"");
      }

      int height = 0;
      SDL_GetWindowSize(m_window, nullptr, &height);
      return height;
    }

    inline
    const std::string&
    SdlApplication::getTitle() const noexcept {
      return m_title;
    }

    inline
    void
    SdlApplication::setIcon(const std::string& icon) {
      if (m_window == nullptr) {
        throw AppException(std::string("Could not set icon for invalid sdl window"));
      }

      // Load this icon.
      SDL_Surface* iconAsSurface = SDL_LoadBMP(icon.c_str());
      if (iconAsSurface == nullptr) {
        throw AppException(std::string("Could not load icon \"") + icon + "\" (err: \"" + SDL_GetError() + "\")");
      }

      SDL_SetWindowIcon(m_window, iconAsSurface);

      SDL_FreeSurface(iconAsSurface);
    }

    inline
    std::string
    SdlApplication::getName() const noexcept {
      return m_name;
    }

    inline
    void
    SdlApplication::run() {
      // Start event handling.
      m_eventsHandler.run();

      // Start rendering.
      performRendering();
    }

    inline
    void
    SdlApplication::onQuitEvent(const SDL_QuitEvent& event) {
      std::lock_guard<std::mutex> guard(m_locker);
      m_renderingRunning = false;
    }

    inline
    void
    SdlApplication::addWidget(sdl::core::SdlWidgetShPtr widget) {
      if (widget == nullptr) {
        throw AppException(std::string("Cannot add null widget"));
      }

      std::lock_guard<std::mutex> guard(m_widgetsLocker);
      m_widgets[widget->getName()] = widget;
      m_eventsHandler.addListener(widget.get());
    }

    inline
    void
    SdlApplication::removeWidget(sdl::core::SdlWidgetShPtr widget) {
      if (widget == nullptr) {
        throw AppException(std::string("Cannot remove null widget"));
      }

      std::lock_guard<std::mutex> guard(m_widgetsLocker);
      m_widgets.erase(widget->getName());
      m_eventsHandler.removeListener(widget.get());
    }

    inline
    void
    SdlApplication::initializeSdlLib() const {
      if (SDL_WasInit(SDL_INIT_VIDEO) != 0) {
        return;
      }

      int initStatus = SDL_Init(SDL_INIT_VIDEO);
      if (initStatus != 0) {
        throw AppException(std::string("Could not initialize SDL library (err: \"") + SDL_GetError() + "\")");
      }
    }

    inline
    void
    SdlApplication::lock() {
      m_locker.lock();
    }

    inline
    void
    SdlApplication::unlock() {
      m_locker.unlock();
    }

    inline
    void
    SdlApplication::render() {
      const unsigned int startingRenderingTime = SDL_GetTicks();

      SDL_RenderClear(m_renderer);
      renderWidgets();
      SDL_RenderPresent(m_renderer);

      const unsigned int renderingDuration = SDL_GetTicks() - startingRenderingTime;

      if (renderingDuration > m_frameDuration) {
        utils::core::Logger::getInstance().logWarning(
          std::string("Frame took ") + std::to_string(renderingDuration) + "ms " +
          "which is greater than the " + std::to_string(m_frameDuration) + "ms " +
          " authorized to maintain " + std::to_string(m_framerate) + "fps",
          std::string("sdl_application")
        );
      }
      else {
        const unsigned int remainingDuration = m_frameDuration - renderingDuration;
        if (remainingDuration > 3u) {
          SDL_Delay(remainingDuration);
        }
      }
    }

    inline
    void
    SdlApplication::renderWidgets() {
      std::lock_guard<std::mutex> guard(m_widgetsLocker);

      for (std::unordered_map<std::string, sdl::core::SdlWidgetShPtr>::iterator widgetsIterator = m_widgets.begin() ;
          widgetsIterator != m_widgets.end() ;
          ++widgetsIterator)
      {
        sdl::core::SdlWidgetShPtr widget = widgetsIterator->second;

        // Draw this object (caching is handled by the object itself).
        SDL_Renderer* renderer = m_renderer;
        utils::core::launchProtected(
          [renderer, widget]() {
            SDL_Texture* texture = widget->draw(renderer);
            const utils::maths::Boxf render = widget->getRenderingArea();
            SDL_Rect dstArea = utils::sdl::toSDLRect(render);
            SDL_RenderCopy(renderer, texture, nullptr, &dstArea);
          },
          std::string("draw_widget"),
          std::string("sdl_application")
        );
      }
    }

  }
}

#endif    /* SDLAPPLICATION_HXX */
