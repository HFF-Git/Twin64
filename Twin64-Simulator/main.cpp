//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Simulator Main 
//
//----------------------------------------------------------------------------------------
// Welcome to the T64 Simulator. It is a program to simulate a T64 system. 
// First we process the command line options. Next, set up the console IO, the
// environment variable subsystem, the window display and the T64 system which
// is the backbone of all configured modules. As the last step we start the 
// window subsystem, which will present the command line interface. 
//
// Optionally, the command interpreter will try to load a configuration file.
// This is a file of plain command lines, such as creating the modules, setting
// values in memory and so on.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Simulator Main
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
#include "T64-SimDeclarations.h"

//----------------------------------------------------------------------------------------
// Here we go. 
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
    glb -> winDisplay   -> startWinDisplay( );
    
    return ( 0 );
}
