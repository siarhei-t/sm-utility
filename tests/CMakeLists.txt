cmake_minimum_required (VERSION 3.15)

project (test)

set(EXECUTABLE ${PROJECT_NAME})

set (DIR_SRCS
        ../src/sm_modbus.cpp
        ../src/sm_client.cpp
        test.cpp
    )
add_executable (${EXECUTABLE} ${DIR_SRCS})
target_include_directories(${EXECUTABLE} PRIVATE
        ../inc    
        )
