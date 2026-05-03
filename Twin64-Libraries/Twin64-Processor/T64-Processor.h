//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - Processor
//
//----------------------------------------------------------------------------------------
// The Twin64 processor module contains the CPU core, TLBs and caches. The processor
// module connects to the system bus for memory and IO access.
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
// This program is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE.  See the GNU General Public License for more details. You 
// should have received a copy of the GNU General Public License along with this 
// program. If not, see <http://www.gnu.org/licenses/>.
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
struct T64Processor;

//----------------------------------------------------------------------------------------
// Processor Options. None defined yet. A place holder. 
//
//----------------------------------------------------------------------------------------
enum T64Options : uint32_t {

    T64_PO_NIL = 0
};

//----------------------------------------------------------------------------------------
// Caches. Caches are sub modules to the processor. We support a cache type of 
// set associative caches, 2, 4, and 8-way. There is a cache line info with flags
// and the tag and an array of bytes which holds the cache data. Cache kind 
// specifies the kind of cache, i.e. instruction, data or unified cache. Cache 
// configuration is encoded as follows:
//
//  T64_CT_<ways>W_<sets>S_<words>L
//
// Currently, we do not support caches. The simulator will run with a single 
// cycle memory access. We may change this one day to learn about cache 
// coherence and so on.
//
//----------------------------------------------------------------------------------------
enum T64CacheKind : int {

    T64_CK_NIL              = 0,
    T64_CK_INSTR_CACHE      = 1,
    T64_CK_DATA_CACHE       = 2,
    T64_CK_UNIFIED_CACHE    = 3,
};

enum T64CacheType : int {

    T64_CT_NIL              = 0,
    T64_CT_2W_128S_4L       = 1,
    T64_CT_4W_128S_4L       = 2,
    T64_CT_8W_128S_4L       = 3,

    T64_CT_2W_64S_8L        = 4,
    T64_CT_4W_64S_8L        = 5,
    T64_CT_8W_64S_8L        = 6
};

//----------------------------------------------------------------------------------------
// TLBs. A translation lookaside buffer is essential. The TLB kind specifies 
// the kind of TLB, i.e. instruction, data or unified TLB. The TLB type specifies
// type of TLB. Currently there are fully associative TLBs defined. The TLB  
// configuration is encoded as follows:
//
//  T64_TT_<type>_<sets>S
//
// Currently, the processor supports a unified TLB with two small sets of an
// instruction and data translation buffer on top.
// 
//----------------------------------------------------------------------------------------
enum T64TlbKind : int {

    T64_TK_NIL          = 0,
    T64_TK_INSTR_TLB    = 1,
    T64_TK_DATA_TLB     = 2,
    T64_TK_UNIFIED_TLB  = 3
};

enum T64TlbType : int {

    T64_TT_NIL          = 0,
    T64_TT_FA_16S       = 1,
    T64_TT_FA_32S       = 2,
    T64_TT_FA_64S       = 3,
    T64_TT_FA_128S      = 4
};

//----------------------------------------------------------------------------------------
// TLB Entry. The TLB entry stores one translation along with several flags. 
//
//----------------------------------------------------------------------------------------
struct T64TlbEntry {

    public:

    bool            valid           = false;
    bool            uncached        = false;
    bool            locked          = false;
    bool            modified        = false;
    T64PageType     pageType        = PT_NONE;
    bool            pLev1           = false;
    bool            pLev2           = false;
    uint32_t        pageSize        = 0;
    T64Word         pageMask        = 0;
    T64Word         vAdr            = 0;
    T64Word         pAdr            = 0; 
};

//----------------------------------------------------------------------------------------
// The TLB submodule. In the real worlds most processors have an ITLB and DTLB.
// Our TLB consist of three simple arrays of entries. There are two very small
// arrays, the ITLB and DTLB L1 structure. Both are loaded from the unified 
// UTLB. The CPU uses dedicated methods for instruction and data address lookup.
// The insert and purge method apply to the UTLB. The simulator uses these 
// methods for display and directly inserting or removing an UTLB entry. Note
// that ITLB and DTLB are indirectly manipulated by operations on the UTLB.
//
//----------------------------------------------------------------------------------------
struct T64Tlb {
    
    public:

    T64Tlb( T64Processor *proc, T64TlbKind tlbKind, T64TlbType tlbType );

    virtual         ~ T64Tlb( );
    
    void            reset( );

    T64TlbEntry     *lookupItlb( T64Word vAdr );
    T64TlbEntry     *lookupDtlb( T64Word vAdr );
    
    bool            insertTlb( T64Word vAdr, T64Word info );
    bool            purgeTlb( T64Word vAdr );

    int             getUTlbSize( ) const;
    int             getITlbSize( ) const;
    int             getDTlbSize( ) const;

