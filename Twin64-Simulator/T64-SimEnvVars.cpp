//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Environment Variables
//
//----------------------------------------------------------------------------------------
// The simulator environment has a set of environment variables. They are simple
// "name = value" pairs for integers, booleans and strings.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Environment Variables
// Copyright (C) 2020 - 2026 Helmut Fieres
//
/// This program is free software: you can redistribute it and/or modify it under the 
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
// Local name space. We try to keep utility functions local to the file.  
// None so far.
//
//----------------------------------------------------------------------------------------
namespace {

    void upshiftStr( char *s ) {

    while ( *s ) {

        *s = toupper((unsigned char) *s );
        s++;
    }
}

}; // namespace


//******************************************************************************
//******************************************************************************
//
// Object methods.
//
//******************************************************************************
//******************************************************************************

//----------------------------------------------------------------------------------------
// There predefined and user defined variables. Predefined variables are created
// at program start and initialized. They are marked predefined and optional 
// readonly by the ENV command. Also, their type cannot be changed by a new value
// of a different type.
//
// User defined variables can be changed in type and value. They are by definition 
// read and write enabled and can also be removed.
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// The ENV variable object. The table is dynamically allocated, the HWM and limit 
// pointer are used to manage the search, add and remove functions.
//
//----------------------------------------------------------------------------------------
SimEnv::SimEnv( SimGlobals *glb, int size ) {
   
    table       = (SimEnvTabEntry *) calloc( size, sizeof( SimEnvTabEntry ));
    hwm         = table;
    limit       = &table[ size ];
    this -> glb = glb;
}

//----------------------------------------------------------------------------------------
// Utility functions to return variable attributes.
//
//----------------------------------------------------------------------------------------
bool SimEnv::isValid( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].valid );
    else return( false );
}

bool SimEnv::isReadOnly( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].readOnly );
    else return( false );
}

bool SimEnv::isPredefined( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].predefined );
    else return( false );
}

SimEnvTabEntry *SimEnv::getEnvEntry( char *name ) {
    
    int index = lookupEntry( name );
    if ( index >= 0 ) return( &table[ index ] );
    else return( nullptr );
}

SimEnvTabEntry *SimEnv::getEnvEntry( int index ) {
    
    if (( index >= 0 ) && ( index < ( hwm- table ))) return( &table[ index ] );
    else return( nullptr );
}

int SimEnv::getEnvHwm( ) {

    return( hwm - table );
}

//----------------------------------------------------------------------------------------
// Look a variable. We just do a linear search up to the HWM. If not found, a 
// -1 is returned. Straightforward.
//
//----------------------------------------------------------------------------------------
int SimEnv::lookupEntry( char *name ) {
    
    SimEnvTabEntry *entry = table;

    char tmp[ MAX_ENV_NAME_SIZE ];
    strncpy( tmp, name, MAX_ENV_NAME_SIZE );

    upshiftStr( tmp );
    
    while ( entry < hwm ) {
        
        if (( entry -> valid ) && ( strcmp( entry -> name, tmp ) == 0 )) 
            return((int) ( entry - table ));
        else entry ++;
    }
    
    return( -1 );
}

//----------------------------------------------------------------------------------------
// Find a free slot for a variable. First we look for a free entry in the range
// up to the HWM. If there is none, we try to increase the HWM. If all fails, 
// the table is full.
//
//----------------------------------------------------------------------------------------
int SimEnv::findFreeEntry( ) {
    
    SimEnvTabEntry *entry = table;
    
    while ( entry < hwm ) {
        
        if ( ! entry -> valid ) return((int) ( entry - table ));
        else entry ++;
    }
    
    if ( hwm < limit ) {
        
        hwm ++;
        return((int) ( entry - table ));
    }
    else throw( ERR_ENV_TABLE_FULL );
}

