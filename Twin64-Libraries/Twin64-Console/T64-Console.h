//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Console
//
//----------------------------------------------------------------------------------------
// The Twin64 processor module contains ...
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Processor
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
#pragma once

#include "T64-Common.h"
#include "T64-Util.h"
#include "T64-System.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

//----------------------------------------------------------------------------------------
// Forwards.
//
//----------------------------------------------------------------------------------------
struct T64System;

//----------------------------------------------------------------------------------------
// Processor Options. None defined yet. A place holder. 
//
//----------------------------------------------------------------------------------------
enum T64ConsoleOptions : uint32_t {

    T64_CO_NIL = 0
};

//----------------------------------------------------------------------------------------
// The console module HPA page definitions. There are a couple of register sets. 
// Reg
// set 0 is the processor reg set itself. It contains the module relevant 
// definitions, such as configuration options. 
//
// Reg Set 1 contains the TLB data elements. Each TLB entry is represented by
// two 64-bit words. The first word contains the virtual address and the second
// word contains the physical address and tlb information.
//
//----------------------------------------------------------------------------------------
enum T64ConsoleRegSetOfs : int {

    
};

//----------------------------------------------------------------------------------------
// The Console Module.
//
//----------------------------------------------------------------------------------------
struct T64Console : T64ThreadModule {
    
    public:

    T64Console( T64System           *sys,
                int                 modNum,
                T64ConsoleOptions   options  
              );
    
    virtual         ~ T64Console( );

    void            initModule( ) override;
    void            resetModule( ) override;
    T64TrapCode     executeUnit( ) override;

    T64BusOpStat    busOpReadEvent( T64Word pAdr, 
                                    uint8_t *data, 
                                    size_t len ) override;

    T64BusOpStat    busOpWriteEvent( T64Word pAdr, 
                                     uint8_t *data, 
                                     size_t len ) override; 

    T64BusOpStat    busOpControlEvent( T64BusOpControlEvents id, 
                                       T64Word            arg1, 
                                       T64Word            arg2 ) override;
                        
private:

    T64BusOpStat    handleHPARead( T64Word pAdr, uint8_t *data, size_t len );
    T64BusOpStat    handleHPAWrite( T64Word pAdr, uint8_t *data, size_t len );

    T64BusOpStat    handleControlEvent( T64BusOpControlEvents  event, 
                                        T64Word                 arg1, 
                                        T64Word                 arg2);

    T64System       *sys = nullptr;
};
