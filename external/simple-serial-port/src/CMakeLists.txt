cmake_minimum_required (VERSION 3.15)

project (simple-serial-port)

enable_language(CXX)

option(TARGET_WINDOWS "build for Windows" OFF)
option(TARGET_LINUX   "build for Lunux"   OFF)

message("*************************************")
message("")
if(TARGET_WINDOWS)
message("target platform selected as Windows.")
set(TARGET_PLATFORM PLATFORM_WINDOWS)
set(PLATFORM_SOURCES
        sp_windows.cpp
)
set(PLATFORM_HEADERS
        ../inc/platform/sp_windows.hpp
)
endif()

if(TARGET_LINUX)
message("target platform selected as Linux.")
set(TARGET_PLATFORM PLATFORM_LINUX)
set(PLATFORM_SOURCES
        sp_linux.cpp
)
set(PLATFORM_HEADERS
        ../inc/platform/sp_linux.hpp
)
endif()

set(COMMON_SOURCES
        serial_port.cpp
        sp_error.cpp
)

set(COMMON_HEADERS
        ../inc/sp_types.hpp
        ../inc/serial_port.hpp
        ../inc/sp_error.hpp
)

message("")
message("*************************************")

if(NOT DEFINED TARGET_PLATFORM)
message( SEND_ERROR "target platform is nor defined.")
endif()

add_library (${PROJECT_NAME} STATIC ${COMMON_SOURCES} ${PLATFORM_SOURCES} ${COMMON_HEADERS} ${PLATFORM_HEADERS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${TARGET_PLATFORM}=1)
target_include_directories(${PROJECT_NAME} PRIVATE
        ../inc
        ../inc/platform
        )

