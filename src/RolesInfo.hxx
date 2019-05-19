#ifndef    ROLES_INFO_HXX
# define   ROLES_INFO_HXX

# include "RolesInfo.hh"

namespace sdl {
  namespace app {

    inline
    RolesInfo::RolesInfo():
      m_roles()
    {}

    inline
    void
    RolesInfo::assignOrCreateDimsForRole(const WidgetRole& role,
                                         const bool setWidth,
                                         const float& w,
                                         const bool setHeight,
                                         const float& h)
    {
      // Check whether at least one of the dimensions should be set.
      if (!setWidth && !setHeight) {
        return;
      }

      // Check whether the input `role` already exist in the input information array.
      Roles::iterator boxToUpdate = m_roles.find(role);

      if (boxToUpdate == m_roles.cend()) {
        // Create an empty box.
        utils::Boxf box;

        // Assign the dimensions based on the input requirements.
        if (setWidth) {
          box.w() = w;
        }

        if (setHeight) {
          box.h() = h;
        }

        // Insert this box under the specified role.
        m_roles[role] = box;
      }
      else {
        // Update the required dimensions.
        if (setWidth) {
          boxToUpdate->second.w() = w;
        }

        if (setHeight) {
          boxToUpdate->second.h() = h;
        }
      }
    }

    inline
    void
    RolesInfo::assignOrCreatePositionForRole(const WidgetRole& role,
                                             const bool setAbscissa,
                                             const float& x,
                                             const bool setOrdinate,
                                             const float& y)
    {
      // Check whether at least one of the coordinates should be set.
      if (!setAbscissa && !setOrdinate) {
        return;
      }

      // Check whether the input `role` already exist in the input information array.
      Roles::iterator boxToUpdate = m_roles.find(role);
      
      if (boxToUpdate == m_roles.cend()) {
        // Create an empty box.
        utils::Boxf box;

        // Assign the position based on the input requirements.
        if (setAbscissa) {
          box.x() = x;
        }

        if (setOrdinate) {
          box.y() = y;
        }

        // Insert this box under the specified role.
        m_roles[role] = box;
      }
      else {
        // Update the required coordinates.
        if (setAbscissa) {
          boxToUpdate->second.x() = x;
        }

        if (setOrdinate) {
          boxToUpdate->second.y() = y;
        }
      }
    }

    inline
    utils::Boxf
    RolesInfo::getBoxForRole(const WidgetRole& role) const noexcept {
      // Check whether the input `role` already exists in the input information array.
      Roles::const_iterator boxForRole = m_roles.find(role);

      if (boxForRole == m_roles.cend()) {
        // Return an empty box.
        return utils::Boxf();
      }

      return boxForRole->second;
    }

  }
}

#endif    /* ROLES_INFO_HXX */
