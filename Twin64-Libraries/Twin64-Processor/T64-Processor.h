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
#include "T64-Tlb.h"
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
// The processor HPA page definitions. There are a couple of register sets. Reg
// set 0 is the processor reg set itself. It contains the module relevant 
// definitions, such as configuration options. 
//
// Reg Set 1 contains the TLB data elements. Each TLB entry is represented by
// two 64-bit words. The first word contains the virtual address and the second
// word contains the physical address and tlb information.
//
//----------------------------------------------------------------------------------------
enum T64ProcRegSetOfs : int {

    T64_IO_TLB_STATUS_REG_OFS   = 8,
    T64_IO_TLB_CONFIG_REG_OFS   = 9,

    T64_IO_ITLB_HITS_OFS        = 12,
    T64_IO_ITLB_MISSES_OFS      = 13,
    T64_IO_ITLB_GTLB_HITS_OFS   = 14,
    T64_IO_ITLB_GTLB_MISSES_OFS = 15,

    T64_IO_DTLB_HITS_OFS        = 16,
    T64_IO_DTLB_MISSES_OFS      = 17,
    T64_IO_DTLB_GTLB_HITS_OFS   = 18,
    T64_IO_DTLB_GTLB_MISSES_OFS = 19
};

//----------------------------------------------------------------------------------------
// A processor maintains a local ITLB and DTLB. These are small sets of TLB
// entries to consult for each access.
//
//----------------------------------------------------------------------------------------
struct T64LocalTlb {

 public:

    T64LocalTlb( T64Processor *proc, T64TlbKind tlbKind, T64TlbType tlbType );

    virtual         ~ T64LocalTlb( );
    
    void            reset( );

    bool            lookupItlb( T64Word vadr, T64Word *pAdr, uint16_t *tlbInfo );
    bool            lookupDtlb( T64Word vadr, T64Word *pAdr, uint16_t *tlbInfo );

    bool            purgeTlb( T64Word vAdr );

    T64TlbEntry     *getITlbEntry( int index );
    T64TlbEntry     *getDTlbEntry( int index );

    T64Word         getTlbStatus( );
    T64Word         getTlbConfig( );
    
    T64Word         getItlbHits( );
    T64Word         getItlbMisses( );
    T64Word         getItlbMissGTlbHits( );
    T64Word         getItlbMissGTlbMisses( );

    T64Word         getDtlbHits( );
    T64Word         getDtlbMisses( );
    T64Word         getDtlbMissGTlbHits( );
    T64Word         getDtlbMissGTlbMisses( ); 

    private:

    T64Processor    *proc               = nullptr;

    T64TlbKind      tlbKind             = T64_TK_NIL;
    T64TlbType      tlbType             = T64_TT_NIL;

    T64TlbEntry     *iTlb               = nullptr;
    T64TlbEntry     *dTlb               = nullptr;

    int             iTlbEntries         = 0;
    int             dTlbEntries         = 0;
   
    uint32_t        iTlbRoundRobin      = 0;       

    T64Word         tlbStatus           = 0;
    T64Word         tlbConfig           = 0;
    
    T64Word         iTlbHits            = 0;
    T64Word         iTlbMisses          = 0;
    T64Word         iTlbMissGTlbHits    = 0;
    T64Word         iTlbMissGTlbMisses  = 0;

    T64Word         dTlbHits            = 0;
    T64Word         dTlbMisses          = 0;
    T64Word         dTlbMissGTlbHits    = 0;
    T64Word         dTlbMissGTlbMisses  = 0; 
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
    bool            executeInstr( );

    T64Word         getGeneralReg( int index );
    void            setGeneralReg( int index, T64Word val );

    T64Word         getControlReg( int index );
    void            setControlReg( int index, T64Word val );

    T64Word         getPsrReg( );
    void            setPsrReg( T64Word val );

    private: 

    int             evalCond( int cond, T64Word val1, T64Word val2 );

    void            machineCheckTrap( T64Word adr );
    void            privModeOperationTrap( );

    void            instrTlbMissTrap( T64Word adr );
    void            instrMemAccRightsTrap( T64Word adr );
    void            instrMemProtectionTrap( T64Word adr );
    void            instrMemAlignmentTrap( T64Word adr );

    void            dataMemTlbMissTrap( T64Word adr );
    void            dataMemNonAccessTlbMissTrap( T64Word adr );
    void            dataMemAccRightsTrap( T64Word adr );
    void            dataMemProtectionTrap( T64Word adr );
    void            dataMemAlignmentTrap( T64Word adr );

    void            overFlowTrap( );
    void            illegalInstrTrap( );

