//----------------------------------------------------------------------------------------
//
//  Twin64Sim - A 64-bit CPU Simulator - Simulator Constants and Tables
//
//----------------------------------------------------------------------------------------
// SimTables contains the simulator constant table data, such as the command line
// options, token tables and so on.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Simulator Constants and Tables
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

#include "T64-SimDeclarations.h"

//----------------------------------------------------------------------------------------
// The simulator offers a set of command line options. Each option has a name, an 
// argument type and a value returned when the option is found.
//
//----------------------------------------------------------------------------------------
static struct SimCmdLineOptions optionTable[ ] = {

        { "help",       CL_OPT_NO_ARGUMENT,        CL_ARG_VAL_HELP },
        { "version",    CL_OPT_NO_ARGUMENT,        CL_ARG_VAL_VERSION },
        { "verbose",    CL_OPT_NO_ARGUMENT,        CL_ARG_VAL_VERBOSE },
        { "configfile", CL_OPT_REQUIRED_ARGUMENT,  CL_ARG_VAL_CONFIG_FILE },
        { "logfile",    CL_OPT_REQUIRED_ARGUMENT,  CL_ARG_VAL_LOG_FILE },
        {0,             CL_OPT_NO_ARGUMENT,        0}
    };

//----------------------------------------------------------------------------------------
// The global command interpreter token table. All reserved words are allocated
// in this table. Each entry has the token name, the token id, the token type 
// id, i.e. its type, and a value associated with the token. The value allows 
// for a constant token. The parser can directly use the value in expressions.
//
//----------------------------------------------------------------------------------------
const SimToken cmdTokTab[ ] = {
    
    //------------------------------------------------------------------------------------
    // General tokens.
    //
    //------------------------------------------------------------------------------------
    { .name = "NIL",        .typ = TYP_SYM,     .tid = TOK_NIL,   .u = { .val = 0  }},
    { .name = "ALL",        .typ = TYP_SYM,     .tid = TOK_ALL                      },
    { .name = "SYS",        .typ = TYP_SYM,     .tid = TOK_SYS                      },
    { .name = "MOD",        .typ = TYP_SYM,     .tid = TOK_MOD                      },
    { .name = "PROC",       .typ = TYP_SYM,     .tid = TOK_PROC                     },              
    { .name = "CPU",        .typ = TYP_SYM,     .tid = TOK_CPU                      },
    { .name = "ITLB",       .typ = TYP_SYM,     .tid = TOK_ITLB                     },
    { .name = "DTLB",       .typ = TYP_SYM,     .tid = TOK_DTLB                     },
    { .name = "ICACHE",     .typ = TYP_SYM,     .tid = TOK_ICACHE                   },
    { .name = "DCACHE",     .typ = TYP_SYM,     .tid = TOK_DCACHE                   },
    { .name = "MEM",        .typ = TYP_SYM,     .tid = TOK_MEM                      },
    { .name = "IO",         .typ = TYP_SYM,     .tid = TOK_IO                       },
    { .name = "TEXT",       .typ = TYP_SYM,     .tid = TOK_TEXT                     },

    { .name = "DEC",        .typ = TYP_SYM,     .tid = TOK_DEC,   .u = { .val = 10 }},
    { .name = "HEX",        .typ = TYP_SYM,     .tid = TOK_HEX,   .u = { .val = 16 }},
    { .name = "CODE",       .typ = TYP_SYM,     .tid = TOK_CODE                     },
    
    { .name = "COMMANDS",   .typ = TYP_CMD,     .tid = CMD_SET                      },
    { .name = "WCOMMANDS",  .typ = TYP_WCMD,    .tid = WCMD_SET                     },
    { .name = "PREDEFINED", .typ = TYP_P_FUNC,  .tid = PF_SET                       },
    
    //------------------------------------------------------------------------------------
    // Command Line tokens.
    //
    //------------------------------------------------------------------------------------
    { .name = "HELP",       .typ = TYP_CMD,     .tid = CMD_HELP                     },
    { .name = "?",          .typ = TYP_CMD,     .tid = CMD_HELP                     },
    
    { .name = "EXIT",       .typ = TYP_CMD,     .tid = CMD_EXIT                     },
    { .name = "E",          .typ = TYP_CMD,     .tid = CMD_EXIT                     },
    
    { .name = "HIST",       .typ = TYP_CMD,     .tid = CMD_HIST                     },
    { .name = "DO",         .typ = TYP_CMD,     .tid = CMD_DO                       },
    { .name = "REDO",       .typ = TYP_CMD,     .tid = CMD_REDO                     },
    { .name = "ENV",        .typ = TYP_CMD,     .tid = CMD_ENV                      },
    { .name = "XF",         .typ = TYP_CMD,     .tid = CMD_XF                       },
    { .name = "LF",         .typ = TYP_CMD,     .tid = CMD_LF                       },
    { .name = "W",          .typ = TYP_CMD,     .tid = CMD_WRITE_LINE               },
    
    { .name = "DM",         .typ = TYP_CMD,     .tid = CMD_DM                       },
    { .name = "DW",         .typ = TYP_CMD,     .tid = CMD_DW                       },
    { .name = "NM",         .typ = TYP_CMD,     .tid = CMD_NM                       },
    { .name = "RM",         .typ = TYP_CMD,     .tid = CMD_RM                       },
    { .name = "RESET",      .typ = TYP_CMD,     .tid = CMD_RESET                    },
    { .name = "RUN",        .typ = TYP_CMD,     .tid = CMD_RUN                      },
    { .name = "STEP",       .typ = TYP_CMD,     .tid = CMD_STEP                     },
    { .name = "S",          .typ = TYP_CMD,     .tid = CMD_STEP                     },
    
    { .name = "MR",         .typ = TYP_CMD,     .tid = CMD_MR                       },
    { .name = "DA",         .typ = TYP_CMD,     .tid = CMD_DA                       },
    { .name = "MA",         .typ = TYP_CMD,     .tid = CMD_MA                       },
    
    { .name = "IITLB",      .typ = TYP_CMD,     .tid = CMD_ITLB_I                   },
    { .name = "IDTLB",      .typ = TYP_CMD,     .tid = CMD_ITLB_D                   },
    { .name = "PITLB",      .typ = TYP_CMD,     .tid = CMD_PTLB_I                   },
    { .name = "PDTLB",      .typ = TYP_CMD,     .tid = CMD_PTLB_D                   },
    
    { .name = "PICA",       .typ = TYP_CMD,     .tid = CMD_PCA_I                    },
    { .name = "PDCA",       .typ = TYP_CMD,     .tid = CMD_PCA_D                    },
    { .name = "FICA",       .typ = TYP_CMD,     .tid = CMD_FCA_I                    },
    { .name = "FDCA",       .typ = TYP_CMD,     .tid = CMD_FCA_D                    },
    
    //------------------------------------------------------------------------------------
    // Window command tokens.
    //
    //------------------------------------------------------------------------------------
    { .name = "WON",        .typ = TYP_WCMD,    .tid = CMD_WON                      },
    { .name = "WOFF",       .typ = TYP_WCMD,    .tid = CMD_WOFF                     },
    { .name = "WDEF",       .typ = TYP_WCMD,    .tid = CMD_WDEF                     },
    { .name = "WSE",        .typ = TYP_WCMD,    .tid = CMD_WSE                      },
    { .name = "WSD",        .typ = TYP_WCMD,    .tid = CMD_WSD                      },  
    { .name = "CWL",        .typ = TYP_WCMD,    .tid = CMD_CWL                      },
    { .name = "CWC",        .typ = TYP_WCMD,    .tid = CMD_CWC                      },
    
    { .name = "WE",         .typ = TYP_WCMD,    .tid = CMD_WE                       },
    { .name = "WD",         .typ = TYP_WCMD,    .tid = CMD_WD                       },
    { .name = "WR",         .typ = TYP_WCMD,    .tid = CMD_WR                       },
    { .name = "WF",         .typ = TYP_WCMD,    .tid = CMD_WF                       },
    { .name = "WB",         .typ = TYP_WCMD,    .tid = CMD_WB                       },
    { .name = "WH",         .typ = TYP_WCMD,    .tid = CMD_WH                       },
    { .name = "WJ",         .typ = TYP_WCMD,    .tid = CMD_WJ                       },
    { .name = "WL",         .typ = TYP_WCMD,    .tid = CMD_WL                       },
    { .name = "WN",         .typ = TYP_WCMD,    .tid = CMD_WN                       },
    { .name = "WK",         .typ = TYP_WCMD,    .tid = CMD_WK                       },
    { .name = "WC",         .typ = TYP_WCMD,    .tid = CMD_WC                       },
    { .name = "WS",         .typ = TYP_WCMD,    .tid = CMD_WS                       },
    { .name = "WT",         .typ = TYP_WCMD,    .tid = CMD_WT                       },
    { .name = "WX",         .typ = TYP_WCMD,    .tid = CMD_WX                       },
    
    //------------------------------------------------------------------------------------
    // General registers.
    //
    //------------------------------------------------------------------------------------
    { .name = "R0",         .typ = TYP_GREG,    .tid = GR_0,     .u = { .val =  0  }},
    { .name = "R1",         .typ = TYP_GREG,    .tid = GR_1,     .u = { .val =  1  }},
    { .name = "R2",         .typ = TYP_GREG,    .tid = GR_2,     .u = { .val =  2  }},
    { .name = "R3",         .typ = TYP_GREG,    .tid = GR_3,     .u = { .val =  3  }},
    { .name = "R4",         .typ = TYP_GREG,    .tid = GR_4,     .u = { .val =  4  }},
    { .name = "R5",         .typ = TYP_GREG,    .tid = GR_5,     .u = { .val =  5  }},
    { .name = "R6",         .typ = TYP_GREG,    .tid = GR_6,     .u = { .val =  6  }},
    { .name = "R7",         .typ = TYP_GREG,    .tid = GR_7,     .u = { .val =  7  }},
    { .name = "R8",         .typ = TYP_GREG,    .tid = GR_8,     .u = { .val =  8  }},
    { .name = "R9",         .typ = TYP_GREG,    .tid = GR_9,     .u = { .val =  9  }},
    { .name = "R10",        .typ = TYP_GREG,    .tid = GR_10,    .u = { .val =  10 }},
    { .name = "R11",        .typ = TYP_GREG,    .tid = GR_11,    .u = { .val =  11 }},
    { .name = "R12",        .typ = TYP_GREG,    .tid = GR_12,    .u = { .val =  12 }},
    { .name = "R13",        .typ = TYP_GREG,    .tid = GR_13,    .u = { .val =  13 }},
    { .name = "R14",        .typ = TYP_GREG,    .tid = GR_14,    .u = { .val =  14 }},
    { .name = "R15",        .typ = TYP_GREG,    .tid = GR_15,    .u = { .val =  15 }},
    { .name = "GR",         .typ = TYP_GREG,    .tid = GR_SET,   .u = { .val =  0  }},

    //------------------------------------------------------------------------------------
    // Runtime architecture register names for general registers.
    //
    //------------------------------------------------------------------------------------
    { .name = "T0",         .typ = TYP_GREG,    .tid = GR_1,     .u = { .val =  1  }},
    { .name = "T1",         .typ = TYP_GREG,    .tid = GR_2,     .u = { .val =  2  }},
    { .name = "T2",         .typ = TYP_GREG,    .tid = GR_3,     .u = { .val =  3  }},
    { .name = "T3",         .typ = TYP_GREG,    .tid = GR_4,     .u = { .val =  4  }},
    { .name = "T4",         .typ = TYP_GREG,    .tid = GR_5,     .u = { .val =  5  }},
    { .name = "T5",         .typ = TYP_GREG,    .tid = GR_6,     .u = { .val =  6  }},
    { .name = "T6",         .typ = TYP_GREG,    .tid = GR_7,     .u = { .val =  7  }},

    { .name = "ARG3",       .typ = TYP_GREG,    .tid = GR_8,     .u = { .val =  8  }},
    { .name = "ARG2",       .typ = TYP_GREG,    .tid = GR_9,     .u = { .val =  9  }},
    { .name = "ARG1",       .typ = TYP_GREG,    .tid = GR_10,    .u = { .val =  10 }},
    { .name = "ARG0",       .typ = TYP_GREG,    .tid = GR_11,    .u = { .val =  11 }},

    { .name = "RET3",       .typ = TYP_GREG,    .tid = GR_8,     .u = { .val =  8  }},
    { .name = "RET2",       .typ = TYP_GREG,    .tid = GR_9,     .u = { .val =  9  }},
    { .name = "RET1",       .typ = TYP_GREG,    .tid = GR_10,    .u = { .val =  10 }},
    { .name = "RET0",       .typ = TYP_GREG,    .tid = GR_11,    .u = { .val =  11 }},
    
    { .name = "DP",         .typ = TYP_GREG,    .tid = GR_13,    .u = { .val =  13 }},
    { .name = "RL",         .typ = TYP_GREG,    .tid = GR_14,    .u = { .val =  14 }},
    { .name = "SP",         .typ = TYP_GREG,    .tid = GR_15,    .u = { .val =  15 }},
    
    //------------------------------------------------------------------------------------
    // Control registers.
    //
    //------------------------------------------------------------------------------------
    { .name = "C0",         .typ = TYP_CREG,    .tid = CR_0,     .u = { .val =  0  }},
    { .name = "C1",         .typ = TYP_CREG,    .tid = CR_1,     .u = { .val =  1  }},
    { .name = "C2",         .typ = TYP_CREG,    .tid = CR_2,     .u = { .val =  2  }},
    { .name = "C3",         .typ = TYP_CREG,    .tid = CR_3,     .u = { .val =  3  }},
    { .name = "C4",         .typ = TYP_CREG,    .tid = CR_4,     .u = { .val =  4  }},
    { .name = "C5",         .typ = TYP_CREG,    .tid = CR_5,     .u = { .val =  5  }},
    { .name = "C6",         .typ = TYP_CREG,    .tid = CR_6,     .u = { .val =  6  }},
    { .name = "C7",         .typ = TYP_CREG,    .tid = CR_7,     .u = { .val =  7  }},
    { .name = "C8",         .typ = TYP_CREG,    .tid = CR_8,     .u = { .val =  8  }},
    { .name = "C9",         .typ = TYP_CREG,    .tid = CR_9,     .u = { .val =  9  }},
    { .name = "C10",        .typ = TYP_CREG,    .tid = CR_10,    .u = { .val =  10 }},
    { .name = "C11",        .typ = TYP_CREG,    .tid = CR_11,    .u = { .val =  11 }},
    { .name = "C12",        .typ = TYP_CREG,    .tid = CR_12,    .u = { .val =  12 }},
    { .name = "C13",        .typ = TYP_CREG,    .tid = CR_13,    .u = { .val =  13 }},
    { .name = "C14",        .typ = TYP_CREG,    .tid = CR_14,    .u = { .val =  14 }},
    { .name = "C15",        .typ = TYP_CREG,    .tid = CR_15,    .u = { .val =  15 }},

    //------------------------------------------------------------------------------------
    // Program State Register
    //
    //------------------------------------------------------------------------------------
    { .name = "IA",         .typ = TYP_PREG,    .tid = PR_IA,    .u = { .val =  1  }},
    { .name = "ST",         .typ = TYP_PREG,    .tid = PR_ST,    .u = { .val =  2  }},
    
    //------------------------------------------------------------------------------------
    // Predefined functions.
    //
    //------------------------------------------------------------------------------------
    { .name = "ASM",        .typ = TYP_P_FUNC, .tid = PF_ASSEMBLE, .u = { .val = 0 }},
    { .name = "DISASM",     .typ = TYP_P_FUNC, .tid = PF_DIS_ASM,  .u = { .val = 0 }},
    { .name = "HASH",       .typ = TYP_P_FUNC, .tid = PF_HASH,     .u = { .val = 0 }},

    // ??? tons of more predefines....

    { .name = "S32",        .typ = TYP_P_FUNC, .tid = PF_S32,      .u = { .val = 0 }},

    //------------------------------------------------------------------------------------
    // TLB and Cache configuration types.
    //
    //------------------------------------------------------------------------------------
    { .name = "TLB_FA_64S",                 .typ = TYP_SYM, 
      .tid = TOK_TLB_FA_64S,                .u = { .val = 0 }},   

    { .name = "TLB_FA_128S",                .typ = TYP_SYM, 
      .tid = TOK_TLB_FA_128S,               .u = { .val = 0 }},

    { .name = "CACHE_SA_2W_128S_4L",        .typ = TYP_SYM, 
      .tid = TOK_CACHE_SA_2W_128S_4L,       .u = { .val = 0 }},

    { .name = "CACHE_SA_4W_128S_4L",        .typ = TYP_SYM, 
      .tid = TOK_CACHE_SA_2W_128S_4L,       .u = { .val = 0 }},

    { .name = "CACHE_SA_8W_128S_4L",        .typ = TYP_SYM, 
      .tid = TOK_CACHE_SA_2W_128S_4L,       .u = { .val = 0 }},

    { .name = "CACHE_SA_2W_64S_8L",         .typ = TYP_SYM, 
      .tid = TOK_CACHE_SA_2W_64S_8L,        .u = { .val = 0 }},

    { .name = "CACHE_SA_4W_64S_8L",         .typ = TYP_SYM, 
      .tid = TOK_CACHE_SA_2W_64S_8L,        .u = { .val = 0 }},  
    
    { .name = "CACHE_SA_8W_64S_8L",         .typ = TYP_SYM, 
      .tid = TOK_CACHE_SA_2W_64S_8L,        .u = { .val = 0 }},

    { .name = "R_ONLY",                     .typ = TYP_SYM, 
      .tid = TOK_MEM_READ_ONLY,             .u = { .val = 0 }},

    { .name = "R_W",                        .typ = TYP_SYM, 
      .tid = TOK_MEM_READ_WRITE,            .u = { .val = 0 }},

    { .name = "SPA_ADR",                    .typ = TYP_SYM, 
      .tid = TOK_MOD_SPA_ADR,               .u = { .val = 0 }},

    { .name = "SPA_LEN",            .typ = TYP_SYM, 
      .tid = TOK_MOD_SPA_LEN,               .u = { .val = 0 }}

};

