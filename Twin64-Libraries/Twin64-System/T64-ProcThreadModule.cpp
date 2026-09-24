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
T64ProcThreadModule::T64ProcThreadModule( T64System        *sys,
                                          T64ModuleType    modType, 
                                          int              modNum,
                                          T64Word          spaAdr,
                                          int              spaLen ) 
                                          : T64Module ( sys,
                                                        modType, 
                                                        modNum,
                                                        spaAdr, 
                                                        spaLen ) { 

    mTrapCode = NO_TRAP;
    moduleState.store( T64_MOD_STATE_HALTED, std::memory_order_release );      
}

T64ProcThreadModule:: ~ T64ProcThreadModule( ) {

    mTrapCode = NO_TRAP;
    moduleState.store( T64_MOD_STATE_TERMINATE, std::memory_order_release );
 //   mCondVar.notify_one( );

    if ( mWorker.joinable( )) mWorker.join();
}

//----------------------------------------------------------------------------------------
// Module state routines. This is our way to control what the module thread is 
// doing. We provide methods for RESET, HALT and EXECUTE. The module state is
// the atomic variable "mState". The mutex ensures that we do a synchronized
// update. Finally, we wake up the thread which is waiting in the "mCondVar".
//
//----------------------------------------------------------------------------------------
void T64ProcThreadModule::setModuleState( T64ModuleState state ) {

    {
        std::lock_guard<std::mutex> lk(mLock);
        moduleState = state;
    }

    mCondVar.notify_one( );
}

void T64ProcThreadModule::initModule( ) {

    mWorker = std::thread( &T64ProcThreadModule::moduleWorker, this );
}

void T64ProcThreadModule::resetModule( ) {

    mUnitCount = 0;
    setModuleState( T64_MOD_STATE_RESET );
}

void T64ProcThreadModule::haltModule( ) {

    mUnitCount = 0;
    setModuleState( T64_MOD_STATE_HALTED );
}

void T64ProcThreadModule::runModule( ) {

    mUnitCount = -1;
    setModuleState( T64_MOD_STATE_EXECUTE );
}

void T64ProcThreadModule::execModule( int units, bool haltOnTrap ) {

    if ( units < -1 ) units = -1;

    mUnitCount      = units;
    enterSimOnTrap  = haltOnTrap;

    setModuleState( T64_MOD_STATE_EXECUTE );
}

//----------------------------------------------------------------------------------------
// Support for LDR/STC instructions.
//
//----------------------------------------------------------------------------------------
void T64ProcThreadModule::setRsvInfo( T64Word pAdr, bool valid ) {

    rsvInfo  = pAdr;
    rsvValid = valid;
}
    
T64Word T64ProcThreadModule::getRsvAdr( ) {

    return( rsvInfo );
}

bool T64ProcThreadModule::isRsvValid( ) {

    return( rsvValid );
}

//----------------------------------------------------------------------------------------
// A little helper to return the module state and trap code.
//
//----------------------------------------------------------------------------------------
T64TrapCode T64ProcThreadModule::getTrapCode( ) {

    return( mTrapCode );
}

 void T64ProcThreadModule::setEnterSimOnTrap( bool val ) {

    enterSimOnTrap = val;
 }

//----------------------------------------------------------------------------------------
// The module thread worker routine. The module is the class for processors.
//
// If the thread is in HALT state, we wait on the "mCondVar" variable. Our mutex
// ensures synchronized access. If the thread was awoken we continue the main 
// loop, where we get as first thing the new state, so we know what we should do.
//
// Think of it like a CPU:
//
//      procState = control register
//      notify_one() = interrupt
//      wait() = halt instruction
//      loop = fetch-decode-execute
//
//----------------------------------------------------------------------------------------
void T64ProcThreadModule::moduleWorker( ) {
 
    moduleState.store( T64_MOD_STATE_RESET, std::memory_order_release );

    while ( true ) {

        T64ModuleState s = moduleState.load( std::memory_order_acquire );

        if ( s == T64_MOD_STATE_HALTED ) {

            std::unique_lock<std::mutex> lk( mLock );

            mCondVar.wait( lk, [this] {

                return( moduleState.load(std::memory_order_acquire) != 
                            T64_MOD_STATE_HALTED );
            });

            continue;
        }
        
        switch ( s ) {

            case T64_MOD_STATE_RESET: {

               // resetModule( );

                mTrapCode = NO_TRAP;
                moduleState.store( T64_MOD_STATE_HALTED, 
                                 std::memory_order_release );

                mCondVar.notify_one( );

            } break;

            case T64_MOD_STATE_EXECUTE: {

                while (true) {

                    // Has this module been stopped?
                    if ( moduleState.load( std::memory_order_acquire )
                            != T64_MOD_STATE_EXECUTE) {
                        break;
                    }

                    // Has this module completed its requested execution?
                    if ( mUnitCount == 0 ) {

                        mTrapCode = NO_TRAP;

                        moduleState.store( T64_MOD_STATE_HALTED,
                                           std::memory_order_release) ;

                        sys->moduleRunComplete( );
                        break;
                    }

                    // Has the entire simulated system been stopped?
                    if ( sys -> getSystemState( ) == T64_SYS_STATE_HALT ) {

                        moduleState.store( T64_MOD_STATE_HALTED,
                                           std::memory_order_release );

                        sys->moduleRunComplete( );
                        break;
                    }

                    // Execute one unit.
                    mTrapCode = executeUnit( );

                    // A trap stops the entire simulated system.
                    if (( mTrapCode != NO_TRAP)  && enterSimOnTrap ) {

                        moduleState.store( T64_MOD_STATE_HALTED,
                                           std::memory_order_release) ;

                        sys->simHalt( );
                        sys->moduleRunComplete( );
                        break;
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