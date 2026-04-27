//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - TLB
//
//----------------------------------------------------------------------------------------
// The T64 CPU Simulator has a unified TLB with two small TLB cache structures.
// It is a fully associative TLB with a LRU mechanism to select replacements.
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
// This program is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details. 
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
//
//----------------------------------------------------------------------------------------
inline uint64_t canonicalizeVa( uint64_t vAdr ) {
    
    return ( vAdr & (( 1ULL << T64_PAGE_OFS_BITS ) - 1 ));
}

//----------------------------------------------------------------------------------------
// The page mask is key for the TLB supporting multiple page sizes. When we 
// lookup an entry, the comparison will use the mask to compare the bits 
// corresponding to the page size.
//
//----------------------------------------------------------------------------------------
inline uint64_t pageMask( int pSize ) {
    
    return ~(( T64_PAGE_SIZE_BYTES << ( pSize * 4 )) - 1 );
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
    ptr -> pageSize     = T64_PAGE_SIZE_BYTES;
    ptr -> vAdr         = 0;
    ptr -> pAdr         = 0;
    ptr -> lastUsed     = 0;
    ptr -> pageMask     = 0;
}

//----------------------------------------------------------------------------------------
// Search the L1 level caches. Instruction and data path have a small buffer
// for their translation. It is a simple serial search. The comparison uses
// the page mask to compare the correct address bits.
//
//----------------------------------------------------------------------------------------
T64TlbEntry* lookupL1( T64TlbEntry *tlb, uint32_t tlbEntries, T64Word  vAdr ) {
    
    for ( int i = 0; i < tlbEntries; i++ ) {
        
        if ( tlb[ i ].valid && (( vAdr & tlb[ i ].pageMask ) == tlb[ i ].vAdr ))
            return ( &tlb[ i ] );
    }
    
    return ( nullptr );
}

