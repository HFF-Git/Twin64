//----------------------------------------------------------------------------------------
//
//  Twin64Sim - A 64-bit CPU Simulator - Declarations
//
//----------------------------------------------------------------------------------------
// The Twin-64 Simulator is an interactive program for simulating a running Twin-64
// system. A simulation consist of modules, such as a processor, memory and I/O 
// module components, which together build the Twin-64 system. The system is created
// during simulator program start and can also be changed later on. Interaction is
// done via a terminal window environment, where windows represent the individual
// components. This file includes all the window environment related declarations.
// 
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Declarations
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

#include "T64-SimVersion.h"
#include "T64-Common.h"
#include "T64-Util.h"
#include "T64-ConsoleIO.h"
#include "T64-InlineAsm.h"
#include "T64-System.h"
#include "T64-Processor.h"
#include "T64-Memory.h"

//----------------------------------------------------------------------------------------
// When we say windows, don't think about a modern graphical window system. The 
// simulator is a simple terminal screen with portions of the screen representing 
// a "window". The general screen structure is:
//
//          |---> column (absolute)
//          |
//          v       :--------------------------------------------------------:
//        rows      :                                                        :
//     (absolute)   :                                                        :
//                  :              Active windows space                      :
//                  :                                                        :
//                  :--------------------------------------------------------:
//                  :                                                        :
//                  :              Command Window space                      :
//                  :                                                        :
//                  :--------------------------------------------------------:
//
// General window structure:
//
//          |---> column (relative)
//          |
//          v       :--------------------------------------------------------:
//        rows      :       Window Banner Line                               :
//      (relative)  :--------------------------------------------------------:
//                  :                                                        :
//                  :                                                        :
//                  :                                                        :
//                  :       Window Content                                   :
//                  :                                                        :
//                  :                                                        :
//                  :--------------------------------------------------------:
//
// Total size of the screen can vary. It is the sum of all active window line plus
// the command window lines. Command window is a bit special on that it has an input
// line at the lowest line. Scroll lock after the active windows before the command
// window. Routines to move cursor, print fields with attributes.
//
// In addition, windows can be organized in stacks. The stacks are displayed next
// to each other, which is quite helpful, but could make the columns needed quite
// large. The command window will in this case span all stacks.
//
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
//
//  Global Window commands:
//
//  WON, WOFF       -> on, off
//  WDEF            -> window defaults, show initial screen.
//  WSE, WSD        -> winStackEnable/Disable
//
//  Window commands:
//
//  enable, disable -> winEnable        -> E, D
//  back, forward   -> winMove          -> B, F
//  home, jump      -> winJump          -> H, J
//  rows            -> setRows          -> L
//  radix           -> setRadix         -> R
//  new             -> newUserWin       -> N
//  kill            -> winUserKill      -> K
//  current         -> currentUserWin   -> C
//  toggle          -> winToggle        -> T
//
//  Windows:
//
//  CPU Window      -> CPU
//  TLB Window      -> TLB
//  Cache Window    -> CACHE
//  Memory Window   -> MEM
//  Program Code    -> CODE
//  Text Window     -> TEXT
//  Commands        -> n/a
//
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Fundamental constants for the T64 window system.
//
//----------------------------------------------------------------------------------------
const int MAX_FILE_PATH_SIZE        = 256;
const int MAX_TEXT_FIELD_LEN        = 132;
const int MAX_TEXT_LINE_SIZE        = 256;

const int MAX_WINDOWS               = 32;
const int MAX_WIN_STACKS            = 8;
const int MAX_WIN_ROW_SIZE          = 64;
const int MAX_WIN_COL_SIZE          = 256;
const int MAX_WIN_OUT_LINES         = 256;
const int MAX_WIN_OUT_LINE_SIZE     = 256;
const int MAX_WIN_NAME              = 8;
const int MAX_WIN_TOGGLES           = 8;

const int MAX_CMD_HIST              = 64;
const int MAX_CMD_LINES             = 64;
const int MAX_CMD_LINE_SIZE         = 256;

const int MAX_TOK_STR_SIZE          = 256;
const int MAX_TOK_NAME_SIZE         = 32;

const int MAX_ENV_NAME_SIZE         = 32;
const int MAX_ENV_VARIABLES         = 256;

//----------------------------------------------------------------------------------------
// Windows have a type. The type is primarily used to specify what type of window
// to create. 
//
//----------------------------------------------------------------------------------------
enum SimWinType : int {

    WT_NIL,                     WT_CMD_WIN,                 WT_CONSOLE_WIN,
    WT_TEXT_WIN,                WT_CPU_WIN,                 WT_TLB_WIN,
    WT_CACHE_WIN,               WT_MEM_WIN,                 WT_CODE_WIN
};

//----------------------------------------------------------------------------------------
// Command line tokens and expression have a type.
//
//----------------------------------------------------------------------------------------
enum SimTokTypeId : int {

    TYP_NIL,                    TYP_NUM,                    TYP_STR,
    TYP_BOOL,                   TYP_SYM,                    TYP_IDENT,
    TYP_CMD,                    TYP_WCMD,                   TYP_P_FUNC,
    TYP_GREG,                   TYP_CREG,                   TYP_PREG
};

//----------------------------------------------------------------------------------------
// Tokens are the labels for reserved words and symbols recognized by the tokenizer 
// objects. Tokens have a name, a token id, a token type and an optional value with 
// further data. See also the "SimTables" include file how types and token Id are 
// used to build the command and expression tokens.
//
//----------------------------------------------------------------------------------------
enum SimTokId : uint16_t {

