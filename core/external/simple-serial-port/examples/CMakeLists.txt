cmake_minimum_required (VERSION 3.20)

set(CMAKE_PROJECT_NAME 32f0308discovery-demo)

project (read-write-example)
add_executable (${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        read_write.cpp
)

add_subdirectory(../ serial-port)
get_directory_property(TARGET_PLATFORM DIRECTORY ../ DEFINITION TARGET_PLATFORM)

target_link_libraries (${PROJECT_NAME} simple-serial-port)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${TARGET_PLATFORM}=1)

#common include path
target_include_directories(${PROJECT_NAME} PRIVATE
        ../inc
        )
