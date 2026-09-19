//----------------------------------------------------------------------------------------
//
// Twin-64 - System
//
//----------------------------------------------------------------------------------------
// "T64System" is the system we simulate. It consist of a set of modules. A 
// module represents a processor, a memory unit, and so on. This of the system
// as a bus where the modules are plugged into.
//
//----------------------------------------------------------------------------------------
//
// Twin-64 - System
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
    if ( aSpaLen > INT64_MAX - aSpaStart ) return ( true );

    const T64Word aSpaEnd = aSpaStart + aSpaLen - 1;

    const T64Word bSpaStart = b -> getSpaAdr( );
    if ( bSpaLen > INT64_MAX - bSpaStart ) return ( true );

    const T64Word bSpaEnd = bSpaStart + bSpaLen - 1;

    const bool ovlSpa = ( aSpaStart <= bSpaEnd ) && ( aSpaEnd   >= bSpaStart );

    const T64Word aHpaLen   = a -> getHpaLen( );
    const T64Word bHpaLen   = b -> getHpaLen( );
    const T64Word aHpaStart = a -> getHpaAdr( );

    if ( aHpaLen > INT64_MAX - aHpaStart ) return ( true );

    const T64Word aHpaEnd = aHpaStart + aHpaLen - 1;

    const T64Word bHpaStart = b -> getHpaAdr( );
    if ( bHpaLen > INT64_MAX - bHpaStart ) return ( true );

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
        systemProcMap[ i ] = nullptr;
    }

    for ( int i = 0; i < MAX_MOD_MAP_ENTRIES * 2; i++ ) {

        systemPhysMemMap[ i ] = nullptr;
        systemIoMemMap[ i ] = nullptr;
    }

    systemPhysMemMapHwm     = 0;
    systemIoMemMapHwm      = 0;
    systemProcMapHwm     = 0;
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

        for ( int i = 0; i < systemPhysMemMapHwm; ++i ) {

            if ( overlap( moduleMap[ i ], module )) return ( -3 );
        }

        for ( int i = 0; i < systemIoMemMapHwm; ++i ) {

            if ( overlap( moduleMap[ i ], module )) return ( -3 );
        }

        if ( isIo ) {

            rStat = insertIntoMap( systemIoMemMap,
                                   module,
                                   &systemIoMemMapHwm,
                                   MAX_MOD_MAP_ENTRIES );
        } 
        else {

            rStat = insertIntoMap( systemPhysMemMap,
                                   module,
                                   &systemPhysMemMapHwm,
                                   MAX_MOD_MAP_ENTRIES );
        }
    
        if ( rStat != 0 ) return( -2 );
    }

    moduleMap[ module -> getModuleNum( ) ] = module;    

    if ( module -> getModuleType( ) == MT_PROC ) {

        rStat = insertIntoMap( systemProcMap,
                               module,
                               &systemProcMapHwm,
                               MAX_MOD_MAP_ENTRIES );

        if ( rStat != 0 ) return( -2 );
    }

    module -> initModule( );
    return ( 0 );
}

//----------------------------------------------------------------------------------------
// Remove a module from the module map and system memory and IO module maps. 
// Both maps remain sorted by SPA address. The module pointer is simply removed
// from the module map. Finally, the module is stopped and deleted.
//
// However, before we can delete the module, we need to inform all others about
// the upcoming purge. There might be modules, such as the processor module, 
// that locally kept a direct pointer to the module. 
//
// The function returns 0 on success, -1 if not found.
//
//----------------------------------------------------------------------------------------
int T64System::removeModule( T64Module *module ) {

    int modNum = module -> getModuleNum( );

    busOpControl( nullptr, T64_CNTRL_EVENT_MODULE_PURGE, modNum, 0 );

    removeFromMap( systemPhysMemMap, module, &systemPhysMemMapHwm );
    removeFromMap( systemIoMemMap, module, &systemIoMemMapHwm );
    removeFromMap( systemProcMap, module, &systemProcMapHwm );
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

        int modNum = static_cast<int>( extractField64( adr, 12, 8 ));

        if ( modNum > MAX_MOD_MAP_ENTRIES - 1 ) return( nullptr );

        return( moduleMap[ modNum ] );
    }
    else {

        for ( int i = 0; i < systemPhysMemMapHwm; i++ ) {

            T64Module *mPtr = systemPhysMemMap[ i ];

            if (( adr >= mPtr -> getSpaAdr( )) && 
                ( adr <  mPtr -> getSpaAdr( ) + mPtr -> getSpaLen( ))) 
                return ( mPtr ); 
        }

        for ( int i = 0; i < systemIoMemMapHwm; i++ ) {

            T64Module *mPtr = systemIoMemMap[ i ];

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

        if ( auto *m = dynamic_cast<T64ProcThreadModule *> ( moduleMap[ modNum ] ))
            m -> haltModule( );
    }
}

