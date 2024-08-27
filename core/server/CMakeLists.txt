cmake_minimum_required (VERSION 3.20)

project (sm-server)

enable_language(CXX)

set(COMMON_SOURCES
        src/sm_server.cpp

)

set(COMMON_HEADERS
        inc/sm_server.hpp
        inc/sm_modbus.hpp
)

add_library (${PROJECT_NAME} STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
        inc
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
