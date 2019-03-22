
# include "AppDecorator.hh"

namespace sdl {
  namespace app {

    AppDecorator::AppDecorator(core::engine::EngineShPtr engine,
                               const utils::Uuid& canvas,
                               const utils::Uuid& window):
      core::engine::EngineDecorator(engine),
      m_canvas(canvas),
      m_window(window)
    {}

    AppDecorator::~AppDecorator() {
      // Destroy the window and main canvas if any.
      if (m_canvas.valid()) {
        destroyTexture(m_canvas);
      }
      if (m_window.valid()) {
        destroyWindow(m_window);
      }
    }

  }
}

