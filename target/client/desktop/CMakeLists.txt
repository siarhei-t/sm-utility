cmake_minimum_required (VERSION 3.20)

project (sm_target_demo)

set (DIR_SRCS
        main.cpp
    )

add_executable (${PROJECT_NAME} ${DIR_SRCS})

add_subdirectory(../../../core/client sm-client)

target_link_directories(${PROJECT_NAME} PUBLIC ../../../core/client)

target_link_libraries (${PROJECT_NAME} sm-client)

target_include_directories(${PROJECT_NAME} PRIVATE
        ../../../core/client/inc
        ../../../core/common
)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
