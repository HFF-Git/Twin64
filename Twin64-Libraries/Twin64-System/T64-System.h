//----------------------------------------------------------------------------------------
//
// Twin-64 - System
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
//  Twin-64 - System
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
#ifndef T64_System_h
#define T64_System_h

#include "T64-Common.h"
#include "T64-Util.h"

// ??? on a step, all processor modules advance.
// ??? after that each module is give a change to do processing. I.e. check for
// a key pressed, etc.
// ??? we also need to handle interrupts too...

//----------------------------------------------------------------------------------------
// The architecture defines 64 module on the system bus so far. Typically the number
// of imaginary boards is much smaller. However, a board could have several modules on
// one board. To the software this transparent.
//
//----------------------------------------------------------------------------------------
const int MAX_MODULES           = 16;
const int MAX_MOD_MAP_ENTRIES   = MAX_MODULES;

//----------------------------------------------------------------------------------------
// Modules have a type, submodules a subtype.
//
//----------------------------------------------------------------------------------------
enum T64ModuleType {

    MT_NIL          = 0,
    MT_PROC         = 10,
    MT_CPU_CORE     = 11,
    MT_CPU_TLB      = 12, 
    MT_MEM          = 20,
    MT_IO           = 30   
};

//----------------------------------------------------------------------------------------
// Modules have registers in their HPA.
//
//----------------------------------------------------------------------------------------
// 0 - status
// 1 - command
// 2 - HPA address
// 3 - SPA address
// 4 - SPA len
// 5 - number of I/O elements
// 6 - module hardware version 
// 7 - module software version
// 8 - interrupt target ( when sending an interrupt -> processor + mask )

// ?? the HPA also has a the IODC, a piece that describes the IO Module and 
// code to execute module specific functions.

// I/O Element. Allocated in SPA space. Up to 128 bytes in size -> 16 Regs. 
// ??? need for a larger I/O element ?

// SPA can be USER mode too and directly mapped to user segments, etc.

//----------------------------------------------------------------------------------------
// The T64Module object represents and an object in the system. It is the base class
// for all concrete modules and reacts to bus operations. Each module has a HPA
// address range and an optional SPA address range in I/O memory.
//
//----------------------------------------------------------------------------------------
struct T64Module {
    
    public:

    T64Module( T64ModuleType    modType, 
               int              modNum,
               T64Word          spaAdr,
               int              spaLen  );

    virtual void    reset( ) = 0;
    virtual void    step( ) = 0;

    // ??? each module needs to implement a handler to a bus event.

    virtual bool    busOpReadEvent( int     srcModNum,
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len ) = 0;

    virtual bool    busOpWriteEvent( int     srcModNum,
                                     T64Word pAdr, 
                                     uint8_t *data, 
                                     int     len ) = 0;

    T64ModuleType   getModuleType( );
    int             getModuleNum( );
    const char      *getModuleTypeName( );
    T64Word         getHpaAdr( );
    int             getHpaLen( );
    T64Word         getSpaAdr( );
    int             getSpaLen( );

    public: 

    T64ModuleType   moduleTyp   = MT_NIL;
    int             moduleNum   = 0;
    T64Word         hpaAdr      = 0;
    int             hpaLen      = 0;
    T64Word         spaAdr      = 0;
    int             spaLen      = 0;
    T64Word         spaLimit    = 0;
};

//----------------------------------------------------------------------------------------
// Each module is stored in the module map. Since module is an abstract class the 
// module map cannot be just an array of modules. We package it into a struct.
//
//----------------------------------------------------------------------------------------
struct T64ModuleMapEntry {

    T64Module *module = nullptr;
};

//----------------------------------------------------------------------------------------
// A T64 system is a bus where you plug in modules. A module represents an entity such
// as a processor, a memory module, an I/O module and so on. At program start we create
// the module objects and add them to the systemMap and moduleMap. 
//
//----------------------------------------------------------------------------------------
struct T64System {

    public: 

    T64System( );

    int                 getSystemState( );

    int                 addToModuleMap( T64Module *module );
    int                 removeFromModuleMap( T64Module *module );
    
    T64ModuleType       getModuleType( int modNum ) const;
    T64Module           *lookupByModNum( int modNum ) const;
    T64Module           *lookupByAdr( T64Word adr ) const;                

    void                reset( );
    void                run( );
    void                step( int steps, int modNum );

    bool                busOpRead( int reqModNum,
                                   T64Word pAdr, 
                                   uint8_t *data, 
                                   int     len );

    bool                busOpWrite( int reqModNum,
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len );

    private:

    void                initModuleMap( );
                            
    T64Module           *moduleMap[ MAX_MOD_MAP_ENTRIES ];
    T64Module           *systemMemMap[ MAX_MOD_MAP_ENTRIES ];
    int                 systemMemMapHwm = 0;
};

#endif