    T64TlbEntry     *getUTLBEntry( int index ) const;
    T64TlbEntry     *getITLBEntry( int index ) const;
    T64TlbEntry     *getDTLBEntry( int index ) const;

    T64TlbKind      getTlbKind( ) const;  
    char            *getTlbKindString( ) const;
    T64TlbType      getTlbType( ) const;
    char            *getTlbTypeString( ) const;

    private:

    T64Processor    *proc               = nullptr;

    T64TlbKind      tlbKind             = T64_TK_NIL;
    T64TlbType      tlbType             = T64_TT_NIL;
    T64TlbEntry     *iTlb               = nullptr;
    T64TlbEntry     *dTlb               = nullptr;
    T64TlbEntry     *uTlb               = nullptr;

    int             iTlbEntries         = 0;
    int             dTlbEntries         = 0;
    int             uTlbEntries         = 0;

    uint32_t        uTlbRoundRobin      = 0;       
    uint32_t        iTlbRoundRobin      = 0;       
    
    T64Word         iTlbHits            = 0;
    T64Word         iTlbMisses          = 0;
    T64Word         iTlbMissUTlbHits    = 0;
    T64Word         iTlbMissUTlbMisses  = 0;

    T64Word         dTlbHits            = 0;
    T64Word         dTlbMisses          = 0;
    T64Word         dTlbMissUTlbHits    = 0;
    T64Word         dTlbMissUTlbMisses  = 0; 
};

//----------------------------------------------------------------------------------------
// CPU. The execution unit. We could support several types. So far, we do not.
//
//----------------------------------------------------------------------------------------
enum T64CpuType : int {

    T64_CPU_T_NIL = 0
};

//----------------------------------------------------------------------------------------
// The CPU is the execution unit of the processor. It provides access to the 
// registers and executes an instruction.
//
//----------------------------------------------------------------------------------------
struct T64Cpu {

    public: 

    T64Cpu( T64Processor *proc, T64CpuType cpuType );

    virtual         ~ T64Cpu( );

    void            reset( );
    void            executeInstr( );

    T64Word         getGeneralReg( int index );
    void            setGeneralReg( int index, T64Word val );

    T64Word         getControlReg( int index );
    void            setControlReg( int index, T64Word val );

    T64Word         getPsrReg( );
    void            setPsrReg( T64Word val );

    private: 

    bool            isPhysMemAdr( T64Word vAdr );
    int             evalCond( int cond, T64Word val1, T64Word val2 );

    void            privModeOperationTrap( );
    void            instrTlbMissTrap( T64Word adr );
    void            instrAlignmentTrap( T64Word adr );
    void            instrMemProtectionTrap( T64Word adr );
    void            dataTlbMissTrap( T64Word adr );
    void            dataAlignmentTrap( T64Word adr );
    void            dataMemProtectionTrap( T64Word adr );
    void            overFlowTrap( );
    void            illegalInstrTrap( );

    void            privModeCheck( );
    bool            regionIdCheck( uint32_t pId, bool wMode );
    void            instrAlignmentCheck( T64Word vAdr );
    void            instrRegionIdCheck( T64Word adr );
    void            instrAccessRightsCheck( T64TlbEntry *tlbPtr, uint8_t accMode );
    void            dataAlignmentCheck( T64Word vAdr, int len );
    void            dataRegionIdCheck( T64Word adr, bool wMode );
    void            dataAccessRightsCheck( T64TlbEntry *tlbPtr, uint8_t accMode );
    void            addOverFlowCheck( T64Word val1, T64Word val2 );
    void            subUnderFlowCheck( T64Word val1, T64Word val2 );

    void            nextInstr( );

    T64Word         getRegR( uint32_t instr );
    T64Word         getRegB( uint32_t instr );
    T64Word         getRegA( uint32_t instr );
    void            setRegR( uint32_t instr, T64Word val );
   
    T64Word         instrRead( T64Word vAdr );
    T64Word         dataRead( T64Word vAdr, int len, bool sExt );
    T64Word         dataReadRegBOfsImm13( uint32_t instr, bool sExt );
    T64Word         dataReadRegBOfsRegX( uint32_t instr, bool sExt );

    void            dataWrite( T64Word vAdr, T64Word val, int len );
    void            dataWriteRegBOfsImm13( uint32_t instr );
    void            dataWriteRegBOfsRegX( uint32_t instr );

