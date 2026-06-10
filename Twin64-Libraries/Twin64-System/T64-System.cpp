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

//----------------------------------------------------------------------------------------
// Insert a module in one of the auxiliary maps.
//
//----------------------------------------------------------------------------------------
int insertIntoMap( T64Module **map, T64Module *module, int *hwm, int maxEntries ) {

    if ( *hwm >= maxEntries ) return ( -1 );

    int pos = 0;

    while (( pos < *hwm ) &&
           ( map[ pos ] -> getSpaAdr( ) < module -> getSpaAdr( ))) {

        pos++;
    }

    for ( int i = *hwm; i > pos; --i ) {

        map[i] = map[i - 1];
    }

    map[ pos ] = module;

    (*hwm) ++;

    return ( 0 );
}

//----------------------------------------------------------------------------------------
// Remove a module from one of the auxiliary maps. We locate the module, remove
// it and maintain the rest of the map sorted. If found the HWM is decremented.
//----------------------------------------------------------------------------------------
void removeFromMap( T64Module **map, T64Module *module, int *hwm ) {

    int pos = -1;

    for ( int i = 0; i < *hwm; i++ ) {

        if ( map[ i ] == module ) {

            pos  = i;
            break;  
        }
    }

    if ( pos >= 0 ) {

        for ( int i = pos; i < *hwm - 1; ++i ) {

            map[ i ] = map[ i + 1 ];
        }

        (*hwm) --;

        map[ *hwm ] = nullptr;
    }
}

}; // namespace

//----------------------------------------------------------------------------------------
// The T64System object.
//
//----------------------------------------------------------------------------------------
T64System::T64System( ) {

   initModuleMap( );
}

//----------------------------------------------------------------------------------------
// Init the data structures. There are twice as many entries in the system and 
// IO map as there are modules, simply because each module can have two SPA
// ranges.
//
//----------------------------------------------------------------------------------------
void T64System::initModuleMap( ) {

    for ( int i = 0; i < MAX_MOD_MAP_ENTRIES; i++ ) {

        moduleMap[ i ] = nullptr;
        systemRsvMap[ i ] = nullptr;
    }

    for ( int i = 0; i < MAX_MOD_MAP_ENTRIES * 2; i++ ) {

        systemMemMap[ i ] = nullptr;
        systemIoMap[ i ] = nullptr;
    }

    systemMemMapHwm     = 0;
    systemIoMapHwm      = 0;
    systemRsvMapHwm     = 0;
}

//----------------------------------------------------------------------------------------
//
// ??? under construction... what should it report ?
//----------------------------------------------------------------------------------------
int T64System::getSystemState( ) {

    return( 0 );
}

//----------------------------------------------------------------------------------------
// Add a module. There are three tables. The first table just contains the 
// modules, indexed by module number. The second and third table contain only 
// the modules  that have an SPA address. The entries these table are sorted by
// the SPA address range, which also cannot overlap. A nice side effect of the 
// sorted address range and the separation of memory and SPA address ranges, is
// that memory comes first and the lookup will be quick. It is by far the most
// used lookup.
//
// The function returns 0 on success, -1 on an invalid module number, -2 when 
// the table is full, a -3 on address range overlap, and a -4 when the module 
// number is already used.
//
//----------------------------------------------------------------------------------------
int T64System::addModule( T64Module *module ) {

    if (( module -> getModuleNum( ) > MAX_MOD_MAP_ENTRIES )) return ( -1 );
    if (moduleMap[ module -> getModuleNum( ) ] != nullptr ) return ( -4 );

    bool isIo = isInRange( module->getSpaAdr(),
                           T64_IO_SPA_MEM_START,
                           T64_IO_SPA_MEM_LIMIT);

    int rStat;

    if ( module -> getSpaLen( ) > 0 ) {

        for ( int i = 0; i < systemMemMapHwm; ++i ) {

            if ( overlap( moduleMap[ i ], module )) return ( -3 );
        }

        for ( int i = 0; i < systemIoMapHwm; ++i ) {

            if ( overlap( moduleMap[ i ], module )) return ( -3 );
        }

        if ( isIo ) {

            rStat = insertIntoMap( systemIoMap,
                                   module,
                                   &systemIoMapHwm,
                                   MAX_MOD_MAP_ENTRIES );
        } 
        else {

            rStat = insertIntoMap( systemMemMap,
                                   module,
                                   &systemMemMapHwm,
                                   MAX_MOD_MAP_ENTRIES );
        }
    
        if ( rStat != 0 ) return( -2 );
    }

    moduleMap[ module -> getModuleNum( ) ] = module;    

    if ( module -> getModuleType( ) == MT_PROC ) {

        rStat = insertIntoMap( systemRsvMap,
                               module,
                               &systemRsvMapHwm,
                               MAX_MOD_MAP_ENTRIES );

        if ( rStat != 0 ) return( -2 );
    }

    module -> initModule( );
    return ( 0 );
}

