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
    MainWindowLayout::removeDockWidget(core::SdlWidget* item) {
      // Before trying to remove the widget, we need to first determine
      // its precise role. Indeed based on the area in which the widget
      // is located, its role will be different.
      // So first, try to retrieve the index of this widget inside the
      // internal array.
      const int id = getIndexOf(item);

      // Check whether we could find this item.
      if (!isValidIndex(id)) {
        error(
          std::string("Cannot get index for item \"") + item->getName() + "\" from layout",
          std::string("Widget is not managed by this layout")
        );
      }

      // Check that this item is registered in the information array.
      InfosMap::const_iterator info = m_infos.find(id);
      if (info == m_infos.cend()) {
        error(
          std::string("Cannot retrieve role for item \"") + item->getName() + "\"",
          std::string("Inexisting key")
        );
      }

      // Check whether the role for this item is actually a dock widget. In
      // any other case we abort the deletion of the item as it is not what
      // is expected by the caller.
      if (!isDockWidgetRole(info->second.role)) {
        error(
          std::string("Could not remove item \"") + item->getName() + "\" which is not a dock widget",
          std::string("Role \"") + roleToName(info->second.role) + " is not a valid dock widget role"
        );
      }

      // The input item is actually a dock widget we can remove it.
      removeItem(item);
    }

    inline
    void
    MainWindowLayout::updatePrivate(const utils::Boxf& window) {
      // And if some items are managed by this layout.
      if (empty()) {
        return;
      }

      // Proceed by activating the internal handler.
      computeGeometry(window);
    }

    inline
    void
    MainWindowLayout::removeItemFromIndex(int item) {
      // We need to both remove the item using the base handler and also remove the
      // corresponding entry in the internal information map.

      // Remove the item using the base class handler.
      graphic::GridLayout::removeItemFromIndex(item);

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
    utils::Boxi
    MainWindowLayout::getGridCoordinatesFromRole(const WidgetRole& role) const {
      switch (role) {
        case WidgetRole::MenuBar:
          return utils::Boxi(0, 0, 3, 1);
        case WidgetRole::ToolBar:
          return utils::Boxi(0, 1, 3, 1);
        case WidgetRole::LeftDockWidget:
          return utils::Boxi(0, 2, 1, 3);
        case WidgetRole::RightDockWidget:
          return utils::Boxi(2, 2, 1, 3);
        case WidgetRole::TopDockWidget:
          return utils::Boxi(1, 2, 1, 1);
        case WidgetRole::CentralDockWidget:
          return utils::Boxi(1, 3, 1, 1);
        case WidgetRole::BottomDockWidget:
          return utils::Boxi(1, 4, 1, 1);
        case WidgetRole::StatusBar:
          return utils::Boxi(0, 5, 3, 1);
        default:
          break;
      }

      error(
        std::string("Cannot determine grid coordinates for role \"") + roleToName(role) + "\"",
        std::string("Unknown role")
      );
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
      // Retrieve the coordinates for this role.
      utils::Boxi gridCoords = getGridCoordinatesFromRole(role);

      // Add the item using the base handler.
      const int index = addItem(widget, gridCoords.x(), gridCoords.y(), gridCoords.w(), gridCoords.h());

      // TODO: We should probably create a tab widget with all relevant widgets
      // in case a single dock widget area contains more than one widget.
      // Register this item in the internal table of information if
      // a valid index was generated.
      if (index >= 0) {
        m_infos[index] = ItemInfo{
          role,
          area,
          widget
        };
      }
    }

  }
}

#endif    /* MAIN_WINDOW_LAYOUT_HXX */
