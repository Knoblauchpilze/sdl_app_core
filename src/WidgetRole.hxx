#ifndef    WIDGET_ROLE_HXX
# define   WIDGET_ROLE_HXX

# include "WidgetRole.hh"
# include <core_utils/CoreException.hh>

namespace sdl {
  namespace app {

    inline
    std::string
    areaToName(const DockWidgetArea& area) noexcept {
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
    roleToName(const WidgetRole& role) noexcept {
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
    isDockWidgetRole(const WidgetRole& role) noexcept {
      return role != WidgetRole::MenuBar && role != WidgetRole::StatusBar && role != WidgetRole::ToolBar;
    }

    inline
    WidgetRole
    roleFromArea(const DockWidgetArea& area) {
      switch (area) {
        case DockWidgetArea::LeftArea:
          return WidgetRole::LeftDockWidget;
        case DockWidgetArea::RightArea:
          return WidgetRole::RightDockWidget;
        case DockWidgetArea::TopArea:
          return WidgetRole::TopDockWidget;
        case DockWidgetArea::BottomArea:
          return WidgetRole::BottomDockWidget;
        case DockWidgetArea::CentralArea:
          return WidgetRole::CentralDockWidget;
        default:
          break;
      }

      throw utils::CoreException(
        "Could not determine widget role for area " + areaToName(area),
        "app_utils",
        "app_core",
        "Invalid dock area"
      );
    }

  }
}

#endif    /* WIDGET_ROLE_HXX */