    //------------------------------------------------------------------------------------
    // General tokens and symbols.
    //
    //------------------------------------------------------------------------------------
    TOK_NIL,                    TOK_ERR,                    TOK_EOS,
    TOK_COMMA,                  TOK_PERIOD,                 TOK_COLON,
    TOK_LPAREN,                 TOK_RPAREN,                 TOK_QUOTE,
    TOK_EQUAL,                  TOK_PLUS,                   TOK_MINUS,
    TOK_MULT,                   TOK_DIV,                    TOK_MOD,
    TOK_REM,                    TOK_NEG,                    TOK_AND,
    TOK_OR,                     TOK_XOR,                    TOK_EQ,
    TOK_NE,                     TOK_LT,                     TOK_GT,
    TOK_LE,                     TOK_GE,

    //------------------------------------------------------------------------------------
    // Token symbols.
    //------------------------------------------------------------------------------------
    TOK_IDENT,                  TOK_NUM,                    TOK_STR,
    TOK_DEF,                    TOK_ALL,                    TOK_DEC,
    TOK_HEX,                    TOK_MEM,                    TOK_CODE,
    TOK_STATS,                  TOK_TEXT,                   TOK_SYS,
    TOK_PROC,                   TOK_CPU,                    TOK_IO,
    TOK_ITLB,                   TOK_DTLB,                   TOK_ICACHE,
    TOK_DCACHE,                 TOK_TLB_FA_64S,             TOK_TLB_FA_128S,
    TOK_CACHE_SA_2W_128S_4L,    TOK_CACHE_SA_4W_128S_4L,    TOK_CACHE_SA_8W_128S_4L,
    TOK_CACHE_SA_2W_64S_8L,     TOK_CACHE_SA_4W_64S_8L,     TOK_CACHE_SA_8W_64S_8L,
    TOK_MEM_READ_ONLY,          TOK_MEM_READ_WRITE,         TOK_MOD_SPA_ADR,
    TOK_MOD_SPA_LEN,

    //------------------------------------------------------------------------------------
    // Line Commands.
    //
    //------------------------------------------------------------------------------------
    CMD_SET,                    CMD_EXIT,                   CMD_HELP,
    CMD_DO,                     CMD_REDO,                   CMD_HIST,
    CMD_ENV,                    CMD_XF,                     CMD_LF,
    CMD_WRITE_LINE,             CMD_DM,                     CMD_DW,
    CMD_NM,                     CMD_RM,                     CMD_RESET,
    CMD_RUN,                    CMD_STEP,                   CMD_MR,
    CMD_DA,                     CMD_MA,                     CMD_ITLB_I,
    CMD_ITLB_D,                 CMD_PTLB_I,                 CMD_PTLB_D,
    CMD_PCA_I,                  CMD_PCA_D,                  CMD_FCA_I,
    CMD_FCA_D,

    //------------------------------------------------------------------------------------
    // Window Commands Tokens.
    //
    //------------------------------------------------------------------------------------
    WCMD_SET,                   WTYPE_SET,                  CMD_WON,
    CMD_WOFF,                   CMD_WDEF,                   CMD_CWL,
    CMD_CWC,                    CMD_WSE,                    CMD_WSD,
    CMD_WE,                     CMD_WD,                     CMD_WR,
    CMD_WF,                     CMD_WB,                     CMD_WH,
    CMD_WJ,                     CMD_WL,                     CMD_WN,
    CMD_WK,                     CMD_WS,                     CMD_WC,
    CMD_WT,                     CMD_WX,

    //------------------------------------------------------------------------------------
    // Predefined Function Tokens.
    //
    //------------------------------------------------------------------------------------
    PF_SET,                     PF_ASSEMBLE,                PF_DIS_ASM,
    PF_HASH,                    PF_S32,

    //------------------------------------------------------------------------------------
    // General, Control and PSW Register Tokens.
    //
    //------------------------------------------------------------------------------------
    REG_SET,                    GR_0,                       GR_1,
    GR_2,                       GR_3,                       GR_4,
    GR_5,                       GR_6,                       GR_7,
    GR_8,                       GR_9,                       GR_10,
    GR_11,                      GR_12,                      GR_13,
    GR_14,                      GR_15,                      GR_SET,

    CR_0,                       CR_1,                       CR_2,
    CR_3,                       CR_4,                       CR_5,
    CR_6,                       CR_7,                       CR_8,
    CR_9,                       CR_10,                      CR_11,
    CR_12,                      CR_13,                      CR_14,
    CR_15,                      CR_SET,                     

    PR_IA,                      PR_ST
};

//----------------------------------------------------------------------------------------
// Our error messages IDs. There is a routine that maps the ID to a text string.
//
// ??? clean up, keep the ones we need...
//----------------------------------------------------------------------------------------
enum SimErrMsgId : int {
    
    NO_ERR                          = 0,
    ERR_NOT_SUPPORTED               = 1,
    ERR_NOT_IN_WIN_MODE             = 2,
    ERR_TOO_MANY_ARGS_CMD_LINE      = 3,
    ERR_CMD_LINE_TOO_LONG           = 4,
    ERR_EXTRA_TOKEN_IN_STR          = 5,
    ERR_INVALID_CHAR_IN_TOKEN_LINE  = 6,
    ERR_INVALID_CHAR_IN_IDENT       = 7,
    ERR_NUMERIC_OVERFLOW            = 8,

    ERR_INVALID_CMD                 = 10,
    ERR_INVALID_EXPR                = 20,
    ERR_INVALID_ARG                 = 11,
    ERR_INVALID_WIN_STACK_ID        = 12,
    ERR_INVALID_WIN_ID              = 13,
    ERR_INVALID_WIN_TYPE            = 14,
    ERR_INVALID_EXIT_VAL            = 15,
    ERR_INVALID_RADIX               = 16,
    ERR_INVALID_REG_ID              = 17,
    ERR_INVALID_FMT_OPT             = 23,

    ERR_INVALID_MODULE_TYPE         = 24,
   
    ERR_INVALID_NUM                 = 25,
    
