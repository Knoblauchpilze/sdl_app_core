
# include "VirtualLayoutItem.hh"
# include <sdl_engine/ResizeEvent.hh>

namespace sdl {
  namespace app {

    VirtualLayoutItem::VirtualLayoutItem(const std::string& name,
                                         const utils::Sizef& min,
                                         const utils::Sizef& hint,
                                         const utils::Sizef& max,
                                         const core::SizePolicy& policy):
      core::LayoutItem(name, hint),
      m_manageWidth(false),
      m_manageHeight(false),
      m_box()
    {
      // Assign size hints.
      setMinSize(min);
      setMaxSize(max);
      setSizePolicy(policy);

      // Register this object as a filter event for itself: it allows to
      // filter any event which is not of type `Resize`.
      installEventFilter(this);
    }

    bool
    VirtualLayoutItem::filterEvent(core::engine::EngineObject* watched,
                                   core::engine::EventShPtr e)
    {
      // We want to filter any event which is not of type `Resize` so let's proceed.
      // As an additional security measure, we also check that the `watched` object
      // corresponds to `this` object so that we don't filter events for somebody
      // else without even noticing.

      // No filtering for other object than `this`.
      if (watched != this) {
        return false;
      }

      // No filtering of event with type `Resize`.
      return e->getType() != core::engine::Event::Type::Resize;
    }

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
    }

    void
    VirtualLayoutItem::updateMaxSize(const utils::Sizef& upperBound) {
      // We need to update the internal max, min and size hint based on the value
      // of the input `upperBound`.
      // Basically we want to decrease the maximum size to not be greater than the
      // `upperBound`. We also want to decrease the size hint to not be greater
      // than the new maximum size. This is only possible if the widget can shrink
      // and if the minimum size allows it.
      // And we want to make sure that the input `upperBound` is not smaller than
      // the minimum size otherwise we will not be able to apply the computed size
      // from the layout item to the actual real widget.

      // In order to work efficiently, we retrieve each size hint into a local
      // variable to avoid posting events all the time.
      utils::Sizef min = getMinSize();
      utils::Sizef hint = getSizeHint();
      utils::Sizef max = getMaxSize();

      // First, let's handle trivial cases where the maximum size is already smaller
      // than the `upperBound`. If this is the case, as we assume that the initial
      // configuration of the layout item is valid it means that both the size hint
      // and minimum size are also smaller than the `upperBound`.
      if (max.w() <= upperBound.w() && max.h() <= upperBound.h()) {
        // All good, nothing to do.
        return;
      }

      // We know at this step that at least the width of the height of the maximum
      // size is greater than the provided `upperBound`.
      // We need to update it first.
      if (max.w() > upperBound.w()) {
        max.w() = upperBound.w();
      }
      if (max.h() > upperBound.h()) {
        max.h() = upperBound.h();
      }

      // Now the maximum size is consistent with the desired `upperBound`. We need
      // to handle the size hint. If it is not valid, nothing to worry about. Otherwise
      // we need to make sure that it is not greater than the maximum size.

      // All this is scheduled only if the hint is valid.
      if (hint.isValid()) {

        // Check whether we need to update the hint.
        if (hint.w() > max.w() || hint.h() > max.h()) {
          // The current `hint` size is larger than the desired maximum size based on
          // the input `upperBound`. This is only a problem if we cannot shrink the
          // widget: otherwise we can just shrink it and move on the handling of the
          // minimum size.

          if (hint.w() > max.w()) {
            // Check whether we can shrink the widget horizontally.
            if (!getSizePolicy().canShrinkHorizontally()) {
              // The widget cannot be shrunk, this is a problem.
              error(
                std::string("Cannot assign upper bound " + upperBound.toString() + " to layout item"),
                std::string("Widget cannot shrink horizontally")
              );
            }
            else {
              hint.w() = max.w();
            }
          }

          if (hint.h() > max.h()) {
            // Check whether we can shrink the widget vertically.
            if (!getSizePolicy().canShrinkVertically()) {
              // The widget cannot be shrunk, this is a problem.
              error(
                std::string("Cannot assign upper bound " + upperBound.toString() + " to layout item"),
                std::string("Widget cannot shrink vertically")
              );
            }
            else {
              hint.h() = max.h();
            }
          }
        }
      }

      // The size hint is now eiether not existing or consistent with the maximum size.
      // One last step is to ensure that the minimum size is also consistent with it.
      // Basically we cannot do much here, either the maximum size is larger than the
      // minimum size and we're all set, or it is not in which case it means that no
      // matter what we do we will not be able to assign properly the computed size to
      // the real widget afterwards.

      // All this is bound to whether we have a minimum size in the firts place.
      if (min.isValid()) {
        if (min.w() > max.w()) {
          // The minimum size is larger than the desired `upperBound`, this is a problem.
          error(
            std::string("Cannot assign upper bound " + upperBound.toString() + " to layout item"),
            std::string("Inconsistent with desired minimum width")
          );
        }

        if (min.h() > max.h()) {
          // The minimum size is larger than the desired `upperBound`, this is a problem.
          error(
            std::string("Cannot assign upper bound " + upperBound.toString() + " to layout item"),
            std::string("Inconsistent with desired minimum height")
          );
        }
      }

      // When reaching this point, we have updated all the size hints for this layout item,
      // we only have to assign it so that it is used in the next optimization process.
      setMinSize(min);
      setSizeHint(hint);
      setMaxSize(max);
    }

  }
}