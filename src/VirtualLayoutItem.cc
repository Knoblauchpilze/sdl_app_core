
# include "VirtualLayoutItem.hh"
# include <sdl_engine/ResizeEvent.hh>

namespace sdl {
  namespace app {

    VirtualLayoutItem::VirtualLayoutItem(const std::string& name,
                                         const utils::Sizef& hint):
      core::LayoutItem(name, hint, false, true),
      m_manageWidth(false),
      m_manageHeight(false),
      m_box()
    {}

    void
    VirtualLayoutItem::postLocalEvent(core::engine::EventShPtr e) {
      // The virtual layout item does not perform events queuing.
      // Instead it performs direct analysis of the event to react
      // upon receiving a resize event.

      // Check whether this is a resize event.
      if (e == nullptr || e->getType() != core::engine::Event::Type::Resize) {
        // Discard this event.
        return;
      }

      // Cast the input event into its dynamic type.
      core::engine::ResizeEventShPtr resize = std::dynamic_pointer_cast<core::engine::ResizeEvent>(e);

      // We need to keep only the coordinate and dimensions as indicated
      // by the internal flags.
      const utils::Boxf box = resize->getNewSize();

      if (isWidthManaged()) {
        m_box.x() = box.x();
        m_box.w() = box.w();
      }

      if (isHeightManaged()) {
        m_box.y() = box.y();
        m_box.h() = box.h();
      }

      log("Virtual item has size " + box.toString() + " and local " + m_box.toString(), utils::Level::Error);
    }

  }
}