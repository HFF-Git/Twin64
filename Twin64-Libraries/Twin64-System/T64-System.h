//----------------------------------------------------------------------------------------
//
// Twin-64 - System
//
//----------------------------------------------------------------------------------------
// The T64-System represent the system consisting of several modules. Modules 
// are for example processor, memory and I/O modules. The simulator is connected
// to the system which handles all module functions. A program start, the all
// modules are registered to the system. Think of a kind of bus where you plug
// in boards.  
//
//----------------------------------------------------------------------------------------
//
//  Twin-64 - System
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
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

//----------------------------------------------------------------------------------------
// The architecture defines 64 module on the system bus so far. Typically the 
// number of imaginary boards is much smaller. However, a board could have 
// several modules on one board. To the software this transparent.
//
//----------------------------------------------------------------------------------------
const int MAX_MOD_MAP_ENTRIES   = T64_IO_MAX_MODULES;

//----------------------------------------------------------------------------------------
// Modules have a type, submodules a subtype.
//
//----------------------------------------------------------------------------------------
enum T64ModuleType {

    MT_NIL          = 0,
    MT_PROC         = 10,
    MT_CPU_CORE     = 12,
    MT_CPU_TLB      = 13, 
    MT_GTLB         = 20,
    MT_MEM          = 30,
    MT_IO           = 40   
};

//----------------------------------------------------------------------------------------
// Modules send a control event to the system. They all will run serialized.
//
//----------------------------------------------------------------------------------------
enum T64BBusOpControlEvents {

    T64_CNTRL_EVENT_MODULE_PURGE  = 1,
    T64_CNTRL_EVENT_TLB_PURGE     = 2,
    T64_CNTRL_EVENT_TLB_INSERT    = 3,
    T64_CNTRL_EVENT_STORE_OP      = 4
};

//----------------------------------------------------------------------------------------
// Module state.
//
//----------------------------------------------------------------------------------------
enum T64ModuleState : int {

    T64_MOD_STATE_NIL          = 0,
    T64_MOD_STATE_RESET        = 1,
    T64_MOD_STATE_EXECUTE      = 2, 
    T64_MOD_STATE_HALTED       = 3,
    T64_MOD_STATE_TERMINATE    = 4    
};

//----------------------------------------------------------------------------------------
// Modules have registers in their HPA. The can be accessed via load / store 
// instructions. 
//
// ??? I/O Elements also use these names ?
//----------------------------------------------------------------------------------------
enum T64ModuleRegs : int {

    T64_MREG_STATUS     = 0,
    T64_MREG_COMMAND    = 1,
    T64_MREG_EIR        = 2,
    T64_MREG_CONFIG     = 3,
    T64_MREG_VERSION    = 4,
    T64_MREG_SPA_ADR_1  = 5,
    T64_MREG_SPA_LEN_1  = 6,
    T64_MREG_SPA_ADR_2  = 7,
    T64_MREG_SPA_LEN_2  = 8
};

//----------------------------------------------------------------------------------------
// The T64Module object represents and an object in the system. It is the base 
// class for all concrete modules and reacts to bus operations. Each module has
// a HPA address range and an optional SPA address range in I/O memory. 
//
//----------------------------------------------------------------------------------------
struct T64Module {
    
    public:

    T64Module( T64ModuleType    modType, 
               int              modNum,
               T64Word          spaAdr,
               int              spaLen  );

    virtual             ~T64Module()            = default;

    virtual void        initModule( )           = 0;
    virtual void        resetModule( )          = 0;

    virtual bool        
    busOpReadEvent( T64Word pAdr, uint8_t *data, int len ) = 0;

    virtual bool        
    busOpWriteEvent( T64Word pAdr, uint8_t *data, int len ) = 0;

    virtual bool        
    busOpControlEvent( T64BBusOpControlEvents event, 
                       T64Word  arg1, T64Word arg2 ) = 0;

    T64ModuleType       getModuleType( );
    int                 getModuleNum( );
    const char          *getModuleTypeName( );
    
    T64Word             getHpaAdr( );
    int                 getHpaLen( );
    T64Word             getSpaAdr( );
    int                 getSpaLen( );

    public: 

    T64ModuleType       moduleTyp   = MT_NIL;
    int                 moduleNum   = 0;
    
    protected: 

    T64Word             hpaAdr      = 0;
    int                 hpaLen      = 0;

    T64Word             spaAdr      = 0;
    int                 spaLen      = 0;

    bool                rsvValid    = false;
    T64Word             rsvInfo     = 0;

