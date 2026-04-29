//----------------------------------------------------------------------------------------
//
// Twin-64 - System
//
//----------------------------------------------------------------------------------------
// "T64System" is the system we simulate. It consist of a set of modules. A module
// represents a processor, a memory unit, and so on. This of the system as a bus 
// where the modules are plugged into.
//
//----------------------------------------------------------------------------------------
//
// Twin-64 - System
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
#include "T64-System.h"

//----------------------------------------------------------------------------------------
// Thread model. The system maintains also an array of threads. There could be 
// a thread for each processor or I/O module. 
// 
// Basic idea:  
//
//  #include <array>
//  std::array<T64Thread, MAX_THREADS> threadMap;
//
//  for ( int i = 0; < MAX_THREADS; i++ ) 
//      threadMap[ i ] = str::thread( &T64Processor::processorThread, processor[ i ] );
// 
// A module needs a mutex, and a state. And a signal mask.
//
//  std::atomic<uint32_t> mask = 0;
//
// The thread execution loop would be something like this:
//
//  while ( true ) {
//     executeInstruction( );
//     if ( mask.load( memory_ordered_release ) != 0 ) handleSignals( );
//  }
//
// We would also need a condition variable where a thread can wait for a signal.
//
// Need to think about all this .... :-).
// 
//--------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Name space for local routines.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// Check whether the module number is equal and whether the HPA or SPA address 
// range of two modules overlap. If we are passed the same modules, we will 
// by definition overlap. Modules with an SPA len of zero do never overlap. 
//
//----------------------------------------------------------------------------------------
bool overlap( T64Module *a, T64Module *b ) {

    const T64Word aModuleNum = a -> getModuleNum( );
    const T64Word bModuleNum = b -> getModuleNum( );
    if ( aModuleNum == bModuleNum ) return true;

    const T64Word aSpaLen = a -> getSpaLen( );
    const T64Word bSpaLen = b -> getSpaLen( );
    if (( aSpaLen == 0 ) || ( bSpaLen == 0 )) return ( false );

    const T64Word aSpaStart = a -> getSpaAdr( );
    if ( aSpaLen > UINT64_MAX - aSpaStart ) return ( true );

    const T64Word aSpaEnd = aSpaStart + aSpaLen - 1;

    const T64Word bSpaStart = b -> getSpaAdr( );
    if ( bSpaLen > UINT64_MAX - bSpaStart ) return ( true );

    const T64Word bSpaEnd = bSpaStart + bSpaLen - 1;

    const bool ovlSpa = ( aSpaStart <= bSpaEnd ) && ( aSpaEnd   >= bSpaStart );

    const T64Word aHpaLen   = a -> getHpaLen( );
    const T64Word bHpaLen   = b -> getHpaLen( );
    const T64Word aHpaStart = a -> getHpaAdr( );

    if ( aHpaLen > UINT64_MAX - aHpaStart ) return ( true );

    const T64Word aHpaEnd = aHpaStart + aHpaLen - 1;

    const T64Word bHpaStart = b -> getHpaAdr( );
    if ( bHpaLen > UINT64_MAX - bHpaStart ) return ( true );

    const T64Word bHpaEnd = bHpaStart + bHpaLen - 1;

    const bool ovlHpa = ( aHpaStart <= bHpaEnd ) && ( aHpaEnd   >= bHpaStart );

    return ( ovlSpa || ovlHpa );
}

};

//----------------------------------------------------------------------------------------
// The T64System object.
//
//----------------------------------------------------------------------------------------
T64System::T64System( ) {

   initModuleMap( );
}

void T64System::initModuleMap( ) {

    for ( int i = 0; i < MAX_MOD_MAP_ENTRIES; i++ ) {

        moduleMap[ i ] = nullptr;
    }

    systemMemMapHwm = 0;
}

//----------------------------------------------------------------------------------------
//
// ??? under construction...
//----------------------------------------------------------------------------------------
int T64System::getSystemState( ) {

    return( 0 );
}

