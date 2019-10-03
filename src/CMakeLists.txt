
set (CMAKE_CXX_STANDARD 14)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

set (SOURCES
  SdlApplication.cc
  AppDecorator.cc
  MainWindowLayout.cc
  VirtualLayoutItem.cc
  )

add_library (sdl_app_core SHARED
  ${SOURCES}
  )

set(SDL_APP_CORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

target_link_libraries(sdl_app_core
  core_utils
  sdl_engine
  sdl_core
  sdl_graphic
  )

target_include_directories (sdl_app_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )
