cmake_minimum_required (VERSION 3.20)

project (sm_server_desktop)

set (DIR_SRCS
        main.cpp
        platform.cpp
    )

set(PATH_TO_LIBRARY ../../../core/server)
set(PATH_TO_SERIAL ../../../core/external/simple-serial-port)

set(LOG_LEVEL "1")
set(LOG_OUTPUT_FN "std::printf")

add_executable (${PROJECT_NAME} ${DIR_SRCS})

configure_file(
    ${PATH_TO_LIBRARY}/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/sm-server/config.hpp
)

add_subdirectory(${PATH_TO_SERIAL} serial-port)
add_subdirectory(${PATH_TO_LIBRARY} sm-server)

target_include_directories(sm-server PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/sm-server")


target_include_directories(${PROJECT_NAME} PRIVATE
        ${PATH_TO_LIBRARY}/inc
        ${PATH_TO_LIBRARY}/../common
        ${PATH_TO_SERIAL}/inc
        )

target_link_directories(${PROJECT_NAME} PUBLIC ${PATH_TO_SERIAL})
target_link_directories(${PROJECT_NAME} PUBLIC ${PATH_TO_LIBRARY})

target_link_libraries (${PROJECT_NAME} simple-serial-port)
target_link_libraries (${PROJECT_NAME} sm-server)

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