    // -----
   
    
    ERR_EXPECTED_COMMA              = 100,
    ERR_EXPECTED_COLON              = 101,
    ERR_EXPECTED_LPAREN             = 102,
    ERR_EXPECTED_RPAREN             = 103,
    ERR_EXPECTED_CLOSING_QUOTE      = 104,
    ERR_EXPECTED_NUMERIC            = 105,
    ERR_EXPECTED_EXT_ADR            = 106,
    ERR_EXPECTED_FILE_NAME          = 107,
    ERR_EXPECTED_WIN_ID             = 108,
    ERR_EXPECTED_WIN_TYPE           = 109,
    ERR_EXPECTED_STACK_ID           = 110,
    ERR_EXPECTED_REG_OR_SET         = 111,
    ERR_EXPECTED_REG_SET            = 112,
    ERR_EXPECTED_GENERAL_REG        = 113,
  
    ERR_EXPECTED_OFS                = 213,
    ERR_EXPECTED_START_OFS          = 214,
    ERR_EXPECTED_LEN                = 215,
    ERR_EXPECTED_STEPS              = 116,
    ERR_EXPECTED_INSTR_VAL          = 117,
    ERR_EXPECTED_INSTR_OPT          = 318,

    ERR_EXPECTED_MOD_NUM            = 219,
   
    ERR_IN_ASM_PFUNC                = 320,
    ERR_IN_DISASM_PFUNC             = 321,

   
    ERR_INVALID_ELF_FILE            = 700,
    ERR_ELF_INVALID_ADR_RANGE       = 701,
    ERR_ELF_MEMORY_SIZE_EXCEEDED    = 702,
    ERR_INVALID_ELF_BYTE_ORDER      = 703,

    ERR_EXPECTED_AN_OFFSET_VAL      = 321,
    ERR_EXPECTED_FMT_OPT            = 322,
   
    ERR_EXPECTED_STR                = 324,
    ERR_EXPECTED_EXPR               = 325,
    
    ERR_FILE_NOT_FOUND              = 350,
    ERR_UNEXPECTED_EOS              = 351,
    
    ERR_ENV_VAR_NOT_FOUND           = 400,
    ERR_ENV_VALUE_EXPR              = 401,
    ERR_ENV_PREDEFINED              = 403,
    ERR_ENV_TABLE_FULL              = 404,
    ERR_OPEN_EXEC_FILE              = 405,
    
    ERR_EXPR_TYPE_MATCH             = 406,
    ERR_EXPR_FACTOR                 = 407,

    ERR_OFS_LEN_LIMIT_EXCEEDED      = 408,
    ERR_INSTR_HAS_NO_OPT            = 409,
    ERR_IMM_VAL_RANGE               = 410,
   
    ERR_POS_VAL_RANGE               = 412,
    ERR_LEN_VAL_RANGE               = 413,
    ERR_OFFSET_VAL_RANGE            = 414,
    
    ERR_OUT_OF_WINDOWS              = 415,
    ERR_WIN_TYPE_NOT_CONFIGURED     = 416,
    
    ERR_UNDEFINED_PFUNC             = 417,

    ERR_NUMERIC_RANGE               = 420,

    ERR_TLB_TYPE                    = 500,
    ERR_TLB_PURGE_OP                = 501,
    ERR_TLB_INSERT_OP               = 502,
    ERR_TLB_ACC_DATA                = 503,
    ERR_TLB_ADR_DATA                = 504,
    ERR_TLB_NOT_CONFIGURED          = 505,
    ERR_TLB_SIZE_EXCEEDED           = 506,
    
    ERR_CACHE_TYPE                  = 600,
    ERR_CACHE_FLUSH_OP              = 601,
    ERR_CACHE_PURGE_OP              = 602,
    ERR_CACHE_SET_NUM               = 603,
    ERR_CACHE_NOT_CONFIGURED        = 604,
    ERR_CACHE_SIZE_EXCEEDED         = 605,

    ERR_MEM_OP_FAILED               = 700,

    ERR_CREATE_PROC_MODULE          = 701,
    ERR_CREATE_MEM_MODULE           = 702,

    ERR_INVALID_TLB_ACC_FLAG        = 800
};

//----------------------------------------------------------------------------------------
// Predefined environment variable names. When you create another one, put its name
// here.
//
//----------------------------------------------------------------------------------------
const char ENV_NIL[ ]                   = "NIL";
const char ENV_TRUE[ ]                  = "TRUE";
const char ENV_FALSE[ ]                 = "FALSE";

const char ENV_PROG_VERSION [ ]         = "PROG_VERSION";
const char ENV_PATCH_LEVEL [ ]          = "PATCH_LEVEL";
const char ENV_GIT_BRANCH[ ]            = "GIT_BRANCH";

const char ENV_SHOW_CMD_CNT[ ]          = "SHOW_CMD_CNT" ;
const char ENV_CMD_CNT[ ]               = "CMD_CNT" ;
const char ENV_ECHO_CMD_INPUT[ ]        = "ECHO_CMD_INPUT";
const char ENV_EXIT_CODE [ ]            = "EXIT_CODE";

const char ENV_RDX_DEFAULT [ ]          = "RDX_DEFAULT";
const char ENV_WORDS_PER_LINE [ ]       = "WORDS_PER_LINE";
const char ENV_WIN_MIN_ROWS[ ]          = "WIN_MIN_ROWS";
const char ENV_WIN_TEXT_LINE_WIDTH[ ]   = "WIN_TEXT_WIDTH";

//----------------------------------------------------------------------------------------
// Forward declaration of the globals structure. Every object will have access to 
// the globals structure, so we do not have to pass around references to all the
// individual objects. The globals structure contains references to all the important
// objects in the simulator. 
//
//----------------------------------------------------------------------------------------
struct SimGlobals;

//----------------------------------------------------------------------------------------
// Command line option argument types and structure. This is the argc, argv parser
// used to parse long options (e.g. --option=value).
//
//----------------------------------------------------------------------------------------
enum SimCmdLineArgOptions : int {

    CL_OPT_NO_ARGUMENT,         
    CL_OPT_REQUIRED_ARGUMENT,   
    CL_OPT_OPTIONAL_ARGUMENT
};