//----------------------------------------------------------------------------------------
// "setEnvVar" is a set of function signatures that modify an ENV variable value.
// If the variable is a predefined variable, the readOnly option is checked as 
// well as that the variable type matches. A user defined variable is always 
// read/write enabled and the type changes based on the type of the value set. 
// If the variable is not found, a new variable will be allocated. One more 
// thing. If the ENV variable type is string and we set a value, the old string
// is deallocated.
//
//----------------------------------------------------------------------------------------
void SimEnv::setEnvVar( char *name, T64Word val ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) {
            
            throw ( ERR_ENV_VALUE_EXPR );
        }

        if (( ptr -> typ == TYP_STR ) && ( ptr -> u.strVal != nullptr )) {
            
            free( ptr -> u.strVal );
        }
         
        ptr -> typ      = TYP_NUM;
        ptr -> u.iVal   = val;
    }
    else enterVar( name, val );
}

void SimEnv::setEnvVar( char *name, bool val )  {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_BOOL )) {
            
            throw ( ERR_ENV_VALUE_EXPR );
        }

        if (( ptr -> typ == TYP_STR ) && ( ptr -> u.strVal != nullptr )) {
            
            free( ptr -> u.strVal );
        }
         
        ptr -> typ      = TYP_BOOL;
        ptr -> u.bVal   = val;
    }
    else enterVar( name, val );
}

void SimEnv::setEnvVar( char *name, char *str )  {
   
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_STR )) {
            
            throw ( ERR_ENV_VALUE_EXPR );
        }

        if (( ptr -> typ == TYP_STR ) && ( ptr -> u.strVal != nullptr )) {
            
            free( ptr -> u.strVal );
        }
        
        ptr -> typ      = TYP_STR;
        ptr -> u.strVal = (char *) malloc( strlen( str ) + 1 );
        strcpy( ptr -> u.strVal, str );
    }
    else enterVar( name, str );
}

//----------------------------------------------------------------------------------------
// Sometimes, we need to set a variable, and then mark it as predefined and/or
// readOnly.
//
//----------------------------------------------------------------------------------------
void SimEnv::setEnvAttr( char *name, bool predefined, bool readOnly ) {

    int index = lookupEntry( name );

    if ( index >= 0 ) {

        SimEnvTabEntry *ptr = &table[ index ];

        ptr -> predefined = predefined;
        ptr -> readOnly   = readOnly;
    }
}

//----------------------------------------------------------------------------------------
// Environment variables getter functions. Just look up the entry and return the 
// value. If the entry does not exist, we return an optional default.
//
//----------------------------------------------------------------------------------------
bool SimEnv::getEnvVarBool( char *name, bool def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].u.bVal );
    else                return ( def );
}

T64Word SimEnv::getEnvVarInt( char *name, T64Word def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].u.iVal );
    else                return ( def );
}

char *SimEnv::getEnvVarStr( char *name, char *def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].u.strVal );
    else                return (def );
}

