//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Processor Module
//
//----------------------------------------------------------------------------------------
// The processor object is a module for the T64 System. It consist of the CPU 
// and the TLB subsystems. The processor will run as a thread in the simulator.
//
// The processor thread interacts with the system bus, which is essentially a
// shared resource. The system bus will route read and write request to the 
// responsible module that is responsible for the address. A module, including
// the processor, can also be responsible for an address range. In this case, 
// the busOpEvent routines of the module are called to handle the bus event. 
//
// If there more than one processor, there needs to be a selection process for
// the monarch, which will bring up the system. 
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Processor Module
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
#include "T64-Processor.h"

//----------------------------------------------------------------------------------------
// Name space for local routines.
//
//----------------------------------------------------------------------------------------
namespace {

};

//****************************************************************************************
//****************************************************************************************
//
// Processor. 
//
//----------------------------------------------------------------------------------------
// A processor is a module with one CPU, TLBs and one day also Caches. We create
// the component objects right here and pass them our instance, such that they 
// have access to these components. Typically, they keep local copies of the 
// references they need.
//
// The processor runs its own thread. The thread logic is inherited from the 
// T64ThreadModule class. We need to implement the abstract methods of a module
// in which we do our processor specific work and invoke the protected methods
// of the thread class.
// 
//----------------------------------------------------------------------------------------
T64Processor::T64Processor( T64System           *sys,
                            int                 modNum,
                            T64Options          options,  
                            T64CpuType          cpuType,
                            T64TlbType          tlbType,
                            T64CacheType        cacheType ) : 

                            T64ThreadModule(    MT_PROC, 
                                                modNum,
                                                0,
                                                0 ) {

    this -> sys = sys;

    cpu     = new T64Cpu( this, cpuType );
    tlb     = new T64LocalTlb( this, T64_TK_UNIFIED_TLB, tlbType );
    
    cpu -> reset( );
    tlb -> reset( );
}

//----------------------------------------------------------------------------------------
// Destructor. We just do our work, the base classes will do theirs next.
//
//----------------------------------------------------------------------------------------
T64Processor:: ~T64Processor( ) {

    delete cpu;
    delete tlb;
}

//----------------------------------------------------------------------------------------
// Implement the module abstract methods for init, reset, halt and execute.
//
//----------------------------------------------------------------------------------------
void T64Processor::initModule( ) {

    cpu -> reset( );
    tlb -> reset( );
    
    threadModuleStart( );
}

void T64Processor::resetModule( ) {

    cpu -> reset( );
    tlb -> reset( );

    threadModuleReset( );
}

void T64Processor::haltModule( ) {

    threadModuleHalt( );
}

void T64Processor::runModule( ) {

    threadModuleExec( -1 );
}

void T64Processor::execModule( int units ) {

    threadModuleExec( units );
}

//----------------------------------------------------------------------------------------
// The thread class expect to call a routine that will execute a number of units.
// For a processor, the unit is an instruction step. The routine will also return
// a boolean value which indicates whether the thread required attention. For a
// processor this is a trap that was raised. The thread will then be placed in a
// "HALT" state for examination.
//
// ??? flesh this out a little. What exactly do we need to do ? 
// ??? explain how traps are handled when we don't need attention...
//
//----------------------------------------------------------------------------------------
bool T64Processor::executeUnit( ) {

    return( cpu -> executeInstr( ));
};

//----------------------------------------------------------------------------------------
// Little helpers.
//
//----------------------------------------------------------------------------------------
char *T64Processor::getProcStateStr( ) {

    return( getModuleStateStr( ));
}

T64Cpu *T64Processor::getCpuPtr( ) {

    return ( cpu );
}

T64LocalTlb *T64Processor::getTlbPtr( ) {

    return ( tlb );
}

//----------------------------------------------------------------------------------------
// We have a read request for the processor HPA address range. 
//
//----------------------------------------------------------------------------------------
bool T64Processor::handleHPARead( T64Word pAdr, uint8_t *data, int len ) {

    return ( false );
}

//----------------------------------------------------------------------------------------
// We have a write request for the processor HPA address range.
//
//----------------------------------------------------------------------------------------
bool T64Processor::handleHPAWrite( T64Word pAdr, uint8_t *data, int len ) {

    return ( false );
}

//----------------------------------------------------------------------------------------
// System Bus operations interface routines. The bus operations for read and 
// and write transactions are just passed through to the system layer. The bus
// operations for read and write events are the handlers a bus transaction that
// concerns our module. A processor still implements a HPA page in I/O memory
// and needs to reacts to read and write request.
//
// Finally, there are bus broadcasts. All modules are required to react to 
// a broadcast event. The processor will handle TLB events, such as TLB entry
// purge and so on.
//
//----------------------------------------------------------------------------------------
bool T64Processor::busOpRead( T64Word adr, 
                             uint8_t *data, 
                             int len ) {

    return( sys -> busOpRead( moduleNum, adr, data, len ));
}

bool T64Processor::busOpWrite( T64Word adr, 
                              uint8_t *data, 
                              int len ) {

    return( sys -> busOpWrite( moduleNum, adr, data, len ));
}

bool T64Processor::busOpReadEvent( int     reqModNum,
                                   T64Word pAdr, 
                                   uint8_t *data, 
                                   int     len ) {

    if ( reqModNum == moduleNum ) return( false );

    T64Processor *proc = (T64Processor *) sys -> lookupByAdr( pAdr );
    if ( proc == this ) {

        return( handleHPARead( pAdr, data, len ));
    }
    else return( false );
}

bool T64Processor::busOpWriteEvent( int     reqModNum,
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len ) {

    if ( reqModNum == moduleNum ) return( false );

    T64Processor *proc = (T64Processor *) sys -> lookupByAdr( pAdr );
    if ( proc == this ) {

        return( handleHPAWrite( pAdr, data, len ));
    }
    else  return( false );
}   

bool T64Processor::busOpBroadcastEvent( int                 srcModNum,
                                        T64BroadcastEvents  event, 
                                         T64Word            arg1, 
                                         T64Word            arg2 ) {

    switch ( event ) {

        case T64_BCAST_TLB_PURGE: {
            
            return( tlb -> purgeTlb( arg1 ));

        } break;

        // ??? need the LDR/STC mechanism ...

        default: ;
    }

    return( true );
}
