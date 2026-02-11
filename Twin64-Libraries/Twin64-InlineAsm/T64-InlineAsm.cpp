//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - One Line Assembler
//
//----------------------------------------------------------------------------------------
// The one line assembler assembles an instruction without further context. It is 
// intended to for testing instructions in the monitor. There is no symbol table 
// or any concept of assembling multiple instructions. The instruction to generate 
// test is completely self sufficient. The parser is a straightforward recursive 
// descendant parser, LL1 grammar. It uses the C++ try / catch to escape when an 
// error is detected. Considering that we only have one line to  parse, there is 
// no need to implement a better parser error recovery method.
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - One Line Assembler
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
// program.  If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-InlineAsm.h"

//----------------------------------------------------------------------------------------
// Local namespace. These routines are not visible outside this source file.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// General constants.
//
//----------------------------------------------------------------------------------------
const int   MAX_INPUT_LINE_SIZE = 256;
const int   MAX_TOKEN_NAME_SIZE = 32;
const char  EOS_CHAR            = 0;

//----------------------------------------------------------------------------------------
// Assembler error codes.
//
//----------------------------------------------------------------------------------------
enum ErrId : int {
    
    NO_ERR                          = 0,
    
    ERR_EXTRA_TOKEN_IN_STR          = 10,
    ERR_INVALID_CHAR_IN_IDENT       = 11,
    ERR_INVALID_EXPR                = 12,
    ERR_INVALID_NUM                 = 13,
    ERR_INVALID_OP_CODE             = 14,
    ERR_INVALID_INSTR_MODE          = 15,
    ERR_INVALID_OFS                 = 16,
    ERR_INVALID_INSTR_OPT           = 17,
    
    ERR_EXPECTED_CLOSING_QUOTE      = 20,
    ERR_EXPECTED_NUMERIC            = 21,
    ERR_EXPECTED_COMMA              = 22,
    ERR_EXPECTED_LPAREN             = 23,
    ERR_EXPECTED_RPAREN             = 24,
    ERR_EXPECTED_STR                = 25,
    ERR_EXPECTED_OPCODE             = 26,
    ERR_EXPECTED_INSTR_OPT          = 27,
    ERR_EXPECTED_DIAG_OP            = 28,
    ERR_EXPECTED_GENERAL_REG        = 29,
    ERR_EXPECTED_POS_ARG            = 30,
    ERR_EXPECTED_LEN_ARG            = 31,
    ERR_BIT_RANGE_EXCEEDS           = 32,
    ERR_EXPECTED_BR_OFS             = 33,
    ERR_EXPECTED_CONTROL_REG        = 34,
    ERR_EXPECTED_PRB_ARG            = 35,
    ERR_UNEXPECTED_EOS              = 36,
   
    ERR_EXPR_TYPE_MATCH             = 40,
    ERR_NUMERIC_OVERFLOW            = 41,
    ERR_IMM_VAL_RANGE               = 42,
    ERR_DUPLICATE_INSTR_OPT         = 43
};

//----------------------------------------------------------------------------------------
// Error message strings.
//
//----------------------------------------------------------------------------------------
struct ErrMsg {
    
    int msgId;
    char *msg;
};

const ErrMsg ErrMsgTable[ ] = {
    
    { NO_ERR,                       (char *) "No error" },
    
    { ERR_EXTRA_TOKEN_IN_STR,       (char *) "Extra tokens in input line" },
    { ERR_INVALID_CHAR_IN_IDENT,    (char *) "Invalid char in input line" },
    { ERR_INVALID_EXPR,             (char *) "Invalid expression" },
    { ERR_NUMERIC_OVERFLOW,         (char *) "Numeric overflow" },
    { ERR_INVALID_NUM,              (char *) "Invalid number" },
    { ERR_INVALID_OP_CODE,          (char *) "Invalid OpCode" },
    { ERR_INVALID_INSTR_MODE,       (char *) "Invalid instruction mode" },
    { ERR_INVALID_OFS,              (char *) "Invalid offset" },
    { ERR_INVALID_INSTR_OPT,        (char *) "Invalid instruction option" },
    
    { ERR_EXPECTED_CLOSING_QUOTE,   (char *) "Expected a closing quote" },
    { ERR_EXPECTED_NUMERIC,         (char *) "Expected a numeric value" },
    { ERR_EXPECTED_COMMA,           (char *) "Expected a comma" },
    { ERR_EXPECTED_LPAREN,          (char *) "Expected a left parenthesis" },
    { ERR_EXPECTED_RPAREN,          (char *) "Expected a right parenthesis" },
    { ERR_EXPECTED_STR,             (char *) "Expected a string" },
    { ERR_EXPECTED_OPCODE,          (char *) "Expected an opCode" },
    { ERR_EXPECTED_INSTR_OPT,       (char *) "Expected an instruction option" },
    { ERR_EXPECTED_DIAG_OP,         (char *) "Expected the DIAG opCode" },
    { ERR_EXPECTED_GENERAL_REG,     (char *) "Expected a general register" },
    { ERR_EXPECTED_POS_ARG,         (char *) "Expected a position argument" },
    { ERR_EXPECTED_LEN_ARG,         (char *) "Expected a length argument" },
    { ERR_BIT_RANGE_EXCEEDS,        (char *) "Bit range exceeds word size" }, 
    { ERR_EXPECTED_BR_OFS,          (char *) "Expected a branch offset" },
    { ERR_EXPECTED_CONTROL_REG,     (char *) "Expected a control register" },
    { ERR_EXPECTED_PRB_ARG,         (char *) "Expected the PRB argument" },
    { ERR_UNEXPECTED_EOS,           (char *) "Unexpected end of string" },   
   
    { ERR_EXPR_TYPE_MATCH ,         (char *) "Expression type mismatch" },
    { ERR_IMM_VAL_RANGE,            (char *) "Value range error " },
    { ERR_DUPLICATE_INSTR_OPT,      (char *) "Duplicate Instruction option " }
};

const int MAX_ERR_MSG_TAB = sizeof( ErrMsgTable ) / sizeof( ErrMsg );

//----------------------------------------------------------------------------------------
// Command line tokens and expression have a type.
//
//----------------------------------------------------------------------------------------
enum TokTypeId : int {
    
    TYP_NIL                 = 0,
    TYP_SYM                 = 1,        
    TYP_IDENT               = 2,        
    TYP_PREDEFINED_FUNC     = 3,
    TYP_NUM                 = 4,        
    TYP_STR                 = 5,       
    TYP_OP_CODE             = 6,
    TYP_GREG                = 7,        
    TYP_CREG                = 8,
};

//----------------------------------------------------------------------------------------
// Tokens are the labels for reserved words and symbols recognized by the 
// tokenizer objects. Tokens have a name, a token id, a token type and an 
// optional value with further data.
//
//----------------------------------------------------------------------------------------
enum TokId : int {
    
    //------------------------------------------------------------------------------------
    // General tokens and symbols.
    //------------------------------------------------------------------------------------
    TOK_NIL         = 0,    TOK_ERR         = 1,    TOK_EOS         = 2,        
    TOK_COMMA       = 3,    TOK_PERIOD      = 4,    TOK_LPAREN      = 5,
    TOK_RPAREN      = 6,    TOK_PLUS        = 8,    TOK_MINUS       = 9,        
    TOK_MULT        = 10,   TOK_DIV         = 11,   TOK_MOD         = 12,       
    TOK_REM         = 13,   TOK_NEG         = 14,   TOK_AND         = 15,       
    TOK_OR          = 16,   TOK_XOR         = 17,   TOK_IDENT       = 24,       
    TOK_NUM         = 25,   TOK_STR         = 26,
    
    //------------------------------------------------------------------------------------
    // General, Segment and Control Registers Tokens.
    //------------------------------------------------------------------------------------
    REG_SET         = 100,
    
    TOK_GR_0        = 101,  TOK_GR_1        = 102,  TOK_GR_2        = 103,
    TOK_GR_3        = 104,  TOK_GR_4        = 105,  TOK_GR_5        = 106,
    TOK_GR_6        = 107,  TOK_GR_7        = 108,  TOK_GR_8        = 109,
    TOK_GR_9        = 110,  TOK_GR_10       = 111,  TOK_GR_11       = 112,
    TOK_GR_12       = 113,  TOK_GR_13       = 114,  TOK_GR_14       = 115,
    TOK_GR_15       = 116,
    
    TOK_CR_0        = 121,  TOK_CR_1        = 122,  TOK_CR_2        = 123,
    TOK_CR_3        = 124,  TOK_CR_4        = 125,  TOK_CR_5        = 126,
    TOK_CR_6        = 127,  TOK_CR_7        = 128,  TOK_CR_8        = 129,
    TOK_CR_9        = 130,  TOK_CR_10       = 131,  TOK_CR_11       = 132,
    TOK_CR_12       = 133,  TOK_CR_13       = 134,  TOK_CR_14       = 136,
    TOK_CR_15       = 137,
    
    //------------------------------------------------------------------------------------
    // OP Code Tokens.
    //------------------------------------------------------------------------------------
    TOK_OP_NOP      = 300,
    
    TOK_OP_AND      = 301,  TOK_OP_OR       = 302,  TOK_OP_XOR      = 303,
    TOK_OP_ADD      = 304,  TOK_OP_SUB      = 305,  TOK_OP_CMP      = 306,
    
    TOK_OP_EXTR     = 311,  TOK_OP_DEP      = 312,  TOK_OP_DSR      = 313,
    TOK_OP_SHL1A    = 314,  TOK_OP_SHL2A    = 315,  TOK_OP_SHL3A    = 316,
    TOK_OP_SHR1A    = 317,  TOK_OP_SHR2A    = 318,  TOK_OP_SHR3A    = 319,
    
    TOK_OP_LDIL     = 331,  TOK_OP_ADDIL    = 332,  TOK_OP_LDO      = 333,
    TOK_OP_LD       = 334,  TOK_OP_LDR      = 335,
    TOK_OP_ST       = 337,  TOK_OP_STC      = 338,
    
    TOK_OP_B        = 341,  TOK_OP_BR       = 342,  TOK_OP_BV       = 343,
    TOK_OP_BE       = 344,

    TOK_OP_BB       = 345,  TOK_OP_CBR      = 346,  TOK_OP_MBR      = 347,
    TOK_OP_ABR      = 348,
    
    TOK_OP_MFCR     = 351,  TOK_OP_MTCR     = 352,  TOK_OP_MFIA     = 353,
    TOK_OP_RSM      = 354,  TOK_OP_SSM      = 355,
    TOK_OP_LPA      = 356,  TOK_OP_PRB      = 357,

    TOK_OP_IITLB    = 361,  TOK_OP_IDTLB    = 362,
    TOK_OP_PITLB    = 363,  TOK_OP_PDTLB    = 364,
    
    TOK_OP_PICA     = 365,  TOK_OP_PDCA     = 366,  
    TOK_OP_FICA     = 367,  TOK_OP_FDCA     = 368,
    
    TOK_OP_RFI      = 371,  TOK_OP_DIAG     = 372,  TOK_OP_TRAP     = 373,
    
    //------------------------------------------------------------------------------------
    // Synthetic OP Code Tokens.
    //
    // ??? tbd
    //------------------------------------------------------------------------------------
   
};

//----------------------------------------------------------------------------------------
// The one line assembler works the assembly line string processed as a list of 
// tokens. 
//
//----------------------------------------------------------------------------------------
struct Token {
    
    char        name[ MAX_TOKEN_NAME_SIZE ] = { };
    TokTypeId   typ                         = TYP_NIL;
    TokId       tid                         = TOK_NIL;
     T64Word    val                         = 0;  
};

//----------------------------------------------------------------------------------------
// An instruction template consists of the instruction group bits ( 31,30 ), the op 
// code family bits ( 29, 28, 27, 26 ) and the option or mode bits ( 21, 20, 19 ). The
// mode bits are for some instruction the default and could be changed during the 
// parsing process. From the defined constants we will build the instruction template 
// which is stored for the opcode mnemonic in the token value field. The values for the
// opcode group and the opcode families are in the "T64-Types" include file.
//
//----------------------------------------------------------------------------------------
enum InstrTemplate : uint32_t {
    
    OPG_ALU      = ( OPC_GRP_ALU  << 30 ),
    OPG_MEM      = ( OPC_GRP_MEM  << 30 ),
    OPG_BR       = ( OPC_GRP_BR   << 30 ),
    OPG_SYS      = ( OPC_GRP_SYS  << 30 ),
  