//----------------------------------------------------------------------------------------
// Run a module. We will return to the caller and the threads now run in parallel.
//
//----------------------------------------------------------------------------------------
void T64System::runModule( int modNum ) {

    if (( modNum >= 0 ) && ( modNum < MAX_MOD_MAP_ENTRIES )) {

        if ( auto *m = dynamic_cast<T64ProcThreadModule *> ( moduleMap[ modNum ] ))
            m -> runModule( );
    }
}

//----------------------------------------------------------------------------------------
// Run a module for n execution units. We will wait until these units have been 
// executed. 
//
//----------------------------------------------------------------------------------------
void T64System::execModule( int modNum, int units, bool haltOnTrap ) {

    if (( modNum >= 0 ) && ( modNum < MAX_MOD_MAP_ENTRIES )) {

        if ( moduleMap[ modNum ] == nullptr ) return;

        if (( units >= 1 ) && ( units < INT32_MAX )) {

            if ( auto *m = dynamic_cast<T64ProcThreadModule *> ( moduleMap[ modNum ] )) {

                m -> execModule( units, haltOnTrap );   
                m -> waitUntilStopped( );

            }
        }
    }
}

//----------------------------------------------------------------------------------------
// RUN. The simulator can just run the system. We just enter an endless loop 
// which single steps all modules. 
//
//----------------------------------------------------------------------------------------
void T64System::simRun( ) {

    // ??? signal all processors 
}

void T64System::simHalt( ) {

    // ??? signal all processors 

}

void T64System::simStep( unsigned steps ) {

    // ??? signal all processors ?
    // ??? have an option to just advance one processor ?

}


void T64System::initBreakPointMap( ) {

    breakPointMap.enabled = true;
    breakPointMap.hwm     =  0;

    for ( unsigned i = 0; i < MAX_SIM_BREAKPOINTS; i++ ) {

        T64SimBreakPointEntry *ptr = &breakPointMap.map[ i ];

        ptr -> type     = T64_SIM_BREAK_NIL;
        ptr -> enabled  = false;
        ptr -> adr      = 0;
        ptr -> adrMask  = 0;
        ptr -> modMask  = 0;
    }
}

bool T64System::addBreakPoint( T64SimBreakPointType type,
                                       T64Word adr,
                                       T64Word len,
                                       uint64_t modMask ) {

     if ( breakPointMap.hwm >= MAX_SIM_BREAKPOINTS ) return ( false );

    auto *ptr = &breakPointMap.map[ breakPointMap.hwm ];

    ptr -> type    = type;
    ptr -> enabled = true;
      
    ptr -> adr     = adr & ~(len - 1);
    ptr -> adrMask = ~(len - 1);

    ptr -> modMask = modMask;

    breakPointMap.hwm ++;
    breakPointMap.enabled = true;

    return ( true );
}

bool T64System::removeBreakPoint( unsigned bNum ) {

    if ( bNum < breakPointMap.hwm ) {

        breakPointMap.map[ bNum ] = breakPointMap.map[ breakPointMap.hwm - 1 ];
        breakPointMap.hwm --;

        return( true );
    }
    else return( false );
}

bool T64System::enableBreakPoint( unsigned bNum, bool enb ) {

    if ( bNum >= breakPointMap.hwm ) return( false );
   
    breakPointMap.map[ bNum ].enabled = enb;
    
    for ( unsigned i = 0; i < breakPointMap.hwm; i++ ) {

        if ( breakPointMap.map[ i ].enabled ) {
            
            breakPointMap.enabled = true;
            return( true );
        }
    }

    breakPointMap.enabled = false;
    return( true );
}

bool T64System::isBreakPointEnabled( unsigned bNum ) {

     if ( bNum < breakPointMap.hwm ) {

        return( breakPointMap.map[ bNum ].enabled );
    }
    else return( false );
}

