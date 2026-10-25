# ----------------------------------------------------------------------------------------
#
#  Copyright (C) 2020 - 2026  Helmut Fieres
# ----------------------------------------------------------------------------------------
project( Twin64-Disc C CXX ASM )

add_library( ${PROJECT_NAME} STATIC  

    T64-Disc.h
    T64-Disc.cpp
) 

target_link_libraries( ${PROJECT_NAME} PUBLIC 

    Twin64-Common 
    Twin64-System 
)
    
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )