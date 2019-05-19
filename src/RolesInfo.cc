
# include "RolesInfo.hh"

namespace sdl {
  namespace app {

    void
    RolesInfo::consolidateRolesDimensions(const utils::Sizef& margin) {
      // Here we want to update the position of each area based on the dimensions
      // of their relative position. Upon entering this function, each area has
      // valid dimensions but no position yet.
      // We need to correct this fact so that we obtain a nice layout based on the
      // expected position of each area.
      // For example the central area should be offset so that it lands on the right
      // of the left area and on the left of the right area.
      // Basically we will update each individual area based on the position of the
      // layout.

      // Menu bar's position is only determined by the margins of this layout.
      assignOrCreatePositionForRole(
        WidgetRole::MenuBar,
        true,
        margin.w(),
        true,
        margin.h()
      );

      // Tool bar is right below the menu bar and is only offset along the `x` axis
      // based on the margin.
      float offset = margin.h() + getBoxForRole(WidgetRole::MenuBar).h();
      assignOrCreatePositionForRole(
        WidgetRole::ToolBar,
        true,
        margin.w(),
        true,
        offset
      );

      // The left area position's position is determined by the bottom bound of the
      // tool bar and is only offset along the `x` axis based on the margin.
      offset += getBoxForRole(WidgetRole::ToolBar).h();
      assignOrCreatePositionForRole(
        WidgetRole::LeftDockWidget,
        true,
        margin.w(),
        true,
        offset
      );

      // Top, central and bottom area are on the right of the left area.
      // Also each one is stacked on top of each other.
      const float xOffsetForCentralAreas = margin.w() + getBoxForRole(WidgetRole::LeftDockWidget).w();

      float yOffsetForCentralAreas = offset;
      assignOrCreatePositionForRole(
        WidgetRole::TopDockWidget,
        true,
        xOffsetForCentralAreas,
        true,
        yOffsetForCentralAreas
      );

      yOffsetForCentralAreas += getBoxForRole(WidgetRole::TopDockWidget).h();
      assignOrCreatePositionForRole(
        WidgetRole::CentralDockWidget,
        true,
        xOffsetForCentralAreas,
        true,
        yOffsetForCentralAreas
      );

      yOffsetForCentralAreas += getBoxForRole(WidgetRole::CentralDockWidget).h();
      assignOrCreatePositionForRole(
        WidgetRole::BottomDockWidget,
        true,
        xOffsetForCentralAreas,
        true,
        yOffsetForCentralAreas
      );

      // Right area is on the right of the central areas.
      const float xOffsetForRightArea = xOffsetForCentralAreas + getBoxForRole(WidgetRole::TopDockWidget).w();
      assignOrCreatePositionForRole(
        WidgetRole::TopDockWidget,
        true,
        xOffsetForRightArea,
        true,
        offset
      );

      // The status bar is below the left area.
      offset += getBoxForRole(WidgetRole::LeftDockWidget).h();
      assignOrCreatePositionForRole(
        WidgetRole::StatusBar,
        true,
        margin.w(),
        true,
        offset
      );
    }

  }
}