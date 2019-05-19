#ifndef    ROLES_INFO_HH
# define   ROLES_INFO_HH

# include <unordered_map>
# include <maths_utils/Box.hh>
# include "WidgetRole.hh"

namespace sdl {
  namespace app {

    class RolesInfo {
      public:

        RolesInfo();

        ~RolesInfo() = default;

        /**
         * @brief - Assigns or creates a box with the specified dimensions
         *          for the input `role`. One can specify which dimensions
         *          should be assigned for the `role`. If a dimension is
         *          not set to be assigned, its value will be left unmodified
         *          if the data for the role already exists and set to `0`
         *          otherwise.
         * @param role - the role for which data should be assigned or created.
         * @param setWidth - true if the width of the data for the input `role`
         *                   should be defined.
         * @param w - the value of the width for the `role`. Note that this
         *            value is ignored if `setWidth` is set to `false`.
         * @param setHeight - true if the height of the data for the input `role`
         *                   should be defined.
         * @param h - the value of the height for the `role`. Note that
         *            this value is ignored if `setHeight` is set to `false`.
         */
        void
        assignOrCreateDimsForRole(const WidgetRole& role,
                                  const bool setWidth = false,
                                  const float& w = 0.0f,
                                  const bool setHeight = false,
                                  const float& h = 0.0f);

        /**
         * @brief - Assigns or creates a bow with the specified position
         *          for the input `role`. One can specify which coordinates
         *          should be assigned for the `role`. If a coordinate is not
         *          set to be assigned, its value will be left unmodified
         *          if the data for the role already exists and set to `0`
         *          otherwise.
         * @param role - the role for which data should be assigned or created.
         * @param setAbscissa - true if the abscissa of the data for the input
         *                      `role` should be defined.
         * @param abscissa - the value of the abscissa for the `role`. Note that
         *                   this value is ignored if `setAbscissa` is set to
         *                   `false`.
         * @param setOrdinate - true if the ordinate of the data for the input
         *                      `role` should be defined.
         * @param ordinate - the value of the ordinate for the `role`. Note that
         *                   this value is ignored if `setOrdinate` is set to
         *                   `false`.
         */
        void
        assignOrCreatePositionForRole(const WidgetRole& role,
                                      const bool setAbscissa = false,
                                      const float& x = 0.0f,
                                      const bool setOrdinate = false,
                                      const float& y = 0.0f);

        utils::Boxf
        getBoxForRole(const WidgetRole& role) const noexcept;

        /**
         * @brief - Used to assign valid positions to each role defined in the internal
         *          table based on the layout we want to build for each role. This method
         *          effectively describes the relative position of widgets and areas
         *          between each other.
         *          The aim is to provide a final set of boxes in the internal `m_roles`
         *          table which can be used to assign position to each widget.
         *          The user can then fetch the resulting box for each area through the
         *          `getBoxForRole` method.
         * @param margin - a description of the unusable space along the borders of the
         *                 layout associated to the widget's position.
         */
        void
        consolidateRolesDimensions(const utils::Sizef& margin);

      private:

        using Roles = std::unordered_map<WidgetRole, utils::Boxf>;

        Roles m_roles;
    };

  }
}

# include "RolesInfo.hxx"

#endif    /* ROLES_INFO_HH */