//----------------------------------------------------------------------------------------
// Remove a module from the module map and system memory and IO module maps. Both
// maps remain sorted by SPA address. The module pointer is simply removed from
// the module map. Finally, the module is stopped and deleted.
//
// However, before we can delete the module, we need to inform all others about
// the upcoming purge. There might be modules, such as the processor module, that
// kept a direct pointer to the module. 
//
// The function returns 0 on success, -1 if not found.
//
//----------------------------------------------------------------------------------------
int T64System::removeModule( T64Module *module ) {

    int modNum = module -> getModuleNum( );

    busOpControl( nullptr, T64_CNTRL_EVENT_MODULE_PURGE, modNum, 0 );

    removeFromMap( systemMemMap, module, &systemMemMapHwm );
    removeFromMap( systemIoMap, module, &systemIoMapHwm );
    removeFromMap( systemRsvMap, module, &systemRsvMapHwm );
    moduleMap[ modNum ] = nullptr;
    delete module;

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
// Find the first module with a matching type.
//
//----------------------------------------------------------------------------------------
T64Module *T64System::lookupByModuleType( T64ModuleType typ ) {

    for ( int i = 0; i < MAX_MOD_MAP_ENTRIES; i++ ) {

        if (( moduleMap[ i ] != nullptr ) &&
            ( moduleMap[ i ]-> moduleTyp == typ )) return ( moduleMap[ i ] );
    }

    return ( nullptr );
}

//----------------------------------------------------------------------------------------
// Find the module entry that covers the address. Since we have only a small
// number of module, we do a simple linear search of the system map. We check 
// whether the address is in the MEM or IO SPA or HPA address range. We return
// the first match, which is ok since the address ranges cannot overlap. 
//
// For performance reasons there is a certain order how we search. We will 
// first check with a simple HPA range comparison whether we look at an HPA 
// address range. Next, we will look at the MEM SPA range followed by the IO
// SPA range. Access to the MEM SPA range, which holds the physical memory, is
// by far the most performed access.
//
//----------------------------------------------------------------------------------------
T64Module *T64System::lookupByAdr ( T64Word adr ) const {

    if (( adr >= T64_IO_HPA_MEM_START ) && ( adr < T64_IO_HPA_MEM_LIMIT )) {

        int modNum = extractField64( adr, 12, 6 );

        if ( modNum > MAX_MOD_MAP_ENTRIES - 1 ) return( nullptr );

        return( moduleMap[ modNum ] );
    }
    else {

        for ( int i = 0; i < systemMemMapHwm; i++ ) {

            T64Module *mPtr = systemMemMap[ i ];

            if (( adr >= mPtr -> getSpaAdr( )) && 
                ( adr <  mPtr -> getSpaAdr( ) + mPtr -> getSpaLen( ))) 
                return ( mPtr ); 
        }

        for ( int i = 0; i < systemIoMapHwm; i++ ) {

            T64Module *mPtr = systemIoMap[ i ];

            if (( adr >= mPtr -> getSpaAdr( )) && 
                ( adr <  mPtr -> getSpaAdr( ) + mPtr -> getSpaLen( ))) 
                return ( mPtr ); 
        }

        return nullptr;
    }
} 

//----------------------------------------------------------------------------------------
// Get the module type.
//
//----------------------------------------------------------------------------------------
T64ModuleType T64System::getModuleType( int modNum ) const {

    T64Module *mod = lookupByModNum( modNum );
    return (( mod != nullptr ) ? mod -> getModuleType( ) : MT_NIL );
}

//----------------------------------------------------------------------------------------
// Reset modules. We just invoke the module handler for the registered module.
// A module number of -1 will reset all modules.
//
//----------------------------------------------------------------------------------------
void T64System::resetModule( int modNum ) {

    if (( modNum >= 0 ) && ( modNum < MAX_MOD_MAP_ENTRIES )) {

        if ( moduleMap[ modNum ] != nullptr )
            moduleMap[ modNum ] -> resetModule( );
    }
}

//----------------------------------------------------------------------------------------
// Halt module. We just invoke the module handler for the registered module.
//
//----------------------------------------------------------------------------------------
void T64System::haltModule( int modNum ) {

    if (( modNum >= 0 ) && ( modNum < MAX_MOD_MAP_ENTRIES )) {

        if ( auto *m = dynamic_cast<T64ThreadModule *> ( moduleMap[ modNum ] ))
            m -> haltModule( );
    }
}

//----------------------------------------------------------------------------------------
// Run a module. We will return to the caller and the threads now run in parallel.
//
//----------------------------------------------------------------------------------------
void T64System::runModule( int modNum ) {

    if (( modNum >= 0 ) && ( modNum < MAX_MOD_MAP_ENTRIES )) {

        if ( auto *m = dynamic_cast<T64ThreadModule *> ( moduleMap[ modNum ] ))
            m -> runModule( );
    }
}

//----------------------------------------------------------------------------------------
// Run a module for n execution units. We will wait until these units have been 
// executed. 
//
//----------------------------------------------------------------------------------------
void T64System::execModule( int modNum, int units ) {

    if (( modNum >= 0 ) && ( modNum < MAX_MOD_MAP_ENTRIES )) {

        if ( moduleMap[ modNum ] == nullptr ) return;

        if (( units >= 1 ) && ( units < UINT32_MAX )) {

            if ( auto *m = dynamic_cast<T64ThreadModule *> ( moduleMap[ modNum ] )) {

                m -> execModule( units );
                m -> waitUntilHalted( );    

            }
        }
    }
}

//----------------------------------------------------------------------------------------
// RUN. The simulator can just run the system. We just enter an endless loop which 
// single steps all modules. 
//
//----------------------------------------------------------------------------------------
void T64System::run( ) {

    // ??? signal all processors 
}

//----------------------------------------------------------------------------------------
// Bus read operation. The system is the dispatcher for bus operations. We look
// up the module that covers the address and call the module's bus event handler. 
// The module can react to the bus event and return true if it has handled the 
// event, or false if it has not handled the event. 
//
//----------------------------------------------------------------------------------------
bool T64System::busOpRead( T64Word pAdr, uint8_t *data, int len ) {

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return( false );

    return ( mPtr -> busOpReadEvent( pAdr, data, len ));
}

//----------------------------------------------------------------------------------------
// Bus read and reserve operation. The LDR instruction implements our foundation
// for mutexes, semaphores, etc. A bus read reserved operation will just as the 
// normal read operation read the data value, and also remember the address 
// used. The operation needs to be protected by a mutex, as we potentially run 
// several processors.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpReadRsv( T64Word pAdr, uint8_t *data, int len ) {

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return( false );

    bool rStat;

    {
        std::lock_guard<std::mutex> lk(sLock);

        rStat = mPtr -> busOpReadEvent( pAdr, data, len );
        if ( rStat ) mPtr -> setRsvInfo( pAdr, true );
    }

    return ( rStat );
}

