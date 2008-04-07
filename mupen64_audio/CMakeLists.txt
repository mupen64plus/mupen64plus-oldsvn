project(mupen64_audio)

include_directories(${GTK_INCLUDE_DIR})

set(MUPEN64_AUDIO_SRCS main.c)

add_library(mupen64_audio MODULE ${MUPEN64_AUDIO_SRCS})
set_target_properties(mupen64_audio PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(mupen64_audio ${GTK_LIBRARIES} pthread)
install(TARGETS mupen64_audio DESTINATION ${PLUGIN_INSTALL_DIR})
