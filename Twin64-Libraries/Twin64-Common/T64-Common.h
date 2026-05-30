//----------------------------------------------------------------------------------------
//
//  Twin64Sim - A 64-bit CPU Simulator - Common Declarations
//
//----------------------------------------------------------------------------------------
// ...
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Common Declarations
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

//----------------------------------------------------------------------------------------
// Mac and Windows know different include files and procedure names for some POSIX 
// routines. Learned the hard way that these files better come really early in the
// project. All libraries and modules depend on these basic type definitions and 
// include the "T64-Common.h" file early on.
//
//----------------------------------------------------------------------------------------
#if __APPLE__
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#else
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <iostream>
#include <io.h>
#endif

//----------------------------------------------------------------------------------------
// Fundamental data types.
//
//----------------------------------------------------------------------------------------
typedef int64_t     T64Word;
typedef uint32_t    T64Instr;

//----------------------------------------------------------------------------------------
// Fundamental constant values.
//
//----------------------------------------------------------------------------------------
const   int     T64_MAX_GREGS               = 16;
const   int     T64_MAX_CREGS               = 16;

const   T64Word T64_IO_MEM_START            = 0xF0000000;
const   T64Word T64_IO_MEM_LIMIT            = 0xFFFFFFFF;

const   T64Word T64_PDC_MEM_START           = 0xF0000000;
const   T64Word T64_PDC_MEM_LIMIT           = 0xF0FFFFFF;  

const   T64Word T64_IO_SPA_MEM_START        = 0xF1000000;
const   T64Word T64_IO_SPA_MEM_LIMIT        = 0xFFFBFFFF;

const   T64Word T64_IO_HPA_MEM_START        = 0xFFFC0000;
const   T64Word T64_IO_HPA_MEM_LIMIT        = 0xFFFFFFFF;

const   T64Word T64_IO_BCAST_MEM_START      = 0xFFFFF000;
const   T64Word T64_IO_BCAST_MEM_LIMIT      = 0xFFFFFFFF;

const   T64Word T64_DEF_PHYS_MEM_SIZE       = 1LL << 32;
const   T64Word T64_DEF_PHYS_MEM_LIMIT      = 0xEFFFFFFF;
const   T64Word T64_MAX_PHYS_MEM_LIMIT      = 0xFFFFFFFFF;

const   T64Word T64_MAX_REGION_ID           = 0xFFFFF;
const   T64Word T64_VIRT_MEM_START          = T64_MAX_PHYS_MEM_LIMIT + 1;
const   T64Word T64_MAX_VIRT_MEM_LIMIT      = 0xFFFFFFFFFFFFF;

const   int     T64_PAGE_SIZE_BYTES         = 4096;
const   int     T64_PAGE_OFS_BITS           = 12;
const   int     T64_VADR_BITS               = 52;
const   int     T64_PADR_BITS               = 36;

const   int     T64_REG_SET_SIZE            = 32;

//----------------------------------------------------------------------------------------
// T64 page types.
//
//----------------------------------------------------------------------------------------
enum T64PageType : uint8_t {

    PT_NONE        = 0,
    PT_READ_ONLY   = 1,
    PT_READ_WRITE  = 2,
    PT_EXECUTE     = 3,
    PT_GATEWAY     = 4
};

//----------------------------------------------------------------------------------------
// T64 access modes..
//
//----------------------------------------------------------------------------------------
enum T64AccMode : uint8_t {

    ACC_MODE_READ = 1,
    ACC_MODE_WRITE = 2,
    ACC_MODE_EXECUTE = 3
};

//----------------------------------------------------------------------------------------
// T64 Traps. Traps are identified by their number. A trap handler is passed 
// further information via the control registers.
//
//----------------------------------------------------------------------------------------
enum T64TrapCode : int {
    
    NO_TRAP                         = 0,
    MACHINE_CHECK                   = 1,

    POWER_FAILURE                   = 2, 
    RECOVERY_COUNTER_TRAP           = 3,
    EXTERNAL_INTERRUPT              = 4,