//----------------------------------------------------------------------------------------
// Bus write operation. The system is the dispatcher for bus operations. We look
// up the module that covers the address and call the module's bus event handler.
// Since the write operation could potentially address a location used by a 
// LDR/STC instruction, we need to synchronize access and if there is an address
// match clear the reservation.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpWrite( T64Word pAdr, uint8_t *data, int len ) {

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return ( false );

    bool rStat;

    {
        std::lock_guard<std::mutex> lk(sLock);
        rStat = mPtr -> busOpWriteEvent( pAdr, data, len );

        for ( int i = 0; i < systemRsvMapHwm; i ++ ) {

            T64Module *m = systemRsvMap[ i ];

            if (( m -> isRsvValid( )) && ( m -> getRsvInfo( ) == pAdr )) {

                m -> setRsvInfo( 0, false );
            }
        }
    }

    return ( rStat );
}

//----------------------------------------------------------------------------------------
// Bus write conditional operation.This  bus operation is only used by the STC 
// instruction. We look up the module that covers the address and look for the
// address in the reservation table. If found and the reservation is still valid, 
// the value is written to that address. Otherwise, we failed and report the 
// status.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpWriteCond( T64Module *mod,
                                T64Word pAdr, 
                                uint8_t *data, 
                                int     len ) {

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return ( false );

    bool rStat = true;

    {
        std::lock_guard<std::mutex> lk(sLock);

        for ( int i = 0; i < systemRsvMapHwm; i ++ ) {

            T64Module *m = systemRsvMap[ i ];

            if ( m == mod ) {

                if (( m -> isRsvValid( )) && ( m -> getRsvInfo( ) == pAdr )) {

                    rStat = mPtr -> busOpWriteEvent( pAdr, data, len );

                    // ??? how exactly do we return failure....
                    rStat = false;
                }
            }
        }
    }

    return ( rStat );
}