//----------------------------------------------------------------------------------------
// A set of helper function to enter a variable. The variable can be a user or 
// predefined one. If it is a predefined variable, the readonly flag marks the
// variable read only for the ENV command.
//
//----------------------------------------------------------------------------------------
void SimEnv::enterVar( char *name, T64Word  val, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        upshiftStr( tmp.name );
        tmp.typ         = TYP_NUM;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.u.iVal      = val;
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

void SimEnv::enterVar( char *name, bool val, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        upshiftStr( tmp.name );
        tmp.typ         = TYP_BOOL;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.u.bVal      = val;
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

void SimEnv::enterVar( char *name, char *str, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        upshiftStr( tmp.name );
        tmp.valid       = true;
        tmp.typ         = TYP_STR;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.u.strVal    = (char *) calloc( strlen( str ), sizeof( char ));
        strcpy( tmp.u.strVal, str );
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

//----------------------------------------------------------------------------------------
// Remove a user defined ENV variable. If the ENV variable is predefined it is 
// an error. If the ENV variable type is a string, free the string space. The 
// entry is marked invalid, i.e. free. Finally, if the entry was at the high 
// water mark, adjust the HWM.
//
//----------------------------------------------------------------------------------------
void SimEnv::removeEnvVar( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index == -1 ) throw ( ERR_ENV_VAR_NOT_FOUND );
        
    SimEnvTabEntry *ptr = &table[ index ];
    
    if ( ptr -> predefined ) throw( ERR_ENV_PREDEFINED );
    
    if (( ptr -> typ == TYP_STR ) && 
        ( ptr -> u.strVal != nullptr )) free( ptr -> u.strVal );
    
    ptr -> valid    = false;
    ptr -> typ      = TYP_NIL;
    
    if ( ptr == hwm - 1 ) {
        
        while (( ptr >= table ) && ( ptr -> valid )) hwm --;
    } 
}

//----------------------------------------------------------------------------------------
// Format the ENV entry. 
//
//----------------------------------------------------------------------------------------
int SimEnv::formatEnvEntry( char *name, char *buf, int bufLen ) {

    int index = lookupEntry( name );
    return( formatEnvEntry( index, buf, bufLen ));
}

int SimEnv::formatEnvEntry( int index, char *buf, int bufLen ) {

    if (( index >= 0 ) && ( index < ( hwm - table ))) {

        SimEnvTabEntry *e = &table[ index ];

        if ( e -> valid ) {

            int len = snprintf( buf, 128, "%-32s", e -> name );
    
            switch ( e -> typ ) {
            
                case TYP_NUM: {

                    if (( e -> u.iVal >= INT_MIN ) && ( e -> u.iVal <= INT_MAX )) {

                        len += snprintf( buf + len, 128, "NUM:     %i", 
                                        (int) e -> u.iVal ); 
                    }
                    else {

                        len += snprintf( buf + len, 128, "NUM:     %llx.", 
                                         e -> u.iVal ); 
                    } 

                } break;
            
                case TYP_STR: {
                
                    // ??? any length checks ?
                    len += snprintf( buf + len, 128, "STR:     \"%s\"", 
                                     e -> u.strVal );
                
                } break;
                
                case TYP_BOOL: {
                    
                    if ( e -> u.bVal )  
                        len += snprintf( buf + len, 128, "BOOL:    TRUE");
                    else                
                        len += snprintf( buf + len, 128, "BOOL:    FALSE"); 
                
                } break;

                default:  len += snprintf( buf + len, 128, "Unknown type" );
            }

            return ( len );
        }
    }

    return ( 0 );
}

//----------------------------------------------------------------------------------------
// Enter the predefined entries.
//
//----------------------------------------------------------------------------------------
void SimEnv::setupPredefined( ) {
    
    enterVar((char *) ENV_PROG_VERSION, (char *) SIM_VERSION, true, false );
    enterVar((char *) ENV_GIT_BRANCH, (char *) SIM_GIT_BRANCH, true, false );
    enterVar((char *) ENV_PATCH_LEVEL, (T64Word) SIM_PATCH_LEVEL, true, false );

    enterVar((char *) ENV_CONFIG_FILE, (char *) "T64ConfigFile", true, false );
    enterVar((char *) ENV_LOG_FILE, (char *) "T64LogFile", true, false );

    enterVar((char *) ENV_SHOW_CMD_CNT, true, true, false );
    enterVar((char *) ENV_CMD_CNT, (T64Word) 0, true, true );
    enterVar((char *) ENV_ECHO_CMD_INPUT, false, true, false );
    enterVar((char *) ENV_EXIT_CODE, (T64Word) 0, true, false );
    
    enterVar((char *) ENV_RDX_DEFAULT, (T64Word) 16, true, false );
    enterVar((char *) ENV_WORDS_PER_LINE, (T64Word) 8, true, false );
    
    enterVar((char *) ENV_WIN_MIN_ROWS, (T64Word) 24, true, false );
    enterVar((char *) ENV_WIN_TEXT_LINE_WIDTH, (T64Word) 90, true, false );
    enterVar((char *) ENV_WIN_TEXT_TAB_SIZE, (T64Word) 4, true, false );

    enterVar((char *) ENV_HALT_ON_TRAPS, (bool) false, true, false );
}