enum SimCmdLineArgVal : int {

    CL_ARG_VAL_NIL,             
    CL_ARG_VAL_HELP,             
    CL_ARG_VAL_VERSION,
    CL_ARG_VAL_VERBOSE,        
    CL_ARG_VAL_CONFIG_FILE,      
    CL_ARG_VAL_LOG_FILE
};

struct SimCmdLineOptions {

    const char              *name;
    SimCmdLineArgOptions    argOpt;   
    int                     val;
};

//----------------------------------------------------------------------------------------
// An error is described in the error message table.
//
//----------------------------------------------------------------------------------------
struct SimErrMsgTabEntry {
    
    SimErrMsgId errNum;
    char        *errStr;
};

//----------------------------------------------------------------------------------------
// An help message is described in the help message table.
//
//----------------------------------------------------------------------------------------
struct SimHelpMsgEntry {
    
    SimTokTypeId    helpTypeId;
    SimTokId        helpTokId;
    char            *cmdNameStr;
    char            *cmdSyntaxStr;
    char            *helpStr;
};

//----------------------------------------------------------------------------------------
// The command line interpreter works the command line as a list of tokens. A 
// token found in a string is recorded using the token structure. The token types 
// are found in a string is recorded using the token structure. The token types 
// are found in a string is recorded using the token structure. The supported 
// token types are numeric and strings. The string is a buffer in the tokenizer. 
// Scanning a new token potentially overwrites or invalidates the string. You 
// need to copy it to a safe place before scanning the next token.
//
//----------------------------------------------------------------------------------------
struct SimToken {

    char         name[ MAX_TOK_NAME_SIZE ];
    SimTokTypeId typ;
    SimTokId     tid;
    
    union {
        
        T64Word val;
        char    *str; 

    } u;
};

//----------------------------------------------------------------------------------------
// Tokenizer base abstract class. The tokenizer object scans an input character
// stream. It breaks the stream into tokens. The tokenizer raises exceptions. 
// The base class contains the common routines and data structures. The derived 
// classes implement the character input routines.
//
//----------------------------------------------------------------------------------------
struct SimTokenizer {

    public:

    SimTokenizer( );

    void            setupTokenizer( char *lineBuf, SimToken *tokTab );
    void            nextToken( );
    
    bool            isToken( SimTokId tokId );
    bool            isTokenTyp( SimTokTypeId typId );
    bool            isTokenIdent( char *name );  

    SimToken        token( );
    SimTokTypeId    tokTyp( );
    SimTokId        tokId( );
    char            *tokName( );
    T64Word         tokVal( );
    char            *tokStr( );
   
    void            checkEOS( );
    void            acceptComma( );
    void            acceptColon( );
    void            acceptEqual( );
    void            acceptLparen( );
    void            acceptRparen( );
    SimTokId        acceptTokSym( SimErrMsgId errId );

    private:
    
    virtual void    nextChar( ) = 0;
    void            parseNum( );
    void            parseString( );
    void            parseIdent( );

    protected:

    char            currentChar     = ' ';
    SimToken        *tokTab         = nullptr;   
    SimToken        currentToken;
     
};

//----------------------------------------------------------------------------------------
// Tokenizer from string. The command line interface parse their input buffer line.
// The tokenizer will return the tokens found in the line. The tokenizer raises 
// exceptions.
//
//----------------------------------------------------------------------------------------
struct SimTokenizerFromString : public SimTokenizer {
    
    public:
    
    SimTokenizerFromString( );
    void setupTokenizer( char *lineBuf, SimToken *tokTab ); 
    
    private:

    void            nextChar( );

    int             currentCharIndex    = 0;
    int             currentLineLen      = 0;
    char            tokenLine[ 256 ]    = { 0 };

};

//----------------------------------------------------------------------------------------
// Tokenizer from file. We may need one day to read commands from a file. This
// tokenizer reads characters from a file stream. The tokenizer raises exceptions.
//
//----------------------------------------------------------------------------------------
struct SimTokenizerFromFile : public SimTokenizer {
    
    public:
    
    SimTokenizerFromFile( );
    virtual ~SimTokenizerFromFile( );

    void    setupTokenizer( char *filePath, SimToken *tokTab );
    int     getCurrentLineIndex( );
    int     getCurrentCharPos( );
    
    private:
    
    void    openFile( char *filePath );
    void    closeFile( );
    void    nextChar( );

    int    currentLineIndex     = 0;
    int    currentCharIndex     = 0;
    FILE   *srcFile             = nullptr;
};

//----------------------------------------------------------------------------------------
// Expression value. The analysis of an expression results in a value. Depending on 
// the expression type, the values are simple scalar values or a structured values.
//
//----------------------------------------------------------------------------------------
struct SimExpr {
    
    SimTokTypeId typ;
   
    union {

        T64Word val;
        bool    bVal;  
        char    *str;

    } u;
};

//----------------------------------------------------------------------------------------
// The expression evaluator object. We use the "parseExpr" routine wherever we expect
// an expression in the command line. The evaluator raises exceptions.
//
//----------------------------------------------------------------------------------------
struct SimExprEvaluator {
    
    public:
    
    SimExprEvaluator( SimGlobals *glb, SimTokenizer *tok );
    
    void            setTokenizer( SimTokenizer *tok );
    void            parseExpr( SimExpr *rExpr );
    T64Word         acceptNumExpr( SimErrMsgId errCode, 
                                   T64Word low = INT64_MIN, 
                                   T64Word high = INT64_MAX );
    
    private:
    
    void            parseTerm( SimExpr *rExpr );
    void            parseFactor( SimExpr *rExpr );

    void            parsePredefinedFunction( SimToken funcId, SimExpr *rExpr );    
    void            pFuncAssemble( SimExpr *rExpr );
    void            pFuncDisAssemble( SimExpr *rExpr );
    void            pFuncHash( SimExpr *rExpr );
    void            pFuncS32( SimExpr *rExpr );
    void            pFuncU32( SimExpr *rExpr );
    
