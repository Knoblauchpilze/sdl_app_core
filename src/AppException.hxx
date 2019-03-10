#ifndef    APPEXCEPTION_HXX
# define   APPEXCEPTION_HXX

# include "AppException.hh"

namespace sdl {
  namespace app {

    inline
    AppException::AppException(const std::string& message,
                               const std::string& cause):
      utils::core::CoreException(message, sk_moduleName, cause)
    {}

  }
}

#endif    /* APPEXCEPTION_HXX */