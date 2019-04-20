
# include "AppDecorator.hh"

namespace sdl {
  namespace app {

    AppDecorator::AppDecorator(core::engine::EngineShPtr engine,
                               const utils::Uuid& canvas,
                               const core::engine::Palette& palette,
                               const utils::Uuid& window):
      core::engine::EngineDecorator(engine, std::string("app_decorator")),
      m_canvas(canvas),
      m_palette(palette),
      m_window(window)
    {}

    AppDecorator::~AppDecorator() {
      // Destroy the window and main canvases if any.
      if (m_canvas.valid()) {
        destroyTexture(m_canvas);
      }
      if (m_window.valid()) {
        destroyWindow(m_window);
      }
    }

  }
}

