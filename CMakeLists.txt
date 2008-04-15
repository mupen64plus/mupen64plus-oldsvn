project(mupen64plus)

# these aren't really all mandatory
find_package(PNG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)

find_program(YASM_EXECUTABLE yasm)
find_program(NASM_EXECUTABLE nasm)

if(YASM_EXECUTABLE )
    set(ASM ${YASM_EXECUTABLE})
elseif(NASM_EXECUTABLE)
    set(ASM ${NASM_EXECUTABLE})
endif(YASM_EXECUTABLE )

if(NOT ASM)
    message(FATAL_ERROR "Neither nasm nor yasm found. Install one of them.")
else(NOT ASM)
    message(STATUS "Found assembler ${ASM}")
endif(NOT ASM)

find_package(PkgConfig REQUIRED)
pkg_search_module(GTK2 REQUIRED gtk+-2.0)
pkg_search_module(GTHREAD2 REQUIRED gthread-2.0)
include_directories(${GTK2_INCLUDE_DIRS})

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/mupen64plus)
set(PLUGIN_INSTALL_DIR ${INSTALL_DIR}/plugins)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
    set(BITS 64)
    set(MARCH amd64)
else(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
    set(BITS 32)
    set(MARCH x86)
endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)

add_subdirectory(mupen64_audio)
add_subdirectory(jttl_audio)
add_subdirectory(rice_video)
add_subdirectory(dummy_video)
add_subdirectory(dummy_audio)
add_subdirectory(rsp_hle)
add_subdirectory(mupen64_input)
add_subdirectory(blight_input)

if(KDE4GUI)
    find_package(Qt4 REQUIRED)
    find_package(KDE4 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})
    add_definitions(${KDE4_DEFINITIONS} ${QT_DEFINITIONS})
    # KDE is a lot stricter about C standards compliance than mupen so reset
    # the cflags
    set(CMAKE_C_FLAGS "")
    add_definitions(-DKDE4)
else(KDE4GUI)
    set(GTK2GUI ON)
    add_definitions(-DUSE_GTK)
endif(KDE4GUI)

add_subdirectory(glN64)

set(CORE_SRCS
    main/main.c
    main/util.c
    main/translate.c
    main/guifuncs.c
    main/config.c
    main/adler32.c
    main/ioapi.c
    main/md5.c
    main/mupenIniApi.c
    main/plugin.c
    main/rom.c
    main/savestates.c
    main/unzip.c
    main/volume.c
    main/cheat.c
    memory/dma.c
    memory/flashram.c
    memory/memory.c
    memory/pif.c
    memory/tlb.c
    r4300/r4300.c
    r4300/bc.c
    r4300/compare_core.c
    r4300/cop0.c
    r4300/cop1.c
    r4300/cop1_d.c
    r4300/cop1_l.c
    r4300/cop1_s.c
    r4300/cop1_w.c
    r4300/exception.c
    r4300/interupt.c
    r4300/profile.c
    r4300/pure_interp.c
    r4300/recomp.c
    r4300/special.c
    r4300/regimm.c
    r4300/tlb.c
)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
    set(DYNAREC x86_64/)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL i686)
    set(DYNAREC x86/)
endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)

if(DYNAREC)
    message("Dynamic recompiler enabled")
    set(DYNAREC_SRCS
        r4300/${DYNAREC}/assemble.c
        r4300/${DYNAREC}/debug.c
        r4300/${DYNAREC}/gbc.c
        r4300/${DYNAREC}/gcop0.c
        r4300/${DYNAREC}/gcop1.c
        r4300/${DYNAREC}/gcop1_d.c
        r4300/${DYNAREC}/gcop1_l.c
        r4300/${DYNAREC}/gcop1_s.c
        r4300/${DYNAREC}/gcop1_w.c
        r4300/${DYNAREC}/gr4300.c
        r4300/${DYNAREC}/gregimm.c
        r4300/${DYNAREC}/gspecial.c
        r4300/${DYNAREC}/gtlb.c
        r4300/${DYNAREC}/regcache.c
        r4300/${DYNAREC}/rjump.c
    )
else(DYNAREC)
    message("Dynamic recompiler disabled")
    set(DYNAREC_SRCS r4300/empty_dynarec.c)
endif(DYNAREC)

set(VCR_SRCS
    main/vcr.c
    main/vcr_compress.c
    main/vcr_resample.c
    main/gui_gtk/vcrcomp_dialog.c
)

set(LIRC_SRCS main/lirc.c)

set(GTK_GUI_SRCS
    main/gui_gtk/main_gtk.c
    main/gui_gtk/aboutdialog.c
    main/gui_gtk/cheatdialog.c
    main/gui_gtk/configdialog.c
    main/gui_gtk/rombrowser.c
    main/gui_gtk/romproperties.c
)

if(KDE4GUI)
    include_directories(main/gui_kde4)
    set(KDE4_GUI_SRCS
        main/gui_kde4/main.cpp
        main/gui_kde4/mainwindow.cpp
        main/gui_kde4/mainwidget.cpp
        main/gui_kde4/romdirectorieslistwidget.cpp
        main/gui_kde4/rommodel.cpp
        main/gui_kde4/plugins.cpp
    )
    kde4_add_kcfg_files(KDE4_GUI_SRCS main/gui_kde4/settings.kcfgc)
    kde4_add_ui_files(KDE4_GUI_SRCS
        main/gui_kde4/mainsettingswidget.ui
        main/gui_kde4/pluginssettingswidget.ui
        main/gui_kde4/romdirectorieslistwidget.ui
        main/gui_kde4/rombrowsersettingswidget.ui
    )
endif(KDE4GUI)

set(DEBUGGER_SRCS
    debugger/debugger.c
    debugger/breakpoints.c
    debugger/desasm.c
    debugger/decoder.c
    debugger/registers.c
    debugger/regGPR.c
    debugger/regCop0.c
    debugger/regSpecial.c
    debugger/regCop1.c
    debugger/regAI.c
    debugger/regPI.c
    debugger/regRI.c
    debugger/regSI.c
    debugger/regVI.c
    debugger/regTLB.c
    debugger/ui_clist_edit.c
)

add_definitions(-DPREFIX='"${CMAKE_INSTALL_PREFIX}"')
include_directories(${SDL_INCLUDE_DIR})

if(KDE4GUI)
    kde4_add_executable(mupen64plus ${CORE_SRCS} ${DYNAREC_SRCS} ${KDE4_GUI_SRCS})
    target_link_libraries(mupen64plus ${SDL_LIBRARY} ${GTK2_LIBRARIES} ${KDE4_KDEUI_LIBS} kio threadweaver pthread z)
    install(FILES main/gui_kde4/mupen64plusui.rc DESTINATION  ${DATA_INSTALL_DIR}/mupen64plus)
else(KDE4GUI)
    add_executable(mupen64plus ${CORE_SRCS} ${DYNAREC_SRCS} ${GTK_GUI_SRCS})
    target_link_libraries(mupen64plus ${SDL_LIBRARY} ${GTK2_LIBRARIES} pthread z)
endif(KDE4GUI)

install(TARGETS mupen64plus RUNTIME DESTINATION bin)
install(FILES doc/HiRezTexture.txt doc/readme.pdf DESTINATION ${INSTALL_DIR}/doc)
install(FILES doc/mupen64plus.1.gz DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)
install(DIRECTORY icons config lang DESTINATION  ${INSTALL_DIR} PATTERN .* EXCLUDE)