    OPF_ADD      = ( OPC_ADD    << 26 ),
    OPF_SUB      = ( OPC_SUB    << 26 ),
    OPF_AND      = ( OPC_AND    << 26 ),
    OPF_OR       = ( OPC_OR     << 26 ),
    OPF_XOR      = ( OPC_XOR    << 26 ),
    OPF_CMP      = ( OPC_CMP_A  << 26 ),
    OPF_CMP_A    = ( OPC_CMP_A  << 26 ),
    OPF_CMP_B    = ( OPC_CMP_B  << 26 ),
    OPF_BITOP    = ( OPC_BITOP  << 26 ),
    OPF_SHAOP    = ( OPC_SHAOP  << 26 ),
    OPF_IMMOP    = ( OPC_IMMOP  << 26 ),
    OPF_LDO      = ( OPC_LDO    << 26 ),
    
    OPF_LD       = ( OPC_LD     << 26 ),
    OPF_ST       = ( OPC_ST     << 26 ),
    OPF_LDR      = ( OPC_LDR    << 26 ),
    OPF_STC      = ( OPC_STC    << 26 ),
    
    OPF_B        = ( OPC_B      << 26 ),
    OPF_BE       = ( OPC_BE     << 26 ),
    OPF_BR       = ( OPC_BR     << 26 ),
    OPF_BV       = ( OPC_BV     << 26 ),
    
    OPF_BB       = ( OPC_BB     << 26 ),
    OPF_CBR      = ( OPC_CBR    << 26 ),
    OPF_MBR      = ( OPC_MBR    << 26 ),
    OPF_ABR      = ( OPC_ABR    << 26 ),
    
    OPF_MR       = ( OPC_MR     << 26 ),
    OPF_LPA      = ( OPC_LPA    << 26 ),
    OPF_PRB      = ( OPC_PRB    << 26 ),
    OPF_TLB      = ( OPC_TLB    << 26 ),
    OPF_CA       = ( OPC_CA     << 26 ),
    OPF_MST      = ( OPC_MST    << 26 ),
    OPF_RFI      = ( OPC_RFI    << 26 ),
    OPF_TRAP     = ( OPC_TRAP   << 26 ),
    OPF_DIAG     = ( OPC_DIAG   << 26 ),
    OPF_NOP      = ( OPC_NOP    << 26 ),
    
    OPM_FLD_0    = ( 0U  << 19 ),
    OPM_FLD_1    = ( 1U  << 19 ),
    OPM_FLD_2    = ( 2U  << 19 ),
    OPM_FLD_3    = ( 3U  << 19 ),
    OPM_FLD_4    = ( 4U  << 19 ),
    OPM_FLD_5    = ( 5U  << 19 ),
    OPM_FLD_6    = ( 6U  << 19 ),
    OPM_FLD_7    = ( 7U  << 19 )
};

//----------------------------------------------------------------------------------------
// Instruction flags. They are used to keep track of instruction attributes used in 
// assembling the final instruction word. Examples are the data width encoded in the 
// opCode and the instruction mask. We define all options and masks for the respective
// instruction where they are valid.
//
//----------------------------------------------------------------------------------------
enum InstrFlags : uint32_t {
   
    IF_NIL      = 0,
    IF_A        = ( 1U << 1  ),
    IF_B        = ( 1U << 2  ),
    IF_C        = ( 1U << 3  ),
    IF_D        = ( 1U << 4  ),
    IF_F        = ( 1U << 5  ),
    IF_G        = ( 1U << 6  ),
    IF_H        = ( 1U << 7  ),
    IF_I        = ( 1U << 8  ),
    IF_L        = ( 1U << 9  ),
    IF_M        = ( 1U << 11 ),
    IF_N        = ( 1U << 12 ),
    IF_Q        = ( 1U << 13 ),
    IF_R        = ( 1U << 14 ),
    IF_S        = ( 1U << 15 ),
    IF_T        = ( 1U << 16 ),
    IF_U        = ( 1U << 17 ),
    IF_W        = ( 1U << 18 ),
    IF_Z        = ( 1U << 19 ),
    
    IF_EQ       = ( 1U << 24 ),
    IF_LT       = ( 1U << 25 ),
    IF_NE       = ( 1U << 26 ),
    IF_LE       = ( 1U << 27 ),
    IF_GT       = ( 1U << 28 ),
    IF_GE       = ( 1U << 29 ),
    IF_EV       = ( 1U << 30 ),
    IF_OD       = ( 1U << 31 ),
    
    IM_NIL      = 0,
    IM_ADD_OP   = ( IF_B | IF_H | IF_W | IF_D ),
    IM_SUB_OP   = ( IF_B | IF_H | IF_W | IF_D ),
    IM_AND_OP   = ( IF_B | IF_H | IF_W | IF_D | IF_N | IF_C ),
    IM_OR_OP    = ( IF_B | IF_H | IF_W | IF_D | IF_N | IF_C ),
    IM_XOR_OP   = ( IF_B | IF_H | IF_W | IF_D | IF_N ),
    IM_CMP_OP   = ( IF_B | IF_H | IF_W | IF_D | 
                    IF_EQ | IF_NE | IF_LT | IF_LE | IF_GT | IF_GE ),
    IM_EXTR_OP  = ( IF_S ),
    IM_DEP_OP   = ( IF_Z | IF_I ),
    IM_SHLxA_OP = ( IF_I ),
    IM_SHRxA_OP = ( IF_I ),
    IM_LDI_OP   = ( IF_L | IF_M | IF_U ),
    IM_LDO_OP   = ( IF_B | IF_H | IF_W | IF_D ),
    IM_LD_OP    = ( IF_B | IF_H | IF_W | IF_D | IF_U ),
    IM_ST_OP    = ( IF_B | IF_H | IF_W | IF_D ),
    IM_LDR_OP   = ( IF_D | IF_U ),
    IM_STC_OP   = ( IF_D ),
    IM_B_OP     = ( IF_G ),
    IM_BR_OP    = ( IF_W | IF_D | IF_Q ),
    IM_BV_OP    = ( IF_W | IF_D | IF_Q ),
    IM_BB_OP    = ( IF_T | IF_F ),
    IM_CBR_OP   = ( IF_EQ | IF_LT | IF_NE | IF_LE | IF_GT | IF_GE ),
    IM_MBR_OP   = ( IF_EQ | IF_LT | IF_NE | IF_LE | IF_GT | IF_GE | IF_EV | IF_OD ),
    IM_ABR_OP   = ( IF_EQ | IF_LT | IF_NE | IF_LE | IF_GT | IF_GE | IF_EV | IF_OD ),
    IM_MFIA_OP  = ( IF_A | IF_L | IF_R ),
    IM_ITLB_OP  = ( IF_I | IF_D ),
    IM_PTLB_OP  = ( IF_I | IF_D ),
    IM_PCA_OP   = ( IF_I | IF_D )
};

