#ifndef    WIDGET_ROLE_HH
# define   WIDGET_ROLE_HH

namespace sdl {
  namespace app {

    /**
     * @brief - Enumeration to describe the role of each widget
     *          added to this layout.
     */
    enum class WidgetRole {
      MenuBar,
      StatusBar,
      ToolBar,
      LeftDockWidget,
      RightDockWidget,
      TopDockWidget,
      BottomDockWidget,
      CentralDockWidget
    };

    /**
     * @brief - Enumeration to describe the area associated to a
     *          dock widget.
     */
    enum class DockWidgetArea {
      None        = 0,
      LeftArea    = 1 << 0,
      RightArea   = 1 << 1,
      TopArea     = 1 << 2,
      BottomArea  = 1 << 3,
      CentralArea = 1 << 4,
      All         = 1 << 5
    };

    std::string
    areaToName(const DockWidgetArea& area) noexcept;

    std::string
    roleToName(const WidgetRole& role) noexcept;

    bool
    isDockWidgetRole(const WidgetRole& role) noexcept;

    WidgetRole
    roleFromArea(const DockWidgetArea& area);

  }
}

# include "WidgetRole.hxx"

#endif    /* WIDGET_ROLE_HH */
