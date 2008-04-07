project(dummy_video)

set(DUMMY_VIDEO_SRCS video.cpp)

if(UNIX)
    add_definitions(-D__LINUX__)
endif(UNIX)

add_library(dummy_video MODULE ${DUMMY_VIDEO_SRCS})
set_target_properties(dummy_video PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(dummy_video)
install(TARGETS dummy_video DESTINATION ${PLUGIN_INSTALL_DIR})
