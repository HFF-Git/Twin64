# ----------------------------------------------------------------------------------------
#  CMAKE File
#  Copyright (C) 2020 - 2026  Helmut Fieres
# ----------------------------------------------------------------------------------------
cmake_minimum_required( VERSION 3.29 )

project( Twin64 C CXX ASM )

set( CMAKE_C_STANDARD 11 )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_C_EXTENSIONS ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS)
set( CMAKE_CXX_STANDARD_REQUIRED ON)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )

# Cross-platform compiler warnings

if(MSVC)
    add_compile_options( /W4 /permissive- )
else()
    add_compile_options( 
        -Wall 
        -Wno-unused-parameter 
        -Wno-gnu-anonymous-struct 
        -Wnested-anon-types)
endif()

add_library(ELFIO INTERFACE)
target_include_directories(ELFIO INTERFACE ${CMAKE_SOURCE_DIR}/../ELFIO)

add_subdirectory( Twin64-Asmtest )
add_subdirectory( Twin64-Simulator )
add_subdirectory( Twin64-SPL )

add_subdirectory( Twin64-Libraries/Twin64-Common )
add_subdirectory( Twin64-Libraries/Twin64-System )
add_subdirectory( Twin64-Libraries/Twin64-Processor )
add_subdirectory( Twin64-Libraries/Twin64-Memory )
add_subdirectory( Twin64-Libraries/Twin64-InlineAsm )
add_subdirectory( Twin64-Libraries/Twin64-ConsoleIO )
