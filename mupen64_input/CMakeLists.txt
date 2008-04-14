project(mupen64_input)

set(MUPEN64_INPUT_SRCS main.c)

add_definitions(-DUSE_GTK)
include_directories(${SDL_INCLUDE_DIR} ${GTK2_INCLUDE_DIRS})
set(MUPEN64_INPUT_LIBRARIES ${SDL_LIBRARY} ${GTK2_LIBRARIES})

add_library(mupen64_input MODULE ${MUPEN64_INPUT_SRCS})
set_target_properties(mupen64_input PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(mupen64_input ${MUPEN64_INPUT_LIBRARIES})
install(TARGETS mupen64_input DESTINATION ${PLUGIN_INSTALL_DIR})