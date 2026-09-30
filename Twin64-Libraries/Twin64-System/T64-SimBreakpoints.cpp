//----------------------------------------------------------------------------------------
//
// Twin-64 - Simulator breakpoints
//
//----------------------------------------------------------------------------------------
// "T64System" is the system we simulate. It consist of a set of modules. A 
// module represents a processor, a memory unit, and so on. This of the system
// as a bus where the modules are plugged into. The simulator breakpoints are
// a subsystem which allows to set a code or data breakpoint at a physical 
// address.
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
// Build a module mask from the module number. A module number of -1 represents
// all modules.
//
//----------------------------------------------------------------------------------------
uint64_t getModuleMask( int modNum ) {

    if ( modNum < 0   ) return( UINT64_MAX );
    if ( modNum >= 64 ) return( 0 );

    return( 1ULL << modNum );
}

//----------------------------------------------------------------------------------------
// When changing a breakpoint enablement, the overall enable flags are potentially
// updated too. When no breakpoint of a class is enabled the global flag is
// disabled.
//
//----------------------------------------------------------------------------------------
void updateBrkPointEnableFlags( T64SimBreakPointMap *map ) {

    map -> codeBrkPointEnabled = false;
    map -> dataBrkPointEnabled = false;

    for ( unsigned i = 0; i < map -> hwm; i++ ) {

        T64SimBreakPointEntry *e = &map -> map[ i ];

        if ( e -> enabled ) {

            switch ( e -> type ) {

                case T64_SIM_BREAK_R:
                case T64_SIM_BREAK_W:
                case T64_SIM_BREAK_RW: {

                    map -> dataBrkPointEnabled = true;

                } break;

                case T64_SIM_BREAK_X: {

                    map -> codeBrkPointEnabled = true;

                } break;

                default: ;

            }
        }
    }
}

} // namespace

//----------------------------------------------------------------------------------------
// Setup the breakpoint table.
//
//----------------------------------------------------------------------------------------
void T64System::initBreakPointMap( ) {

    breakPointMap.codeBrkPointEnabled   = false;
    breakPointMap.dataBrkPointEnabled   = false;
    breakPointMap.hwm                   =  0;

    for ( unsigned i = 0; i < MAX_SIM_BREAKPOINTS; i++ ) {

        T64SimBreakPointEntry *ptr = &breakPointMap.map[ i ];

        ptr -> type     = T64_SIM_BREAK_NIL;
        ptr -> enabled  = false;
        ptr -> adr      = 0;
        ptr -> adrMask  = 0;
        ptr -> modMask  = 0;
    }
}

//----------------------------------------------------------------------------------------
// Breakpoint high water mark in the map.
//
//----------------------------------------------------------------------------------------
unsigned T64System::getBreakPointMapHwm( ) {

    return( breakPointMap.hwm );
}

//----------------------------------------------------------------------------------------
// Overall breakpoint state. We can enable and disable all code and data break
// points. When no breakpoint is enabled, the respective flag is set to false,
// allowing a quick check before checking the breakpoint map entries.
//
//----------------------------------------------------------------------------------------
bool T64System::setCodeBrkPointEnable( bool enable ) {

    bool tmp = breakPointMap.codeBrkPointEnabled;
    breakPointMap.codeBrkPointEnabled = enable;
    return( tmp );
}

bool T64System::setDataBrkPointEnable( bool enable ) {

    bool tmp = breakPointMap.dataBrkPointEnabled;
    breakPointMap.dataBrkPointEnabled = enable;
    return( tmp );
}

bool T64System::isCodeBrkPointEnabled( ) {

    return ( breakPointMap.codeBrkPointEnabled );
}

bool T64System::isDataBrkPointENabled( ) {

    return ( breakPointMap.dataBrkPointEnabled );
}

