project(rsp_hle)

set(RSP_HLE_SRCS
    main.c
    disasm.c
    jpeg.c
    ucode1.cpp
    ucode2.cpp
    ucode3.cpp
    ucode3mp3.cpp
)

include_directories(${GTK2_INCLUDE_DIRS})
add_definitions(-DUSE_GTK)

add_library(mupen64_hle_rsp_azimer MODULE ${RSP_HLE_SRCS})
set_target_properties(mupen64_hle_rsp_azimer PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(mupen64_hle_rsp_azimer ${GTK2_LIBRARIES})
install(TARGETS mupen64_hle_rsp_azimer DESTINATION ${PLUGIN_INSTALL_DIR})
