//----------------------------------------------------------------------------------------
//
//  Twin64Sim - A 64-bit CPU Simulator - Common utility functions
//
//----------------------------------------------------------------------------------------
// Throughout the Twin64 project we use a few common utility functions. They are
// collected here.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Common Declarations
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
#pragma once
#include "T64-Common.h"

//----------------------------------------------------------------------------------------
// Byte order conversion functions. They are different on Mac and Windows and LINUX,
// GCC and CLANG.
//
//----------------------------------------------------------------------------------------
#if defined(__APPLE__)
  #include <libkern/OSByteOrder.h>
  #define HOST_IS_BIG_ENDIAN  (__BIG_ENDIAN__)
#elif defined(_WIN32)
  #define HOST_IS_BIG_ENDIAN  0
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  #define HOST_IS_BIG_ENDIAN  1
#else
  #define HOST_IS_BIG_ENDIAN  0
#endif

#if HOST_IS_BIG_ENDIAN
    inline uint16_t toBigEndian16(uint16_t val) { return val; }
    inline uint32_t toBigEndian32(uint32_t val) { return val; }
    inline uint64_t toBigEndian64(uint64_t val) { return val; }
#else
  #if defined(__APPLE__)
    #include <libkern/OSByteOrder.h>
    inline uint16_t toBigEndian16(uint16_t val) { return OSSwapHostToBigInt16(val); }
    inline uint32_t toBigEndian32(uint32_t val) { return OSSwapHostToBigInt32(val); }
    inline uint64_t toBigEndian64(uint64_t val) { return OSSwapHostToBigInt64(val); }
  #elif defined(_MSC_VER)
    #include <intrin.h>
    inline uint16_t toBigEndian16(uint16_t val) { return _byteswap_ushort(val); }
    inline uint32_t toBigEndian32(uint32_t val) { return _byteswap_ulong(val); }
    inline uint64_t toBigEndian64(uint64_t val) { return _byteswap_uint64(val); }
  #else
    inline uint16_t toBigEndian16(uint16_t val) { return __builtin_bswap16(val); }
    inline uint32_t toBigEndian32(uint32_t val) { return __builtin_bswap32(val); }
    inline uint64_t toBigEndian64(uint64_t val) { return __builtin_bswap64(val); }
  #endif
#endif

//----------------------------------------------------------------------------------------
// Helper functions.
//
//----------------------------------------------------------------------------------------
inline bool isInRange( T64Word adr, T64Word low, T64Word high ) {
    
    return (( adr >= low ) && ( adr <= high ));
}

inline T64Word roundup(T64Word arg, int round) {

    if ( round == 0 ) return arg;
    return (( arg + round - 1 ) / round ) * round;
}

inline T64Word rounddown( T64Word arg, int round ) {

    if (round == 0) return arg;
    return (arg / round) * round;
}

inline bool isAlignedDataAdr( T64Word adr, int align ) {

    if (( align == 1 ) || ( align == 2 ) || 
        ( align == 4 ) || ( align == 8 )) {
        
        return (( adr & ( align - 1 )) == 0 );
    }
    else return( false );
}

inline bool isAlignedPageAdr( T64Word adr, int align ) {

    if (( align == T64_PAGE_SIZE_BYTES                  ) ||
        ( align == 16 * T64_PAGE_SIZE_BYTES             ) ||
        ( align == 16 * 16 * T64_PAGE_SIZE_BYTES        ) ||
        ( align == 16 * 16 * 16 * T64_PAGE_SIZE_BYTES   )) {

            return (( adr & ( align - 1 )) == 0 );
        }
        else return( false );
}

inline bool isAlignedInstrAdr( T64Word adr ) {

    return (( adr & 0x3 ) == 0 );
}

inline bool isAlignedOfs( T64Word ofs,  int align ) {

    return (( ofs & ( align - 1 )) == 0 );
}

//----------------------------------------------------------------------------------------
// "copyToBigEndian" converts the input data in little endian format to big endian
// format. Both source and destination must be aligned to the length of the data
// input.
//
//----------------------------------------------------------------------------------------
inline bool copyToBigEndian( uint8_t *dst, uint8_t *src, int len ) {

    if (( len != 1 ) && ( len != 2 ) && 
        ( len != 4 ) && ( len != 8 )) return( false );     

    if (((uintptr_t)dst & ( len - 1 )) != 0) return false;
    if (((uintptr_t)src & ( len - 1 )) != 0) return false;

    switch ( len ) {

        case 1: {
            
            *dst = *src;

        } break;

        case 2: {

            uint16_t val;
            memcpy( &val, src, sizeof( val ));
            val = toBigEndian16( val );
            memcpy( dst, &val, sizeof( val ));
            
        } break;

        case 4: {

            uint32_t val;
            memcpy( &val, src, sizeof( val ));
            val = toBigEndian32( val );
            memcpy( dst, &val, sizeof( val ));
    
        } break;

        case 8: {

            uint64_t val;
            memcpy( &val, src, sizeof( val ));
            val = toBigEndian64( val );
            memcpy( dst, &val, sizeof( val ));
           
        } break;
    }

    return( true );
}

