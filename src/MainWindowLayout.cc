
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    MainWindowLayout::MainWindowLayout(const utils::Boxf& area,
                                       const float& margin,
                                       const utils::Sizef& centralWidgetSize):
      core::Layout(nullptr, margin, true),
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
    }

    MainWindowLayout::~MainWindowLayout() {}

    void
    MainWindowLayout::updatePrivate(const utils::Boxf& window) {
      // The main window layout can be represented as below:
      //
      //  +---------------------------------+
      //  |             Menu bar            |
      //  +---------------------------------+
      //  |            Toolbars             |
      //  |                                 |
      //  |  +---+-------------------+---+  |
      //  |  |   |   Dock widgets    |   |  |
      //  |  +---+-------------------+---+  |
      //  |  |   |                   |   |  |
      //  |  |   |  Central  widget  |   |  |
      //  |  |   |                   |   |  |
      //  |  +---+-------------------+---+  |
      //  |  |   |                   |   |  |
      //  |  +---+-------------------+---+  |
      //  |                                 |
      //  |---------------------------------+
      //  |            Status bar           |
      //  +---------------------------------+
      //
      // Each region can have its own widgets assigned to it,
      // each one defined and added using the provided enumeration
      // to describe the role of the widget added.
      // Each area is expanded/grown/shrunk according to the policy
      // of the widgets inside it to reach a correct state.
      // Note that the initial repartition of space is not equivalent
      // between regions, which correspond to the intuitive idea of
      // the central widget being more important than the other elements.
      //
      // Even though several widgets can occupy most of the roles, only
      // one widget can be assigned as the central widget.
      //
      // The process to perform the layout is split into two main parts:
      // just like some other layouts we distinguish the width and height
      // optimizations as they are independent.
      //
      // For each dimension we start by finding the policy for each
      // area implied in the process and then perform a loop to split
      // the available space equally between elements.
      // For example in the case of horizontal adjustment, we determine
      // among all widgets sharing the left area the minimum and
      // maximum extent possible and save it. Then we do the same for
      // the right area. And we proceed to an optimization using the
      // two computed values and the central widget.
      // The result is then applied to each element of the initial areas.

      // Compute the available size for this layout. We need to take margins
      // into account. This is done by the base handler.
      const utils::Sizef internalSize = computeAvailableSize(window);

      // Copy the current size of widgets so that we can work with it without
      // requesting constantly information or setting information multiple times.
      std::vector<WidgetInfo> widgetsInfo = computeWidgetsInfo();

      // The goal is to provide a position for each of the dock widget area
      // in this layout.
      AreasInfo areas;

      // Perform an horizontal adjustment.
      adjustAreasHorizontally(internalSize, widgetsInfo, areas);

      // Perform a vertical adjustment.
      adjustAreasVertically(internalSize, widgetsInfo, areas);

      // Compute bounding boxes from the result of the optimization.
      std::vector<utils::Boxf> outputBoxes(getItemsCount());

      for (int index = 0u ; index < getItemsCount() ; ++index) {
        // Position the widget based on the role it is assuming within this
        // layout. Each widget has a corresponding base area associated to
        // it such as toolbar, left area, top area, etc. This area should be
        // layout according to the result of the adjustment and used to
        // compute the final position of the widget.
        // Usually the size provided by the ajustment can be used right away
        // however in the case of several widgets sharing a single area the
        // computed dimensions might not fit all of them.
        // In this case we handle a centering mechanism so that even if a
        // widget gets assigned a space larger than it can use it is still
        // displayed nicely.

        // Retrieve the area's information for this widget.
        const InfosMap::const_iterator info = m_infos.find(index);
        if (info == m_infos.end()) {
          error(
            std::string("Could not retrieve information for widget \"") +
            getWidgetAt(index)->getName() + "\" while updating main window layout",
            std::string("Unable to find information for widget")
          );
        }

        // Retrieve the area's position from the adjustments we made if needed.
        if (info->second.role == WidgetRole::DockWidget) {
          log(
            std::string("Skipping position for widget \"") + getWidgetAt(index)->getName() + "\" with unsupported role DockWidget",
            utils::Level::Warning
          );
          continue;
        }
        const AreasInfo::const_iterator area = areas.find(info->second.area);
        if (area == areas.cend()) {
          error(
            std::string("Could not retrieve area position for \"" + std::to_string(static_cast<int>(info->second.area)) + "\", widget ") +
            getWidgetAt(index)->getName() + " has invalid position",
            std::string("Invalid area information")
          );
        }

        // Assign position from this area's information.
        float xWidget = area->second.x();
        float yWidget = area->second.y();

        // Center the position (because `Boxf` are centered).
        xWidget += (area->second.w() / 2.0f);
        yWidget += (area->second.h() / 2.0f);

        // Handle the centering of the widget in case it is smaller than the
        // desired width or height.
        // To do so, compute the size the widget _should_ have based on its
        // area and try to apply it to the widget. If the provided is different
        // from the provided size we handle centering.
        utils::Sizef achievableSize = computeSizeFromPolicy(
          outputBoxes[index],
          area->second.toSize(),
          widgetsInfo[index]
        );

        // Handle centering anyway: if the achievable size is identical to the
        // desired size the centering will not modify the position of the widget.
        // The position is already correct because we used the position of the
        // area (and not the position computed from the actual size of the widget)
        // so we just have to equalize the width and height of the output box with
        // the achievable dimensions.
        outputBoxes[index] = utils::Boxf(
          xWidget, yWidget,
          achievableSize.w(), achievableSize.h()
        );
      }

      // Assign rendering areas using the base handler.
      assignRenderingAreas(outputBoxes, window);
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
          removeItem(info->first);
        }
      }
      while (stillWidgetWithRole);
    }

    int
    MainWindowLayout::getIndexAndCheck(core::SdlWidget* widget,
                                       const WidgetRole& role)
    {
      // Check whether the item is valid.
      if (widget == nullptr) {
        error(
          std::string("Cannot remove item from layout"),
          std::string("Invalid null item")
        );
      }

      // Use the dedicated method to retrieve the item's index and determine
      // whether it is actually a toolbar.
      const int id = getIndexOf(widget);

      // Check whether we could find this item.
      if (!isValidIndex(id)) {
        error(
          std::string("Cannot get index for item \"") + widget->getName() + "\" from layout",
          std::string("Widget is not managed by this layout")
        );
      }

      // Check the role of this item in the internal table.
      InfosMap::const_iterator info = m_infos.find(id);
      if (info == m_infos.cend()) {
        error(
          std::string("Cannot retrieve role for item \"") + widget->getName() + "\"",
          std::string("Inexisting key")
        );
      }

      if (info->second.role != role) {
        error(
          std::string("Item \"") + widget->getName() + "\" has not expected role in layout",
          std::string("Expected ") + std::to_string(static_cast<int>(role)) +
          " got " + std::to_string(static_cast<int>(info->second.role))
        );
      }

      // We can retrieve the computed id.
      return id;
    }

    void
    MainWindowLayout::adjustAreasHorizontally(const utils::Sizef& /*internalSize*/,
                                              const std::vector<WidgetInfo>& /*widgetsInfo*/,
                                              AreasInfo& /*areas*/)
    {
      // TODO: Implementation.
      log("Should perform horizontal adjustment");
    }

    void
    MainWindowLayout::adjustAreasVertically(const utils::Sizef& /*internalSize*/,
                                            const std::vector<WidgetInfo>& /*widgetsInfo*/,
                                            AreasInfo& /*areas*/)
    {
      // TODO: Implementation.
      log("Should perform vertical adjustment");
    }

  }
}