const int MAX_CMD_TOKEN_TAB = sizeof( cmdTokTab ) / sizeof( SimToken );

//----------------------------------------------------------------------------------------
// The error message table. Each entry has the error number and the 
// corresponding error message text.
//
// ??? sort the entries... remove what is not needed anymore...
//----------------------------------------------------------------------------------------
const SimErrMsgTabEntry errMsgTab [ ] = {
    
    { .errNum = NO_ERR,                         
      .errStr = (char *) "NO_ERR" },

    { .errNum = ERR_NOT_SUPPORTED,              
      .errStr = (char *) "Command or Function not supported (yet)" },
    
    { .errNum = ERR_INVALID_CMD,                
      .errStr = (char *) "Invalid command, use help" },

    { .errNum = ERR_NUMERIC_OVERFLOW,                
      .errStr = (char *) "Numeric overflow in expression" },
    
    { .errNum = ERR_INVALID_CHAR_IN_TOKEN_LINE, 
      .errStr = (char *) "Invalid char in input line" },
    
    { .errNum = ERR_INVALID_ARG,                
      .errStr = (char *) "Invalid argument for command" },

    { .errNum = ERR_INVALID_WIN_ID,             
      .errStr = (char *) "Invalid window Id" },

    { .errNum = ERR_INVALID_REG_ID,             
      .errStr = (char *) "Invalid register Id" },

    { .errNum = ERR_INVALID_RADIX,              
      .errStr = (char *) "Invalid radix" },

    { .errNum = ERR_INVALID_EXIT_VAL,           
      .errStr = (char *) "Invalid program exit code" },

    { .errNum = ERR_INVALID_WIN_STACK_ID,       
      .errStr = (char *) "Invalid window stack Id" },

    { .errNum = ERR_INVALID_EXPR,               
      .errStr = (char *) "Invalid expression" },

    { .errNum = ERR_INVALID_NUM,
      .errStr = (char *) "Invalid number" },

    { .errNum = ERR_INVALID_FMT_OPT,            
      .errStr = (char *) "Invalid format option" },

    { .errNum = ERR_INVALID_WIN_TYPE,           
      .errStr = (char *) "Invalid window type" },

    { .errNum = ERR_INVALID_MODULE_TYPE,           
      .errStr = (char *) "Invalid module type" },

    { .errNum = ERR_EXPECTED_INSTR_VAL,         
      .errStr = (char *) "Expected the instruction value" },

    { .errNum = ERR_EXPECTED_FILE_NAME,         
      .errStr = (char *) "Expected a file name" },

    { .errNum = ERR_EXPECTED_STACK_ID,          
      .errStr = (char *) "Expected stack Id" },

    { .errNum = ERR_EXPECTED_WIN_ID,            
      .errStr = (char *) "Expected a window Id" },

    { .errNum = ERR_EXPECTED_LPAREN,            
      .errStr = (char *) "Expected a left paren" },

    { .errNum = ERR_EXPECTED_RPAREN,            
      .errStr = (char *) "Expected a right paren" },

    { .errNum = ERR_EXPECTED_COMMA,             
      .errStr = (char *) "Expected a comma" },

    { .errNum = ERR_EXPECTED_STR,               
      .errStr = (char *) "Expected a string value" },

    { .errNum = ERR_EXPECTED_REG_SET,           
      .errStr = (char *) "Expected a register set" },

    { .errNum = ERR_EXPECTED_REG_OR_SET,        
      .errStr = (char *) "Expected a register or register set" },

    { .errNum = ERR_EXPECTED_NUMERIC,           
      .errStr = (char *) "Expected a numeric value" },

    { .errNum = ERR_EXPECTED_EXT_ADR,           
      .errStr = (char *) "Expected a virtual address" },

    { .errNum = ERR_EXPECTED_GENERAL_REG,       
      .errStr = (char *) "Expected a general reg" },

    { .errNum = ERR_EXPECTED_STEPS,             
      .errStr = (char *) "Expected number of steps/instr" },

    { .errNum = ERR_EXPECTED_START_OFS,         
      .errStr = (char *) "Expected start offset" },
        
    { .errNum = ERR_EXPECTED_LEN,               
      .errStr = (char *) "Expected length argument" },

    { .errNum = ERR_EXPECTED_OFS,               
      .errStr = (char *) "Expected an address" },

    { .errNum = ERR_EXPECTED_INSTR_OPT,         
      .errStr = (char *) "Expected the instruction options" },

    { .errNum = ERR_EXPECTED_MOD_NUM,         
      .errStr = (char *) "Expected a module number" },
      
    { .errNum = ERR_EXPECTED_AN_OFFSET_VAL,     
      .errStr = (char *) "Expected an offset value" },

    { .errNum = ERR_EXPECTED_FMT_OPT,           
      .errStr = (char *) "Expected a format option" },

    { .errNum = ERR_EXPECTED_WIN_TYPE,          
      .errStr = (char *) "Expected a window type" },

    { .errNum = ERR_EXPECTED_EXPR,              
      .errStr = (char *) "Expected an expression" },

    { .errNum = ERR_FILE_NOT_FOUND,             
      .errStr = (char *) "File not found" },
   
    { .errNum = ERR_UNEXPECTED_EOS,             
      .errStr = (char *) "Unexpected end of command line" },

    { .errNum = ERR_NOT_IN_WIN_MODE,            
      .errStr = (char *) "Command only valid in Windows mode" },

    { .errNum = ERR_OPEN_EXEC_FILE,             
      .errStr = (char *) "Error while opening file" },

    { .errNum = ERR_EXTRA_TOKEN_IN_STR,         
      .errStr = (char *) "Extra tokens in command line" },

    { .errNum = ERR_ENV_VALUE_EXPR,             
      .errStr = (char *) "Invalid expression for ENV variable" },

    { .errNum = ERR_ENV_VAR_NOT_FOUND,          
      .errStr = (char *) "ENV variable not found" },

    { .errNum = ERR_WIN_TYPE_NOT_CONFIGURED,    
      .errStr = (char *) "Win object type not configured" },

    { .errNum = ERR_EXPR_TYPE_MATCH,            
      .errStr = (char *) "Expression type mismatch" },

    { .errNum = ERR_EXPR_FACTOR,                
      .errStr = (char *) "Expression error: factor" },

    { .errNum = ERR_TOO_MANY_ARGS_CMD_LINE,     
      .errStr = (char *) "Too many args in command line" },

    { .errNum = ERR_CMD_LINE_TOO_LONG,    
      .errStr = (char *) "Command line input too long" },

    { .errNum = ERR_OFS_LEN_LIMIT_EXCEEDED,     
      .errStr = (char *) "Offset/Length exceeds limit" },

    { .errNum = ERR_UNDEFINED_PFUNC,            
      .errStr = (char *) "Unknown predefined function" },

    { .errNum = ERR_IN_ASM_PFUNC,            
      .errStr = (char *) "Error in ASM function" },

    { .errNum = ERR_IN_DISASM_PFUNC,            
    .errStr = (char *) "Error in DISASM function" },
    
    { .errNum = ERR_ENV_PREDEFINED,             
      .errStr = (char *) "ENV variable is predefined" },

    { .errNum = ERR_ENV_TABLE_FULL,             
      .errStr = (char *) "ENV Table is full" },

    { .errNum = ERR_INSTR_HAS_NO_OPT,           
      .errStr = (char *) "Instruction has no option" },

    { .errNum = ERR_IMM_VAL_RANGE,              
      .errStr = (char *) "Immediate value out of range" },

    { .errNum = ERR_POS_VAL_RANGE,              
      .errStr = (char *) "Bit position value out of range" },

    { .errNum = ERR_LEN_VAL_RANGE,              
      .errStr = (char *) "Bit field length value out of range" },

    { .errNum = ERR_OFFSET_VAL_RANGE,           
      .errStr = (char *) "Offset value out of range" },

    { .errNum = ERR_OUT_OF_WINDOWS,             
      .errStr = (char *) "Cannot create more windows" },
        
    { .errNum = ERR_TLB_TYPE,                   
      .errStr = (char *) "Expected a TLB type" },

    { .errNum = ERR_TLB_INSERT_OP,              
      .errStr = (char *) "Insert in TLB operation error" },

    { .errNum = ERR_TLB_PURGE_OP,               
      .errStr = (char *) "Purge from TLB operation error" },

    { .errNum = ERR_TLB_ACC_DATA,               
      .errStr = (char *) "Invalid TLB insert access data" },

    { .errNum = ERR_TLB_ADR_DATA,               
      .errStr = (char *) "Invalid TLB insert address data" },

    { .errNum = ERR_TLB_NOT_CONFIGURED,         
      .errStr = (char *) "TLB type not configured" },

    { .errNum = ERR_TLB_SIZE_EXCEEDED,          
      .errStr = (char *) "TLB size exceeded" },
        
    { .errNum = ERR_CACHE_TYPE,                 
      .errStr = (char *) "Expected a cache type" },

    { .errNum = ERR_CACHE_FLUSH_OP,             
      .errStr = (char *) "Flush from cache operation error" },

    { .errNum = ERR_CACHE_PURGE_OP,             
      .errStr = (char *) "Purge from cache operation error" },

    { .errNum = ERR_CACHE_NOT_CONFIGURED,       
      .errStr = (char *) "Cache type not configured" },

    { .errNum = ERR_CACHE_SIZE_EXCEEDED,        
      .errStr = (char *) "Cache size exceeded" },

    { .errNum = ERR_CACHE_SET_NUM,              
      .errStr = (char *) "Invalid cache set" },

    { .errNum = ERR_MEM_OP_FAILED,              
      .errStr = (char *) "Memory operation error" },  

    { .errNum = ERR_CREATE_PROC_MODULE,              
      .errStr = (char *) "Create processor module error" }, 

    { .errNum = ERR_CREATE_MEM_MODULE,              
      .errStr = (char *) "Create memory module error" }
   
};

