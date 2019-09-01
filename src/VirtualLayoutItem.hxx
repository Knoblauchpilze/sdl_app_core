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
    VirtualLayoutItem::setX(const float& x) noexcept {
      m_box.x() = x;
    }

    inline
    void
    VirtualLayoutItem::setWidth(const float& width) noexcept {
      // Only assign if the width of this item is not managed.
      if (!isWidthManaged()) {
        m_box.w() = width;
      }
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
    void
    VirtualLayoutItem::setY(const float& y) noexcept {
      m_box.y() = y;
    }

    inline
    void
    VirtualLayoutItem::setHeight(const float& height) noexcept {
      // Only assign if the height of this item is not managed.
      if (!isHeightManaged()) {
        m_box.h() = height;
      }
    }

    inline
    utils::Boxf
    VirtualLayoutItem::getRenderingArea() const noexcept {
      return m_box;
    }

    inline
    void
    VirtualLayoutItem::setVisible(bool visible) noexcept {
      // We wil trigger directly the `showEvent` or `hideEvent` handler based
      // on the input `visible` value.
      if (visible) {
        core::engine::Event s(core::engine::Event::Type::Show, this);
        s.setEmitter(this);

        showEvent(s);
      }
      else {
        core::engine::Event h(core::engine::Event::Type::Hide, this);
        h.setEmitter(this);

        hideEvent(h);
      }
    }

  }
}

#endif    /* VIRTUAL_LAYOUT_ITEM_HXX */
