
# include "MainWindowLayout.hh"

namespace sdl {
  namespace app {

    MainWindowLayout::MainWindowLayout(const utils::Boxf& area,
                                       const float& margin):
      core::Layout(nullptr, margin),
      m_area(area)
    {}

    MainWindowLayout::~MainWindowLayout() {}

    void
    MainWindowLayout::updatePrivate(const utils::Boxf& window) {
      // The main window layout can be represented as below:
      //
      //  +---------------------------------+
      //  |             Menu bar            |
      //  +---------------------------------+
      //  |            Toolbars             |
      //  |                                 |
      //  |  +---+-------------------+---+  |
      //  |  |   |   Dock widgets    |   |  |
      //  |  +---+-------------------+---+  |
      //  |  |   |                   |   |  |
      //  |  |   |  Central  widget  |   |  |
      //  |  |   |                   |   |  |
      //  |  +---+-------------------+---+  |
      //  |  |   |                   |   |  |
      //  |  +---+-------------------+---+  |
      //  |                                 |
      //  |---------------------------------+
      //  |            Status bar           |
      //  +---------------------------------+
      //
      // Each region can have its own widgets assigned to it,
      // each one defined and added using the provided enumeration
      // to describe the role of the widget added.
      // Each area is expanded/grown/shrunk according to the policy
      // of the widgets inside it to reach a correct state.
      // Note that the initial repartition of space is not equivalent
      // between regions, which correspond to the intuitive idea of
      // the central widget being more important than the other elements.
      //
      // Even though several widgets can occupy most of the roles, only
      // one widget can be assigned as the central widget.
      // TODO: Implementation.
      log("Requesting update with window " + window.toString());
    }

  }
}