//----------------------------------------------------------------------------------------
// Add a module. There are two tables. The first table just contains the modules.
// The index is the module number. The second table contains on the modules which
// have an SPA address. The entries this table are sorted by the SPA address 
//range, which also cannot overlap. We look for the insertion position,
// shift all entries up after this position and insert the new entry. A nice
// side effect of the sorted address range, is that memory comes first and 
// the lookup will be quick. It is by far the most used lookup.
//
// Returns 0 on success, -1 on an invalid module number, -2 when the table is
// full, a -3 on address range overlap, and a -4 when the module number is
// already used.
//
//----------------------------------------------------------------------------------------
int T64System::addToModuleMap( T64Module *module ) {

    if (( module -> getModuleNum( ) > MAX_MOD_MAP_ENTRIES )) return ( -1 );
    if ( systemMemMapHwm >= MAX_MOD_MAP_ENTRIES ) return ( -2 );
    
    for ( int i = 0; i < systemMemMapHwm; ++i ) {

        if ( overlap( moduleMap[ i ], module )) return ( -3 );
    }

    if ( moduleMap[ module -> getModuleNum( ) ] != nullptr ) return ( -4 );
    moduleMap[ module -> getModuleNum( ) ] = module;    

    if ( module -> getSpaLen( ) == 0 ) return ( 0 );

    int pos = 0;
    while (( pos < systemMemMapHwm ) &&
           ( systemMemMap[ pos ] -> getSpaAdr( ) < module->getSpaAdr( ))) {
        pos++;
    }

    for ( int i = systemMemMapHwm; i > pos; i-- ) {

        systemMemMap[ i ] = systemMemMap[ i - 1] ;
    }

    systemMemMap[ pos ] = module;
    systemMemMapHwm++;

    return ( 0 );
}

//----------------------------------------------------------------------------------------
// Remove a module from the module map. The map remains sorted by SPA address.
// We find the module, shift all entries after it down, and decrement HWM.
//
// Returns 0 on success, -1 if not found.
//----------------------------------------------------------------------------------------
int T64System::removeFromModuleMap( T64Module *module ) {

    T64Module *mPtr = nullptr;
    int         pos = -1;

    for ( int i = 0; i < systemMemMapHwm; ++i ) {

        if ( systemMemMap[ i ] == module ) {

            pos  = i;
            mPtr = systemMemMap[ i ];
            break;  
        }
    }

    moduleMap[ mPtr -> getModuleNum( ) ] = nullptr;

    if ( pos < 0 ) return ( -1 );
    
    for ( int i = pos; i < systemMemMapHwm - 1; ++i ) {

        systemMemMap[ i ] = systemMemMap    [ i + 1 ];
    }

    systemMemMapHwm--;

    return ( 0 );
}

//----------------------------------------------------------------------------------------
// Find the module entry by its module number.
//
//----------------------------------------------------------------------------------------
T64Module *T64System::lookupByModNum( int modNum ) const {

    if ( modNum > MAX_MOD_MAP_ENTRIES ) return ( nullptr );
    return ( moduleMap[ modNum ] );
}

//----------------------------------------------------------------------------------------
// Find the module entry that covers the address. Since we have only a small
// number of module, we do a simple linear search. We check whether the address
// is in the SPA or HPA address range. We return the first match, which is ok 
// since the address ranges cannot overlap. We search the SPA range first, since
// this is the main access path. Since physical represents the lower SPA address
// range, a lookup of a memory address will be rather quick, which is key for
// efficient physical memory access.
//
// ??? well, we run through the module, which are not sorted by SPA...
//----------------------------------------------------------------------------------------
T64Module *T64System::lookupByAdr ( T64Word adr ) const {

    for ( int i = 0; i < systemMemMapHwm; i++ ) {

        T64Module *mPtr = systemMemMap[ i ];

        if (( adr >= mPtr -> getSpaAdr( )) && 
            ( adr <  mPtr -> getSpaAdr( ) + mPtr -> getSpaLen( ))) 
            return ( mPtr ); 

        if (( adr >= mPtr -> getHpaAdr( )) && 
            ( adr <  mPtr -> getHpaAdr( ) + mPtr -> getHpaLen( ))) 
            return ( mPtr );
    }

    return nullptr;
} 

//----------------------------------------------------------------------------------------
// Get the module type.
//
// ??? where used... rather encode where used ?
//----------------------------------------------------------------------------------------
T64ModuleType T64System::getModuleType( int modNum ) const {

    T64Module *mod = lookupByModNum( modNum );
    return (( mod != nullptr ) ? mod -> getModuleType( ) : MT_NIL );
}

