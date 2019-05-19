
# include "MainWindowLayout.hh"

# include <sstream>

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

      setService("main_layout");
    }

    MainWindowLayout::~MainWindowLayout() {}

    void
    MainWindowLayout::removeDockWidget(core::SdlWidget* item) {
      // Before trying to remove the widget, we need to first determine
      // its precise role. Indeed based on the area in which the widget
      // is located, its role will be different.
      // So first, try to retrieve the index of this widget inside the
      // internal array.

      // Check whether the item is valid.
      if (item == nullptr) {
        error(
          std::string("Cannot remove item from layout"),
          std::string("Invalid null item")
        );
      }

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
      if (!isValidDockWidgetRole(info->second.role)) {
        error(
          std::string("Could not remove item \"") + item->getName() + "\" which is not a dock widget",
          std::string("Role \"") + getNameFromRole(info->second.role) + " is not a valid dock widget role"
        );
      }

      // The input item is actually a dock widget we can remove it.
      removeItemFromIndex(id);
    }

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
      //  |  |   +-------------------+   |  |
      //  |  |   |                   |   |  |
      //  |  |   |  Central  widget  |   |  |
      //  |  |   |                   |   |  |
      //  |  |   +-------------------+   |  |
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

      // The goal is to provide a position for each of the area in this layout.
      RolesInfo roles;

      // Perform an horizontal adjustment.
      adjustRolesHorizontally(internalSize, widgetsInfo, roles);

      // Perform a vertical adjustment.
      adjustRolesVertically(internalSize, widgetsInfo, roles);

      // Consolidate roles dimensions.
      roles.consolidateRolesDimensions(getMargin());

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

        // Retrieve the role's position from the adjustments we made.
        const utils::Boxf areaForRole = roles.getBoxForRole(info->second.role);
        if (!areaForRole.valid()) {
          error(
            std::string("Could not retrieve role position for \"" + getNameFromRole(info->second.role) + "\", widget ") +
            getWidgetAt(index)->getName() + " has invalid position",
            std::string("Invalid role information")
          );
        }

        log("Area is now " + areaForRole.toString(), utils::Level::Warning);

        // Assign position first by using the global offset of the input `window`.
        float xWidget = (window.x() - window.w() / 2.0f);
        float yWidget = (window.y() - window.h() / 2.0f);

        // Assign position from this role's information: this accounts for the offset
        // of the role inside the texture representing this layout.
        xWidget += areaForRole.x();
        yWidget += areaForRole.y();

        // Handle the centering of the widget in case it is smaller than the
        // desired width or height.
        // Basically we are using a centered representation for the bounding
        // boxes, so we should add the following expression to the position
        // of the box:
        //
        // xWidget += areaForRole.w() / 2.0f
        // yWidget += areaForRole.h() / 2.0f
        //
        // This will guarantee that the widget is in the middle of the area
        // assigned to it no matter its real size computed with the following
        // function (i.e. `computeSizeFromPolicy`).
        utils::Sizef achievableSize = computeSizeFromPolicy(
          outputBoxes[index],
          areaForRole.toSize(),
          widgetsInfo[index]
        );

        // Center the position (because `Boxf` is a centered representation of
        // a bounding box).
        xWidget += (areaForRole.w() / 2.0f);
        yWidget += (areaForRole.h() / 2.0f);

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

      // Assign rendering areas using the base handler without converting the
      // positions as this layout handles position in the application itself.
      assignRenderingAreas(outputBoxes, window, false);
    }

    void
    MainWindowLayout::invalidate() noexcept {
      // We need to update the local information about widgets. This means basically updating the
      // `m_infos` attribute. In order to do so, we need to rely on some invariant properties
      // of the widget which have been updated. We will use the address in order to maintain some
      // consistency between widgets.

      // So first copy the internal information table so that we can build a new one right away.
      InfosMap old;
      old.swap(m_infos);

      // Traverse the old information and try to build the new table.
      for (InfosMap::const_iterator oldWidget = old.cbegin() ;
           oldWidget != old.cend() ;
           ++oldWidget)
      {
        // Try to find the index of this widget.
        const int newID = getIndexOf(oldWidget->second.widget);

        // From there, we have two main cases: either the widget still exists in the layout,
        // or it doesn't. This is defined by the fact that the `newID` is positive or negative.
        // If the widget still exists, we need to update the information contained in the
        // internal map so that further update of the layout yields correct results. If the
        // widget does not exist anymore in the layout, we have to ignore this widget and not
        // add it to the new internal map.

        // Insert the widget with its new information only if it still exists in the layout.
        if (isValidIndex(newID)) {
          m_infos[newID] = oldWidget->second;
        }
      }

      // Call parent method.
      core::Layout::invalidate();
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
          removeItemFromIndex(info->first);
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
      // whether it is actually a valid widget.
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
          std::string("Item \"") + widget->getName() + "\" does not have expected role in layout",
          std::string("Expected ") + getNameFromRole(role) +
          " got " + getNameFromRole(info->second.role)
        );
      }

      // We can retrieve the computed id.
      return id;
    }

    void
    MainWindowLayout::adjustRolesHorizontally(const utils::Sizef& window,
                                              const std::vector<WidgetInfo>& widgetsInfo,
                                              RolesInfo& roles)
    {
      // The goal of this function is to perform an adjustment of the horizontal areas.
      // Relevant areas which can impact the horizontal adjustement are the left dock
      // area, the central widget, bottom and top dock areas and the right dock area.
      // Apart from these areas, the tool bars, status bar and menu bar will be given
      // access to the whole width of the layout.
      // In order to perform the adjustment, we first try to determine a global policy
      // for each area: this is done by scanning each widget using the area and determining
      // the bounds of the size which can be assigned to it.
      // We update the global policy for each widget encountered.
      //
      // Once we got a global policy for each area, we can try to assign the total space
      // based on the percentages assigned to each area for this layout. Each area will
      // be assigned a default space which will then be constrained by the global policy
      // and updated.
      // We then loop until no more modifications are made to any widget. This allows
      // to use additional space not used by an area if another area can take it. In
      // general it guarantees a better repartition of the space between areas.
      //
      // Once we reached a stable state, we can update the input `roles` array with
      // the width of each column.
      std::unordered_set<WidgetRole> dockRoles;

      // First, compute a global policy for each relevant area.
      dockRoles.clear();
      dockRoles.insert(WidgetRole::LeftDockWidget);
      core::Layout::WidgetInfo leftPolicy = computeSizePolicyForRoles(dockRoles, widgetsInfo);

      dockRoles.clear();
      dockRoles.insert(WidgetRole::RightDockWidget);
      core::Layout::WidgetInfo rightPolicy = computeSizePolicyForRoles(dockRoles, widgetsInfo);

      // Agregate a relevant area for the top and bottom area along with the central widget.
      // As these areas as all aligned horizontally, any space used by one of them will be
      // shared with the other areas, and thus any constraint applied to any of the area will
      // translate into a constraint into the other areas.
      // Even if the widgets spanning a particular area might not be able to use the full
      // extent of all the constraints, this will be used nonetheless. We can then proceed
      // to centering for example to try to make use of the constraints.
      dockRoles.clear();
      dockRoles.insert(WidgetRole::TopDockWidget);
      dockRoles.insert(WidgetRole::CentralDockWidget);
      dockRoles.insert(WidgetRole::BottomDockWidget);
      core::Layout::WidgetInfo centralPolicy = computeSizePolicyForRoles(dockRoles, widgetsInfo);

      // Once policies for each area are computed, we can start the optimization process.
      // We basically try to allocate fairly the remaining space between each area at any point
      // and proceed to several iteration to allocate the space that could not be allocated in
      // the previous iteration.
      // This allows for a simple iterative algorithm where at each step we try to divide the space
      // equally and each area has the opportunity to grow by the same amount: if for some reasons
      // the area cannot make use of the space, we will redistribute it in the next iteration to
      // other areas and see what they can do with it.
      std::unordered_set<unsigned> rolesToAdjust;
      std::vector<core::Layout::WidgetInfo> rolesData;

      // In a first approach all the widgets can be adjusted.
      for (unsigned index = 0u ; index < 3u ; ++index) {
        rolesToAdjust.insert(index);
      }
      rolesData.push_back(leftPolicy);
      rolesData.push_back(centralPolicy);
      rolesData.push_back(rightPolicy);

      // Also assume that we didn't use up all the available space.
      float spaceToUse = window.w();
      bool allSpaceUsed = false;

      float achievedWidth = 0.0f;

      // Loop until no more widgets can be used to adjust the space needed or all the
      // available space has been used up.
      // TODO: Handle cases where the widgets are too large to fit into the layout ?
      while (!rolesToAdjust.empty() && !allSpaceUsed) {

        // Compute the amount of space we will try to allocate to each area still
        // available for adjustment.
        // The `defaultWidth` is computed by dividing equally the remaining `spaceToUse`
        // among all the available areas.
        const float defaultWidth = allocateFairly(spaceToUse, rolesToAdjust.size());

        log(std::string("Default width is ") + std::to_string(defaultWidth), utils::Level::Info);

        // Loop on all the areas that can still be used to adjust the space used by
        // this layout and perform the size adjustements.
        for (std::unordered_set<unsigned>::const_iterator role = rolesToAdjust.cbegin() ;
             role != rolesToAdjust.cend() ;
             ++role)
        {
          // Try to assign the `defaultWidth` to this area: we use a dedicated handler
          // to handle the case where the provided space is too large/small/not suited
          // to the area for some reasons, in which case the handler will provide a
          // size which can be applied to the widget.
          float width = computeWidthFromPolicy(rolesData[*role].area, defaultWidth, rolesData[*role]);

          // We now need to distribute this width to the current `area`: in order to do
          // so, let's compute the size increase provided for this area by the current
          // iteration: this is the size which belongs to the area.
          rolesData[*role].area.w() += (width - rolesData[*role].area.w());

          log(std::string("Role ") + std::to_string(*role) + " reach size " + rolesData[*role].area.toString());
        }

        // We have tried to apply the `defaultWidth` to all the areas. This might have fail
        // in some cases (for example due to a `Fixed` size policy for an area) and thus
        // we might end up with a total size for all the areas different from the one desired
        // and expected when the `defaultWidth` has been computed.
        // In order to fix things, we must compute the deviation from the expected size and
        // try to allocate the remaining space to other areas (or remove the missing space
        // from areas which can give up some).

        // Compute the total size of the bounding boxes.
        achievedWidth = computeSizeOfRoles(rolesData).w();

        const utils::Sizef achievedSize(achievedWidth, window.h());

        // Check whether all the space have been used.
        if (achievedSize.fuzzyEqual(window, 1.0f)) {
          // We used up all the available space, no more adjustments to perform.
          allSpaceUsed = true;
          continue;
        }


        // All space has not been used. Update the relevant `spaceToUse` in order to perform
        // the next iteration.
        spaceToUse = computeSpaceAdjustmentNeeded(achievedSize, window).w();

        // Determine the policy to apply based on the achieved size.
        const core::SizePolicy action = shrinkOrGrow(window, achievedSize, 0.5f);

        log(std::string("Desired ") + window.toString() + ", achieved: " + std::to_string(achievedWidth) + ", space: " + std::to_string(spaceToUse), utils::Level::Info);

        // We now know what should be done to make the `achievedSize` closer to `desiredSize`.
        // Based on the `policy` provided by the base class method, we can now determine which
        // area should be used to perform the needed adjustments.
        std::unordered_set<unsigned> rolesToUse;
        for (unsigned index = 0u ; index < 3u ; ++index) {
          // Check whether this area can be used to grow/shrink.
          std::pair<bool, bool> usable = canBeUsedTo(std::string("Role") + std::to_string(index), rolesData[index], rolesData[index].area, action);

          // Only care for the horizontal direction, vertical direction will be handled later.
          if (usable.first) {
            log("Role " + std::to_string(index) + " can be used to perform action " + action.toString());

            rolesToUse.insert(index);
          }
        }

        // There's one more thing to determine: the `Expanding` flag on any area's policy should
        // mark it as priority over other areas. For example if two areas can grow, one having
        // the flag `Grow` and the other the `Expand` flag, we should make priority for the one
        // with `Expanding` flag.
        // Areas with `Grow` flag will only grow when all `Expanding` areas have been maxed out.
        // Of course this does not apply in case areas should be shrunk: all areas are treated
        // equally in this case and there's not preferred areas to shrink.
        if (action.canExtendHorizontally()) {
          // Select only `Expanding` widget if any.
          std::unordered_set<unsigned> rolesToExpand;

          for (std::unordered_set<unsigned>::const_iterator role = rolesToUse.cbegin() ;
               role != rolesToUse.cend() ;
               ++role)
          {
            // Check whether this area can expand.
            if (rolesData[*role].policy.canExpandHorizontally()) {
              log("Role " + std::to_string(*role) + " can be expanded horizontally");
              rolesToExpand.insert(*role);
            }
          }

          log(
            "Saved " + std::to_string(rolesToExpand.size()) + " which can expand compared to " +
            std::to_string(rolesToUse.size()) + " which can extend",
            utils::Level::Info
          );
          // Check whether we could select at least one area to expand: if this is not the
          // case we can proceed to extend the areas with only a `Grow` flag.
          if (!rolesToExpand.empty()) {
            rolesToUse.swap(rolesToExpand);
          }
        }


        // Use the computed list of areas to perform the next action in order
        // to reach the desired space.
        rolesToAdjust.swap(rolesToUse);
      }

      // Warn the user in case we could not use all the space.
      if (!allSpaceUsed) {
        log(
          std::string("Could only achieve width of ") + std::to_string(achievedWidth) +
          " but available space is " + window.toString(),
          utils::Level::Error
        );
      }

      // Update the input argument `roles`. This attribute contains a box for each dock area
      // and should be updated with the content of the internal `areasData` array.
      // We know by construction the position of each area so we can just perform the update
      // for each relevant area.

      // First left area.
      roles.assignOrCreateDimsForRole(WidgetRole::LeftDockWidget, true, rolesData[0u].area.w());

      // Central, top and bottom area are shared.
      roles.assignOrCreateDimsForRole(WidgetRole::TopDockWidget, true, rolesData[1u].area.w());
      roles.assignOrCreateDimsForRole(WidgetRole::CentralDockWidget, true, rolesData[1u].area.w());
      roles.assignOrCreateDimsForRole(WidgetRole::BottomDockWidget, true, rolesData[1u].area.w());

      // And finally right area.
      roles.assignOrCreateDimsForRole(WidgetRole::RightDockWidget, true, rolesData[2u].area.w());

      // Assign the total window's size to the menu bar, tool bar and status bar.
      roles.assignOrCreateDimsForRole(WidgetRole::MenuBar, true, window.w());
      roles.assignOrCreateDimsForRole(WidgetRole::ToolBar, true, window.w());
      roles.assignOrCreateDimsForRole(WidgetRole::StatusBar, true, window.w());
    }

    void
    MainWindowLayout::adjustRolesVertically(const utils::Sizef& window,
                                            const std::vector<WidgetInfo>& widgetsInfo,
                                            RolesInfo& roles)
    {
      // The goal of this function is to perform an adjustment of the vertical areas.
      // Relevant areas which can impact the vertical adjustement are the dock widgets
      // areas, the menu bar, status bar and tool bar.
      // In order to perform the adjustment, we first try to determine a global policy
      // for each area: this is done by scanning each widget using the area and determining
      // the bounds of the size which can be assigned to it.
      // We update the global policy for each widget encountered.
      //
      // Once we got a global policy for each area, we can try to assign the total space
      // based on the percentages assigned to each area for this layout. Each area will
      // be assigned a default space which will then be constrained by the global policy
      // and updated.
      // We then loop until no more modifications are made to any widget. This allows
      // to use additional space not used by an area if another area can take it. In
      // general it guarantees a better repartition of the space between areas.
      //
      // Once we reached a stable state, we can update the input `roles` array with
      // the width of each column.
      std::unordered_set<WidgetRole> dockRoles;

      // First, compute a global policy for each relevant area. We need to aggregate a
      // global policy for all the dock widgets as they will be assigned a single height
      // in the layout.
      dockRoles.clear();
      dockRoles.insert(WidgetRole::LeftDockWidget);
      dockRoles.insert(WidgetRole::TopDockWidget);
      dockRoles.insert(WidgetRole::CentralDockWidget);
      dockRoles.insert(WidgetRole::BottomDockWidget);
      dockRoles.insert(WidgetRole::RightDockWidget);
      core::Layout::WidgetInfo dockAreaPolicy = computeSizePolicyForRoles(dockRoles, widgetsInfo);

      // Aggregate the policy for each bar.
      dockRoles.clear();
      dockRoles.insert(WidgetRole::MenuBar);
      core::Layout::WidgetInfo menuBarPolicy = computeSizePolicyForRoles(dockRoles, widgetsInfo);

      dockRoles.clear();
      dockRoles.insert(WidgetRole::ToolBar);
      core::Layout::WidgetInfo toolBarPolicy = computeSizePolicyForRoles(dockRoles, widgetsInfo);

      dockRoles.clear();
      dockRoles.insert(WidgetRole::StatusBar);
      core::Layout::WidgetInfo statusBarPolicy = computeSizePolicyForRoles(dockRoles, widgetsInfo);

      // Once policies for each area are computed, we can start the optimization process.
      // We basically try to allocate fairly the remaining space between each area at any point
      // and proceed to several iteration to allocate the space that could not be allocated in
      // the previous iteration.
      // This allows for a simple iterative algorithm where at each step we try to divide the space
      // equally and each area has the opportunity to grow by the same amount: if for some reasons
      // the area cannot make use of the space, we will redistribute it in the next iteration to
      // other areas and see what they can do with it.
      std::unordered_set<unsigned> rolesToAdjust;
      std::vector<core::Layout::WidgetInfo> rolesData;

      // In a first approach all the widgets can be adjusted.
      for (unsigned index = 0u ; index < 4u ; ++index) {
        rolesToAdjust.insert(index);
      }
      rolesData.push_back(menuBarPolicy);
      rolesData.push_back(toolBarPolicy);
      rolesData.push_back(dockAreaPolicy);
      rolesData.push_back(statusBarPolicy);

      // Also assume that we didn't use up all the available space.
      float spaceToUse = window.w();
      bool allSpaceUsed = false;

      float achievedHeight = 0.0f;

      // Loop until no more widgets can be used to adjust the space needed or all the
      // available space has been used up.
      // TODO: Handle cases where the widgets are too large to fit into the layout ?
      while (!rolesToAdjust.empty() && !allSpaceUsed) {

        // Compute the amount of space we will try to allocate to each area still
        // available for adjustment.
        // The `defaultHeight` is computed by dividing equally the remaining `spaceToUse`
        // among all the available areas.
        const float defaultHeight = allocateFairly(spaceToUse, rolesToAdjust.size());

        log(std::string("Default height is ") + std::to_string(defaultHeight), utils::Level::Info);

        // Loop on all the areas that can still be used to adjust the space used by
        // this layout and perform the size adjustements.
        for (std::unordered_set<unsigned>::const_iterator role = rolesToAdjust.cbegin() ;
             role != rolesToAdjust.cend() ;
             ++role)
        {
          // Try to assign the `defaultWidth` to this area: we use a dedicated handler
          // to handle the case where the provided space is too large/small/not suited
          // to the area for some reasons, in which case the handler will provide a
          // size which can be applied to the widget.
          float height = computeHeightFromPolicy(rolesData[*role].area, defaultHeight, rolesData[*role]);

          // We now need to distribute this width to the current `area`: in order to do
          // so, let's compute the size increase provided for this area by the current
          // iteration: this is the size which belongs to the area.
          rolesData[*role].area.h() += (height - rolesData[*role].area.h());

          log(std::string("Role ") + std::to_string(*role) + " reach size " + rolesData[*role].area.toString());
        }

        // We have tried to apply the `defaultHeight` to all the areas. This might have fail
        // in some cases (for example due to a `Fixed` size policy for an area) and thus
        // we might end up with a total size for all the areas different from the one desired
        // and expected when the `defaultHeight` has been computed.
        // In order to fix things, we must compute the deviation from the expected size and
        // try to allocate the remaining space to other areas (or remove the missing space
        // from areas which can give up some).

        // Compute the total size of the bounding boxes.
        achievedHeight = computeSizeOfRoles(rolesData).h();

        const utils::Sizef achievedSize(window.w(), achievedHeight);

        // Check whether all the space have been used.
        if (achievedSize.fuzzyEqual(window, 1.0f)) {
          // We used up all the available space, no more adjustments to perform.
          allSpaceUsed = true;
          continue;
        }


        // All space has not been used. Update the relevant `spaceToUse` in order to perform
        // the next iteration.
        spaceToUse = computeSpaceAdjustmentNeeded(achievedSize, window).h();

        // Determine the policy to apply based on the achieved size.
        const core::SizePolicy action = shrinkOrGrow(window, achievedSize, 0.5f);

        log(std::string("Desired ") + window.toString() + ", achieved: " + std::to_string(achievedHeight) + ", space: " + std::to_string(spaceToUse), utils::Level::Info);

        // We now know what should be done to make the `achievedSize` closer to `desiredSize`.
        // Based on the `policy` provided by the base class method, we can now determine which
        // area should be used to perform the needed adjustments.
        std::unordered_set<unsigned> rolesToUse;
        for (unsigned index = 0u ; index < 4u ; ++index) {
          // Check whether this area can be used to grow/shrink.
          std::pair<bool, bool> usable = canBeUsedTo(std::string("Role") + std::to_string(index), rolesData[index], rolesData[index].area, action);

          // Only care for the vertical direction, horizontal direction will be handled later.
          if (usable.second) {
            log("Role " + std::to_string(index) + " can be used to perform action " + action.toString());

            rolesToUse.insert(index);
          }
        }

        // There's one more thing to determine: the `Expanding` flag on any area's policy should
        // mark it as priority over other areas. For example if two areas can grow, one having
        // the flag `Grow` and the other the `Expand` flag, we should make priority for the one
        // with `Expanding` flag.
        // Areas with `Grow` flag will only grow when all `Expanding` areas have been maxed out.
        // Of course this does not apply in case areas should be shrunk: all areas are treated
        // equally in this case and there's not preferred areas to shrink.
        if (action.canExtendVertically()) {
          // Select only `Expanding` widget if any.
          std::unordered_set<unsigned> rolesToExpand;

          for (std::unordered_set<unsigned>::const_iterator role = rolesToUse.cbegin() ;
               role != rolesToUse.cend() ;
               ++role)
          {
            // Check whether this area can expand.
            if (rolesData[*role].policy.canExpandVertically()) {
              log("Role " + std::to_string(*role) + " can be expanded vertically");
              rolesToExpand.insert(*role);
            }
          }

          log(
            "Saved " + std::to_string(rolesToExpand.size()) + " which can expand compared to " +
            std::to_string(rolesToUse.size()) + " which can extend",
            utils::Level::Info
          );
          // Check whether we could select at least one area to expand: if this is not the
          // case we can proceed to extend the areas with only a `Grow` flag.
          if (!rolesToExpand.empty()) {
            rolesToUse.swap(rolesToExpand);
          }
        }


        // Use the computed list of areas to perform the next action in order
        // to reach the desired space.
        rolesToAdjust.swap(rolesToUse);
      }

      // Warn the user in case we could not use all the space.
      if (!allSpaceUsed) {
        log(
          std::string("Could only achieve height of ") + std::to_string(achievedHeight) +
          " but available space is " + window.toString(),
          utils::Level::Error
        );
      }

      // Update the input argument `roles`. This attribute contains a box for each dock area
      // and should be updated with the content of the internal `areasData` array.
      // We know by construction the position of each area so we can just perform the update
      // for each relevant area.

      // First left area.
      roles.assignOrCreateDimsForRole(WidgetRole::LeftDockWidget, false, 0.0f, true, rolesData[2u].area.h());

      // Central, top and bottom area are shared.
      // TODO: How to do that ???
      roles.assignOrCreateDimsForRole(WidgetRole::TopDockWidget, false, 0.0f, true, 0.0f);
      roles.assignOrCreateDimsForRole(WidgetRole::CentralDockWidget, false, 0.0f, true, rolesData[2u].area.h());
      roles.assignOrCreateDimsForRole(WidgetRole::BottomDockWidget, false, 0.0f, true, 0.0f);

      // And finally right area.
      roles.assignOrCreateDimsForRole(WidgetRole::RightDockWidget, false, 0.0f, true, rolesData[2u].area.h());

      // Assign the computed height to the menu bar, tool bar and status bar.
      roles.assignOrCreateDimsForRole(WidgetRole::MenuBar, false, 0.0f, true, rolesData[0u].area.h());
      roles.assignOrCreateDimsForRole(WidgetRole::ToolBar, false, 0.0f, true, rolesData[1u].area.h());
      roles.assignOrCreateDimsForRole(WidgetRole::StatusBar, false, 0.0f, true, rolesData[3u].area.h());
    }

    core::Layout::WidgetInfo
    MainWindowLayout::computeSizePolicyForRoles(const std::unordered_set<WidgetRole>& roles,
                                                const std::vector<WidgetInfo>& widgetsInfo) const
    {
      // Create a default policy.
      core::Layout::WidgetInfo policy;

      std::stringstream ss;
      ss << "Computing policy for roles: ";
      for (std::unordered_set<WidgetRole>::const_iterator role = roles.cbegin() ;
           role != roles.cend() ;
           ++role)
      {
        ss << getNameFromRole(*role) << " ";
      }
      ss << "out of " << m_infos.size() << " info";
      log(ss.str(), utils::Level::Notice);

      // Traverse the internal set of widgets to determine the global policy.
      for (InfosMap::const_iterator info = m_infos.cbegin() ;
           info != m_infos.cend() ;
           ++info)
      {
        // Retrieve the area associated to the current widget.
        const ItemInfo item = info->second;

        // Check whether the widget belongs to the input role: if this is not the
        // case we do not consider it for the determination of the global policy.
        if (roles.count(item.role) == 0) {
          // No need to consider this item.
          continue;
        }

        log("Considering widget assuming correct role " + getNameFromRole(item.role));

        // We now know that the widget does assume the input role: we shall use it for
        // the determination of the global policy.
        // In order to do that, we need to update the dimensions which might be provided
        // by the widget and also the policy.
        // We will select the less restrictive policies for any area in terms of maximum
        // size and the most restrictive policies for minimum size.
        // To do all that, we first need to retrieve the policy of the widget associated to
        // the information we are processing.

        // Retrieve the information for this widget.
        const WidgetInfo& wigInfo = widgetsInfo[info->first];

        // First adjust the size information for the global policy. We update both the
        // maximum and minimum size if they are larger respectively than the current
        // maximum and minimum size. This guarantees that we will expand as much as possible
        // and shrink as little as needed.
        // In order to keep things simple we will use the dedicated handler from this class.
        consolidatePolicyFromItem(policy, wigInfo);

        log(
          "Policy is now: min=" + policy.min.toString() + ", hint= " + policy.hint.toString() + ", max= " + policy.hint.toString(),
          utils::Level::Notice
        );
      }

      // Return the computed policy for the input areas.
      return policy;
    }

  }
}
