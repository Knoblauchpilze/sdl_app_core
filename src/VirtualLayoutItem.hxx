#ifndef    VIRTUAL_LAYOUT_ITEM_HXX
# define   VIRTUAL_LAYOUT_ITEM_HXX

# include "VirtualLayoutItem.hh"

namespace sdl {
  namespace app {

    inline
    VirtualLayoutItem::~VirtualLayoutItem() {}

    inline
    void
    VirtualLayoutItem::setManageWidth(const bool managed) noexcept {
      m_manageWidth = managed;
    }

    inline
    bool
    VirtualLayoutItem::isWidthManaged() const noexcept {
      return m_manageWidth;
    }

    inline
    void
    VirtualLayoutItem::setManageHeight(const bool managed) noexcept {
      m_manageHeight = managed;
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

    inline
    void
    VirtualLayoutItem::makeGeometryDirty() {}

  }
}

#endif    /* VIRTUAL_LAYOUT_ITEM_HXX */
