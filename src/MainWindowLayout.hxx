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
      removeAll(WidgetRole::CentralWidget);

      // Use internal handler.
      addItemWithRoleAndArea(item, WidgetRole::CentralWidget, DockWidgetArea::CentralArea);
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
    MainWindowLayout::WidgetRole
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
          role = WidgetRole::CentralWidget;
          break;
        default:
          error(
            std::string("Could not determine widget role for area ") + std::to_string(static_cast<int>(area)),
            std::string("Invalid dock area")
          );
          break;
      }

      return role;
    }

    inline
    bool
    MainWindowLayout::isValidDockWidgetRole(const WidgetRole& role) noexcept {
      return
        role == WidgetRole::LeftDockWidget ||
        role == WidgetRole::RightDockWidget ||
        role == WidgetRole::TopDockWidget ||
        role == WidgetRole::BottomDockWidget
      ;
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
        case WidgetRole::CentralWidget:
          return "central_dock_widget";
        default:
          return "unknown_role";
      }
    }

    inline
    void
    MainWindowLayout::addItemWithRoleAndArea(core::SdlWidget* widget,
                                             const WidgetRole& role,
                                             const DockWidgetArea& area)
    {
      // Add the item using the base handler.
      const int index = addItem(widget);

      // Register this item in the internal table of information if
      // a valid index was generated.
      if (index >= 0) {
        m_infos[index] = {
          index,
          role,
          area
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
    MainWindowLayout::consolidatePolicyFromItem(WidgetInfo& policy,
                                                const WidgetInfo& item) noexcept
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
    void
    MainWindowLayout::assignOrCreateWidthForRole(const WidgetRole& role,
                                                 const float& width,
                                                 RolesInfo& roles) const
    {
      // Check whether the input `role` already exist in the input information array.
      RolesInfo::iterator boxToUpdate = roles.find(role);

      if (boxToUpdate == roles.cend()) {
        // This role does not exist yet, register it.
        roles[role] = utils::Boxf(
          0.0f,
          0.0f,
          width,
          0.0f
        );
      }
      else {
        // The role already exists, replace the width value.
        boxToUpdate->second.w() = width;
      }
    }

    inline
    void
    MainWindowLayout::assignOrCreateHeightForRole(const WidgetRole& role,
                                                  const float& height,
                                                  RolesInfo& roles) const
    {
      // Check whether the input `role` already exist in the input information array.
      RolesInfo::iterator boxToUpdate = roles.find(role);

      if (boxToUpdate == roles.cend()) {
        // This role does not exist yet, register it.
        roles[role] = utils::Boxf(
          0.0f,
          0.0f,
          0.0f,
          height
        );
      }
      else {
        // The role already exists, replace the height value.
        boxToUpdate->second.h() = height;
      }
    }

    inline
    void
    MainWindowLayout::assignAbscissaForRole(const WidgetRole& role,
                                            const float& x,
                                            RolesInfo& roles) const
    {
      // Check whether the input `role` already exist in the input information array.
      RolesInfo::iterator boxToUpdate = roles.find(role);
      
      if (boxToUpdate == roles.cend()) {
        error(
          std::string("Could not update abscissa for role ") + getNameFromRole(role),
          std::string("No such role available")
        );
      }

      // Update the abscissa with the provided argument.
      boxToUpdate->second.x() = x;
    }

    inline
    void
    MainWindowLayout::assignOrdinateForRole(const WidgetRole& role,
                                            const float& y,
                                            RolesInfo& roles) const
    {
      // Check whether the input `role` already exist in the input information array.
      RolesInfo::iterator boxToUpdate = roles.find(role);
      
      if (boxToUpdate == roles.cend()) {
        error(
          std::string("Could not update ordinate for role ") + getNameFromRole(role),
          std::string("No such role available")
        );
      }

      // Update the ordinate with the provided argument.
      boxToUpdate->second.y() = y;
    }

    inline
    utils::Boxf
    MainWindowLayout::getLocationOfRole(const WidgetRole& role,
                                        RolesInfo& roles) const
    {
      // Check whether the input `role` already exist in the input information array.
      RolesInfo::iterator boxToUpdate = roles.find(role);
      
      if (boxToUpdate == roles.cend()) {
        error(
          std::string("Could not retrieve location for role ") + getNameFromRole(role),
          std::string("No such role available")
        );
      }

      return boxToUpdate->second;
    }

  }
}

#endif    /* MAIN_WINDOW_LAYOUT_HXX */
