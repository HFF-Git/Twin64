//----------------------------------------------------------------------------------------
//
// Twin-64 - A 64-bit CPU - Physical memory
//
//----------------------------------------------------------------------------------------
// This module contains the implementation of the physical memory modules.
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

//----------------------------------------------------------------------------------------
// For a threaded simulator, access to the memory must be synchronized. We need
// for writes a mutex. To still have a decent performance, we implement a table
// of mutexes which are indexed bases on the memory address. A write to memory 
// needs to acquire the mutex  before writing writes memory.
//
// std::shared_mutex locks[N];
// size_t idx = ( addr >> 12 ) & ( N - 1 );
//
// class Memory {
//     std::shared_mutex lock;
//    uint8_t data[SIZE];
//
// public:
//    uint64_t read(size_t addr) {
//        std::shared_lock<std::shared_mutex> l(lock);
//        return data[addr];
//    }
//
//    void write(size_t addr, uint64_t value) {
//        std::unique_lock<std::shared_mutex> l(lock);
//        data[addr] = value;
//    }
// };
// 
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------------------
namespace {

} // namespace

//****************************************************************************************
//****************************************************************************************
//
// Physical memory
//
//----------------------------------------------------------------------------------------
// We have a rather simple module for memory. It is just a range of bytes. The 
// SPA range describes where in physical memory this memory is allocated. It is
// possible to have several memory modules, each mapping a different range of 
// physical memory. The read and write function merely copy data from and to 
// memory. The address must however be aligned to the length of the data to 
// fetch.
//
//----------------------------------------------------------------------------------------
T64Memory::T64Memory( T64System     *sys, 
                      int           modNum, 
                      T64MemKind    mKind,
                      T64MemType    mType,
                      T64Word       spaAdr,
                      int           spaLen ) : 

                      T64Module(    MT_MEM, 
                                    modNum,
                                    spaAdr,
                                    spaLen
                                 ) {
    
    this -> sys     = sys;
    this -> mKind   = mKind;
    this -> mType   = mType;
    this -> memData = nullptr;
    reset( );
}

T64Memory:: ~T64Memory( ) {

}

//----------------------------------------------------------------------------------------
// Reset the memory module. We clear out the physical memory range. The easiest
// way is to free the old memory and create a new one.
//
//----------------------------------------------------------------------------------------
void T64Memory::reset( ) {

    if ( memData != nullptr ) free( memData );
    this -> memData  = (uint8_t *) calloc( spaLen, sizeof( uint8_t ));
}

//----------------------------------------------------------------------------------------
// Each module has a step function. Ours does nothing.
// 
//----------------------------------------------------------------------------------------
void T64Memory::step( ) { 
    
}

//----------------------------------------------------------------------------------------
// Read a data from memory. The address the physical address and we compute the
// offset on our SPA range. The address needs to be aligned with length parameter.
//
// ??? this is not thread safe, memcpy is not.
//----------------------------------------------------------------------------------------
bool T64Memory::read( T64Word adr, uint8_t *data, int len ) {

    if ( isInIoAdrRange( adr )) {

         // for now ...
        *data = 0;
        return ( false );
    }
    else {

        if ( adr + len >= spaAdr + spaLen ) return( false );
        if ( ! isAlignedDataAdr( adr, len )) return( false );

        uint8_t *srcPtr = &memData[ adr - spaAdr ];

        memcpy( data, srcPtr, len );
        return( true );
    }
}

//----------------------------------------------------------------------------------------
// Write data to memory. The address the physical address and we compute the 
// offset on our SPA range. The address needs to be aligned with length parameter.
//
// ??? this is not thread safe, memcpy is not.
//----------------------------------------------------------------------------------------
bool T64Memory::write( T64Word adr, uint8_t *data, int len ) {

    if ( isInIoAdrRange( adr )) {

         // for now ...
        *data = 0;
        return ( false );
    }
    else {

        if ( adr + len >= spaAdr + spaLen ) return( false );
        if ( ! isAlignedDataAdr( adr, len )) return( false );
        if ( spaReadOnly ) return ( false );

        uint8_t *dstPtr = &memData[ adr - spaAdr ];

        memcpy( dstPtr, data, len );
        return( true );
    }
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

char *T64Memory::getMemTypeString( ) const {   

    switch ( mType ) {

        case T64_MT_RAM:   return((char *) "RAM" );
        case T64_MT_ROM:   return((char *) "ROM" );
        default:           return((char *) "Unknown Mem Type" );
    }
}

bool T64Memory::busOpReadEvent( int     reqModNum,
                                T64Word pAdr, 
                                uint8_t *data, 
                                int     len ) {
    
    return( read( pAdr, data, len ));
}

bool T64Memory::busOpWriteEvent( int     reqModNum,
                                 T64Word pAdr, 
                                 uint8_t *data, 
                                 int     len ) {
    return( write( pAdr, data, len ));
}
