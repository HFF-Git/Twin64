//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Processor Module
//
//----------------------------------------------------------------------------------------
// The processor object is a module for the T64 System. It consist of the CPU 
// and the TLB subsystems.
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
// Threading Notes.
//
// A processor could become a thread of execution implemented with C++ threads. 
// This way we can exploit a multi-core host CPU. The processor thread would 
// then be responsible for executing the instructions. 
//
// The processor thread interacts with the system bus, which is essentially a
// shared resource. The system bus will route read and write request to the 
// responsible module that is responsible for the address. A module, including
// the processor, can also be responsible for an address range. In this case, 
// the busOpEvent routines of the module are called to handle the bus event. 
//
// Over time the simulator will just be the launcher of the system. A processor
// will simply just run execution. We could think however think about ways to
// intercept traps and machine checks for analysis and display purposes.
//
// A processor thread is still an object that is created and managed by the 
// simulator. The processor thread is not a thread that is created by the 
// processor object itself, threads are created by the simulator main thread.
// Care needs to be taken with access to these object.
//
// Processor states: Running, Stopped, Stepping, Halted, Resetting.
//
// If there more than one processor, there needs to be a selection process for
// the monarch, which will bring up the system.  






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

    this -> modNum  = modNum;
    this -> sys     = sys;

    cpu     = new T64Cpu( this, cpuType );
    tlb     = new T64Tlb( this, T64_TK_UNIFIED_TLB, tlbType );

    this -> cpu -> reset( );
    this -> tlb -> reset( );

    instructionCount    = 0;

    this -> procState = T64_PROC_STATE_HALTED;

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
// doing. We provide methods for RESET, HALT and STEP.
//
//----------------------------------------------------------------------------------------
void T64Processor::setProcessorState( T64ProcState state ) {

    this -> procState = state;
    procCondVar.notify_one( );
}

void T64Processor::resetModule( ) {

    procState = T64_PROC_STATE_RESET;
    procCondVar.notify_one( );
}

void T64Processor::haltModule( ) {

    procState = T64_PROC_STATE_HALTED;
    procCondVar.notify_one( );
}

void T64Processor::stepModule( ) {

    procState = T64_PROC_STATE_SINGLE_STEP;
    procCondVar.notify_one( );
}

//----------------------------------------------------------------------------------------
// The processor thread. The processor thread is responsible for executing T64
// instructions. There is a state variable that is used to signal the processor
// thread what to do next.
//
// Mental model:
//
//      procState           -> control register
//      signal( )           -> writing to control register
//      cv.notify_one( )    -> raising a wake-up signal
//      wait( )             -> CPU entering low-power halt
//
//
// ??? we start by resetting the processor, and then we wait for the next signal.
// The processor thread is responsible for executing instructions, but it is also
// responsible for handling signals and managing its state.
//
//----------------------------------------------------------------------------------------
void T64Processor::processorThread( ) {

    procState.store( T64_PROC_STATE_RESET, std::memory_order_release );

    while ( true ) {
        
        T64ProcState s = procState.load( std::memory_order_acquire );

        switch ( s ) {

            case T64_PROC_STATE_RESET: {

                cpu -> reset( );
                tlb -> reset( );

                procState.store( T64_PROC_STATE_HALTED, 
                                 std::memory_order_release);

            } break;

            case T64_PROC_STATE_RUNNING: {

                while ( procState.load(std::memory_order_acquire) == 
                                                T64_PROC_STATE_RUNNING ) {

                    try {

                        cpu -> executeInstr( );
                    }
                    catch (const T64Trap& t) {

                    }
                }

            } break;

            case T64_PROC_STATE_SINGLE_STEP: {

                try {
        
                    cpu -> executeInstr( );
                }
    
                catch ( const T64Trap t ) {
        
                }

                procState.store( T64_PROC_STATE_HALTED, 
                    std::memory_order_release);

            } break;

            case T64_PROC_STATE_HALTED: {

                std::unique_lock<std::mutex> lk( procLock );
                procCondVar.wait( lk, [ this ]{ 

                    return ( procState.load( 
                        std::memory_order_acquire ) != T64_PROC_STATE_HALTED ); 
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

bool T64Processor::busOpBroadcastEvent( int     srcModNum,
                                         int     id, 
                                         T64Word arg1, 
                                         T64Word arg2 ) {

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
        case T64_PROC_STATE_HALTED:         return ((char *) "HALT" );
        case T64_PROC_STATE_RUNNING:        return ((char *) "RUN" );
        case T64_PROC_STATE_SINGLE_STEP:    return ((char *) "STEP" );
        case T64_PROC_STATE_TERMINATE:      return ((char *) "EXIT" );
    }
}
    