    SimGlobals      *glb        = nullptr;
    SimTokenizer    *tok        = nullptr;
    T64Assemble     *inlineAsm  = nullptr;
    T64DisAssemble  *disAsm     = nullptr;
};

//----------------------------------------------------------------------------------------
// Environment table entry, Each environment variable has a name, a couple of flags 
// and the value. There are predefined variables and user defined variables.
//
//----------------------------------------------------------------------------------------
struct SimEnvTabEntry {
    
    char            name[ MAX_ENV_NAME_SIZE ]   = { 0 };
    bool            valid                       = false;
    bool            predefined                  = false;
    bool            readOnly                    = false;
    SimTokTypeId    typ                         = TYP_NIL;
    
    union {
    
        bool        bVal; 
        T64Word     iVal;        
        char        *strVal;      

    } u;
};

//----------------------------------------------------------------------------------------
// Environment variables. The simulator has a global table where all variables are 
// kept. It is a simple array with a high water mark concept. The table will be 
// allocated at simulator start.
//
//----------------------------------------------------------------------------------------
struct SimEnv {

    public:
    
    SimEnv( SimGlobals *glb, int size );
    
    void            setupPredefined( );
   
    void            setEnvVar( char *name, T64Word val );
    void            setEnvVar( char *name, bool val );
    void            setEnvVar( char *name, char *str );
    void            removeEnvVar( char *name );
    
    bool            getEnvVarBool( char *name,bool def = false );
    T64Word         getEnvVarInt( char *name, T64Word def = 0 );
    char            *getEnvVarStr( char *name, char *def = nullptr );
    SimEnvTabEntry  *getEnvEntry( char *name );
    SimEnvTabEntry  *getEnvEntry( int index );

    int             getEnvHwm( );
    int             formatEnvEntry( char *name, char *buf, int bufLen );
    int             formatEnvEntry( int index, char *buf, int bufLen );
    
    bool            isValid( char *name );
    bool            isReadOnly( char *name );
    bool            isPredefined( char *name );
   
    private:
    
    int             lookupEntry( char *name );
    int             findFreeEntry( );
    
    void            enterVar( char *name, 
                              T64Word val, 
                              bool predefined = false, 
                              bool rOnly = false );

    void            enterVar( char *name, 
                              bool val, 
                              bool predefined = false, 
                              bool rOnly = false );

    void            enterVar( char *name, 
                              char *str, 
                              bool predefined = false, 
                              bool rOnly = false );
   
    SimEnvTabEntry  *table  = nullptr;
    SimEnvTabEntry  *hwm    = nullptr;
    SimEnvTabEntry  *limit  = nullptr;
    SimGlobals      *glb    = nullptr;
};

//----------------------------------------------------------------------------------------
// Command History. The simulator command interpreter features a simple command history.
// It is a circular buffer that holds the last commands. There are functions to show
// the command history, re-execute a previous command and to retrieve a previous 
// command for editing.
//
//----------------------------------------------------------------------------------------
struct SimCmdHistEntry {
    
    int  cmdId;
    char cmdLine[ MAX_CMD_LINE_SIZE ];
};

struct SimCmdHistory {
    
    public:
    
    SimCmdHistory( );
    
    void addCmdLine( char *cmdStr );
    char *getCmdLine( int cmdRef, int *cmdId = nullptr );
    int  getCmdCount( );
    int  getCmdNum( );
   
    private:
    
    int nextCmdNum          = 0;
    int head                = 0;
    int tail                = 0;
    int count               = 0;
    
    SimCmdHistEntry history[ MAX_CMD_HIST ];
};

//----------------------------------------------------------------------------------------
// Command and Console Window output buffer. The output buffer will store all output
// from the command window to support scrolling. This is the price you pay when normal
// terminal scrolling is restricted to an area of the screen. The buffer offers a
// simple interface. Any character added will be stored in a line, a "\n" will advance
// to the next line to store. The buffer itself is a circular buffer. Each time a 
// command line is entered, the display will show the last N lines entered. A cursor
// is defined which is manipulated by the cursor up or down routines.
//
//----------------------------------------------------------------------------------------
struct SimWinOutBuffer : SimFormatter {
    
    public:
    
    SimWinOutBuffer( );
    
    void        initBuffer( );
    void        addToBuffer( const char *data );
    int         writeChars( const char *format, ... );
    int         writeChar( const char ch );
    void        setScrollWindowSize( int size );
    
    void        resetLineCursor( );
    char        *getLineRelative( int lineBelowTop );
    int         getCursorIndex( );
    int         getTopIndex( );
    
    void        scrollUp( int lines = 1 );
    void        scrollDown( int lines = 1 );
    
    private:
    
    char        buffer[ MAX_WIN_OUT_LINES ] [ MAX_WIN_OUT_LINE_SIZE ];
    int         topIndex    = 0; // Index of the next line to use.
    int         cursorIndex = 0; // Index of the last line currently shown.
    int         screenLines = 0; // Number of lines displayed in the window.
    int         charPos     = 0; // Current character position in the line.
};

//----------------------------------------------------------------------------------------
// A window area is defined by the number of rows and columns.e
// 
//----------------------------------------------------------------------------------------
struct SimWinSize {

    int         col = 0;
    int         row = 0;
};

//----------------------------------------------------------------------------------------
// The "SimWin" class. The simulator will in screen mode feature a set of stacks each 
// with a list of screen sub windows. The default is one stack, the general register
// set window and the command line window, which also spans all stacks. Each sub window
// is an instance of a specific window class with this class as the base class. There
// are routines common to all windows to enable/ disable, set the lines displayed and
// so on. There are also abstract methods that the inheriting class needs to implement.
// Examples are to initialize a window, redraw and so on.
//
// A window can also implement different views of the data. This is handled by a 
// toggle mechanism. The window maintains the current toggle value as well as the
// default and actual sizes of the windows for each toggle view.
//
// Most windows will be associated with a module or submodule. The window also keeps
// the simulator module number it is associated with.
//
//----------------------------------------------------------------------------------------
struct SimWin {
    
