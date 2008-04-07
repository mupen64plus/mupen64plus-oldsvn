project(blight_input)

set(BLIGHT_INPUT_SRCS plugin.c pad.c configdialog_sdl.c arial.ttf.c)

add_definitions(-DGUI_SDL)
include_directories(${SDL_INCLUDE_DIR} ${SDLTTF_INCLUDE_DIR})

add_library(blight_input MODULE ${BLIGHT_INPUT_SRCS})
set_target_properties(blight_input PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(blight_input ${SDL_LIBRARY} ${SDLTTF_LIBRARY} freetype)
install(TARGETS blight_input DESTINATION ${PLUGIN_INSTALL_DIR})
