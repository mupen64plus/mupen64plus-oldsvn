project(glN64)

set(GLN64_VIDEO_SRCS
    2xSAI.cpp
    Combiner.cpp
    Config_linux.cpp
    CRC.cpp
    DepthBuffer.cpp
    F3D.cpp
    F3DDKR.cpp
    F3DEX2.cpp
    F3DEX.cpp
    F3DPD.cpp
    F3DWRUS.cpp
    FrameBuffer.cpp
    GBI.cpp
    gDP.cpp
    glN64.cpp
    gSP.cpp
    L3D.cpp
    L3DEX2.cpp
    L3DEX.cpp
    N64.cpp
    NV_register_combiners.cpp
    OpenGL.cpp
    RDP.cpp
    RSP.cpp
    S2DEX2.cpp
    S2DEX.cpp
    texture_env_combine.cpp
    texture_env.cpp
    Textures.cpp
    VI.cpp
)

set_source_files_properties(glN64.cpp PROPERTIES COMPILE_FLAGS -DMAINDEF)

# TODO: enable ASM as needed

if(UNIX)
    add_definitions(-D__LINUX__)
endif(UNIX)

include_directories(
    ${SDL_INCLUDE_DIR}
    ${GTK2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GTHREAD2_INCLUDE_DIRS}
)

add_library(gln64_video MODULE ${GLN64_VIDEO_SRCS})
set_target_properties(gln64_video PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(gln64_video
    ${SDL_LIBRARY}
    ${GTK2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GTHREAD2_LIBRARIES}
)
install(TARGETS gln64_video DESTINATION ${PLUGIN_INSTALL_DIR})
