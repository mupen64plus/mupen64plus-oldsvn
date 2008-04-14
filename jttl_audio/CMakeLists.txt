project(jttl_audio)

set(JTTL_LIBS ${SDL_LIBRARY})
include_directories(${SDL_INCLUDE_DIR})

add_definitions(-DUSE_GTK)
set(JTTL_LIBS ${JTTL_LIBS} ${GTK_LIBRARIES})
include_directories(${GTK_INCLUDE_DIR})

set(JTTL_AUDIO_SRCS main.c)

add_library(jttl_audio MODULE ${JTTL_AUDIO_SRCS})
set_target_properties(jttl_audio PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(jttl_audio ${JTTL_LIBS})
install(TARGETS jttl_audio DESTINATION ${PLUGIN_INSTALL_DIR})
