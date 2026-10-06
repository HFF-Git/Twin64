//----------------------------------------------------------------------------------------
//
// Twin-64 - Module Threads
//
//----------------------------------------------------------------------------------------
// The T64-System represent the system consisting of several modules. One class
// of modules are the ones that run in a thread. This class implements the C++
// threads for them. An inheriting module is required to implement the execution
// unit methods and call the thread reset, halt and execute methods in its 
// implementation of these functions.
//
//----------------------------------------------------------------------------------------
//
//  Twin-64 - System
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
// The object creator. We pass the common module data to the base class.
//
//
//----------------------------------------------------------------------------------------
T64ThreadModule::T64ThreadModule( T64System        *sys,
                                  T64ModuleType    modType, 
                                  int              modNum,
                                  T64Word          spaAdr,
                                  unsigned         spaLen ) 
                                  : T64Module ( sys,
                                                modType, 
                                                modNum,
                                                spaAdr, 
                                                spaLen ) { 

    mTrapCodeOnUnitExec = NO_TRAP;
    mTrapAdrOnUnitExec = 0;

    moduleState.store( T64_MOD_STATE_HALTED, std::memory_order_release );      
}

T64ThreadModule:: ~ T64ThreadModule( ) {

    mTrapCodeOnUnitExec = NO_TRAP;
    mTrapAdrOnUnitExec = 0;
    moduleState.store( T64_MOD_STATE_TERMINATE, std::memory_order_release );

    if ( mWorker.joinable( )) mWorker.join();
}

//----------------------------------------------------------------------------------------
// Module state routines. This is our way to control what the module thread is 
// doing. We provide methods for RESET, HALT and EXECUTE. The module state is
// the atomic variable "mState". The mutex ensures that we do a synchronized
// update. Finally, we wake up the thread which is waiting in the "mCondVar".
//
//----------------------------------------------------------------------------------------
void T64ThreadModule::setModuleState( T64ModuleState state ) {

    {
        std::lock_guard<std::mutex> lk(mLock);
        moduleState = state;
    }

    mCondVar.notify_one( );
}

void T64ThreadModule::initModule( ) {

    mWorker = std::thread( &T64ThreadModule::moduleWorker, this );
}

void T64ThreadModule::resetModule( ) {

    mUnitCount = 0;
    setModuleState( T64_MOD_STATE_RESET );
}

void T64ThreadModule::execModule( int units, bool haltOnTrap ) {

    if ( units < -1 ) units = -1;

    mUnitCount         = units;
    enterSimOnT64Trap  = haltOnTrap;

    setModuleState( T64_MOD_STATE_EXECUTE );
}

//----------------------------------------------------------------------------------------
// Support for LDR/STC instructions.
//
//----------------------------------------------------------------------------------------
void T64ThreadModule::setRsvInfo( T64Word pAdr, bool valid ) {

    rsvInfo  = pAdr;
    rsvValid = valid;
}
    
T64Word T64ThreadModule::getRsvAdr( ) {

    return( rsvInfo );
}

bool T64ThreadModule::isRsvValid( ) {

    return( rsvValid );
}

//----------------------------------------------------------------------------------------
// A little helper to return the module state and trap code.
//
//----------------------------------------------------------------------------------------
T64TrapCode T64ThreadModule::getTrapCode( ) {

    return( mTrapCodeOnUnitExec );
}

T64Word T64ThreadModule::getTrapAdr( ) {

    return( mTrapAdrOnUnitExec );
}

 void T64ThreadModule::setEnterSimOnTrap( bool arg ) {

    enterSimOnT64Trap = arg;
 }

