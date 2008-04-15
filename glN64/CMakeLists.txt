project(glN64)

set(GLN64_VIDEO_SRCS
    2xSAI.cpp
    Combiner.cpp
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

if(GTK2_UI)
    set(GLN64_VIDEO_SRCS ${GLN64_VIDEO_SRCS} Config_gtk2.cpp)
    include_directories(${GTHREAD2_INCLUDE_DIRS})
    set(GLN64_VIDEO_LIBS ${GTHREAD2_LIBRARIES} ${GTK2_LIBRARIES} ${PNG_LIBRARY})
elseif(KDE4_UI)
    set(GLN64_VIDEO_SRCS ${GLN64_VIDEO_SRCS} Config_kde4.cpp)
    set(GLN64_VIDEO_LIBS ${KDE4_KDEUI_LIBS} ${PNG_LIBRARY})
    kde4_add_kcfg_files(GLN64_VIDEO_SRCS gln64settings.kcfgc)
    kde4_add_ui_files(GLN64_VIDEO_SRCS gln64config.ui)
endif(GTK2_UI)

set_source_files_properties(glN64.cpp PROPERTIES COMPILE_FLAGS -DMAINDEF)

# TODO: enable ASM as needed

if(UNIX)
    add_definitions(-D__LINUX__)
endif(UNIX)

include_directories(
    ${SDL_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
)

add_library(gln64_video MODULE ${GLN64_VIDEO_SRCS})
set_target_properties(gln64_video PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(gln64_video
    ${SDL_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${GLN64_VIDEO_LIBS}
)
install(TARGETS gln64_video DESTINATION ${PLUGIN_INSTALL_DIR})
