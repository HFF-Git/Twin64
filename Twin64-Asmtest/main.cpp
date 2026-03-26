//----------------------------------------------------------------------------------------
//
// Twin-64 - Inline Assembler/Disassembler Test Program.
//
//----------------------------------------------------------------------------------------
// AsmTest is a simple program for testing the inline assembler and disassembler. 
// It reads commands from standard input and executes them. The commands are:
//
//  A <argStr> -> assemble input argument
//  D <val>    -> disassemble instruction value
//  T <argStr> -> assemble input, show and pass to disassemble
//  E          -> exit   
//  ?          -> show help
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - One Line Assembler
// Copyright (C) 2020 - 2026 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details. You should have received a copy of the GNU General Public
// License along with this program. If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-Common.h"
#include "T64-InlineAsm.h"

T64Assemble     *doAsm  = nullptr;
T64DisAssemble  *disAsm = nullptr;

//----------------------------------------------------------------------------------------
// Program input parameters.
//
//----------------------------------------------------------------------------------------
void parseParameters( int argc, const char * argv[] ) {
    
}

//----------------------------------------------------------------------------------------
// Command handlers.
//
//----------------------------------------------------------------------------------------
void assemble( char *asmStr ) {
    
    static uint32_t instr;
    int errCode = doAsm -> assembleInstr( asmStr, &instr );

    if ( errCode == 0 ) printf( "0x%08x\n", instr );
    else                printf( "%s\n", doAsm -> getErrStr( doAsm -> getErrId( )));
}

void disassemble( uint32_t instr ) {
    
    char buf[ 128 ];
    disAsm -> formatInstr( buf, sizeof( buf ), instr, 16 );
    printf( "\"%s\"\n", buf );
}

void testAsmDisAsm( char *asmStr ) {

    static uint32_t instr;
    char            buf[ 128 ];
    
    int errCode = doAsm -> assembleInstr( asmStr, &instr );
    if ( errCode == 0 ) {
        
        printf( "0x%08x -> ", instr );

        disAsm -> formatInstr( buf, sizeof( buf ), instr, 16 );
        printf( "\"%s\"\n", buf );
    }
    else printf( "%s\n", doAsm -> getErrStr( doAsm -> getErrId( )));
}

void printHelp( ) {
    
    printf( "A <argStr> -> assemble input argument\n" );
    printf( "D <val>    -> disassemble instruction value\n" );
    printf( "T <argStr> -> assemble input, show and pass to disassemble\n" );
    printf( "E          -> exit\n" );
}

//----------------------------------------------------------------------------------------
// Read the command input, strip newline char and convert to uppercase.
//
//----------------------------------------------------------------------------------------
int getInput( char *buf ) {
    
    if ( fgets( buf, 128, stdin ) == nullptr ) return( -1 );
    buf[ strcspn( buf, "\n") ] = '\0';

    for ( char *s = buf; *s; s++ ) *s = toupper((unsigned char) *s );
    return((uint32_t) strlen( buf ));
}

//----------------------------------------------------------------------------------------
// Here we go.
//
//----------------------------------------------------------------------------------------
int main( int argc, const char * argv[] ) {
    
    parseParameters( argc, argv );

    doAsm  = new T64Assemble( );
    disAsm = new T64DisAssemble( );

    while ( true ) {
        
        printf( "-> ");
        
        char input[ 128 ];
        
        if ( getInput( input ) < 0 ) {
            
            printf("Error reading input\n");
            break;
        }
       
        char *cmd = strtok( input, " \t");
        char *arg = strtok( NULL, "" );
        if ( cmd == nullptr ) continue;
             
        if ( strcmp( cmd, "A" ) == 0 ) {
            
            if ( arg != nullptr ) assemble( arg );
            else printf( "Expected assembler input string\n" );
        }
        else if ( strcmp( cmd, "D" ) == 0 ) {
            
            uint32_t val;
            if (( arg != nullptr ) && ( sscanf( arg, "%i", &val ) == 1 )) {
                
                disassemble( val );
            }
            else printf( "Invalid number for disassembler\n" );
        }
        else if ( strcmp( cmd, "T" ) == 0 ) {
        
            if ( arg != nullptr ) testAsmDisAsm( arg );
            else printf( "Expected assembler input string\n" );
        }  
        else if ( strcmp( cmd, "E" ) == 0 ) {
        
            break;
        }
        else if ( strcmp( cmd, "?" ) == 0 ) {
            
            printHelp( );
        }
        else printf("Unknown command.\n");
    }

    return 0;
}
