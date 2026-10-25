//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Disc Module
//
//----------------------------------------------------------------------------------------
// The disc module represents ...
//
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Disc Module
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
#include "T64-Disc.h"

//----------------------------------------------------------------------------------------
// Name space for local routines.
//
//----------------------------------------------------------------------------------------
namespace {

};

//****************************************************************************************
//****************************************************************************************
//
// Disc Module Object.
//
//----------------------------------------------------------------------------------------
// The disc module represents ...
//
// A disc runs in its own thread. The thread logic is inherited from the 
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
T64Disc::T64Disc( T64System         *sys,
                  int               modNum,
                  T64DiscOptions    options 

                      ) : T64ThreadModule( sys,
                                           T64_MOD_TYPE_DISC, 
                                           modNum,
                                           0,
                                           0 ) {

    this -> sys = sys;

   
}

//----------------------------------------------------------------------------------------
// Destructor. We just do our work, the base classes will do theirs next.
//
//----------------------------------------------------------------------------------------
T64Disc:: ~T64Disc( ) {

    
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
void T64Disc::initModule( ) {

    // ??? what to init ...

    T64ThreadModule::initModule( );
}

void T64Disc::resetModule( ) {

    // ??? what to reset ...

    T64ThreadModule::resetModule( );
}

T64TrapCode T64Disc::executeUnit( ) {

    // ??? rough idea:
    // ??? we will execute the read and write operation. A read would just copy
    // the character to the IO register. A special character needs to be defined
    // that gets us back to he simulator. A write operation copies a character
    // if available to the terminal window.

    return( NO_TRAP );
};


//----------------------------------------------------------------------------------------
// We have a read request for the processor HPA address range. 
//
// The data is stored in 64-bit registers. The data is big endian in our CPU
// architecture. However, the data stored in a variable is potentially little
// endian on our simulator environment. We need to first correct the endianess 
// and then return the requested number of bytes at the offset in the register.
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64Disc::handleHPARead( T64Word pAdr, 
                                     uint8_t *data, 
                                     size_t  len ) {

    // ??? access to the disc module registers... 
    // ??? factor out the architected registers in all modules...

   return( T64_SYS_OP_M_CHECK );
}

//----------------------------------------------------------------------------------------
// We have a write request for the processor HPA address range.
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64Disc::handleHPAWrite( T64Word pAdr, 
                                      uint8_t *data, 
                                      size_t len ) {

    // ??? access to the disc module registers... 
    // ??? factor out the architected registers in all modules...

    return ( T64_SYS_OP_M_CHECK );
}

//----------------------------------------------------------------------------------------
// We have a broadcast event.
// 
//
//----------------------------------------------------------------------------------------
T64BusOpStat T64Disc::handleControlEvent( T64BusOpControlEvents event, 
                                          T64Word arg1, 
                                          T64Word arg2 ) {

    switch ( event ) {


        default: return ( T64_SYS_OP_M_CHECK );
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
T64BusOpStat T64Disc::busOpReadEvent( T64Word pAdr, 
                                      uint8_t *data, 
                                      size_t len ) {

    return( handleHPARead( pAdr, data, len ));
}

T64BusOpStat T64Disc::busOpWriteEvent( T64Word pAdr, 
                                       uint8_t *data, 
                                       size_t len ) {

    return( handleHPAWrite( pAdr, data, len ));
}   

T64BusOpStat T64Disc::busOpControlEvent( T64BusOpControlEvents  event, 
                                         T64Word             arg1, 
                                         T64Word             arg2 ) {

    return( handleControlEvent( event, arg1, arg2 ));
}
