#ifndef    APPEXCEPTION_HXX
# define   APPEXCEPTION_HXX

# include "AppException.hh"

namespace sdl {
  namespace app {

    inline
    AppException::AppException(const std::string& message,
                               const std::string& module,
                               const std::string& cause):
      utils::CoreException(message, module, sk_serviceName, cause)
    {}

  }
}

#endif    /* APPEXCEPTION_HXX */