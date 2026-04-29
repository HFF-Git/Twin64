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

  //  iCache  = new T64Cache( this, T64_CK_INSTR_CACHE, iCacheType );
  //  dCache  = new T64Cache( this, T64_CK_DATA_CACHE, dCacheType );

    this -> reset( );
}

//----------------------------------------------------------------------------------------
// Destructor.
//
//----------------------------------------------------------------------------------------
T64Processor:: ~T64Processor( ) {

    delete cpu;
    delete tlb;
   // delete iCache;
   // delete dCache;
}

//----------------------------------------------------------------------------------------
// Reset the processor and its submodules.
//
//----------------------------------------------------------------------------------------
void T64Processor::reset( ) {

    this -> cpu -> reset( );
    this -> tlb -> reset( );
 //   this -> iCache -> reset( );
 //   this -> dCache -> reset( );

    instructionCount    = 0;
    cycleCount          = 0;
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

        // ??? we are responsible...
    }

    return( true );
}

bool T64Processor::busOpWriteEvent( int     reqModNum,
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len ) {

    if ( reqModNum == moduleNum ) return( false );

    T64Processor *proc = (T64Processor *) sys -> lookupByAdr( pAdr );
    if ( proc == this ) {

        // ??? we are responsible...
    }
        
    return( true );
}   

//----------------------------------------------------------------------------------------
// The step routine is the entry point to the processor for executing one or 
// more instructions.
//
//----------------------------------------------------------------------------------------
void T64Processor::run( ) {

    try {

        while ( true ) step( );
    }
    catch ( const T64Trap t ) {
        
    }
}

//----------------------------------------------------------------------------------------
// The step routine is the entry point to the processor for executing one or 
// more instructions.
//
//----------------------------------------------------------------------------------------
void T64Processor::step( ) {

    try {
        
        cpu -> step( );
    }
    
    catch ( const T64Trap t ) {
        
    }
}