//----------------------------------------------------------------------------------------
// Helper function to check a bit range value in the instruction.
//
//----------------------------------------------------------------------------------------
inline bool isInRangeForInstrBitFieldS( int val, int bitLen ) {
    
    int min = - ( 1 << (( bitLen - 1 ) % 64 ));
    int max = ( 1 << (( bitLen - 1 ) % 64 )) - 1;
    return (( val <= max ) && ( val >= min ));
}

inline bool isInRangeForInstrBitFieldU( uint32_t val, int bitLen ) {
    
    uint32_t max = (( 1 << ( bitLen % 64 )) - 1 );
    return ( val <= max );
}

//----------------------------------------------------------------------------------------
// Instruction field routines.
//
//----------------------------------------------------------------------------------------
inline int extractInstrBit( T64Instr arg, int bitpos ) {
    
    if ( bitpos > 31 ) return ( 0 );
    return (( arg >> bitpos ) & 0x1 );
}

inline int extractInstrFieldU( T64Instr arg, int bitpos, int len ) {
    
    if ( bitpos > 31 ) return ( 0 );
    if ( bitpos + len > 32 ) return ( 0 );
    return (( arg >> bitpos ) & (( 1L << len ) - 1 ));
}

inline int extractInstrFieldS( T64Instr arg, int bitpos, int len ) {

    if ( len == 0 ) return ( 0 );
    uint64_t field = (arg >> bitpos) & ((1ULL << len) - 1);
    return (int)((int32_t)(field << (32 - len)) >> (32 - len));
}

inline T64Word signExtend( T64Word data, int pos ) {

    T64Word mask = (T64Word)1 << pos;       
    T64Word extend = ~(mask - 1);       

    return (data & mask) ? (data | extend) : (data & ~extend);
} 

inline int extractInstrOpGroup( T64Instr instr ) {
    
    return ( extractInstrFieldU( instr, 30, 2 ));
}

inline int extractInstrOpCode( T64Instr instr ) {
    
    return ( extractInstrFieldU( instr, 26, 4 ));
}

inline int extractInstrOptField( T64Instr instr ) {
    
    return ( extractInstrFieldU( instr, 19, 3 ));
}

inline int extractInstrRegR( T64Instr instr ) {
    
    return ( extractInstrFieldU( instr, 22, 4 ));
}

inline int extractInstrRegB( T64Instr instr ) {
    
    return ( extractInstrFieldU( instr, 15, 4 ));
}

inline int extractInstrRegA( T64Instr instr ) {
    
    return ( extractInstrFieldU( instr, 9, 4 ));
}

inline int extractInstrDwField( T64Instr instr) {
    
    return ( extractInstrFieldU( instr, 13, 2 ));
}

inline int extractInstrSignedImm13( T64Instr instr ) {
    
    return ( extractInstrFieldS( instr, 0, 13 ));
}

inline int extractInstrSignedScaledImm13( T64Instr instr ) {
    
    return ( extractInstrSignedImm13( instr ) << extractInstrDwField( instr ));
}

inline int extractInstrSignedImm15( T64Instr instr ) {
    
    return ( extractInstrFieldS( instr, 0, 15 ));
}

inline int extractInstrSignedImm19( T64Instr instr ) {
    
    return ( extractInstrFieldS( instr, 0, 19 ));
}

inline int extractInstrImm20( T64Instr instr ) {

    return ( extractInstrFieldU( instr, 0, 20 ));
}

//----------------------------------------------------------------------------------------
// Helper functions for depositing value in the instruction. The are mainly used 
// by the inline assembler.
//
//----------------------------------------------------------------------------------------
inline void depositInstrField( T64Instr *instr, int bitpos, int len, T64Word value ) {
    
    uint32_t mask = (( 1 << len ) - 1 ) << bitpos;
    *instr = (( *instr & ~ mask ) | (( value << bitpos ) & mask ));
}

inline void depositInstrBit( T64Instr *instr, int bitpos, bool value ) {
    
    uint32_t mask = 1 << bitpos;
    *instr = (( *instr & ~mask ) | (( value << bitpos ) & mask ));
}

inline void depositInstrRegR( T64Instr *instr, uint32_t regId ) {
    
    depositInstrField( instr, 22, 4, regId );
}

inline void depositInstrRegB( T64Instr *instr, uint32_t regId ) {
    
   depositInstrField( instr, 15, 4, regId );
}

inline void depositInstrRegA( T64Instr *instr, uint32_t regId ) {
    
    depositInstrField( instr, 9, 4, regId );
}

//----------------------------------------------------------------------------------------
// General extract, deposit and shift functions.
//
//----------------------------------------------------------------------------------------
inline T64Word extractBit64( T64Word arg, int bitpos ) {
    
    if ( bitpos > 63 ) return ( 0 );
    return ( arg >> bitpos ) & 1;
}

