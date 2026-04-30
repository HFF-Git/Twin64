//----------------------------------------------------------------------------------------
//
// Twin-64 - A 64-bit CPU - Physical memory
//
//----------------------------------------------------------------------------------------
// A physical memory module is an array of pages. Each module covers a range of
// physical memory and reacts to read and write bus operations. The address range
// is defined by the HPA and SPA address range. A memory module typically uses
// the SPA address range for the memory and the HPA address range for control 
// registers.
//
// Access to the memory is protected by a lock. The lock is held during the entire
// bus operation, including the time it takes to read or write the memory. 
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
#ifndef T64_Memory_h
#define T64_Memory_h

#include "T64-Util.h"
#include "T64-Common.h"
#include "T64-System.h"
#include <atomic>

//----------------------------------------------------------------------------------------
// Memory. There are two basic kinds of memory. ReadWrite and ReadOnly.
//
//----------------------------------------------------------------------------------------
enum T64MemKind : int {

    T64_MK_NIL = 0
};

enum T64MemType : int {

    T64_MT_NIL  = 0,
    T64_MT_RAM  = 1,
    T64_MT_ROM  = 2
};

//----------------------------------------------------------------------------------------
// T64 Memory module object.
//
//----------------------------------------------------------------------------------------
struct T64Memory : T64Module {
    
public:
    
    T64Memory( T64System    *sys, 
               int          modNum, 
               T64MemKind   mKind,
               T64MemType   mType,
               T64Word      spaAdr,
               int          spaLen );

    virtual     ~ T64Memory( );
    
    void        reset( );
    void        step( );
   
    bool        busOpReadEvent( int     reqModNum,
                                T64Word pAdr, 
                                uint8_t *data, 
                                int     len );

    bool        busOpWriteEvent( int     reqModNum,
                                 T64Word pAdr, 
                                 uint8_t *data, 
                                 int     len );

    T64MemKind  getMemKind( ) const;
    T64MemType  getMemType( ) const;
    char        *getMemTypeString( ) const;
    void        setSpaReadOnly( bool arg );
                      
private:

    T64MemKind          mKind       = T64_MK_NIL;
    T64MemType          mType       = T64_MT_NIL;
    T64System           *sys        = nullptr;
    uint8_t             *memData    = nullptr;
    bool                spaReadOnly = false;
    std::atomic<bool>   memLock     = false;
};

#endif // T64-Memory.h