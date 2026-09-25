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
// The simulator offers a set of command line options. Each option has a name, 
// an argument type and a value returned when the option is found.
//
//----------------------------------------------------------------------------------------
const SimCmdLineOptions optionTable[ ] = {

    { "help",       CL_OPT_NO_ARGUMENT,        CL_ARG_VAL_HELP },
    { "version",    CL_OPT_NO_ARGUMENT,        CL_ARG_VAL_VERSION },
    { "verbose",    CL_OPT_NO_ARGUMENT,        CL_ARG_VAL_VERBOSE },
    { "configfile", CL_OPT_REQUIRED_ARGUMENT,  CL_ARG_VAL_CONFIG_FILE },
    { "logfile",    CL_OPT_REQUIRED_ARGUMENT,  CL_ARG_VAL_LOG_FILE },
    { 0,            CL_OPT_NO_ARGUMENT,        0 }
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
    { .name = "TLB",        .typ = TYP_SYM,     .tid = TOK_TLB                      },
    { .name = "MEM",        .typ = TYP_SYM,     .tid = TOK_MEM                      },
    { .name = "IO",         .typ = TYP_SYM,     .tid = TOK_IO                       },
    { .name = "TEXT",       .typ = TYP_SYM,     .tid = TOK_TEXT                     },

    { .name = "&&",         .typ = TYP_SYM,     .tid = TOK_LAND                     },
    { .name = "||",         .typ = TYP_SYM,     .tid = TOK_LOR                      },
    { .name = "!",          .typ = TYP_SYM,     .tid = TOK_LNOT                     },

    { .name = "AND",         .typ = TYP_SYM,     .tid = TOK_LAND                     },
    { .name = "OR",          .typ = TYP_SYM,     .tid = TOK_LOR                      },
    { .name = "NOT",         .typ = TYP_SYM,     .tid = TOK_LNOT                     },

    { .name = "DEC",        .typ = TYP_SYM,     .tid = TOK_DEC,   .u = { .val = 10 }},
    { .name = "HEX",        .typ = TYP_SYM,     .tid = TOK_HEX,   .u = { .val = 16 }},
    { .name = "HEX32",      .typ = TYP_SYM,     .tid = TOK_HEX32,                   },
    { .name = "HEX64",      .typ = TYP_SYM,     .tid = TOK_HEX64,                   },
    { .name = "ASCII",      .typ = TYP_SYM,     .tid = TOK_ASCII,                   },
    { .name = "CODE",       .typ = TYP_SYM,     .tid = TOK_CODE                     },

    { .name = "BYTE",       .typ = TYP_SYM,     .tid = TOK_BYTE,   .u = { .val = 1  }},
    { .name = "UBYTE",      .typ = TYP_SYM,     .tid = TOK_UBYTE,  .u = { .val = 1  }},
    { .name = "SHORT",      .typ = TYP_SYM,     .tid = TOK_SHORT,  .u = { .val = 2  }},
    { .name = "USHORT",     .typ = TYP_SYM,     .tid = TOK_USHORT, .u = { .val = 2  }},
    { .name = "HALF",       .typ = TYP_SYM,     .tid = TOK_HALF,   .u = { .val = 2  }},
    { .name = "UHALF",      .typ = TYP_SYM,     .tid = TOK_UHALF,  .u = { .val = 2  }},
    { .name = "WORD",       .typ = TYP_SYM,     .tid = TOK_WORD,   .u = { .val = 4  }},
    { .name = "UWORD",      .typ = TYP_SYM,     .tid = TOK_UWORD,  .u = { .val = 4  }},
    { .name = "DWORD",      .typ = TYP_SYM,     .tid = TOK_DWORD,  .u = { .val = 8  }},
    { .name = "DOUBLE",     .typ = TYP_SYM,     .tid = TOK_DOUBLE, .u = { .val = 8  }},
   
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
    { .name = "LOADELF",    .typ = TYP_CMD,     .tid = CMD_LOADELF                  },
    { .name = "W",          .typ = TYP_CMD,     .tid = CMD_WRITE_LINE               },
    { .name = "DWIN",       .typ = TYP_CMD,     .tid = CMD_DWIN                     },
    { .name = "ECHO",       .typ = TYP_CMD,     .tid = CMD_ECHO                     },

    { .name = "DMOD",       .typ = TYP_CMD,     .tid = CMD_DMOD                     },
    { .name = "NMOD",       .typ = TYP_CMD,     .tid = CMD_NMOD                     },
    { .name = "RMOD",       .typ = TYP_CMD,     .tid = CMD_RMOD                     },
    
    { .name = "RESET",      .typ = TYP_CMD,     .tid = CMD_RESET                    },            
    { .name = "RUN",        .typ = TYP_CMD,     .tid = CMD_RUN                      },
    { .name = "STEP",       .typ = TYP_CMD,     .tid = CMD_STEP                     },
    { .name = "S",          .typ = TYP_CMD,     .tid = CMD_STEP                     },
    
    { .name = "MR",         .typ = TYP_CMD,     .tid = CMD_MR                       },
    { .name = "DM",         .typ = TYP_CMD,     .tid = CMD_DM                       },
    { .name = "MB",         .typ = TYP_CMD,     .tid = CMD_MB                       },
    { .name = "MS",         .typ = TYP_CMD,     .tid = CMD_MS                       },
    { .name = "MW",         .typ = TYP_CMD,     .tid = CMD_MW                       },
    { .name = "MD",         .typ = TYP_CMD,     .tid = CMD_MD                       },

    { .name = "ITLB",       .typ = TYP_CMD,     .tid = CMD_ITLB                     },
    { .name = "PTLB",       .typ = TYP_CMD,     .tid = CMD_PTLB                     },

    { .name = "ASSERT",     .typ = TYP_CMD,     .tid = CMD_ASSERT                   },
    { .name = "CHECK",      .typ = TYP_CMD,     .tid = CMD_CHECK                    },
    { .name = "LOG",        .typ = TYP_CMD,     .tid = CMD_LOG                      },

    { .name = "IF",         .typ = TYP_CMD,     .tid = CMD_IF                       },
    { .name = "ELSEIF",     .typ = TYP_CMD,     .tid = CMD_ELSEIF                   },
    { .name = "ELSE",       .typ = TYP_CMD,     .tid = CMD_ELSE                     },
    { .name = "ENDIF",      .typ = TYP_CMD,     .tid = CMD_ENDIF                    },

    { .name = "BL",         .typ = TYP_CMD,     .tid = CMD_BL                       },
    { .name = "BN",         .typ = TYP_CMD,     .tid = CMD_BN                       },
    { .name = "BK",         .typ = TYP_CMD,     .tid = CMD_BK                       },
    { .name = "BE",         .typ = TYP_CMD,     .tid = CMD_BE                       },
    { .name = "BD",         .typ = TYP_CMD,     .tid = CMD_BD                       },

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

    { .name = "CR0",        .typ = TYP_CREG,    .tid = CR_0,     .u = { .val =  0  }},
    { .name = "CR1",        .typ = TYP_CREG,    .tid = CR_1,     .u = { .val =  1  }},
    { .name = "CR2",        .typ = TYP_CREG,    .tid = CR_2,     .u = { .val =  2  }},
    { .name = "CR3",        .typ = TYP_CREG,    .tid = CR_3,     .u = { .val =  3  }},
    { .name = "CR4",        .typ = TYP_CREG,    .tid = CR_4,     .u = { .val =  4  }},
    { .name = "CR5",        .typ = TYP_CREG,    .tid = CR_5,     .u = { .val =  5  }},
    { .name = "CR6",        .typ = TYP_CREG,    .tid = CR_6,     .u = { .val =  6  }},
    { .name = "CR7",        .typ = TYP_CREG,    .tid = CR_7,     .u = { .val =  7  }},
    { .name = "CR8",        .typ = TYP_CREG,    .tid = CR_8,     .u = { .val =  8  }},
    { .name = "CR9",        .typ = TYP_CREG,    .tid = CR_9,     .u = { .val =  9  }},
    { .name = "CR10",       .typ = TYP_CREG,    .tid = CR_10,    .u = { .val =  10 }},
    { .name = "CR11",       .typ = TYP_CREG,    .tid = CR_11,    .u = { .val =  11 }},
    { .name = "CR12",       .typ = TYP_CREG,    .tid = CR_12,    .u = { .val =  12 }},
    { .name = "CR13",       .typ = TYP_CREG,    .tid = CR_13,    .u = { .val =  13 }},
    { .name = "CR14",       .typ = TYP_CREG,    .tid = CR_14,    .u = { .val =  14 }},
    { .name = "CR15",       .typ = TYP_CREG,    .tid = CR_15,    .u = { .val =  15 }},

    // ???  name the control registers... watch out for name clashes though.

    //------------------------------------------------------------------------------------
    // Program State Register
    //
    //------------------------------------------------------------------------------------
    { .name = "IA",         .typ = TYP_PREG,    .tid = PR_IA,    .u = { .val =  1  }},
    { .name = "ST",         .typ = TYP_PREG,    .tid = PR_ST,    .u = { .val =  2  }},
    { .name = "RL",         .typ = TYP_PREG,    .tid = GR_13,    .u = { .val =  13 }},
    { .name = "SP",         .typ = TYP_PREG,    .tid = GR_14,    .u = { .val =  14 }},
    { .name = "DP",         .typ = TYP_PREG,    .tid = GR_15,    .u = { .val =  15 }},
    { .name = "SAR",        .typ = TYP_CREG,    .tid = CR_2,     .u = { .val =  2 }},
    { .name = "IVA_ADR",    .typ = TYP_CREG,    .tid = CR_8,     .u = { .val =  8 }},

    //------------------------------------------------------------------------------------
    // Predefined functions.
    //
    //------------------------------------------------------------------------------------
    { .name = "ASM",        .typ = TYP_P_FUNC, .tid = PF_ASSEMBLE, .u = { .val = 0 }},
    { .name = "DISASM",     .typ = TYP_P_FUNC, .tid = PF_DIS_ASM,  .u = { .val = 0 }},
    { .name = "ADD_OFS",    .typ = TYP_P_FUNC, .tid = PF_ADD_OFS,  .u = { .val = 0 }},
    { .name = "REGION",     .typ = TYP_P_FUNC, .tid = PF_REGION,   .u = { .val = 0 }},
    { .name = "OFS",        .typ = TYP_P_FUNC, .tid = PF_OFS,      .u = { .val = 0 }},
    { .name = "PAGE",       .typ = TYP_P_FUNC, .tid = PF_PAGE,     .u = { .val = 0 }},

    //------------------------------------------------------------------------------------
    // Breakpoint tokens for data breakpoints.
    //
    //------------------------------------------------------------------------------------
    { .name = "DATA_R",                     .typ = TYP_SYM, 
      .tid = TOK_DATA_R,                    .u = { .val = 0 }},

    { .name = "DATA_W",                     .typ = TYP_SYM, 
      .tid = TOK_DATA_W,                    .u = { .val = 0 }},

    { .name = "DATA_RW",                    .typ = TYP_SYM, 
      .tid = TOK_DATA_R,                    .u = { .val = 0 }},

    { .name = "DATA",                       .typ = TYP_SYM, 
      .tid = TOK_DATA_RW,                   .u = { .val = 0 }},

    //------------------------------------------------------------------------------------
    // TLB and Cache configuration types.
    //
    //------------------------------------------------------------------------------------
    { .name = "TLB_FA_16S",                 .typ = TYP_SYM, 
      .tid = TOK_TLB_FA_16S,                .u = { .val = 0 }},

    { .name = "TLB_FA_32S",                 .typ = TYP_SYM, 
      .tid = TOK_TLB_FA_32S,                .u = { .val = 0 }},

    { .name = "TLB_FA_64S",                 .typ = TYP_SYM, 
      .tid = TOK_TLB_FA_64S,                .u = { .val = 0 }},   

    { .name = "TLB_FA_128S",                .typ = TYP_SYM, 
      .tid = TOK_TLB_FA_128S,               .u = { .val = 0 }},

    { .name = "ROM",                        .typ = TYP_SYM, 
      .tid = TOK_MEM_ROM,                   .u = { .val = 0 }},

    { .name = "RAM",                        .typ = TYP_SYM, 
      .tid = TOK_MEM_RAM,                   .u = { .val = 0 }},

    { .name = "SPA_ADR",                    .typ = TYP_SYM, 
      .tid = TOK_MOD_SPA_ADR,               .u = { .val = 0 }},

    { .name = "SPA_LEN",                    .typ = TYP_SYM, 
      .tid = TOK_MOD_SPA_LEN,               .u = { .val = 0 }},

    //------------------------------------------------------------------------------------
    // Constants.
    //
    // ??? we could add numeric constants such as MAX_INT, etc.
    //
    // ??? we could add some more architecture specific constants such as page
    // size, etc.
    //------------------------------------------------------------------------------------
    { .name = "TRUE",                        .typ = TYP_BOOL, 
      .tid  = TOK_TRUE,                      .u = { .val = 1 }},

    { .name = "FALSE",                        .typ = TYP_BOOL, 
      .tid  = TOK_FALSE,                       .u = { .val = 0 }},
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
      .errStr = "NO_ERR" },

    { .errNum = ERR_NOT_SUPPORTED,              
      .errStr = "Command or Function not supported (yet)" },
    
    { .errNum = ERR_INVALID_CMD,                
      .errStr = "Invalid command, use help" },

    { .errNum = ERR_NUMERIC_OVERFLOW,                
      .errStr = "Numeric overflow in expression" },
    
    { .errNum = ERR_INVALID_CHAR_IN_TOKEN_LINE, 
      .errStr = "Invalid char in input line" },
    
    { .errNum = ERR_INVALID_ARG,                
      .errStr = "Invalid argument for command" },

    { .errNum = ERR_INVALID_WIN_ID,             
      .errStr = "Invalid window Id" },

    { .errNum = ERR_INVALID_REG_ID,             
      .errStr = "Invalid register Id" },

    { .errNum = ERR_INVALID_RADIX,              
      .errStr = "Invalid radix" },

    { .errNum = ERR_INVALID_EXIT_VAL,           
      .errStr = "Invalid program exit code" },

    { .errNum = ERR_INVALID_WIN_STACK_ID,       
      .errStr = "Invalid window stack Id" },

    { .errNum = ERR_INVALID_EXPR,               
      .errStr = "Invalid expression" },

    { .errNum = ERR_INVALID_NUM,
      .errStr = "Invalid number" },

    { .errNum = ERR_NUMERIC_RANGE,
      .errStr = "Numeric overflow" },

    { .errNum = ERR_UNALIGNED_ADDR,             
      .errStr = "Unaligned address" },

    { .errNum = ERR_INVALID_ADDR,           
      .errStr = "Invalid address" },

    { .errNum = ERR_INVALID_FMT_OPT,            
      .errStr = "Invalid format option" },

    { .errNum = ERR_INVALID_TOGGLE_VAL,            
      .errStr = "Invalid toggle value" },

    { .errNum = ERR_INVALID_WIN_TYPE,           
      .errStr = "Invalid window type" },

    { .errNum = ERR_INVALID_MODULE_TYPE,           
      .errStr = "Invalid module type" },

    { .errNum = ERR_INVALID_MOD_NUM,           
      .errStr = "Invalid module number" },

    { .errNum = ERR_EXPECTED_INSTR_VAL,         
      .errStr = "Expected the instruction value" },

    { .errNum = ERR_EXPECTED_FILE_NAME,         
      .errStr = "Expected a file name" },

    { .errNum = ERR_EXPECTED_STACK_ID,          
      .errStr = "Expected stack Id" },

    { .errNum = ERR_EXPECTED_WIN_ID,            
      .errStr = "Expected a window Id" },

    { .errNum = ERR_EXPECTED_LPAREN,            
      .errStr = "Expected a left paren" },

    { .errNum = ERR_EXPECTED_RPAREN,            
      .errStr = "Expected a right paren" },

    { .errNum = ERR_EXPECTED_LBRACK,            
      .errStr = "Expected a left bracket" },

    { .errNum = ERR_EXPECTED_RBRACK,            
      .errStr = "Expected a right bracket" },

    { .errNum = ERR_EXPECTED_COMMA,             
      .errStr = "Expected a comma" },

    { .errNum = ERR_EXPECTED_STR,               
      .errStr = "Expected a string value" },

    { .errNum = ERR_EXPECTED_REG_SET,           
      .errStr = "Expected a register set" },

    { .errNum = ERR_EXPECTED_REG_OR_SET,        
      .errStr = "Expected a register or register set" },

    { .errNum = ERR_EXPECTED_NUM_VALUE,           
      .errStr = "Expected a numeric value" },

    { .errNum = ERR_EXPECTED_BOOL_VALUE,           
      .errStr = "Expected a boolean value" },

    { .errNum = ERR_EXPECTED_STRING_VALUE,           
      .errStr = "Expected a string value" },

    { .errNum = ERR_EXPECTED_BRK_INDEX,
      .errStr = "Expected a breakpoint Id" },

    { .errNum = ERR_EXPECTED_BRK_TYPE,
      .errStr = "Expected a breakpoint type" },

    { .errNum = ERR_EXPECTED_REL_OP,           
      .errStr = "Expected a relational operator" },

    { .errNum = ERR_EXPECTED_EXT_ADR,           
      .errStr = "Expected a virtual address" },

    { .errNum = ERR_EXPECTED_GENERAL_REG,       
      .errStr = "Expected a general reg" },

    { .errNum = ERR_EXPECTED_STEPS,             
      .errStr = "Expected number of steps/instr" },

    { .errNum = ERR_EXPECTED_START_OFS,         
      .errStr = "Expected start offset" },
        
    { .errNum = ERR_EXPECTED_LEN,               
      .errStr = "Expected length argument" },

    { .errNum = ERR_EXPECTED_OFS,               
      .errStr = "Expected an address" },

    { .errNum = ERR_EXPECTED_INSTR_OPT,         
      .errStr = "Expected the instruction options" },

    { .errNum = ERR_EXPECTED_MOD_NUM,         
      .errStr = "Expected a module number" },
      
    { .errNum = ERR_EXPECTED_AN_OFFSET_VAL,     
      .errStr = "Expected an offset value" },

    { .errNum = ERR_EXPECTED_FMT_OPT,           
      .errStr = "Expected a format option" },

    { .errNum = ERR_EXPECTED_WIN_TYPE,          
      .errStr = "Expected a window type" },

    { .errNum = ERR_EXPECTED_EXPR,              
      .errStr = "Expected an expression" },

    { .errNum = ERR_EXPCTED_PROC_MODULE,              
      .errStr = "Expected a processor module" },
      
    { .errNum = ERR_FILE_NOT_FOUND,             
      .errStr = "File not found" },
   
    { .errNum = ERR_UNEXPECTED_EOS,             
      .errStr = "Unexpected end of command line" },

    { .errNum = ERR_NOT_IN_WIN_MODE,            
      .errStr = "Command only valid in Windows mode" },

    { .errNum = ERR_NOT_INTERACTIVE,            
      .errStr = "Command only valid in interactive mode" },

    { .errNum = ERR_OPEN_EXEC_FILE,             
      .errStr = "Error while opening exec file" },

    { .errNum = ERR_OPEN_LOG_FILE,             
      .errStr = "Error while opening log file" },

    { .errNum = ERR_NO_LOG_FILE_CONFIGURED,             
      .errStr = "No log file configured" },

    { .errNum = ERR_EXTRA_TOKEN_IN_STR,         
      .errStr = "Extra tokens in command line" },

    { .errNum = ERR_ENV_VALUE_EXPR,             
      .errStr = "Invalid expression for ENV variable" },

    { .errNum = ERR_ENV_VAR_NOT_FOUND,          
      .errStr = "ENV variable not found" },

    { .errNum = ERR_WIN_TYPE_NOT_CONFIGURED,    
      .errStr = "Win object type not configured" },

    { .errNum = ERR_EXPR_TYPE_MATCH,            
      .errStr = "Expression type mismatch" },

    { .errNum = ERR_EXPR_FACTOR,                
      .errStr = "Expression error: factor" },

    { .errNum = ERR_INVALID_HEX_ESCAPE,                
      .errStr = "Invalid hex escape code for char" },

    { .errNum = ERR_INVALID_UNICODE_ESCAPE,                
      .errStr = "Invalid unicode escape code for char" },

    { .errNum = ERR_STRING_TOO_LONG,                
      .errStr = "String size exceeds limit" },

    { .errNum = ERR_TOO_MANY_ARGS_CMD_LINE,     
      .errStr = "Too many args in command line" },

    { .errNum = ERR_CMD_LINE_TOO_LONG,    
      .errStr = "Command line input too long" },

    { .errNum = ERR_OUT_OF_HIST_BOUNDS, 
      .errStr = "Command Id out of history buffer bounds" },

    { .errNum = ERR_OFS_LEN_LIMIT_EXCEEDED,     
      .errStr = "Offset/Length exceeds limit" },

    { .errNum = ERR_UNDEFINED_PFUNC,            
      .errStr = "Unknown predefined function" },

    { .errNum = ERR_IN_ASM_PFUNC,            
      .errStr = "Error in ASM function" },

    { .errNum = ERR_IN_DISASM_PFUNC,            
      .errStr = "Error in DISASM function" },
    
    { .errNum = ERR_ENV_PREDEFINED,             
      .errStr = "ENV variable is predefined" },

    { .errNum = ERR_ENV_TABLE_FULL,             
      .errStr = "ENV Table is full" },

    { .errNum = ERR_INSTR_HAS_NO_OPT,           
      .errStr = "Instruction has no option" },

    { .errNum = ERR_IMM_VAL_RANGE,              
      .errStr = "Immediate value out of range" },

    { .errNum = ERR_POS_VAL_RANGE,              
      .errStr = "Bit position value out of range" },

    { .errNum = ERR_LEN_VAL_RANGE,              
      .errStr = "Bit field length value out of range" },

    { .errNum = ERR_OFFSET_VAL_RANGE,           
      .errStr = "Offset value out of range" },

    { .errNum = ERR_OUT_OF_WINDOWS,             
      .errStr = "Cannot create more windows" },
        
    { .errNum = ERR_TLB_TYPE,                   
      .errStr = "Expected a TLB type" },

    { .errNum = ERR_TLB_INSERT_OP,              
      .errStr = "Insert in TLB operation error" },

    { .errNum = ERR_TLB_PURGE_OP,               
      .errStr = "Purge from TLB operation error" },

    { .errNum = ERR_TLB_ACC_DATA,               
      .errStr = "Invalid TLB insert access data" },

    { .errNum = ERR_TLB_ADR_DATA,               
      .errStr = "Invalid TLB insert address data" },

    { .errNum = ERR_TLB_NOT_CONFIGURED,         
      .errStr = "TLB type not configured" },

    { .errNum = ERR_TLB_SIZE_EXCEEDED,          
      .errStr = "TLB size exceeded" },

    { .errNum = ERR_MEM_OP_FAILED,              
      .errStr = "Memory operation error" },  

    { .errNum = ERR_RESET_MODULE,              
      .errStr = "Reset module error" }, 

    { .errNum = ERR_HALT_MODULE,              
      .errStr = "Halt module error" },

    { .errNum = ERR_STEP_MODULE,              
      .errStr = "Step module error" },

    { .errNum = ERR_MODULE_TABLE_FULL,              
      .errStr = "Module table full" },

    { .errNum = ERR_MODULE_RANGE_OVERLAP,              
      .errStr = "Module SPA range overlap" },

    { .errNum = ERR_MODULE_ALREADY_USED,              
      .errStr = "Module number already in use" },

    { .errNum = ERR_CREATE_PROC_MODULE,              
      .errStr = "Create processor module error" }, 

    { .errNum = ERR_CREATE_MEM_MODULE,              
      .errStr = "Create memory module error" },

    { .errNum = ERR_CREATE_BRK_FAILED,              
      .errStr = "Create simulator breakpoint failed" },

     { .errNum = ERR_REMOVE_BRK_FAILED,              
      .errStr = "Remove simulator breakpoint failed" },

    { .errNum = ERR_INVALID_ELF_FILE,              
      .errStr = "Error while open ELF file" },

    { .errNum = ERR_ELF_INVALID_ADR_RANGE,              
      .errStr = "ELF: invalid address range" },

    { .errNum = ERR_ELF_MEMORY_SIZE_EXCEEDED,              
      .errStr = "ELF: memory size exceeded" },

    { .errNum = ERR_INVALID_ELF_BYTE_ORDER,              
      .errStr = "EELF: invalid byte order" }
   
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
        .cmdNameStr     = "help",
        .cmdSyntaxStr   = "help ( cmdId | ‘commands‘ | "
                                   "'wcommands‘ | ‘predefined‘ )",
        .helpStr        = "list help info"
    },
  
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_EXIT,
        .cmdNameStr     = "exit",
        .cmdSyntaxStr   = "exit (e) [ <val> ]",
        .helpStr        = "program exit"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_HIST,
        .cmdNameStr     = "hist",
        .cmdSyntaxStr   = "hist [ depth ]",
        .helpStr        = "command history"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DO,
        .cmdNameStr     = "do",
        .cmdSyntaxStr   = "do [ cmdNum ]",
        .helpStr        = "re-execute command"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_REDO,
        .cmdNameStr     = "redo",
        .cmdSyntaxStr   = "redo [ cmdNum ]",
        .helpStr        = "edit and then re-execute command"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_ENV,
        .cmdNameStr     = "env",
        .cmdSyntaxStr   = "env [ <var> [ , <val> ]] | env <var>",
        .helpStr        = "lists the env tab, a variable, sets a variable"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_CHECK,
        .cmdNameStr     = "check",
        .cmdSyntaxStr   = "check <expr> [ , <str> ]",
        .helpStr        = "check a boolean condition"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_ASSERT,
        .cmdNameStr     = "assert",
        .cmdSyntaxStr   = "assert <expr> [ , <str> ]",
        .helpStr        = "assert a boolean condition"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_LOG,
        .cmdNameStr     = "log",
        .cmdSyntaxStr   = "log <str> [ , <expr> ]",
        .helpStr        = "log a message to the log file"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_XF,
        .cmdNameStr     = "xf",
        .cmdSyntaxStr   = "xf \"<filePath>\"",
        .helpStr        = "execute commands from a file"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_LOADELF,
        .cmdNameStr     = "loadelf",
        .cmdSyntaxStr   = "loadelf \"<filePath>\"",
        .helpStr        = "loads an ELF file"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RESET,
        .cmdNameStr     = "reset",
        .cmdSyntaxStr   = "reset ( <modNum> | 'ALL' )",
        .helpStr        = "reset module(s)"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RUN,
        .cmdNameStr     = "run",
        .cmdSyntaxStr   = "run",
        .helpStr        = "run the system ( all processors )"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_STEP,
        .cmdNameStr     = "step",
        .cmdSyntaxStr   = "s [ <steps> [ , <modNum> ]]",
        .helpStr        = "single step a module"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_BN,
        .cmdNameStr     = "bn",
        .cmdSyntaxStr   = "bn <mod> , <typ> , <adr> [ , <len> ]",
        .helpStr        = "Adds a simulator breakpoint"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_BK,
        .cmdNameStr     = "bk",
        .cmdSyntaxStr   = "bk <bNum> [ , <modNum> ]",
        .helpStr        = "Removes a simulator breakpoint"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_BE,
        .cmdNameStr     = "be",
        .cmdSyntaxStr   = "be <bNum>",
        .helpStr        = "Enables a simulator breakpoint"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_BD,
        .cmdNameStr     = "bd",
        .cmdSyntaxStr   = "bd <bNum>",
        .helpStr        = "Disables a simulator breakpoint"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_BL,
        .cmdNameStr     = "bl",
        .cmdSyntaxStr   = "bl",
        .helpStr        = "Displays the simulator breakpoints"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_WRITE_LINE,
        .cmdNameStr     = "w",
        .cmdSyntaxStr   = "w <expr> [ , <rdx> ]",
        .helpStr        = "evaluates and prints an expression"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_NMOD,
        .cmdNameStr     = "nmod",
        .cmdSyntaxStr   = "nmod <mType> , [ <key> = <val> { , <key> = <val> } ]",
        .helpStr        = "adds a module to the system"
    },

     {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RMOD,
        .cmdNameStr     = "rmod",
        .cmdSyntaxStr   = "rmod <mNum> | ALL",
        .helpStr        = "removes modules from the system"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DMOD,
        .cmdNameStr     = "dmod",
        .cmdSyntaxStr   = "dmod [ <mNum>]",
        .helpStr        = "display module info"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DWIN,
        .cmdNameStr     = "dwin",
        .cmdSyntaxStr   = "dwin [ <sNum>]",
        .helpStr        = "display window list info"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MR,
        .cmdNameStr     = "mr",
        .cmdSyntaxStr   = "mr <reg> , <val>",
        .helpStr        = "modify a CPU register"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DM,
        .cmdNameStr     = "dam",
        .cmdSyntaxStr   = "dm <adr> [ , <len> ] [ , <fmt> ]",
        .helpStr        = "display memory"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MB,
        .cmdNameStr     = "mb",
        .cmdSyntaxStr   = "mb <adr> , <val>",
        .helpStr        = "modify memory byte"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MS,
        .cmdNameStr     = "ms",
        .cmdSyntaxStr   = "ms <adr> , <val>",
        .helpStr        = "modify memory short"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MW,
        .cmdNameStr     = "mw",
        .cmdSyntaxStr   = "mw <adr> , <val>",
        .helpStr        = "modify memory word"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MD,
        .cmdNameStr     = "md",
        .cmdSyntaxStr   = "md <adr> , <val>",
        .helpStr        = "modify memory double"
    },
     
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_ITLB,
        .cmdNameStr     = "itlb",
        .cmdSyntaxStr   = "itlb <arg1> , <arg2>",
        .helpStr        = "insert into the global TLB"
    },

    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_PTLB,
        .cmdNameStr     = "ptlb",
        .cmdSyntaxStr   = "ptlb <vAdr>",
        .helpStr        = "purge from the global TLB"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_WON,
        .cmdNameStr     = "won",
        .cmdSyntaxStr   = "won",
        .helpStr        = "enables windows mode / redraw"
    },

    //------------------------------------------------------------------------------------
    // Window commands.
    //
    //------------------------------------------------------------------------------------
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_CWC,
        .cmdNameStr     =  "cwc",
        .cmdSyntaxStr   =  "cwc",
        .helpStr        =  "clears the command window"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_CWL,
        .cmdNameStr     =  "cwl",
        .cmdSyntaxStr   =  "cwl <lines>",
        .helpStr        =  "set command window lines"
    },

    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WOFF,
        .cmdNameStr     = "woff",
        .cmdSyntaxStr   = "woff",
        .helpStr        = "disables windows mode"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WDEF,
        .cmdNameStr     = "wdef",
        .cmdSyntaxStr   = "wdef [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        = "reset the windows to their default values"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WB,
        .cmdNameStr     =  "wb",
        .cmdSyntaxStr   =  "wb [ <amt> ] [ , <winNum> ]",
        .helpStr        =  "move backward by n items"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WC,
        .cmdNameStr     =  "wc",
        .cmdSyntaxStr   =  "wc <winNum>",
        .helpStr        =  "set the window as current window"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WE,
        .cmdNameStr     =  "we",
        .cmdSyntaxStr   =  "we [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        =  "enable window"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WD,
        .cmdNameStr     =  "wd",
        .cmdSyntaxStr   =  "wd [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        =  "disable window"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WF,
        .cmdNameStr     =  "wf",
        .cmdSyntaxStr   =  "wf [ <amt> ] [ , <winNum> ]",
        .helpStr        =  "move forward by n items"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WH,
        .cmdNameStr     =  "wh",
        .cmdSyntaxStr   =  "wh [ <itemAdr> ] [ , <winNum> ]",
        .helpStr        =  "set window home position"
    },

      {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WJ,
        .cmdNameStr     =  "wj",
        .cmdSyntaxStr   =  "wj <itemAdr> [ , <winNum> ]",
        .helpStr        =  "set window start to new position"
    },

     {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WK,
        .cmdNameStr     =  "wk",
        .cmdSyntaxStr   =  "wk [ <start> [ , <end> ]] | 'ALL'",
        .helpStr        =  "remove a range of windows"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WL,
        .cmdNameStr     =  "wl",
        .cmdSyntaxStr   =  "wl <lines> [ , <winNum> ]",
        .helpStr        =  "set window lines including banner line"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WN,
        .cmdNameStr     =  "wn",
        .cmdSyntaxStr   =  "wn <type> [ , <arg1> [ , <arg2> ]]",
        .helpStr        =  "create a new window " 
                                    "( PROC, TLB, MEM, TEXT )"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WR,
        .cmdNameStr     =  "wr",
        .cmdSyntaxStr   =  "wr [ <rdx> [ , <winNum> ]]",
        .helpStr        =  "set window radix"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WS,
        .cmdNameStr     =  "ws",
        .cmdSyntaxStr   =  "ws <stackNum> [ , <start> ] [ , <end>]",
        .helpStr        =  "move a range of windows into stack <stackNum>"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WSE,
        .cmdNameStr     = "wse",
        .cmdSyntaxStr   = "wse <stackNum> | ALL",
        .helpStr        = "enable window stacks"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WSD,
        .cmdNameStr     = "wsd",
        .cmdSyntaxStr   = "wsd <stackNum> | ALL",
        .helpStr        = "disable window stacks"
    },

    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WT,
        .cmdNameStr     =  "wt",
        .cmdSyntaxStr   =  "wt [ <winNum>, [ <toggleVal ]]",
        .helpStr        =  "toggle through alternate window content"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WX,
        .cmdNameStr     =  "wx",
        .cmdSyntaxStr   =  "wx <winNum>",
        .helpStr        =  "exchange current window with this window"
    },
    
    //------------------------------------------------------------------------------------
    // Predefined Functions.
    //
    //------------------------------------------------------------------------------------
    {
        .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_ASSEMBLE,
        .cmdNameStr     = "asm",
        .cmdSyntaxStr   = "asm ( <asmStr> )",
        .helpStr        = "returns the instruction value for an assemble string"
    },
    
    {
        .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_DIS_ASM,
        .cmdNameStr     = "disasm",
        .cmdSyntaxStr   = "disasm ( <instr> )",
        .helpStr        = "returns the assemble string for an instruction value"
    },

    {
        .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_REGION,
        .cmdNameStr     = "region",
        .cmdSyntaxStr   = "region ( <addr> )",
        .helpStr        = "returns the virtual region portion of an address"
    },

    {
        .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_PAGE,
        .cmdNameStr     = "page",
        .cmdSyntaxStr   = "page ( <addr> )",
        .helpStr        = "returns the virtual page portion of an address"
    },

    {
        .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_OFS,
        .cmdNameStr     = "ofs",
        .cmdSyntaxStr   = "ofs ( <addr> )",
        .helpStr        = "returns the virtual offset portion of an address"
    },

    {   .helpTypeId = TYP_P_FUNC,  .helpTokId  = PF_ADD_OFS,
        .cmdNameStr     = "add_ofs",
        .cmdSyntaxStr   = "add_ofs ( <addr> )",
        .helpStr        = "returns the address with the offset added"
    }
   
};

const int MAX_CMD_HELP_TAB = sizeof( cmdHelpTab ) / sizeof( SimHelpMsgEntry );