inline T64Word extractField64( T64Word arg, int bitpos, int len ) {
    
    if ( bitpos > 63 ) return ( 0 );
    if ( bitpos + len > 64 ) return ( 0 );
    return ( arg >> bitpos ) & (( 1LL << len ) - 1 );
}

inline T64Word extractSignedField64( T64Word arg, int bitpos, int len ) {
    
    T64Word field = ( arg >> bitpos ) & (( 1ULL << len ) - 1 );
    
    if ( len < 64 )  return ( field << ( 64 - len )) >> ( 64 - len );
    else             return ( field );
}

inline T64Word depositField( T64Word word, int bitpos, int len, T64Word value ) {
    
    T64Word mask = (( 1ULL << len ) - 1 ) << bitpos;
    return ( word & ~mask ) | (( value << bitpos ) & mask );
}

inline T64Word shiftRight128( T64Word hi, T64Word lo, int shift ) {
    
    if      ( shift == 0 ) return ( lo );
    else if (( shift > 0 ) && ( shift < 64 )) {
        
        return (((uint64_t) hi << (64 - shift)) | ((uint64_t) lo >> shift));
    }
    else return ( lo );
}

//----------------------------------------------------------------------------------------
// Signed 64-bit numeric operations and overflow check.
//
//----------------------------------------------------------------------------------------
inline bool willAddOverflow( T64Word a, T64Word b ) {
    
    if (( b > 0 ) && ( a > INT64_MAX - b )) return true;
    if (( b < 0 ) && ( a < INT64_MIN - b )) return true;
    return false;
}

inline bool willSubOverflow( T64Word a, T64Word b ) {
    
    if (( b < 0 ) && ( a > INT64_MAX + b )) return true;
    if (( b > 0 ) && ( a < INT64_MIN + b )) return true;
    return false;
}

inline bool willMultOverflow( T64Word a, T64Word b ) {

    if (( a == 0 ) ||( b == 0 )) return ( false );

    if (( a == INT64_MIN ) && ( b == -1 )) return ( true );
    if (( b == INT64_MIN ) && ( a == -1 )) return ( true );

    if ( a > 0 ) {

        if ( b > 0 ) return ( a > INT64_MAX / b );
        else         return ( b < INT64_MIN / a );
    }
    else {

        if ( b > 0 ) return ( a < INT64_MIN / b );
        else         return ( a < INT64_MAX / b );
    }
}

inline bool willDivOverflow( T64Word a, T64Word b ) {

    if ( b == 0 ) return ( true );

    if (( a == INT64_MIN ) && ( b == -1 )) return ( true );

    return ( false );
}

inline bool willShiftLeftOverflow( T64Word val, int shift ) {
    
    if (( shift < 0 ) || ( shift >= 63 ))   return ( true );
    if ( shift == 0 )                       return ( false );
    
    T64Word shifted     = val << shift;
    T64Word recovered   = shifted >> shift;
    
    return ( recovered != val );
}

//----------------------------------------------------------------------------------------
// Extract functions for virtual addresses.
//
//----------------------------------------------------------------------------------------
inline T64Word vAdrRegionId( T64Word vAdr ) {

    return( extractField64( vAdr, 32, 20 ));
}

inline T64Word vAdrRegionOfs( T64Word vAdr ) {

   return( extractField64( vAdr, 0, 32 ));
}

inline T64Word vAdrPageNum( T64Word vAdr ) {

   return( extractField64( vAdr, 12, 40 ));
}

inline T64Word vAdrPageOfs( T64Word vAdr ) {

   return( extractField64( vAdr, 0, 12 ));
}

//----------------------------------------------------------------------------------------
// Extract functions for the PSR status bits.
//
//----------------------------------------------------------------------------------------
inline bool extractPsrMbit( T64Word psr ) {

    return( extractBit64( psr, 63 ));
}

inline bool extractPsrXbit( T64Word psr ) {

    return( extractBit64( psr, 61 ));
}

// ??? more to come, align with document...

//----------------------------------------------------------------------------------------
// Address arithmetic. Address are computed using an unsigned 32-bit arithmetic.
// The address "adr" is a 64 bit address to which we will add in the offset portion
// the 32-bit signed "ofs" value. 
//
//----------------------------------------------------------------------------------------
inline T64Word addAdrOfs32( T64Word adr, T64Word ofs ) {
    
    uint64_t uadr   = (uint64_t) adr;
    uint32_t lo     = (uint32_t) uadr;
    uint32_t newLo  = lo + (uint32_t) ofs;

    uint64_t result = (uadr & 0xFFFFFFFF00000000ULL) | (uint64_t)newLo;
    return (T64Word) result;
}

//----------------------------------------------------------------------------------------
// Virtual address range check.
//
//----------------------------------------------------------------------------------------
inline bool isInIoAdrRange( T64Word adr ) {

    return(( adr >= T64_IO_MEM_START ) && ( adr <= T64_IO_MEM_LIMIT ));
}

inline bool isInPhysMemAdrRange( T64Word adr ) {

    return(( adr >= 0 ) && ( adr <= T64_MAX_PHYS_MEM_LIMIT ));
}