    public:
    
    SimWin( SimGlobals *glb );
    virtual         ~ SimWin( );
    
    void            setWinType( SimWinType type );
    SimWinType      getWinType( );
    
    void            setWinIndex( int index );
    int             getWinIndex( );

    void            setWinName( char *name );
    char            *getWinName( );

    void            setWinModNum( int modNum );
    int             getWinModNum( );

    void            setEnable( bool arg );
    bool            isEnabled( );
    
    void            setRadix( int radix );
    int             getRadix( );
    
    int             getDefRows( );
    int             getRows( );
    void            setRows( int arg );
    
    int             getDefColumns( );
    int             getColumns( );
    void            setColumns( int arg );

    void            setWinOrigin( int row, int col );
    void            setWinCursor( int row, int col );
    
    int             getWinCursorRow( );
    int             getWinCursorCol( );
    
    int             getWinStack( );
    void            setWinStack( int wStack );

    void            setWinToggleLimit( int limit );
    int             getWinToggleLimit( );

    int             getWinToggleVal( );
    void            setWinToggleVal( int val );

    SimWinSize      getWinDefSize( int toggleVal );
    void            setWinDefSize( int toggleVal, int row, int col );

    void            initWinToggleSizes( );
    
    void            printNumericField(  T64Word val,
                                        uint32_t fmtDesc = 0,
                                        int len = 0,
                                        int row = 0,
                                        int col = 0 );
    
    void            printTextField( char *text,
                                    uint32_t fmtDesc = 0,
                                    int len = 0,
                                    int row = 0,
                                    int col = 0 );

    void            printBitField(  T64Word val, 
                                    int pos,
                                    char printChar,
                                    uint32_t fmtDesc = 0,
                                    int len = 0,
                                    int row = 0,
                                    int col = 0 ); 
    
    void            printRadixField( uint32_t fmtDesc = 0,
                                     int len = 0,
                                     int row = 0,
                                     int col = 0 );
    
    void            printWindowIdField( uint32_t fmtDesc = 0,
                                        int row = 0,
                                        int col = 0 );
    
    void            padLine( uint32_t fmtDesc = 0 );
    void            padField( int dLen, int fLen );
    void            clearField( int len, uint32_t fmtDesc = 0 ); 
    
    void            reDraw( );
    
    virtual void    toggleWin( int toggleVal = 0 );
    virtual void    setDefaults( )  = 0;
    virtual void    drawBanner( )   = 0;
    virtual void    drawBody( )     = 0;
    
    protected:
    
    SimGlobals      *glb;
   
    private:
    
    SimWinType      winType             = WT_NIL;
    int             winIndex            = 0;
    int             winModNum           = -1;
    char            winName[ MAX_WIN_NAME];
    SimWinSize      winDefSizes[ MAX_WIN_TOGGLES ];
    
    bool            winEnabled          = false;
    int             winRadix            = 16;
    int             winStack            = 0;
    
    int             winToggleLimit      = 0;
    int             winToggleVal        = 0;
    
    int             winColumns          = 0;
    int             winRows             = 0;       

    int             winAbsCursorRow     = 0;
    int             winAbsCursorCol     = 0;
    int             lastRowPos          = 0;
    int             lastColPos          = 0;    
};

//----------------------------------------------------------------------------------------
// "WinScrollable" is an extension to the basic window. It implements scrollable
// windows with a number of lines. There is a high level concept of a starting index
// of zero and a limit. The meaning i.e. whether the index is a memory address or an
// index into a TLB or Cache array is determined by the inheriting class. The 
// scrollable window will show a number of lines, the "drawLine" method needs to be
// implemented by the inheriting class. The routine is passed the item address for 
// the line and is responsible for the correct address interpretation. The 
// "lineIncrement" is the increment value for the item address passed.
//
// There is the scenario that a line item actually spans to or even more lines. The
// actual rows needed is the line increment times the rows per line item. In most 
// cases there is however a one to one mapping.
// 
//----------------------------------------------------------------------------------------
struct SimWinScrollable : SimWin {
    
    public:
    
    SimWinScrollable( SimGlobals *glb );

    virtual         ~ SimWinScrollable( );
    
    void            setHomeItemAdr( T64Word adr );
    T64Word         getHomeItemAdr( );

    void            setCurrentItemAdr( T64Word adr );
    T64Word         getCurrentItemAdr( );
    
    void            setLimitItemAdr( T64Word adr );
    T64Word         getLimitItemAdr( );
    
    void            setLineIncrementItemAdr( int arg );
    int             getLineIncrementItemAdr( );

    void            winHome( T64Word pos = 0 );
    void            winJump( T64Word pos );
    void            winForward( T64Word amt );
    void            winBackward( T64Word amt );
   
    virtual void    drawBody( );
    virtual void    drawLine( T64Word index ) = 0;
    
    private:
    
    T64Word         homeItemAdr         = 0;
    T64Word         currentItemAdr      = 0;
    T64Word         limitItemAdr        = 0;
    T64Word         lineIncrement       = 0;
    int             rowsPerItemLine     = 1;
};

//----------------------------------------------------------------------------------------
// CPU Register Window. This window holds the programmer visible state. The window
// is a toggle window, top show different sets of register data. The constructor is
// passed our globals and the module number of the processor.
//
//----------------------------------------------------------------------------------------
struct SimWinCpuState : SimWin {
    
    public:
    
    SimWinCpuState( SimGlobals *glb, int modNum );
    
    void setDefaults( );
    void drawBanner( );
    void drawBody( );

    private:

    int          modNum  = 0;
    T64Processor *proc   = nullptr;
};