//----------------------------------------------------------------------------------------
// Search the unified cache. We need to scan all valid entries which to find the 
// best match. 
//
// ??? add comments why ... allow for a range overlap another range ...
// ??? add a good comment here...
//----------------------------------------------------------------------------------------
T64TlbEntry* lookupUnified( T64TlbEntry *tlb, uint32_t tlbEntries, T64Word vAdr ) {
    
    T64TlbEntry *best = nullptr;

    for ( int i = 0; i < tlbEntries; i++ ) {
        
        T64TlbEntry *e = &tlb[ i ];

        if ( ! e -> valid ) continue;

        if (( vAdr & e -> pageMask ) == e -> vAdr ) {
            
            if (( best != nullptr ) || ( e -> pageMask > best -> pageMask )) {
                
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
// structure. Right now, we only support a unified TLB of different sizes.
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

    uTlbRoundRobin  = 0;       
    iTlbRoundRobin  = 0;       
    dTlbtimeGlobal  = 0;       
}

//----------------------------------------------------------------------------------------
// Lookup a Instruction TLB. If we do not find the entry in the L1, we consult 
// the unified TLB buffer and if a translation is found, the L1 buffer is updated.
// The entry to use is found via a simple round robin selection. Instruction fetch 
// tends to be rather serial in contrast to a data TLB.
//  
//----------------------------------------------------------------------------------------
T64TlbEntry *T64Tlb::lookupItlb( T64Word vAdr ) {
    
    T64TlbEntry *e = lookupL1( iTlb, iTlbEntries, vAdr );
    if ( e != nullptr ) return ( e );

    e = lookupUnified( uTlb, uTlbEntries, vAdr );
    if ( e == nullptr ) return ( nullptr );

    int idx = iTlbRoundRobin & ( iTlbEntries - 1 );
    iTlbRoundRobin ++;
    iTlb[ idx ] = *e;
    
    return ( e );
}

//----------------------------------------------------------------------------------------
// Lookup a Data TLB. If we do not find the entry in the L1 buffer, we consult 
// the unified TLB buffer and if a translation is found, the L1 buffer is updated.
// The entry to use is found via a last used selection. Data access tends to be
// rather random in contrast to a instruction TLB.
//
//----------------------------------------------------------------------------------------
T64TlbEntry *T64Tlb::lookupDtlb( T64Word vAdr ) {
                        
    T64TlbEntry *e = lookupL1( dTlb, dTlbEntries, canonicalizeVa( vAdr ));

    if ( e != nullptr ) {

        e -> lastUsed = dTlbtimeGlobal;
        dTlbtimeGlobal ++;
        return ( e );
    }

    e = lookupUnified( dTlb, dTlbEntries, vAdr );
    if ( e == nullptr ) return ( nullptr );

    int victim = 0;

    for ( int i = 1; i < dTlbEntries; i++ ) {

        if ( dTlb[ i ].lastUsed < victim ) victim = i;
    }
   
    dTlb[ victim ] = *e;
    return ( e );
}

//----------------------------------------------------------------------------------------
// The insert method inserts a new entry into the TLB subsystem. First we check 
// if the virtual address is in the physical address range. We do not enter such
// ranges in the TLB. 
//
//
// ??? work on comment ...

//
// Next, we check whether the new entry would overlap an already existing UTLB 
// virtual address range. If there is an overlap, the overlapping entry is 
// invalidated. If none found, find a free entry to use. If none found, we replace
// the least recently used entry. 
//


// Since entries can be locked in the TLB, there is the case that all entries are
// locked and we cannot find a free entry. In this case, we just unlock entry zero.
// Note that this is a rather unlikely case, OS software has to ensure that we do
// not lock all entries. 
// 

//
// Finally, we check the alignment of both virtual and physical address according
// to the page size. If not aligned, the insert operation fails.       
//
// 
//    vAdr Word: "0:VPN:0" encoded.
//
//    Info Word: Flags:8, 0:12, Acc:4, Size:4, PPN:24, 12:0
//    
//    Acc: type:2, Pl1:1, Pl2:1
//
//    Flags: V:1, M:1, L:1, U:1, 0:4   
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
   
    /* 1. replace identical or same mapping */

    for ( int i = 0; i < uTlbEntries; i++ ) {

        T64TlbEntry *e = &uTlb[ i ];

        if ( ! e -> valid ) continue;

        /* identical mapping → skip */

        if (( e -> vAdr == entry.vAdr ) &&
            ( e -> pageMask == entry.pageMask ) &&
            ( e -> pAdr == entry.pAdr )) {

            // ??? what about the case where attributes changed ?    
            return ( true );
        }

        /* same VA + size → replace */

        if (( e -> vAdr == entry.vAdr ) &&
            ( e -> pageMask == entry.pageMask )) {

            if (( e -> locked ) && ( ! entry.locked ))
                return ( true );

            uTlb[i] = entry;
            return ( true );
        }
    }

    /* 2. free slot and insert */

    for ( int i = 0; i < uTlbEntries; i++ ) {

        if ( ! uTlb[ i ].valid ) {

            uTlb[ i ] = entry;
            return ( true );
        }
    }

    /* 3. FIFO replace (skip locked) */

    for ( int i = 0; i < dTlbEntries; i++ ) {

        int idx = uTlbRoundRobin++ % dTlbEntries;

        if ( ! uTlb[ idx ].locked ) {

            uTlb[ idx ] = entry;
            return ( true );
        }
    }

    /* 4. all locked → drop */
}

//----------------------------------------------------------------------------------------
// Remove the TLB entry that contains the virtual address. The entry is removed
// regardless if it is locked or not. 
// 
// ??? we need to also remove in L1 ?
//----------------------------------------------------------------------------------------
bool T64Tlb::purgeTlb( T64Word vAdr ) {

    T64TlbEntry *e = lookupUnified( uTlb, uTlbEntries, vAdr );
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
T64TlbEntry *T64Tlb::getTlbEntry( int index ) {
    
  if ( isInRange( index, 0, uTlbEntries - 1 )) return( &uTlb[ index ] );
  else                                         return( nullptr );
}

int T64Tlb::getTlbSize( ) {

    return ( uTlbEntries );
}

T64TlbKind T64Tlb::getTlbKind( ) {

    return ( tlbKind );
}

T64TlbType T64Tlb::getTlbType( ) {

    return ( tlbType );
}

char *T64Tlb::getTlbTypeString( ) {

    switch ( tlbType ) {

        case T64_TT_FA_16S:     return ( (char *) "FA_16" );
        case T64_TT_FA_32S:     return ( (char *) "FA_32" );
        case T64_TT_FA_64S:     return ( (char *) "FA_64S" );
        case T64_TT_FA_128S:    return ( (char *) "FA_128S" );
        default:                return ( (char *) "Unknown TLB Type" );
    }
}
