cmake_minimum_required (VERSION 3.15)

project (sm_utility)

set(EXECUTABLE ${PROJECT_NAME})

set (DIR_SRCS
        utility.cpp
    )
add_executable (${EXECUTABLE} ${DIR_SRCS})

add_subdirectory(../lib sm-client)
get_directory_property(TARGET_PLATFORM DIRECTORY ../lib DEFINITION TARGET_PLATFORM)

target_link_directories(${EXECUTABLE} PUBLIC ../src)
target_link_libraries (${EXECUTABLE} sm-client)
target_compile_definitions(${EXECUTABLE} PRIVATE ${TARGET_PLATFORM}=1)

target_include_directories(${EXECUTABLE} PRIVATE
        ../lib/inc
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)