//----------------------------------------------------------------------------------------
// Absolute Memory Window. A memory window will show the absolute memory content
// starting with the current address followed by a number of data words. The number
// of words shown is the number of lines of the window times the number of items, 
// i.e. words, on a line.
//
//----------------------------------------------------------------------------------------
struct SimWinAbsMem : SimWinScrollable {
    
    public:
    
    SimWinAbsMem( SimGlobals *glb, int modNum, T64Word adr );
    
    void setDefaults( );
    void drawBanner( );
    void drawLine( T64Word index );

    private:

    T64Word adr = 0;
};

//----------------------------------------------------------------------------------------
// Code Memory Window. A code memory window will show the instruction memory content
// starting with the current address followed by the instruction and a human readable
// disassembled version.
//
//----------------------------------------------------------------------------------------
struct SimWinCode : SimWinScrollable {
    
    public:
    
    SimWinCode( SimGlobals *glb, int modNum, T64Word adr );
    ~ SimWinCode( );
    
    void setDefaults( );
    void drawBanner( );
    void drawLine( T64Word index );
    
    private:

    T64Word         adr     = 0;
    T64DisAssemble  *disAsm = nullptr;
};

//----------------------------------------------------------------------------------------
// TLB Window. The TLB data window displays the TLB entries.
//
//----------------------------------------------------------------------------------------
struct SimWinTlb : SimWinScrollable {
    
    public:
    
    SimWinTlb( SimGlobals *glb, int modNum, T64Tlb *tlb );
    
    void setDefaults( );
    void drawBanner( );
    void drawLine( T64Word index );

    private:

    T64Tlb *tlb = nullptr;
};

//----------------------------------------------------------------------------------------
// Cache Window. The memory object window display the cache date lines. Since we can
// have caches with more than one set, the toggle function allows to flip through the
// sets, one at a time.
//
//----------------------------------------------------------------------------------------
struct SimWinCache : SimWinScrollable {
    
    public:
    
    SimWinCache( SimGlobals *glb, int modNum, T64Cache *cache );
    
    void setDefaults( );
    void drawBanner( );
    void drawLine( T64Word index );

    private:

    T64Cache    *cache  = nullptr;
};

//----------------------------------------------------------------------------------------
// Text Window. It may be handy to also display an ordinary ASCII text file. One day
// this will allow us to display for example the source code to a running program 
// when symbolic debugging is supported.
//
//----------------------------------------------------------------------------------------
struct SimWinText : SimWinScrollable {
    
    public:
    
    SimWinText( SimGlobals *glb, char *fName );
    ~ SimWinText( );
    
    void    setDefaults( );
    void    drawBanner( );
    void    drawLine( T64Word index );
    
    private:

    bool    openTextFile( );
    int     readTextFileLine( int linePos, char *lineBuf, int bufLen );
    
    FILE    *textFile          = nullptr;
    int     fileSizeLines      = 0;
    int     lastLinePos        = 0;
    char    fileName[ MAX_FILE_PATH_SIZE ] = { 0 };
};

//----------------------------------------------------------------------------------------
// Console Window. When the CPU is running, it has access to a "console window". This
// is a rather simple console IO window. Care needs to be taken however what character
// IO directed to this window means. For example, escape sequences cannot be just 
// printed out as it would severely impact the simulator windows. Likewise scrolling 
// and line editing are to be handheld.
//
//----------------------------------------------------------------------------------------
struct SimWinConsole : SimWin {
    
public:
    
    SimWinConsole( SimGlobals *glb );
   
    void    setDefaults( );
    void    drawBanner( );
    void    drawBody( );
    
    void    putChar( char ch );
    
    // ??? methods to read a character ?
    // ??? methods to switch between command and console mode ?
    
private:
    
    SimGlobals      *glb    = nullptr;
    SimWinOutBuffer *winOut = nullptr;
};

//----------------------------------------------------------------------------------------
// Command Line Window. The command window is a special class, which comes always
// last in the windows list and cannot be disabled. It is intended to be a scrollable
// window, where only the banner line is fixed.
//
//----------------------------------------------------------------------------------------
struct SimCommandsWin : SimWin {
    
public:
    
    SimCommandsWin( SimGlobals *glb );
    
    void            setDefaults( );
    void            drawBanner( );
    void            drawBody( );
    void            clearCmdWin( );
    SimTokId        getCurrentCmd( );
    void            cmdInterpreterLoop( );
    
private:
    
    void            printWelcome( );
    int             buildCmdPrompt( char *promptStr, int promptStrLen );
    int             readCmdLine( char *cmdBuf, int cmdBufLen, char *promptStr );
    void            evalInputLine( char *cmdBuf );
    void            cmdLineError( SimErrMsgId errNum, char *argStr = nullptr );
    int             promptYesNoCancel( char *promptStr );

    void            configureT64Sim( );

    void            ensureWinModeOn( );
    void            printStackInfoField( uint32_t fmtDesc = 0,
                                         int row = 0,
                                         int col = 0 );
  
    void            displayAbsMemContent( T64Word ofs, T64Word len, int rdx = 16 );
    void            displayAbsMemContentAsCode( T64Word ofs, T64Word len );

    void            parseWinNumRange( int *winNumStart, int *winNumEnd );
    
    void            exitCmd( );
    void            helpCmd( );
    void            envCmd( );
    void            execFileCmd( );
    void            loadElfFileCmd( );
    void            loadElfFile( char *fileName );
    void            writeLineCmd( );
    void            execCmdsFromFile( char* fileName );
    
    void            histCmd( );
    void            doCmd( );
    void            redoCmd( );
    
    void            addProcModule( );
    void            addMemModule( );
    void            addIoModule( );

    void            addModuleCmd( );
    void            removeModuleCmd( );

    void            displayModuleCmd( );
    void            displayStackCmd( );
    void            displayWindowCmd( );

    void            resetCmd( );
    void            runCmd( );
    void            stepCmd( );
   
    void            modifyRegCmd( );
    
    void            displayAbsMemCmd( );
    void            modifyAbsMemCmd( );
    
