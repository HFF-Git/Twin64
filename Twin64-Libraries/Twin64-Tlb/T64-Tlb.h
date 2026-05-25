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
#pragma once

#include "T64-Common.h"
#include "T64-Util.h"
#include "T64-System.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

//----------------------------------------------------------------------------------------
// The Twin64 Simulator features a global TLB used by all processor resources.
// It is a module by itself and follows the module protocols. The TLB data
// is mapped into the HPA address range and can be examined with a simple memory
// display command.
//
//----------------------------------------------------------------------------------------
struct T64GlobalTlb : T64Module {

    public:

    T64GlobalTlb( T64ModuleType    modType, 
                  int              modNum, 
                  T64TlbKind       tlbKind,
                  T64TlbType       tlbType );

    virtual ~T64GlobalTlb( );

    bool        lookupTlb( T64Word vAdr, T64TlbEntry *e );
    bool        insertTlbEntry( T64Word arg1, T64Word arg2 );
    bool        removeTlbEntry( T64Word vAdr );

    void        initModule( );
    void        resetModule( );
    void        haltModule( );
    void        runModule( );
    void        execModule( int steps );
    bool        executeUnit( );
    void        waitUntilHalted( );

    bool        busOpReadEvent( int reqModNum, T64Word pAdr, uint8_t *data, int len );
    bool        busOpWriteEvent( int reqModNum, T64Word pAdr, uint8_t *data, int len );  

    bool        busOpBroadcastEvent( int srcModNum,
                                     T64BroadcastEvents id, 
                                     T64Word            arg1, 
                                     T64Word            arg2 );

    private:

    T64TlbKind          tlbKind;
    T64TlbType          tlbType;
    int                 tlbSize;
    T64TlbEntry         *tlbTable;
    int                 tlbRoundRobin;
    std::shared_mutex   tLock;

    T64Word             tlbMissCount;
    T64Word             tlbHitCount;

};
