//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Simulator Main 
//
//----------------------------------------------------------------------------------------
// 
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Simulator Main
// Copyright (C) 2020 - 2026 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under the 
// terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should
//  have received a copy of the GNU General Public License along with this program.  
// If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-SimDeclarations.h"

//----------------------------------------------------------------------------------------
// Here we go. 
//
// ??? this should be done from a configuration file. We just hard code it for 
// now.
//
//----------------------------------------------------------------------------------------
int main( int argc, char * argv[] ) {

    SimGlobals *glb     = new SimGlobals( );

    processCmdLineOptions( glb, argc, argv );
   
    glb -> console      = new SimConsoleIO( );
    glb -> env          = new SimEnv( glb, 100 );
    glb -> winDisplay   = new SimWinDisplay( glb );
    glb -> system       = new T64System( );  
    
    glb -> console      -> initConsoleIO( );
    glb -> env          -> setupPredefined( );
    glb -> winDisplay   -> setupWinDisplay( );

    T64Processor *proc = 
        new T64Processor(   glb -> system,
                            3,
                            T64_PO_NIL,
                            T64_CPU_T_NIL,
                            T64_TT_FA_64S,
                            T64_CT_NIL,
                            0,
                            0 );

    T64Memory *pdc = 
        new T64Memory(  glb -> system,
                        0, 
                        T64_MK_NIL,
                        T64_MT_ROM,
                        T64_PDC_MEM_START,
                        4 * T64_PAGE_SIZE_BYTES );

    T64Memory *mem1 = 
        new T64Memory(  glb -> system,
                        1, 
                        T64_MK_NIL,
                        T64_MT_RAM,
                        0,
                        64 * T64_PAGE_SIZE_BYTES );

    T64Memory *mem2 = 
        new T64Memory(  glb -> system,
                        2,
                        T64_MK_NIL,
                        T64_MT_RAM, 
                        64 * T64_PAGE_SIZE_BYTES,
                        64 * T64_PAGE_SIZE_BYTES );

    if ( glb -> system -> addToModuleMap( pdc ) != 0 ) {

        glb -> console -> writeChars( "Config Error: Module PDC\n" );
        return( -1 );
    }

    pdc -> setSpaReadOnly( true );
    
    if ( glb -> system -> addToModuleMap( mem1 ) != 0 ) {

        glb -> console -> writeChars( "Config Error: Module MEM 1\n" );
        return( -1 );
    }
    
    if ( glb -> system -> addToModuleMap( mem2 ) != 0 ) {

        glb -> console -> writeChars( "Config Error: Module MEM 2\n" );
        return( -1 );
    }

    if ( glb -> system -> addToModuleMap( proc ) != 0 ) {

        glb -> console -> writeChars( "Config Error: Module PROC\n" );
        return( -1 );
    }

    glb -> winDisplay   -> startWinDisplay( );
    
    return 0;
}