//----------------------------------------------------------------------------------------
// The module thread worker routine. The module is the class for processors.
//
// The worker is essentially the processor's execution loop. The module state 
// acts like a control register: HALTED waits for a state change, while EXECUTE
// repeatedly executes one unit and handles traps, simulator breakpoints, and 
// execution limits. Code breakpoints are recognized when an instruction 
// completes and establishes the breakpoint address as the new IA; data 
// breakpoints are detected by the bus operations.
//
// Think of it like a CPU:
//
//      procState = control register
//      notify_one() = interrupt
//      wait() = halt instruction
//      loop = fetch-decode-execute
//
//----------------------------------------------------------------------------------------
void T64ThreadModule::moduleWorker( ) {
 
    moduleState.store( T64_MOD_STATE_RESET, std::memory_order_release );

    while ( true ) {

        T64ModuleState s = moduleState.load( std::memory_order_acquire );

        if ( s == T64_MOD_STATE_HALTED ) {

            std::unique_lock<std::mutex> lk( mLock );

            mCondVar.wait( lk, [this] {

                return( moduleState.load( std::memory_order_acquire) != 
                            T64_MOD_STATE_HALTED );
            });

            continue;
        }
        
        switch ( s ) {

            case T64_MOD_STATE_RESET: {

                mUnitCount  = 0;
                mTrapCodeOnUnitExec = NO_TRAP;
                mTrapAdrOnUnitExec  = 0;
                moduleState.store( T64_MOD_STATE_HALTED, 
                                   std::memory_order_release );

                mCondVar.notify_one( );

            } break;

            case T64_MOD_STATE_EXECUTE: {

                while (true) {

                    if ( sys -> getSystemState( ) == T64_SYS_STATE_HALT ) {

                        moduleState.store( T64_MOD_STATE_HALTED,
                                           std::memory_order_release );

                        mUnitCount = 0;
                        sys -> moduleRunComplete( );
                        break;
                    }

                    if ( moduleState.load( std::memory_order_acquire )
                            != T64_MOD_STATE_EXECUTE) {

                        mUnitCount = 0;
                        break;
                    }

                    if ( mUnitCount == 0 ) {

                        mTrapCodeOnUnitExec = NO_TRAP;
                        mTrapAdrOnUnitExec  = 0;

                        moduleState.store( T64_MOD_STATE_HALTED,
                                           std::memory_order_release ) ;

                        sys -> moduleRunComplete( );
                        break;
                    }

                    mTrapCodeOnUnitExec = executeUnit( );

                    if ( mTrapCodeOnUnitExec == NO_TRAP ) {

                        T64Word ia = mTrapNextInstAdr;

                        if ( sys -> checkBreakPoint( T64_SIM_BREAK_X,
                                                    ia,
                                                    getModuleNum( )) != -1 ) {

                            mTrapCodeOnUnitExec = SIM_BRK_TRAP_X;
                            mTrapAdrOnUnitExec  = ia;

                            sys -> breakPointOccurred( getModuleNum( ));
                        }
                    }

                    if  ( mTrapCodeOnUnitExec != NO_TRAP ) {

                         if (( mTrapCodeOnUnitExec == MACHINE_CHECK  ) ||
                             ( mTrapCodeOnUnitExec == SIM_BRK_TRAP_X ) ||
                             ( mTrapCodeOnUnitExec == SIM_BRK_TRAP_R ) ||
                             ( mTrapCodeOnUnitExec == SIM_BRK_TRAP_W )) {

                            moduleState.store( T64_MOD_STATE_HALTED,
                                           std::memory_order_release) ;

                            sys -> simHalt( -1 );
                            sys -> moduleRunComplete( );
                            break;
                         }

                         if ( enterSimOnT64Trap ) {

                            moduleState.store( T64_MOD_STATE_HALTED,
                                           std::memory_order_release) ;

                            sys -> simHalt( -1 );
                            sys -> moduleRunComplete( );
                            break;
                         }
                    }

                    if ( mUnitCount > 0 ) mUnitCount--;
                }

                mCondVar.notify_one();

            } break;

            case T64_MOD_STATE_TERMINATE: return;

            default: return;
        }
    }
}