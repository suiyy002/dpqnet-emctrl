# define the executable target
set (EXE_TARGET "dpqnet_gui")
#message("EXE_TARGET: ${EXE_TARGET}")

message("_ARCH_TYPE: ${_ARCH_TYPE}")

add_sources(main.cpp p_keybd.cpp p_socket.cpp p_timer.cpp)
include(../../common/commu/CMakeLists_c.txt)
include(../../common/misc/CMakeLists_c.txt)
include(../../common/thread/CMakeLists.txt)
include_directories (data device device/${_ARCH_TYPE} EEW GUI IPC security)
aux_source_directory(data DATA_SRC)
aux_source_directory(device DEVICE_SRC)
aux_source_directory(EEW EEW_SRC)
aux_source_directory(IPC IPC_SRC)
aux_source_directory(security SECURITY_SRC)
#aux_source_directory(thread THREAD_SRC)
aux_source_directory(GUI GUI_SRC)
aux_source_directory(GUI/component GUI_COMP_SRC)
aux_source_directory(GUI/display GUI_DIS_SRC)
#aux_source_directory(GUI/display/fb_vga GUI_DISFB_SRC)
aux_source_directory(GUI/form GUI_FORM_SRC)
aux_source_directory(GUI/form/${VIEW_TYPE} GUI_FORM_VW_SRC)

set ( SRC_LIST ${SRC_LIST} ${DATA_SRC} ${DEVICE_SRC} ${EEW_SRC} ${IPC_SRC} 
	  ${SECURITY_SRC} ${GUI_SRC} ${GUI_COMP_SRC} ${GUI_DIS_SRC}
	  ${GUI_FORM_SRC} ${GUI_FORM_VW_SRC} )
message("SRC_LIST=${SRC_LIST}")

#set (EXTRA_LIBS ${EXTRA_LIBS} pthread dl)
set (EXTRA_LIBS ${EXTRA_LIBS} m pthread z)
# add the executable
add_executable (${EXE_TARGET} ${SRC_LIST})
target_link_libraries (${EXE_TARGET} ${EXTRA_LIBS})

# Set the GCC Additional flags to use when compiling this target's sources
# -O0 -O1 -O2 -O3 ...
set_property(TARGET ${EXE_TARGET} PROPERTY COMPILE_FLAGS -O2)

# Set the GCC Additional flags to use when linking this target
# -shared-libgcc -static-libstdc++ -static-libgcc ...
set_property(TARGET ${EXE_TARGET} PROPERTY LINK_FLAGS -static-libgcc)

# specifies the directory into which runtime target files should be built
#set_property( TARGET ${EXE_TARGET} PROPERTY RUNTIME_OUTPUT_DIRECTORY
#              ${PROJECT_BINARY_DIR}/bin )

# add the install targets
install (TARGETS ${EXE_TARGET} DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h" DESTINATION include)
