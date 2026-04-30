//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - TLB
//
//----------------------------------------------------------------------------------------
// The T64 CPU Simulator has a unified TLB with two small TLB cache structures.
// It is a fully associative TLB with a LRU mechanism to select replacements.
// Our TLB is a unified TLB with two small TLB buffers for instruction and data 
// translation. The TLB supports multiple page sizes. A miss in the the small
// instruction or data TLB will trigger a search in the unified TLB. A miss in
// the unified TLB will trigger a TLB miss trap. The TLB supports a locked entry,
// which is not replaced by the LRU mechanism. 
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
// Calculate the page size from the size field in the TLB entry. Currently, there
// are four sizes defined. They are 4 Kb, 64 Kb, 1 Mb and 16 Mb.
// 
//----------------------------------------------------------------------------------------
int tlbPageSize( int size ) {

    return( T64_PAGE_SIZE_BYTES * ( 1U << ( size * 4 )));
}

//----------------------------------------------------------------------------------------
// "canonicalizeVa" clears the page offset part of a virtual address.
//
//----------------------------------------------------------------------------------------
inline T64Word canonicalizeVa( T64Word vAdr ) {

    return vAdr & (( 1ULL << T64_VADR_BITS ) - 1);
}

//----------------------------------------------------------------------------------------
// The page mask is key for the TLB supporting multiple page sizes. When we 
// lookup an entry, the comparison will use the mask to compare the bits 
// corresponding to the page size.
//
//----------------------------------------------------------------------------------------
inline T64Word pageMask( int pSize ) {
    
    int shift = T64_PAGE_OFS_BITS + ( pSize * 4 );
    return ~(( 1ULL << shift ) - 1 );
}

//----------------------------------------------------------------------------------------
// Clear a TLB entry.
//
//----------------------------------------------------------------------------------------
void resetTlbEntry( T64TlbEntry *ptr ) {
    
    ptr -> valid        = false;
    ptr -> locked       = false;
    ptr -> modified     = false;
    ptr -> uncached     = false;
    ptr -> pLev1        = false;
    ptr -> pLev2        = false;
    ptr -> pageType     = PT_NONE;
    ptr -> pageSize     = 0;
    ptr -> vAdr         = 0;
    ptr -> pAdr         = 0;
    ptr -> pageMask     = 0;
}

//----------------------------------------------------------------------------------------
// Search the L1 level TLB caches. Instruction and data path have a small buffer
// for their translation. It is a simple serial search. The comparison uses
// the page mask to compare the correct address bits.
//
//----------------------------------------------------------------------------------------
T64TlbEntry* lookupL1( T64TlbEntry *tlb, uint32_t tlbEntries, T64Word  vAdr ) {

    if ( ! tlb ) return ( nullptr );
    
    for ( int i = 0; i < tlbEntries; i++ ) {
        
        if ( tlb[ i ].valid && (( vAdr & tlb[ i ].pageMask ) == tlb[ i ].vAdr )) {

            return ( &tlb[ i ] );
        }
            
    }
    
    return ( nullptr );
}

//----------------------------------------------------------------------------------------
// Search the unified cache. We need to scan all valid entries which to find the 
// best match. This is because we can deliberately allow for overlapping entries,
// which can be used to deploy a large page and overlap another smaller range.
// The best match is the entry with the largest page mask, which is the smallest
// page size.
//
//----------------------------------------------------------------------------------------
T64TlbEntry* lookupUnified( T64TlbEntry *tlb, uint32_t tlbEntries, T64Word vAdr ) {
    
    T64TlbEntry *best = nullptr;

    for ( int i = 0; i < tlbEntries; i++ ) {
        
        T64TlbEntry *e = &tlb[ i ];

        if ( ! e -> valid ) continue;

        if (( vAdr & e -> pageMask ) == e -> vAdr ) {
            
            if (( best == nullptr ) || ( e -> pageMask > best -> pageMask )) {
    
                best = e;
            }
        }
    }

    return ( best );
}

} // namespace


