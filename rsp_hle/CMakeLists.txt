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

if (GTK2GUI)
    include_directories(${GTK2_INCLUDE_DIRS})
    set(RSP_HLE_LIBRARIES ${GTK2_LIBRARIES})
endif(GTK2GUI)

add_library(mupen64_hle_rsp_azimer MODULE ${RSP_HLE_SRCS})
set_target_properties(mupen64_hle_rsp_azimer PROPERTIES PREFIX "") # make sure we get no lib prefix
target_link_libraries(mupen64_hle_rsp_azimer ${RSP_HLE_LIBRARIES})
install(TARGETS mupen64_hle_rsp_azimer DESTINATION ${PLUGIN_INSTALL_DIR})
