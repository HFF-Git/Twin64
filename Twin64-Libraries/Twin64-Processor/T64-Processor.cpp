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


// ???? rewrite to use thread module...

#if NEW_PROC == 0

//****************************************************************************************
//****************************************************************************************
//
// Processor. 
//
//----------------------------------------------------------------------------------------
// A processor is a module with one CPU, TLBs and Caches. We create the component
// objects right here and pass them our instance, such that they have access to 
// these components. Typically, they keep local copies of the references they need.
// 
//----------------------------------------------------------------------------------------
T64Processor::T64Processor( T64System           *sys,
                            int                 modNum,
                            T64Options          options,  
                            T64CpuType          cpuType,
                            T64TlbType          tlbType,
                            T64CacheType        cacheType,
                            T64Word             spaAdr,
                            int                 spaLen ) : 

                            T64Module(      MT_PROC, 
                                            modNum,
                                            spaAdr,
                                            spaLen ) {

    cpu     = new T64Cpu( this, cpuType );
    tlb     = new T64Tlb( this, T64_TK_UNIFIED_TLB, tlbType );

    this -> cpu -> reset( );
    this -> tlb -> reset( );

    this -> sys         = sys;
    this -> procState   = T64_PROC_STATE_HALTED;
}

//----------------------------------------------------------------------------------------
// Destructor.
//
//----------------------------------------------------------------------------------------
T64Processor:: ~T64Processor( ) {

    delete cpu;
    delete tlb;

    stopModule( );
}

//----------------------------------------------------------------------------------------
// Start and stop the processor thread. The processor thread is responsible for 
// executing instructions. The thread is created with the startModule method
// and stopped by notifying the thread to stop. In between start and stop the
// processor thread is signaled by the simulator console thread what to do next. T
//
//----------------------------------------------------------------------------------------
void T64Processor::startModule( ) {

    worker = std::thread( &T64Processor::processorThread, this );

    threadId = static_cast<uint32_t>(

        std::hash<std::thread::id>{}(worker.get_id( ))
    );
}

void T64Processor::stopModule( ) {

    procState.store( T64_PROC_STATE_TERMINATE, std::memory_order_release );
    procCondVar.notify_one( );

    if ( worker.joinable( )) worker.join();
}

//----------------------------------------------------------------------------------------
// Processor state routines. This is our way to control what the processor is 
// doing. We provide methods for RESET, HALT and EXECUTE. The processor state is
// the atomic variable "procState". The mutex ensures that we do a synchronized
// update. Finally, we wake up the thread which is waiting in the "procCondVar".
//
//----------------------------------------------------------------------------------------
void T64Processor::setProcessorState( T64ProcState state ) {

    {
        std::lock_guard<std::mutex> lk(procLock);
        procState = state;
    }

    procCondVar.notify_one( );
}

void T64Processor::resetModule( ) {

    setProcessorState( T64_PROC_STATE_RESET );
}

void T64Processor::haltModule( ) {

    setProcessorState( T64_PROC_STATE_HALTED );
}

void T64Processor::execModule( int units ) {

    instrCount = units;
    setProcessorState( T64_PROC_STATE_EXECUTE );
}

