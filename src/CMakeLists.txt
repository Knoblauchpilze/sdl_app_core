
target_include_directories (sdl_app_core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	)

target_sources (sdl_app_core PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/SdlApplication.cc
	${CMAKE_CURRENT_SOURCE_DIR}/AppDecorator.cc
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindowLayout.cc
	)
