#ifndef    MAIN_WINDOW_LAYOUT_HH
# define   MAIN_WINDOW_LAYOUT_HH

# include <memory>
# include <maths_utils/Box.hh>
# include <sdl_core/Layout.hh>

namespace sdl {
  namespace app {

    class MainWindowLayout: public core::Layout {
      public:

        MainWindowLayout(const utils::Boxf& area,
                         const float& margin = 1.0f);

        virtual ~MainWindowLayout();

        void
        setArea(const utils::Boxf& area) noexcept;

        void
        update() override;

      protected:

        /**
         * @brief - Enumeration to describe the role of each widget
         *          added to this layout.
         */
        enum class WidgetRole {
          MenuBar,
          StatusBar,
          Toolbar,
          DockWidget,
          CentralWidget
        };

        void
        updatePrivate(const utils::Boxf& window) override;

      private:

        utils::Boxf m_area;

    };

    using MainWindowLayoutShPtr = std::shared_ptr<MainWindowLayout>;
  }
}

# include "MainWindowLayout.hxx"

#endif    /* MAIN_WINDOW_LAYOUT_HH */
