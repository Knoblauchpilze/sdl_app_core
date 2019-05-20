#ifndef    MAIN_WINDOW_LAYOUT_HXX
# define   MAIN_WINDOW_LAYOUT_HXX

# include <sdl_core/SdlWidget.hh>
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    inline
    void
    MainWindowLayout::setArea(const utils::Boxf& area) noexcept {
      // Assign the area.
      m_area = area;

      // Invalidate the layout.
      invalidate();
    }

    inline
    void
    MainWindowLayout::update() {
      // Check if this layout is dirty.
      if (!isDirty()) {
        return;
      }

      // And if some items are managed by this layout.
      if (empty()) {
        return;
      }

      // Update with private handler.
      updatePrivate(m_area);

      recomputed();
    }

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
      // Use internal handler.
      addItemWithRoleAndArea(
        item,
        determineDockWidgetRoleFromArea(area),
        area
      );
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
      // Try to retrieve the index for this item.
      const int index = getIndexAndCheck(item, WidgetRole::ToolBar);

      // Remove it using the dedicated handler.
      removeItemFromIndex(index);
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
    std::string
    MainWindowLayout::getNameFromArea(const DockWidgetArea& area) noexcept {
      switch (area) {
        case DockWidgetArea::LeftArea:
          return "left_area";
        case DockWidgetArea::RightArea:
          return "right_area";
        case DockWidgetArea::TopArea:
          return "top_area";
        case DockWidgetArea::BottomArea:
          return "bottom_area";
        case DockWidgetArea::CentralArea:
          return "central_area";
        default:
          return "unknow_area";
      }
    }

    inline
    std::string
    MainWindowLayout::getNameFromRole(const WidgetRole& role) noexcept {
      switch (role) {
        case WidgetRole::MenuBar:
          return "menu_bar";
        case WidgetRole::StatusBar:
          return "status_bar";
        case WidgetRole::ToolBar:
          return "tool_bar";
        case WidgetRole::LeftDockWidget:
          return "left_dock_widget";
        case WidgetRole::RightDockWidget:
          return "right_dock_widget";
        case WidgetRole::TopDockWidget:
          return "top_dock_widget";
        case WidgetRole::BottomDockWidget:
          return "bottom_dock_widget";
        case WidgetRole::CentralDockWidget:
          return "central_dock_widget";
        default:
          return "unknown_role";
      }
    }

    inline
    bool
    MainWindowLayout::isValidDockWidgetRole(const WidgetRole& role) noexcept {
      return
        role == WidgetRole::LeftDockWidget ||
        role == WidgetRole::RightDockWidget ||
        role == WidgetRole::TopDockWidget ||
        role == WidgetRole::BottomDockWidget ||
        role == WidgetRole::CentralDockWidget
      ;
    }

    inline
    WidgetRole
    MainWindowLayout::determineDockWidgetRoleFromArea(const DockWidgetArea& area) {
      WidgetRole role;

      switch (area) {
        case DockWidgetArea::LeftArea:
          role = WidgetRole::LeftDockWidget;
          break;
        case DockWidgetArea::RightArea:
          role = WidgetRole::RightDockWidget;
          break;
        case DockWidgetArea::TopArea:
          role = WidgetRole::TopDockWidget;
          break;
        case DockWidgetArea::BottomArea:
          role = WidgetRole::BottomDockWidget;
          break;
        case DockWidgetArea::CentralArea:
          role = WidgetRole::CentralDockWidget;
          break;
        default:
          error(
            std::string("Could not determine widget role for area ") + getNameFromArea(area),
            std::string("Invalid dock area")
          );
          break;
      }

      return role;
    }

    inline
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

    inline
    void
    MainWindowLayout::addItemWithRoleAndArea(core::SdlWidget* widget,
                                             const WidgetRole& role,
                                             const DockWidgetArea& area)
    {
      // Add the item using the base handler.
      const int index = addItem(widget);

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

    inline
    utils::Sizef
    MainWindowLayout::computeSizeOfRoles(const std::vector<core::Layout::WidgetInfo>& roles) const noexcept {
      // Create a default size.
      utils::Sizef achievedSize;

      // Traverse the input information and append the size to the global achieved size.
      for (unsigned index = 0u ; index < roles.size() ; ++index) {
        // Increment the achieved size with the dimensions of the current box.
        achievedSize.w() += roles[index].area.w();
        achievedSize.h() += roles[index].area.h();
      }

      // Return the computed size.
      return achievedSize;
    }

    inline
    void
    MainWindowLayout::consolidatePolicyFromItem(const std::unordered_set<WidgetRole>& roles,
                                                WidgetInfo& policy,
                                                const WidgetInfo& item,
                                                const utils::Sizef& size) const noexcept
    {
      // Update minimum size only if it is larger than the current minimum.
      if (item.min.w() > policy.min.w()) {
        policy.min.w() = item.min.w();
      }

      if (item.min.h() > policy.min.h()) {
        policy.min.h() = item.min.h();
      }

      // Update maximum size only if it is larger than the current maximum.
      if (item.max.w() > policy.max.w()) {
        policy.max.w() = item.max.w();
      }

      if (item.max.h() > policy.max.h()) {
        policy.max.h() = item.max.h();
      }

      // Do not update size hint, we assume regular distribution during the
      // process.
      // On the other hand we now need to update the maximum size of the policy
      // based on the roles it is supposed to serve.
      utils::Sizef max = computeMaxSizeFromRoles(size, roles);

      if (policy.max.w() > max.w()) {
        policy.max.w() = max.w();
      }

      if (policy.max.h() > max.h()) {
        policy.max.h() = max.h();
      }

      // Update the policy if it contains flag `expanding`: this allows widgets
      // to expand if needed and does not prevent anything from shrinking because
      // we do not provide any size hint.
      if (item.policy.canExpandHorizontally()) {
        policy.policy.setHorizontalPolicy(core::SizePolicy::Expanding);
      }

      if (item.policy.canExpandVertically()) {
        policy.policy.setVerticalPolicy(core::SizePolicy::Expanding);
      }
    }

    inline
    utils::Sizef
    MainWindowLayout::computeMaxSizeFromRoles(const utils::Sizef& size,
                                              const std::unordered_set<WidgetRole>& roles) const noexcept
    {
      // The maximum size without constraints correspond to the input maximum size.
      utils::Sizef maxSize = size;

      // If the central widget role does exist in there, we can return early: indeed
      // the central widget will be allowed all of the space.
      if (roles.count(WidgetRole::CentralDockWidget)) {
        return maxSize;
      }

      // Now compute the portion of the width and height which is assigned to the roles
      // by traversing the input array.
      // We now know that the central widget does not belong to the input `roles` so we
      // can safely add percentages.
      float wPerc = 0.0f;
      float hPerc = 0.0f;

      for (std::unordered_set<WidgetRole>::const_iterator role = roles.cbegin() ;
           role != roles.cend() ;
           ++role)
      {
        // Retrieve the percentage based on the role.
        switch (*role) {
          case WidgetRole::MenuBar:
            hPerc += m_menuBarPercentage;
            break;
          case WidgetRole::StatusBar:
            hPerc += m_statusBarPercentage;
            break;
          case WidgetRole::ToolBar:
            hPerc += m_toolBarPercentage;
            break;
          case WidgetRole::LeftDockWidget:
            wPerc += m_leftAreaPercentage;
            break;
          case WidgetRole::RightDockWidget:
            wPerc += m_rightAreaPercentage;
            break;
          case WidgetRole::TopDockWidget:
            hPerc += m_topAreaPercentage;
            break;
          case WidgetRole::BottomDockWidget:
            hPerc += m_bottomAreaPercentage;
            break;
          default:
            break;
        }
      }

      // Scale the size by the percentage assigned to the input roles.
      maxSize.w() = wPerc;
      maxSize.h() = hPerc;

      // This is the maximum size available for all the roles.
      return maxSize;
    }

  }
}

#endif    /* MAIN_WINDOW_LAYOUT_HXX */
