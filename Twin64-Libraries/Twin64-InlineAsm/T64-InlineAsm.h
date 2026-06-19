//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - InLine Assembler
//
//----------------------------------------------------------------------------------------
// The one line assembler assembles an instruction without further context. It 
// is intended to for use in the simulator. There is no symbol table or any 
// concept of assembling multiple instructions. The instruction to generate test
// is completely self sufficient. The parser is a straightforward recursive 
// descendant parser, LL1 grammar. It uses the C++ try / catch to escape when 
// an error is detected. Considering that we only have one line to parse, there
// is no need to implement a better parser error recovery method.
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - InLine Assembler
// Copyright (C) 2020 - 2026 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. 
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#ifndef T64_InlineAsm_h
#define T64_InlineAsm_h

#include "T64-Common.h"
#include "T64-Util.h"

//----------------------------------------------------------------------------------------
// "T64Assemble" is a one line assembler. It just parses the instruction string
// and produces an instruction. Utility routines for converting an error code to
// an error message and an index into the input source line to where the error 
// occurred is provided too.
//
//----------------------------------------------------------------------------------------
struct T64Assemble {
    
public:
    
    T64Assemble( );
    
    int         assembleInstr( char *inputStr, uint32_t *instr );

    int         getErrId( );
    int         getErrPos( );
    const char  *getErrStr( int errId );
};

//----------------------------------------------------------------------------------------
// "T64DisAssemble" will disassemble an instruction and return a human readable
// form. The disassembled string can also contains  two parts, which are the 
// opcode part and the operand part. There are options to just one of the parts
// or both. The split allows for displaying the disassembled instruction in an 
// aligned fashion, when printing several lines.
//
//----------------------------------------------------------------------------------------
struct T64DisAssemble {
    
public:
    
    T64DisAssemble( );
    
    int formatInstr( char *buf, int bufLen, uint32_t instr, int rdx );
    int formatOpCode( char *buf, int bufLen, uint32_t instr );
    int formatOperands( char *buf, int bufLen, uint32_t instr, int rdx );
    int getOpCodeFieldWidth( );
    int getOperandsFieldWidth( );
};

#endif // T64_InlineAsm_h