//----------------------------------------------------------------------------------------
// When a processor encounters a trap or an external interrupt, a valid 
// reservation initiated by this processor will be cleared in the processor.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpClearRsv(  int reqModNum ) {

    {
        std::lock_guard<std::mutex> lk(sLock);

        for ( int i = 0; i < systemRsvMapHwm; i ++ ) {

            T64Module *m = systemRsvMap[ i ];

                if ( m -> getModuleType( ) == reqModNum ) {

                m -> setRsvInfo( 0, false );
            }
        }
    }

    return( true );
}

// ??? think about rather doing TLB and module events via direct calls ...


bool T64System::busOpGTlbPurge( T64Word vAdr ) {

    {
        std::lock_guard<std::mutex> lk(sLock);                              

        for ( int i = 0; i < MAX_MOD_MAP_ENTRIES; i++ ) {

            if ( moduleMap[ i ] != nullptr ) {

                // ??? if a processor or global TLB ...
            }
        }
    }

    return( true );
}

bool T64System::busOpModulePurge( int modNum ) {

    {
        std::lock_guard<std::mutex> lk(sLock);                              

        for ( int i = 0; i < MAX_MOD_MAP_ENTRIES; i++ ) {

            if ( moduleMap[ i ] != nullptr ) {

                // ??? if a processor or global TLB ...
            }
        }
    }

    return( true );
}

// ??? we would also need to have a reveiver routine in the modules...
// ??? add them to processor and global tlb class
// ??? check via dynamic_cast ?

//----------------------------------------------------------------------------------------
// Bus broadcast operation. We need to provide a way to signal global events
// such as a TLB entry purge to all modules. We will lock the system mutex and 
// inform all modules.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpControl( T64Module *mod,
                              T64BBusOpControlEvents event,
                              T64Word            arg1, 
                              T64Word            arg2 ) {

    {
        std::lock_guard<std::mutex> lk(sLock);                              

        for ( int i = 0; i < MAX_MOD_MAP_ENTRIES; i++ ) {

            if ( moduleMap[ i ] != nullptr )
                moduleMap[ i ] -> busOpControlEvent( event, arg1, arg2 );
        }
    }

    return( true );
}