    void            instrAluNopOp( T64Instr instr );
    void            instrAluAddOp( T64Instr instr );
    void            instrMemAddOp( T64Instr instr );
    void            instrAluSubOp( T64Instr instr );
    void            instrMemSubOp( T64Instr instr );
    void            instrAluAndOp( T64Instr instr );
    void            instrMemAndOp( T64Instr instr );
    void            instrAluOrOp( T64Instr instr );
    void            instrMemOrOp( T64Instr instr );
    void            instrAluXorOp( T64Instr instr );
    void            instrMemXorOp( T64Instr instr );
    void            instrAluCmpOp( T64Instr instr );
    void            instrMemCmpOp( T64Instr instr );
    void            instrAluBitOp( T64Instr instr );
    void            instrAluShaOP( T64Instr instr );
    void            instrAluImmOp( T64Instr instr );
    void            instrAluLdoOp( T64Instr instr );
    void            instrMemLdOp( T64Instr instr );
    void            instrMemLdrOp( T64Instr instr);
    void            instrMemStOp( T64Instr instr );
    void            instrMemStcOp( T64Instr instr );
    void            instrBrBOp( T64Instr instr );
    void            instrBrBeOp( T64Instr instr );
    void            instrBrBrOp( T64Instr instr );
    void            instrBrBvOp( T64Instr instr );
    void            instrBrBbOp( T64Instr instr );
    void            instrBrAbrOp( T64Instr instr );
    void            instrBrCbrOp( T64Instr instr );
    void            instrBrMbrOp( T64Instr instr );
    void            instrSysMrOp( T64Instr instr );
    void            instrSysLpaOp( T64Instr instr );
    void            instrSysPrbOp( T64Instr instr );
    void            instrSysTlbOp( T64Instr instr );
    void            instrSysCaOp( T64Instr instr );
    void            instrSysMstOp( T64Instr instr );
    void            instrSysRfiOp( T64Instr instr );
    void            instrSysDiagOp( T64Instr instr );
    void            instrSysTrapOp( T64Instr instr );

    void            instrExecute( uint32_t instr );
    void            handleExtInterrupts( );

    T64Word         diagOpHandler( int opt, T64Word arg1, T64Word arg2 );

    T64Word         cRegFile[ T64_MAX_CREGS ];
    T64Word         gRegFile[ T64_MAX_GREGS ];
    T64Word         psrReg;
    uint32_t        instrReg;
    T64Word         resvReg;

    T64CpuType      cpuType = T64_CPU_T_NIL;
    T64Processor    *proc   = nullptr;
   
    T64Word         lowerPhysMemAdr = 0;
    T64Word         upperPhysMemAdr = T64_MAX_PHYS_MEM_LIMIT;
};

//----------------------------------------------------------------------------------------
// Processor states.
//
//----------------------------------------------------------------------------------------
enum T64ProcState : int {

    T64_PROC_STATE_NIL          = 0,
    T64_PROC_STATE_RESET        = 1,
    T64_PROC_STATE_RUNNING      = 2,
    T64_PROC_STATE_SINGLE_STEP  = 3, 
    T64_PROC_STATE_HALTED       = 4,
    T64_PROC_STATE_TERMINATE    = 5    
};

//----------------------------------------------------------------------------------------
// The CPU core executes the instructions. A processor module contains the CPU 
// core, TLBs and caches. The processor module connects to the system bus for 
// memory and IO access. The unit is a single step, i.e. one instruction. A call
// to step must run to completion:
// 
// STEP
//      EXEC
//          SYS.read
//              ... cache / mem work
//          end
//
//          EXEC work
//      END
// END
//
// The processor participates in the cache coherence protocol and has methods that
// are called from the system object.
//
//----------------------------------------------------------------------------------------
struct T64Processor : T64Module {
    
    public:

    T64Processor( T64System         *sys,
                  int               modNum,
                  T64Options        options,  
                  T64CpuType        cpuType,
                  T64TlbType        tlbType,
                  T64CacheType      cacheType,
                  T64Word           spaAdr,
                  int               spaLen );
    
    virtual        ~ T64Processor( );
    
    void            resetModule( );
    void            haltModule( );
    void            startModule( );
    void            stopModule( );

    void            processorThread( );

    void            signal( T64ProcState newState );    

    bool            busOpRead( T64Word adr, 
                              uint8_t *data, 
                              int len );

    bool            busOpWrite( T64Word adr, 
                               uint8_t *data, 
                               int len );

    bool            busOpReadEvent( int     reqModNum,
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len );

    bool            busOpWriteEvent( int     reqModNum,
                                     T64Word pAdr, 
                                     uint8_t *data, 
                                     int     len );  
                        
    T64Cpu          *getCpuPtr( );
    T64Tlb          *getTlbPtr( );
   
private:

    bool            handleHPARead( T64Word pAdr, uint8_t *data, int len );
    bool            handleHPAWrite( T64Word pAdr, uint8_t *data, int len );

    friend struct               T64Cpu;

    T64System                   *sys                    = nullptr;
    T64Cpu                      *cpu                    = nullptr;
    T64Tlb                      *tlb                    = nullptr;
  
    int                         modNum                  = 0;
    T64Word                     instructionCount        = 0;
 
    std::atomic<T64ProcState>   procState { T64_PROC_STATE_NIL };
    std::mutex                  procLock;
    std::condition_variable     procCondVar;
    std::thread                 worker;
};
