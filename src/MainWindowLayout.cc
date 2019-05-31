
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    MainWindowLayout::MainWindowLayout(const float& margin,
                                       const utils::Sizef& centralWidgetSize):
      core::Layout(nullptr, margin, true, std::string("main_window_layout"), true),
      m_infos(),

      m_leftAreaPercentage(),
      m_rightAreaPercentage(),

      m_menuBarPercentage(),
      m_toolBarPercentage(),
      m_topAreaPercentage(),
      m_bottomAreaPercentage(),
      m_statusBarPercentage(),

      m_hLayout(1u, 6u, margin, nullptr, true),
      m_vLayout(3u, 3u, margin, nullptr, true)
    {
      // Assign the percentages from the input central widget size.
      assignPercentagesFromCentralWidget(centralWidgetSize);

      setService("main_layout");

      // Assign events queue to internal layouts.
      registerToSameQueue(&m_hLayout);
      registerToSameQueue(&m_vLayout);
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

      // Compute geometry of internal layouts.
      m_hLayout.updatePrivate(window);
      m_vLayout.updatePrivate(window);

      // Now build the area to assign to each widget based on the internal virtual items.
      std::vector<utils::Boxf> boxes(m_infos.size());

      log("Main window layout is now assigning " + std::to_string(boxes.size()), utils::Level::Notice);

      int id = 0;
      for (InfosMap::const_iterator widgetInfo = m_infos.cbegin() ;
           widgetInfo != m_infos.cend() ;
           ++widgetInfo)
      {
        // Retrieve the widget's info.
        const ItemInfo& info = widgetInfo->second;

        // The box is obtained directly through the virtual layout item associated to this widget.
        boxes[id] = info.item->getRenderingArea();

        log("Box is thus " + boxes[id].toString(), utils::Level::Warning);

        ++id;
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
      const float menuAndStatus = (1.0f - periphericalAreas - toolbarsPercentage);

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
