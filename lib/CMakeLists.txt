cmake_minimum_required (VERSION 3.15)

project (sm-client)

enable_language(CXX)

set(COMMON_SOURCES
        src/sm_client.cpp
        src/sm_modbus.cpp
)
set(COMMON_HEADERS
        inc/sm_client.hpp
        inc/sm_modbus.hpp
)

add_library (${PROJECT_NAME} STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

add_subdirectory(../external/simple-serial-port/src serial-port)
get_directory_property(TARGET_PLATFORM DIRECTORY ../external/simple-serial-port/src DEFINITION TARGET_PLATFORM)

target_link_directories(${PROJECT_NAME} PUBLIC ../external/simple-serial-port/src)
target_link_libraries (${PROJECT_NAME} simple-serial-port)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${TARGET_PLATFORM}=1)

target_include_directories(${PROJECT_NAME} PRIVATE
        inc
        ../external/simple-serial-port/inc
        ../external/simple-serial-port/inc/platform
        )