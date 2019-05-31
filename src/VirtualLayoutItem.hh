#ifndef    VIRTUAL_LAYOUT_ITEM_HH
# define   VIRTUAL_LAYOUT_ITEM_HH

# include <memory>
# include <sdl_core/LayoutItem.hh>

namespace sdl {
  namespace app {

    class VirtualLayoutItem: public core::LayoutItem {
      public:

        VirtualLayoutItem(const std::string& name,
                          const utils::Sizef& hint = utils::Sizef());

        virtual ~VirtualLayoutItem();

        /**
         * @brief - Indicates that this item should manage the width of another item.
         *          This indicates that received `Resize` events should only be considered
         *          for their width component.
         *          Note that the `setManageHeight` can also be activated at the same time,
         *          both flags are not mutually exclusive.
         */
        void
        setManageWidth() noexcept;

        bool
        isWidthManaged() const noexcept;

        /**
         * @brief - Indicates that this item should manage the height of another item.
         *          This indicates that received `Resize` events should only be considered
         *          for their height component.
         *          Note that the `setManageWidth` can also be activated at the same time,
         *          both flags are not mutually exclusive.
         */
        void
        setManageHeight() noexcept;

        bool
        isHeightManaged() const noexcept;

        void
        postLocalEvent(core::engine::EventShPtr e) override;

        /**
         * @brief - Used to retrieve the virtual box computed from all the `Resize` events
         *          received by this item and considering the internal flags indicating
         *          whether the width or height should be retrieved.
         *          This is a reimplementation of the base `LayoutItem` method in order to
         *          retrieve the virtual box instead of the actual rendering area.
         * @return - the box computed from `Resize` event.
         */
        utils::Boxf
        getRenderingArea() const noexcept override;

      private:

        bool m_manageWidth;
        bool m_manageHeight;

        utils::Boxf m_box;
    };

    using VirtualLayoutItemShPtr = std::shared_ptr<VirtualLayoutItem>;
  }
}

# include "VirtualLayoutItem.hxx"

#endif    /* VIRTUAL_LAYOUT_ITEM_HH */