T64SimBreakPointEntry *T64System::getBreakPointEntry( unsigned bNum ) {

    if ( bNum < breakPointMap.hwm ) {

        return( &breakPointMap.map[ bNum ] );
    }
    else return( nullptr );
} 

static inline bool breakPointAddressMatches(
    const T64SimBreakPointEntry& bp,
    T64Word adr) {

    return ( adr & bp.adrMask ) == bp.adr;
}

int T64System::checkBreakPoint( T64SimBreakPointType type,
                                T64Word               adr,
                                unsigned              modNum ) {

    if ( !breakPointMap.enabled )
        return( -1 );

    uint64_t modBit = 1ULL << modNum;

    for ( unsigned i = 0; i < breakPointMap.hwm; i++ ) {

        const T64SimBreakPointEntry& bp = breakPointMap.map[ i ];

        if ( ! bp.enabled )                 continue;
        if ( bp.type != type )              continue;
        if (( bp.modMask & modBit ) == 0 )  continue;

        if (( adr & bp.adrMask ) == bp.adr ) return( i );     
    }

    return( -1 );
}

//----------------------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------------------
T64SystemState T64System::getSystemState( ) {

    return( sysState );
}

//----------------------------------------------------------------------------------------
// Bus read operation. The system is the dispatcher for bus operations. We look
// up the module that covers the address and call the module's bus event handler. 
// The module can react to the bus event and return true if it has handled the 
// event, or false if it has not handled the event. 
//
// For supporting the LDR/STC instruction, we need to support a bus read 
// reserved operation. In this case, we lock the access, read the data and
// set the reservation info in the calling processor module.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpRead( T64Module *mod, 
                           T64Word pAdr, 
                           uint8_t *data, 
                           size_t len,
                           bool rsv ) {

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return( false );

    if ( rsv ) {

        { 
            std::lock_guard<std::mutex> lk(sLock);

            if ( ! mPtr -> busOpReadEvent( pAdr, data, len )) return( false );

            if (dynamic_cast<T64ProcThreadModule*>( mod )) {

                ( reinterpret_cast<T64ProcThreadModule *> ( mod )) -> 
                                                    setRsvInfo( pAdr, true );
            }

             return ( true );
        }
    }
    else return ( mPtr -> busOpReadEvent( pAdr, data, len ));
}

//----------------------------------------------------------------------------------------
// Bus write operation. The system is the dispatcher for bus operations. We look
// up the module that covers the address and call the module's bus event handler.
//
// The cond parameter indicates whether the write operation is conditional.
// A conditional write operation is used by the STC instruction. In this case, 
// we need to check whether the calling module is a processor and has a valid 
// reservation for the address. 
//
// If the reservation is valid, we clear the reservation and perform the write
// operation. If the reservation is not valid, we do not perform the write
// operation. The return value indicates whether the write operation was
// performed or not.
//
// For normal write operations, we just perform the write operation and clear
// any reservation for the address.
//
//----------------------------------------------------------------------------------------
bool T64System::busOpWrite( T64Module *mod, 
                            T64Word pAdr, 
                            uint8_t *data, 
                            size_t len, 
                            bool cond ) {

    bool rStat = false;

    T64Module *mPtr = lookupByAdr( pAdr );
    if ( mPtr == nullptr ) return ( false );

    {
        std::lock_guard<std::mutex> lk(sLock);

        if ( cond ) {

            if ( auto p = dynamic_cast<T64ProcThreadModule*>( mod )) {

                if ( p -> getRsvAdr( ) == pAdr ) {

                    if (  p -> isRsvValid( )) {

                        p -> setRsvInfo( pAdr, false );
                        rStat = mPtr -> busOpWriteEvent( pAdr, data, len );
                    }
                    else rStat = false;
                }
                else {

                    rStat = mPtr -> busOpWriteEvent( pAdr, data, len );
                }
            }
        }
        else rStat = mPtr -> busOpWriteEvent( pAdr, data, len );

        for ( int i = 0; i < systemProcMapHwm; i ++ ) {

            if ( auto p = dynamic_cast<T64ProcThreadModule*>( systemProcMap[ i ] )) {

                if ( p -> getRsvAdr( ) == pAdr ) {

                    p -> setRsvInfo( pAdr, false );
                }
            }
        }

        return ( rStat );
    }
}

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

