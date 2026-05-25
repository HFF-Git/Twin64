//----------------------------------------------------------------------------------------
//
// Twin-64 - System Wide TLB
//
//----------------------------------------------------------------------------------------
// The T64-System represent the system consisting of several modules. Modules are
// for example processor, memory and I/O modules. The simulator is connected to
// the system which handles all module functions. A program start, the individual
// modules are registered to the system. Think of a kind of bus where you plug in
// boards.  
//
//----------------------------------------------------------------------------------------
//
//  Twin-64 - SSystem Wide TLB
// Copyright (C) 2020 - 2026 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under the 
// terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should
//  have received a copy of the GNU General Public License along with this program.  
// If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-Tlb.h"


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
inline T64Word tlbPageMask( int pSize ) {
    
    int shift = T64_PAGE_OFS_BITS + ( pSize * 4 );
    return ~(( 1ULL << shift ) - 1 );
}

//----------------------------------------------------------------------------------------
// Search the global TLB. We need to scan all valid entries which to find the 
// best match. This is because we can deliberately allow for overlapping entries,
// which can be used to deploy a large page and overlap another smaller range.
// The best match is the entry with the largest page mask, which is the smallest
// page size.
//
//----------------------------------------------------------------------------------------
T64TlbEntry* lookupTlbEntry( T64TlbEntry *tlb, 
                             uint32_t    tlbEntries, 
                             T64Word     vAdr ) {
    
    T64TlbEntry *best = nullptr;

    for ( int i = 0; i < tlbEntries; i++ ) {
        
        T64TlbEntry *e = &tlb[ i ];

        if ( ! ( e -> tlbInfo & 0x8000 )) continue;

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
// Object creator. Based on kind and type of TLB, we allocate the global TLB 
// data structure. Right now, we only support a fully associative unified TLB of
// different sizes.
//
//----------------------------------------------------------------------------------------
T64GlobalTlb::T64GlobalTlb( T64ModuleType    modType, 
                            int              modNum, 
                            T64TlbKind       tlbKind,
                            T64TlbType       tlbType ) : 
                            T64Module( modType, modNum, 0, 0 ) {

    this -> tlbKind = tlbKind;
    this -> tlbType = tlbType;

    switch ( tlbType ) {

        case T64_TT_FA_16S:     tlbSize = 16; break;
        case T64_TT_FA_32S:     tlbSize = 32; break;
        case T64_TT_FA_64S:     tlbSize = 64; break;
        case T64_TT_FA_128S:    tlbSize = 128; break;
        default:                tlbSize = 64;
    }

    tlbMissCount = 0;
    tlbHitCount  = 0;

    tlbTable = ( T64TlbEntry *) calloc( tlbSize, sizeof( T64TlbEntry ));
}

//----------------------------------------------------------------------------------------
// Destructor. We just return the allocated memory.
//
//----------------------------------------------------------------------------------------
T64GlobalTlb:: ~ T64GlobalTlb( ) {

    if ( tlbTable != nullptr ) free ( tlbTable );
}

//----------------------------------------------------------------------------------------
// Lookup a translation. We are passed the full virtual address. We need to scan
// all valid entries which to find the best match, because we can deliberately 
// allow for overlapping entries, which can be used to deploy a large page and 
// overlap another smaller range. The best match is the entry with the largest
// page mask, which is the smallest page size. If we found a match, we copy the 
// entry to the passed parameter.
// 
//----------------------------------------------------------------------------------------
bool T64GlobalTlb::lookupTlb( T64Word vAdr, T64TlbEntry *e ) {

    std::shared_lock lock( tLock );

    T64TlbEntry *entry = lookupTlbEntry( tlbTable, tlbSize, vAdr );
    if ( entry == nullptr ) return( false );

    *e = *entry;
    
    tlbHitCount ++;
    return( true );
}

//----------------------------------------------------------------------------------------
// Insert a new translation.
//
//----------------------------------------------------------------------------------------
bool T64GlobalTlb::insertTlbEntry( T64Word arg1, T64Word arg2 ) {

    std::unique_lock lock( tLock );

    int pSize   = tlbPageSize(( arg2 >> 48 ) & T64_TM_PSIZE );

    if ( isInIoAdrRange( arg1 )) return ( true );
    if ( ! isAlignedPageAdr( arg1, pSize )) return ( false );
    if ( ! isAlignedPageAdr( arg2, pSize )) return ( false );

    T64TlbEntry entry;

    entry.pageMask  = tlbPageMask( pSize );
    entry.vAdr      = arg1 & entry.pageMask;
    entry.pAdr      = arg2 & entry.pageMask;
    entry.tlbInfo   = arg2 >> 48;

    for ( int i = 0; i < tlbSize; i++ ) {

        T64TlbEntry *e = &tlbTable[ i ];

         if ( ! ( e -> tlbInfo & T64_TM_VALID )) continue;

        if (( e -> vAdr == entry.vAdr ) &&
            ( e -> pageMask == entry.pageMask )) {

            if (( e -> tlbInfo & T64_TM_LOCKED ) && 
                ( ! ( entry.tlbInfo & T64_TM_LOCKED ))) {

                return ( true );
            }
            
            tlbTable[ i ] = entry;
            return ( true );
        }
    }

    for ( int i = 0; i < tlbSize; i++ ) {

        if ( ! ( tlbTable[ i ].tlbInfo & T64_TM_VALID )) {

            tlbTable[ i ] = entry;
            return ( true );
        }
    }

    for ( int i = 0; i < tlbSize; i++ ) {

        int idx = tlbRoundRobin++ % tlbSize;

        if ( ! ( tlbTable[ idx ].tlbInfo & T64_TM_LOCKED )) {

            tlbTable[ idx ] = entry;
            return ( true );
        }
    }

    return( false );
}

//----------------------------------------------------------------------------------------
// Remove an entry. We simply find it and reset the "valid" bit.
//
//----------------------------------------------------------------------------------------
bool T64GlobalTlb::removeTlbEntry( T64Word vAdr ) {

    std::unique_lock lock( tLock );

    T64TlbEntry *e = lookupTlbEntry( tlbTable, tlbSize, vAdr );
    if ( e == nullptr ) return( true );
    
    e -> tlbInfo &= ( ~ T64_TM_VALID );
    return ( true );
}

//----------------------------------------------------------------------------------------
// Module interface. Although we are not really a module such as processors and
// I/O, it is convenient to implement the global TLB as a module in our simulator.
// The key benefit is that all TLB data can nicely be mapped into the HPA address
// range and we do not need special functions to display the TLB content. 
//
//
//----------------------------------------------------------------------------------------
void T64GlobalTlb::initModule( ) { 

    resetModule( );
}

void T64GlobalTlb::resetModule( ) { 

    tlbMissCount = 0;
    tlbHitCount  = 0;

    if ( tlbTable != nullptr ) free ( tlbTable );

    tlbTable = ( T64TlbEntry *) calloc( tlbSize, sizeof( T64TlbEntry ));
}

//----------------------------------------------------------------------------------------
// Dummy functions, we are a shared state module and not a thread.
//
//----------------------------------------------------------------------------------------
void T64GlobalTlb::haltModule( ) { }
void T64GlobalTlb::runModule( ) { };
void T64GlobalTlb::execModule( int steps ) { }
bool T64GlobalTlb::executeUnit( ) { return( false ); }
void T64GlobalTlb::waitUntilHalted( ) { }

//----------------------------------------------------------------------------------------
// A bus read event that concerns us. We only listen to our HPA address range.
//
//----------------------------------------------------------------------------------------
bool 
T64GlobalTlb::busOpReadEvent( int reqModNum, T64Word pAdr, uint8_t *data, int len )  {

    if ( ! isInIoHpaRange( pAdr )) return( false );
    if ( ! isAlignedPageAdr( pAdr, sizeof( T64Word) )) return ( false );

    int wordIndex = ( pAdr - T64_IO_HPA_MEM_START ) >> 3;

    if ( wordIndex < 32 ) { // ??? come up wth named constant ... 

        switch ( wordIndex ) {  

            // ??? simple cases are the variables such miss or hot count.

            // ??? complex cases are the words from the TLB entry. We may 
            // not do it in the switch statement.

            // ??? need to build the second word from TÖB pAdr and info field.

            default: ;
        }
    }
    else {

        // ??? TLB array access in the reg sets 1 ... 8

    }

    *data = 0; // ??? for now ...
    return ( true );
}

//----------------------------------------------------------------------------------------
// A bus write event that concerns us. Right now, there is no write operation
// defined for the TLB.
//
//----------------------------------------------------------------------------------------
bool 
T64GlobalTlb::busOpWriteEvent( int reqModNum, T64Word pAdr, uint8_t *data, int len )  {

    return ( false );
} 

//----------------------------------------------------------------------------------------
// A bus broadcast event. The processor uses such events to signal a TLB flush
// for example.
//
//----------------------------------------------------------------------------------------
bool T64GlobalTlb::busOpBroadcastEvent( int srcModNum,
                                        T64BroadcastEvents id, 
                                        T64Word            arg1, 
                                        T64Word            arg2 )  {

    // ??? need to implement this ....

    return ( false );
}