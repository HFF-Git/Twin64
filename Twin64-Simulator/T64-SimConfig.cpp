//----------------------------------------------------------------------------------------
//
//  Twin64Sim - A 64-bit CPU Simulator - Configuration 
//
//----------------------------------------------------------------------------------------
// 
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Configuration 
// Copyright (C) 2020 - 2026 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under 
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE.  See the GNU General Public License for more details. You 
// should have received a copy of the GNU General Public License along with this 
// program. If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-SimDeclarations.h"
#include "T64-SimTables.h"

//----------------------------------------------------------------------------------------
// Local data for command line option parsing.
//
//----------------------------------------------------------------------------------------
namespace {

    int  optIndex = 1;  
}

//----------------------------------------------------------------------------------------
// "parseCmdLineOptions" function to parse long command line options. This routine 
// is called repeatedly to parse all command line options. It returns the value field
// of the matched option or -1 if no more options are found. The optArg variable is set
// to the argument string if an argument is expected.   
//
//----------------------------------------------------------------------------------------
int parseCmdLineOptions( int    argc, 
                         char   *argv[ ],
                         const  SimCmdLineOptions *optionTable,
                         char   **optArg ) {

    if ( optIndex >= argc ) return ( -1 );

    const char *arg = argv[ optIndex ];

    if ( strncmp(arg, "--", 2) != 0 ) {

        optIndex++;
        return 0;
    }

    const char *name    = arg + 2;
    const char *eq      = strchr(name, '=');
    size_t nameLen      = (( eq ) ? ((size_t)( eq - name )) : ( strlen( name )));

    for ( int i = 0; ( optionTable && optionTable[i].name ); i++ ) {

        if (( strncmp( name, optionTable[i].name, nameLen ) == 0 ) &&
             ( strlen(optionTable[i].name) == nameLen )) {

            if ( optionTable[i].argOpt == CL_OPT_REQUIRED_ARGUMENT) {
                
                if ( eq ) {

                    *optArg = (char *)( eq + 1 );
                }
                else if ( optIndex + 1 < argc ) {

                    *optArg = argv[ ++optIndex ];
                }
                else {
                    
                    fprintf( stderr, "Option '--%s' requires an argument\n", 
                            optionTable[ i ].name);
                    return ( -1 );
                }
            } 
            else if ( optionTable[ i ].argOpt == CL_OPT_OPTIONAL_ARGUMENT ) {
                
                *optArg = (( eq ) ? ((char *)( eq + 1 )) : ( NULL ));
            
            } else *optArg = NULL;
    
            optIndex++;
            return optionTable[ i ].val;
        }
    }

    fprintf( stderr, "Unknown option '%s'\n", arg ) ;
    optIndex++;
    return '?';
}

//----------------------------------------------------------------------------------------
// "processCmdLineOptions" function to process all command line options. We call the
// parser in a loop to get all options one by one. The help option is special in 
// that it will list the program call help and then exit.
//
//----------------------------------------------------------------------------------------
void processCmdLineOptions( SimGlobals *glb, int argc, char *argv[ ] ) {

    char *optArg = NULL;
    int   optVal = 0;

    while (( optVal = parseCmdLineOptions( argc, 
                                           argv, 
                                           optionTable, 
                                           &optArg )) != -1 ) {

        switch ( optVal ) {

            case CL_ARG_VAL_HELP:  {

                printf( "Twin64 Simulator Version %s, PatchLevel %d\n\n", 
                        SIM_VERSION, SIM_PATCH_LEVEL );

                printf( "Usage: Twin64-Simulator [options]\n\n" );

                printf( "Options:\n" );
                printf( "  --help               : display this help message\n" );
                printf( "  --version            : display program version\n" );
                printf( "  --verbose            : enable verbose output\n" );
                printf( "  --configfile=<file>  : specify configuration file\n" );
                printf( "  --logfile=<file>     : specify log file\n" );
                printf( "  --initfile=<file>    : specify init file\n" );
                exit( 0 );
            
            } break;

            case CL_ARG_VAL_VERSION: {

                printf( "Twin64 Simulator Version %s, PatchLevel %d\n\n", 
                        SIM_VERSION, SIM_PATCH_LEVEL );
                exit( 0 );

            } break;

            case CL_ARG_VAL_VERBOSE: {

                glb -> verboseFlag = true;
        
            } break;

            case CL_ARG_VAL_CONFIG_FILE: {

                if ( optArg ) {

                    strncpy( glb -> configFileName, optArg, MAX_FILE_PATH_SIZE - 1 );
                    glb -> configFileName[ MAX_FILE_PATH_SIZE - 1 ] = '\0';
                } 
                else {
                
                    printf( "Error: --configfile requires a filename\n" );
                    exit( 1 );
                }

            } break; 

            case CL_ARG_VAL_LOG_FILE: {

                if ( optArg ) {
        
                    strncpy( glb -> logFileName, optArg, MAX_FILE_PATH_SIZE - 1 );
                    glb -> logFileName[ MAX_FILE_PATH_SIZE - 1 ] = '\0';

                    #if 0
                    glb -> logFp = fopen(glb->logFile, "w");
                    if ( ! glb-> logFp ) {
                        
                        perror( "Failed to open log file\n" );
                        exit( 1 );
                    }
                    #endif
                }
                else {
        
                    printf( "Error: --logfile requires a filename\n" );
                    exit( 1 );
                }

            } break; 

            default: {

                printf( "Invalid command parameter option, use help\n" );
                exit( 1 );
            };
        }
    }
}