//----------------------------------------------------------------------------------------
// The processor thread. The processor thread is responsible for executing T64
// instructions. If the processor is on HALT state, we wait on the "procCondVar"
// variable. Our mutex ensures synchronized access. If the thread was awoken
// we continue the main loop, where we get as first thing the new state, so we
// know what we should do.
//
// Think of it like a CPU:
//
//      procState = control register
//      notify_one() = interrupt
//      wait() = halt instruction
//      loop = fetch-decode-execute
//
//----------------------------------------------------------------------------------------
void T64Processor::processorThread( ) {
    
    procState.store( T64_PROC_STATE_RESET, std::memory_order_release );

    while ( true ) {

        T64ProcState s = procState.load( std::memory_order_acquire );

        if ( s == T64_PROC_STATE_HALTED ) {

            std::unique_lock<std::mutex> lk(procLock);

            procCondVar.wait(lk, [this] {

                return procState.load(std::memory_order_acquire)

                       != T64_PROC_STATE_HALTED;

            });

            continue;
        }
        
        switch ( s ) {

            case T64_PROC_STATE_RESET: {

                cpu -> reset( );
                tlb -> reset( );

                procState.store( T64_PROC_STATE_HALTED, 
                                 std::memory_order_release );

                procCondVar.notify_one( );

            } break;

            case T64_PROC_STATE_EXECUTE: {

                while (true) {

                    if ( procState.load( std::memory_order_acquire ) != 
                            T64_PROC_STATE_EXECUTE ) break;

                    if ( instrCount == 0 ) break;

                    try {

                        cpu -> executeInstr( );
                    }
                    catch ( const T64Trap ) {

                        bool haltOnCpuTrap = false; // Fix, where do we get it from ?

                        if ( haltOnCpuTrap ) {

                            procState.store( T64_PROC_STATE_TRAP, 
                                             std::memory_order_release);

                            procCondVar.notify_one( );
                            break;
                        }
                    }

                    if ( instrCount > 0 ) instrCount --;
                }

                procState.store( T64_PROC_STATE_HALTED, 
                                 std::memory_order_release );
                procCondVar.notify_one( );

            } break;

            case T64_PROC_STATE_TRAP:
            case T64_PROC_STATE_HALTED: {

                std::unique_lock<std::mutex> lk(procLock);
                procCondVar.wait(lk, [this] {
                return  procState != T64_PROC_STATE_TRAP &&
                        procState != T64_PROC_STATE_HALTED;
                });

            } break;

            case T64_PROC_STATE_TERMINATE: 
            default: {

                return;
            }
        }
    }
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
//
//----------------------------------------------------------------------------------------
bool T64Processor::handleHPAWrite( T64Word pAdr, uint8_t *data, int len ) {

    return ( false );
}

//----------------------------------------------------------------------------------------
// System Bus operations interface routines.
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

        case T64_BCAST_TLB_INSERT: {

            return( tlb -> insertTlb( arg1, arg2 ));

        } break;

        case T64_BCAST_TLB_PURGE: {
            
            return( tlb -> purgeTlb( arg1 ));

        } break;
    }

    return( true );
}

//----------------------------------------------------------------------------------------
// Get the reference to the processor components.
//
//----------------------------------------------------------------------------------------
T64Cpu *T64Processor::getCpuPtr( ) {

    return ( cpu );
}

T64Tlb *T64Processor::getTlbPtr( ) {

    return ( tlb );
}

char *T64Processor::getProcStateStr( ) {

    switch( procState ) {

        case T64_PROC_STATE_NIL:            return ((char *) "NIL" );
        case T64_PROC_STATE_RESET:          return ((char *) "RESET" );
        case T64_PROC_STATE_EXECUTE:        return ((char *) "RUN");
        case T64_PROC_STATE_HALTED:         return ((char *) "HALT" );
        case T64_PROC_STATE_TRAP:           return ((char *) "TTRAP" );
        case T64_PROC_STATE_TERMINATE:      return ((char *) "EXIT" );
    }
}
    
#else






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
                            T64CacheType        cacheType,
                            T64Word             spaAdr,
                            int                 spaLen ) : 

                            T64ThreadModule(    MT_PROC, 
                                                modNum,
                                                spaAdr,
                                                spaLen ) {

    cpu     = new T64Cpu( this, cpuType );
    tlb     = new T64Tlb( this, T64_TK_UNIFIED_TLB, tlbType );

    this -> sys = sys;
    this -> cpu -> reset( );
    this -> tlb -> reset( );
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
// Implement the module abstract methods for reset, halt and execute.
//
//----------------------------------------------------------------------------------------
void T64Processor::resetModule( ) {

    cpu -> reset( );
    tlb -> reset( );

    threadModuleReset( );
}

void T64Processor::haltModule( ) {

    threadModuleHalt( );
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

T64Tlb *T64Processor::getTlbPtr( ) {

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

        case T64_BCAST_TLB_INSERT: {

            return( tlb -> insertTlb( arg1, arg2 ));

        } break;

        case T64_BCAST_TLB_PURGE: {
            
            return( tlb -> purgeTlb( arg1 ));

        } break;
    }

    return( true );
}


#endif
