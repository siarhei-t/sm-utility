cmake_minimum_required (VERSION 3.20)

option(TARGET_WINDOWS "build for Windows" OFF)
option(TARGET_LINUX   "build for Linux"   OFF)

message("*************************************")
message("")
if(TARGET_WINDOWS)
message("target platform selected as Windows.")
set(TARGET_PLATFORM PLATFORM_WINDOWS)
set(PLATFORM_SOURCES
        src/sp_windows.cpp
)
set(PLATFORM_HEADERS
        inc/platform/sp_windows.hpp
)
endif()

if(TARGET_LINUX)
message("target platform selected as Linux.")
set(TARGET_PLATFORM PLATFORM_LINUX)
set(PLATFORM_SOURCES
        src/sp_linux.cpp
)
set(PLATFORM_HEADERS
        inc/platform/sp_linux.hpp
)
endif()

set(COMMON_SOURCES
        src/serial_port.cpp
        src/sp_error.cpp
)

set(COMMON_HEADERS
        inc/sp_types.hpp
        inc/serial_port.hpp
        inc/sp_error.hpp
)

message("")
message("*************************************")

if(NOT DEFINED TARGET_PLATFORM)
message( SEND_ERROR "target platform is not defined.")
endif()

project (simple-serial-port)
add_library (${PROJECT_NAME} INTERFACE)

enable_language(CXX)

target_sources(${PROJECT_NAME} INTERFACE
        ${COMMON_SOURCES}
        ${PLATFORM_SOURCES}
        ${COMMON_HEADERS}
        ${PLATFORM_HEADERS}
        )

target_compile_definitions(${PROJECT_NAME} INTERFACE ${TARGET_PLATFORM}=1)

target_include_directories(${PROJECT_NAME} INTERFACE
        inc
        inc/platform
        )
