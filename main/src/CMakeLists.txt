# define the executable target
set (EXE_TARGET "dpqnet_mn")
#message("EXE_TARGET: ${EXE_TARGET}")

add_sources(main.cpp p_keybd.cpp p_socket.cpp p_timer.cpp)
include(../../common/commu/CMakeLists_c.txt)
include(../../common/misc/CMakeLists_c.txt)

include_directories (comm config IPC misc peripheral pqm_func test thread)
aux_source_directory(comm COMM_SRC)
aux_source_directory(config CONFIG_SRC)
aux_source_directory(IPC IPC_SRC)
aux_source_directory(misc MISC_SRC)
aux_source_directory(peripheral PERIPHERAL_SRC)
aux_source_directory(pqm_func PQMFUNC_SRC)
aux_source_directory(test TEST_SRC)
aux_source_directory(thread THREAD_SRC)

set (SRC_LIST ${SRC_LIST} main.cpp p_save.cpp p_serial.cpp p_socket.cpp)
set (SRC_LIST ${SRC_LIST} ${COMM_SRC} ${CONFIG_SRC} ${IPC_SRC} ${MISC_SRC} 
	 ${PERIPHERAL_SRC} ${PQMFUNC_SRC} ${TEST_SRC} ${THREAD_SRC})
#set (EXTRA_LIBS ${EXTRA_LIBS} pthread dl)
set (EXTRA_LIBS ${EXTRA_LIBS} m pthread)
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
