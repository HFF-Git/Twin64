//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Processor Module
//
//----------------------------------------------------------------------------------------
// The processor object is a module for the T64 System. It consist of the CPU 
// itself, the TLB and Cache subsystems.
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
// A processor is a module with one CPU, TLBs and Caches. We create the component
// objects right here and pass them our instance, such that they have access to 
// these components. Typically, they keep local copies of the references they need.
// 
//----------------------------------------------------------------------------------------
T64Processor::T64Processor( T64System           *sys,
                            int                 modNum,
                            T64Options          options,  
                            T64CpuType          cpuType,
                            T64TlbType          iTlbType,
                            T64TlbType          dTlbType,
                            T64CacheType        iCacheType,
                            T64CacheType        dCacheType,
                            T64Word             spaAdr,
                            int                 spaLen ) : 

                            T64Module(      MT_PROC, 
                                            modNum,
                                            spaAdr,
                                            spaLen ) {

    this -> modNum  = modNum;
    this -> sys     = sys;

    cpu     = new T64Cpu( this, cpuType );

    iTlb    = new T64Tlb( this, T64_TK_INSTR_TLB, iTlbType );
    dTlb    = new T64Tlb( this, T64_TK_DATA_TLB, dTlbType );

    iCache  = new T64Cache( this, T64_CK_INSTR_CACHE, iCacheType );
    dCache  = new T64Cache( this, T64_CK_DATA_CACHE, dCacheType );

    this -> reset( );
}

//----------------------------------------------------------------------------------------
// Destructor.
//
//----------------------------------------------------------------------------------------
T64Processor:: ~T64Processor( ) {

    delete cpu;
    delete iTlb;
    delete dTlb;
    delete iCache;
    delete dCache;
}

//----------------------------------------------------------------------------------------
// Reset the processor and its submodules.
//
//----------------------------------------------------------------------------------------
void T64Processor::reset( ) {

    this -> cpu -> reset( );
    this -> iTlb -> reset( );
    this -> dTlb -> reset( );
    this -> iCache -> reset( );
    this -> dCache -> reset( );

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

T64Tlb *T64Processor::getITlbPtr( ) {

    return( iTlb );
}

T64Tlb *T64Processor::getDTlbPtr( ) {

    return( dTlb );
}

T64Cache *T64Processor::getICachePtr( ) {

    return( iCache );
}

T64Cache *T64Processor::getDCachePtr( ) {

    return( dCache );
}

//----------------------------------------------------------------------------------------
// System Bus operations interface routines. When a module issues a request, any 
// other module will be informed. We can now check whether the bus transactions 
// would concern us.
//
//      busReadSharedBlock:
//
//      Another module is requesting a shared cache block read. If we are a 
//      an observer, we need to check that we do not have the block exclusive.
//      If so and modified, the block is written back to memory and marked as 
//      shared.
//
//      busReadPrivateBlock:
//      
//      Another module is requesting a private copy. If we are an observer, we
//      need to check that we do not have that copy exclusive or shared. In the 
//      exclusive case, we flush and purge the block. In the shared case we just 
//      purge the block from our cache.
//      
//      busWriteBlock:
//
//      Another module is writing back an exclusive copy if its cache block. By
//      definition, we do not own that block in any case.
//
// For all cases, we first check that we are not the originator of that request. 
// Just a little sanity check. Next, we lookup the responsible module by the 
// physical address of the request. If we are not the address range owner, we 
// perform the operations described above on our caches. If we are the owner, 
// we simply carry the request.
//
// A processor cannot be the target of a cache operation. It does not own a 
// physical address range other then its HPA address range. And this range can 
// only be accessed uncached.
//
//----------------------------------------------------------------------------------------
bool T64Processor::busOpReadSharedBlock(  int      reqModNum, 
                                          T64Word  pAdr, 
                                          uint8_t  *data, 
                                          int      len ) {

    if ( reqModNum == moduleNum ) return( false );

    T64Processor *proc = (T64Processor *) sys -> lookupByAdr( pAdr );
    if ( proc == this ) {

        // ??? we are responsible...
    }
    else {

        // ??? we are not responsible, but may have to say something ...

        iCache -> flush( pAdr );
        dCache -> flush( pAdr );
    }

    return (true );
}

bool T64Processor::busOpReadPrivateBlock( int     reqModNum, 
                                          T64Word pAdr, 
                                          uint8_t *data, 
                                          int     len ) {

    if ( reqModNum == moduleNum ) return( false );

    T64Processor *proc = (T64Processor *) sys -> lookupByAdr( pAdr );
    if ( proc == this ) {

        // ??? we are responsible...
    }
    else {

        // ??? we are not responsible, but may have to say something ...

        iCache -> purge( pAdr );
        dCache -> purge( pAdr );
    }

    return (true );
}

bool T64Processor::busOpWriteBlock( int     reqModNum, 
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len ) {
               
    if ( reqModNum == moduleNum ) return( false );

    // by definition, if someone is issuing a write block, the cache line 
    // is exclusive with the module. we ignore... ???
    return (true );
}

//----------------------------------------------------------------------------------------
// System Bus operations non-cache interface routines. When a module issues a 
// request, any other module will be informed. We can now check whether the bus
// transactions would concern us.
//
//      busReadUncached:
//
//      Another module issued an uncached read. We check wether this concerns 
//      our HPA address range. If so, we return the data from the HPA space.
//
//      busWriteUncached:
//
//      Another module issued an uncached write. We check wether this concerns 
//      our HPA address range. If so, we update the data in our HPA space.
//    
// ??? need a function to call for processor HPA data
//
//----------------------------------------------------------------------------------------
bool T64Processor::busOpReadUncached( int     reqModNum, 
                                      T64Word pAdr, 
                                      uint8_t *data, 
                                      int     len ) {
    
    if ( reqModNum == moduleNum ) return( false );

    T64Processor *proc = (T64Processor *) sys -> lookupByAdr( pAdr );
    if ( proc == this ) {

        // ??? we are the target, deliver the data
        *data = 0;
        return (true );
    }
    else {

         // ??? we could have that data in our caches... remove.

        iCache -> flush( pAdr );
        dCache -> flush( pAdr );
        iCache -> purge( pAdr );
        dCache -> purge( pAdr );
    }

    return( true );
}

bool T64Processor::busOpWriteUncached( int     reqModNum,
                                       T64Word pAdr, 
                                       uint8_t *data, 
                                       int     len ) {

   if ( reqModNum == moduleNum ) return( false );

    T64Processor *proc = (T64Processor *) sys -> lookupByAdr( pAdr );
    if ( proc == this ) {

        // ??? we are the target, deliver the data
        *data = 0;
        return (true );
    }
    else {

        // ??? if we have a copy of that data, flush it first. delete the line.

        iCache -> flush( pAdr );
        dCache -> flush( pAdr );
        iCache -> purge( pAdr );
        dCache -> purge( pAdr );
    }
        
    return( false );
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
