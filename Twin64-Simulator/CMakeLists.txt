# ----------------------------------------------------------------------------------------
#  CMAKE File
#  Copyright (C) 2020 - 2026  Helmut Fieres
# ----------------------------------------------------------------------------------------
project( Twin64-Simulator )

add_executable( ${PROJECT_NAME} 

    main.cpp
    T64-SimVersion.h
    T64-SimDeclarations.h 
    T64-SimTables.h
    T64-SimConfig.cpp
    T64-SimTokenizer.cpp
    T64-SimExprEvaluator.cpp
    T64-SimEXprFunctions.cpp
    T64-SimEnvVars.cpp
    T64-SimWinBaseClasses.cpp
    T64-SimWinClasses.cpp
    T64-SimWinElfFileCmd.cpp
    T64-SimWinCmd.cpp
    T64-SimWinDisplay.cpp
)

target_link_libraries( ${PROJECT_NAME} PRIVATE 

    Twin64-Common 
    Twin64-ConsoleIO 
    Twin64-InlineAsm 
    Twin64-System
    Twin64-Processor
    Twin64-Memory
    ELFIO
)