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
T64ThreadModule::T64ThreadModule( T64ModuleType    modType, 
                                  int              modNum,
                                  T64Word          spaAdr,
                                  int              spaLen ) 
                                  : T64Module ( modType, 
                                                modNum,
                                                spaAdr, 
                                                spaLen ) { 

    mState.store( T64_MOD_STATE_HALTED, std::memory_order_release );
                
}

T64ThreadModule:: ~ T64ThreadModule( ) {

    mState.store( T64_MOD_STATE_TERMINATE, std::memory_order_release );
    mCondVar.notify_one( );

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
        mState = state;
    }

    mCondVar.notify_one( );
}

void T64ThreadModule::initModule( ) {

    mWorker = std::thread( &T64ThreadModule::moduleWorker, this );
}

void T64ThreadModule::resetModule( ) {

    setModuleState( T64_MOD_STATE_RESET );
}

void T64ThreadModule::haltModule( ) {

    setModuleState( T64_MOD_STATE_HALTED );
}

void T64ThreadModule::runModule( ) {

    mUnitCount = -1;
    setModuleState( T64_MOD_STATE_EXECUTE );
}

void T64ThreadModule::execModule( int units ) {

    mUnitCount = units;
    setModuleState( T64_MOD_STATE_EXECUTE );
}

//----------------------------------------------------------------------------------------
// Wait for the thread to complete execution. When the processor goes into the
// HALT state, the waiting thread is awoken.
//
//----------------------------------------------------------------------------------------
void T64ThreadModule::waitUntilHalted( ) {

    std::unique_lock<std::mutex> lk(mLock);

    mCondVar.wait(lk, [this] {

        return mState.load() == T64_MOD_STATE_HALTED;
    });
}

//----------------------------------------------------------------------------------------
// A little helper to return a string version of the module state.
//
//----------------------------------------------------------------------------------------
char *T64ThreadModule::getModuleStateStr( ) {

    T64ModuleState s = mState.load( std::memory_order_acquire );

    switch( s ) {

        case T64_MOD_STATE_NIL:            return ((char *) "NIL" );
        case T64_MOD_STATE_RESET:          return ((char *) "RESET" );
        case T64_MOD_STATE_EXECUTE:        return ((char *) "RUN");
        case T64_MOD_STATE_HALTED:         return ((char *) "HALT" );
        case T64_MOD_STATE_TRAP:           return ((char *) "TRAP" );
        case T64_MOD_STATE_TERMINATE:      return ((char *) "EXIT" );
    }
}

//----------------------------------------------------------------------------------------
// The module thread worker routine. The module is the base class for processors
// and I/O modules. 
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
void T64ThreadModule::moduleWorker( ) {
    
    mState.store( T64_MOD_STATE_RESET, std::memory_order_release );

    while ( true ) {

        T64ModuleState s = mState.load( std::memory_order_acquire );

        if ( s == T64_MOD_STATE_HALTED ) {

            std::unique_lock<std::mutex> lk(mLock);

            mCondVar.wait(lk, [this] {

                return mState.load(std::memory_order_acquire)
                                                != T64_MOD_STATE_HALTED;

            });

            continue;
        }
        
        switch ( s ) {

            case T64_MOD_STATE_RESET: {

                resetModule( );

                mState.store( T64_MOD_STATE_HALTED, 
                                 std::memory_order_release );

                mCondVar.notify_one( );

            } break;

            case T64_MOD_STATE_EXECUTE: {

                while ( true ) {

                    if ( mState.load( std::memory_order_acquire )
                            != T64_MOD_STATE_EXECUTE ) {

                        break;
                    }

                    if ( mUnitCount == 0 ) {

                        mState.store( T64_MOD_STATE_HALTED,
                                      std::memory_order_release );

                        break;
                    }

                    bool needAttention = executeUnit();

                    if ( needAttention ) {

                        mState.store(T64_MOD_STATE_TRAP,
                                    std::memory_order_release);

                        break;
                    }

                    if ( mUnitCount > 0 ) mUnitCount--;
                }

                mCondVar.notify_one();

            } break;

            case T64_MOD_STATE_TRAP:
            case T64_MOD_STATE_HALTED: {

                std::unique_lock<std::mutex> lk(mLock);
                mCondVar.wait(lk, [this] {

                    T64ModuleState s = mState.load(std::memory_order_acquire);

                    return (( s != T64_MOD_STATE_TRAP ) &&
                            ( s != T64_MOD_STATE_HALTED ));
                });

            } break;

            case T64_MOD_STATE_TERMINATE: return;

            default: return;
        }
    }
}