//----------------------------------------------------------------------------------------
// Reset the system. We just invoke the module handler for each registered module.
//
//----------------------------------------------------------------------------------------
void T64System::reset( ) {

    for ( int i = 0; i < systemMemMapHwm; i++ ) {

        if ( moduleMap[ i ] != nullptr ) moduleMap[ i ] -> reset( ); 
    }
}

//----------------------------------------------------------------------------------------
// RUN. The simulator can just run the system. We just enter an endless loop which 
// single steps all modules. 
//
//----------------------------------------------------------------------------------------
void T64System::run( ) {

    while ( true ) step( 1, -1 );
}

//----------------------------------------------------------------------------------------
// Step the system. We step all modules, or just the one specified by the module
// number. The module number is -1 for all modules.
//
//----------------------------------------------------------------------------------------
void T64System::step( int steps, int modNum ) {

    if ( modNum != -1 ) {

        T64Module *mPtr = lookupByModNum( modNum );
        if ( mPtr != nullptr ) mPtr -> step( ); 
    }
    else

        for ( int i = 0; i < systemMemMapHwm; i++ ) {

        if ( moduleMap[ i ] != nullptr ) moduleMap[ i ] -> step( ); 
    }
}

//----------------------------------------------------------------------------------------
// Bus read operation. The system is the dispatcher for bus operations. We look
// up the module that covers the address and call the module's bus event handler. 
// The module can react to the bus event and return true if it has handled the 
// event, or false if it has not handled the event. 
//
//----------------------------------------------------------------------------------------
bool T64System::busOpRead( int reqModNum,
                           T64Word pAdr, 
                           uint8_t *data, 
                           int     len ) {

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return ( false );

    return ( mPtr -> busOpReadEvent( reqModNum, pAdr, data, len ));
}

//----------------------------------------------------------------------------------------
// Bus write operation. The system is the dispatcher for bus operations. We look
// up the module that covers the address and call the module's bus event handler.
// The module can react to the bus event and return true if it has handled the 
// event, or false if it has not handled the event.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpWrite( int reqModNum,
                            T64Word pAdr, 
                            uint8_t *data, 
                           int     len ) {

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return ( false );

    return ( mPtr -> busOpWriteEvent( reqModNum, pAdr, data, len ));
}

//****************************************************************************************
//****************************************************************************************
//
// Module
//----------------------------------------------------------------------------------------
// A module is an object plugged into the imaginary system bus. It has a type and 
// a module number, which is the slot in that bus. Each module has a dedicated memory
// page page in the IO HPA space. The address is easily computed from the slot 
// number. In addition, a module can have several SPA regions. This is however module
// specific and not stored at the common module level.
//
//----------------------------------------------------------------------------------------
T64Module::T64Module( T64ModuleType    modType, 
                      int              modNum,
                      T64Word          spaAdr,
                      int              spaLen ) {

    this -> moduleTyp   = modType;
    this -> moduleNum   = modNum;
    this -> hpaAdr      =  T64_IO_HPA_MEM_START + ( modNum * T64_PAGE_SIZE_BYTES );
    this -> hpaLen      = T64_PAGE_SIZE_BYTES;
    this -> spaAdr      = spaAdr;
    this -> spaLen      = spaLen;
    this -> spaLimit    = spaAdr + spaLen - 1;
}

int T64Module::getModuleNum( ) {

    return ( moduleNum );
}

T64ModuleType T64Module::getModuleType( ) {

    return ( moduleTyp );
}

const char *T64Module::getModuleTypeName( ) {

    switch ( moduleTyp ) {

        case MT_PROC:       return ((char *) "PROC" );
        case MT_CPU_CORE:   return ((char *) "CPU" );
        case MT_CPU_TLB:    return ((char *) "TLB"  );
        case MT_IO:         return ((char *) "IO" );
        case MT_MEM:        return ((char *) "MEM" );

        case MT_NIL:
        default:            return ((char *) "NIL" );
    }
}

T64Word T64Module::getHpaAdr( ) {

    return ( hpaAdr );
}

int T64Module::getHpaLen( ) {

    return ( hpaLen );
}

T64Word T64Module::getSpaAdr( )  {

    return ( spaAdr );
}

int T64Module::getSpaLen( )  {

    return ( spaLen );
}
