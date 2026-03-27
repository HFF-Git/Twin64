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
const   T64Word T64_IO_SPA_MEM_LIMIT        = 0xFEFFFFFF;

const   T64Word T64_IO_HPA_MEM_START        = 0xFF000000;
const   T64Word T64_IO_HPA_MEM_LIMIT        = 0xFFFFFFFF;

const   T64Word T64_IO_BCAST_MEM_START      = 0xFFFFF000;
const   T64Word T64_IO_BCAST_MEM_LIMIT      = 0xFFFFFFFF;

const   T64Word T64_DEF_PHYS_MEM_SIZE       = 1LL << 32;
const   T64Word T64_DEF_PHYS_MEM_LIMIT      = 0xEFFFFFFF;
const   T64Word T64_MAX_PHYS_MEM_LIMIT      = 0xFFFFFFFFF;

const   T64Word T64_MAX_REGION_ID           = 0xFFFFF;
const   T64Word T64_MAX_VIRT_MEM_LIMIT      = 0xFFFFFFFFFFFFF;

const   int     T64_PAGE_SIZE_BYTES         = 4096;
const   int     T64_PAGE_OFS_BITS           = 12;
const   int     T64_VADR_BITS               = 52;
const   int     T64_PADR_BITS               = 36;

//----------------------------------------------------------------------------------------
// T64 page types.
//
// ??? also doubles as acc mode ?
//----------------------------------------------------------------------------------------
enum T64PageType : uint8_t {

    PT_NONE        = 0,
    PT_READ_ONLY   = 1,
    PT_READ_WRITE  = 2,
    PT_EXECUTE     = 3
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
    INSTR_PROTECTION_TRAP           = 11,
    INSTR_ALIGNMENT_TRAP            = 12,

    DATA_TLB_MISS_TRAP              = 13,
    NON_ACC_DATA_TLB_MISS_TRAP      = 14,
    DATA_ACC_RIGHTS_TRAP            = 15,
    DATA_PROTECTION_TRAP            = 16,
    DATA_ALIGNMENT_TRAP             = 17,
      
    PAGE_REF_TRAP                   = 18,
    BREAK_INSTR_TRAP                = 19,
    
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