    // ??? work in progress ... how do we best represent the regs in a module ?
    // ??? we should have the fields that are common to every module...

    T64Word             mrStatus;
    T64Word             mrCommand;
    T64Word             mrData;
    T64Word             mrEir;
    T64Word             mrConfig;
    T64Word             mrVersion;
    T64Word             mrType;
    T64Word             mrId;
};

//----------------------------------------------------------------------------------------
// The processor thread module implements the thread logic for our processors. 
// The inheriting processor module is required to implement the "execModule" 
// method, which actually executes instructions for a processor. 
//
//----------------------------------------------------------------------------------------
struct T64ProcThreadModule : T64Module {

    public:

    T64ProcThreadModule( T64ModuleType    modType, 
                         int              modNum,
                         T64Word          spaAdr,
                         int              spaLen );

    ~ T64ProcThreadModule( );

    virtual void            initModule( );
    virtual void            resetModule( );
    virtual void            haltModule( );
    virtual void            runModule( );
    virtual void            execModule( int steps, bool haltOnTrap );
    virtual T64TrapCode     waitUntilStopped( );
    
    virtual T64TrapCode     executeUnit( ) = 0;

    T64ModuleState          getModuleState( );
    T64TrapCode             getTrapCode( );
    void                    setEnterSimOnTrap( bool val );
  
    void                    setRsvInfo( T64Word pAdr, bool valid );
    T64Word                 getRsvInfo( );
    bool                    isRsvValid( );

    private: 

    void                    setModuleState( T64ModuleState state );
    void                    moduleWorker( );

    std::atomic<T64ModuleState> mState { T64_MOD_STATE_NIL };
    std::mutex                  mLock;
    std::condition_variable     mCondVar;
    std::thread                 mWorker;
    T64TrapCode                 mTrapCode      = NO_TRAP;
    int                         mUnitCount     = 0;
    bool                        enterSimOnTrap = false;
    bool                        rsvValid       = false;
    T64Word                     rsvInfo        = 0;
};

//----------------------------------------------------------------------------------------
// Each module is stored in the module map. Since module is an abstract class 
// the module map cannot be just an array of modules. We package it into a 
// struct.
//
//----------------------------------------------------------------------------------------
struct T64ModuleMapEntry {

    T64Module *module = nullptr;
};

//----------------------------------------------------------------------------------------
// A T64 system is a bus where you plug in modules. A module represents an 
// entity such as a processor, a memory module, an I/O module and so on. At 
// program start we create the module objects and add them to the respective 
// maps. 
//
//----------------------------------------------------------------------------------------
struct T64System {

    public: 

    T64System( );

    int                 getSystemState( );

    int                 addModule( T64Module *module );
    int                 removeModule( T64Module *module );

    void                resetModule( int modNum );
    void                haltModule( int modNum );
    void                runModule( int modNum );
    void                execModule( int modNum, int steps, bool haltOnTrap );
    bool                isModuleHalted( int modNum );   
    
    void                run( );
    
    T64ModuleType       getModuleType( int modNum ) const;
    char                *getModuleStateStr( int modNum ) const;
    T64Module           *lookupByModNum( int modNum ) const;
    T64Module           *lookupByModuleType( T64ModuleType typ );
    T64Module           *lookupByAdr( T64Word adr ) const;  
    
    bool                translateAdr( T64Word vAdr, T64Word *pAdr );

    bool                busOpRead(  T64Module *mod, 
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int len );

    bool                busOpReadRsv( T64Module *mod, 
                                      T64Word pAdr, 
                                      uint8_t *data, 
                                      int len );

    bool                busOpWrite( T64Module *mod, 
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int len );

    bool                busOpWriteCond( T64Module *mod,
                                        T64Word pAdr, 
                                        uint8_t *data, 
                                        int     len );    

    bool                busOpControl( T64Module *mod,
                                      T64BBusOpControlEvents event,
                                      T64Word             arg1, 
                                      T64Word             arg2 );

    private:

    void                initModuleMap( );
                            
    T64Module           *moduleMap[ MAX_MOD_MAP_ENTRIES ];

    T64Module           *systemPhysMemMap[ MAX_MOD_MAP_ENTRIES * 2 ];
    int                 systemPhysMemMapHwm = 0;

    T64Module           *systemIoMemMap[ MAX_MOD_MAP_ENTRIES * 2 ];
    int                 systemIoMemMapHwm = 0;

    T64Module           *systemProcMap[ MAX_MOD_MAP_ENTRIES ];
    int                 systemProcMapHwm;

    std::mutex          sLock;
};
