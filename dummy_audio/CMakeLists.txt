project(dummy_audio)

set(DUMMY_AUDIO_SRCS audio.c)

add_library(dummy_audio MODULE ${DUMMY_AUDIO_SRCS})
set_target_properties(dummy_audio PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(dummy_audio)
install(TARGETS dummy_audio DESTINATION ${PLUGIN_INSTALL_DIR})
