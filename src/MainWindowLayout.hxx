#ifndef    MAIN_WINDOW_LAYOUT_HXX
# define   MAIN_WINDOW_LAYOUT_HXX

# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    inline
    void
    MainWindowLayout::setArea(const utils::Boxf& area) noexcept {
      // Assign the area.
      m_area = area;

      // Invalidate the layout.
      invalidate();
    }

    inline
    void
    MainWindowLayout::update() {
      // Check if this layout is dirty.
      if (!m_dirty) {
        return;
      }

      // And if some items are managed by this layout.
      if (m_items.empty()) {
        return;
      }

      // Update with private handler.
      updatePrivate(m_area);

      m_dirty = false;
    }

  }
}

#endif    /* MAIN_WINDOW_LAYOUT_HXX */
