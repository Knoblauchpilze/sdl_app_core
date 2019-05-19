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

  }
}

#endif    /* WIDGET_ROLE_HH */
