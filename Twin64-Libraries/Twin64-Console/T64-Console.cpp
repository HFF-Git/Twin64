//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Console Module
//
//----------------------------------------------------------------------------------------
// The console module represents the system console. The simulator switches 
// terminal control to this module when the RUN command is issued. 
//
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
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with 
// this program. If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-Console.h"

//----------------------------------------------------------------------------------------
// Name space for local routines.
//
//----------------------------------------------------------------------------------------
namespace {

};

//****************************************************************************************
//****************************************************************************************
//
// Console Module Object.
//
//----------------------------------------------------------------------------------------
// The console module represents the console terminal for the T64 system.
//
// The console runs its own thread. The thread logic is inherited from the 
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
T64Console::T64Console( T64System           *sys,
                        int                 modNum,
                        T64ConsoleOptions   options 

                      ) : T64ThreadModule( sys,
                                           T64_MOD_TYPE_CONSOLE, 
                                           modNum,
                                           0,
                                           0 ) {

    this -> sys = sys;
}

//----------------------------------------------------------------------------------------
// Destructor. We just do our work, the base classes will do theirs next.
//
//----------------------------------------------------------------------------------------
T64Console:: ~T64Console( ) {

}

//----------------------------------------------------------------------------------------
// Implement the module abstract methods for init, reset and execute. The thread 
// class expect to call a routine that will execute a number of units. For a 
// processor, the unit is an instruction step. The routine will also return
// a boolean value which indicates whether the thread required attention. For a
// processor this is a trap that was raised. The thread will then be placed in 
// a "HALT" state for examination.
//
//----------------------------------------------------------------------------------------
void T64Console::initModule( ) {

    // ??? what to init ...

    T64ThreadModule::initModule( );
}

void T64Console::resetModule( ) {

    // ??? what to reset ...

    T64ThreadModule::resetModule( );
}

T64TrapCode T64Console::executeUnit( ) {

    // ??? if transmit reg non-empty -> set write status to busy and write to 
    // terminal. set the write status to ready.

    // ??? set status to ready )( ? ).
    // ??? do a non-block terminal read.
    // ??? if there is a normal character store to read buffer, set read data 
    //     available in status reg. 
    // ??? else if the character is a special char, we cause a break...

    // ??? should we use all 8 bytes of the data regs ? Then we would need 
    // a kind if size for each reg ...

    return( NO_TRAP );
};


//----------------------------------------------------------------------------------------
// We have a read request for the processor HPA address range. The data is stored
// in 64-bit registers from which we can read a portion if necessary. The data 
// is big endian in our CPU architecture. However, the data stored in a variable
// is potentially little endian on our simulator environment. We need to first 
// correct the endianess and then return the requested number of bytes at the 
// offset in the register.
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64Console::handleHPARead( T64Word pAdr, 
                                        uint8_t *data, 
                                        size_t  len ) {

    size_t wordIndex = static_cast<size_t>(( pAdr - hpaAdr ) / 
                            static_cast<T64Word>( sizeof( T64Word )));

    size_t wordOfs = static_cast<size_t>( pAdr % 
                            static_cast<T64Word>( sizeof( T64Word )));

    switch( wordIndex ) {

        case T64_MREG_STATUS: {

            copyFromReg( data, mrStatus, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_COMMAND: {

            copyFromReg( data, mrCommand, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_CONFIG: {

            copyFromReg( data, mrConfig, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_EIRR: {

            copyFromReg( data, mrEirr, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_SPA_ADR: {

            copyFromReg( data, mrSpaAdr, wordOfs, len );
            return ( T64_SYS_OP_OK );

        } break;

        case T64_MREG_CONSOLE_READ_DATA: {

            return ( T64_SYS_OP_M_CHECK );

        } break;

        case T64_MREG_CONSOLE_WRITE_DATA: {

            return ( T64_SYS_OP_M_CHECK );

        } break;

        default: {

            *data = 0;
            return ( T64_SYS_OP_M_CHECK );
        }
    }
}

//----------------------------------------------------------------------------------------
// We have a write request for the processor HPA address range. Like the read 
// operation, we can also write a portion to the register, if required. 
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64Console::handleHPAWrite( T64Word pAdr, 
                                         uint8_t *data, 
                                         size_t  len ) {

    size_t wordIndex = static_cast<size_t>(( pAdr - hpaAdr ) / 
                            static_cast<T64Word>( sizeof( T64Word )));

    size_t wordOfs = static_cast<size_t>( pAdr % 
                            static_cast<T64Word>( sizeof( T64Word )));

    switch( wordIndex ) {

        case T64_MREG_STATUS: {

            copyToReg( &mrStatus, data, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_COMMAND: {

            copyToReg( &mrCommand, data, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_CONFIG: {

            copyToReg( &mrConfig, data, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_EIRR: {

            copyToReg( &mrEirr, data, wordOfs, len );
            return ( T64_SYS_OP_OK );
        }

        case T64_MREG_SPA_ADR: {

            copyToReg( &mrSpaAdr, data, wordOfs, len );
            return ( T64_SYS_OP_OK );

        } break;

        case T64_MREG_CONSOLE_READ_DATA: {

            return ( T64_SYS_OP_M_CHECK );

        } break;

        case T64_MREG_CONSOLE_WRITE_DATA: {

            return ( T64_SYS_OP_M_CHECK );

        } break;

        default: {

            return ( T64_SYS_OP_M_CHECK );
        }
    }
}

//----------------------------------------------------------------------------------------
// We have a broadcast event.
// 
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64Console::handleControlEvent( T64BusOpControlEvents event, 
                                             T64Word arg1, 
                                             T64Word arg2 ) {

    switch ( event ) {


        default: return( T64_SYS_OP_M_CHECK );
    }
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
T64BusOpStat T64Console::busOpReadEvent( T64Word pAdr, 
                                         uint8_t *data, 
                                          size_t len ) {

    if ( isInIoHpaRange( pAdr )) {

        return( handleHPARead( pAdr, data, len ));
    }
    else return ( T64_SYS_OP_INVALID_ADR );
}

T64BusOpStat T64Console::busOpWriteEvent( T64Word pAdr, 
                                          uint8_t *data, 
                                          size_t len ) {

    if ( isInIoHpaRange( pAdr )) {

        return( handleHPAWrite( pAdr, data, len ));
    }
    else return ( T64_SYS_OP_INVALID_ADR );
}   

T64BusOpStat T64Console::busOpControlEvent( T64BusOpControlEvents  event, 
                                            T64Word                arg1, 
                                            T64Word                arg2 ) {

    return( handleControlEvent( event, arg1, arg2 ));
}
