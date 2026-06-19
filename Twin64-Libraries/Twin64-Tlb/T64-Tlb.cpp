//----------------------------------------------------------------------------------------
//
// Twin-64 - System Wide TLB
//
//----------------------------------------------------------------------------------------
// The T64-System represent the system consisting of several modules. Modules 
// are for example processor, memory and I/O modules. The simulator is connected
// to the system which handles all module functions. A program start, all
// modules are registered to the system. Think of a kind of bus where you plug
// in boards.  
//
//----------------------------------------------------------------------------------------
//
//  Twin-64 - SSystem Wide TLB
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
#include "T64-Tlb.h"


//----------------------------------------------------------------------------------------
// Local name space.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// Calculate the page size from the size field in the TLB entry. The sizes 
// inclement in multiples of four of the base page size.
// 
//----------------------------------------------------------------------------------------
int tlbPageSize( int size ) {

    return( T64_PAGE_SIZE_BYTES * ( 1U << ( size * 4 )));
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
// data structure. Right now, we only support a fully associative unified TLB 
// of different sizes.
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

    uint16_t tlbInfo = arg2 >> 48;
    int      pSize   = tlbPageSize( tlbInfoPageSize( tlbInfo ));
    
    if ( isInIoAdrRange( arg1 )) return ( true );
    if ( ! isAlignedPageAdr( arg1, pSize )) return ( false );
    if ( ! isAlignedPageAdr( arg2, pSize )) return ( false );

    T64TlbEntry entry;

    entry.pageMask  = tlbPageMask( tlbInfoPageSize( tlbInfo ));
    entry.vAdr      = vAdr( arg1 & entry.pageMask );
    entry.pAdr      = arg2 & entry.pageMask & 0xFFFFFFFFFFULL;
    entry.tlbInfo   = tlbInfo | 0x8000;
    
    for ( int i = 0; i < tlbSize; i++ ) {

        T64TlbEntry *e = &tlbTable[ i ];

         if ( ! ( tlbInfoIsValid( e -> tlbInfo ))) continue;

        if (( e -> vAdr == entry.vAdr ) &&
            ( e -> pageMask == entry.pageMask )) {

            if (( tlbInfoIsLocked( e -> tlbInfo )) && 
                ( ! ( tlbInfoIsLocked( entry.tlbInfo )))) {

                return ( true );
            }
            
            tlbTable[ i ] = entry;
            return ( true );
        }
    }

    for ( int i = 0; i < tlbSize; i++ ) {

        if ( ! ( tlbInfoIsValid( tlbTable[ i ].tlbInfo ))) {

            tlbTable[ i ] = entry;
            return ( true );
        }
    }

    for ( int i = 0; i < tlbSize; i++ ) {

        int idx = tlbRoundRobin++ % tlbSize;

        if ( ! ( tlbInfoIsLocked( tlbTable[ idx ].tlbInfo ))) {

            tlbTable[ idx ] = entry;
            return ( true );
        }
    }

    return( false );
}

//----------------------------------------------------------------------------------------
// Remove an entry. We simply find it and reset the "valid" bit.
//
// ??? ... but keep the lock bit if it is set. 
// We want to be able to remove locked entries.
//----------------------------------------------------------------------------------------
bool T64GlobalTlb::removeTlbEntry( T64Word vAdr ) {

    std::unique_lock lock( tLock );

    T64TlbEntry *e = lookupTlbEntry( tlbTable, tlbSize, vAdr );
    if ( e == nullptr ) return( true );
    
    e -> tlbInfo &= 0x7FFF; 
    return ( true );
}

//----------------------------------------------------------------------------------------
// Routines for the simulator concerning the global TLB:
//
//----------------------------------------------------------------------------------------
int T64GlobalTlb::getTlbSize( ) {

    return( tlbSize );
}

char *T64GlobalTlb::getTlbTypeStr( ) {

    switch ( tlbType ) {

        case T64_TT_FA_16S:     return ((char *) "FA_16S" ); break;
        case T64_TT_FA_32S:     return ( (char *) "FA_32S" ); break;
        case T64_TT_FA_64S:     return ( (char *) "FA_64S" ); break;
        case T64_TT_FA_128S:    return ( (char *) "FA_128S" ); break;
        default:                return ( (char *) "TLB_**" ); break;
    }
}

T64TlbEntry *T64GlobalTlb::getTlbEntry( int index ) {

    if (( index < 0 ) || ( index > tlbSize - 1 )) return ( nullptr );
    if ( tlbTable == nullptr ) return ( nullptr );

    return( &tlbTable[ index ] );
 }


bool T64GlobalTlb::translateAdr( T64Word vAdr, T64Word *pAdr ) {

    T64TlbEntry e;
    
    if ( ! lookupTlb( vAdr, &e )) return( false );
    
    *pAdr = ( vAdr & ~ e.pageMask ) | e.pAdr;
    return( true );
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
// A bus read event that concerns us. We only listen to our HPA address range.
//
//----------------------------------------------------------------------------------------
bool 
T64GlobalTlb::busOpReadEvent( T64Word pAdr, uint8_t *data, int len )  {

    if ( ! isInIoHpaRange( pAdr )) return( false );
    if ( ! isAlignedAdr( pAdr, sizeof( T64Word) )) return ( false );

    T64Word hpaAdr  =  T64_IO_HPA_MEM_START + moduleNum * T64_PAGE_SIZE_BYTES;
    int     wordIndex           = (( pAdr - hpaAdr ) >> 3 );
    int     regSetIndex         = wordIndex / T64_IO_REG_SET_SIZE;
    int     wordInRegSetIndex   = wordIndex % T64_IO_REG_SET_SIZE;
    int     wordOfs             = pAdr % sizeof( T64Word );
    T64Word tmp                 = 0;

    // ??? what registers do we have ?

    copyFromReg( data, tmp, 0, len );
    return ( true );
}

//----------------------------------------------------------------------------------------
// A bus write event that concerns us. Right now, there is no write operation
// defined for the TLB.
//
//----------------------------------------------------------------------------------------
bool 
T64GlobalTlb::busOpWriteEvent( T64Word pAdr, uint8_t *data, int len )  {

    if ( ! isInIoHpaRange( pAdr )) return( false );
    if ( ! isAlignedAdr( pAdr, sizeof( T64Word) )) return ( false );

    T64Word hpaAdr  =  T64_IO_HPA_MEM_START + moduleNum * T64_PAGE_SIZE_BYTES;
    int     wordIndex           = (( pAdr - hpaAdr ) >> 3 );
    int     regSetIndex         = wordIndex / T64_IO_REG_SET_SIZE;
    int     wordInRegSetIndex   = wordIndex % T64_IO_REG_SET_SIZE;
    int     wordOfs             = pAdr % sizeof( T64Word );
    T64Word tmp                 = 0;

    // ??? what registers can we write ?

    return ( false );
} 

//----------------------------------------------------------------------------------------
// A bus control event. The processor uses such events to signal a TLB flush
// for example.
//
//----------------------------------------------------------------------------------------
bool T64GlobalTlb::busOpControlEvent( T64BBusOpControlEvents id, 
                                      T64Word            arg1, 
                                      T64Word            arg2 )  {

    switch ( id ) {

        case T64_CNTRL_EVENT_TLB_INSERT: {

            return( insertTlbEntry( arg1, arg2));  
              
        } break;

        case T64_CNTRL_EVENT_TLB_PURGE: {

            return( removeTlbEntry( arg1 ));
              
        } break;

        default: return( false );
    }
}