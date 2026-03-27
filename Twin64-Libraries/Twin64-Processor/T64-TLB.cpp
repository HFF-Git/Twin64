//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - TLB
//
//----------------------------------------------------------------------------------------
// The T64 CPU Simulator has a unified TLB. It is a fully associative TLB with 
// a LRU mechanism to select replacements.
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

} // namespace


//****************************************************************************************
//****************************************************************************************
//
// TLB
//
//----------------------------------------------------------------------------------------
// Object creator. Based on kind and type of TLB, we allocate the TLB data
// structure.
//
//----------------------------------------------------------------------------------------
T64Tlb::T64Tlb( T64Processor *proc, T64TlbKind tlbKind, T64TlbType tlbType ) {

    this -> proc    = proc;
    this -> tlbKind = tlbKind;
    this -> tlbType = tlbType;

    switch ( tlbType ) {

        case T64_TT_FA_16S:     tlbEntries = 16;  break;
        case T64_TT_FA_32S:     tlbEntries = 32;  break;
        case T64_TT_FA_64S:     tlbEntries = 64;  break;
        case T64_TT_FA_128S:    tlbEntries = 128; break;
        default:                tlbEntries = 64;
    }

    map = (T64TlbEntry *) malloc( tlbEntries * sizeof( T64TlbEntry ));
    reset( );
}

//----------------------------------------------------------------------------------------
// Destructor.  
//
//----------------------------------------------------------------------------------------
T64Tlb::~T64Tlb( ) {

    free( map );
}

//----------------------------------------------------------------------------------------
// Reset a TLB.
//
//----------------------------------------------------------------------------------------
void T64Tlb::reset( ) {
    
    for ( int i = 0; i < tlbEntries; i++ ) {

        T64TlbEntry *ptr = &map[ i ];
        
        ptr -> valid          = false;
        ptr -> locked         = false;
        ptr -> modified       = false;
        ptr -> uncached       = false;
        ptr -> pLev1          = false;
        ptr -> pLev2          = false;
        ptr -> pageType       = PT_NONE;
        ptr -> pageSize       = T64_PAGE_SIZE_BYTES;
        ptr -> vAdr           = 0;
        ptr -> pAdr           = 0;
        ptr -> lastUsed       = 0;
    }

    timeCounter = 0;
}

//----------------------------------------------------------------------------------------
// The lookup method checks all valid entries if they cover the virtual address.
// If found we update the last used field and return the entry.
//
//----------------------------------------------------------------------------------------
T64TlbEntry *T64Tlb::lookup( T64Word vAdr ) {

    timeCounter ++;
    
    for ( int i = 0; i < tlbEntries; i++ ) {
        
        T64TlbEntry *ptr     = &map[ i ];
        
        if (( ptr -> valid ) && 
            ( isInRange( vAdr, ptr -> vAdr, ptr -> vAdr + ptr -> pageSize - 1 ))) {
         
            ptr -> lastUsed = timeCounter;
            return( ptr );
        }
    }
    
    return( nullptr );
}

//----------------------------------------------------------------------------------------
// The insert method inserts a new entry. First we check if the virtual address 
// is in the physical address range. We do not enter such ranges in the TLB. 
// Next, we check whether the new entry would overlap an already existing TLB 
// virtual address range. If there is an overlap, the overlapping entry is 
// invalidated. If none found, find a free entry to use. If none found, we replace
// the least recently used entry. 
//
// Since entries can be locked in the TLB, there is the case that all entries are
// locked and we cannot find a free entry. In this case, we just unlock entry zero.
// Note that this is a rather unlikely case, OS software has to ensure that we do
// not lock all entries. 
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
bool T64Tlb::insert( T64Word vAdr, T64Word info ) {

    timeCounter++;

    int         pSize    = tlbPageSize( extractField64( info, 36, 4 ) );
    T64Word     pAdr     = extractField64( info, 12, 24 ) << T64_PAGE_OFS_BITS;
    T64TlbEntry *entry   = nullptr;

    if ( isInIoAdrRange( vAdr )) return ( true );

    if ( ! isAlignedPageAdr( vAdr, pSize )) return ( false );
    if ( ! isAlignedPageAdr( pAdr, pSize )) return ( false );

    for ( int i = 0; i < tlbEntries; i++ ) {
    
        T64TlbEntry *ptr = &map[ i] ;
        if ( ! ptr -> valid ) continue;

        T64Word vStart1 = ptr -> vAdr;
        T64Word vEnd1   = ptr -> vAdr + ptr -> pageSize - 1;

        T64Word vStart2 = vAdr;
        T64Word vEnd2   = vEnd2 = vAdr + pSize - 1;

        if (( vStart1 <= vEnd2 ) && ( vEnd1 >= vStart2 )) {

            ptr -> valid = false;
        }
    }

    for ( int i = 0; i < tlbEntries; i++ ) {

        if ( ! map[ i ].valid ) {
            
            entry = &map[ i ];
            break;
        }
    }

    if ( entry == nullptr ) {

        for ( int i = 0; i < tlbEntries; i++ ) {

            T64TlbEntry *ptr = &map[ i ];
            if (( ptr -> valid ) && ( ! ptr -> locked ) &&
                ( ptr -> lastUsed < entry -> lastUsed )) {

                    entry = ptr;
            }
        }
    }

    if ( entry == nullptr ) entry = &map[ 0 ];

    entry -> valid        = true;
    entry -> modified     = false;
    entry -> locked       = extractBit64( info, 63 );
    entry -> uncached     = extractBit64( info, 62 );
    entry -> pLev1        = extractBit64( info, 41 );
    entry -> pLev2        = extractBit64( info, 40 );
    entry -> pageType     = (T64PageType) extractField64( info, 42, 2 );
    entry -> pageSize     = pSize;
    entry -> vAdr         = vAdr;
    entry -> pAdr         = pAdr;
    entry -> lastUsed     = timeCounter;

    return( true );
}

//----------------------------------------------------------------------------------------
// Remove the TLB entry that contains the virtual address. The entry is removed
// regardless if it is locked or not.
// 
//----------------------------------------------------------------------------------------
bool T64Tlb::purge( T64Word vAdr ) {
    
    for ( int i = 0; i < tlbEntries; i++ ) {
        
        T64TlbEntry *ptr = &map[ i ];
     
        if ( ptr -> valid ) {

            if ( isInRange( vAdr, 
                            ptr ->vAdr, 
                            ptr -> vAdr + ptr -> pageSize -1  )) {
        
                ptr -> valid = false;
            }
        }
    }

    return( true );
}

//----------------------------------------------------------------------------------------
// "getTlbEntry" is used by the simulator for the display routines.
// 
//----------------------------------------------------------------------------------------
T64TlbEntry *T64Tlb::getTlbEntry( int index ) {
    
    if ( isInRange( index, 0, tlbEntries - 1 )) return( &map[ index ] );
    else                                        return( nullptr );
}

int T64Tlb::getTlbSize( ) {

    return ( tlbEntries );
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
