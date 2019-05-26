
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    MainWindowLayout::MainWindowLayout(const utils::Boxf& area,
                                       const float& margin,
                                       const utils::Sizef& centralWidgetSize):
      graphic::GridLayout(3u, 6u, margin, nullptr, true),
      m_area(area),
      m_infos(),

      m_leftAreaPercentage(),
      m_rightAreaPercentage(),

      m_menuBarPercentage(),
      m_toolBarPercentage(),
      m_topAreaPercentage(),
      m_bottomAreaPercentage(),
      m_statusBarPercentage()
    {
      // Assign the percentages from the input central widget size.
      assignPercentagesFromCentralWidget(centralWidgetSize);

      setService("main_layout");
    }

    MainWindowLayout::~MainWindowLayout() {}

    void
    MainWindowLayout::adjustWidgetToConstraints(const utils::Sizef& window,
                                                std::vector<WidgetInfo>& widgets) const noexcept {
      // This method is used to adjust each cell to the constraints needed to provide a
      // nice layout. We still want to benefit from the processing done by the parent
      // 'GridLayout' class but we also add to want a layer by using the internal variables
      // such as `m_menuBarPercentage` and so on.
      // Thus we first apply the parent handler and then proceed to update the produced
      // array of constraints.

      // Apply the parent handler.
      graphic::GridLayout::adjustWidgetToConstraints(window, widgets);

      // Now adjust the size of each widget based on the widget associated to it and the
      // constraints of its related area.

      // Traverse the input array.
      for (unsigned widget = 0u ; widget < widgets.size() ; ++widget) {
        // Find the corresponding widget role and area.
        const InfosMap::const_iterator info = m_infos.find(widget);
        if (info == m_infos.cend()) {
          error(
            std::string("Could not adjust widget ") + std::to_string(widget) + " to minimum constraints",
            std::string("Inexisting widget")
          );
        }

        // Compute the maximum size which can be assigned to this widget based on its role.
        utils::Sizef max = computeMaxSizeForRole(window, info->second.role);

        // Update the maximum size for this widget if needed.
        WidgetInfo& data = widgets[widget];
        if (data.max.w() > max.w()) {
          data.max.w() = max.w();
        }
        if (data.max.h() > max.h()) {
          data.max.w() = max.w();
        }

        // Handle adjustment for hint and minimum size as we may have modified the maximum
        // size.
        if (data.hint.w() > data.max.w()) {
          data.hint.w() = data.max.w();
        }
        if (data.hint.w() > data.max.w()) {
          data.hint.h() = data.max.w();
        }

        if (data.hint.isValid()) {
          if (data.min.w() > data.hint.w()) {
            data.min.w() = data.hint.w();
          }
          if (data.min.w() > data.hint.w()) {
            data.min.h() = data.hint.w();
          }
        }
        else {
          if (data.min.w() > data.max.w()) {
            data.min.w() = data.max.w();
          }
          if (data.min.w() > data.max.w()) {
            data.min.h() = data.max.w();
          }
        }
      }
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

      // Check whether the role for this item is actually a dock widget. In
      // any other case we abort the deletion of the item as it is not what
      // is expected by the caller.
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
