#ifndef    MAIN_WINDOW_LAYOUT_HH
# define   MAIN_WINDOW_LAYOUT_HH

# include <memory>
# include <unordered_map>
# include <unordered_set>
# include <maths_utils/Box.hh>
# include <sdl_core/Layout.hh>
# include <sdl_core/SizePolicy.hh>
# include "WidgetRole.hh"
# include "RolesInfo.hh"

namespace sdl {
  namespace app {

    class MainWindowLayout: public core::Layout {
      public:

        enum class DockWidgetArea {
          None        = 0,
          LeftArea    = 1 << 0,
          RightArea   = 1 << 1,
          TopArea     = 1 << 2,
          BottomArea  = 1 << 3,
          CentralArea = 1 << 4,
          All         = 1 << 5
        };

      public:

        /**
         * @brief - The area available for the layout.
         * @param margin - the margin around the borders of the layout. Expressed in pixels and similar
         *                 for width and height.
         * @param centralWidgetSize - a size describing both for width and height the percentage of the
         *                            total area occupied by the central widget. The rest of the area
         *                            is divided between the other sections.
         *                            Note that the values should be in the range `[0; 1]` with `0`
         *                            meaning that the central widget does not have any portion of the
         *                            total area and `1` meaning that it occupies all the available
         *                            space.
         */
        MainWindowLayout(const utils::Boxf& area,
                         const float& margin = 1.0f,
                         const utils::Sizef& centralWidgetSize = utils::Sizef(0.7f, 0.5f));

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
         * @brief - Describes additional information to locate the widget
         *          in the layout. Each data is linked to a widget which
         *          is represneted through the `widget` attribute. To get
         *          the index of the item in the parent `m_items` table
         *          one can use the `getIndexOf` method.
         *          The role of the widget is specified using the `role`
         *          attribute and according to the role the relevant area
         *          is provided to locate the widget in the layout.
         *          Note that if the role for the widget is not set to
         *          `DockWidget` the `area` attribute is not relevant and
         *          set to its default value, i.e. `None`.
         */
        struct ItemInfo {
          WidgetRole role;
          DockWidgetArea area;
          core::SdlWidget* widget;
        };

        using InfosMap = std::unordered_map<int, ItemInfo>;

        void
        updatePrivate(const utils::Boxf& window) override;

        void
        invalidate() noexcept override;

        void
        removeItemFromIndex(int item) override;

      private:

        // Inline functions.

        static
        std::string
        getNameFromArea(const DockWidgetArea& area) noexcept;

        static
        std::string
        getNameFromRole(const WidgetRole& role) noexcept;

        static
        bool
        isValidDockWidgetRole(const WidgetRole& role) noexcept;

        WidgetRole
        determineDockWidgetRoleFromArea(const DockWidgetArea& area);

        void
        assignPercentagesFromCentralWidget(const utils::Sizef& centralWidgetSize);

        void
        addItemWithRoleAndArea(core::SdlWidget* widget,
                               const WidgetRole& role,
                               const DockWidgetArea& area = DockWidgetArea::None);

        // Implemented functions.

        /**
         * @brief - Remove all the widgets of this layout which are currently
         *          assuming the input `role`.
         *          Note that several calls to invalidate might be triggered if
         *          several widgets assume the input `role` in this layout.
         * @brief - The role for which all widgets should be removed.
         */
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

        void
        adjustRolesHorizontally(const utils::Sizef& window,
                                const std::vector<WidgetInfo>& widgetsInfo,
                                RolesInfo& roles);

        void
        adjustRolesVertically(const utils::Sizef& window,
                              const std::vector<WidgetInfo>& widgetsInfo,
                              RolesInfo& roles);

        /**
         * @brief - Used to compute the global size policy for the input roles from the
         *          widgets described by the array `widgetsInfo`. Basically combines the
         *          individual widget's policies into a single global one which is the
         *          less restrictive and can be used as a rwa approximation of the widgets
         *          seen as a single one.
         * @param roles - a set of roles which indicates the widgets to consider.
         * @param widgetsInfo - the information about the policy for individual widgets.
         * @return - a single policy for all the widgets assuming the input roles. This
         *           includes a minimum and maximum size, a hint and a policy.
         */
        core::Layout::WidgetInfo
        computeSizePolicyForRoles(const std::unordered_set<WidgetRole>& roles,
                                  const std::vector<WidgetInfo>& widgetsInfo) const;

        /**
         * @brief - Similar to `computeSizePolicyForDockWidgets` but dedicated to compute
         *          a policy for dock widgets. This is a bit special because the dock
         *          widgets staks in the vertical direction and thus we cannot just select
         *          the minimum/maximum item from the policies.
         * @param widgetsInfo - the information about the policy for individual widgets.
         * @return - a single policyt for all the dock widgets assuming the input roles.
         */
        core::Layout::WidgetInfo
        computeSizePolicyForDockWidgets(const std::vector<WidgetInfo>& widgetsInfo) const;

        utils::Sizef
        computeSizeOfRoles(const std::vector<core::Layout::WidgetInfo>& roles) const noexcept;

        static
        void
        consolidatePolicyFromItem(WidgetInfo& policy,
                                  const WidgetInfo& item) noexcept;

      private:

        utils::Boxf m_area;
        InfosMap m_infos;

        /**
         * @brief - The main window layout is divided into three horizontal sections:
         *          the left part, the right part and the remaining section is occupied
         *          by the central widget.
         *          Each section occupies a certain percentage of the total available
         *          area as described by the variables below.
         *          The sum of the percentage should add up to 1.
         *          Note that the percentage of the area occupied by the central widget
         *          is deduced from the two other percentages.
         */
        float m_leftAreaPercentage;
        float m_rightAreaPercentage;

        /**
         * @brief - The main window layout is divided into six vertical sections:
         *          the menu bar, the tool bars, the top widgets, the central widget, the
         *          bottom section and the status bar.
         *          As for the horizontal sections, the percentages should add up to 1.
         *          Note that the percentage of the area occupied by the central widget
         *          is deduced from the other percentages.
         */
        float m_menuBarPercentage;
        float m_toolBarPercentage;
        float m_topAreaPercentage;
        float m_bottomAreaPercentage;
        float m_statusBarPercentage;

    };

    using MainWindowLayoutShPtr = std::shared_ptr<MainWindowLayout>;
  }
}

# include "MainWindowLayout.hxx"

#endif    /* MAIN_WINDOW_LAYOUT_HH */