//----------------------------------------------------------------------------------------
// Add a break point. We first check of this breakpoint already exists. If 
// so, we just update the module mask. Otherwise we try to find a free entry.
// If there is none found and we still have room, increment the high water
// mark and setup the new entry. Breakpoint ranges cannot overlap. We check 
// all existing breakpoint ranges when we allocate a new entry.
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64System::addBreakPoint( int                  modNum,
                                       T64SimBreakPointType type,
                                       T64Word              adr,
                                       T64Word              len ) {

    uint64_t modMask = getModuleMask( modNum );
    if ( modMask == 0 ) return( T64_SYS_OP_INV_MOD_NUM );

    T64Word bpAdr     = adr & ~( len - 1 );
    T64Word bpAdrMask = ~( len - 1 );

    for ( unsigned i = 0; i < breakPointMap.hwm; i++ ) {

        auto *ptr = &breakPointMap.map[ i ];

        if ( ptr -> type    == type &&
             ptr -> adr     == bpAdr &&
             ptr -> adrMask == bpAdrMask ) {

            ptr -> modMask |= modMask;
            ptr -> enabled = true;

            updateBrkPointEnableFlags( &breakPointMap );
            return( T64_SYS_OP_OK );
        }
    }

    for ( unsigned i = 0; i < breakPointMap.hwm; i++ ) {

        auto *ptr = &breakPointMap.map[ i ];

        if ( ptr -> type == T64_SIM_BREAK_NIL ) continue;
        if ( ptr -> type != type ) continue;

        T64Word ptrLen = ~ptr -> adrMask + 1;

        T64Word ptrEnd = ptr -> adr + ptrLen;
        T64Word bpEnd  = bpAdr + len;

        if ( bpAdr < ptrEnd && ptr -> adr < bpEnd ) { 
            
            return( T64_SYS_OP_ADR_OVERLAP );
        }
    }

    unsigned bNum = breakPointMap.hwm;

    for ( unsigned i = 0; i < breakPointMap.hwm; i++ ) {

        if ( breakPointMap.map[ i ].type == T64_SIM_BREAK_NIL ) {

            bNum = i;
            break;
        }
    }

    if ( bNum == breakPointMap.hwm ) {

        if ( breakPointMap.hwm >= MAX_SIM_BREAKPOINTS ) {
            
            return( T64_SYS_OP_INVALID_BNUM );
        }

        breakPointMap.hwm++;
    }

    auto *ptr = &breakPointMap.map[ bNum ];

    ptr -> type         = type;
    ptr -> enabled      = true;
    ptr -> adr          = bpAdr;
    ptr -> adrMask      = bpAdrMask;
    ptr -> modMask      = modMask;
    ptr -> armedMask    = modMask;

    updateBrkPointEnableFlags( &breakPointMap );
    return( T64_SYS_OP_OK );
}

//----------------------------------------------------------------------------------------
// Remove a module from the breakpoint mask. If no modules are left in the 
// module mask, the breakpoint itself is removed. A module number of -1 has
// the same effect. When a breakpoint is removed and it is the entry at the 
// high water mark, we shrink the high water mark accordingly.
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64System::removeBreakPoint( unsigned bNum, int modNum ) {

    uint64_t modMask = getModuleMask( modNum );
    if ( modMask == 0 ) return( T64_SYS_OP_INV_MOD_NUM );

    if ( bNum >= breakPointMap.hwm ) return( T64_SYS_OP_INV_MOD_NUM );

    auto *ptr = &breakPointMap.map[ bNum ];
    if ( ptr -> type == T64_SIM_BREAK_NIL ) return( T64_SYS_OP_INVALID_BNUM );

    ptr -> modMask &= ~modMask;
    if ( ptr -> modMask != 0 ) return( T64_SYS_OP_OK );

    ptr -> type    = T64_SIM_BREAK_NIL;
    ptr -> enabled = false;
    ptr -> adr     = 0;
    ptr -> adrMask = 0;
    ptr -> modMask = 0;

    while ( breakPointMap.hwm > 0 ) {

        if ( breakPointMap.map[ breakPointMap.hwm - 1 ].type
                                     != T64_SIM_BREAK_NIL ) break;
        breakPointMap.hwm--;
    }

    updateBrkPointEnableFlags( &breakPointMap );
    return( T64_SYS_OP_OK );
}

