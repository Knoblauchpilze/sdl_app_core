#ifndef    MAIN_WINDOW_LAYOUT_HXX
# define   MAIN_WINDOW_LAYOUT_HXX

# include <sdl_core/SdlWidget.hh>
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    inline
    void
    MainWindowLayout::setMenuBar(core::SdlWidget* item) {
      // Remove existing menu bar.
      removeAll(WidgetRole::MenuBar);

      // Use internal handler.
      addItemWithRoleAndArea(item, WidgetRole::MenuBar);
    }

    inline
    void
    MainWindowLayout::addToolBar(core::SdlWidget* item) {
      // Use internal handler.
      addItemWithRoleAndArea(item, WidgetRole::ToolBar);
    }

    inline
    void
    MainWindowLayout::setCentralWidget(core::SdlWidget* item) {
      // Remove existing central widget.
      removeAll(WidgetRole::CentralDockWidget);

      // Use internal handler.
      addItemWithRoleAndArea(item, WidgetRole::CentralDockWidget, DockWidgetArea::CentralArea);
    }

    inline
    void
    MainWindowLayout::addDockWidget(core::SdlWidget* item,
                                    const DockWidgetArea& area)
    {
      // Determine the role from the input `area` and use the internal handler.
      addItemWithRoleAndArea(item, roleFromArea(area), area);
    }

    inline
    void
    MainWindowLayout::setStatusBar(core::SdlWidget* item) {
      // Remove existing status bar.
      removeAll(WidgetRole::StatusBar);

      // Use internal handler.
      addItemWithRoleAndArea(item, WidgetRole::StatusBar);
    }

    inline
    void
    MainWindowLayout::removeToolBar(core::SdlWidget* item) {
      // Remove it using the dedicated handler.
      removeItemFromRole(item, WidgetRole::ToolBar);
    }

    inline
    void
    MainWindowLayout::setEventsQueue(core::engine::EventsQueue* queue) noexcept {
      // Use the base handler to register `this` to the provided `queue`.
      core::engine::EngineObject::setEventsQueue(queue);

      // Register both children layout to this queue.
      registerToSameQueue(&m_hLayout);
      registerToSameQueue(&m_vLayout);
    }

    inline
    void
    MainWindowLayout::removeItemFromIndex(int item) {
      // We need to both remove the item using the base handler and also remove the
      // corresponding entry in the internal information map.

      // Remove the item using the base class handler.
      core::Layout::removeItemFromIndex(item);

      // Erase the corresponding entry in the internal table.
      const std::size_t count = m_infos.erase(item);

      // Check whether we could remove the input item.
      if (count != 1) {
        log(
          std::string("Invalid removed item count while deleting item ") + std::to_string(item) +
          std::string("(removed ") + std::to_string(count) + " item(s))",
          utils::Level::Warning
        );
      }
    }

    inline
    std::pair<bool, bool>
    MainWindowLayout::dimensionManagedForRole(const WidgetRole& role) const noexcept {
      bool manageWidth = false;
      bool manageHeight = false;

      switch (role) {
        case WidgetRole::MenuBar:
        case WidgetRole::ToolBar:
        case WidgetRole::StatusBar:
          manageHeight = true;
          break;
        case WidgetRole::LeftDockWidget:
        case WidgetRole::RightDockWidget:
          manageWidth = true;
          break;
        case WidgetRole::TopDockWidget:
        case WidgetRole::CentralDockWidget:
        case WidgetRole::BottomDockWidget:
          manageWidth = true;
          manageHeight = true;
          break;
        default:
          break;
      }

      // Return the computed values as a pair.
      return std::make_pair(manageWidth, manageHeight);
    }

    inline
    utils::Boxi
    MainWindowLayout::getGridCoordinatesFromRole(const WidgetRole& role,
                                                 const bool hRole) const
    {
      if (hRole) {
        switch (role) {
          case WidgetRole::LeftDockWidget:
            return utils::Boxi(0, 0, 1, 3);
          case WidgetRole::TopDockWidget:
            return utils::Boxi(1, 0, 1, 1);
          case WidgetRole::CentralDockWidget:
            return utils::Boxi(1, 1, 1, 1);
          case WidgetRole::BottomDockWidget:
            return utils::Boxi(1, 2, 1, 1);
          case WidgetRole::RightDockWidget:
            return utils::Boxi(2, 0, 1, 3);
          default:
            break;
        }
      }

      if (!hRole) {
        switch (role) {
          case WidgetRole::MenuBar:
            return utils::Boxi(0, 0, 1, 1);
          case WidgetRole::ToolBar:
            return utils::Boxi(0, 1, 1, 1);
          case WidgetRole::TopDockWidget:
            return utils::Boxi(0, 2, 1, 1);
          case WidgetRole::CentralDockWidget:
            return utils::Boxi(0, 3, 1, 1);
          case WidgetRole::BottomDockWidget:
            return utils::Boxi(0, 4, 1, 1);
          case WidgetRole::StatusBar:
            return utils::Boxi(0, 5, 1, 1);
          default:
            break;
        }
      }

      error(
        std::string("Cannot determine grid coordinates for role \"") + roleToName(role) + "\"",
        std::string("Unknown role")
      );

      // Silent compiler, even though `error` will throw.
      return utils::Boxi();
    }

    inline
    utils::Sizef
    MainWindowLayout::computeMaxSizeForRole(const utils::Sizef& window,
                                            const WidgetRole& role) const
    {
      // Each role provides a maximum size either through width or height regulation.
      // At first assumes we can take up all the input window and adjust using the
      // provided `role`.

      float wMax = window.w();
      float hMax = window.h();

      // Apply width constraints.
      switch (role) {
        case WidgetRole::LeftDockWidget:
          wMax = window.w() * m_leftAreaPercentage;
          break;
        case WidgetRole::RightDockWidget:
          wMax = window.w() * m_rightAreaPercentage;
          break;
        default:
          break;
      }

      // Apply height constraints.
      switch (role) {
        case WidgetRole::MenuBar:
          hMax = window.h() * m_menuBarPercentage;
          break;
        case WidgetRole::ToolBar:
          hMax = window.h() * m_toolBarPercentage;
          break;
        case WidgetRole::TopDockWidget:
          hMax = window.h() * m_topAreaPercentage;
          break;
        case WidgetRole::BottomDockWidget:
          hMax = window.h() * m_bottomAreaPercentage;
          break;
        case WidgetRole::StatusBar:
          hMax = window.h() * m_statusBarPercentage;
          break;
        default:
          break;
      }

      // Return the maximum size for this role.
      return utils::Sizef(wMax, hMax);
    }

    inline
    void
    MainWindowLayout::addItemWithRoleAndArea(core::SdlWidget* widget,
                                             const WidgetRole& role,
                                             const DockWidgetArea& area)
    {
      // Add the item using the base handler.
      const int index = addItem(widget);

      // Check whether the widget could be inserted in the layout.
      if (index < 0) {
        return;
      }

      // Create the virtual layout item associated to this widget. Based on the
      // role and area of the input `widget` we will add the virtual item into
      // the internal layouts.
      // Based on the role of the widget, the virtual layout item wil be set to
      // only care about specific dimensions.

      // Define properties for the virtual layout item based on the role which
      // shall be assumed by the widget.
      std::pair<bool, bool> manageDims = dimensionManagedForRole(role);

      // Retrieve the grid coordinates based on the assumed `role` of the widget.
      // We also need to assign the size hints for this item. The important part
      // is to keep values from the input `widget` and to account for the maximum
      // size available based on the `role`.
      // Right now we cannot guarantee that the area defined for this layout is
      // the one which will be used during the optimization process. We will use
      // the maximum size of the `widget` for now and update it when the relevant
      // `computeGeometry` call is processed.

      VirtualLayoutItemShPtr item = std::make_shared<VirtualLayoutItem>(
        widget->getName(),
        widget->getMinSize(),
        widget->getSizeHint(),
        widget->getMaxSize(),
        widget->getSizePolicy()
      );
      item->setVisible(widget->isVisible());

      // Register the widget in the corresponding layouts.
      if (manageDims.first) {
        utils::Boxi box = getGridCoordinatesFromRole(role, true);
        item->setManageWidth(true);
        m_hLayout.addItem(item.get(), box.x(), box.y(), box.w(), box.h());
      }

      if (manageDims.second) {
        utils::Boxi box = getGridCoordinatesFromRole(role, false);
        item->setManageHeight(true);
        m_vLayout.addItem(item.get(), box.x(), box.y(), box.w(), box.h());
      }

      // Register this item in the internal table of information if a valid index
      // was generated.
      m_infos[index] = ItemInfo{
        role,
        area,
        widget,
        item
      };
    }

  }
}

#endif    /* MAIN_WINDOW_LAYOUT_HXX */
