cmake_minimum_required (VERSION 3.20)

project (sm_target_demo)

set (DIR_SRCS
        main.cpp
    )

add_executable (${PROJECT_NAME} ${DIR_SRCS})

add_subdirectory(../../core/client sm-client)

get_directory_property(TARGET_PLATFORM DIRECTORY ../../core/client DEFINITION TARGET_PLATFORM)
get_directory_property(LIBRARY_NAME DIRECTORY ../../core/client DEFINITION LIBRARY_NAME)

target_link_directories(${PROJECT_NAME} PUBLIC ../../core/client)

target_link_libraries (${PROJECT_NAME} ${LIBRARY_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${TARGET_PLATFORM}=1)

target_include_directories(${PROJECT_NAME} PRIVATE
        ../../core/client/inc
        ../../core/common
        ../../core/external/simple-serial-port-1.03/lib/inc/
        ../../core/external/simple-serial-port-1.03/lib/inc/platform
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
