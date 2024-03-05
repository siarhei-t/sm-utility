cmake_minimum_required (VERSION 3.15)

project (sm-client)

enable_language(CXX)

set(COMMON_SOURCES
        src/sm_client.cpp
        src/sm_modbus.cpp
)
set(COMMON_HEADERS
        inc/sm_client.hpp
        inc/sm_client.hpp
)

add_library (${PROJECT_NAME} STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE
        inc
        )