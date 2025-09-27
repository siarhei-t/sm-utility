cmake_minimum_required (VERSION 3.20)

project (sm-client)

enable_language(CXX)

set(COMMON_SOURCES
        src/client.cpp
        src/message.cpp
        src/error.cpp
        src/file.cpp
)

set(COMMON_HEADERS
        inc/client.hpp
        inc/message.hpp
        inc/error.hpp
        inc/file.hpp
        ../common/common.hpp
        ../common/modbus.hpp
)

add_subdirectory(../external/simple-serial-port serial-port)

set(LIBRARY_NAME sm-client)

add_library (${LIBRARY_NAME} STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_link_directories(${LIBRARY_NAME} PUBLIC ../external/simple-serial-port)
target_link_libraries (${LIBRARY_NAME} simple-serial-port)

target_include_directories(${LIBRARY_NAME} PRIVATE
        inc
        ../external/simple-serial-port/inc
        )

target_compile_options(${LIBRARY_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)