    void            privModeCheck( );
    bool            regionIdCheck( uint32_t pId, bool wMode );
    void            instrAlignmentCheck( T64Word vAdr );
    void            instrAccCheck( T64Word vAdr, uint16_t tlbInfo );
    void            dataAlignmentCheck( T64Word vAdr, int len );
    void            dataReadAccCheck( T64Word vAdr, uint16_t tlbInfo );
    void            dataWriteAccCheck( T64Word vAdr, uint16_t tlbInfo );
    void            addOverFlowCheck( T64Word val1, T64Word val2 );
    void            subUnderFlowCheck( T64Word val1, T64Word val2 );

    void            nextInstr( );

    T64Word         getRegR( uint32_t instr );
    T64Word         getRegB( uint32_t instr );
    T64Word         getRegA( uint32_t instr );
    void            setRegR( uint32_t instr, T64Word val );
   
    T64Word         instrRead( T64Word vAdr );
    T64Word         dataRead( T64Word vAdr, int len, bool sExt, bool rsv = false );
    T64Word         dataReadRegBOfsImm13( uint32_t instr, bool sExt, bool rsv = false );
    T64Word         dataReadRegBOfsRegX( uint32_t instr, bool sExt );

    bool            dataWrite( T64Word vAdr, T64Word val, int len, bool cond = false );
    bool            dataWriteRegBOfsImm13( uint32_t instr, bool cond = false );
    bool            dataWriteRegBOfsRegX( uint32_t instr );

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

   
    void            handleExtInterrupts( );  // ??? will go out ?

    T64Word         diagOpHandler( int opt, T64Word arg1, T64Word arg2 );

    T64Word         cRegFile[ T64_MAX_CREGS ];
    T64Word         gRegFile[ T64_MAX_GREGS ];
    T64Word         psrReg;
    uint32_t        instrReg;
    uint16_t        instrTlbInfo;

    T64CpuType      cpuType         = T64_CPU_T_NIL;
    T64Word         physMemSize     = T64_MAX_PHYS_MEM_LIMIT;
    T64Processor    *proc           = nullptr;
};

//----------------------------------------------------------------------------------------
// The CPU core executes the instructions. A processor module contains the CPU 
// core, TLBs and one day caches. The processor module connects to the system bus
// for  memory and IO access. The unit is a single step, i.e. one instruction. 
//
//----------------------------------------------------------------------------------------
struct T64Processor : T64ThreadModule {
    
    public:

    T64Processor( T64System         *sys,
                  int               modNum,
                  T64Options        options,  
                  T64CpuType        cpuType,
                  T64TlbType        tlbType,
                  T64CacheType      cacheType );
    
    virtual         ~ T64Processor( );

    void            initModule( );
    void            resetModule( );
    void            haltModule( );
    void            runModule( );
    void            execModule( int steps );
    bool            executeUnit( );

    bool            busOpRead( T64Word adr, uint8_t *data, int len );
    bool            busOpReadRsv( T64Word adr, uint8_t *data, int len );
    bool            busOpWrite( T64Word adr, uint8_t *data, int len );
    bool            busOpWriteCond( T64Word adr, uint8_t *data, int len );

    bool            busOpBroadCast( T64BroadcastEvents id, 
                                    T64Word            arg1, 
                                    T64Word            arg2 );

    bool            busOpReadEvent( int     reqModNum,
                                    T64Word pAdr, 
                                    uint8_t *data, 
                                    int     len );

    bool            busOpWriteEvent( int     reqModNum,
                                     T64Word pAdr, 
                                     uint8_t *data, 
                                     int     len );  

    bool            busOpBroadcastEvent( int                srcModNum,
                                         T64BroadcastEvents id, 
                                         T64Word            arg1, 
                                         T64Word            arg2 );
                        
    T64Cpu          *getCpuPtr( );
    T64LocalTlb     *getLocalTlbPtr( );
    char            *getProcStateStr( );
    T64GlobalTlb    *getGlobalTlbPtr( );
    bool            isHalted( );

private:

    bool            handleHPARead( T64Word pAdr, uint8_t *data, int len );
    bool            handleHPAWrite( T64Word pAdr, uint8_t *data, int len );

    bool            handleHPABroadcast( T64BroadcastEvents  event, 
                                        T64Word             arg1, 
                                        T64Word             arg2);

    friend struct   T64Cpu;

    T64System       *sys                    = nullptr;
    T64Cpu          *cpu                    = nullptr;
    T64LocalTlb     *localTlb               = nullptr;
    T64GlobalTlb    *globalTlb              = nullptr;
};