//****************************************************************************************
//****************************************************************************************
//
// TLB
//
//----------------------------------------------------------------------------------------
// Object creator. Based on kind and type of TLB, we allocate the TLB data
// structure. Right now, we only support a fully associative unified TLB of
// different sizes.
//
//----------------------------------------------------------------------------------------
T64Tlb::T64Tlb( T64Processor *proc, T64TlbKind tlbKind, T64TlbType tlbType ) {

    this -> proc    = proc;
    this -> tlbKind = tlbKind;
    this -> tlbType = tlbType;

    iTlbEntries = 4;
    dTlbEntries = 4;

    switch ( tlbType ) {

        case T64_TT_FA_16S:     uTlbEntries = 16;   break;
        case T64_TT_FA_32S:     uTlbEntries = 32;   break;
        case T64_TT_FA_64S:     uTlbEntries = 64;   break;
        case T64_TT_FA_128S:    uTlbEntries = 128;  break;
        default:                uTlbEntries = 64;
    }

    iTlb = (T64TlbEntry *) calloc( iTlbEntries, sizeof( T64TlbEntry ));
    dTlb = (T64TlbEntry *) calloc( dTlbEntries, sizeof( T64TlbEntry ));
    uTlb = (T64TlbEntry *) calloc( uTlbEntries, sizeof( T64TlbEntry ));

    reset( );
}

//----------------------------------------------------------------------------------------
// Destructor.  
//
//----------------------------------------------------------------------------------------
T64Tlb::~T64Tlb( ) {

    free( iTlb );
    free( dTlb );
    free( uTlb );
}

