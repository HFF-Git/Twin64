# ----------------------------------------------------------------------------------------
#
#  Copyright (C) 2020 - 2026  Helmut Fieres
# ----------------------------------------------------------------------------------------
project( Twin64-Tlb C CXX ASM )

add_library( ${PROJECT_NAME} STATIC  

    T64-Tlb.h 
    T64-Tlb.cpp
) 

target_link_libraries( ${PROJECT_NAME} PUBLIC 

    Twin64-Processor 
    Twin64-Memory 
    Twin64-Common 
)

target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )