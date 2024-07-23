cmake_minimum_required (VERSION 3.20)

project (sm_target_demo)

set(EXECUTABLE ${PROJECT_NAME})

set (DIR_SRCS
        main.cpp
    )
add_executable (${EXECUTABLE} ${DIR_SRCS})

add_subdirectory(lib sm-target)

target_link_directories(${EXECUTABLE} PUBLIC lib)
target_link_libraries (${EXECUTABLE} sm-target)

target_include_directories(${EXECUTABLE} PRIVATE
        lib/inc
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
