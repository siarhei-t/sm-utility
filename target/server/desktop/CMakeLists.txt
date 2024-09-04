cmake_minimum_required (VERSION 3.20)

project (sm_server_desktop)

set (DIR_SRCS
        main.cpp
        platform.cpp
    )

add_executable (${PROJECT_NAME} ${DIR_SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE
        ../../../core/server/inc
        ../../../core/common
        ../../../core/external/simple-serial-port-1.03/lib/inc/
        ../../../core/external/simple-serial-port-1.03/lib/inc/platform
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
