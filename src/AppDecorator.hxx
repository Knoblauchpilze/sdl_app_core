#ifndef    APP_DECORATOR_HXX
# define   APP_DECORATOR_HXX

# include "AppDecorator.hh"

namespace sdl {
  namespace app {

    inline
    void
    AppDecorator::setDrawingCanvas(const utils::Uuid& canvas) {
      // Check that we're assigning a valid canvas.
      if (!canvas.valid()) {
        error(std::string("Cannot assign invalid canvas"));
      }

      m_canvas = canvas;
    }

    inline
    void
    AppDecorator::clearWindow(const utils::Uuid& /*uuid*/) {
      if (!m_canvas.valid()) {
        error(std::string("Cannot clear invalid canvas"));
      }

      core::engine::EngineDecorator::fillTexture(m_canvas, m_palette);
    }

    inline
    void
    AppDecorator::renderWindow(const utils::Uuid& uuid) {
      // Check whether the canvas is valid before trying
      // to render the window.
      if (!m_canvas.valid()) {
        error(std::string("Cannot render invalid canvas"));
      }

      // Render the canvas onto the screen.
      core::engine::EngineDecorator::drawTexture(m_canvas, nullptr, nullptr, nullptr);

      // Render the window.
      core::engine::EngineDecorator::renderWindow(uuid);
    }

    inline
    utils::Uuid
    AppDecorator::createTexture(const utils::Uuid& /*win*/,
                                const utils::Sizef& size,
                                const core::engine::Palette::ColorRole& role)
    {
      return core::engine::EngineDecorator::createTexture(m_window, size, role);
    }

    inline
    utils::Uuid
    AppDecorator::createTexture(const utils::Sizef& size,
                                const core::engine::Palette::ColorRole& role)
    {
      return core::engine::EngineDecorator::createTexture(m_window, size, role);
    }

    inline
    utils::Uuid
    AppDecorator::createTextureFromFile(const utils::Uuid& /*win*/,
                                        core::engine::ImageShPtr img,
                                        const core::engine::Palette::ColorRole& role)
    {
      return core::engine::EngineDecorator::createTextureFromFile(m_window, img, role);
    }

    inline
    utils::Uuid
    AppDecorator::createTextureFromFile(core::engine::ImageShPtr img,
                                        const core::engine::Palette::ColorRole& role)
    {
      return core::engine::EngineDecorator::createTextureFromFile(m_window, img, role);
    }

    inline
    utils::Uuid
    AppDecorator::createTextureFromText(const utils::Uuid& /*win*/,
                                        const std::string& text,
                                        const utils::Uuid& font,
                                        const core::engine::Palette::ColorRole& role)
    {
      return core::engine::EngineDecorator::createTextureFromText(m_window, text, font, role);
    }

    inline
    utils::Uuid
    AppDecorator::createTextureFromText(const std::string& text,
                                        const utils::Uuid& font,
                                        const core::engine::Palette::ColorRole& role)
    {
      return core::engine::EngineDecorator::createTextureFromText(m_window, text, font, role);
    }

    inline
    void
    AppDecorator::drawTexture(const utils::Uuid& tex,
                              const utils::Boxf* from,
                              const utils::Uuid* on,
                              const utils::Boxf* where)
    {
      // Check whether the `on` is null. In this case we should override
      // the settings so that we draw on the internal canvas.
      // The real `m_canvas` is only used when we need to actually repaint
      // the window and make the content displayed on it visible.
      if (on == nullptr && m_canvas.valid()) {
        core::engine::EngineDecorator::drawTexture(tex, from, &m_canvas, where);
        return;
      }

      core::engine::EngineDecorator::drawTexture(tex, from, on, where);
    }

  }
}

#endif    /* APP_DECORATOR_HXX */
