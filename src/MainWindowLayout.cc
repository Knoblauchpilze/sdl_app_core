
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    MainWindowLayout::MainWindowLayout(const float& margin,
                                       const utils::Sizef& centralWidgetSize):
      core::Layout(std::string("main_window_layout"), nullptr, margin),
      m_infos(),

      m_leftAreaPercentage(),
      m_rightAreaPercentage(),

      m_menuBarPercentage(),
      m_toolBarPercentage(),
      m_topAreaPercentage(),
      m_bottomAreaPercentage(),
      m_statusBarPercentage(),

      m_hLayout(std::string("m_hLayout"), nullptr, 3u, 3u, margin),
      m_vLayout(std::string("m_vLayout"), nullptr, 1u, 6u, margin)
    {
      // Assign the percentages from the input central widget size.
      assignPercentagesFromCentralWidget(centralWidgetSize);

      setService("main_layout");

      // Assign events queue to internal layouts.
      registerToSameQueue(&m_hLayout);
      registerToSameQueue(&m_vLayout);

      // Update properties of layouts.
      setBoxesFormat(core::Layout::BoxesFormat::Window);

      m_hLayout.allowLog(false);
      m_vLayout.allowLog(false);
    }

    MainWindowLayout::~MainWindowLayout() {}

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

    void
    MainWindowLayout::computeGeometry(const utils::Boxf& window) {
      // To fully build the layout we need to compute the repartition in both direction (horizontal and
      // vertical) using the two internal layouts.
      // Each layout will use virtual layout item so that we do not pollute the existing widgets with
      // unecessary events.
      // Once this is done, we need to retrieve the properties of each area and build the final areas
      // for each widget.
      // We use the standard process to subtract the margin from the input size and to compute information
      // about the widgets, so that we get a way to iterate on registered widgets.

      const utils::Sizef internalSize = computeAvailableSize(window);

      // Retrieve widgets' info.
      std::vector<core::Layout::WidgetInfo> infos = computeItemsInfo();

      // We need to update the maximum size of each virtual layout item based on the inpu `window` size.
      // This will ensure that each individual virtual item is set up with up to date information regarding
      // its size.
      // We need to traverse each virtual layout item and use the dedicated handler to update the maximum
      // size to its latest value.
      // In addition to that we need to update the visibility status of the virtual layout item based on the
      // current visibility status of each widget.
      for (InfosMap::const_iterator widgetInfo = m_infos.cbegin() ;
           widgetInfo != m_infos.cend() ;
           ++widgetInfo)
      {
        // Update max size.
        utils::Sizef maxFromLayout = computeMaxSizeForRole(internalSize, widgetInfo->second.role);
        widgetInfo->second.item->updateMaxSize(maxFromLayout);

        // Update visibility status.
        widgetInfo->second.item->setVisible(infos[widgetInfo->first].visible);
      }

      // Compute geometry of internal layouts. Virtual layout item need to be set up in order to care about
      // the modification of width or height based on the layout which is currently applied to them.
      // For instance, the virtual layout item representing the central dock widget is registered in both
      // the horizontal layout and the vertical layout. However we do not want the width computed by the
      // `m_vLayout` to override the value computed by the `m_hLayout`. Thus we need to manually set the
      // the manage dimensions flags between calls to each layout.

      // Activate width management for each widget role.
      for (InfosMap::const_iterator widgetInfo = m_infos.cbegin() ;
           widgetInfo != m_infos.cend() ;
           ++widgetInfo)
      {
        std::pair<bool, bool> manageDims = dimensionManagedForRole(widgetInfo->second.role);
        widgetInfo->second.item->setManageWidth(manageDims.first);

        if (manageDims.first) {
          utils::Sizef maxFromLayout = computeMaxSizeForRole(internalSize, widgetInfo->second.role);
          widgetInfo->second.item->updateMaxSize(maxFromLayout);
        }
        else {
          // The role do not support width management, we thus need to assign the total width
          // available to the virtual layout item.
          widgetInfo->second.item->setX(0.0f);
          widgetInfo->second.item->setWidth(internalSize.w());
        }
      }

      log("Updating h layout", utils::Level::Notice);
      m_hLayout.update(window);

      // Activate height management for each widget role. Also, deactivate width management for each widget.
      for (InfosMap::const_iterator widgetInfo = m_infos.cbegin() ;
           widgetInfo != m_infos.cend() ;
           ++widgetInfo)
      {
        std::pair<bool, bool> manageDims = dimensionManagedForRole(widgetInfo->second.role);

        widgetInfo->second.item->setManageHeight(manageDims.second);
        widgetInfo->second.item->setManageWidth(false);

        if (manageDims.second) {
          utils::Sizef maxFromLayout = computeMaxSizeForRole(internalSize, widgetInfo->second.role);
          widgetInfo->second.item->updateMaxSize(maxFromLayout);
        }
        else {
          // The role do not support height management, we thus need to assign the total height
          // available to the virtual layout item.
          widgetInfo->second.item->setY(0.0f);
          widgetInfo->second.item->setHeight(internalSize.h());
        }
      }

      log("Updating v layout", utils::Level::Notice);
      m_vLayout.update(window);

      // Now build the area to assign to each widget based on the internal virtual items. There are
      // only two exceptions to the general process: the left and right dock widget. Indeed we have
      // to not manage the height of these widgets because of the following:
      //
      // Imagine a situation where we have a left dock widget and a central widget. We will only
      // consider the heigth aspect of the problem (as the width part works just fine).
      // The representation in terms of grid coordinates in the vertical layout is as below:
      //
      //  +------+-------------------+
      //  |      |   Inexisting top  |
      //  |      +-------------------+
      //  |      |                   |
      //  | Left |   Central widget  |
      //  |      |                   |
      //  |      +-------------------+
      //  |      | Inexisting bottom |
      //  +------+-------------------+
      //
      // The grid layout during the optimization process will assign only a third of the available area
      // to the central widget because the `LeftDockWidget` would exist in the row 0 and 2 which will
      // result in an incorrect layout. At least not what we would expect.
      //
      // The solution to this problem is to make the left dock area not managed in height: this way it
      // will be assigned the total height available. But if we now consider the following situation:
      //
      //  +--------------------------+
      //  |        Menu bar          |
      //  +------+-------------------+
      //  |      |   Inexisting top  |
      //  |      +-------------------+
      //  |      |                   |
      //  | Left |   Central widget  |
      //  |      |                   |
      //  |      +-------------------+
      //  |      | Inexisting bottom |
      //  +------+-------------------+
      //
      // We see that even this solution is not without flaws. Indeed we do not want to assign a height
      // corresponding to all the available height to the left dock widget but rather a height which
      // corresponds to the combined height of the top, central and bottom dock areas.
      // Note that this also applies to the right dock area.
      // So when encountering such a widget, we need to perform the needed computations to assign a valid
      // height and ordinate to these dock areas.

      // Perform the computations to determine the height and ordinate to assign to top and right dock
      // areas beforehand.
      float offsetOrdinate = std::numeric_limits<float>::lowest();
      float heightMenu = 0.0f;
      float heightTool = 0.0f;
      float heightTop = 0.0f;
      float heightCentral = 0.0f;
      float heightBottom = 0.0f;
      float heightStatus = 0.0f;

      bool noInfoForDockAreas = true;

      for (InfosMap::const_iterator widgetInfo = m_infos.cbegin() ;
           widgetInfo != m_infos.cend() ;
           ++widgetInfo)
      {
        const WidgetRole& role = widgetInfo->second.role;
        const utils::Boxf area = widgetInfo->second.item->getRenderingArea();

        // Check whether this item is useful for our computation: we keep track of the
        // largest widget encountered so far for each role.
        if (role == WidgetRole::MenuBar && area.h() >= heightMenu) {
          heightMenu = area.h();
        }

        if (role == WidgetRole::ToolBar && area.h() >= heightTool) {
          heightTool = area.h();
        }

        if (role == WidgetRole::TopDockWidget && area.h() >= heightTop) {
          heightTop = area.h();
        }

        if (role == WidgetRole::CentralDockWidget && area.h() >= heightCentral) {
          heightCentral = area.h();
        }

        if (role == WidgetRole::BottomDockWidget && area.h() >= heightBottom) {
          heightBottom = area.h();
        }

        if (role == WidgetRole::StatusBar && area.h() >= heightStatus) {
          heightStatus = area.h();
        }

        // We also keep track of the smallest ordinate which is not taken by any role above
        // the left and right dock areas.
        if (role == WidgetRole::MenuBar && area.y() - area.h() / 2.0f > offsetOrdinate) {
          noInfoForDockAreas = false;
          offsetOrdinate = area.y() - area.h() / 2.0f;
        }

        if (role == WidgetRole::ToolBar && area.y() - area.h() / 2.0f > offsetOrdinate) {
          noInfoForDockAreas = false;
          offsetOrdinate = area.y() - area.h() / 2.0f;
        }

        if (role == WidgetRole::MenuBar && area.y() - area.h() / 2.0f > offsetOrdinate) {
          noInfoForDockAreas = false;
          offsetOrdinate = area.y() - area.h() / 2.0f;
        }
      }

      // Gather final values from each virtual item to assign to left and right dock areas.
      float combinedHeight = heightTop + heightCentral + heightBottom;
      if (utils::fuzzyEqual(combinedHeight, 0.0f)) {
        combinedHeight = internalSize.h() - (heightMenu + heightTool + heightStatus);
      }
      if (noInfoForDockAreas) {
        offsetOrdinate = 0.0f;
      }
      else {
        offsetOrdinate = offsetOrdinate - combinedHeight / 2.0f;
      }

      std::vector<utils::Boxf> boxes(m_infos.size());

      for (InfosMap::const_iterator widgetInfo = m_infos.cbegin() ;
           widgetInfo != m_infos.cend() ;
           ++widgetInfo)
      {
        // Retrieve the widget's info.
        const ItemInfo& info = widgetInfo->second;

        // Check for special case of left and right dock areas.
        if (info.role == WidgetRole::LeftDockWidget || info.role == WidgetRole::RightDockWidget) {
          info.item->setY(offsetOrdinate);
          info.item->setHeight(combinedHeight);
        }

        // The box is obtained directly through the virtual layout item associated to this widget.
        boxes[widgetInfo->first] = info.item->getRenderingArea();
      }

      // Assign the areas using the dedicated handler.
      assignRenderingAreas(boxes, window);
    }

    void
    MainWindowLayout::removeAll(const WidgetRole& role) {
      // Traverse the internal table of content and remove each one which role matches
      // the input value. The process is not as straightforward as it seems as we cannot
      // really rely on the index of the item to perform the removal.
      // Indeed consider the following situation:
      //
      // Content of `m_items`:
      //
      // m_items[0] = role
      // m_items[1] = not_role
      // m_items[2] = role
      //
      // Content of `m_infos`:
      //
      // m_infos[0] = widget_0
      // m_infos[1] = widget_1
      // m_infos[2] = widget_2
      //
      // We need to remove the item 0 and 2. If we just traverse the internal list of info,
      // select the one matching the input `role` and then sequentially perform the removal
      // of each one, we will run into trouble because the indices of the items will change
      // precisely because of the removal of other items.
      // So instead we have to rely on a less efficient strategy to remove one item at a
      // time and continue removing items until no more can be found for this role.

      // Iterate until no more items are found for the input `role`.
      bool stillWidgetWithRole = true;
      do {
        // Create an iterator to traverse the internal table.
        InfosMap::const_iterator info = m_infos.cbegin();
        while (info != m_infos.cend() && info->second.role != role) {
          ++info;
        }

        // Check why we're out of the loop. There are two main scenarii:
        // 1) we reached the end of the internal table.
        // 2) a widget with the specified `role` has been found.
        // The first scenario is cool because it means that there are no more widgets with
        // the input `role` in this layout. We can exit the method.
        // in the second case we need to remove the item we found and then start again.

        if (info == m_infos.cend()) {
          // All good.
          stillWidgetWithRole = false;
          continue;
        }
        else {
          // Remove the item we found.
          removeItem(info->second.widget);
        }
      }
      while (stillWidgetWithRole);
    }

    void
    MainWindowLayout::assignPercentagesFromCentralWidget(const utils::Sizef& centralWidgetSize) {
      // Left and right areas share equal percentage of the remaining space.
      const float sidePercentage = (1.0f - centralWidgetSize.w()) / 2.0f;
      m_leftAreaPercentage = sidePercentage;
      m_rightAreaPercentage = sidePercentage;

      // From the remaining vertical space, the top and bottom areas take
      // up to 60%, toolbars 20% and the menu and status bars share the rest.
      const float remaining = 1.0f - centralWidgetSize.h();

      const float periphericalAreas = 0.6f * remaining;
      const float toolbarsPercentage = 0.2f * remaining;
      const float menuAndStatus = (remaining - periphericalAreas - toolbarsPercentage);

      m_menuBarPercentage = menuAndStatus / 2.0f;
      m_toolBarPercentage = toolbarsPercentage;
      m_topAreaPercentage = periphericalAreas / 2.0f;
      m_bottomAreaPercentage = periphericalAreas / 2.0f;
      m_statusBarPercentage = menuAndStatus / 2.0f;
    }

    void
    MainWindowLayout::removeItemFromRole(core::SdlWidget* widget,
                                         const WidgetRole& role)
    {
      // Before trying to remove the widget, we need to first determine
      // its precise role. If it does not match the expected input `role`
      // we do not proceed to removing it.
      // So first, try to retrieve the index of this widget inside the
      // internal array.
      const int id = getIndexOf(widget);

      // Check whether we could find this item.
      if (!isValidIndex(id)) {
        error(
          std::string("Cannot get index for item \"") + widget->getName() + "\" from layout",
          std::string("Widget is not managed by this layout")
        );
      }

      // Check that this item is registered in the information array.
      InfosMap::const_iterator info = m_infos.find(id);
      if (info == m_infos.cend()) {
        error(
          std::string("Cannot retrieve role for item \"") + widget->getName() + "\"",
          std::string("Inexisting key")
        );
      }

      // Check whether the role for this item is actually valid considering the
      // input `role`. In any other case we abort the deletion of the item as it
      // is not what is expected by the caller.
      if (role != info->second.role) {
        error(
          std::string("Could not remove item \"") + widget->getName() + "\" which is not a dock widget",
          std::string("Role \"") + roleToName(info->second.role) + " does not match expected role \"" + roleToName(role)
        );
      }

      // Remove the item using the parent method.
      removeItem(widget);
    }

  }
}