//----------------------------------------------------------------------------------------
// Reset the TLB.
//
//----------------------------------------------------------------------------------------
void T64Tlb::reset( ) {
    
    for ( int i = 0; i < iTlbEntries; i++ ) resetTlbEntry( &iTlb[ i ] );
    for ( int i = 0; i < dTlbEntries; i++ ) resetTlbEntry( &dTlb[ i ] );
    for ( int i = 0; i < uTlbEntries; i++ ) resetTlbEntry( &uTlb[ i ] );

    uTlbRoundRobin      = 0;       
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
T64TlbEntry *T64Tlb::lookupItlb( T64Word vAdr ) {
    
    T64TlbEntry *e = lookupL1( iTlb, iTlbEntries, canonicalizeVa( vAdr ));
    if ( e != nullptr ) {
        
        iTlbHits ++;
        return ( e );
    }
    else iTlbMisses ++;

    e = lookupUnified( uTlb, uTlbEntries, canonicalizeVa( vAdr ));
    if ( e == nullptr ) {
        
        iTlbMissUTlbMisses ++;
        return ( nullptr );
    }
    else {
        
        iTlbMissUTlbHits ++;

        iTlb[ iTlbRoundRobin & ( iTlbEntries - 1 ) ] = *e;
        iTlbRoundRobin ++;
    
        return ( e );
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
T64TlbEntry *T64Tlb::lookupDtlb( T64Word vAdr ) {
                        
    T64TlbEntry *e = lookupL1( dTlb, dTlbEntries, canonicalizeVa( vAdr ));
    if ( e != nullptr ) {

        dTlbHits ++;

        int         idx = e - dTlb;
        T64TlbEntry hit = *e;

        for ( int i = idx; i > 0; i-- ) dTlb[ i ] = dTlb[ i - 1 ];
        dTlb[0] = hit;
        return ( e );
    }
    else dTlbMisses ++;

    e = lookupUnified( uTlb, uTlbEntries, canonicalizeVa( vAdr ));
    if ( e == nullptr ) {

        dTlbMissUTlbMisses ++;
        return ( nullptr );
    }
    else {

        dTlbMissUTlbHits ++;

        for ( int i = dTlbEntries - 1; i > 0; i-- ) dTlb[i] = dTlb[i - 1];
        dTlb[0] = *e;
        return ( e );
    }
}

//----------------------------------------------------------------------------------------
// The insert method inserts a new entry into the TLB subsystem. First we check 
// if the virtual address is in the architected physical address range. We do 
// not enter such ranges in the TLB. 
//
// We first check if the entry already exists. If so, we update the fields. If
// the is not found, we check for a free entry and insert the new entry. If no
// free entry is found, we replace an existing entry using a round robin 
// replacement policy. We skip locked entries. If all entries are locked, we 
// just drop the new entry.
//
//----------------------------------------------------------------------------------------
bool T64Tlb::insertTlb( T64Word vAdr, T64Word info ) {

    int         pSize   = tlbPageSize( extractField64( info, 36, 4 ) );
    T64Word     pAdr    = extractField64( info, 12, 24 ) << T64_PAGE_OFS_BITS;
    T64TlbEntry entry;

    if ( isInIoAdrRange( vAdr )) return ( true );
    if ( ! isAlignedPageAdr( vAdr, pSize )) return ( false );
    if ( ! isAlignedPageAdr( pAdr, pSize )) return ( false );
    
    entry.valid         = true;
    entry.modified      = false;
    entry.locked        = extractBit64( info, 63 );
    entry.uncached      = extractBit64( info, 62 );
    entry.pLev1         = extractBit64( info, 41 );
    entry.pLev2         = extractBit64( info, 40 );
    entry.pageType      = (T64PageType) extractField64( info, 42, 2 );
    entry.pageSize      = pSize;
    entry.pageMask      = pageMask( pSize );
    entry.vAdr          = canonicalizeVa( vAdr ) & entry.pageMask;
    entry.pAdr          = pAdr & entry.pageMask;
   
    for ( int i = 0; i < uTlbEntries; i++ ) {

        T64TlbEntry *e = &uTlb[ i ];

        if ( ! e -> valid ) continue;

        if (( e -> vAdr == entry.vAdr ) &&
            ( e -> pageMask == entry.pageMask )) {

            if (( e -> locked ) && ( ! entry.locked ))
                return ( true );

            uTlb[i] = entry;
            return ( true );
        }
    }

    for ( int i = 0; i < uTlbEntries; i++ ) {

        if ( ! uTlb[ i ].valid ) {

            uTlb[ i ] = entry;
            return ( true );
        }
    }

    for ( int i = 0; i < uTlbEntries; i++ ) {

        int idx = uTlbRoundRobin++ % uTlbEntries;

        if ( ! uTlb[ idx ].locked ) {

            uTlb[ idx ] = entry;
            return ( true );
        }
    }

    return( false );
}

//----------------------------------------------------------------------------------------
// Remove the TLB entry that contains the virtual address. The entry is removed
// regardless if it is locked or not. 
// 
//----------------------------------------------------------------------------------------
bool T64Tlb::purgeTlb( T64Word vAdr ) {

    T64TlbEntry *e = lookupL1( iTlb, iTlbEntries, vAdr );
    if ( e != nullptr ) {   

        e -> valid = false;
        return( true );
    }

    e = lookupL1( dTlb, dTlbEntries, vAdr );
    if ( e != nullptr ) {   

        e -> valid = false;
        return( true );
    }

    e = lookupUnified( uTlb, uTlbEntries, vAdr );
    if ( e != nullptr ) {

        e -> valid = false;
        return( true );
    }
    else return( false );
}

//----------------------------------------------------------------------------------------
// Utility routines used by the Simulator.
// 
//----------------------------------------------------------------------------------------
T64TlbEntry *T64Tlb::getUTLBEntry( int index ) const {
    
  if ( isInRange( index, 0, uTlbEntries - 1 )) return( &uTlb[ index ] );
  else                                         return( nullptr );
}

T64TlbEntry *T64Tlb::getITLBEntry( int index ) const {
    
  if ( isInRange( index, 0, iTlbEntries - 1 )) return( &iTlb[ index ] );
  else                                         return( nullptr );
}

T64TlbEntry *T64Tlb::getDTLBEntry( int index ) const {
    
  if ( isInRange( index, 0, dTlbEntries - 1 )) return( &dTlb[ index ] );
  else                                         return( nullptr );
}

int T64Tlb::getTlbSize( ) const {

    return ( uTlbEntries );
}

T64TlbKind T64Tlb::getTlbKind( ) const{

    return ( tlbKind );
}

T64TlbType T64Tlb::getTlbType( ) const {

    return ( tlbType );
}

char *T64Tlb::getTlbTypeString( ) const {

    switch ( tlbType ) {

        case T64_TT_FA_16S:     return ( (char *) "FA_16" );
        case T64_TT_FA_32S:     return ( (char *) "FA_32" );
        case T64_TT_FA_64S:     return ( (char *) "FA_64S" );
        case T64_TT_FA_128S:    return ( (char *) "FA_128S" );
        default:                return ( (char *) "Unknown TLB Type" );
    }
}
