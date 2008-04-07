project(glide64_video)

set(GLIDE64_VIDEO_SRCS
    3dmath.cpp
    Combine.cpp
    Config.cpp
    CRC.cpp
    Debugger.cpp
    DepthBufferRender.cpp
    Ini.cpp
    Main.cpp
    messagebox.c
    rdp.cpp
    support.c
    TexBuffer.cpp
    TexCache.cpp
    Util.cpp
    wrapper/2xsai.cpp
    wrapper/benchmark.cpp
    wrapper/combiner.cpp
    wrapper/config.cpp
    wrapper/filter.cpp
    wrapper/geometry.cpp
    wrapper/hq2x.cpp
    wrapper/hq4x.cpp
    wrapper/main.cpp
    wrapper/textures.cpp
)

add_executable(compiletex compiletex.c)

add_custom_target(cursor.h
    ./compiletex ${CMAKE_CURRENT_SOURCE_DIR}/cursor.tex cursor.h cursor
    DEPENDS compiletex
)
add_custom_target(font.h
    ./compiletex ${CMAKE_CURRENT_SOURCE_DIR}/font.tex font.h font
    DEPENDS compiletex
)
add_custom_target(Tmem_nasm.o
    ${YASM} -f elf -DBITS=${BITS} -m ${MARCH} -i Tmem_nasm.o ${CMAKE_CURRENT_SOURCE_DIR}/Tmem_nasm.asm
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${OPENGL_INCLUDE_DIR} ${SDL_INCLUDE_DIR} ${GTK_INCLUDE_DIR} wrapper)

add_definitions(-DUSE_GTK)
if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-DGCC)
endif(CMAKE_COMPILER_IS_GNUCC)

add_library(glide64_video MODULE ${GLIDE64_VIDEO_SRCS})
set_target_properties(glide64_video PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(glide64_video ${OPENGL_LIBRARIES} ${SDL_LIBRARIES} ${GTK_LIBRARIES})
install(TARGETS glide64_video DESTINATION ${PLUGIN_INSTALL_DIR})

add_dependencies(glide64_video cursor.h font.h Tmem_nasm.o)