    void            displayCacheCmd( );
    void            purgeCacheCmd( );
    void            flushCacheCmd( );
    
    void            displayTLBCmd( );
    void            insertTLBCmd( );
    void            purgeTLBCmd( );
    
    void            winOnCmd( );
    void            winOffCmd( );
    void            winDefCmd( );
    void            winStacksEnableCmd( bool enable );
    
    void            winCurrentCmd( );
    void            winEnableCmd( bool enable );
    void            winSetRadixCmd( );
    
    void            winForwardCmd( );
    void            winBackwardCmd( );
    void            winHomeCmd( );
    void            winJumpCmd( );
    void            winSetRowsCmd( );
    void            winSetCmdWinRowsCmd( );
    void            winClearCmdWinCmd( ); 
    void            winNewWinCmd( );
    void            winKillWinCmd( );
    void            winSetStackCmd( );
    void            winToggleCmd( );
    void            winExchangeCmd( );

private:
    
    SimGlobals              *glb        = nullptr;
    SimCmdHistory           *hist       = nullptr;
    SimTokenizerFromString  *tok        = nullptr;
    SimExprEvaluator        *eval       = nullptr;
    SimWinOutBuffer         *winOut     = nullptr;
    T64Assemble             *inlineAsm  = nullptr;
    T64DisAssemble          *disAsm     = nullptr;   
    SimTokId                currentCmd  = TOK_NIL;
};

//----------------------------------------------------------------------------------------
// The window display screen object is the central object of the simulator. Commands
// send from the command input will eventually end up as calls to this object. A 
// simulator screen is an ordered list of windows. Although you can disable a window
// such that it disappears on the screen, when enabled, it will show up in the place
// intended for it. For example, the program state register window will always be on
// top, followed by the special regs. The command input scroll area is always last and
// is the only window that cannot be disabled. In addition, windows can be grouped in
// stacks that are displayed next to each other. The exception is the command window
// area which is always displayed across the entire terminal window width.
//
//----------------------------------------------------------------------------------------
struct SimWinDisplay {
    
public:
    
    SimWinDisplay( SimGlobals *glb );
    
    void            setupWinDisplay( );
    void            startWinDisplay( );
    SimTokId        getCurrentCmd( );

    void            setWinMode( bool winOn );
    bool            isWinModeOn( );
    
    void            setWinReFormat( );
    bool            isWinReFormat( );
    void            reDraw( );
    
    void            windowsOn( );
    void            windowsOff( );
    void            windowDefaults( int winNumStart, int winNumEnd );
    void            windowCurrent( int winNum );
    void            windowEnable( int winNumStart, int winNumEnd, bool enable );
    void            winStacksEnable( int stackNum, bool enable );
    void            windowRadix( int rdx, int winNum );
    void            windowSetRows( int rows, int winNum );
    void            windowSetCmdWinRows( int rows );
    void            windowClearCmdWin( );
    
    void            windowHome( int amt, int winNum );
    void            windowForward( int amt, int winNum );
    void            windowBackward( int amt, int winNum );
    void            windowJump( int amt, int winNum );
    void            windowToggle( int winNum, int toggleVal );
    void            windowExchangeOrder( int winNum );
    
    void            windowNewAbsMem( int modNum, T64Word adr );
    void            windowNewAbsCode( int modNum, T64Word adr );
    void            windowNewCpuState( int modNum );
    void            windowNewTlb( int modNum, T64TlbKind tTyp );
    void            windowNewCache( int modNum, T64CacheKind cTyp );
    void            windowNewText( char *pathStr );

    void            windowKill( int winNumStart, int winNumEnd );
    void            windowKillByModNum( int modNum );
    void            windowSetStack( int winStack, int winNumStart, int winNumEnd );
    
    int             getCurrentWindow( );
    void            setCurrentWindow( int winNum );
    bool            isCurrentWin( int winNum );
    SimWinType      getCurrentWinType( );
    int             getCurrentWinModNum( );
    bool            isScrollableWin ( int winNum );
    bool            isWinEnabled( int winNum );
    bool            isWindowsOn( );

    bool            validWindowType( SimTokId winType );
    bool            validWindowNum( int winNum );
    bool            validWindowStackNum( int winNum );

    char            *getWinName( int winNum );
    int             getWinStackNum( int winNum );
    char            *getWinTypeName( int winNum );  
    int             getWinModNum( int winNum );

    private:
    
    int             getFreeWindowSlot( );
    int             computeColumnsNeeded( int winStack );
    int             computeRowsNeeded( int winStack );
    void            setWindowColumns( int winStack, int columns );
    void            setWindowOrigins( int winStack, int rowOfs = 1, int colOfs = 1 );
   
    int             currentWinNum                   = -1;
    int             previousWinNum                  = -1;
    bool            winModeOn                       = true;
    bool            winReFormatPending              = false;

    SimGlobals      *glb                            = nullptr;
    SimWin          *windowList[ MAX_WINDOWS ]      = { nullptr };

    public: 

    SimCommandsWin  *cmdWin                         = nullptr;
    
};

//----------------------------------------------------------------------------------------
// The globals, accessible to all objects. To ease the passing around there is the
// idea a global structure with a reference to all the individual objects.
//
//----------------------------------------------------------------------------------------
struct SimGlobals {
    
    SimConsoleIO        *console        = nullptr;
    SimEnv              *env            = nullptr;
    SimWinDisplay       *winDisplay     = nullptr;
    T64System           *system         = nullptr;

    bool                verboseFlag                             = false;
    char                configFileName[ MAX_FILE_PATH_SIZE ]    = { 0 };
    char                logFileName[ MAX_FILE_PATH_SIZE ]       = { 0 };
};

//----------------------------------------------------------------------------------------
// Our entry into parsing program command line options.
//
//----------------------------------------------------------------------------------------
void processCmdLineOptions( SimGlobals *glb, int argc, char *argv[ ] );
