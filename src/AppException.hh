#ifndef    APPEXCEPTION_HH
# define   APPEXCEPTION_HH

# include <core_utils/CoreException.hh>

namespace sdl {
  namespace app {

    class AppException: public utils::CoreException {
      public:

        AppException(const std::string& message,
                     const std::string& module,
                     const std::string& cause = std::string());

        virtual ~AppException() = default;

      private:

        static const char* sk_serviceName;
    };

  }
}

# include "AppException.hxx"

#endif    /* APPEXCEPTION_HH */