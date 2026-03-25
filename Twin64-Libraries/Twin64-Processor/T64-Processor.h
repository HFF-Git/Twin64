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
// CPU. The execution unit. We could support several types. So far, we do not.
//
//----------------------------------------------------------------------------------------
enum T64CpuType : int {

    T64_CPU_T_NIL = 0
};

//----------------------------------------------------------------------------------------
// TLBs. A translation lookaside buffer is essential. We support a TLB kind, and
// fully associative TLBs. TLB kind specifies the kind of cache, i.e. instruction, 
// data or unified TLB. TLB type encoded as follows:
//
//  T64_TT_<type>_<sets>S
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
    T64_TT_FA_64S       = 1,
    T64_TT_FA_128S      = 2
};

//----------------------------------------------------------------------------------------
// Caches. Caches are sub modules to the processor. We support a cache type, set 
// associative caches, 2, 4, and 8-way. There is a cache line info with flags and 
// the tag and an array of bytes which holds the cache data. Cache kind specifies 
// the kind of cache, i.e. instruction, data or unified cache. Cache type encoded
// as follows:
//
//  T64_CT_<ways>W_<sets>S_<words>L
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
// Cache line info consisting of valid, modified and the cache tag.
//
//----------------------------------------------------------------------------------------
struct T64CacheLineInfo {

    bool        valid;
    bool        modified;
    uint32_t    tag;
};

//----------------------------------------------------------------------------------------
// The cache submodule. The CPU can have one or two caches. All access will go 
// through the cache submodule, even when the request is a non-cached request. 
// The CPU uses the read, write, flush and purge methods for access. The getting 
// a cache line method is used by the simulator for displaying cache data. In
// addition, the cache maintains a set of statistics.
//
//----------------------------------------------------------------------------------------
struct T64Cache {

    public:

    T64Cache( T64Processor  *proc, 
              T64CacheKind  cacheType, 
              T64CacheType  cacheStructure );

    virtual         ~ T64Cache( );

    void                reset( );
    void                step( );

    void                read( T64Word pAdr, uint8_t *data, int len, bool cached = true);
    void                write( T64Word pAdr, uint8_t *data, int len, bool cached = true );
    void                flush( T64Word pAdr );
    void                purge( T64Word pAdr );

    bool                getCacheLineByIndex( uint32_t          way,
                                             uint32_t          set, 
                                             T64CacheLineInfo  **info,
                                             uint8_t           **data );

    bool                purgeCacheLineByIndex( uint32_t way, uint32_t set );
    bool                flushCacheLineByIndex( uint32_t way, uint32_t set );

    int                 getRequestCount( );
    int                 getHitCount( );
    int                 getMissCount( );
    int                 getWays( );
    int                 getSetSize( );
    int                 getCacheLineSize( );
    T64CacheKind        getCacheKind( );
    T64CacheType        getCacheType( );
    char                *getCacheTypeString( );

    private: 

    bool                lookupCache( T64Word          pAdr, 
                                     T64CacheLineInfo **info, 
                                     uint8_t          **data );

    void                readCacheData( T64Word pAdr, uint8_t *data, int len );
    void                writeCacheData( T64Word pAdr, uint8_t *data, int len );
    void                flushCacheLine( T64Word pAdr );
    void                purgeCacheLine( T64Word pAdr );

    bool                getCacheLineData( uint8_t *line, 
                                          int     lineOfs,
                                          int     len,
                                          uint8_t *data );

    bool                setCacheLineData( uint8_t *line,
                                          int     lineOfs,
                                          int     len,
                                          uint8_t *data ); 
                                          
    uint32_t            getTag( T64Word pAdr );                           
    uint32_t            getSetIndex( T64Word  paAdr );
    uint32_t            getLineOfs( T64Word  paAdr );
    T64Word             pAdrFromTag( uint32_t tag, uint32_t index );
    int                 plruVictim( );
    void                plruUpdate( );

    private: 

    T64CacheKind        cacheKind       = T64_CK_NIL;
    T64CacheType        cacheType       = T64_CT_NIL;

    T64CacheLineInfo    *cacheInfo      = nullptr;
    uint8_t             *cacheData      = nullptr;
    T64Processor        *proc           = nullptr;
    T64System           *sys            = nullptr;

