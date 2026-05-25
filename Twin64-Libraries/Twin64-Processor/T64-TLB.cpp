//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - TLB
//
//----------------------------------------------------------------------------------------
// The T64 CPU Simulator has a unified TLB with two small TLB cache structures
// in each processor. It is a fully associative TLB with a LRU mechanism to 
// select replacements. The TLB supports multiple page sizes. A miss in the the
// small instruction or data TLB will trigger a search in the unified TLB. A miss 
// in the unified TLB will trigger a TLB miss trap. The TLB supports a locked
// entry, which is not replaced by the LRU mechanism. 
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - TLB
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
#include "T64-Common.h"
#include "T64-Util.h"
#include "T64-System.h"
#include "T64-Processor.h"

//----------------------------------------------------------------------------------------
// Local name space.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// "canonicalizeVa" clears the page offset part of a virtual address.
//
//----------------------------------------------------------------------------------------
inline T64Word canonicalizeVa( T64Word vAdr ) {

    return vAdr & (( 1ULL << T64_VADR_BITS ) - 1);
}

//----------------------------------------------------------------------------------------
// Clear a TLB entry.
//
//----------------------------------------------------------------------------------------
void resetTlbEntry( T64TlbEntry *ptr ) {
    
    ptr -> tlbInfo      = 0;
    ptr -> vAdr         = 0;
    ptr -> pAdr         = 0;
    ptr -> pageMask     = 0;
}

//----------------------------------------------------------------------------------------
// Search the TLB. Instruction and data path have a small buffer or their 
// translation. It is a simple serial search. The comparison use the page mask
// to compare the correct address bits.
//
//----------------------------------------------------------------------------------------
T64TlbEntry* lookup( T64TlbEntry *tlb, uint32_t tlbSize, T64Word  vAdr ) {

    if ( ! tlb ) return ( nullptr );
    
    for ( int i = 0; i < tlbSize; i++ ) {
        
        if (( tlb[ i ].tlbInfo & T64_TM_VALID ) && 
            (( vAdr & tlb[ i ].pageMask ) == tlb[ i ].vAdr )) {

            return ( &tlb[ i ] );
        }
            
    }
    
    return ( nullptr );
}

} // namespace


//****************************************************************************************
//****************************************************************************************
//
// TLB
//
//----------------------------------------------------------------------------------------
// Object creator. Based on kind and type of TLB, we allocate the local TLB data
// structure. 
//
//----------------------------------------------------------------------------------------
T64LocalTlb::T64LocalTlb( T64Processor *proc, T64TlbKind tlbKind, T64TlbType tlbType ) {

    this -> proc    = proc;
    this -> tlbKind = tlbKind;
    this -> tlbType = tlbType;

    iTlbEntries = 4;
    dTlbEntries = 4;

    iTlb = (T64TlbEntry *) calloc( iTlbEntries, sizeof( T64TlbEntry ));
    dTlb = (T64TlbEntry *) calloc( dTlbEntries, sizeof( T64TlbEntry ));

    reset( );
}

//----------------------------------------------------------------------------------------
// Destructor.  
//
//----------------------------------------------------------------------------------------
T64LocalTlb::~T64LocalTlb( ) {

    free( iTlb );
    free( dTlb );
}

//----------------------------------------------------------------------------------------
// Reset the local TLB.
//
//----------------------------------------------------------------------------------------
void T64LocalTlb::reset( ) {
    
    for ( int i = 0; i < iTlbEntries; i++ ) resetTlbEntry( &iTlb[ i ] );
    for ( int i = 0; i < dTlbEntries; i++ ) resetTlbEntry( &dTlb[ i ] );
     
    iTlbRoundRobin      = 0;            
    
    iTlbHits            = 0;
    iTlbMisses          = 0;
    iTlbMissUTlbHits    = 0;
    iTlbMissUTlbMisses  = 0;

    dTlbHits            = 0;
    dTlbMisses          = 0;
    dTlbMissUTlbHits    = 0;
    dTlbMissUTlbMisses  = 0;
}

