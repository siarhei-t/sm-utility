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
)

add_library (${PROJECT_NAME} STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

add_subdirectory(../external/simple-serial-port-1.03/lib serial-port)
get_directory_property(TARGET_PLATFORM DIRECTORY ../external/simple-serial-port-1.03/lib DEFINITION TARGET_PLATFORM)

target_link_directories(${PROJECT_NAME} PUBLIC ../external/simple-serial-port-1.03/lib)
target_link_libraries (${PROJECT_NAME} simple-serial-port)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${TARGET_PLATFORM}=1)

target_include_directories(${PROJECT_NAME} PRIVATE
        inc
        ../external/simple-serial-port-1.03/lib/inc
        ../external/simple-serial-port-1.03/lib/inc/platform
        )

target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)