    ILLEGAL_INSTR_TRAP              = 5,
    PRIV_OPERATION_TRAP             = 6,
    PRIV_REGISTER_TRAP              = 7,
    OVERFLOW_TRAP                   = 8,

    INSTR_TLB_MISS_TRAP             = 9,
    NON_ACC_INSTR_TLB_MISS_TRAP     = 10,
    INSTR_ACC_RIGHTS_TRAP           = 11,
    INSTR_PROTECTION_TRAP           = 12,
    INSTR_ALIGNMENT_TRAP            = 13,

    DATA_TLB_MISS_TRAP              = 14,
    NON_ACC_DATA_TLB_MISS_TRAP      = 15,
    DATA_ACC_RIGHTS_TRAP            = 16,
    DATA_PROTECTION_TRAP            = 17,
    DATA_ALIGNMENT_TRAP             = 18,
      
    PAGE_REF_TRAP                   = 19,
    BREAK_INSTR_TRAP                = 20,

    USER_DEFINED_TRAP               = 21,
    
};

//----------------------------------------------------------------------------------------
// Trap definition. A Trap will consist of a trap code, the trapping instruction 
// address and up to two additional arguments. On an error condition,tThe processor
// subsystems throw a trap. 
//
//----------------------------------------------------------------------------------------
struct T64Trap {
    
public:
    
    T64Trap( T64TrapCode    trapCode,
             T64Word        instrAdr = 0,
             T64Word        arg0     = 0,
             T64Word        arg1     = 0 ) {
        
        this -> trapCode = trapCode;
        this -> instrAdr = instrAdr;
        this -> arg0     = arg0;
        this -> arg1     = arg1;
    }
    
    T64TrapCode trapCode;
    T64Word     instrAdr;
    uint32_t    arg0;
    T64Word     arg1;
};

//----------------------------------------------------------------------------------------
// Control registers.
//
//----------------------------------------------------------------------------------------
enum ControlRegId : int {
    
    CTL_REG_CPU_INFO    = 0,
    CTL_REG_SHAMT       = 1,
    CTL_REG_REC_CNTR    = 2,
    CTL_REG_RESERVED_3  = 3,

    CTL_REG_PID_0       = 4,
    CTL_REG_PID_1       = 5,
    CTL_REG_PID_2       = 6,
    CTL_REG_PID_3       = 7,

    CTL_REG_IVA         = 8,
    CTL_REG_EIEM        = 9,
    CTL_REG_IPSR        = 10,
    CTL_REG_IARG_0      = 11,
    CTL_REG_IARG_1      = 12,

    CTL_REG_SCRATCH_0    = 13,
    CTL_REG_SCRATCH_1    = 14,
    CTL_REG_SCRATCH_2    = 15
};

//----------------------------------------------------------------------------------------
// Instruction groups and opcode families. Instructions are decoded in three 
// fields. The first two bits contain the instruction group. Next are 4 bits for
// opcode family. Bits 19..21 are further qualifying the instruction.
//
//----------------------------------------------------------------------------------------
enum OpCodeGroup : uint32_t {
    
    OPC_GRP_ALU     = 0U,
    OPC_GRP_MEM     = 1U,
    OPC_GRP_BR      = 2U,
    OPC_GRP_SYS     = 3U
};

enum OpCodeFam : uint32_t {
    
    OPC_NOP         = 0U,
    
    OPC_ADD         = 1U,
    OPC_SUB         = 2U,
    OPC_AND         = 3U,
    OPC_OR          = 4U,
    OPC_XOR         = 5U,
    OPC_CMP_A       = 6U,
    OPC_CMP_B       = 7U,

    OPC_BITOP       = 8U,
    OPC_SHAOP       = 9U,
    OPC_IMMOP       = 10U,
    OPC_LDO         = 11U,
    
    OPC_LD          = 8U,
    OPC_ST          = 9U,
    OPC_LDR         = 10U,
    OPC_STC         = 11U,
    
