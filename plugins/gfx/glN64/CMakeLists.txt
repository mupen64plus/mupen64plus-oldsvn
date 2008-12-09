set(glN64_sources
    2xSAI.cpp
    CRC.cpp
    Combiner.cpp
    DepthBuffer.cpp
    F3D.cpp
    F3DDKR.cpp
    F3DEX.cpp
    F3DEX2.cpp
    F3DPD.cpp
    F3DWRUS.cpp
    FrameBuffer.cpp
    GBI.cpp
    L3D.cpp
    L3DEX.cpp
    L3DEX2.cpp
    N64.cpp
    NV_register_combiners.cpp
    OpenGL.cpp
    RDP.cpp
    RSP.cpp
    S2DEX.cpp
    S2DEX2.cpp
    Textures.cpp
    VI.cpp
    gDP.cpp
    gSP.cpp
    glN64.cpp
    texture_env.cpp
    texture_env_combine.cpp
)

add_definitions(-DPIC -D__LINUX__ -Wall)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    add_definitions(-fpic)
endif()

if(UI_QT4)
    add_definitions(-DQT4_GUI)
    set(libraries ${QT_LIBRARIES})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    qt4_wrap_ui(ui_sources gln64config.ui)
    set(glN64_sources ${glN64_sources} Config_qt4.cpp ${ui_sources})
elseif(UI_GTK2)
    add_definitions(-DUSE_GTK)
    set(libraries ${GTK2_LIBRARIES} ${GTHREAD2_LIBRARIES})
    set(glN64_sources ${glN64_sources} Config_gtk2.cpp)
else()
    set(glN64_sources ${glN64_sources} Config_nogui.cpp)
endif()

add_library(glN64 SHARED ${glN64_sources})
target_link_libraries(glN64 ${libraries}  ${OPENGL_LIBRARIES} ${SDL_LIBRARY})
set_target_properties(glN64 PROPERTIES PREFIX "")
install(TARGETS glN64
        DESTINATION ${PLUGIN_INSTALL_DIR})
