cmake_minimum_required (VERSION 3.20)

project (sm-server)

enable_language(CXX)

set(COMMON_SOURCES
        src/server.cpp
        src/init.cpp
        src/resources.cpp
)

set(COMMON_HEADERS
        inc/node.hpp
        inc/com.hpp
        inc/resources.hpp
        inc/server.hpp
        inc/timer.hpp
        inc/init.hpp
        ../common/common.hpp
        ../common/modbus.hpp
)

set(LIBRARY_NAME sm-server)

add_library (${LIBRARY_NAME} STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_include_directories(${LIBRARY_NAME} PRIVATE
        inc
        ../common
        )

target_compile_options(${LIBRARY_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)