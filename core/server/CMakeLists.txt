cmake_minimum_required (VERSION 3.20)

project (sm-server)

enable_language(CXX)

set(COMMON_SOURCES
        src/sm_server.cpp
        src/sm_init.cpp
        src/sm_resources.cpp
)

set(COMMON_HEADERS
        inc/sm_node.hpp
        inc/sm_com.hpp
        inc/sm_resources.hpp
        inc/sm_server.hpp
        inc/sm_timer.hpp
        inc/sm_init.hpp
        ../common/sm_common.hpp
        ../common/sm_modbus.hpp
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