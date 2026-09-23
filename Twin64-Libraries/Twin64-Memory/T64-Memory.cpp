//----------------------------------------------------------------------------------------
//
// Twin-64 - A 64-bit CPU - Physical memory
//
//----------------------------------------------------------------------------------------
// This module contains the implementation of the physical memory modules. We 
// have a rather simple module for memory. It is just a range of bytes. The 
// SPA range describes where in physical memory this memory is allocated. It is
// possible to have several memory modules, each mapping a different range of 
// physical memory. The read and write function merely copy data from and to 
// memory. The address must however be aligned to the length of the data to 
// fetch. The memory is protected by a lock, which is held during the entire 
// read or write operation.
//
//----------------------------------------------------------------------------------------
//
// Twin-64 - A 64-bit CPU - Physical memory
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
#include "T64-Memory.h"

// ??? we need a mechanism for the LDR/STC instructions. Since we do not have a
// cache, we need to have a way to check a reservation address whether it
// is still valid. We can do this by having a reservation address and a reservation
// flag. 

// ??? the issue is in contrast to a cache line, that memory can be quite large.
// What is a good way to divided the memory range into smaller chunks, so that we
// can check whether a reservation is still valid. ????

// ??? as an alternative, we could have a reservation address and a flag for 
// each processor accessing this memory. This would limit the number of entries
// to the number of processors, which can be checked easy and quick. 

// ??? to think about...

//****************************************************************************************
//****************************************************************************************
//
// Physical memory
//
//----------------------------------------------------------------------------------------
// Object constructor. We need to initialize the memory data and the lock.
//
//----------------------------------------------------------------------------------------
T64Memory::T64Memory( T64System     *sys, 
                      int           modNum, 
                      T64MemKind    mKind,
                      T64MemType    mType,
                      T64Word       spaAdr,
                      T64Word       spaLen ) : 

                      T64Module(    sys,
                                    MT_MEM, 
                                    modNum,
                                    spaAdr,
                                    spaLen
                                 ) {
    
    this -> sys     = sys;
    this -> mKind   = mKind;
    this -> mType   = mType;
    this -> memData = nullptr;
    this -> memLock = false;

    resetModule( );
}

//----------------------------------------------------------------------------------------
// Object destructor. We need to free the memory we allocated.
//
//----------------------------------------------------------------------------------------
T64Memory:: ~T64Memory( ) { 

    if ( memData != nullptr ) free( memData );
}

//----------------------------------------------------------------------------------------
// Reset the memory module. We clear out the physical memory range. The easiest
// way is to free the old memory and create a new one.
//
//----------------------------------------------------------------------------------------
void T64Memory::initModule( ) { 

    resetModule( );
}

void T64Memory::resetModule( ) {

    if ( memData != nullptr ) free( memData );
    this -> memData  = (uint8_t *) calloc( spaLen, sizeof( uint8_t ));
}

//----------------------------------------------------------------------------------------
// Read a data from memory. The address the physical address and we compute the
// offset on our SPA range. The address needs to be aligned with length parameter.
//
//----------------------------------------------------------------------------------------
bool T64Memory::busOpReadEvent( T64Word pAdr, uint8_t *data, size_t len ) {

    if ( isInIoAdrRange( pAdr )) {

        memset( data, 0, len );
        return ( false );
    }
    else {

        std::atomic<bool> *lockPtr = &memLock;
        while ( lockPtr -> exchange( true )) { /* spin */ };
            
         if ( pAdr + len >= spaAdr + spaLen ) {
            lockPtr -> store( false );
            return( false );
        }

        if ( ! isAlignedAdr( pAdr, len )) {
            lockPtr -> store( false );
            return( false );
        }
       
        uint8_t *srcPtr = &memData[ pAdr - spaAdr ];
        memcpy( data, srcPtr, len );
        lockPtr -> store( false );
        return( true );
    }
}

//----------------------------------------------------------------------------------------
// Write data to memory. The address the physical address and we compute the 
// offset on our SPA range. The address needs to be aligned with length parameter.
//
//----------------------------------------------------------------------------------------
bool T64Memory::busOpWriteEvent( T64Word pAdr, uint8_t *data, size_t len ) {

    if ( isInIoAdrRange( pAdr )) {

        memset( data, 0, len );
        return ( false );
    }
    else {

        std::atomic<bool> *lockPtr = &memLock;
        while ( lockPtr -> exchange( true )) { /* spin */ };

        if ( pAdr + len >= spaAdr + spaLen ) {
            lockPtr -> store( false );
            return( false );
        }
        if ( ! isAlignedAdr( pAdr, len )) {
            lockPtr -> store( false );
            return( false );
        }

        if ( spaReadOnly ) {
            lockPtr -> store( false );
            return ( false );
        }

        uint8_t *dstPtr = &memData[ pAdr - spaAdr ];
        memcpy( dstPtr, data, len );
        lockPtr -> store( false );
        return( true );
    }
}

bool T64Memory::busOpControlEvent( T64BBusOpControlEvents id, 
                                   T64Word            arg1, 
                                   T64Word            arg2 ) {

    return( true );
}

//----------------------------------------------------------------------------------------
// A memory address range can be set road only, This is used when we model a ROM.
//
//----------------------------------------------------------------------------------------
void  T64Memory::setSpaReadOnly( bool arg ) {

    spaReadOnly = arg;
}

//----------------------------------------------------------------------------------------
// Getters for memory kind and type.
//
//----------------------------------------------------------------------------------------
T64MemKind T64Memory::getMemKind( ) const {   

    return( mKind );
}           

T64MemType T64Memory::getMemType( ) const {   

    return( mType );
}

const char *T64Memory::getMemTypeString( ) const {   

    switch ( mType ) {

        case T64_MT_RAM:   return( "RAM" );
        case T64_MT_ROM:   return( "ROM" );
        default:           return( "Unknown Mem Type" );
    }
}