//----------------------------------------------------------------------------------------
// The global token table or the one line assembler. All reserved words are allocated 
// in this table. Each entry has the token name, the token id, the token type id, i.e.
// its type, and a value associated with the token. The value allows for a constant 
// token. The parser can directly use the value in an expression.
//
//----------------------------------------------------------------------------------------
const Token AsmTokTab[ ] = {
    
    //------------------------------------------------------------------------------------
    // General registers.
    //
    //------------------------------------------------------------------------------------
    {   .name = "R0",   .typ = TYP_GREG,    .tid = TOK_GR_0,    .val = 0    },
    {   .name = "R1",   .typ = TYP_GREG,    .tid = TOK_GR_1,    .val = 1    },
    {   .name = "R2",   .typ = TYP_GREG,    .tid = TOK_GR_2,    .val = 2    },
    {   .name = "R3",   .typ = TYP_GREG,    .tid = TOK_GR_3,    .val = 3    },
    {   .name = "R4",   .typ = TYP_GREG,    .tid = TOK_GR_4,    .val = 4    },
    {   .name = "R5",   .typ = TYP_GREG,    .tid = TOK_GR_5,    .val = 5    },
    {   .name = "R6",   .typ = TYP_GREG,    .tid = TOK_GR_6,    .val = 6    },
    {   .name = "R7",   .typ = TYP_GREG,    .tid = TOK_GR_7,    .val = 7    },
    {   .name = "R8",   .typ = TYP_GREG,    .tid = TOK_GR_8,    .val = 8    },
    {   .name = "R9",   .typ = TYP_GREG,    .tid = TOK_GR_9,    .val = 9    },
    {   .name = "R10",  .typ = TYP_GREG,    .tid = TOK_GR_10,   .val = 10   },
    {   .name = "R11",  .typ = TYP_GREG,    .tid = TOK_GR_11,   .val = 11   },
    {   .name = "R12",  .typ = TYP_GREG,    .tid = TOK_GR_12,   .val = 12   },
    {   .name = "R13",  .typ = TYP_GREG,    .tid = TOK_GR_13,   .val = 13   },
    {   .name = "R14",  .typ = TYP_GREG,    .tid = TOK_GR_14,   .val = 14   },
    {   .name = "R15",  .typ = TYP_GREG,    .tid = TOK_GR_15,   .val = 15   },
    
    //------------------------------------------------------------------------------------
    // Control registers.
    //
    //------------------------------------------------------------------------------------
    {   .name = "C0",   .typ = TYP_CREG,    .tid = TOK_CR_0,    .val = 0    },
    {   .name = "C1",   .typ = TYP_CREG,    .tid = TOK_CR_1,    .val = 1    },
    {   .name = "C2",   .typ = TYP_CREG,    .tid = TOK_CR_2,    .val = 2    },
    {   .name = "C3",   .typ = TYP_CREG,    .tid = TOK_CR_3,    .val = 3    },
    {   .name = "C4",   .typ = TYP_CREG,    .tid = TOK_CR_4,    .val = 4    },
    {   .name = "C5",   .typ = TYP_CREG,    .tid = TOK_CR_5,    .val = 5    },
    {   .name = "C6",   .typ = TYP_CREG,    .tid = TOK_CR_6,    .val = 6    },
    {   .name = "C7",   .typ = TYP_CREG,    .tid = TOK_CR_7,    .val = 7    },
    {   .name = "C8",   .typ = TYP_CREG,    .tid = TOK_CR_8,    .val = 8    },
    {   .name = "C9",   .typ = TYP_CREG,    .tid = TOK_CR_9,    .val = 9    },
    {   .name = "C10",  .typ = TYP_CREG,    .tid = TOK_CR_10,   .val = 10   },
    {   .name = "C11",  .typ = TYP_CREG,    .tid = TOK_CR_11,   .val = 11   },
    {   .name = "C12",  .typ = TYP_CREG,    .tid = TOK_CR_12,   .val = 12   },
    {   .name = "C13",  .typ = TYP_CREG,    .tid = TOK_CR_13,   .val = 13   },
    {   .name = "C14",  .typ = TYP_CREG,    .tid = TOK_CR_14,   .val = 14   },
    {   .name = "C15",  .typ = TYP_CREG,    .tid = TOK_CR_15,   .val = 15   },

    //------------------------------------------------------------------------------------
    // Runtime architecture register names for general registers.
    //
    //------------------------------------------------------------------------------------
    { .name = "T0",     .typ = TYP_GREG,    .tid = TOK_GR_1,    .val =  1   },
    { .name = "T1",     .typ = TYP_GREG,    .tid = TOK_GR_2,    .val =  2   },
    { .name = "T2",     .typ = TYP_GREG,    .tid = TOK_GR_3,    .val =  3   },
    { .name = "T3",     .typ = TYP_GREG,    .tid = TOK_GR_4,    .val =  4   },
    { .name = "T4",     .typ = TYP_GREG,    .tid = TOK_GR_5,    .val =  5   },
    { .name = "T5",     .typ = TYP_GREG,    .tid = TOK_GR_6,    .val =  6   },
    { .name = "T6",     .typ = TYP_GREG,    .tid = TOK_GR_7,    .val =  7   },
    
    { .name = "ARG3",   .typ = TYP_GREG,    .tid = TOK_GR_8,    .val =  8   },
    { .name = "ARG2",   .typ = TYP_GREG,    .tid = TOK_GR_9,    .val =  9   },
    { .name = "ARG1",   .typ = TYP_GREG,    .tid = TOK_GR_10,   .val =  10  },
    { .name = "ARG0",   .typ = TYP_GREG,    .tid = TOK_GR_11,   .val =  11  },
    
    { .name = "RET3",   .typ = TYP_GREG,    .tid = TOK_GR_8,    .val =  8   },
    { .name = "RET2",   .typ = TYP_GREG,    .tid = TOK_GR_9,    .val =  9   },
    { .name = "RET1",   .typ = TYP_GREG,    .tid = TOK_GR_10,   .val =  10  },
    { .name = "RET0",   .typ = TYP_GREG,    .tid = TOK_GR_11,   .val =  11  },
    
    { .name = "DP",     .typ = TYP_GREG,    .tid = TOK_GR_13,   .val =  13  },
    { .name = "RL",     .typ = TYP_GREG,    .tid = TOK_GR_14,   .val =  14  },
    { .name = "SP",     .typ = TYP_GREG,    .tid = TOK_GR_15,   .val =  15  },
    
    { .name = "SAR",    .typ = TYP_CREG,    .tid = TOK_CR_4,    .val =  2   },
    
    //------------------------------------------------------------------------------------
    // Assembler mnemonics. Like all other tokens, we have the name, the type and the 
    // token Id. In addition, the ".val" field contains the initial instruction mask
    // with opCode group, opCode family and the bits set in the first option field to
    // further qualify the instruction.
    //
    //------------------------------------------------------------------------------------
    {   .name   = "ADD",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_ADD,   .val = ( OPG_ALU | OPF_ADD    | OPM_FLD_0 ) },

    {   .name   = "SUB",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_SUB,   .val = ( OPG_ALU | OPF_SUB    | OPM_FLD_0 ) },

    {   .name   = "AND",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_AND,   .val = ( OPG_ALU | OPF_AND    | OPM_FLD_0 ) },

    {   .name   = "OR",         .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_OR,    .val = ( OPG_ALU | OPF_OR     | OPM_FLD_0 ) },
    
    {   .name   = "XOR",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_XOR,   .val = ( OPG_ALU | OPF_XOR    | OPM_FLD_0 ) },
    
    {   .name   = "CMP",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_CMP,   .val = ( OPG_ALU | OPF_CMP_A  | OPM_FLD_0 ) },
    
    {   .name   = "EXTR",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_EXTR,  .val = ( OPG_ALU | OPF_BITOP  | OPM_FLD_0 ) },

    {   .name   = "DEP",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_DEP,   .val = ( OPG_ALU | OPF_BITOP  | OPM_FLD_1 ) },

    {   .name   = "DSR",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_DSR,   .val = ( OPG_ALU | OPF_BITOP  | OPM_FLD_2 ) },
    
    {   .name   = "SHL1A",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_SHL1A, .val = ( OPG_ALU | OPF_SHAOP  | OPM_FLD_0 ) },

    {   .name   = "SHL2A",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_SHL2A, .val = ( OPG_ALU | OPF_SHAOP  | OPM_FLD_0 ) },

    {   .name   = "SHL3A",      .typ = TYP_OP_CODE, 
        .tid = TOK_OP_SHL3A,    .val = ( OPG_ALU | OPF_SHAOP  | OPM_FLD_0 ) },

    {   .name   = "SHR1A",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_SHR1A, .val = ( OPG_ALU | OPF_SHAOP  | OPM_FLD_2 ) },

    {   .name   = "SHR2A",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_SHR2A, .val = ( OPG_ALU | OPF_SHAOP  | OPM_FLD_2 ) },

    {   .name   = "SHR3A",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_SHR3A, .val = ( OPG_ALU | OPF_SHAOP  | OPM_FLD_2 ) },
    
    {   .name   = "LDIL",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_LDIL,   .val = ( OPG_ALU | OPF_IMMOP  | OPM_FLD_0 ) },

    {   .name   = "ADDIL",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_ADDIL, .val = ( OPG_ALU | OPF_IMMOP  | OPM_FLD_0 ) },

    {   .name   = "LDO",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_LDO,   .val = ( OPG_ALU | OPF_LDO    | OPM_FLD_0 ) },
    
    {   .name   = "LD",         .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_LD,    .val = ( OPG_MEM | OPF_LD     | OPM_FLD_0 ) },

    {   .name   = "LDR",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_LDR,   .val = ( OPG_MEM | OPF_LDR    | OPM_FLD_0 ) },

    {   .name   = "ST",         .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_ST,    .val = ( OPG_MEM | OPF_ST     | OPM_FLD_1 ) },

    {   .name   = "STC",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_STC,   .val = ( OPG_MEM | OPF_STC    | OPM_FLD_1 ) },
    
    {   .name   = "B",          .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_B,     .val = ( OPG_BR  | OPF_B      | OPM_FLD_0 ) },

    {   .name   = "BE",         .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_BE,    .val = ( OPG_BR  | OPF_BE     | OPM_FLD_0 ) },

    {   .name   = "BR",         .typ = TYP_OP_CODE, 
        .tid    =   TOK_OP_BR,  .val = ( OPG_BR  | OPF_BR     | OPM_FLD_0 ) },

    {   .name   = "BV",         .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_BV,    .val = ( OPG_BR  | OPF_BV     | OPM_FLD_0 ) },
   
    {   .name   = "BB",         .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_BB,    .val = ( OPG_BR  | OPF_BB     | OPM_FLD_0 ) },
    
    {   .name   = "CBR",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_CBR,   .val = ( OPG_BR  | OPF_CBR    | OPM_FLD_0 ) },

    {   .name   = "MBR",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_MBR,   .val = ( OPG_BR  | OPF_MBR    | OPM_FLD_0 ) },

    {   .name   = "ABR",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_ABR,   .val = ( OPG_BR  | OPF_ABR    | OPM_FLD_0 ) },
    
    {   .name   = "MFCR",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_MFCR,  .val = ( OPG_SYS | OPF_MR     | OPM_FLD_0 ) },

    {   .name   = "MTCR",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_MTCR,  .val = ( OPG_SYS | OPF_MR     | OPM_FLD_1 ) },

    {   .name   = "MFIA",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_MFIA,  .val = ( OPG_SYS | OPF_MR     | OPM_FLD_2 ) },
    
    {   .name   = "LPA",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_LPA,   .val = ( OPG_SYS | OPF_LPA    | OPM_FLD_0 ) },
    
    {   .name   = "PRB",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_PRB,   .val = ( OPG_SYS | OPF_PRB    | OPM_FLD_0 ) },

    {   .name   = "IITLB",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_IITLB, .val = ( OPG_SYS | OPF_TLB    | OPM_FLD_0 ) },

    {   .name   = "IDTLB",      .typ = TYP_OP_CODE,
        .tid    = TOK_OP_IDTLB, .val = ( OPG_SYS | OPF_TLB    | OPM_FLD_1 ) },

    {   .name   = "PITLB",      .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_PITLB, .val = ( OPG_SYS | OPF_TLB    | OPM_FLD_2 ) },

    {   .name   = "PDTLB",      .typ = TYP_OP_CODE,
        .tid    = TOK_OP_PDTLB, .val = ( OPG_SYS | OPF_TLB    | OPM_FLD_3 ) },
    
    {   .name   = "PICA",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_PICA,  .val = ( OPG_SYS | OPF_CA     | OPM_FLD_0 ) },

    {   .name   = "PDCA",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_PDCA,   .val = ( OPG_SYS | OPF_CA    | OPM_FLD_1 ) },

    {   .name   = "FICA",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_FICA,   .val = ( OPG_SYS | OPF_CA    | OPM_FLD_2 ) },

    {   .name   = "FDCA",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_FDCA,   .val = ( OPG_SYS | OPF_CA    | OPM_FLD_3 ) },
    
    {   .name   = "RSM",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_RSM,   .val = ( OPG_SYS | OPF_MST    | OPM_FLD_0 ) },

    {   .name   = "SSM",        .typ = TYP_OP_CODE,
        .tid    = TOK_OP_SSM,   .val = ( OPG_SYS | OPF_MST    | OPM_FLD_1 ) },
    
    {   .name   = "TRAP",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_TRAP,  .val = ( OPG_SYS | OPF_TRAP   | OPM_FLD_1 ) },
   
    {   .name   = "RFI",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_RFI,   .val = ( OPG_SYS | OPF_RFI    | OPM_FLD_0 ) },

    {   .name   = "DIAG",       .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_DIAG,  .val = ( OPG_SYS | OPF_DIAG   | OPM_FLD_0 ) },

    {   .name   = "NOP",        .typ = TYP_OP_CODE, 
        .tid    = TOK_OP_NOP,   .val = ( OPG_ALU | OPF_NOP    | OPM_FLD_0 ) },
    
    //------------------------------------------------------------------------------------
    // Assembler synthetic mnemonics. They are like the assembler mnemonics, except 
    // that they pre decode some bits settings in the option fields and reduce the 
    // ".<opt>" notation settings through meaningful instruction mnemonics.
    //
    // ??? tbd...
    //------------------------------------------------------------------------------------
   
};

const int MAX_ASM_TOKEN_TAB = sizeof( AsmTokTab ) / sizeof( Token );

//----------------------------------------------------------------------------------------
// Expression value. The analysis of an expression results in a value. Depending on 
// the expression type, the values are simple scalar values or a structured value, such
// as a register pair or virtual address.
//
//----------------------------------------------------------------------------------------
struct Expr {
    
    TokTypeId  typ;
    T64Word    val;  
};

const Expr INIT_EXPR = { .typ = TYP_NIL, .val = 0 }; 

//----------------------------------------------------------------------------------------
// Global variables for the tokenizer.
//
//----------------------------------------------------------------------------------------
int     lastErr                             = NO_ERR;
char    tokenLine[ MAX_INPUT_LINE_SIZE ]    = { 0 };
int     currentLineLen                      = 0;
int     currentCharIndex                    = 0;
int     currentTokCharIndex                 = 0;
char    currentChar                         = ' ';
Token   currentToken;

//----------------------------------------------------------------------------------------
// Forward declarations.
//
//----------------------------------------------------------------------------------------
void parseExpr( Expr *rExpr );

//----------------------------------------------------------------------------------------
// The token lookup function. We just do a linear search.
//
//----------------------------------------------------------------------------------------
int lookupToken( char *inputStr, const Token *tokTab ) {
    
    if (( strlen( inputStr ) == 0 ) || 
        ( strlen ( inputStr ) > MAX_TOKEN_NAME_SIZE )) return ( -1 );
    
    for ( int i = 0; i < MAX_ASM_TOKEN_TAB; i++  ) {
        
        if ( strcmp( inputStr, tokTab[ i ].name ) == 0 ) return ( i );
    }
    
    return ( -1 );
}

//----------------------------------------------------------------------------------------
// "nextChar" returns the next character from the token line string.
//
//----------------------------------------------------------------------------------------
void nextChar( ) {
    
    if ( currentCharIndex < currentLineLen ) {
        
        currentChar = tokenLine[ currentCharIndex ];
        currentCharIndex ++;
    }
    else currentChar = EOS_CHAR;
}

//----------------------------------------------------------------------------------------
// "addChar" adds a character to a string buffer if there is enough space.
//
//----------------------------------------------------------------------------------------
void addChar( char *buf, int size, char ch ) {
    
    int len = (int) strlen( buf );
    
    if ( len + 1 < size ) {
        
        buf[ len ]     = ch;
        buf[ len + 1 ] = 0;
    }
}

//----------------------------------------------------------------------------------------
// A little helper for upshift.
//
//----------------------------------------------------------------------------------------
void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) {
            
            str[ i ] = (char) toupper((int) str[ i ] );
        }
    }
}