const int MAX_ERR_MSG_TAB = sizeof( errMsgTab ) / sizeof( SimErrMsgTabEntry );

//----------------------------------------------------------------------------------------
// Help message text table. Each entry has a type field, a token field, a command syntax 
// field and are explanation field.
//
//----------------------------------------------------------------------------------------
const SimHelpMsgEntry cmdHelpTab[ ] = {
    
    //------------------------------------------------------------------------------------
    // Commands.
    //
    //------------------------------------------------------------------------------------
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_HELP,
        .cmdNameStr     = (char *) "help",
        .cmdSyntaxStr   = (char *) "help ( cmdId | ‘commands‘ | "
                                   "'wcommands‘ | ‘predefined‘ )",
        .helpStr        = (char *) "list help info"
    },
  
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_EXIT,
        .cmdNameStr     = (char *) "exit",
        .cmdSyntaxStr   = (char *) "exit (e) [ <val> ]",
        .helpStr        = (char *) "program exit"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_HIST,
        .cmdNameStr     = (char *) "hist",
        .cmdSyntaxStr   = (char *) "hist [ depth ]",
        .helpStr        = (char *) "command history"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DO,
        .cmdNameStr     = (char *) "do",
        .cmdSyntaxStr   = (char *) "do [ cmdNum ]",
        .helpStr        = (char *) "re-execute command"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_REDO,
        .cmdNameStr     = (char *) "redo",
        .cmdSyntaxStr   = (char *) "redo [ cmdNum ]",
        .helpStr        = (char *) "edit and then re-execute command"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_ENV,
        .cmdNameStr     = (char *) "env",
        .cmdSyntaxStr   = (char *) "env [ <var> [ <val> ]] | env <var>",
        .helpStr        = (char *) "lists the env tab, a variable, sets a variable"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_XF,
        .cmdNameStr     = (char *) "xf",
        .cmdSyntaxStr   = (char *) "xf \"<filePath>\"",
        .helpStr        = (char *) "execute commands from a file"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_LF,
        .cmdNameStr     = (char *) "lf",
        .cmdSyntaxStr   = (char *) "lf \"<filePath>\"",
        .helpStr        = (char *) "loads an ELF file"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RESET,
        .cmdNameStr     = (char *) "reset",
        .cmdSyntaxStr   = (char *) "reset ( 'SYS' | 'STATS' )",
        .helpStr        = (char *) "resets the system, etc."
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RUN,
        .cmdNameStr     = (char *) "run",
        .cmdSyntaxStr   = (char *) "run",
        .helpStr        = (char *) "run the system ( all CPUs )"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_STEP,
        .cmdNameStr     = (char *) "step",
        .cmdSyntaxStr   = (char *) "s [ <steps> ]",
        .helpStr        = (char *) "single step the system"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_WRITE_LINE,
        .cmdNameStr     = (char *) "w",
        .cmdSyntaxStr   = (char *) "w <expr> [ , <rdx> ]",
        .helpStr        = (char *) "evaluates and prints an expression"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_NM,
        .cmdNameStr     = (char *) "nm",
        .cmdSyntaxStr   = (char *) "nm <mType> , [ <key> = <val> { , <key> = <val> } ]",
        .helpStr        = (char *) "adds a module to the system"
    },

     {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RM,
        .cmdNameStr     = (char *) "rm",
        .cmdSyntaxStr   = (char *) "rm <mNum>",
        .helpStr        = (char *) "removes a module from the system"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DM,
        .cmdNameStr     = (char *) "dm",
        .cmdSyntaxStr   = (char *) "dm [ <mNum>]",
        .helpStr        = (char *) "display module info"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DW,
        .cmdNameStr     = (char *) "dw",
        .cmdSyntaxStr   = (char *) "dw [ <sNum>]",
        .helpStr        = (char *) "display window list info"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MR,
        .cmdNameStr     = (char *) "mr",
        .cmdSyntaxStr   = (char *) "mr <reg> <val>",
        .helpStr        = (char *) "modify a CPU register"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DA,
        .cmdNameStr     = (char *) "da",
        .cmdSyntaxStr   = (char *) "da <adr> [ , <len> ] [ , <fmt> ]",
        .helpStr        = (char *) "display absolute memory"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MA,
        .cmdNameStr     = (char *) "ma",
        .cmdSyntaxStr   = (char *) "ma <adr> <val>",
        .helpStr        = (char *) "modify absolute memory"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_PCA_I,
        .cmdNameStr     = (char *) "pica",
        .cmdSyntaxStr   = (char *) "pica <vAdr>",
        .helpStr        = (char *) "purges instruction cache line data"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_PCA_D,
        .cmdNameStr     = (char *) "pdca",
        .cmdSyntaxStr   = (char *) "pdca <vAdr>",
        .helpStr        = (char *) "purges data cache line data"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_FCA_D,
        .cmdNameStr     = (char *) "fdca",
        .cmdSyntaxStr   = (char *) "fdca <vAdr>",
        .helpStr        = (char *) "flushes data cache line data"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_ITLB_I,
        .cmdNameStr     = (char *) "iitlb",
        .cmdSyntaxStr   = (char *) "iitlb <vAdr> , <pAdr> , <size> , "
                                   "<acc> , [ , L [ , U ]]",
        .helpStr        = (char *) "inserts an entry into the instruction TLB"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_ITLB_D,
        .cmdNameStr     = (char *) "idtlb",
        .cmdSyntaxStr   = (char *) "idtlb <vAdr> , <pAdr> , <size> , "
                                   "<acc> , [ , L [ , U ]]",
        .helpStr        = (char *) "inserts an entry into the data TLB"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_PTLB_I,
        .cmdNameStr     = (char *) "pitlb",
        .cmdSyntaxStr   = (char *) "pitlb <vAdr>",
        .helpStr        = (char *) "purges an entry from the instruction TLB"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_PTLB_D,
        .cmdNameStr     = (char *) "pdtlb",
        .cmdSyntaxStr   = (char *) "pdtlb <vAdr>",
        .helpStr        = (char *) "purges an entry from the data TLB"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_WON,
        .cmdNameStr     = (char *) "won",
        .cmdSyntaxStr   = (char *) "won",
        .helpStr        = (char *) "enables windows mode / redraw"
    },

    //------------------------------------------------------------------------------------
    // Window commands.
    //
    //------------------------------------------------------------------------------------
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_CWC,
        .cmdNameStr     = (char *)  "cwc",
        .cmdSyntaxStr   = (char *)  "cwc",
        .helpStr        = (char *)  "clears the command window"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_CWL,
        .cmdNameStr     = (char *)  "cwl",
        .cmdSyntaxStr   = (char *)  "cwl <lines>",
        .helpStr        = (char *)  "set command window lines"
    },

    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WOFF,
        .cmdNameStr     = (char *) "woff",
        .cmdSyntaxStr   = (char *) "woff",
        .helpStr        = (char *) "disables windows mode"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WDEF,
        .cmdNameStr     = (char *) "wdef",
        .cmdSyntaxStr   = (char *) "wdef [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        = (char *) "reset the windows to their default values"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WB,
        .cmdNameStr     = (char *)  "wb",
        .cmdSyntaxStr   = (char *)  "wb [ <amt> ] [ , <winNum> ]",
        .helpStr        = (char *)  "move backward by n items"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WC,
        .cmdNameStr     = (char *)  "wc",
        .cmdSyntaxStr   = (char *)  "wc <winNum>",
        .helpStr        = (char *)  "set the window as current window"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WE,
        .cmdNameStr     = (char *)  "we",
        .cmdSyntaxStr   = (char *)  "we [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        = (char *)  "enable window"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WD,
        .cmdNameStr     = (char *)  "wd",
        .cmdSyntaxStr   = (char *)  "wd [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        = (char *)  "disable window"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WF,
        .cmdNameStr     = (char *)  "wf",
        .cmdSyntaxStr   = (char *)  "wf [ <amt> ] [ , <winNum> ]",
        .helpStr        = (char *)  "move forward by n items"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WH,
        .cmdNameStr     = (char *)  "wh",
        .cmdSyntaxStr   = (char *)  "wh [ <itemAdr> ] [ , <winNum> ]",
        .helpStr        = (char *)  "set window home position"
    },

      {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WJ,
        .cmdNameStr     = (char *)  "wj",
        .cmdSyntaxStr   = (char *)  "wj <itemAdr> [ , <winNum> ]",
        .helpStr        = (char *)  "set window start to new position"
    },

     {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WK,
        .cmdNameStr     = (char *)  "wk",
        .cmdSyntaxStr   = (char *)  "wk [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        = (char *)  "remove a range of windows"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WL,
        .cmdNameStr     = (char *)  "wl",
        .cmdSyntaxStr   = (char *)  "wl <lines> [ , <winNum> ]",
        .helpStr        = (char *)  "set window lines including banner line"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WN,
        .cmdNameStr     = (char *)  "wn",
        .cmdSyntaxStr   = (char *)  "wn <type> [ , <arg1> [ , <arg2> ]]",
        .helpStr        = (char *)  "create a new window " 
                                    "( CPU, ITLB, DTLB, ICACHE, DCACHE, MEM, CODE, TEXT )"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WR,
        .cmdNameStr     = (char *)  "wr",
        .cmdSyntaxStr   = (char *)  "wr [ <rdx> [ , <winNum> ]]",
        .helpStr        = (char *)  "set window radix"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WS,
        .cmdNameStr     = (char *)  "ws",
        .cmdSyntaxStr   = (char *)  "ws <stackNum> [ , <start> ] [ , <end>]",
        .helpStr        = (char *)  "move a range of windows into stack <stackNum>"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WSE,
        .cmdNameStr     = (char *) "wse",
        .cmdSyntaxStr   = (char *) "wse <stackNum> | ALL",
        .helpStr        = (char *) "enable window stacks"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WSD,
        .cmdNameStr     = (char *) "wsd",
        .cmdSyntaxStr   = (char *) "wsd <stackNum> | ALL",
        .helpStr        = (char *) "disable window stacks"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WT,
        .cmdNameStr     = (char *)  "wt",
        .cmdSyntaxStr   = (char *)  "wt [ <winNum>, [ <toggleVal ]]",
        .helpStr        = (char *)  "toggle through alternate window content"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WX,
        .cmdNameStr     = (char *)  "wx",
        .cmdSyntaxStr   = (char *)  "wx <winNum>",
        .helpStr        = (char *)  "exchange current window with this window"
    },
    
    //------------------------------------------------------------------------------------
    // Predefined Functions.
    //
    //------------------------------------------------------------------------------------
    {
        .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_ASSEMBLE,
        .cmdNameStr     = (char *) "asm",
        .cmdSyntaxStr   = (char *) "asm ( <asmStr> )",
        .helpStr        = (char *) "returns the instruction value for an assemble string"
    },
    
    {
        .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_DIS_ASM,
        .cmdNameStr     = (char *) "disasm",
        .cmdSyntaxStr   = (char *) "disasm ( <instr> )",
        .helpStr        = (char *) "returns the assemble string for an instruction value"
    },

    // ??? a ton of functions for processor attributes ....
   
};

const int MAX_CMD_HELP_TAB = sizeof( cmdHelpTab ) / sizeof( SimHelpMsgEntry );
