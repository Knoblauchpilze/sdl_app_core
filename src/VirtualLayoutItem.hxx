#ifndef    VIRTUAL_LAYOUT_ITEM_HXX
# define   VIRTUAL_LAYOUT_ITEM_HXX

# include "VirtualLayoutItem.hh"

namespace sdl {
  namespace app {

    inline
    VirtualLayoutItem::~VirtualLayoutItem() {}

    inline
    void
    VirtualLayoutItem::setManageWidth() noexcept {
      m_manageWidth = true;
    }

    inline
    bool
    VirtualLayoutItem::isWidthManaged() const noexcept {
      return m_manageWidth;
    }

    inline
    void
    VirtualLayoutItem::setManageHeight() noexcept {
      m_manageHeight = true;
    }

    inline
    bool
    VirtualLayoutItem::isHeightManaged() const noexcept {
      return m_manageHeight;
    }

    inline
    utils::Boxf
    VirtualLayoutItem::getRenderingArea() const noexcept {
      return m_box;
    }

  }
}

#endif    /* VIRTUAL_LAYOUT_ITEM_HXX */
