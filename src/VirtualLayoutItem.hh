#ifndef    VIRTUAL_LAYOUT_ITEM_HH
# define   VIRTUAL_LAYOUT_ITEM_HH

# include <memory>
# include <sdl_core/LayoutItem.hh>

namespace sdl {
  namespace app {

    class VirtualLayoutItem: public core::LayoutItem {
      public:

        VirtualLayoutItem(const std::string& name,
                          const utils::Sizef& min = utils::Sizef(),
                          const utils::Sizef& hint = utils::Sizef(),
                          const utils::Sizef& max = utils::Sizef::max(),
                          const core::SizePolicy& policy = core::SizePolicy());

        virtual ~VirtualLayoutItem();

        /**
         * @brief - Indicates that this item should manage the width of another item.
         *          This indicates that received `Resize` events should only be considered
         *          for their width component.
         *          Note that the `setManageHeight` can also be activated at the same time,
         *          both flags are not mutually exclusive.
         * @param managed - true if the width for this virtual layout item is managed,
         *                  false otherwise.
         */
        void
        setManageWidth(const bool managed) noexcept;

        bool
        isWidthManaged() const noexcept;

        /**
         * @brief - Used to assign a position along the x cooridnate for this virtual layout
         *          item. Note that this function doesn't have any effect if the width of this
         *          item is set to managed.
         * @param x - the x coordinate to assign to this layout item.
         */
        void
        setX(const float& x) noexcept;

        /**
         * @brief - Used to assign a width for this virtual layout item. Note that this function
         *          doesn't have any effect if the width of this item is set to managed.
         * @param width - the width to assign to this layout item.
         */
        void
        setWidth(const float& width) noexcept;

        /**
         * @brief - Indicates that this item should manage the height of another item.
         *          This indicates that received `Resize` events should only be considered
         *          for their height component.
         *          Note that the `setManageWidth` can also be activated at the same time,
         *          both flags are not mutually exclusive.
         * @param managed - true if the width for this virtual layout item is managed,
         *                  false otherwise.
         */
        void
        setManageHeight(const bool managed) noexcept;

        bool
        isHeightManaged() const noexcept;

        /**
         * @brief - Used to assign a position along the y cooridnate for this virtual layout
         *          item. Note that this function doesn't have any effect if the height of this
         *          item is set to managed.
         * @param y - the y coordinate to assign to this layout item.
         */
        void
        setY(const float& y) noexcept;

        /**
         * @brief - Used to assign a height for this virtual layout item. Note that this
         *          function doesn't have any effect if the height of this item is set
         *          to managed.
         * @param height - the height to assign to this layout item.
         */
        void
        setHeight(const float& height) noexcept;

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

        /**
         * @brief - Reimplementation of the base `LayoutItem` method in order to provide
         *          a direct application of the visible status. Indeed as the virtual
         *          layout item does not process events at all we wouldn't get notified
         *          of the change in case we relied on the basic process.
         *          For this item calling `isVisible` right after calling this method
         *          does return the value set by `visible` in here.
         * @param visible - the visibility status to apply to this layout item.
         */
        void
        setVisible(bool visible) noexcept override;

        /**
         * @brief - Used to update the internal maximum size based on the value provided in
         *          the input `upperBound` size. This new size will replace the old maixmum
         *          size in case it is smaller than the initial value.
         *          Of course we also take care of the minimum size so that we keep a valid
         *          configuration for this widget.
         *          Note that if no valid configuration can be found, an error is raised.
         * @param upperBound - the maximum size which should be checked against the internal
         *                     maximum size.
         */
        void
        updateMaxSize(const utils::Sizef& upperBound);

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
