project(mupen64plus)
cmake_minimum_required(VERSION 2.6)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})
set(libraries ${libraries} ${SDL_LIBRARY})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
set(libraries ${libraries} ${PNG_LIBRARIES})
add_definitions(${PNG_DEFINITIONS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
set(libraries ${libraries} ${OPENGL_LIBRARIES})

find_package(PkgConfig REQUIRED)
pkg_search_module(FREETYPE REQUIRED freetype2)
include_directories(${FREETYPE_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS})
set(libraries ${libraries} ${FREETYPE_LIBRARIES})

if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-Wall)
endif(CMAKE_COMPILER_IS_GNUCXX)

set(core_sources
    backend/main/adler32.c
    backend/main/cheat.c
    backend/main/config.c
    backend/main/main.c
    backend/main/md5.c
    backend/main/plugin.c
    backend/main/rom.c
    backend/main/romcache.c
    backend/main/savestates.c
    backend/main/util.c
    backend/main/7zip/7zAlloc.c
    backend/main/7zip/7zBuffer.c
    backend/main/7zip/7zCrc.c
    backend/main/7zip/7zDecode.c
    backend/main/7zip/7zExtract.c
    backend/main/7zip/7zHeader.c
    backend/main/7zip/7zIn.c
    backend/main/7zip/7zItem.c
    backend/main/7zip/7zMain.c
    backend/main/7zip/BranchX86.c
    backend/main/7zip/BranchX86_2.c
    backend/main/7zip/LzmaDecode.c
    backend/main/bzip2/blocksort.c
    backend/main/bzip2/bzlib.c
    backend/main/bzip2/compress.c
    backend/main/bzip2/crctable.c
    backend/main/bzip2/decompress.c
    backend/main/bzip2/huffman.c
    backend/main/bzip2/randtable.c
    backend/main/lzma/buffer.c
    backend/main/lzma/io.c
    backend/main/lzma/main.c
    backend/main/zip/ioapi.c
    backend/main/zip/unzip.c
)

# Qt4 reimplements the translation functions
if(NOT ui STREQUAL qt4)
    set(core_sources
        ${core_sources}
        backend/main/translate.c
    )
endif(NOT ui STREQUAL qt4)

set(lirc_sources
    backend/main/lirc.c
)

set(memory_sources
    backend/memory/dma.c
    backend/memory/flashram.c
    backend/memory/memory.c
    backend/memory/pif.c
    backend/memory/tlb.c
)

set(opengl_sources
    backend/opengl/OGLFT.cpp
    backend/opengl/osd.cpp
    backend/opengl/screenshot.cpp
)

set(r4300_common_sources
    backend/r4300/bc.c
    backend/r4300/compare_core.c
    backend/r4300/cop0.c
    backend/r4300/cop1.c
    backend/r4300/cop1_d.c
    backend/r4300/cop1_l.c
    backend/r4300/cop1_s.c
    backend/r4300/cop1_w.c
    backend/r4300/exception.c
    backend/r4300/interupt.c
    backend/r4300/profile.c
    backend/r4300/pure_interp.c
    backend/r4300/r4300.c
    backend/r4300/recomp.c
    backend/r4300/regimm.c
    backend/r4300/special.c
    backend/r4300/tlb.c
)

set(r4300_empty_dynarec_sources
    backend/r4300/empty_dynarec.c
)

set(r4300_x86_dynarec_sources
    backend/r4300/x86/assemble.c
    backend/r4300/x86/debug.c
    backend/r4300/x86/gbc.c
    backend/r4300/x86/gcop0.c
    backend/r4300/x86/gcop1.c
    backend/r4300/x86/gcop1_d.c
    backend/r4300/x86/gcop1_l.c
    backend/r4300/x86/gcop1_s.c
    backend/r4300/x86/gcop1_w.c
    backend/r4300/x86/gr4300.c
    backend/r4300/x86/gregimm.c
    backend/r4300/x86/gspecial.c
    backend/r4300/x86/gtlb.c
    backend/r4300/x86/regcache.c
    backend/r4300/x86/rjump.c
)

set(r4300_x86_64_dynarec_sources
    backend/r4300/x86_64/assemble.c
    backend/r4300/x86_64/debug.c
    backend/r4300/x86_64/gbc.c
    backend/r4300/x86_64/gcop0.c
    backend/r4300/x86_64/gcop1.c
    backend/r4300/x86_64/gcop1_d.c
    backend/r4300/x86_64/gcop1_l.c
    backend/r4300/x86_64/gcop1_s.c
    backend/r4300/x86_64/gcop1_w.c
    backend/r4300/x86_64/gr4300.c
    backend/r4300/x86_64/gregimm.c
    backend/r4300/x86_64/gspecial.c
    backend/r4300/x86_64/gtlb.c
    backend/r4300/x86_64/regcache.c
    backend/r4300/x86_64/rjump.c
)

set(ui_qt4_sources
    frontends/qt4/globals.cpp
    frontends/qt4/main.cpp
    frontends/qt4/mainwidget.cpp
    frontends/qt4/mainwindow.cpp
    frontends/qt4/romdelegate.cpp
    frontends/qt4/romdirectorieslistwidget.cpp
    frontends/qt4/rominfodialog.cpp
    frontends/qt4/rommodel.cpp
    frontends/qt4/settingsdialog.cpp
    frontends/qt4/starlabel.cpp
    frontends/qt4/translate.cpp
)

set(ui_qt4_ui_files
    frontends/qt4/mainwidget.ui
    frontends/qt4/mainwindow.ui
    frontends/qt4/romdirectorieslistwidget.ui
    frontends/qt4/rominfodialog.ui
    frontends/qt4/settingsdialog.ui
)

set(ui_qt4_qobject_headers
    frontends/qt4/mainwidget.h
    frontends/qt4/mainwindow.h
    frontends/qt4/romdelegate.h
    frontends/qt4/romdirectorieslistwidget.h
    frontends/qt4/rominfodialog.h
    frontends/qt4/rommodel.h
    frontends/qt4/settingsdialog.h
    frontends/qt4/starlabel.h
)

set(ui_gtk2_sources
    frontends/gtk2/aboutdialog.c
    frontends/gtk2/cheatdialog.c
    frontends/gtk2/configdialog.c
    frontends/gtk2/icontheme.c
    frontends/gtk2/main_gtk.c
    frontends/gtk2/rombrowser.c
    frontends/gtk2/romproperties.c
)

# shared source
set(sources
    ${core_sources}
    ${memory_sources}
    ${opengl_sources}
    ${r4300_common_sources}
)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL i686)
    add_definitions(-DNO_GUI)
    set(sources
        ${sources}
        ${r4300_x86_dynarec_sources}
    )
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
    set(sources
        ${sources}
        ${r4300_x86_64_dynarec_sources}
    )