    OPC_B           = 1U,
    OPC_BE          = 2U,
    OPC_BR          = 3U,
    OPC_BV          = 4U,
   
    OPC_BB          = 8U,
    OPC_CBR         = 9U,
    OPC_MBR         = 10U,
    OPC_ABR         = 11U,
    
    OPC_MR          = 1U,
    OPC_LPA         = 2U,
    OPC_PRB         = 3U,
    OPC_TLB         = 4U,
    OPC_CA          = 5U,
    OPC_MST         = 6U,
    OPC_RFI         = 7U,
    OPC_TRAP        = 14U,
    OPC_DIAG        = 15U
};

//----------------------------------------------------------------------------------------
// IO Module register set definitions. A hard physical page consists of several
// register sets, where only the first one is partially architected. The following
// constants represent the index into the register sets of the HPA page. Note 
// that the constants are just offsets into the HPA page and used to select a
// register. The implementation of the actual hardware, etc. is entirely up to
// the module. 
//
//----------------------------------------------------------------------------------------
enum T64IoRegSetIndex : int {

    T64_IO_REG_SET_0_OFS    =  0 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_1_OFS    =  1 * T64_REG_SET_SIZE, 
    T64_IO_REG_SET_2_OFS    =  2 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_3_OFS    =  3 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_4_OFS    =  4 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_5_OFS    =  5 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_6_OFS    =  6 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_7_OFS    =  7 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_8_OFS    =  8 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_9_OFS    =  9 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_10_OFS   = 10 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_11_OFS   = 11 * T64_REG_SET_SIZE, 
    T64_IO_REG_SET_12_OFS   = 12 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_13_OFS   = 13 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_14_OFS   = 14 * T64_REG_SET_SIZE,
    T64_IO_REG_SET_15_OFS   = 15 * T64_REG_SET_SIZE
};

//----------------------------------------------------------------------------------------
// Within a regs set, registers defined. Register set zero has architected 
// registers.
//
//----------------------------------------------------------------------------------------
enum T64IoRegIndex : int {

    T64_IO_STATUS_REG_OFS   = 0,
    T64_IO_COMMAND_REG_OFS  = 1,
    T64_IO_CONFIG_REG_OFS   = 2,
    T64_IO_SPA_ADR_REG_OFS  = 3

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
    T64_TK_UNIFIED_TLB  = 3,
    T64_TK_GLOBAL_TLB   = 4,
};

enum T64TlbType : int {

    T64_TT_NIL          = 0,
    T64_TT_FA_16S       = 1,
    T64_TT_FA_32S       = 2,
    T64_TT_FA_64S       = 3,
    T64_TT_FA_128S      = 4
};

//----------------------------------------------------------------------------------------
// A TLB entry in the a TLB table. There are the virtual starting address and
// the corresponding physical page address. The range and alignment is encoded
// in the info field "pSize". There is also the precomputed page mask used for 
// masking off bits depending on the page size. The info field contains besides
// the page size encoding, the access rights and several flags.
//
//
//  TLB Info Parameter:
//
//        15  14  13  12  11  10  9   8   7   6   5..4    3..0
//      :-------------------------------------------------------:
//      : V : L : U : M : 0 : 0 : 0 : 0 : P1: P2: ACC   : PSize :
//      :-------------------------------------------------------:
// 
//----------------------------------------------------------------------------------------
struct T64TlbEntry {

    T64Word     vAdr;
    T64Word     pAdr;
    T64Word     pageMask;
    uint16_t    tlbInfo;
};

enum T64TbInfoFieldMask : uint16_t {

    T64_TM_VALID    = 0x8000,
    T64_TM_LOCKED   = 0x4000,
    T64_TM_UNCACHED = 0x2000,
    T64_TM_MODIFIED = 0x1000,

    T64_TM_PRIV1    = 0x0080,
    T64_TM_PRIV2    = 0x0040,
    T64_TM_ACC      = 0x0030,
    T64_TM_PSIZE    = 0x000F
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