//----------------------------------------------------------------------------------------
// Signed 64-bit numeric operations with checking.
//
//----------------------------------------------------------------------------------------
T64Word addOp( Expr *a, Expr *b ) {

    if ( a -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( b -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( willAddOverflow( a -> val, b -> val )) throw( ERR_NUMERIC_OVERFLOW );
    return ( a -> val + b -> val );
}

T64Word subOp( Expr *a, Expr *b ) {

    if ( a -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( b -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( willSubOverflow( a -> val, b -> val )) throw( ERR_NUMERIC_OVERFLOW );
    return ( a -> val - b -> val );
}

T64Word multOp( Expr *a, Expr *b ) {

    if ( a -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( b -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( willMultOverflow( a -> val, b -> val )) throw( ERR_NUMERIC_OVERFLOW );
    return ( a -> val * b -> val );
}

T64Word divOp( Expr *a, Expr *b ) {

    if ( a -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( b -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( willDivOverflow( a -> val, b -> val )) throw( ERR_NUMERIC_OVERFLOW );
    return ( a -> val / b -> val );
}

T64Word modOp( Expr *a, Expr *b ) {

    if ( a -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( b -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    if ( willDivOverflow( a -> val, b -> val )) throw( ERR_NUMERIC_OVERFLOW );
    return ( a -> val % b -> val );
}

//----------------------------------------------------------------------------------------
// "parseNum" will parse a number. We accept decimals, hexadecimals and binary 
// numbers. The numeric string can also contain "_" characters for readability.
// Hex numbers start with "0x", binary with "0b", decimals just with numeric digits.
//----------------------------------------------------------------------------------------
void parseNum( ) {
    
    currentToken.tid    = TOK_NUM;
    currentToken.typ    = TYP_NUM;
    currentToken.val    = 0;
    
    int     base        = 10;
    int     maxDigits   = 22;
    int     digits      = 0;
    T64Word tmpVal      = 0;
    
    if ( currentChar == '0' ) {
        
        nextChar( );
        if (( currentChar == 'X' ) || ( currentChar == 'x' )) {
            
            base        = 16;
            maxDigits   = 16;
            nextChar( );
        }
        else if (( currentChar == 'B' ) || ( currentChar == 'b' )) {
            
            base        = 2;
            maxDigits   = 64;
            nextChar( );
        }
        else if ( !isdigit( currentChar )) {

            return;
        }
    }
    
    do {
        
        if ( currentChar == '_' ) {
            
            nextChar( );
            continue;
        }
        else {

            if ( isdigit( currentChar )) {

                int digit = currentChar - '0';
                if ( digit >= base ) throw ( ERR_INVALID_NUM );

                tmpVal = ( tmpVal * base ) + digit;
            }
            else if (( base == 16         ) && 
                     ( currentChar >= 'a' ) && 
                     ( currentChar <= 'f' )) {

                tmpVal = ( tmpVal * base ) + currentChar - 'a' + 10;            
            }
            else if (( base == 16         ) && 
                     ( currentChar >= 'A' ) && 
                     ( currentChar <= 'F' )) {

                tmpVal = ( tmpVal * base ) + currentChar - 'A' + 10;
            }      
            else throw ( ERR_INVALID_NUM );
            
            nextChar( );
            digits ++;
            
            if ( digits > maxDigits ) throw ( ERR_INVALID_NUM );
        }
    }
    while (
        ( currentChar == '_' ) ||
        ( isdigit( currentChar )) ||
        ( base == 16 &&
          (( currentChar >= 'a' && currentChar <= 'f' ) ||
           ( currentChar >= 'A' && currentChar <= 'F' ) ) 
        )
    );

    currentToken.val = tmpVal;
}

//----------------------------------------------------------------------------------------
// "parseIdent" parses an identifier. It is a sequence of characters starting with an
// alpha character. An identifier found in the token table will assume the type and 
// value of the token found. Any other identifier is just an identifier symbol. There
// is one more thing. There are qualified constants that begin with a character followed 
// by a percent character, followed by a numeric value. During the character analysis,
// We first check for these kind of qualifiers and if found hand over to parse a number.
//
//----------------------------------------------------------------------------------------
void parseIdent( ) {
    
    currentToken.tid    = TOK_IDENT;
    currentToken.typ    = TYP_IDENT;
    currentToken.val    = 0;
    
    char identBuf[ MAX_INPUT_LINE_SIZE ] = "";
    
    if (( currentChar == 'L' ) || ( currentChar == 'l' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.val &= 0x00000000FFFFF000;
                currentToken.val >>= 12;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    else if (( currentChar == 'R' ) || ( currentChar == 'r' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.val &= 0x0000000000000FFF;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    else if (( currentChar == 'M' ) || ( currentChar == 'm' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.val &= 0x000FFFFF00000000;
                currentToken.val >>= 32;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    else if (( currentChar == 'U' ) || ( currentChar == 'u' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.val &= 0xFFF0000000000000;
                currentToken.val >>= 52;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    
    while (( isalnum( currentChar )) || ( currentChar == '_' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
    }
    
    upshiftStr( identBuf );
    
    int index = lookupToken( identBuf, AsmTokTab );
    
    if ( index == - 1 ) {
        
        strcpy( currentToken.name, identBuf );
        currentToken.typ = TYP_IDENT;
        currentToken.tid = TOK_IDENT;
        currentToken.val = 0;
    }
    else currentToken = AsmTokTab[ index ];
}

//----------------------------------------------------------------------------------------
// "nextToken" is the entry point to the lexer.
//
//----------------------------------------------------------------------------------------
void nextToken( ) {
    
    currentToken.name[ 0 ]  = 0;
    currentToken.typ        = TYP_NIL;
    currentToken.tid        = TOK_NIL;
    currentToken.val        = 0;
    
    while (( currentChar == ' ' ) || 
            ( currentChar == '\n' ) || 
            ( currentChar == '\r' )) nextChar( );
    
    currentTokCharIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
        parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
        parseNum( );
    }
    else if ( currentChar == '.' ) {
        
        currentToken.typ   = TYP_SYM;
        currentToken.tid   = TOK_PERIOD;
        nextChar( );
    }
    else if ( currentChar == '+' ) {
        
        currentToken.tid = TOK_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MOD;
        nextChar( );
    }
    else if ( currentChar == '&' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_AND;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_COMMA;
        nextChar( );
    }
    else if ( currentChar == ';' ) {
        
        currentCharIndex    = currentLineLen;
        currentToken.typ    = TYP_NIL;
        currentToken.tid    = TOK_EOS;
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentToken.typ    = TYP_NIL;
        currentToken.tid    = TOK_EOS;
    }
    else {
        
        currentToken.tid = TOK_ERR;
        throw ( ERR_INVALID_CHAR_IN_IDENT );
    }
}

//----------------------------------------------------------------------------------------
// Initialize the tokenizer and get the first token.
//
//----------------------------------------------------------------------------------------
void setupTokenizer( char *inputStr ) {
    
    strcpy( tokenLine, inputStr );
    upshiftStr( tokenLine );
    
    currentLineLen          = (int) strlen( tokenLine);
    currentCharIndex        = 0;
    currentTokCharIndex     = 0;
    currentChar             = ' ';
    
    nextToken( );
}

//----------------------------------------------------------------------------------------
// Parser helper functions.
//
//----------------------------------------------------------------------------------------
static inline bool isToken( TokId tid ) {
    
    return ( currentToken.tid == tid );
}

static inline bool isTokenTyp( TokTypeId typ ) {
    
    return ( currentToken.typ == typ );
}

static inline void acceptEOS( ) {
    
    if ( ! isToken( TOK_EOS )) throw ( ERR_EXTRA_TOKEN_IN_STR );
}

static inline void acceptComma( ) {
    
    if ( isToken( TOK_COMMA )) nextToken( );
    else throw ( ERR_EXPECTED_COMMA );
}

static inline void acceptLparen( ) {
    
    if ( isToken( TOK_LPAREN )) nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
}

static inline void acceptRparen( ) {
    
    if ( isToken( TOK_RPAREN )) nextToken( );
    else throw ( ERR_EXPECTED_RPAREN );
}

//----------------------------------------------------------------------------------------
// "parseFactor" parses the factor syntax part of an expression.
//
//      <factor> -> <number>            |
//                  <gregId>            |
//                  <cregId>            |
//                  "~" <factor>        |
//                  "(" <expr> ")"
//
//----------------------------------------------------------------------------------------
void parseFactor( Expr *rExpr ) {
    
    rExpr -> typ  = TYP_NIL;
    rExpr -> val = 0;
    
    if ( isToken( TOK_NUM )) {
        
        rExpr -> typ    = TYP_NUM;
        rExpr -> val = currentToken.val;
        nextToken( );
    }
    else if ( isTokenTyp( TYP_GREG )) {
        
        rExpr -> typ    = TYP_GREG;
        rExpr -> val = currentToken.val;
        nextToken( );
    }
    else if ( isTokenTyp( TYP_CREG )) {
        
        rExpr -> typ    = TYP_CREG;
        rExpr -> val = currentToken.val;
        nextToken( );
    }
    else if ( isToken( TOK_NEG )) {
        
        parseFactor( rExpr );
        rExpr -> val = ~ rExpr -> val;
    }
    else if ( isToken( TOK_LPAREN )) {
        
        nextToken( );
        parseExpr( rExpr );
        acceptRparen( );
    }
    else throw ( ERR_INVALID_EXPR );
}

//----------------------------------------------------------------------------------------
// "parseTerm" parses the term syntax.
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
//----------------------------------------------------------------------------------------
void parseTerm( Expr *rExpr ) {
    
    Expr lExpr;
    
    parseFactor( rExpr );
    
    while (( isToken( TOK_MULT ))   ||
           ( isToken( TOK_DIV  ))   ||
           ( isToken( TOK_MOD  ))   ||
           ( isToken( TOK_AND  )))  {
        
        uint8_t op = currentToken.tid;
        
        nextToken( );
        parseFactor( &lExpr );
        
        if ( rExpr -> typ != lExpr.typ ) throw ( ERR_EXPR_TYPE_MATCH );
        
        switch( op ) {

            case TOK_MULT: rExpr -> val = multOp( rExpr, &lExpr );  break;
            case TOK_DIV:  rExpr -> val = divOp( rExpr, &lExpr );   break;
            case TOK_MOD:  rExpr -> val = modOp( rExpr, &lExpr );   break;
            case TOK_AND:  rExpr -> val = rExpr -> val & lExpr.val; break;
        }
    }
}

//----------------------------------------------------------------------------------------
// "parseExpr" parses the expression syntax.
//
//      <expr>      ->  [ ( "+" | "-" ) ] <term> { <exprOp> <term> }
//      <exprOp>    ->  "+" | "-" | "|" | "^"
//
//----------------------------------------------------------------------------------------
void parseExpr( Expr *rExpr ) {
    
    Expr lExpr;
    
    if ( isToken( TOK_PLUS )) {
        
        nextToken( );
        parseTerm( rExpr );
        
        if ( ! ( rExpr -> typ == TYP_NUM )) throw ( ERR_EXPECTED_NUMERIC );
    }
    else if ( isToken( TOK_MINUS )) {
        
        nextToken( );
        parseTerm( rExpr );

        if ( rExpr -> typ == TYP_NUM ) rExpr -> val = - rExpr -> val;
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    else parseTerm( rExpr );
    
    while (( isToken( TOK_PLUS  )) ||
           ( isToken( TOK_MINUS )) ||
           ( isToken( TOK_OR    )) ||
           ( isToken( TOK_XOR   ))) {
        
        uint8_t op = currentToken.tid;
        
        nextToken( );
        parseTerm( &lExpr );
        
        if ( rExpr -> typ != lExpr.typ ) throw ( ERR_EXPR_TYPE_MATCH );
        
        switch ( op ) {
   
            case TOK_PLUS:  rExpr -> val = addOp( rExpr, &lExpr );      break;
            case TOK_MINUS: rExpr -> val = subOp( rExpr, &lExpr );      break;
            case TOK_OR:    rExpr -> val = rExpr -> val | lExpr.val;    break;
            case TOK_XOR:   rExpr -> val = rExpr -> val ^ lExpr.val;    break;
        }
    }
}

//----------------------------------------------------------------------------------------
// Helper functions for instruction fields.
//
//----------------------------------------------------------------------------------------
inline void depositInstrFieldS( T64Instr *instr, int bitpos, int len, T64Word value ) {
    
    if ( isInRangeForInstrBitFieldS( value, len )) 
        depositInstrField( instr, bitpos, len, value );
    else throw ( ERR_IMM_VAL_RANGE );
}

inline void depositInstrFieldU( T64Instr *instr, int bitpos, int len, uint32_t value ) {
    
    if ( isInRangeForInstrBitFieldU( value, len )) 
        depositInstrField( instr, bitpos, len, value );
    else throw ( ERR_IMM_VAL_RANGE );
}

inline void depositInstrImm13( T64Instr *instr, int val ) {

    depositInstrFieldS( instr, 0, 13, val );
}

inline void depositInstrScaledImm13( T64Instr *instr, int val ) {
   
    val = val >> extractInstrFieldU( *instr, 13, 2 );
    depositInstrFieldS( instr, 0, 13, val );
}

inline void depositInstrImm15( T64Instr *instr, int val ) {

    depositInstrFieldS( instr, 0, 15, val );
}

inline void depositInstrImm19( T64Instr *instr, int val ) {

    depositInstrFieldS( instr, 0, 19, val );
}

inline void depositInstrImm20U( T64Instr *instr, uint32_t val ) {

    depositInstrFieldU( instr, 0, 20, val );
}

inline bool hasDataWidthFlags( uint32_t instrFlags ) {
    
    return (( instrFlags & IF_B ) || ( instrFlags & IF_H ) ||
            ( instrFlags & IF_W ) || ( instrFlags & IF_D ));
}

inline bool hasCmpCodeFlags( uint32_t instrFlags ) {
    
    return (( instrFlags & IF_EQ ) || ( instrFlags & IF_NE ) ||
            ( instrFlags & IF_LT ) || ( instrFlags & IF_LE ) ||
            ( instrFlags & IF_GT ) || ( instrFlags & IF_GE ) ||
            ( instrFlags & IF_EV ) || ( instrFlags & IF_OD ));
}

inline void replaceInstrGroupField( T64Instr *instr, uint32_t instrMask ) {
    
    *instr &= 0x3FFFFFFF;
    *instr |= instrMask & 0xC0000000;
}

inline void replaceInstrOpCodeField( T64Instr *instr, uint32_t instrMask ) {
    
    *instr &= 0xC3FFFFFF;
    *instr |= instrMask & 0x3c000000;
}

//----------------------------------------------------------------------------------------
// Set the condition field for compare type instructions based on the instruction
// flags.
//
//----------------------------------------------------------------------------------------
void setInstrCompareCondField( uint32_t *instr, uint32_t instrFlags ) {

    if      ( instrFlags & IF_EQ )  depositInstrFieldU( instr, 19, 3, 0 );
    else if ( instrFlags & IF_LT )  depositInstrFieldU( instr, 19, 3, 1 );
    else if ( instrFlags & IF_GT )  depositInstrFieldU( instr, 19, 3, 2 );
    else if ( instrFlags & IF_EV )  depositInstrFieldU( instr, 19, 3, 3 );
    else if ( instrFlags & IF_NE )  depositInstrFieldU( instr, 19, 3, 4 );
    else if ( instrFlags & IF_GE )  depositInstrFieldU( instr, 19, 3, 5 );
    else if ( instrFlags & IF_LE )  depositInstrFieldU( instr, 19, 3, 6 );
    else if ( instrFlags & IF_OD )  depositInstrFieldU( instr, 19, 3, 7 );
}

//----------------------------------------------------------------------------------------
// Set the data width field for memory access type instructions based on the instruction
// flags. If no date width flags is set, we set the default, which is "D".
//
//----------------------------------------------------------------------------------------
void setInstrDwField( uint32_t *instr, uint32_t instrFlags ) {
    
    if (! hasDataWidthFlags( instrFlags )) instrFlags |= IF_D;
   
    if      ( instrFlags & IF_B )   depositInstrFieldU( instr, 13, 2, 0 );
    else if ( instrFlags & IF_H )   depositInstrFieldU( instr, 13, 2, 1 );
    else if ( instrFlags & IF_W )   depositInstrFieldU( instr, 13, 2, 2 );
    else if ( instrFlags & IF_D )   depositInstrFieldU( instr, 13, 2, 3 );
}

//----------------------------------------------------------------------------------------
// For the indexed addressing mode, the offset needs to be in alignment with the
// data width.
//
//----------------------------------------------------------------------------------------
void checkOfsAlignment( int ofs, uint32_t instrFlags ) {

    if ( ! hasDataWidthFlags( instrFlags )) instrFlags |= IF_D;

    if ( ! ( instrFlags & IF_B )) { 

        if ( ! ((( instrFlags & IF_H ) && ( isAlignedDataAdr( ofs, 2 ))) ||
            (( instrFlags & IF_W ) && ( isAlignedDataAdr( ofs, 4 ))) ||
            (( instrFlags & IF_D ) && ( isAlignedDataAdr( ofs, 8 ))))) 
            throw( ERR_INVALID_OFS );
    }
}

//----------------------------------------------------------------------------------------
// "parseInstrOptions" will analyze the opCode option string. An opCode option 
// string is a sequence of characters after the ".". We will look at each char in 
// the "name" and set the options for the particular instruction. There are also 
// options where the option is a multi-character sequence. They cannot be in the 
// same group with a sequence of individual individual characters. Currently only
// the CMP, CBR and MBR instructions are such cases.
//
// The assembler can handle multiple ".xxx" sequences. One could for example put 
// each individual character in a separate ".x" location. Once we have all options 
// seen, we check that there are no conflicting options where only one option out 
// of an option group can be set.
//
// There are some instruction options that are exclusive to each other. There will 
// be a check for this case after all options have been analyzed and the option 
// mask has been created. There are also some instructions that require at least
// one option from a particular group to be set. And there are instructions that 
// have a default option if none from a particular group has been set. This will
// also be handled after all options have been analyzed.
//
// One more quirk. The branch instruction "B" conflicts with the ".B" option. We
// need to check the context. The tokenizer recognizes an opCOde, we patch the 
// token to be an identifier.
//
// Once all instruction options have been analyzed, we do a final check to see if 
// the options set are valid for the particular instruction. For example, the EXTR  
// instruction cannot have any data width options set.
//
//----------------------------------------------------------------------------------------
void parseInstrOptions( uint32_t *instrFlags, uint32_t instrOpToken ) {
    
    uint32_t instrMask = IM_NIL;
    
    while ( isToken( TOK_PERIOD )) {
        
        nextToken( );
        
        if ( isToken( TOK_OP_B )) {
            
            currentToken.typ = TYP_IDENT;
            currentToken.tid = TOK_IDENT;
            strncpy( currentToken.name, "B", 4 );
        }
        
        if ( ! isToken( TOK_IDENT )) throw ( ERR_EXPECTED_INSTR_OPT );

        char        *optBuf     = currentToken.name;
        int         optStrLen   = (int) strlen( optBuf );
        
        if      ( strcmp( optBuf, ((char *) "EQ" )) == 0 ) instrMask |= IF_EQ;
        else if ( strcmp( optBuf, ((char *) "LT" )) == 0 ) instrMask |= IF_LT;
        else if ( strcmp( optBuf, ((char *) "NE" )) == 0 ) instrMask |= IF_NE;
        else if ( strcmp( optBuf, ((char *) "GE" )) == 0 ) instrMask |= IF_GE;
        else if ( strcmp( optBuf, ((char *) "GT" )) == 0 ) instrMask |= IF_GT;
        else if ( strcmp( optBuf, ((char *) "LE" )) == 0 ) instrMask |= IF_LE;
        else if ( strcmp( optBuf, ((char *) "OD" )) == 0 ) instrMask |= IF_OD;
        else if ( strcmp( optBuf, ((char *) "EV" )) == 0 ) instrMask |= IF_EV;
            
        else {
            
            for ( int i = 0; i < optStrLen; i ++ ) {
                
                switch ( optBuf[ i ] ) {
                 
                    case 'A': instrMask = instrMask |= IF_A; break;
                    case 'B': instrMask = instrMask |= IF_B; break;
                    case 'C': instrMask = instrMask |= IF_C; break;
                    case 'D': instrMask = instrMask |= IF_D; break;
                    case 'F': instrMask = instrMask |= IF_F; break;
                    case 'G': instrMask = instrMask |= IF_G; break;
                    case 'H': instrMask = instrMask |= IF_H; break;
                    case 'I': instrMask = instrMask |= IF_I; break;
                    case 'L': instrMask = instrMask |= IF_L; break;
                    case 'M': instrMask = instrMask |= IF_M; break;
                    case 'N': instrMask = instrMask |= IF_N; break;
                    case 'Q': instrMask = instrMask |= IF_Q; break;
                    case 'S': instrMask = instrMask |= IF_S; break;
                    case 'T': instrMask = instrMask |= IF_T; break;
                    case 'U': instrMask = instrMask |= IF_U; break;
                    case 'W': instrMask = instrMask |= IF_W; break;
                    case 'Z': instrMask = instrMask |= IF_Z; break;
                    default: throw ( ERR_INVALID_INSTR_OPT );
                }
            }
        }

        nextToken( );
    }

    int cnt = 0;
    if ( instrMask & IF_W   ) cnt ++;
    if ( instrMask & IF_D   ) cnt ++;
    if ( instrMask & IF_Q   ) cnt ++;
    if ( cnt >  1 ) throw ( ERR_DUPLICATE_INSTR_OPT );

    cnt = 0;
    if ( instrMask & IF_B   ) cnt ++;
    if ( instrMask & IF_H   ) cnt ++;
    if ( instrMask & IF_W   ) cnt ++;
    if ( instrMask & IF_D   ) cnt ++;
    if ( cnt >  1 ) throw ( ERR_DUPLICATE_INSTR_OPT );
    
    cnt = 0;
    if ( instrMask & IF_EQ ) cnt ++;
    if ( instrMask & IF_LT ) cnt ++;
    if ( instrMask & IF_NE ) cnt ++;
    if ( instrMask & IF_LE ) cnt ++;
    if ( instrMask & IF_GT ) cnt ++;
    if ( instrMask & IF_GE ) cnt ++;
    if ( instrMask & IF_OD ) cnt ++;
    if ( instrMask & IF_EV ) cnt ++;
    if ( cnt > 1 ) throw ( ERR_DUPLICATE_INSTR_OPT );
    
    cnt = 0;
    if ( instrMask & IF_T ) cnt ++;
    if ( instrMask & IF_F ) cnt ++;
    if ( cnt > 1 ) throw ( ERR_DUPLICATE_INSTR_OPT );
    
    cnt = 0;
    if ( instrMask & IF_L ) cnt ++;
    if ( instrMask & IF_M ) cnt ++;
    if ( instrMask & IF_U ) cnt ++;
    if ( cnt > 1 ) throw ( ERR_DUPLICATE_INSTR_OPT );

    if ((( instrOpToken == TOK_OP_LD  ) && (( instrMask & IM_LD_OP )  == 0 )) ||
        (( instrOpToken == TOK_OP_ST  ) && (( instrMask & IM_ST_OP )  == 0 )) ||
        (( instrOpToken == TOK_OP_LDO ) && (( instrMask & IM_LDO_OP ) == 0 )) ||
        (( instrOpToken == TOK_OP_LDR )                                     ) ||
        (( instrOpToken == TOK_OP_STC )                                     )) {
            
         instrMask |= IF_D;
    }

    if ((( instrOpToken == TOK_OP_ADD  ) && ( instrMask & ~IM_ADD_OP    )) ||
        (( instrOpToken == TOK_OP_SUB  ) && ( instrMask & ~IM_SUB_OP    )) ||
        (( instrOpToken == TOK_OP_AND  ) && ( instrMask & ~IM_AND_OP    )) ||
        (( instrOpToken == TOK_OP_OR   ) && ( instrMask & ~IM_OR_OP     )) ||
        (( instrOpToken == TOK_OP_XOR  ) && ( instrMask & ~IM_XOR_OP    )) ||
        (( instrOpToken == TOK_OP_CMP  ) && ( instrMask & ~IM_CMP_OP    )) ||

        (( instrOpToken == TOK_OP_EXTR ) && ( instrMask & ~IM_EXTR_OP   )) ||
        (( instrOpToken == TOK_OP_DEP  ) && ( instrMask & ~IM_DEP_OP    )) ||

        (( instrOpToken == TOK_OP_SHL1A ) && ( instrMask & ~IM_SHLxA_OP )) ||
        (( instrOpToken == TOK_OP_SHL2A ) && ( instrMask & ~IM_SHLxA_OP )) ||
        (( instrOpToken == TOK_OP_SHL3A ) && ( instrMask & ~IM_SHLxA_OP )) ||

        (( instrOpToken == TOK_OP_SHR1A ) && ( instrMask & ~IM_SHRxA_OP )) ||
        (( instrOpToken == TOK_OP_SHR2A ) && ( instrMask & ~IM_SHRxA_OP )) ||
        (( instrOpToken == TOK_OP_SHR3A ) && ( instrMask & ~IM_SHRxA_OP )) ||

        (( instrOpToken == TOK_OP_LDO   ) && ( instrMask & ~IM_LDO_OP   )) ||
        (( instrOpToken == TOK_OP_LDIL  ) && ( instrMask & ~IM_LDI_OP   )) ||
        (( instrOpToken == TOK_OP_ADDIL ) && ( instrMask & ~IM_NIL      )) || 
        
        (( instrOpToken == TOK_OP_ABR  ) && ( instrMask & ~IM_ABR_OP    )) ||
        (( instrOpToken == TOK_OP_CBR  ) && ( instrMask & ~IM_CBR_OP    )) ||
        (( instrOpToken == TOK_OP_MBR  ) && ( instrMask & ~IM_MBR_OP    )) ||

        (( instrOpToken == TOK_OP_LD   ) && ( instrMask & ~IM_LD_OP     )) ||
        (( instrOpToken == TOK_OP_ST   ) && ( instrMask & ~IM_ST_OP     )) ||
        (( instrOpToken == TOK_OP_LDR  ) && ( instrMask & ~IM_LDR_OP    )) ||
        (( instrOpToken == TOK_OP_STC  ) && ( instrMask & ~IM_STC_OP    )) ||
    
        (( instrOpToken == TOK_OP_B    ) && ( instrMask & ~IM_B_OP      )) ||
        (( instrOpToken == TOK_OP_BR   ) && ( instrMask & ~IM_BR_OP     )) ||
        (( instrOpToken == TOK_OP_BV   ) && ( instrMask & ~IM_BV_OP     )) ||
        (( instrOpToken == TOK_OP_BB   ) && ( instrMask & ~IM_BB_OP     )) ||

        (( instrOpToken == TOK_OP_MFIA ) && ( instrMask & ~IM_MFIA_OP   )) ||
        
        (( instrOpToken == TOK_OP_NOP  ) && ( instrMask & ~IM_NIL       ))) { 
    
        throw ( ERR_INVALID_INSTR_OPT );
    }

    *instrFlags = instrMask;
}

//----------------------------------------------------------------------------------------
// The following routines parse a general register and store the register ID in the 
// respective instruction field.
//
//----------------------------------------------------------------------------------------
void acceptRegR( uint32_t *instr ) {
    
    Expr rExpr = INIT_EXPR;
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) depositInstrRegR( instr, (uint32_t) rExpr.val );
    else throw ( ERR_EXPECTED_GENERAL_REG );
}

void acceptRegA( uint32_t *instr ) {

    Expr rExpr = INIT_EXPR;

    parseExpr( &rExpr );
    if (  rExpr.typ == TYP_GREG ) depositInstrRegA( instr, (uint32_t) rExpr.val );
    else throw ( ERR_EXPECTED_GENERAL_REG );
}

void acceptRegB( uint32_t *instr ) {

    Expr rExpr = INIT_EXPR;

    parseExpr( &rExpr );
    if (  rExpr.typ == TYP_GREG ) depositInstrRegB( instr, (uint32_t) rExpr.val );
    else throw ( ERR_EXPECTED_GENERAL_REG );
}

//----------------------------------------------------------------------------------------
// The "NOP" instruction. Easy case.
//
//      NOP
//
//----------------------------------------------------------------------------------------
void parseNopInstr( uint32_t *instr, uint32_t instrOpToken ) {
    
    nextToken( );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseModeTypeInstr" parses all instructions of type "mode" based on the syntax, 
// which will determine the exact instruction layout and option setting. 
//
// Instruction group ALU syntax:
//
//      opCode [ "." <opt> ] <targetReg> "," <sourceReg> "," <num>          
//      opCode [ "." <opt> ] <targetReg> "," <sourceReg> "," <sourceRegB> 
//
// Instruction group MEM syntax:
//
//      opCode [ "." <opt> ] <targetReg> "," [ <num> ]  "(" <baseReg> ")"  
//      opCode [ "." <opt> ] <targetReg> "," <indexReg> "(" <baseReg> ")"  
//
// There are a couple of exceptions to handle. First, the CMP instruction mnemonic
// needs to be mapped to CMP_A and CMP_B code, depending on the actual instruction 
// format.
//
// When we have the indexed addressing mode, the numeric offset needs to be in 
// alignment with the data width.
//
//----------------------------------------------------------------------------------------
void parseModeTypeInstr( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );
    acceptRegR( instr );
    acceptComma( );

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {

        replaceInstrGroupField( instr, OPG_MEM );

        if ( instrOpToken == TOK_OP_CMP ) 
           replaceInstrOpCodeField( instr, OPF_CMP_A );

        checkOfsAlignment( rExpr.val, instrFlags );
        setInstrDwField( instr, instrFlags );
        depositInstrScaledImm13( instr, (uint32_t) rExpr.val );
  
        acceptLparen( );
        acceptRegB( instr );        
        acceptRparen( );
        acceptEOS( );
    }
    else if ( rExpr.typ == TYP_GREG ) {
        
        if ( isToken( TOK_COMMA )) {
            
            if ( hasDataWidthFlags( instrFlags )) throw ( ERR_INVALID_INSTR_MODE );

            replaceInstrGroupField( instr, OPG_ALU );

            int tmpRegId = (int) rExpr.val;
            
            nextToken( );
            parseExpr( &rExpr );
            if ( rExpr.typ == TYP_NUM ) {

                if ( instrOpToken == TOK_OP_CMP ) 
                    replaceInstrOpCodeField( instr, OPF_CMP_B );
                else
                    depositInstrBit( instr, 19, true );

                depositInstrRegB( instr, tmpRegId );
                depositInstrImm15( instr, (uint32_t) rExpr.val );
            }
            else if ( rExpr.typ == TYP_GREG ) {

                if ( instrOpToken == TOK_OP_CMP )
                    replaceInstrOpCodeField( instr, OPF_CMP_A );
            
                depositInstrRegB( instr, tmpRegId );
                depositInstrRegA( instr, (uint32_t) rExpr.val );
            }
            else throw ( ERR_EXPECTED_GENERAL_REG );
        
            acceptEOS( );
        }
        else if ( isToken( TOK_LPAREN )) {

            replaceInstrGroupField( instr, OPG_MEM );
            
            if ( instrOpToken == TOK_OP_CMP )
                replaceInstrOpCodeField( instr, OPF_CMP_B );
            else
                depositInstrBit( instr, 19, true );

            setInstrDwField( instr, instrFlags );
            depositInstrRegA( instr, (uint32_t) rExpr.val );
            
            nextToken( );
            acceptRegB( instr );
            acceptRparen( );
            acceptEOS( );
        }
        else throw ( ERR_EXPECTED_COMMA );
    }
    
    if (( instrOpToken == TOK_OP_AND ) || ( instrOpToken == TOK_OP_OR )){
        
        if ( instrFlags & IF_C ) depositInstrBit( instr, 20, true );
        if ( instrFlags & IF_N ) depositInstrBit( instr, 21, true );
    }
    else if ( instrOpToken == TOK_OP_XOR ) {
        
        if ( instrFlags & IF_N ) depositInstrBit( instr, 21, true );
    }
    else if ( instrOpToken == TOK_OP_CMP ) {

        if ( ! hasCmpCodeFlags( instrFlags )) throw( ERR_INVALID_INSTR_MODE );
        setInstrCompareCondField( instr, instrFlags );
    }
}

//----------------------------------------------------------------------------------------
// "parseInstrEXTR" parses the extract instruction. The instruction has two basic 
// formats. When the "A" bit is set, the position will be obtained from the shift 
// amount control register. Otherwise, the position it is encoded in the instruction.
//
//      EXTR [ ".S“ ]  <targetReg> "," <sourceReg> "," <pos> "," <len"
//      EXTR [ ".S" ]  <targetReg> "," <sourceReg> ", "SAR", <len"
//
//----------------------------------------------------------------------------------------
void parseInstrEXTR( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    uint32_t    pos         = 0;
    uint32_t    len         = 0;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );
    acceptRegR( instr );
    acceptComma( );
    acceptRegB( instr ); 
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {
        
       depositInstrFieldU( instr, 6, 6, (uint32_t) rExpr.val );
       pos = (uint32_t) rExpr.val;
    }
    else if (( rExpr.typ == TYP_CREG ) && ( rExpr.val == 2 )) {
        
        depositInstrBit( instr, 13, true );
    }
    else throw ( ERR_EXPECTED_POS_ARG );
    
    acceptComma( );

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) { 
        
        depositInstrFieldU( instr, 0, 6, (uint32_t) rExpr.val );
        len = (uint32_t) rExpr.val;
    }
    else throw ( ERR_EXPECTED_LEN_ARG );
 
    if ( instrFlags & IF_S ) depositInstrBit( instr, 12, true );
    
    acceptEOS( );

    if ( pos + len > 64 ) throw ( ERR_BIT_RANGE_EXCEEDS );
}

//----------------------------------------------------------------------------------------
// "parseInstrDEP" parses the deposit instruction. The instruction has two basic 
// formats. When the "I" option is set, the value to deposit is an immediate value, 
// else the data comes from a general register. When "SAR" is specified instead of a 
// bit position, the "A" bit is encoded in the instruction. When the value to deposit 
// is a numeric value, the "I" bit is set.
//
//      DEP [ ".“ Z ] <targetReg> "," <sourceReg> "," <pos> "," <len>"
//      DEP [ ".“ Z ] <targetReg> "," <sourceReg> "," "SAR" "," <len>"
//      DEP [ ".“ Z ] <targetReg> "," <val>,      "," <pos> "," <len>
//      DEP [ ".“ Z ] <targetReg> "," <val>       "," "SAR" "," <len>
//
//----------------------------------------------------------------------------------------
void parseInstrDEP( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr      = INIT_EXPR;
    uint32_t    instrFlags = IF_NIL;
    uint32_t    pos         = 0;
    uint32_t    len         = 0;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );

    if ( instrFlags & IF_Z ) depositInstrBit( instr, 12, true );
    
    acceptRegR( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) {
        
        depositInstrRegB( instr, (uint32_t) rExpr.val );
    }
    else if ( rExpr.typ == TYP_NUM )    {
        
        depositInstrFieldU( instr, 15, 4, (uint32_t) rExpr.val );
        depositInstrBit( instr, 14, true );
    }
    else throw ( ERR_EXPECTED_POS_ARG );
    
    acceptComma( );

    parseExpr( &rExpr );
    if (( rExpr.typ == TYP_CREG ) && ( rExpr.val == 2 )) {
        
        depositInstrBit( instr, 13, true );
    }
    else if ( rExpr.typ == TYP_NUM ) {
        
        depositInstrFieldU( instr, 6, 6, (uint32_t) rExpr.val );
        pos = (uint32_t) rExpr.val;
    }
    else throw ( ERR_EXPECTED_POS_ARG );
    
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) { 
        
        depositInstrFieldU( instr, 0, 6, (uint32_t) rExpr.val );
        len = (uint32_t) rExpr.val;
    }
    else throw ( ERR_EXPECTED_LEN_ARG );
    
    acceptEOS( );

    if ( pos + len > 64 ) throw ( ERR_BIT_RANGE_EXCEEDS );
}

//----------------------------------------------------------------------------------------
// "parseInstrDSR" parses the double shift instruction. There are two flavors. If the
// "length operand is the "SAR" register, the "A" bit is encoded in the instruction, 
// other wise the instruction "len" field.
//
//      DSR <targetReg> "," <sourceRegA> "," <sourceRegB> "," <len>
//      DSR <targetReg> "," <sourceRegA> "," <sourceRegB> "," SAR
//
//----------------------------------------------------------------------------------------
void parseInstrDSR( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr rExpr = INIT_EXPR;
    
    nextToken( );
    acceptRegR( instr );
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    acceptRegA( instr ); 
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {
        
        depositInstrFieldU( instr, 0, 6, (uint32_t) rExpr.val );
    }
    else if (( rExpr.typ == TYP_CREG ) && ( rExpr.val == 2 )) {
        
        depositInstrBit( instr, 13, true );
    }
    else throw ( ERR_EXPECTED_LEN_ARG );
    
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// The SHLA instruction performs a shift left of "B" by the instruction encoded shift 
// amount and adds the "A" register to it. If the ".I" option is set, the RegA field is
// interpreted as a number.
//
//      SHLxA       <targetReg> "," <sourceRegB> "," <sourceRegA>
//      SHLxA ".I"  <targetReg> "," <sourceRegA> "," <val>
//
//----------------------------------------------------------------------------------------
void parseInstrSHLxA( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );

    switch( instrOpToken ) {
        
        case TOK_OP_SHL1A: depositInstrField( instr, 13, 2, 1 ); break;
        case TOK_OP_SHL2A: depositInstrField( instr, 13, 2, 2 ); break;
        case TOK_OP_SHL3A: depositInstrField( instr, 13, 2, 3 ); break;
    }
    
    acceptRegR( instr );
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) {
        
        depositInstrField( instr, 19, 3, 0 );
        depositInstrRegA( instr, (uint32_t) rExpr.val );
    }
    else if ( rExpr.typ == TYP_NUM ) {
        
        depositInstrField( instr, 19, 3, 1 );
        depositInstrImm13( instr, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// The SHRA instruction performs a shift right of "B" by the instruction encoded shift
// amount and adds the "A" register to it. for a numeric value instead of RegA, the 
// register field is a numeric value and the "I" option is set.
//
//      SHRxA       <targetReg> "," <sourceRegB> "," <sourceRegA>
//      SHRxA ".I"  <targetReg> "," <sourceRegA> "," <val>
//
//----------------------------------------------------------------------------------------
void parseInstrSHRxA( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr      = INIT_EXPR;
    uint32_t    instrFlags = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );
   
    switch( instrOpToken ) {
        
        case TOK_OP_SHR1A: depositInstrField( instr, 13, 2, 1 ); break;
        case TOK_OP_SHR2A: depositInstrField( instr, 13, 2, 2 ); break;
        case TOK_OP_SHR3A: depositInstrField( instr, 13, 2, 3 ); break;
    }
    
    acceptRegR( instr );
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) {
        
        depositInstrField( instr, 19, 3, 2 );
        depositInstrRegA( instr, (uint32_t) rExpr.val );
    }
    else if ( rExpr.typ == TYP_NUM ) {
        
        depositInstrField( instr, 19, 3, 3 );
        depositInstrImm13( instr, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// The IMM-OP instruction group deals with the loading of immediate subfield and the
// addition of the ADDIL instruction, which will add the encoded value left shifted to
// <sourceReg>. The result is in R1.
//
//      LDI [ .L/S/U ] <targetReg> "," <val>
//      ADDIL <sourceReg> "," <val>
//
//----------------------------------------------------------------------------------------
void parseInstrImmOp( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr      = INIT_EXPR;
    uint32_t    instrFlags = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );

    if      ( instrFlags & IF_L ) depositInstrFieldU( instr, 20, 2, 1 );
    else if ( instrFlags & IF_M ) depositInstrFieldU( instr, 20, 2, 2 );
    else if ( instrFlags & IF_U ) depositInstrFieldU( instr, 20, 2, 3 );
    else                          depositInstrFieldU( instr, 20, 2, 1 );

    acceptRegR( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) depositInstrImm20U( instr, (uint32_t) rExpr.val );
    else throw ( ERR_EXPECTED_NUMERIC );
    
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// The "LDO" instruction computes the address of an operand, and stores the result
// in "R". Like the MemOp family, the LDO instruction has a dataWidth field.
//
//      LDO [.B/H/W/D] <targetReg> "," [ <ofs> ] "(" <baseReg> ")"
//      LDO <targetReg> "," [ <indexReg> ] "(" <baseReg> ")"
//
//----------------------------------------------------------------------------------------
void parseInstrLDO( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );
    setInstrDwField( instr, instrFlags );
    acceptRegR( instr );
    acceptComma( );

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {
        
            checkOfsAlignment( rExpr.val, instrFlags );
            depositInstrScaledImm13( instr, (uint32_t) rExpr.val );
    }
    else if ( rExpr.typ == TYP_GREG) {

        if (( hasDataWidthFlags( instrFlags )) && ( ! ( instrFlags & IF_D ))) {
            
            throw ( ERR_INVALID_INSTR_OPT );
        }

        depositInstrFieldU( instr, 13, 2, 0 );
        depositInstrBit( instr, 19, true );
        depositInstrRegA( instr, (uint32_t) rExpr.val );
    }
    else depositInstrScaledImm13( instr, 0 );
    
    acceptLparen( );
    acceptRegB( instr );
    acceptRparen( );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseMemOp" parses the load and store instruction family. The LD and ST family 
// can have an option for specifying the data width. The LDR and STC instruction do 
// not have an option. However we set the dataWidth field for them to "D".
//
//       LD  [.B/H/W/D/U ] <targetReg> ","  [ <ofs> ] "(" <baseReg> ")"
//       LD  [.B/H/W/D/U ] <targetReg> ","  [ <indexReg> ] "(" <baseReg> ")"
//
//       ST  [.B/H/W/D ] <sourceReg> "," [ <ofs> ] "(" <baseReg> ")"
//       ST  [.B/H/W/D ] <sourceReg> ","  [ <indexReg> ] "(" <baseReg> ")"
//
//       LDR               <targetReg> "," [ <ofs> ] "(" <baseReg> ")"
//       STC               <sourceReg> "," [ <ofs> ] "(" <baseReg> ")"
//
//----------------------------------------------------------------------------------------
void parseMemOp( T64Instr *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );  
    setInstrDwField( instr, instrFlags );
    acceptRegR( instr );
    acceptComma( );

    if ( instrFlags & IF_U ) depositInstrBit( instr, 20, true );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {
        
        depositInstrBit( instr, 19, false );
        depositInstrScaledImm13( instr, (uint32_t) rExpr.val );
    }
    else if ( rExpr.typ == TYP_GREG) {
        
        if (( instrOpToken == TOK_OP_LDR ) || ( instrOpToken == TOK_OP_STC )) {
         
            throw ( ERR_INVALID_INSTR_MODE );
        }
        
        depositInstrBit( instr, 19, true );
        depositInstrRegA( instr, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_NUMERIC );

    acceptLparen( );
    acceptRegB( instr );
    acceptRparen( );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseOpB" parses the branch instruction. The branch instruction may have the "gate"
// option.
//
//      B [ .G ] <ofs> [ "," <Reg R> ]
//
//----------------------------------------------------------------------------------------
void parseInstrB( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );
    if ( instrFlags & IF_G ) depositInstrBit( instr, 19, true );

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {

        if ( ! isAlignedOfs( rExpr.val, 4 )) throw( ERR_INVALID_OFS );
     
        rExpr.val = rExpr.val >> 2;
        depositInstrImm19( instr, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_BR_OFS );

    if ( isToken( TOK_COMMA )) {

        nextToken( );
        acceptRegR( instr );
        acceptEOS( );
    }
    else if ( ! isToken( TOK_EOS )) {

        throw ( ERR_EXPECTED_COMMA );
    }  
}

//----------------------------------------------------------------------------------------
// "parseOpBE" is the external branch. We add and offset to RegB which forms the target 
// offset. Optionally, we can specify a return link register.
//
//      BE  [ <ofs> ] "(" <regB> ")" [ "," <regR> ]
//
//----------------------------------------------------------------------------------------
void parseInstrBE( uint32_t *instr, uint32_t instrOpToken ) {

    Expr rExpr = INIT_EXPR;
    
    nextToken( );
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {

        if ( ! isAlignedOfs( rExpr.val, 4 )) throw( ERR_INVALID_OFS );

        depositInstrImm15( instr, ((uint32_t) rExpr.val >> 2 ));    

        acceptLparen( );
        acceptRegB( instr );
        acceptRparen( );
    }
    else if ( rExpr.typ == TYP_GREG ) {

        depositInstrRegB( instr, rExpr.val );
    }
    
    if ( isToken( TOK_COMMA )) {

        nextToken( );
        acceptRegR( instr );
        acceptEOS( );
    }
    else if ( ! isToken( TOK_EOS )) {

        throw ( ERR_EXPECTED_COMMA );
    }  
}

//----------------------------------------------------------------------------------------
// "parseOpBR" is the IA-relative branch adding RegB to IA. Optionally, we can specify
// a return link register.
//
//      BR [.W/D/Q] <regB> [ "," <regR> ]
//
//----------------------------------------------------------------------------------------
void parseInstrBR( uint32_t *instr, uint32_t instrOpToken ) {

    uint32_t instrFlags  = IF_NIL;
   
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );

    if      ( instrFlags & IF_W ) depositInstrField( instr, 13, 2, 0 );
    else if ( instrFlags & IF_D ) depositInstrField( instr, 13, 2, 1 );
    else if ( instrFlags & IF_Q ) depositInstrField( instr, 13, 2, 2 );
    else                          depositInstrField( instr, 13, 2, 0 );

    acceptRegB( instr );
    
    if ( isToken( TOK_COMMA )) {

        nextToken( );
        acceptRegR( instr );
        acceptEOS( );
    }
    else if ( ! isToken( TOK_EOS )) {

        throw ( ERR_EXPECTED_COMMA );
    }  
}

//----------------------------------------------------------------------------------------
// "parseOpBV" is the vectored branch. We add RegX and RegB, which form the target 
// offset. Optionally, we can specify a return link register.
//
//      BV [.W/D/Q] [ <RegX> "," ] "(" <RegB> ")" [ "," <regR> ]
//
//----------------------------------------------------------------------------------------
void parseInstrBV( uint32_t *instr, uint32_t instrOpToken ) {

    Expr    rExpr        = INIT_EXPR;
    uint32_t instrFlags  = IF_NIL;
   
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );

    if      ( instrFlags & IF_W ) depositInstrField( instr, 13, 2, 0 );
    else if ( instrFlags & IF_D ) depositInstrField( instr, 13, 2, 1 );
    else if ( instrFlags & IF_Q ) depositInstrField( instr, 13, 2, 2 );
    else                          depositInstrField( instr, 13, 2, 0 );

    if ( isTokenTyp( TYP_GREG )) {

        acceptRegA( instr );
    }

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) depositInstrRegB( instr, rExpr.val );
    else throw ( ERR_EXPECTED_LPAREN );
    
    if ( isToken( TOK_COMMA )) {

        nextToken( );
        acceptRegR( instr );
        acceptEOS( );
    }
    else if ( ! isToken( TOK_EOS )) {

        throw ( ERR_EXPECTED_COMMA );
    }  
}

//----------------------------------------------------------------------------------------
// "parseOpBB" is the branch on bit instruction.
//
//      BB ".T/F" <regB> "," <pos> "," <target>
//      BB ".T/F" <regB> "," "SAR" "," <target>
//
//----------------------------------------------------------------------------------------
void parseInstrBB( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );
    
    if      ( instrFlags & IF_T ) depositInstrBit( instr, 19, true );
    else if ( instrFlags & IF_F ) depositInstrBit( instr, 19, false );
    else                          throw ( ERR_EXPECTED_INSTR_OPT );
    
    acceptRegR( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {
        
        depositInstrFieldU( instr, 13, 6, (uint32_t) rExpr.val );
    }
    else if (( rExpr.typ == TYP_CREG ) && ( rExpr.val == 2 )) {
        
        depositInstrBit( instr, 20, true );
    }
    else throw ( ERR_EXPECTED_POS_ARG );

    acceptComma( );

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {
     
        if ( ! isAlignedOfs( rExpr.val, 4 )) throw( ERR_INVALID_OFS );

        rExpr.val = rExpr.val >> 2;
        depositInstrImm13( instr, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_BR_OFS );
   
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseOpCBR" performa a compare and a branch based on the condition.
//
//      ABR ".EQ/NE/LT/LE/GT/GE/OD/EV" RegR "," RegB "," <ofs>
//      CBR ".EQ/NE/LT/LE/GT/GE/OD/EV" RegR "," RegB "," <ofs>
//      MBR ".EQ/NE/LT/LE/GT/GE/OD/EV" RegR "," RegB "," <ofs>
//
//----------------------------------------------------------------------------------------
void parseInstrXBR( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr        rExpr       = INIT_EXPR;
    uint32_t    instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );
    if ( ! hasCmpCodeFlags( instrFlags )) throw( ERR_EXPECTED_INSTR_OPT );
    setInstrCompareCondField( instr, instrFlags );
    
    acceptRegR( instr );
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {

        if ( ! isAlignedOfs( rExpr.val, 4 )) throw( ERR_INVALID_OFS );
        
        rExpr.val = rExpr.val >> 2;
        depositInstrImm15( instr, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_BR_OFS );
    
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrMFCR" copies a control register to a general register.
//
//      MFCR <RegB> "," <CReg>
//
//----------------------------------------------------------------------------------------
void parseInstrMFCR( uint32_t *instr, uint32_t instrOpToken ) {
   
    Expr rExpr = INIT_EXPR;

    nextToken( );
    acceptRegB( instr );
    acceptComma( );

    parseExpr( &rExpr );
    if (  rExpr.typ == TYP_CREG ) depositInstrField( instr, 0, 6, (uint32_t) rExpr.val );
    else throw ( ERR_EXPECTED_CONTROL_REG );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrMTCR" copies a general register to control register and saves the old
// in a general register.
//
//      MTCR <RegB> "," <CReg> [ "," <RegR> ]
//
//----------------------------------------------------------------------------------------
void parseInstrMTCR( uint32_t *instr, uint32_t instrOpToken ) {

     Expr rExpr = INIT_EXPR;
   
    nextToken( );
    acceptRegB( instr );
    acceptComma( );

    parseExpr( &rExpr );
    if (  rExpr.typ == TYP_CREG ) depositInstrField( instr, 0, 6, (uint32_t) rExpr.val );
    else throw ( ERR_EXPECTED_CONTROL_REG );

    if ( isToken ( TOK_COMMA )) {

        nextToken( );
        acceptRegR( instr );
    }

    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrMFIA" copies the instruction offset to a general register.
//
//      MFIA [.A/L/R] <RegR>
//
//----------------------------------------------------------------------------------------
void parseInstrMFIA( uint32_t *instr, uint32_t instrOpToken ) {

    uint32_t instrFlags  = IF_NIL;
    
    nextToken( );
    parseInstrOptions( &instrFlags, instrOpToken );

    if      ( instrFlags & IF_A ) depositInstrFieldU( instr, 19, 2, 0 );
    else if ( instrFlags & IF_L ) depositInstrFieldU( instr, 19, 2, 1 );
    else if ( instrFlags & IF_R ) depositInstrFieldU( instr, 19, 2, 2 );
    else                          depositInstrFieldU( instr, 19, 2, 0 );
   
    nextToken( );
    acceptRegR( instr );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrLPA" returns the physical address of a virtual address. It is very 
// similar to the memory type instruction load and store, except it does just do 
// address translation. If the page is not in main memory, a zero is returned.
//
//       LPA <targetReg> ","  [ <indexReg> ] "(" <baseReg> ")"
//
//----------------------------------------------------------------------------------------
void parseInstrLPA( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr rExpr = INIT_EXPR;

    nextToken( );
    acceptRegR( instr );
    acceptComma( );

    if ( isTokenTyp( TYP_GREG )) {

        acceptRegA( instr );
    }

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) depositInstrRegB( instr, rExpr.val );
    else throw ( ERR_EXPECTED_LPAREN );

    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrPRB" probes a virtual address for access. The "P/U" indicate privileged
//  and user mode access.
//
//      PRB <RegR> "," <RegB> "," <RegA>
//      PRB <RegR> "," <RegB> "," <val>
//
//----------------------------------------------------------------------------------------
void parseInstrPRB( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr rExpr = INIT_EXPR;

    nextToken( );
    acceptRegR( instr );
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) {
        
        depositInstrRegA( instr, (uint32_t) rExpr.val );
        depositInstrFieldU( instr, 13, 2, 3 );
    }
    else if ( rExpr.typ == TYP_NUM  ) {
        
        depositInstrField( instr, 13, 2, rExpr.val );
    }
    else throw ( ERR_EXPECTED_PRB_ARG );
    
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrInsertTlbOp" inserts a translation in the TLB. RegB contains the virtual
// address. RegA contains the info on access rights and physical address. The result
// is in RegR.
//
//      IITLB <targetReg> "," <RegB> "," <RegA>
//      IDTLB <targetReg> "," <RegB> "," <RegA>
//
//----------------------------------------------------------------------------------------
void parseInstrInsertTlb( uint32_t *instr, uint32_t instrOpToken ) {

    nextToken( );
    acceptRegR( instr );
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    acceptRegA( instr );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrPurgeTlbOp" removes a translation in the TLB. RegB contains the virtual
// address. The result is in RegR.
//
//      PITLB <targetReg> "," [ <RegX> ] "(" <RegB> ")"
//      PDTLB <targetReg> "," [ <RegX> ] "(" <RegB> ")"
//
//----------------------------------------------------------------------------------------
void parseInstrPurgeTlb( uint32_t *instr, uint32_t instrOpToken ) {

    Expr rExpr = INIT_EXPR;

    nextToken( );
    acceptRegR( instr );
    acceptComma( );

    if ( isTokenTyp( TYP_GREG )) {

        acceptRegA( instr );
    }

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) depositInstrRegB( instr, rExpr.val );
    else throw ( ERR_EXPECTED_LPAREN );

    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrFlushCacheOp" assemble the cache flush operation. This only applies 
// to data or unified caches.
//
//      FICA <targetReg> "," [ <RegX> ] "(" <RegB> ")"
//      FDCA <targetReg> "," [ <RegX> ] "(" <RegB> ")"
//
//----------------------------------------------------------------------------------------
void parseInstrFlushCache( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr rExpr = INIT_EXPR;

    nextToken( );
    acceptRegR( instr );
    acceptComma( );

    if ( isTokenTyp( TYP_GREG )) {

        acceptRegA( instr );
    }

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) depositInstrRegB( instr, rExpr.val );
    else throw ( ERR_EXPECTED_LPAREN );

    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrPurgeCacheOp" assemble the cache purge operation.
//
//      PICA <targetReg> "," [ <RegX> ] "(" <RegB> ")"
//      PDCA <targetReg> "," [ <RegX> ] "(" <RegB> ")"
//
//----------------------------------------------------------------------------------------
void parseInstrPurgeCache( uint32_t *instr, uint32_t instrOpToken ) {

    Expr rExpr = INIT_EXPR;

    nextToken( );
    acceptRegR( instr );
    acceptComma( );

    if ( isTokenTyp( TYP_GREG )) {

        acceptRegA( instr );
    }

    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_GREG ) depositInstrRegB( instr, rExpr.val );
    else throw ( ERR_EXPECTED_LPAREN );

    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrSregOp" sets or clears status register bits. 
//
//      RSM <RegR> "," <val>
//      SSM <RegR> "," <val>
//
//----------------------------------------------------------------------------------------
void parseInstrSregOp( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr rExpr = INIT_EXPR;
   
    nextToken( );
    acceptRegR( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) { 
        
        depositInstrFieldU( instr, 0, 8, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
    
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// The "RFI" instruction is the return from interrupt method. So far it is only
// the instruction with no further options and arguments.
//
//      RFI
//
//----------------------------------------------------------------------------------------
void parseInstrRFI( uint32_t *instr, uint32_t instrOpToken ) {
    
    nextToken( );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrDIAG" is the general purpose diagnostic instruction. It accepts two 
// registers and returns a result in the target register.
//
//      DIAG <RegR> "," <val> "," <RegB> "," <RegA"
//
//----------------------------------------------------------------------------------------
void parseInstrDIAG( uint32_t *instr, uint32_t instrOpToken ) {
    
    Expr rExpr = INIT_EXPR;
   
    nextToken( );
    acceptRegR( instr );
    acceptComma( );
    
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {
        
        depositInstrField( instr, 19, 3, (uint32_t) rExpr.val >> 2 );
        depositInstrField( instr, 20, 2, (uint32_t) rExpr.val );
    }
    else throw ( ERR_EXPECTED_DIAG_OP );
    
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    acceptRegA( instr );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseInstrTrapOp" assembles the trap operations.
//
//      Generic. TRAP <info> "," RegB "," RegA
//
// We have up to 32 trap group IDs. Group zero should be the BRK group ID.
//
//----------------------------------------------------------------------------------------
void parseInstrTrapOp( uint32_t *instr, uint32_t instrOpToken ) {

    Expr rExpr = INIT_EXPR;

    nextToken( );
    parseExpr( &rExpr );
    if ( rExpr.typ == TYP_NUM ) {

        depositInstrField( instr, 13, 2, ( rExpr.val & 0x3 ));
        depositInstrField( instr, 19, 3, (( rExpr.val >> 2 ) & 0x7 ));
    }
    else throw ( ERR_EXPECTED_NUMERIC );
    
    acceptComma( );
    acceptRegB( instr );
    acceptComma( );
    acceptRegA( instr );
    acceptEOS( );
}

//----------------------------------------------------------------------------------------
// "parseLine" will take the input string and parse the line for an instruction. In 
// the one-line case, there is only the opCode mnemonic and the argument list. No 
// labels, comments are ignored.
//
//----------------------------------------------------------------------------------------
void parseLine( char *inputStr, uint32_t *instr ) {
    
    setupTokenizer( inputStr );
 
    if ( isTokenTyp( TYP_OP_CODE )) {
        
        uint32_t instrOpToken   = currentToken.tid;
        *instr                  = (uint32_t) currentToken.val;
        
        switch( instrOpToken ) {
                
            case TOK_OP_NOP:    parseNopInstr( instr, instrOpToken );       break;
                
            case TOK_OP_ADD:
            case TOK_OP_SUB:
            case TOK_OP_AND:
            case TOK_OP_OR:
            case TOK_OP_XOR:
            case TOK_OP_CMP:    parseModeTypeInstr( instr, instrOpToken );  break;
                
            case TOK_OP_EXTR:   parseInstrEXTR( instr, instrOpToken );      break;
            case TOK_OP_DEP:    parseInstrDEP( instr, instrOpToken );       break;
            case TOK_OP_DSR:    parseInstrDSR( instr, instrOpToken );       break;
                
            case TOK_OP_SHL1A:
            case TOK_OP_SHL2A:
            case TOK_OP_SHL3A:  parseInstrSHLxA( instr, instrOpToken );     break;
                
            case TOK_OP_SHR1A:
            case TOK_OP_SHR2A:
            case TOK_OP_SHR3A:  parseInstrSHRxA( instr, instrOpToken );     break;
                
            case TOK_OP_LDIL:
            case TOK_OP_ADDIL:  parseInstrImmOp( instr, instrOpToken );     break;
                
            case TOK_OP_LDO:    parseInstrLDO( instr, instrOpToken );       break;
                
            case TOK_OP_LD:
            case TOK_OP_LDR:
            case TOK_OP_ST:
            case TOK_OP_STC:    parseMemOp( instr, instrOpToken );          break;
                
            case TOK_OP_B:      parseInstrB( instr, instrOpToken );         break;
            case TOK_OP_BE:     parseInstrBE( instr, instrOpToken );        break;
            case TOK_OP_BR:     parseInstrBR( instr, instrOpToken );        break;
            case TOK_OP_BV:     parseInstrBV( instr, instrOpToken );        break;
            case TOK_OP_BB:     parseInstrBB( instr, instrOpToken );        break;
                
            case TOK_OP_ABR:
            case TOK_OP_MBR:
            case TOK_OP_CBR:    parseInstrXBR( instr, instrOpToken );       break;
                
            case TOK_OP_MFCR:   parseInstrMFCR( instr, instrOpToken );      break;
            case TOK_OP_MTCR:   parseInstrMTCR( instr, instrOpToken );      break;

            case TOK_OP_MFIA:   parseInstrMFIA( instr, instrOpToken );      break;
                
            case TOK_OP_LPA:    parseInstrLPA( instr, instrOpToken );       break;
                
            case TOK_OP_PRB:    parseInstrPRB( instr, instrOpToken );       break;
                
            case TOK_OP_IITLB:
            case TOK_OP_IDTLB:  parseInstrInsertTlb( instr, instrOpToken ); break;

            case TOK_OP_PITLB:
            case TOK_OP_PDTLB:  parseInstrPurgeTlb( instr, instrOpToken );  break;
                
            case TOK_OP_PICA:
            case TOK_OP_PDCA:   parseInstrPurgeCache( instr, instrOpToken );break;

            case TOK_OP_FICA:
            case TOK_OP_FDCA:   parseInstrFlushCache( instr, instrOpToken );break;
                
            case TOK_OP_SSM:
            case TOK_OP_RSM:    parseInstrSregOp( instr, instrOpToken );    break;
                
            case TOK_OP_RFI:    parseInstrRFI( instr, instrOpToken );       break;
                
            case TOK_OP_DIAG:   parseInstrDIAG( instr, instrOpToken );      break;
                
            case TOK_OP_TRAP:   parseInstrTrapOp( instr, instrOpToken );    break;
                
            default: throw ( ERR_INVALID_OP_CODE );
        }
    }
    else throw ( ERR_EXPECTED_OPCODE );
}

} // namespace

//----------------------------------------------------------------------------------------
// A simple one line assembler. We will parse a one line input string for a 
// valid instruction, using the syntax of the real assembler. There will be no 
// labels and comments, only the opcode and the operands.
//
//----------------------------------------------------------------------------------------
T64Assemble::T64Assemble( ) { }

int T64Assemble::assembleInstr( char *inputStr, uint32_t *instr ) {
    
    try {
        
        parseLine( inputStr, instr );
        return ( NO_ERR );
    }
    catch ( ErrId errNum ) {
        
        *instr  = 0;
        lastErr = errNum;
        return ( errNum );
    }
}

int T64Assemble::getErrId( ) {
    
    return ( lastErr );
}

int T64Assemble::getErrPos( ) {
    
    return ( currentTokCharIndex );
}

const char *T64Assemble::getErrStr( int errId ) {
    
    for ( int i = 0; i < MAX_ERR_MSG_TAB; i++ ) {
        
        if ( ErrMsgTable[ i ].msgId == errId ) return ( ErrMsgTable[ i ].msg );
    }
    
    return ( "Unknown Error Id" );
}