//----------------------------------------------------------------------------------------
// Enable / disable a disabled breakpoint. If all breakpoints are disabled,
// the global enable flag is also updated.  
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64System::enableBreakPoint( unsigned bNum, bool enb ) {

    if ( bNum >= breakPointMap.hwm ) return( T64_SYS_OP_INVALID_BNUM );
   
    breakPointMap.map[ bNum ].enabled = enb;
    
    for ( unsigned i = 0; i < breakPointMap.hwm; i++ ) {

        if ( breakPointMap.map[ i ].enabled ) {
            
            updateBrkPointEnableFlags( &breakPointMap );
            return( T64_SYS_OP_OK );
        }
    }

    updateBrkPointEnableFlags( &breakPointMap );
    return( T64_SYS_OP_OK );
}

bool T64System::isBreakPointEnabled( unsigned bNum ) {

    if ( bNum < breakPointMap.hwm ) return( breakPointMap.map[ bNum ].enabled );
    else                            return( false );
}

//----------------------------------------------------------------------------------------
// For supporting single stepping, we need to just suspend code breakpoints 
// for that one step.
//
//----------------------------------------------------------------------------------------
void T64System::suspendCodeBreakPoints( bool suspend ) {

    breakPointMap.codeBreakPointsSuspended = suspend;
}

bool T64System::areCodePointsSuspended( ) {

    return( breakPointMap.codeBreakPointsSuspended );
}

//----------------------------------------------------------------------------------------
// Return the armed state of a breakpoint.
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64System::armBreakPoint( int modNum, unsigned bNum, bool arm ) {

    uint64_t modMask = getModuleMask( modNum );
    if ( modMask == 0 ) return( T64_SYS_OP_INV_MOD_NUM );

    if ( bNum >= breakPointMap.hwm )return( T64_SYS_OP_INVALID_BNUM );

    if ( arm )  breakPointMap.map[ bNum ].armedMask |= modMask;
    else        breakPointMap.map[ bNum ].armedMask &= ~modMask;

    return( T64_SYS_OP_OK );
}

bool T64System::isBreakPointArmed( int modNum, unsigned bNum ) {

    uint64_t modMask = getModuleMask( modNum );
    if ( modMask == 0 ) return( false );

    if ( bNum >= breakPointMap.hwm ) return( false );

    return(( breakPointMap.map[ bNum ].armedMask & modMask ) != 0 );
}

//----------------------------------------------------------------------------------------
// Return a pointer to the breakpoint entry.
//
//----------------------------------------------------------------------------------------
T64SimBreakPointEntry *T64System::getBreakPointEntry( unsigned bNum ) {

    if ( bNum < breakPointMap.hwm ) return( &breakPointMap.map[ bNum ] );
    else                            return( nullptr );
} 

//----------------------------------------------------------------------------------------
// Return a string version of the breakpoint type.
//
//----------------------------------------------------------------------------------------
const char  *T64System::getBreakPointTypeStr( T64SimBreakPointType t ) {

    switch( t ) {

        case T64_SIM_BREAK_X:   return ( "CODE"   );
        case T64_SIM_BREAK_R:   return ( "DATA_R" );
        case T64_SIM_BREAK_W:   return ( "DATA_W" );
        case T64_SIM_BREAK_RW:  return ( "DATA"   );
        default:                return ( "BRK:??" );
    }
}

//----------------------------------------------------------------------------------------
// Check for a breakpoint. This routine is called for each instruction fetch
// and data access. We first check that there are breakpoints at all. If so,
// we search for a matching and enabled breakpoint.
//
//----------------------------------------------------------------------------------------
int T64System::checkBreakPoint( T64SimBreakPointType bType,
                                T64Word              adr, 
                                int                  modNum ) {

    if ( bType == T64_SIM_BREAK_NIL ) return( -1 );

    if ( bType == T64_SIM_BREAK_X ) {

        if ( ! breakPointMap.codeBrkPointEnabled ) return( -1 );
    }
    else {

        if ( ! breakPointMap.dataBrkPointEnabled ) return( -1 );
    }
    
    uint64_t modBit = ( modNum == -1 ) ? UINT64_MAX : 1ULL << modNum;

    for ( unsigned i = 0; i < breakPointMap.hwm; i++ ) {

        const T64SimBreakPointEntry& bp = breakPointMap.map[ i ];

        if ( ! bp.enabled )                 continue;
        if (( bp.modMask & modBit ) == 0 )  continue;
        if (( adr & bp.adrMask ) == bp.adr ) return ( static_cast<int> ( i ));  
    }

    return( -1 );
}
