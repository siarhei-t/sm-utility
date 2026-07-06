cmake_minimum_required (VERSION 3.20)

project (sm_utility)

set (DIR_SRCS
        main.cpp
    )

set(PATH_TO_LIBRARY ../../../core/client)
set(LOG_LEVEL "1")
set(LOG_OUTPUT_FN "std::printf")

add_executable (${PROJECT_NAME} ${DIR_SRCS})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

configure_file(
    ${PATH_TO_LIBRARY}/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/sm-client/config.hpp
)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/sm-client")

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
