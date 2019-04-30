#ifndef    MAIN_WINDOW_LAYOUT_HH
# define   MAIN_WINDOW_LAYOUT_HH

# include <memory>
# include <unordered_map>
# include <maths_utils/Box.hh>
# include <sdl_core/Layout.hh>

namespace sdl {
  namespace app {

    class MainWindowLayout: public core::Layout {
      public:

        enum class DockWidgetArea {
          None       = 0,
          LeftArea   = 1 << 0,
          RightArea  = 1 << 1,
          TopArea    = 1 << 2,
          BottomArea = 1 << 3,
          All        = 1 << 4
        };

      public:

        MainWindowLayout(const utils::Boxf& area,
                         const float& margin = 1.0f);

        virtual ~MainWindowLayout();

        void
        setArea(const utils::Boxf& area) noexcept;

        void
        update() override;

        void
        setMenuBar(core::SdlWidget* item);

        void
        addToolBar(core::SdlWidget* item);

        void
        setCentralWidget(core::SdlWidget* item);

        void
        addDockWidget(core::SdlWidget* item,
                      const DockWidgetArea& area);

        void
        setStatusBar(core::SdlWidget* item);

        void
        removeToolBar(core::SdlWidget* item);

        void
        removeDockWidget(core::SdlWidget* item);

      protected:

        /**
         * @brief - Enumeration to describe the role of each widget
         *          added to this layout.
         */
        enum class WidgetRole {
          MenuBar,
          StatusBar,
          ToolBar,
          DockWidget,
          CentralWidget
        };

        /**
         * @brief - Describes additional information to locate the widget
         *          in the layout. Each data is linked to a widget which
         *          can be found at the index `widget` of the `m_items`
         *          list described by the parent `Layout` class. The role
         *          of the widget is specified using the `role` attribute
         *          and according to the role the relevant area is provided
         *          to locate the widget in the layout. Note that if the
         *          role for the widget is not set to `DockWidget` the
         *          `area` attribute is not relevant and set to its default
         *          value, i.e. `None`.
         */
        struct ItemInfo {
          int widget;
          WidgetRole role;
          DockWidgetArea area;
        };

        using InfoMap = std::unordered_map<int, ItemInfo>;

        void
        updatePrivate(const utils::Boxf& window) override;

      private:

        void
        addItemWithRoleAndArea(core::SdlWidget* widget,
                               const WidgetRole& role,
                               const DockWidgetArea& area = DockWidgetArea::None);

        void
        removeAll(const WidgetRole& role);

        /**
         * @brief - Used to retrieve the index of the input `widget` and check that
         *          it is registered with the provided role inside this layout. If
         *          this is not the case an error is raised.
         *          If the widget effectively has this role, its index is returned.
         * @param widget - the widget for whith the index should be retrieved.
         * @param role - the role for which the index should be registered.
         * @return - the index of this widget in the layout.
         */
        int
        getIndexAndCheck(core::SdlWidget* widget,
                         const WidgetRole& role);

      private:

        utils::Boxf m_area;
        InfoMap m_infos;

    };

    using MainWindowLayoutShPtr = std::shared_ptr<MainWindowLayout>;
  }
}

# include "MainWindowLayout.hxx"

#endif    /* MAIN_WINDOW_LAYOUT_HH */
