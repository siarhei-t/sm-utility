cmake_minimum_required (VERSION 3.20)

project (sm-client)

enable_language(CXX)

set(COMMON_SOURCES
        src/sm_client.cpp
        src/sm_message.cpp
        src/sm_error.cpp
        src/sm_file.cpp
)

set(COMMON_HEADERS
        inc/sm_client.hpp
        inc/sm_message.hpp
        inc/sm_error.hpp
        inc/sm_file.hpp
        ../common/sm_common.hpp
        ../common/sm_modbus.hpp
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