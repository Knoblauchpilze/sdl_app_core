#ifndef    MAIN_WINDOW_LAYOUT_HH
# define   MAIN_WINDOW_LAYOUT_HH

# include <memory>
# include <unordered_map>
# include <unordered_set>
# include <maths_utils/Box.hh>
# include <sdl_graphic/GridLayout.hh>
# include <sdl_core/SizePolicy.hh>
# include "WidgetRole.hh"

namespace sdl {
  namespace app {

    class MainWindowLayout: public graphic::GridLayout {
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

        void
        updatePrivate(const utils::Boxf& window) override;

        void
        adjustItemToConstraints(const utils::Sizef& window,
                                std::vector<WidgetInfo>& items) const noexcept override;

      private:

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

        /**
         * @brief - Remove all the widgets of this layout which are currently
         *          assuming the input `role`.
         *          Note that several calls to invalidate might be triggered if
         *          several widgets assume the input `role` in this layout.
         * @brief - The role for which all widgets should be removed.
         */
        void
        removeAll(const WidgetRole& role);

        void
        removeItemFromIndex(int item) override;

        utils::Boxi
        getGridCoordinatesFromRole(const WidgetRole& role) const;

        /**
         * @brief - Used to consolidate the input `gridCoords` by assuming it corresponds
         *          to the input `role`. This allows for example to extend the area allocated
         *          to the central widget if no top or bottom dock widgets are registered for
         *          this layout yet.
         * @param role - the role associated to the input `gridCoords`.
         * @param gridCoords - the input grid coordinates for the input role.
         */
        void
        consolidateGridCoordinatesFromRole(const WidgetRole& role,
                                           utils::Boxi& gridCoords) const;

        utils::Sizef
        computeMaxSizeForRole(const utils::Sizef& window,
                              const WidgetRole& role) const;

      private:

        void
        assignPercentagesFromCentralWidget(const utils::Sizef& centralWidgetSize);

        void
        addItemWithRoleAndArea(core::SdlWidget* widget,
                               const WidgetRole& role,
                               const DockWidgetArea& area = DockWidgetArea::None);

        /**
         * @brief - Used to remove the input `widget` assuming it has the specified
         *          `role` inside this layout. If this is not the case an error is
         *          raised. If the widget effectively has this role, it is removed
         *          from the layout by calling the parent method.
         * @param widget - the widget for whith the index should be retrieved.
         * @param role - the role for which the index should be registered.
         */
        void
        removeItemFromRole(core::SdlWidget* widget,
                           const WidgetRole& role);

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