else(CMAKE_SYSTEM_PROCESSOR STREQUAL i686)
    set(sources
        ${sources}
        ${r4300_empty_dynarec_sources}
    )
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL i686)

set(MUPEN_VERSION "unknown")
set(PLUGIN_VERSION "unknown")

if(NOT MUPEN_RELEASE)
    set(MUPEN_RELEASE 1.4)
endif(NOT MUPEN_RELEASE)

if(CMAKE_HOST_UNIX)
    execute_process(
    COMMAND svn info .
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE SVN_INFO
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(SVN_INFO)
        execute_process(
        COMMAND svn info .
        COMMAND sed -n "/^Revision: /s/^Revision: //p"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE SVN_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
        COMMAND svn info .
        COMMAND sed -n "/^URL: /s/.*mupen64plus.//1p"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE SVN_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
        COMMAND svn info .
        COMMAND md5sum
        COMMAND sed "/.*/s/ -//;/^d41d8cd98f00b204e9800998ecf8427e/d"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE SVN_DIFFHASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        set(MUPEN_VERSION ${MUPEN_RELEASE}-${SVN_BRANCH}-${SVN_REVISION} ${SVN_DIFFHASH})
        set(PLUGIN_VERSION ${MUPEN_RELEASE}-${SVN_BRANCH}-${SVN_REVISION})
    endif(SVN_INFO)
endif(CMAKE_HOST_UNIX)

configure_file(version.h backend/main/version.h)

# add source and build dir to include path, things that generate code need this
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

# default to no gui
if(NOT ui)
    set(ui none)
endif(NOT ui)

if(ui STREQUAL none)
    add_definitions(-DNO_GUI)
elseif(ui STREQUAL qt4)
    find_package(Qt4 REQUIRED)
    include(${QT_USE_FILE})
    set(libraries ${libraries} ${QT_LIBRARIES})

    pkg_search_module(GTK2 REQUIRED gtk+-2.0)
    include_directories(${GTK2_INCLUDE_DIRS})
    link_directories(${GTK2_LIBRARY_DIRS})
    set(libraries ${libraries} ${GTK2_LIBRARIES})

    qt4_wrap_ui(ui_sources ${ui_qt4_ui_files})
    qt4_wrap_cpp(moc_sources ${ui_qt4_qobject_headers})

    set(sources
        ${sources}
        ${ui_qt4_sources}
        ${ui_sources}
        ${moc_sources}
    )
elseif(ui STREQUAL gtk2)
    pkg_search_module(GTK2 REQUIRED gtk+-2.0)
    include_directories(${GTK2_INCLUDE_DIRS})
    link_directories(${GTK2_LIBRARY_DIRS})
    set(libraries ${libraries} ${GTK2_LIBRARIES})

    pkg_search_module(GTHREAD2 REQUIRED gthread-2.0)
    include_directories(${GTHREAD2_INCLUDE_DIRS})
    link_directories(${GTHREAD2_LIBRARY_DIRS})
    set(libraries ${libraries} ${GTHREAD2_LIBRARIES})

    set(sources
        ${sources}
        ${ui_gtk2_sources}
    )
endif(ui STREQUAL none)

add_executable(mupen64plus ${sources})
target_link_libraries(mupen64plus ${libraries})

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(link_flags "${link_flags} -Wl,-export-dynamic")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL i686)
        set(link_flags "${link_flags} -z execstack")
    endif(CMAKE_SYSTEM_PROCESSOR STREQUAL i686)
endif(CMAKE_SYSTEM_NAME STREQUAL Linux)

set_target_properties(mupen64plus PROPERTIES LINK_FLAGS "${link_flags}")