    int                 ways            = 0;
    int                 sets            = 0;
    int                 lineSize        = 0;
    int                 offsetBits      = 0;
    int                 indexBits       = 0;
    T64Word             offsetBitmask   = 0;
    T64Word             indexBitmask    = 0;
    int                 indexShift      = 0;
    int                 tagShift        = 0;
    int                 cacheHits       = 0;
    int                 cacheMiss       = 0;
    uint8_t             plruState       = 0;
};

//----------------------------------------------------------------------------------------
// TLB Entry. The TLB entry stores one translation along with several flags. Each 
// entry has a last used count for the LRU replacement scheme.
//
//----------------------------------------------------------------------------------------
struct T64TlbEntry {

    public:

    bool            valid           = false;
    bool            uncached        = false;
    bool            locked          = false;
    bool            modified        = false;
    bool            trapOnBranch    = false;
    T64Word         vAdr            = 0;
    T64Word         pAdr            = 0;
    int             pSize           = 0;
    uint8_t         pageType        = 0;
    bool            pLev1           = false;
    bool            pLev2           = false;
    T64Word         lastUsed        = 0;
};

//----------------------------------------------------------------------------------------
// The TLB submodule. A CPU can have one or two TLBs. Our TLBs are simple arrays 
// of entries, i.e. modeling a full associative array with a LRU replacement 
// policy. The CPU uses the lookup, insert and purge methods. The simulator uses 
// the methods for display and directly inserting or removing an entry.
//
//----------------------------------------------------------------------------------------
struct T64Tlb {
    
    public:

    T64Tlb( T64Processor *proc, T64TlbKind tlbKind, T64TlbType tlbType );

    virtual         ~ T64Tlb( );
    
    void            reset( );
    T64TlbEntry     *lookup( T64Word vAdr );
    
    bool            insert( T64Word vAdr, T64Word info );
    bool            purge( T64Word vAdr );

    bool            insertTlbByIndex( uint32_t index, T64Word vAdr, T64Word info );
    bool            purgeTlbByIndex( uint32_t index );
    
    int             getTlbSize( );
    T64TlbEntry     *getTlbEntry( int index );

    T64TlbKind      getTlbKind( );  
    T64TlbType      getTlbType( );
    char           *getTlbTypeString( );

    private:
    
    T64TlbKind      tlbKind         = T64_TK_NIL;
    T64TlbType      tlbType         = T64_TT_NIL;
    T64TlbEntry     *map            = nullptr; 
    int             tlbEntries      = 0;
    T64Word         timeCounter     = 0;
    T64Processor    *proc           = nullptr;
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
    void            step( );

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
                  T64TlbType        iTlbType,
                  T64TlbType        dTlbType,
                  T64CacheType      iCacheType,
                  T64CacheType      dCacheType,
                  T64Word           spaAdr,
                  int               spaLen );
    
    virtual        ~ T64Processor( );
    
    void            reset( );
    void            step( );

    bool            busOpReadSharedBlock( int reqModNum, 
                                          T64Word pAdr, 
                                          uint8_t *data, 
                                          int len );

    bool            busOpReadPrivateBlock( int reqModNum, 
                                           T64Word pAdr, 
                                           uint8_t *data, 
                                           int len );

    bool            busOpWriteBlock( int reqModNum, 
                                     T64Word pAdr, 
                                     uint8_t *data, 
                                     int len );

    bool            busOpReadUncached( int reqModNum, 
                                       T64Word adr, 
                                       uint8_t *data, 
                                       int len );

    bool            busOpWriteUncached( int reqModNum, 
                                        T64Word adr, 
                                        uint8_t *data, 
                                        int len );

    T64Cpu          *getCpuPtr( );
    T64Tlb          *getITlbPtr( );
    T64Tlb          *getDTlbPtr( );
    T64Cache        *getICachePtr( );
    T64Cache        *getDCachePtr( );
    
private:

    friend struct   T64Cpu;

    T64System       *sys                = nullptr;
    T64Cpu          *cpu                = nullptr;
    T64Tlb          *iTlb               = nullptr;
    T64Tlb          *dTlb               = nullptr;
    T64Cache        *iCache             = nullptr;
    T64Cache        *dCache             = nullptr;

    int             modNum              = 0;
    T64Word         instructionCount    = 0;
    T64Word         cycleCount          = 0;
};