//----------------------------------------------------------------------------------------
// Lookup a Instruction TLB. If we do not find the entry in the L1, we consult 
// the unified TLB buffer and if a translation is found, the L1 buffer is updated.
// The entry to update is found via a simple round robin selection. Instruction
// fetch tends to be rather serial in contrast to a data TLB.
//  
//----------------------------------------------------------------------------------------
bool T64LocalTlb::lookupItlb( T64Word vAdr, T64Word *pAdr, uint16_t *tlbInfo ) {
    
    T64TlbEntry *e = lookup( iTlb, iTlbEntries, canonicalizeVa( vAdr ));
    if ( e != nullptr ) {

        *pAdr    = e -> pAdr | ( vAdr & ~ tlbEntry.pageMask );  
        *tlbInfo = e -> tlbInfo;
        iTlbHits ++;
        return ( true );
    }
    
    iTlbMisses ++;

    T64GlobalTlb *gTlb = proc -> getGlobalTlbPtr( );
    if ( gTlb == nullptr ) return ( false );

    T64TlbEntry tlbEntry;
    if ( ! gTlb -> lookupTlb( vAdr, &tlbEntry )) {
        
        iTlbMissUTlbMisses ++;
        return ( false );
    }
    else {

        iTlbMissUTlbHits ++;

        iTlb[ iTlbRoundRobin & ( iTlbEntries - 1 ) ] = tlbEntry;
        iTlbRoundRobin ++;

        *pAdr    = tlbEntry.pAdr | ( vAdr & ~ tlbEntry.pageMask );  
        *tlbInfo = tlbEntry.tlbInfo;
        return( true );
    }
}

//----------------------------------------------------------------------------------------
// Lookup a Data TLB. If we find the entry in the L1 buffer, we return it and 
// also place it in the slot 0, so that the next lookup has a higher chance of
// finding it. All other entries are shifted by one. If we do not find the entry
// in the L1 buffer, we consult the unified TLB buffer and if a translation is 
// found, the L1 buffer is updated.
// 
//----------------------------------------------------------------------------------------
bool T64LocalTlb::lookupDtlb( T64Word vAdr, T64Word *pAdr, uint16_t *tlbInfo ) {
                        
    T64TlbEntry *e = lookup( dTlb, dTlbEntries, canonicalizeVa( vAdr ));
    if ( e != nullptr ) {

        dTlbHits ++;

        int         idx = e - dTlb;
        T64TlbEntry hit = *e;

        for ( int i = idx; i > 0; i-- ) dTlb[ i ] = dTlb[ i - 1 ];
        dTlb[ 0 ] = hit;

        *pAdr    = hit.pAdr | ( vAdr & ~ hit.pageMask ); 
        *tlbInfo = hit.tlbInfo;
        return ( true );
    }
   
    dTlbMisses ++;

    T64GlobalTlb *gTlb = proc -> getGlobalTlbPtr( );
    if ( gTlb == nullptr ) return ( false );

    T64TlbEntry tlbEntry;
    if ( ! gTlb -> lookupTlb( vAdr, &tlbEntry )) {
        
        dTlbMissUTlbMisses ++;
        return ( false );
    }
    else {

        dTlbMissUTlbHits ++;

        for ( int i = dTlbEntries - 1; i > 0; i-- ) dTlb[ i ] = dTlb[ i - 1 ];
        dTlb[ 0 ] = tlbEntry;

        *pAdr    = tlbEntry.pAdr | ( vAdr & ~ tlbEntry.pageMask );  
        *tlbInfo = tlbEntry.tlbInfo;
        return ( true );
    }
}

//----------------------------------------------------------------------------------------
// Remove the TLB entry that contains the virtual address from both TLBs. The 
// entry is removed regardless if it is locked or not. 
// 
//----------------------------------------------------------------------------------------
bool T64LocalTlb::purgeTlb( T64Word vAdr ) {

    T64TlbEntry *e = lookup( iTlb, iTlbEntries, vAdr );
    if ( e != nullptr ) e -> tlbInfo &= ~ T64_TM_VALID;

    e = lookup( dTlb, dTlbEntries, vAdr );
    if ( e != nullptr ) e -> tlbInfo &= ~ T64_TM_VALID;
        
    return( true );
}
