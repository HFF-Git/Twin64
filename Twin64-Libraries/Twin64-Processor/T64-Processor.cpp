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
// Interaction with other modules is typically performed via the T64System object.
// However, there is one exception. A reference to the global TLB is directly 
// obtained for performance reasons. It is expected that there is only one global
// TLB and that is created before we create processors.
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

    cpu       = new T64Cpu( this, cpuType );
    localTlb  = new T64LocalTlb( this, T64_TK_UNIFIED_TLB, tlbType );
    globalTlb = dynamic_cast<T64GlobalTlb*>( sys -> lookupByModuleType( MT_GTLB ));
    
    cpu -> reset( );
    localTlb -> reset( );
}

//----------------------------------------------------------------------------------------
// Destructor. We just do our work, the base classes will do theirs next.
//
//----------------------------------------------------------------------------------------
T64Processor:: ~T64Processor( ) {

    delete cpu;
    delete localTlb;
}

//----------------------------------------------------------------------------------------
// Implement the module abstract methods for init, reset, halt and execute.
//
//----------------------------------------------------------------------------------------
void T64Processor::initModule( ) {

    cpu -> reset( );
    localTlb -> reset( );
    
    threadModuleStart( );
}

void T64Processor::resetModule( ) {

    cpu -> reset( );
    localTlb -> reset( );
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

T64LocalTlb *T64Processor::getLocalTlbPtr( ) {

    return ( localTlb );
}

T64GlobalTlb *T64Processor::getGlobalTlbPtr( ) {

    return( globalTlb );
}

//----------------------------------------------------------------------------------------
// We have a read request for the processor HPA address range. 
//
// The data is stored in 64-bit registers. The data is big endian in our CPU
// architecture. However, the data stored in a variable is potentially little
// endian on our simulator environment. We need to first correct the endianess 
// and then return the requested number of bytes at the offset in the register.
//
// ??? document what is covered in the reg sets.
// ??? should we define for the common IO Regs routines at the module level ?
//
//----------------------------------------------------------------------------------------
bool T64Processor::handleHPARead( T64Word pAdr, uint8_t *data, int len ) {

    T64Word hpaAdr  =  T64_IO_HPA_MEM_START + moduleNum * T64_PAGE_SIZE_BYTES;
    int     wordIndex           = (( pAdr - hpaAdr ) >> 3 );
    int     regSetIndex         = wordIndex / T64_IO_REG_SET_SIZE;
    int     wordInRegSetIndex   = wordIndex % T64_IO_REG_SET_SIZE;
    int     wordOfs             = pAdr % sizeof( T64Word );
    T64Word tmp                 = 0;
    
    if ( regSetIndex == 0 ) {

        switch ( wordInRegSetIndex ) {
 
            case T64_IO_STATUS_REG_OFS: {

                tmp = 0xaaaabbbbccccdddd;  // ??? test only ...

                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;

            case T64_IO_COMMAND_REG_OFS: {

                tmp = 0x55;  // ??? test only ...

                copyFromReg( data, tmp, wordOfs, len );
                return ( true );

            } break;

            case T64_IO_CONFIG_REG_OFS: {

                tmp = 66; // ??? test only ...
                copyFromReg( data, tmp, wordOfs, len );
                return ( true );

            } break;

            case T64_IO_SPA_ADR_REG_OFS: {

                copyFromReg( data, 0, wordOfs, len );
                return ( true );

            } break;

            case T64_IO_TLB_STATUS_REG_OFS: {

                tmp = localTlb -> getTlbStatus( );
                copyFromReg( data, tmp, wordOfs, len );
                return ( true );

            } break;
                
            case T64_IO_TLB_CONFIG_REG_OFS: {

                tmp = localTlb -> getTlbConfig( );
                copyFromReg( data, tmp, wordOfs, len );
                return ( true );

            } break;

            case T64_IO_ITLB_HITS_OFS: {

                tmp = localTlb -> getItlbHits();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;

            case T64_IO_ITLB_MISSES_OFS: {

                tmp = localTlb -> getItlbMisses();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;

            case T64_IO_ITLB_GTLB_HITS_OFS: {

                tmp = localTlb -> getItlbMissGTlbHits();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );
            
            } break;  
    
            case T64_IO_ITLB_GTLB_MISSES_OFS: {
                
                tmp = localTlb -> getItlbMissGTlbMisses();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;                

            case T64_IO_DTLB_HITS_OFS: { 

                tmp = localTlb -> getDtlbHits();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;

            case T64_IO_DTLB_MISSES_OFS: {

                tmp = localTlb -> getDtlbMisses();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;

            case T64_IO_DTLB_GTLB_HITS_OFS: {

                tmp = localTlb -> getDtlbMissGTlbHits();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;

            case T64_IO_DTLB_GTLB_MISSES_OFS: {

                tmp = localTlb -> getDtlbMissGTlbMisses();
                copyFromReg( data, tmp, wordOfs, len );
                return( true );

            } break;

            default: {

                copyFromReg( data, tmp, 0, len );
                return ( false );
            }
        }
    }
    else if ( regSetIndex == 1 ) {
    
        T64TlbEntry *e = nullptr;

        if ( wordInRegSetIndex >= 16 ) {

            e = localTlb -> getDTlbEntry(( wordInRegSetIndex - 16 ) / 2 );
            if ( e == nullptr ) return( false );
        }
        else {

             e = localTlb -> getITlbEntry( wordInRegSetIndex / 2 );
             if ( e == nullptr ) return( false );
        }

        if ( wordInRegSetIndex % 2 == 0 ) {

            copyFromReg( data, e -> vAdr, wordOfs, len );
            return( true );
        }
        else {

            T64Word tmp = ((T64Word) e -> tlbInfo << 48 ) | ( e -> pAdr );
            copyFromReg( data, tmp, wordOfs, len );
            return( true );
        }
    }
    else return ( false );
}

//----------------------------------------------------------------------------------------
// We have a write request for the processor HPA address range.
//
//----------------------------------------------------------------------------------------
bool T64Processor::handleHPAWrite( T64Word pAdr, uint8_t *data, int len ) {

    T64Word hpaAdr      =  T64_IO_HPA_MEM_START + moduleNum * T64_PAGE_SIZE_BYTES;
    int     wordIndex   = (( pAdr - hpaAdr ) >> 3 );
    int     regSetIndex = wordIndex / T64_IO_REG_SET_SIZE;
    int     wordOfs     = pAdr % sizeof( T64Word );
    T64Word tmp         = 0;

    // ??? what do we cover here ?

    return ( false );
}

//----------------------------------------------------------------------------------------
// We have a broadcast event.
//
//----------------------------------------------------------------------------------------
bool T64Processor::handleHPABroadcast( T64BroadcastEvents event, 
                                       T64Word arg1, 
                                       T64Word arg2 ) {

    switch ( event ) {

        case T64_BCAST_TLB_PURGE: {
            
            return( localTlb -> purgeTlb( arg1 ));

        } break;

        case T64_BCAST_MODULE_PURGE: {

            // ??? get the module number...
            // ??? has the global TLB been purged ?

        } break;

        default: ;
    }

    return( true );
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

bool T64Processor::busOpReadRsv( T64Word adr, 
                              uint8_t *data, 
                              int len ) {

    return( sys -> busOpReadRsv( moduleNum, adr, data, len ));
}

bool T64Processor::busOpWrite( T64Word adr, 
                               uint8_t *data, 
                               int len ) {

    return( sys -> busOpWrite( moduleNum, adr, data, len ));
}

bool T64Processor::busOpWriteCond( T64Word adr, 
                               uint8_t *data, 
                               int len ) {

    return( sys -> busOpWriteCond( moduleNum, adr, data, len ));
}

bool T64Processor::busOpBroadCast( T64BroadcastEvents id, 
                                   T64Word            arg1, 
                                   T64Word            arg2 ) {

    return( sys -> busOpBroadcast( moduleNum, id, arg1, arg2 ));
}

bool T64Processor::busOpReadEvent( int     reqModNum,
                                   T64Word pAdr, 
                                   uint8_t *data, 
                                   int     len ) {

    if ( reqModNum == moduleNum ) return( false );
    return( handleHPARead( pAdr, data, len ));
}

bool T64Processor::busOpWriteEvent( int     reqModNum,
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len ) {

    if ( reqModNum == moduleNum ) return( false );
    return( handleHPAWrite( pAdr, data, len ));
}   

bool T64Processor::busOpBroadcastEvent( int                 reqModNum,
                                        T64BroadcastEvents  event, 
                                        T64Word             arg1, 
                                        T64Word             arg2 ) {

    if ( reqModNum == moduleNum ) return( false );
    return( handleHPABroadcast( event, arg1, arg2 ));
}
