#ifndef    MAIN_WINDOW_LAYOUT_HH
# define   MAIN_WINDOW_LAYOUT_HH

# include <memory>
# include <unordered_map>
# include <unordered_set>
# include <maths_utils/Box.hh>
# include <sdl_core/Layout.hh>
# include <sdl_core/SizePolicy.hh>
# include <sdl_graphic/GridLayout.hh>
# include "WidgetRole.hh"
# include "VirtualLayoutItem.hh"

namespace sdl {
  namespace app {

    class MainWindowLayout: public core::Layout {
      public:

        /**
         * @brief - Creates a new main window layout with the specified area.
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
        MainWindowLayout(float margin = 1.0f,
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

        /**
         * @brief - Reimplementation of the `EngineObject` method which allows this
         *          class to not only register itself to the provided events queue
         *          but also its chidlren layouts.
         * @param queue - the queue to register.
         */
        void
        setEventsQueue(core::engine::EventsQueue* queue) noexcept override;

      protected:

        void
        computeGeometry(const utils::Boxf& window) override;

        /**
         * @brief - Reimplementation of the base `Layout` method to provide update of the
         *          internal associations table which describes the role for each widget.
         * @param logicID - the logical id which has just been removed.
         * @param physID - the physical id which has just been removed.
         * @return - true as this layout always needs a rebuild when an item is removed.
         */
        bool
        onIndexRemoved(const int logicID,
                       const int physID) override;

        /**
         * @brief - Used to determine which dimensions are managed by the internal
         *          layout's constraints for a specific role. Indeeed the menu bar
         *          for example will always be assigned the complete width of the
         *          layout no matter its dimensions. So we do not want it to interfere
         *          with the repartition of the width of the window for other widgets.
         * @param role - the role for which the dimensions should be checked for
         *               micro-management.
         * @return - true if the role has its dimensions managed by the maximum
         *           constraints of this layout. The first element of the returned pair
         *           corresponds to the width while the second corresponds to the height.
         */
        std::pair<bool, bool>
        dimensionManagedForRole(const WidgetRole& role) const noexcept;

        utils::Boxi
        getGridCoordinatesFromRole(const WidgetRole& role,
                                   const bool hRole) const;

      private:

        /**
         * @brief - Describes additional information to locate the widget
         *          in the layout. Each data is linked to a widget which
         *          is represented through the `widget` attribute. To get
         *          the index of the item in the parent `m_items` table
         *          one can use the `getIndexOf` method.
         *          The role of the widget is specified using the `role`
         *          attribute and according to the role the relevant area
         *          is provided to locate the widget in the layout.
         *          Note that if the role for the widget is not set to
         *          `DockWidget` the `area` attribute is not relevant and
         *          set to its default value, i.e. `None`.
         *          Finally due to the way we handle the repartition of
         *          the widgets, we associate a virtual layout item to any
         *          new widget so that we can gather information from the
         *          internal layouts without polluting the real widgets
         *          with uneeded events.
         */
        struct ItemInfo {
          WidgetRole role;
          DockWidgetArea area;
          core::SdlWidget* widget;
          VirtualLayoutItemShPtr item;
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

        utils::Sizef
        computeMaxSizeForRole(const utils::Sizef& window,
                              const WidgetRole& role) const;


        /**
         * @brief - Used to return a prefix which can be used to provide a name slightly
         *          different from the initial item name when creating a virtual layout
         *          item.
         * @return - the virtual layout item name prefix.
         */
        static
        std::string
        getVirtualLayoutNamePrefix() noexcept;

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

        /**
         * @brief - These layouts allow to handle the repartition of items in both axes.
         *          The `m_hLayout` handles the positioning of items along the x axis (i.e.
         *          handling of their widths) while the `m_vLayout` handles the computing
         *          of the heights of widgets.
         *          The dimension of the `m_hLayout` is thus 1column x 6 rows and for the
         *          `m_vLayout` it is 3 columns x 3 rows.
         *          Note that some widgets are added to only one layout, which means we
         *          have to handle the missing dimension afterwards.
         */
        graphic::GridLayout m_hLayout;
        graphic::GridLayout m_vLayout;

    };

    using MainWindowLayoutShPtr = std::shared_ptr<MainWindowLayout>;
  }
}

# include "MainWindowLayout.hxx"

#endif    /* MAIN_WINDOW_LAYOUT_HH */
