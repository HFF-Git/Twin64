//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command window
//
//----------------------------------------------------------------------------------------
// The command window is the last screen area below all enabled windows displayed.
// It is actually not a window like the others in that it represents lines written
// to the window as well as the command input line. It still has a window header
// and a line drawing area. To enable scrolling of this window, an output buffer
// needs to be implemented that stores all output in a circular buffer to use for
// text output. Just like a "real" terminal. The cursor up and down keys will 
// perform the scrolling. The command line is also a bit special. It is actually
// the one line locked scroll area. Input can be edited on this line, a carriage
// return will append the line to the output buffer area.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command window
// Copyright (C) 2025 - 2025 Helmut Fieres
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
#include "T64-SimDeclarations.h"
#include "T64-SimTables.h"

//----------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// We think outside in window numbers starting at one. Internally, there is an array
// of windows starting at zero. When parsing a command, we map right there. A valid
// window number from 1 to MAX is mapped, all other numbers are mapped to -1.
// 
//----------------------------------------------------------------------------------------
int internalWinNum( int num ) {

    if (( num > 0 ) && ( num <= MAX_WINDOWS )) return( num - 1 );
    else return ( -1 );
}

//----------------------------------------------------------------------------------------
// Little helper functions.
//
//----------------------------------------------------------------------------------------
bool isEscapeChar( int ch ) {
    
    return ( ch == 27 );
}

bool isWInSpecialChar( int ch ) {
    
    return ( ch == 0xe0 );
}

bool isCarriageReturnChar( int ch ) {
    
    return (( ch == '\n' ) || ( ch == '\r' ));
}

bool isBackSpaceChar( int ch ) {
    
    return (( ch == 8 ) || ( ch == 127 ));
}

bool isLeftBracketChar( int ch ) {
    
    return ( ch == '[' );
}

//----------------------------------------------------------------------------------------
// Trim trailing blanks from a string. 
//
//
//----------------------------------------------------------------------------------------
void rtrim( char *s ) {

    char *end = s + strlen( s );
    while (( end > s ) && ( isspace((unsigned char) *(end - 1)))) end--;
    *end = '\0';
}

//----------------------------------------------------------------------------------------
// A little helper function to remove the comment part of a command line. We do 
// the changes on the buffer passed in by just setting the end of string at the
// position of the "#" comment indicator. A "#" inside a string is ignored.
//
//----------------------------------------------------------------------------------------
int removeComment( char *cmdBuf ) {
    
    if ( strlen ( cmdBuf ) > 0 ) {
        
        char *ptr = cmdBuf;
        
        bool inQuotes = false;
        
        while ( *ptr ) {
            
            if ( *ptr == '"' ) {
                
                inQuotes = ! inQuotes;
            }
            else if ( *ptr == '#' && !inQuotes ) {
                
                *ptr = '\0';
                break;
            }
            
            ptr++;
        }
    }
    
    return ((int) strlen( cmdBuf ));
}

//----------------------------------------------------------------------------------------
// "removeChar" will removes the character from the input buffer left of the 
// cursor position and adjust the input buffer string size accordingly. If the 
// cursor is at the end of the string, both string size and cursor position are
// decremented by one.
//
//----------------------------------------------------------------------------------------
void removeChar( char *buf, int *strSize, int *pos ) {
    
    if (( *strSize > 0 ) && ( *strSize == *pos )) {
        
        *strSize        = *strSize - 1;
        *pos            = *pos - 1;
        buf[ *strSize ] = '\0';
    }
    else if (( *strSize > 0 ) && ( *pos > 0 )) {
        
        for ( int i = *pos; i < *strSize; i++ ) buf[ i ] = buf[ i + 1 ];
        
        *strSize        = *strSize - 1;
        *pos            = *pos - 1;
        buf[ *strSize ] = '\0';
    }
}

//----------------------------------------------------------------------------------------
// "insertChar" will insert a character into the input buffer at the cursor 
// position and adjust cursor and overall string size accordingly. There are two
// basic cases. The first is simply appending to the buffer when both current 
// string size and cursor position are equal. The second is when the cursor is
// somewhere in the input buffer. In this case we need to shift the characters 
// to the right to make room first.
//
//----------------------------------------------------------------------------------------
void insertChar( char *buf, int ch, int *strSize, int *pos ) {
    
    if ( *pos == *strSize ) {
        
        buf[ *strSize ] = ch;
    }
    else if ( *pos < *strSize ) {
        
        for ( int i = *strSize; i > *pos; i-- ) buf[ i ] = buf[ i - 1 ];
        buf[ *pos ] = ch;
    }
    
    *strSize        = *strSize + 1;
    *pos            = *pos + 1;
    buf[ *strSize ] = '\0';
}

//----------------------------------------------------------------------------------------
// Line sanitizing. We cannot just print out whatever is in the line buffer, since 
// it may contains dangerous escape sequences, which would garble our terminal 
// screen layout. In the command window we just allow "safe" escape sequences, 
// such as changing the font color and so on. When we encounter an escape character
// followed by a "[" character we scan the escape sequence until the final character,
// which lies between 0x40 and 0x7E. Based on the last character, we distinguish 
// between "safe" and "unsafe" escape sequences. In the other cases, we just copy
// input to output.
//
//----------------------------------------------------------------------------------------
bool isSafeFinalByte( char finalByte ) {
    
    //Example:  m = SGR (color/formatting), others can be added
    return finalByte == 'm';
}

void sanitizeLine( const char *inputStr, char *outputStr ) {
    
    const char  *src = inputStr;
    char        *dst = outputStr;
    
    while ( *src ) {
        
        if ( *src == '\x1B' ) {
            
            if ( *( src + 1 ) == '\0' ) {
                
                *dst++ = *src++;
            }
            else if ( *( src + 1 ) == '[' ) {
                
                const char *escSeqStart = src;
                src += 2;
                
                while (( *src ) && ( ! ( *src >= 0x40 && *src <= 0x7E ))) src++;
                
                if ( *src ) {
                    
                    char finalByte = *src++;
                    
                    if ( isSafeFinalByte( finalByte )) {
                        
                        while ( escSeqStart < src ) *dst++ = *escSeqStart++;
                        
                    } else continue;
                    
                } else break;
                
            } else *dst++ = *src++;
            
        } else *dst++ = *src++;
    }
    
    *dst = '\0';
}

}; // namespace


//****************************************************************************************
//****************************************************************************************
//
// Object methods - SimCmdHistory
//
//----------------------------------------------------------------------------------------
// The simulator command interpreter features a simple command history. It is a 
// circular buffer that holds the last commands. There are functions to show the 
// command history, re-execute a previous command and to retrieve a previous 
// command for editing. The command stack can be accessed with relative command 
// numbers, i.e. "current - 3" or by absolute command number, when still present
// in the history stack.
//
//----------------------------------------------------------------------------------------
SimCmdHistory::SimCmdHistory( ) {
    
    this -> head    = 0;
    this -> tail    = 0;
    this -> count   = 0;
}

//----------------------------------------------------------------------------------------
// Add a command line. If the history buffer is full, the oldest entry is re-used. 
// The head index points to the next entry for allocation.
//
//----------------------------------------------------------------------------------------
void SimCmdHistory::addCmdLine( char *cmdStr ) {
    
    SimCmdHistEntry *ptr = &history[ head ];
    
    ptr -> cmdId = nextCmdNum;
    strncpy( ptr -> cmdLine, cmdStr, 256 );
    
    if ( count == MAX_CMD_HIST ) tail = ( tail + 1 ) % MAX_CMD_HIST;
    else count++;
    
    nextCmdNum ++;
    head = ( head + 1 ) % MAX_CMD_HIST;
}

//----------------------------------------------------------------------------------------
// Get a command line from the command history. If the command reference is 
// negative, the entry relative to the top is used. "head - 1" refers to the last
// entry entered. If the command ID is positive, we search for the entry with the 
// matching command id, if still in the history buffer. Optionally, we return the
// absolute command Id.
//
//----------------------------------------------------------------------------------------
char *SimCmdHistory::getCmdLine( int cmdRef, int *cmdId ) {
    
    if (( cmdRef >= 0 ) && (( nextCmdNum - cmdRef ) > MAX_CMD_HIST ))
         return ( nullptr );

    if (( cmdRef < 0  ) && ( - cmdRef > nextCmdNum )) return ( nullptr );
    
    if ( count == 0 ) return ( nullptr );
    
    if ( cmdRef >= 0 ) {
        
        for ( int i = 0; i < count; i++ ) {
            
            int pos = ( tail + i ) % MAX_CMD_HIST;
            if ( history[ pos ].cmdId == cmdRef ) {
                
                if ( cmdId != nullptr ) *cmdId = history[ pos ].cmdId;
                return ( history[ pos ].cmdLine );
            }
        }
        
        return ( nullptr );
    }
    else {
        
        int pos = ( head + cmdRef + MAX_CMD_HIST ) % MAX_CMD_HIST;
        
        if (( pos < head ) && ( pos >= tail )) {
            
            if ( cmdId != nullptr ) *cmdId = history[ pos ].cmdId;
            return history[ pos ].cmdLine;
        }
        else return ( nullptr );
    }
}

//----------------------------------------------------------------------------------------
// The command history maintains a command counter and number, which we return here.
//
//----------------------------------------------------------------------------------------
int SimCmdHistory::getCmdNum( ) {
    
    return ( nextCmdNum );
}

int  SimCmdHistory::getCmdCount( ) {
    
    return ( count );
}

//****************************************************************************************
//****************************************************************************************
//
// Object methods - SimCommandsWin
//
//----------------------------------------------------------------------------------------
// Object constructor.
//
//----------------------------------------------------------------------------------------
SimCommandsWin::SimCommandsWin( SimGlobals *glb ) : SimWin( glb ) {
    
    this -> glb = glb;
    
    tok        = new SimTokenizerFromString( );
    eval        = new SimExprEvaluator( glb, tok );
    hist        = new SimCmdHistory( );
    winOut      = new SimWinOutBuffer( );
    disAsm      = new T64DisAssemble( );
    inlineAsm   = new T64Assemble( );

    setDefaults( );
}

//----------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first
// time, or for the WDEF command.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::setDefaults( ) {
    
    setWinType( WT_CMD_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 1 );
    setWinDefSize( 0, 24, 100 );
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// The banner line for command window. For now, we just label the banner line and 
// show the system state plus the WIN mode stack info.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE | FMT_UNDER_LINE;

    setWinCursor( 1, 1 );
    printTextField((char *) "Commands", ( fmtDesc | FMT_ALIGN_LFT ), 32 );

    printTextField((char *) "System State: ", fmtDesc );
    printNumericField( glb -> system -> getSystemState( ), fmtDesc | FMT_HEX_4 );
    padLine( fmtDesc ); 

    if ( glb -> winDisplay -> isWindowsOn( )) {

        printStackInfoField( fmtDesc | FMT_LAST_FIELD );
    }
}

//----------------------------------------------------------------------------------------
// The body lines of the command window are displayed after the banner line. The 
// window is filled from the output buffer. We first set the screen lines as the
// length of the command window may have changed.
//
// Rows to show is the number of lines between the header line and the last line,
// which is out command input line. We fill from the lowest line upward to the 
// header line. Finally, we set the cursor to the last line in the command window.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::drawBody( ) {
    
    char lineOutBuf[ MAX_WIN_OUT_LINE_SIZE ];
    
    glb -> console ->setFmtAttributes( FMT_DEF_ATTR );
  
    int rowsToShow = getRows( ) - 2;
    winOut -> setScrollWindowSize( rowsToShow );
    setWinCursor( rowsToShow + 1, 1 );
    
    for ( int i = 0; i < rowsToShow; i++ ) {
        
        char *lineBufPtr = winOut -> getLineRelative( i );
        if ( lineBufPtr != nullptr ) {
            
            sanitizeLine( lineBufPtr, lineOutBuf );
            glb -> console -> clearLine( );
            glb -> console -> writeChars( "%s", lineBufPtr );
        }
        
        setWinCursor( rowsToShow - i, 1 );
    }
    
    setWinCursor( getRows( ), 1 );
}

//----------------------------------------------------------------------------------------
// Clear the command window output buffer.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::clearCmdWin( ) {
    
    winOut -> initBuffer( );
}

//----------------------------------------------------------------------------------------
// "readCmdLine" is used by the command line interpreter to get the command. Since 
// we run in raw mode, the basic handling of backspace, carriage return, relevant
// escape sequences, etc. needs to be processed in this routine directly. 
// Characters other than the special characters are piled up in a local buffer 
// until we read in a carriage return. The core is a state machine that examines 
// a character read to analyze whether this is a special character or sequence. 
// Any "normal" character is just added to the line buffer. The states are as 
// follows:
//
//      CT_NORMAL: got a character, analyze it.
//      CT_ESCAPE: check the characters got. If a "[" we handle an escape sequence.
//      CT_ESCAPE_BRACKET: analyze the argument after "esc[" input got so far.
//      CT_WIN_SPECIAL: analyze a MS windows special character.
//
// A carriage return character will append a zero to the command line input got 
// so far. We check wether the command ended with a "\" in which case we have a 
// multi-line input. A special prompt is displayed and we keep reading in command
// lines. After the final carriage return, we are done reading the input line. 
//
// The prompt and the command string along with a carriage return are appended to
// the command output buffer. Before returning to the caller, the last thing to do
//is to remove any comment from the line.
//
// The left and right arrows move the cursor in the command line. Backspacing and
// inserting will then take place at the current cursor position shifting any 
// content to the right of the cursor when inserting and shifting to the left 
// when deleting.
//
// On MS windows a special character indicates the start of a special button pressed. 
// We currently recognize only the cursor keys.
//
// We also have the option of a prefilled command buffer for editing a command line 
// before hitting return. This option is used by the REDO command which lists a 
// previously entered command presented for editing.
//
// Finally, there is the cursor up and down key. These keys are used to scroll the 
// command line window. This is the case where we need to get lines from the output 
// buffer to fill from top or bottom of the command window display. We also need 
// to ensure that when a new command line is read in, we are with our cursor at 
// the input line, right after the prompt string.
//
// ??? command line editing is still an issue when using cursors...
//----------------------------------------------------------------------------------------
int SimCommandsWin::readCmdLine( char *cmdBuf, int initialCmdBufLen, char *promptBuf ) {
    
    enum CharType : uint16_t { CT_NORMAL, CT_ESCAPE, CT_ESCAPE_BRACKET, CT_WIN_SPECIAL };
    
    int         promptBufLen    = (int) strlen( promptBuf );
    int         cmdBufCursor    = 0;
    int         cmdBufLen       = 0;
    int         ch              = ' ';
    CharType    state           = CT_NORMAL;
    
    if (( promptBufLen > 0 ) && ( glb -> console -> isConsole( ))) {
        
        promptBufLen =  glb -> console -> writeChars( " " );
        promptBufLen += glb -> console -> writeChars( "%s", promptBuf );
    }
    
    if ( initialCmdBufLen > 0 ) {
        
        cmdBuf[ initialCmdBufLen ]  = '\0';
        cmdBufLen                   = initialCmdBufLen;
        cmdBufCursor                = initialCmdBufLen;
    }
    else cmdBuf[ 0 ] = '\0';
    
    while ( true ) {
        
        ch = glb -> console -> readChar( );
        
        switch ( state ) {
                
            case CT_NORMAL: {
                
                if ( isEscapeChar( ch )) {
                    
                    state = CT_ESCAPE;
                }
                else if ( isWInSpecialChar( ch )) {
                    
                    state = CT_WIN_SPECIAL;
                }
                else if ( isCarriageReturnChar( ch )) {

                    if ( cmdBufLen > 0 && cmdBuf[ cmdBufLen - 1 ] == '\\' ) {

                        cmdBufLen--;
                        cmdBuf[ cmdBufLen ] = '\0';

                        glb -> console -> writeCarriageReturn( );

                        if ( glb -> console -> isConsole( ) ) {

                            glb->console -> writeChars( ">>" ); 
                            promptBufLen = 2;                       
                        }

                        cmdBufCursor = cmdBufLen;
                        promptBufLen = 2;
                    }
                    else {
                  
                        cmdBuf[ cmdBufLen ] = '\0';
                        glb -> console -> writeCarriageReturn();

                        winOut -> addToBuffer( promptBuf );
                        winOut -> addToBuffer( cmdBuf );
                        winOut -> addToBuffer( "\n" );
                        cmdBufLen = removeComment( cmdBuf );
                        return ( cmdBufLen );
                    }
                }
                else if ( isBackSpaceChar( ch )) {
                    
                    if ( cmdBufLen > 0 ) {
                        
                        removeChar( cmdBuf, &cmdBufLen, &cmdBufCursor );
                        
                        glb -> console -> eraseChar( );
                        glb -> console -> writeCursorLeft( );
                        glb -> console -> writeChars( "%c", cmdBuf[ cmdBufCursor ] );
                    }
                }
                else {
                   
                    if ( cmdBufLen < MAX_CMD_LINE_SIZE - 1 ) {
                       
                        insertChar( cmdBuf, ch, &cmdBufLen, &cmdBufCursor );
                        
                        if ( isprint( ch )) {

                            glb -> console -> writeChars( "%c", ch );
                        }
                    }
                }

            } break;
                
            case CT_ESCAPE: {
                
                if ( isLeftBracketChar( ch )) state = CT_ESCAPE_BRACKET;
                else                          state = CT_NORMAL;
                
            } break;
                
            case CT_ESCAPE_BRACKET: {
                
                switch ( ch ) {
                        
                    case 'D': {
                        
                        if ( cmdBufCursor > 0 ) {
                            
                            cmdBufCursor --;
                            glb -> console -> writeCursorLeft( );
                        }
                        
                    } break;
                        
                    case 'C': {
                        
                        if ( cmdBufCursor < cmdBufLen ) {
                            
                            cmdBufCursor ++;
                            glb -> console -> writeCursorRight( );
                        }
                        
                    } break;
                        
                    case 'A': {
                        
                        winOut -> scrollUp( );
                        reDraw( );
                        setWinCursor( 0, promptBufLen );
                        
                    } break;
                        
                    case 'B': {
                        
                        winOut -> scrollDown( );
                        reDraw( );
                        setWinCursor( 0, promptBufLen  );
                        
                    } break;
                        
                    default: ;
                }
                
                state = CT_NORMAL;
                
            } break;
                
            case CT_WIN_SPECIAL: {
                
                switch ( ch ) {
                        
                    case 'K': {
                        
                        if ( cmdBufCursor > 0 ) {
                            
                            cmdBufCursor --;
                            glb -> console -> writeCursorLeft( );
                        }
                        
                    } break;
                        
                    case 'M' : {
                        
                        if ( cmdBufCursor < cmdBufLen ) {
                            
                            cmdBufCursor ++;
                            glb -> console -> writeCursorRight( );
                        }
                        
                    } break;
                        
                    case 'H' : {
                        
                        winOut -> scrollUp( );
                        reDraw( );
                        setWinCursor( 0, promptBufLen );
                        
                    } break;
                        
                    case 'P': {
                        
                        winOut -> scrollDown( );
                        reDraw( );
                        setWinCursor( 0, promptBufLen  );
                        
                    } break;
                        
                    default: ;
                }
                
                state = CT_NORMAL;
                
            } break;
        }
    }
}

//----------------------------------------------------------------------------------------
// "commandLineError" is a little helper that prints out the error encountered.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::cmdLineError( SimErrMsgId errNum, char *argStr ) {
    
    for ( int i = 0; i < MAX_ERR_MSG_TAB; i++ ) {
        
        if ( errMsgTab[ i ].errNum == errNum ) {
            
            winOut -> writeChars( "%s\n", errMsgTab[ i ].errStr );
            return;
        }
    }
    
    winOut -> writeChars( "CmdLine Error: %d", errNum );
    if ( argStr != nullptr )  winOut -> writeChars( "%32s", argStr );
    winOut -> writeChars( "\n" );
}

//----------------------------------------------------------------------------------------
// "promptYesNoCancel" is a simple function to print a prompt string with a decision
// question. The answer can be yes/no or cancel. A positive result is a "yes" a 
// negative result a "no", anything else a "cancel".
//
//----------------------------------------------------------------------------------------
int SimCommandsWin::promptYesNoCancel( char *promptStr ) {
    
    char buf[ 256 ] = "";
    int  ret        = 0;
    
    if ( readCmdLine( buf, 0, promptStr ) > 0 ) {
        
        if      (( buf[ 0 ] == 'Y' ) ||  ( buf[ 0 ] == 'y' ))   ret = 1;
        else if (( buf[ 0 ] == 'N' ) ||  ( buf[ 0 ] == 'n' ))   ret = -1;
        else                                                    ret = 0;
    }
    else ret = 0;
    
    winOut -> writeChars( "%s\n", buf );
    return ( ret );
}

//----------------------------------------------------------------------------------------
// "configureT64Sim" configures the simulator system. If there is a config file 
// specified, we read it in and process the commands found there. This way, we
// can set up  the simulator environment before entering the command loop.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::configureT64Sim( ) {     
    
    if ( glb -> console -> isConsole( )) {
        
        winOut -> writeChars( "Configuring Twin-64 Simulator...\n" );
       
        // ??? just use execute file ...
        // ??? if fails write to log ?

        winOut -> writeChars( "Configuration done.\n\n" );
    }
}

//----------------------------------------------------------------------------------------
// A little helper function to ensure that windows are enabled.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::ensureWinModeOn( ) {

    if ( ! glb -> winDisplay -> isWindowsOn( )) throw( ERR_NOT_IN_WIN_MODE );
}

//----------------------------------------------------------------------------------------
// Return the current command token entered.
//
//----------------------------------------------------------------------------------------
SimTokId SimCommandsWin::getCurrentCmd( ) {
    
    return ( currentCmd );
}

//----------------------------------------------------------------------------------------
// Print the stack info data. In the command window line, we will have a field to
// the very right, which is on when we are in windows mode. It will show which 
// stacks are current used, regardless whether they a visible or not.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::printStackInfoField( uint32_t fmtDesc, int row, int col ) {

    int  stacks[ MAX_WIN_STACKS ] = { 0 };
    char stackStr[ 16 ]           = { 0 };
    int  stackStrLen              = 0;

    if ( ! glb -> winDisplay -> isWindowsOn( )) return;

    for ( int i = 0; i < MAX_WINDOWS; i++ ) {

        int stackNum = glb -> winDisplay -> getWinStackNum( i );
        if ( stackNum >= 0 ) stacks[ stackNum ] ++;
    }

    for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {

        if ( stacks[ i ] > 0 ) { 

            stackStrLen = snprintf( stackStr, 4, "S:" );
            break;
        }
    }

    for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {
        
        if ( stacks[ i ] > 0 ) {

            stackStrLen += snprintf( stackStr + stackStrLen, 4, "%d", i + 1 );
        }
    }

    glb -> console -> setFmtAttributes( fmtDesc );
    printTextField( stackStr, fmtDesc, stackStrLen, row, col );
}

//----------------------------------------------------------------------------------------
// Our friendly welcome message with the actual program version. We also set some of the
// environment variables to an initial value. Especially string variables need to be set 
// as they are not initialized from the environment variable table.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::printWelcome( ) {
    
    glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, (T64Word) 0 );
    
    if ( glb -> console -> isConsole( )) {
        
        winOut -> writeChars( "Twin-64 Simulator, Version: %s, Patch Level: %d\n",
                              glb -> env -> getEnvVarStr((char *) ENV_PROG_VERSION ),
                              glb -> env -> getEnvVarStr((char *) ENV_PATCH_LEVEL ));
        
        winOut -> writeChars( "Git Branch: %s\n",
                              glb -> env -> getEnvVarStr((char *) ENV_GIT_BRANCH ));

        if ( glb -> verboseFlag ) {

            if ( strlen( glb -> configFileName ) > 0 ) {
                
                winOut -> writeChars( "Config File: %s\n", glb -> configFileName );
            }

            if ( strlen( glb -> logFileName ) > 0 ) {
                
                winOut -> writeChars( "Log File: %s\n", glb -> logFileName );
            }
        }
        
        winOut -> writeChars( "\n" );
    }
}

//----------------------------------------------------------------------------------------
// "promptCmdLine" lists out the prompt string.
//
//----------------------------------------------------------------------------------------
int SimCommandsWin::buildCmdPrompt( char *promptStr, int promptStrLen ) {
    
    if ( glb -> env -> getEnvVarBool((char *) ENV_SHOW_CMD_CNT )) {
            
        return ( snprintf( promptStr, promptStrLen,
                           "(%i) ->",
                           (int) glb -> env -> getEnvVarInt((char *) ENV_CMD_CNT )));
        }
    else return ( snprintf( promptStr, promptStrLen, "->" ));
}

//----------------------------------------------------------------------------------------
// "addProcModule" parses the parameters for a PROC module. We are entered with 
// the token being the module type keyword. The routine loops over the key/value
// pairs to get all module type info. Omitted key/value pairs are set to reasonable
// defaults.
//
//  NM proc, num, ITLB=xxx, DTLB=xxx, ...
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::addProcModule( ) {

    int          modNum     = -1;
    T64TlbType   iTlbType   = T64_TT_FA_64S; 
    T64TlbType   dTlbType   = T64_TT_FA_64S; 
    T64CacheType iCacheType = T64_CT_2W_128S_4L;
    T64CacheType dCacheType = T64_CT_4W_128S_4L;
    
    tok -> nextToken( );
    while ( tok -> isToken( TOK_COMMA )) {

        tok -> nextToken( );

        switch ( tok -> tokId( )) {

            case TOK_MOD: {

                tok -> nextToken( );
                tok -> acceptEqual( );
                if ( tok -> tokTyp( ) == TYP_NUM ) {

                    modNum = eval -> acceptNumExpr( ERR_INVALID_ARG, 
                                                    0, MAX_MODULES );
                }
                else throw( ERR_INVALID_ARG );

            } break;

            case TOK_ITLB: {

                tok -> nextToken( );
                tok -> acceptEqual( );

                if ( tok -> isToken( TOK_TLB_FA_64S )) 
                    iTlbType = T64_TT_FA_64S;
                else if ( tok -> isToken( TOK_TLB_FA_128S )) 
                    iTlbType = T64_TT_FA_128S;
                else throw( ERR_INVALID_ARG );

            } break;

            case TOK_DTLB: {

                tok -> nextToken( );
                tok -> acceptEqual( );

                if ( tok -> isToken( TOK_TLB_FA_64S )) 
                    dTlbType = T64_TT_FA_64S;
                else if ( tok -> isToken( TOK_TLB_FA_128S )) 
                    dTlbType = T64_TT_FA_128S;
                else throw( ERR_INVALID_ARG );
      
            } break;

            case TOK_ICACHE: {

                tok -> nextToken( );
                tok -> acceptEqual( );

                switch ( tok -> tokId( )) {

                    case TOK_CACHE_SA_2W_128S_4L: {
                        
                        iCacheType = T64_CT_2W_128S_4L; 
                    
                    } break;

                    case TOK_CACHE_SA_4W_128S_4L: {
                        
                        iCacheType = T64_CT_4W_128S_4L; 
                    
                    } break;

                    case TOK_CACHE_SA_8W_128S_4L: {
                        
                        iCacheType = T64_CT_8W_128S_4L; 
                    
                    } break;

                    case TOK_CACHE_SA_2W_64S_8L: {
                        
                        iCacheType = T64_CT_2W_64S_8L; 
                    
                    } break;

                    case TOK_CACHE_SA_4W_64S_8L: {
                        
                        iCacheType = T64_CT_4W_64S_8L; 
                    
                    } break;

                    case TOK_CACHE_SA_8W_64S_8L: {
                        
                        iCacheType = T64_CT_8W_64S_8L; 
                    
                    } break;
                
                    default: throw( ERR_INVALID_ARG );
                }

            } break;

            case TOK_DCACHE: {

                tok -> nextToken( );
                tok -> acceptEqual( );

                switch ( tok -> tokId( )) {

                    case TOK_CACHE_SA_2W_128S_4L: {
                        
                        dCacheType = T64_CT_2W_128S_4L; 
                    
                    } break;

                    case TOK_CACHE_SA_4W_128S_4L: {
                        
                        dCacheType = T64_CT_4W_128S_4L; 
                    
                    } break;

                    case TOK_CACHE_SA_8W_128S_4L: {
                        
                        dCacheType = T64_CT_8W_128S_4L; 
                    
                    } break;

                    case TOK_CACHE_SA_2W_64S_8L: {
                        
                        dCacheType = T64_CT_2W_64S_8L; 
                    
                    } break;

                    case TOK_CACHE_SA_4W_64S_8L: {
                        
                        dCacheType = T64_CT_4W_64S_8L; 
                    
                    } break;

                    case TOK_CACHE_SA_8W_64S_8L: {
                        
                        dCacheType = T64_CT_8W_64S_8L; 
                    
                    } break;
                
                    default: throw( ERR_INVALID_ARG );
                }

            } break;

            default: throw( ERR_INVALID_MODULE_TYPE );
        }

        tok -> nextToken( );
    }

    tok -> checkEOS( );

    if ( modNum == -1 ) throw( SimErrMsgId( ERR_EXPECTED_MOD_NUM ));

    T64Processor *p = new T64Processor( glb -> system,
                                        modNum,
                                        T64_PO_NIL,
                                        T64_CPU_T_NIL,
                                        iTlbType,
                                        dTlbType,
                                        iCacheType,
                                        dCacheType,
                                        0,
                                        0 );
                    
    if ( glb -> system -> addToModuleMap( p ) != 0 ) {

        delete p;
        throw( SimErrMsgId( ERR_CREATE_PROC_MODULE ));
    }    
}

//----------------------------------------------------------------------------------------
// "addMemModule" parses the parameters for a memory module. We are entered with 
// the token being the module type keyword. The routine loops over the key/value
// pairs to get all module type info. Omitted key/value pairs are set to reasonable
// defaults.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::addMemModule( ) {

    int         modNum  = -1;
    T64MemType  mType   = T64_MT_RAM;   
    T64Word     spaAdr  = 0;
    T64Word     spaLen  = 0;

    tok -> nextToken( );
    while ( tok -> isToken( TOK_COMMA )) {

        tok -> nextToken( );

        switch ( tok -> tokId( )) {

            case TOK_MOD: {

                tok -> nextToken( );
                tok -> acceptEqual( );
                if ( tok -> tokTyp( ) == TYP_NUM ) {

                    modNum = eval -> acceptNumExpr( ERR_INVALID_ARG, 
                                                    0, MAX_MODULES );
                }
                else throw( ERR_INVALID_ARG );
                
            } break;

            case TOK_MEM: {

                tok -> nextToken( );
                tok -> acceptEqual( );

                if ( tok -> isToken( TOK_MEM_READ_ONLY )) {

                    mType = T64_MT_ROM;
                }
                else if ( tok -> isToken( TOK_MEM_READ_WRITE )) {

                    mType = T64_MT_RAM;
                }
                else throw( ERR_INVALID_ARG );

            } break;

            case TOK_MOD_SPA_ADR: {

                tok -> nextToken( );
                tok -> acceptEqual( );

                if ( tok -> tokTyp( ) == TYP_NUM ) {

                    spaAdr = eval -> acceptNumExpr( ERR_INVALID_ARG, 
                                                    0, UINT32_MAX );
                }
                else throw( ERR_INVALID_ARG );

            } break;

            case TOK_MOD_SPA_LEN: {

                tok -> nextToken( );
                tok -> acceptEqual( );

                if ( tok -> tokTyp( ) == TYP_NUM ) {

                    spaLen = eval -> acceptNumExpr( ERR_INVALID_ARG, 
                                                    0, UINT32_MAX );
                }
                else throw( ERR_INVALID_ARG );

            } break;

            default: throw( ERR_INVALID_MODULE_TYPE );
        }

        tok -> nextToken( );
    }

    tok -> checkEOS( );

    if ( modNum == -1 ) throw( SimErrMsgId( ERR_EXPECTED_MOD_NUM ));

    T64Memory *m = new T64Memory( glb -> system,
                                  modNum, 
                                  T64_MK_NIL,
                                  mType, 
                                  spaAdr,
                                  spaLen );

    if ( glb -> system -> addToModuleMap( m ) != 0 ) {

        delete m;
        throw( SimErrMsgId( ERR_CREATE_MEM_MODULE )); 
    }    
}

//----------------------------------------------------------------------------------------
// "addIoModule" parses the parameters for an IO module. We are entered with the
// token being the module type keyword. The routine loops over the key/value pairs
// to get all module type info. Omitted key/value pairs are set to reasonable
// defaults.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::addIoModule( ) {

    // ??? analog to mem...

}

//----------------------------------------------------------------------------------------
// Display absolute memory content. We will show the memory starting with offset. 
// The words per line is an environmental variable setting. The offset is rounded 
// down to the next 8-byte boundary, the limit is rounded up to the next 8-byte 
// boundary. We display the data in words.
//
//----------------------------------------------------------------------------------------
void  SimCommandsWin::displayAbsMemContent( T64Word ofs, T64Word len, int rdx ) {
    
    T64Word index        = rounddown( ofs, sizeof( T64Word ));
    T64Word limit        = roundup(( index + len ), sizeof( T64Word ));
    int     wordsPerLine = 4;

    while ( index < limit ) {

        winOut -> printNumber( index, FMT_HEX_2_4_4 );
        winOut -> writeChars( ": " );
        
        for ( uint32_t i = 0; i < wordsPerLine; i++ ) {
            
            if ( index < limit ) {

                T64Word val = 0;
                if ( glb -> system -> readMem( index, 
                                               (uint8_t *) &val, 
                                               sizeof( val ))) {

                    if ( rdx == 16 )
                        winOut -> printNumber( val, FMT_HEX_4_4_4_4 );
                    else if ( rdx == 10 )
                        winOut -> printNumber( val, FMT_DEC_32 );
                    else 
                        winOut -> printNumber( val, FMT_INVALID_NUM | FMT_HEX_4_4_4_4 );

                    winOut -> writeChars( " " );   
                }
                else {

                    winOut -> printNumber( val, FMT_INVALID_NUM | FMT_HEX_4_4_4_4 );
                    winOut -> writeChars( " " );  
                }           
            }
            
            winOut -> writeChars( " " );
            index += sizeof( T64Word );
        }
        
        winOut -> writeChars( "\n" );
    }
    
    winOut -> writeChars( "\n" );
}

//----------------------------------------------------------------------------------------
// Display absolute memory content as code shown in assembler syntax. There is one
// word per line.
//
//----------------------------------------------------------------------------------------
void  SimCommandsWin::displayAbsMemContentAsCode( T64Word adr, T64Word len ) {
    
    T64Word  index  = rounddown( adr, 4 );
    T64Word  limit  = roundup(( index + len ), 4 );
    T64Word  instr  = 0;
    char     buf[ MAX_TEXT_FIELD_LEN ];

    while ( index < limit ) {

        winOut -> printNumber( index, FMT_HEX_2_4_4 );
        winOut -> writeChars( ": " );

        if ( glb -> system -> readMem( index, (uint8_t *) &instr, 4 )) {

            disAsm -> formatInstr( buf, sizeof( buf ), instr, 16 );
            winOut -> writeChars( "%s\n", buf ); 
        }
        else winOut -> writeChars( "******\n" );

        index += sizeof( uint32_t );
    }
    
    winOut -> writeChars( "\n" );
}

//----------------------------------------------------------------------------------------
// "parseWinNumRange" will parse a window number or a range of window numbers. It
// is entered with the token being either "ALL" or a number. The result is the
// start and end window numbers in the range. Used by quite a few window commands.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::parseWinNumRange( int *winNumStart, int *winNumEnd ) {

    *winNumStart = -1;
    *winNumEnd   = - 1;

    if ( tok -> isToken( TOK_EOS )) {
        
        *winNumStart = glb -> winDisplay -> getCurrentWindow( );
        *winNumEnd   = *winNumStart;
    } 
    else if ( tok -> isToken( TOK_ALL )) {

        tok -> nextToken( );

        *winNumStart = 0;
        *winNumEnd   = MAX_WINDOWS - 1;
    }
    else if ( tok -> tokTyp( ) == TYP_NUM ) {

        *winNumStart = eval -> acceptNumExpr( ERR_INVALID_ARG, 0, MAX_WINDOWS - 1 );

        if ( tok -> isToken( TOK_COMMA )) {

            tok -> nextToken( );

            if ( tok -> tokTyp( ) == TYP_NUM ) {

                *winNumEnd = eval -> acceptNumExpr( ERR_INVALID_ARG, 
                                                    0, 
                                                    MAX_WINDOWS - 1 );
            }
            else throw( ERR_INVALID_ARG );
        }
        else *winNumEnd = *winNumStart;

         if ( *winNumStart > *winNumEnd ) {

            int tmp     = *winNumStart;
            *winNumStart = *winNumEnd;
            *winNumEnd   = tmp;
        }

        *winNumStart = internalWinNum( *winNumStart );
        *winNumEnd   = internalWinNum( *winNumEnd );
    }
}

//----------------------------------------------------------------------------------------
// "execCmdsFromFile" will open a text file and interpret each line as a command. 
// This routine is used by the "XF" command and also as the handler for the program
// argument option to execute a file before entering the command loop.
//
// XF "<file-path>"
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::execCmdsFromFile( char* fileName ) {

    char lineBuf[ MAX_CMD_LINE_SIZE ];
    char cmdLineBuf[ MAX_CMD_LINE_SIZE ] = "";

    try {

        if ( strlen( fileName ) == 0 ) throw( ERR_EXPECTED_FILE_NAME );

        rtrim( fileName );

        FILE *f = fopen( fileName, "r" );
        if ( f == nullptr ) {

            winOut -> writeChars( "File: \"%s\"\n", fileName );
            winOut -> writeChars( "File open error: %s\n", strerror( errno ));
            throw( ERR_OPEN_EXEC_FILE );
        }

        cmdLineBuf[0] = '\0';

        while ( fgets( lineBuf, sizeof(lineBuf), f ) != nullptr ) {

            lineBuf[ strcspn(lineBuf, "\r\n") ] = 0;

            size_t len          = strlen(lineBuf);
            int    continuation = 0;

            if (( len > 0 ) && ( lineBuf[ len - 1 ] == '\\' )) {

                continuation = 1;
                lineBuf[ len - 1 ] = '\0';
            }

            if ( strlen(cmdLineBuf) + strlen( lineBuf ) + 1 >= sizeof( cmdLineBuf )) {

                fclose( f );
                throw( ERR_CMD_LINE_TOO_LONG );
            }

            strcat( cmdLineBuf, lineBuf );

            if ( continuation ) continue;

            if ( glb -> env -> getEnvVarBool((char*) ENV_ECHO_CMD_INPUT )) {

                winOut -> writeChars( "%s\n", cmdLineBuf );
            }

            removeComment( cmdLineBuf );
            evalInputLine( cmdLineBuf );

            cmdLineBuf[0] = '\0'; 
        }

        fclose( f );
    }
    catch ( SimErrMsgId errNum ) {

        throw(errNum);
    }
}

//----------------------------------------------------------------------------------------
// Help command. With no arguments, a short help overview is printed. There are 
// commands, widow commands and predefined functions.
//
//  HELP ( cmdId | ‘commands‘ | 'wcommands‘ | ‘wtypes‘ | ‘predefined‘ | 'regset' )
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::helpCmd( ) {
    
    const char FMT_STR_SUMMARY[ ] = "%-16s%s\n";
    const char FMT_STR_DETAILS[ ] = "%s - %s\n";
    
    if ( tok -> isToken( TOK_EOS )) {
        
        for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
            
            if ( cmdHelpTab[ i ].helpTypeId == TYP_CMD )
                winOut -> writeChars( FMT_STR_SUMMARY, 
                    cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
        }
        
        winOut -> writeChars( "\n" );
    }
    else if (( tok -> isTokenTyp( TYP_CMD  )) ||
             ( tok -> isTokenTyp( TYP_WCMD )) ||
             ( tok -> isTokenTyp( TYP_P_FUNC ))) {

        if (( tok -> isToken( CMD_SET )) ||
            ( tok -> isToken( WCMD_SET )) ||
            ( tok -> isToken( REG_SET )) ||
            ( tok -> isToken( WTYPE_SET )) ||
            ( tok -> isToken( PF_SET ))) {

            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == tok -> tokTyp( )) {

                    winOut -> writeChars( FMT_STR_SUMMARY, 
                                          cmdHelpTab[ i ].cmdNameStr, 
                                          cmdHelpTab[ i ].helpStr );
                }
            }
        }
        else {

            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTokId == tok -> tokId( )) {
                    
                    winOut -> writeChars( FMT_STR_DETAILS, 
                        cmdHelpTab[ i ].cmdSyntaxStr, cmdHelpTab[ i ].helpStr );
                }
            }
        }
    }
    else throw ( ERR_INVALID_ARG );
}

//----------------------------------------------------------------------------------------
// Exit command. We will exit with the environment variable value for the exit code
// or the argument value in the command. This will be quite useful for test script
// development.
//
// EXIT <val>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::exitCmd( ) {
    
    if ( tok -> isToken( TOK_EOS )) {
        
        int exitVal = glb -> env -> getEnvVarInt((char *) ENV_EXIT_CODE );
        exit(( exitVal > 255 ) ? 255 : exitVal );
    }
    else {

        exit( eval -> acceptNumExpr( ERR_INVALID_EXIT_VAL, 0, 255 ));
    }
}

//----------------------------------------------------------------------------------------
// ENV command. The test driver has a few global environment variables for data 
// format, command count and so on. The ENV command list them all, one in particular
// and also modifies one if a value is specified. If the ENV variable dos not exist, 
// it will be allocated with the type of the value. 
//
//  ENV [ <var> [ <val> ]]
//
// Removing an environment variable:
//
//  ENV <var> "-" 
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::envCmd( ) {
    
    SimEnv *env = glb -> env;
    
    if ( tok -> isToken( TOK_EOS )) {

        int hwm = env -> getEnvHwm( );
        if ( hwm > 0 ) {

            for ( int i = 0; i < hwm; i++ ) {

                char buf[ 128 ];
                int len = env -> formatEnvEntry( i, buf, sizeof( buf ));
                if ( len > 0 ) winOut -> writeChars( "%s\n", buf );
            }
        }
    }
    else if ( tok -> tokTyp( ) == TYP_IDENT ) {
        
        char envName[ MAX_ENV_NAME_SIZE ];
        
        strcpy( envName, tok -> tokName( ));
   
        tok -> nextToken( );
        if ( tok -> isToken( TOK_EOS )) {
            
            if ( env -> isValid( envName )) {

                char buf[ 128 ];
                int len = env -> formatEnvEntry( envName, buf, sizeof( buf ));
                if ( len > 0 ) winOut -> writeChars( "%s\n", buf );
            } 
            else throw ( ERR_ENV_VAR_NOT_FOUND );
        }
        else {

            if ( tok -> isToken( TOK_MINUS )) {

                env -> removeEnvVar( envName );
            }
            else {

                SimExpr rExpr;
                eval -> parseExpr( &rExpr );

                if ( rExpr.typ == TYP_NUM )       
                    env -> setEnvVar( envName, rExpr.u.val );
                else if ( rExpr.typ == TYP_BOOL ) 
                    env -> setEnvVar( envName, rExpr.u.bVal );
                else if ( rExpr.typ == TYP_STR )  
                    env -> setEnvVar( envName, rExpr.u.str );
            }
        }
    }
}

//----------------------------------------------------------------------------------------
// Execute commands from a file command. 
//
// XF "<filepath>"
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::execFileCmd( ) {
    
    if ( tok -> tokTyp( ) == TYP_STR )  execCmdsFromFile( tok -> tokStr( ));
    else                                throw( ERR_EXPECTED_FILE_NAME );
}

//----------------------------------------------------------------------------------------
// Load an ELF file command. This commands is a bit under construction. So far
// we just call a loader routine which places the segments in physical memory.
// One day, we may have more structure to the command and what it is loading.
//
// LF "<filepath>"
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::loadElfFileCmd( ) {
    
    if ( tok -> tokTyp( ) == TYP_STR )  loadElfFile( tok -> tokStr( ));
    else                                throw( ERR_EXPECTED_FILE_NAME );
}

//----------------------------------------------------------------------------------------
// Add a module to the system. This command will add a module to the system. It is 
// typically used during startup when all modules are created. 
// 
//  NM <mType> [ { "," <key> "=" <value> }* ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::addModuleCmd( ) {

    switch ( tok -> tokId( )) {

        case TOK_PROC:  addProcModule( );   break;
        case TOK_MEM:   addMemModule( );    break;
        case TOK_IO:    addIoModule( );     break;
        default:        throw( ERR_INVALID_MODULE_TYPE );
    }
}

//----------------------------------------------------------------------------------------
// Remove a  module from the system. This command will remove the module and also
// close any related window that was created referencing this module. First, all
// windows associated are removed, then the object itself is removed from the 
// module map. The garbage collector does the (sad) rest.
//
//  RM <mNum>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::removeModuleCmd( ) {

    int modNum = -1;

     if ( tok -> tokTyp( ) == TYP_NUM ) {

        modNum = eval -> acceptNumExpr( ERR_EXPECTED_WIN_ID, 1, MAX_MODULES );    
    }

    tok -> checkEOS( );

    T64Module *m = glb -> system -> lookupByModNum( modNum );
    if ( m == nullptr ) throw((SimErrMsgId) 9999 );

    glb -> winDisplay -> windowKillByModNum( modNum );
    glb -> winDisplay -> setWinReFormat( );
    glb -> system -> removeFromModuleMap( m );
}

//----------------------------------------------------------------------------------------
// Display Module Table command. The simulator features a system bus to which the 
// modules are plugged in. This command shows all known modules.
//
//  DM [ <mNum> ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::displayModuleCmd( ) {

    int modNum = -1;

     if ( tok -> tokTyp( ) == TYP_NUM ) {

        modNum = eval -> acceptNumExpr( ERR_EXPECTED_WIN_ID, 1, MAX_MODULES );
        tok -> checkEOS( );
    }
    else if ( ! tok -> isToken( TOK_EOS )) {
        
        throw ( ERR_INVALID_ARG );
    }

    winOut -> writeChars( "%-5s%-7s%-16s%-16s%-8s\n", 
                            "Mod", "Type", "HPA", "SPA", "Size" );

    for ( int i = 0; i < MAX_MOD_MAP_ENTRIES; i++ ) {

        T64Module *mPtr = glb -> system -> lookupByModNum( i );
        if ( mPtr != nullptr ) {

            if (( modNum != -1 ) && ( modNum != i )) continue;

            winOut -> writeChars( "%02d   ", i  );

            winOut -> writeChars( "%-7s", mPtr -> getModuleTypeName( ));

            winOut -> printNumber( mPtr -> getHpaAdr( ), 
                                   FMT_PREFIX_0X | FMT_HEX_2_4_4 );
            winOut -> writeChars( "  " );

            if ( mPtr -> getSpaLen( ) > 0 ) {

                winOut -> printNumber( mPtr -> getSpaAdr( ), 
                                       FMT_PREFIX_0X | FMT_HEX_2_4_4 );
                winOut -> writeChars( "  " );

                winOut -> printNumber( mPtr -> getSpaLen( ), FMT_HEX_4_4 );
                winOut -> writeChars( "  " );
            }
            
            winOut -> writeChars( "\n" );
        }
    }
}

//----------------------------------------------------------------------------------------
// Display Window Stack Table command. It is quite handy to find out about all 
// windows, especially the ones we disabled.
//
//  DW [ <sNum> ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::displayWindowCmd( ) {

    int sNum = -1;
    
    if ( tok -> tokTyp( ) == TYP_NUM ) {

        sNum = eval -> acceptNumExpr( ERR_EXPECTED_WIN_ID, 1, MAX_WIN_STACKS );
        sNum --;

        tok -> checkEOS( );
    }
    else if ( ! tok -> isToken( TOK_EOS )) {
        
        throw ( ERR_INVALID_ARG );
    }

    winOut -> writeChars( "%-10s%-10s%-10s%-10s%-10s%-10s\n", 
                            "Name", "Stack", "Id", "WType", "Mod", "MType" );

    for ( int i = 0; i < MAX_WINDOWS; i++ ) {

            if ( glb -> winDisplay -> validWindowNum( i )) {

                if (( sNum == -1 ) || 
                    ( glb -> winDisplay -> getWinStackNum( i ) == sNum )) {

                    winOut -> writeChars( "%-10s", 
                                          glb -> winDisplay -> getWinName( i ));
                    winOut -> writeChars( "%-10d", 
                                          glb -> winDisplay -> getWinStackNum( i ) + 1 );
                    
                    winOut -> writeChars( "%-10d", i + 1);

                    winOut -> writeChars( "%-10s", 
                                          glb -> winDisplay -> getWinTypeName( i ));

                    int modNum = glb -> winDisplay -> getWinModNum( i );
                    
                    T64Module *mPtr = glb -> system -> lookupByModNum( modNum);
                    if ( mPtr != nullptr ) {

                        winOut -> writeChars( "%-10d", modNum );
                        winOut -> writeChars( "%-10s", mPtr -> getModuleTypeName( ));
                    }
                    else {

                        winOut -> writeChars( "%-10s", "N/A" );
                        winOut -> writeChars( "%-10s", "N/A" );
                    }
                
                    winOut -> writeChars( "\n" );
                }
            }
        } 
}

//----------------------------------------------------------------------------------------
// Reset command.
//
//  RESET [ ( 'SYS' | 'STATS' ) ]
//
// ??? rethink what we want ... reset the SYSTEM ? all CPUs ?
//----------------------------------------------------------------------------------------
void SimCommandsWin::resetCmd( ) {
    
    if ( tok -> isToken( TOK_EOS )) {
        
        glb -> system -> reset( );
    }
    else if ( tok -> isToken( TOK_SYS )) {

        throw( ERR_NOT_SUPPORTED );
    }
    else if ( tok -> isToken( TOK_STATS )) {
     
        throw( ERR_NOT_SUPPORTED );
    }
    else throw ( ERR_INVALID_ARG );
}

//----------------------------------------------------------------------------------------
// Run command. The command will just run the CPU until a "halt" instruction is 
// detected.
//
//  RUN
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::runCmd( ) {
    
    winOut -> writeChars( "RUN command to come ... \n");
}

//----------------------------------------------------------------------------------------
// Step command. The command will advance all processors by one instruction. Default
// is step number is one instruction.
//
//  S [ <steps> ]
//
// ??? we need to handle the console window. It should be enabled before we pass 
// control to the CPU. Make it the current window, saving the previous current 
// window. Put the console mode into non-blocking and hand over to the CPU. On 
// return from the CPU steps, enable blocking mode again and restore the current 
// window.
// 
//----------------------------------------------------------------------------------------
void SimCommandsWin::stepCmd( ) {
    
    uint32_t numOfSteps = 1;
    
    if ( tok -> tokTyp( ) == TYP_NUM ) {

        numOfSteps = eval -> acceptNumExpr( ERR_EXPECTED_STEPS, 0, UINT32_MAX );
    }
    
    tok -> checkEOS( );
    glb -> system -> step( numOfSteps );
}

//----------------------------------------------------------------------------------------
// Write line command. We analyze the expression and print out the result.
//
//  W <expr> [ , <rdx> ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::writeLineCmd( ) {
    
    SimExpr  rExpr;
    int      rdx;
    
    eval -> parseExpr( &rExpr );
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if (( tok -> isToken( TOK_HEX )) || ( tok -> isToken( TOK_DEC ))) {
            
            rdx = tok -> tokVal( );
            tok -> nextToken( );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    else rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    tok -> checkEOS( );
    
    switch ( rExpr.typ ) {
            
        case TYP_BOOL: {
            
            if ( rExpr.u.bVal == true ) winOut -> writeChars( "TRUE\n" );
            else                        winOut -> writeChars( "FALSE\n" );
            
        } break;
            
        case TYP_NUM: {

            if ( rdx == 16 )   
                winOut -> printNumber( rExpr.u.val, FMT_HEX | FMT_PREFIX_0X );
            else if ( rdx == 10 )   
                winOut -> printNumber( rExpr.u.val, FMT_DEC );
            else                    
                winOut -> writeChars( "Invalid Radix" );
        
            winOut -> writeChars( "\n" );
            
        } break;
            
        case TYP_STR: {
            
            winOut -> writeChars( "\"%s\"\n", rExpr.u.str );
            
        } break;
            
        default: throw (  ERR_INVALID_EXPR );
    }
}

//----------------------------------------------------------------------------------------
// The HIST command displays the command history. Optional, we can only report to
// a certain depth from the top.
//
//  HIST [ depth ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::histCmd( ) {
    
    int     depth = 0;
    int     cmdCount = hist -> getCmdCount( );
    
    if ( tok -> tokId( ) != TOK_EOS ) {

        depth = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, MAX_CMD_HIST );
    }
    
    if (( depth == 0 ) || ( depth > cmdCount )) depth = cmdCount;
    
    for ( int i = - depth; i < 0; i++ ) {
        
        int  cmdRef = 0;
        char *cmdLine = hist -> getCmdLine( i, &cmdRef );
        
        if ( cmdLine != nullptr )
            winOut -> writeChars( "[%d]: %s\n", cmdRef, cmdLine );
    }
}

//----------------------------------------------------------------------------------------
// Execute a previous command again. The command Id can be an absolute command 
// Id or a top of the command history buffer relative command Id. The selected 
// command is copied to the top of the history buffer and then passed to the
// command interpreter for execution.
//
//  DO <cmdNum>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::doCmd( ) {

    int cmdId = -1;
    
    if ( tok -> tokId( ) != TOK_EOS ) {

        cmdId = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, MAX_CMD_HIST );
        tok -> checkEOS( );
    }
    
    char *cmdStr = hist -> getCmdLine( cmdId );
    
    if ( cmdStr != nullptr ) evalInputLine( cmdStr );
}

//----------------------------------------------------------------------------------------
// REDO is almost like DO, except that we retrieve the selected command and put it
// already into the input command line string for the readCmdLine routine. We also 
// print it without a carriage return. The idea is that it can now be edited. The 
// edited command is added to the history buffer and then executed. The REDO command
// itself is not added to the command history stack. If the cmdNum is omitted, REDO 
// will take the last command entered.
//
//  REDO <cmdNum>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::redoCmd( ) {
    
    int cmdId = -1;
    
    if ( tok -> tokId( ) != TOK_EOS ) {
        
        cmdId = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, MAX_CMD_HIST );
    }
    
    char *cmdStr = hist -> getCmdLine( cmdId );
    
    if ( cmdStr != nullptr ) {
        
        char tmpCmd[ 256 ];
        strncpy( tmpCmd, cmdStr, sizeof( tmpCmd ));
        
        glb -> console -> writeChars( "%s", tmpCmd );
        if ( readCmdLine( tmpCmd, (int) strlen( tmpCmd ), (char *)"" ))
             evalInputLine( tmpCmd );
    }
}

//----------------------------------------------------------------------------------------
// Display absolute memory command. The offset address is a byte address, the 
// length is measured in bytes, rounded up to the a word size. We accept any 
// address and length and only check that the offset plus length does not exceed
// the physical address space. The format specifier will allow for HEX, DECIMAL
// and CODE. In the case of the code option, the default number format option is
// used for showing the offset value.
//
//  DA <ofs> [ "," <len> [ "," <fmt> ]]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::displayAbsMemCmd( ) {
    
    int         rdx     = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    T64Word     ofs     = 0;
    T64Word     len     = sizeof( T64Word );
    bool        asCode  = false;

    ofs = eval -> acceptNumExpr( ERR_EXPECTED_START_OFS, 0, T64_MAX_PHYS_MEM_LIMIT );
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isToken( TOK_COMMA )) len = sizeof( T64Word );
        else len = eval -> acceptNumExpr( ERR_EXPECTED_LEN );
    }
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        switch(  tok -> tokId( )) {

            case TOK_HEX: 
            case TOK_DEC:   rdx = tok -> tokVal( );  break;
            
            case TOK_CODE:  asCode = true; break;
            
            case TOK_EOS: 
            default:        throw ( ERR_INVALID_FMT_OPT );
        }

        tok -> nextToken( );
    }
    
    tok -> checkEOS( );
    
    if (((T64Word) ofs + len ) <= T64_MAX_PHYS_MEM_LIMIT ) { 
        
        if ( asCode ) displayAbsMemContentAsCode( ofs, len );
        else          displayAbsMemContent( ofs, len, rdx );
    }
    else throw ( ERR_OFS_LEN_LIMIT_EXCEEDED );
}

//----------------------------------------------------------------------------------------
// Modify absolute memory command. 
//
//  MA <ofs> <val>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::modifyAbsMemCmd( ) {
    
    T64Word adr = eval -> acceptNumExpr( ERR_EXPECTED_OFS, 0, INT64_MAX );
    T64Word val = eval -> acceptNumExpr( ERR_INVALID_NUM );
    tok -> checkEOS( );

    if ( ! glb -> system -> writeMem( adr, (uint8_t *) &val, sizeof( T64Word ))) {

        throw( ERR_MEM_OP_FAILED );
    }
}

//----------------------------------------------------------------------------------------
// Modify register command. This command modifies a register within a register set.
// We must be in windows mode and the current window must be a CPU type window.
//
//  MR <reg> <val>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::modifyRegCmd( ) {
   
    SimTokTypeId    regSetId    = TYP_GREG;
    int             regNum      = 0;
    T64Word         val         = 0;
   
    ensureWinModeOn( );
    
    if (( tok -> tokTyp( ) == TYP_GREG ) ||
        ( tok -> tokTyp( ) == TYP_CREG ) ||
        ( tok -> tokTyp( ) == TYP_PREG )) {
        
        regSetId    = tok -> tokTyp( );
        regNum      = tok -> tokVal( );
        tok -> nextToken( );
    }
    else throw ( ERR_INVALID_REG_ID );
    
    val = eval -> acceptNumExpr( ERR_INVALID_NUM );

    tok -> checkEOS( );

    if ( glb -> winDisplay -> getCurrentWinType( ) != WT_CPU_WIN ) 
        throw( ERR_INVALID_WIN_TYPE );

    int modNum = glb -> winDisplay -> getCurrentWinModNum( );

    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );
    if ( proc -> getModuleType( ) != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    switch( regSetId ) {

        case TYP_GREG:  proc -> getCpuPtr( ) -> setGeneralReg( regNum, val ); break;
        case TYP_CREG:  proc -> getCpuPtr( ) -> setControlReg( regNum, val ); break;

        case TYP_PREG:  {

            T64Word tmp = proc -> getCpuPtr( ) -> getPsrReg( );
            
            if      ( regNum == 1 ) tmp = depositField( tmp, 0, 52, val );
            else if ( regNum == 2 ) tmp = depositField( tmp, 52, 12, val );

            proc -> getCpuPtr( ) -> setPsrReg( tmp );     
            
        } break;
 
        default: throw( ERR_EXPECTED_REG_SET );
    }
}

//----------------------------------------------------------------------------------------
// Purges a cache line from the cache. We must be in windows mode and the current
// window must be a cache window. 
//
//  PICA <vAdr> 
//  PDCA <vAdr> 
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::purgeCacheCmd( ) {
   
    ensureWinModeOn( );
    T64Word vAdr = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC ); 
    tok -> checkEOS( );

    if ( glb -> winDisplay -> getCurrentWinType( ) != WT_CACHE_WIN ) 
        throw( ERR_INVALID_WIN_TYPE );

    int modNum = glb -> winDisplay -> getCurrentWinModNum( );
   
    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );
    if ( proc -> getModuleType( ) != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );
    
    if      ( currentCmd == CMD_PCA_I ) proc -> getICachePtr( ) -> purge( vAdr );
    else if ( currentCmd == CMD_PCA_D ) proc -> getDCachePtr( ) -> purge( vAdr );
    else throw( ERR_CACHE_PURGE_OP );
}

//----------------------------------------------------------------------------------------
// Flushes a cache line from the data cache. We must be in windows mode and the
// current window must be a Cache window. 
//
//  FDCA <vAdr> 
//  FICA <vAdr>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::flushCacheCmd( ) {

    ensureWinModeOn( );
   
    T64Word vAdr = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC ); 
    tok -> checkEOS( );

    if ( glb -> winDisplay -> getCurrentWinType( ) != WT_CACHE_WIN ) 
        throw( ERR_INVALID_WIN_TYPE );

    int modNum = glb -> winDisplay -> getCurrentWinModNum( );
   
    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );
    if ( proc -> getModuleType( ) != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    if ( currentCmd == CMD_FCA_I ) {

         proc -> getICachePtr( ) -> flush( vAdr );
    }
    else if ( currentCmd == CMD_FCA_D ) {

        proc -> getDCachePtr( ) -> flush( vAdr );
       
    }
    else throw( ERR_CACHE_FLUSH_OP );
}

//----------------------------------------------------------------------------------------
// Insert into TLB command. We have two modes. We must be in windows mode and the 
// current window must be a TLB window. 
//
//  IITLB <vAdr> "," <pAdr> "," <pSize> "," <acc> [ "," "L" [ "," "U" ]]
//  IDTLB <vAdr> "," <pAdr> "," <pSize> "," <acc> [ "," "L" [ "," "U" ]]
//
// ??? cross check flags field for correct position...
//----------------------------------------------------------------------------------------
void SimCommandsWin::insertTLBCmd( ) {

    ensureWinModeOn( );
    T64Word vAdr = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, T64_MAX_VIRT_MEM_LIMIT ); 
    tok -> acceptComma( );
    T64Word pAdr = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, T64_MAX_PHYS_MEM_LIMIT ); 
    tok -> acceptComma( );
    T64Word size = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, 15 );
    tok -> acceptComma( );
    T64Word acc = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, 15 );

    T64Word info = 0;    
    info = depositField( info, 40, 4, acc );
    info = depositField( info, 36, 4, size );
    info = depositField( info, 12, 24, pAdr >> T64_PAGE_OFS_BITS );

    if ( tok -> isToken( TOK_COMMA )) {

        tok -> nextToken( );
        if ( tok -> isTokenIdent((char *) "L" )) {

            info = depositField( info, 56, 2, 0x1 );
            tok -> nextToken( );

            if ( tok -> isToken( TOK_COMMA )) {

                tok -> nextToken( );
                if ( tok -> isTokenIdent((char *) "U" )) {

                    info = depositField( info, 58, 2, 0x2 );
                    tok -> nextToken( );
                }
                else throw( ERR_INVALID_TLB_ACC_FLAG );
            }
        }
        else throw( ERR_INVALID_TLB_ACC_FLAG );
    }
    
    tok -> checkEOS( );

    if ( glb -> winDisplay -> getCurrentWinType( ) != WT_TLB_WIN ) 
        throw( ERR_INVALID_WIN_TYPE );

    int modNum = glb -> winDisplay -> getCurrentWinModNum( );
   
    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );
    if ( proc -> getModuleType( ) != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    if ( currentCmd == CMD_ITLB_I ) {
        
        if ( ! proc -> getITlbPtr( ) -> insert( vAdr, info )) 
            throw( ERR_TLB_INSERT_OP );
    }
    else if ( currentCmd == CMD_ITLB_D ) { 
        
        if ( ! proc -> getDTlbPtr( ) -> insert( vAdr, info )) 
            throw( ERR_TLB_INSERT_OP );
    } 
    else throw( ERR_TLB_INSERT_OP ); 
}

//----------------------------------------------------------------------------------------
// Purge from TLB command. We have two modes. We must be in windows mode and the 
// current window must be a TLB window. 
//
//  PITLB  <vAdr>
//  PDTLB  <vAdr>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::purgeTLBCmd( ) {

    ensureWinModeOn( );
    T64Word vAdr = eval -> acceptNumExpr( ERR_INVALID_NUM, 0 ); 
    tok -> checkEOS( );

    if ( glb -> winDisplay -> getCurrentWinType( ) != WT_TLB_WIN ) 
        throw( ERR_INVALID_WIN_TYPE );

    int modNum = glb -> winDisplay -> getCurrentWinModNum( );
   
    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );
    if ( proc -> getModuleType( ) != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    if      ( currentCmd == CMD_PTLB_I ) proc -> getITlbPtr( ) -> purge( vAdr );
    else if ( currentCmd == CMD_PTLB_D ) proc -> getDTlbPtr( ) -> purge( vAdr );
    else ;
}

//----------------------------------------------------------------------------------------
// Global windows commands. 
//
//  WON
//  WOFF
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winOnCmd( ) {
   
    glb -> winDisplay -> windowsOn( );
}

void SimCommandsWin::winOffCmd( ) {
  
    glb -> winDisplay -> windowsOff( );
}

//----------------------------------------------------------------------------------------
// Enable/disable a window stack. If there is no parameter, all stacks are selected, 
// otherwise a single stack is selected.
//
//  WSE [ <stackNum> | ALL ]
//  WSD [ <stackNum> | ALL ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winStacksEnableCmd( bool enable ) {

    int stackNum = -1;
    
    if ( tok -> tokId( ) != TOK_EOS ) {

        if ( tok -> tokId( ) == TOK_NUM ) {

            stackNum = eval -> acceptNumExpr( ERR_EXPECTED_STACK_ID, 1, MAX_WIN_STACKS );
            if ( stackNum > MAX_WIN_STACKS ) throw ( ERR_INVALID_WIN_STACK_ID );

            stackNum -= 1;
            glb -> winDisplay -> winStacksEnable( stackNum, enable );
        }
        else if ( tok -> isToken( TOK_ALL )) {

            for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {

                glb -> winDisplay -> winStacksEnable( i, enable );     
            }
        }
        else throw( ERR_INVALID_ARG );
    }

    tok -> checkEOS( );
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// Window default command. We accept a range of windows.
//
//  WDEF [[ <winNumStart> [ "," <winNumEnd]] | "ALL" ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winDefCmd( ) {

    int winNumStart = -1;
    int winNumEnd   = -1;
    
    if ( ! glb -> winDisplay -> isWinModeOn( )) throw ( ERR_NOT_IN_WIN_MODE );

    parseWinNumRange( &winNumStart, &winNumEnd );
    tok -> checkEOS( );

    glb -> winDisplay -> windowDefaults( winNumStart, winNumEnd );
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// Windows enable and disable. When enabled, a window does show up on the screen. 
// The window number is optional, used for user definable windows.
//
//  <win>E [[ <winNumStart> [ "," <winNumEnd]] || "ALL" ]
//  <win>D [[ <winNumStart> [ "," <winNumEnd]] || "ALL" ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winEnableCmd( bool enable ) {
    
    int winNumStart = -1;
    int winNumEnd   = -1;
    
    if ( ! glb -> winDisplay -> isWinModeOn( )) throw ( ERR_NOT_IN_WIN_MODE );

    parseWinNumRange( &winNumStart, &winNumEnd );
    tok -> checkEOS( );
    
    glb -> winDisplay -> windowEnable( winNumStart, winNumEnd, enable );
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// Windows radix. This command sets the radix for a given window. We parse the 
// command and the format option and pass the tokens to the screen handler. The 
// window number is optional, used for user definable windows.
//
//  <win>R [ <radix> [ "," <winNum> ]]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winSetRadixCmd( ) {

   
    int rdx     = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    int winNum  = -1;
   
    if ( tok -> isToken( TOK_EOS )) {
        
        glb -> winDisplay -> windowRadix( rdx, internalWinNum( winNum ));
        return;
    }
    else if ( tok -> isToken( TOK_COMMA )) {
        
        rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        tok -> nextToken( );

        winNum = eval -> acceptNumExpr( ERR_EXPECTED_WIN_ID, 1, MAX_WINDOWS );
    }
    else {
        
        if      ( tok -> isToken( TOK_DEC )) rdx = 10;
        else if ( tok -> isToken( TOK_HEX )) rdx = 16;
        else throw( ERR_INVALID_RADIX );

        tok -> nextToken( );
        if ( tok -> isToken( TOK_COMMA )) {
        
            tok -> nextToken( );
            winNum = eval -> acceptNumExpr( ERR_EXPECTED_WIN_ID, 1, MAX_WINDOWS );
        }
    }
    
    tok -> checkEOS( );
    glb -> winDisplay -> windowRadix( rdx, internalWinNum( winNum ));
}

//----------------------------------------------------------------------------------------
// Window scrolling. This command advances the item address of a scrollable window 
// by the number of lines multiplied by the number of items on a line forward or 
// backward. The meaning of the item address and line items is window dependent. 
// If the amount is zero, the default value of the window will be used. The window
// number is optional, used for user definable windows. If omitted, we mean the 
// current window.
//
//  <win>F [ <amt> [ , <winNum> ]]
//  <win>B [ <amt> [ , <winNum> ]]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winForwardCmd( ) {
  
    T64Word winItems    = 0;
    int     winNum      = -1;
    
    if ( tok -> tokId( ) != TOK_EOS ) {
      
        winItems = eval -> acceptNumExpr( ERR_INVALID_NUM, 0 );
      
        if ( tok -> isToken( TOK_COMMA )) {
            
            tok -> nextToken( );
            winNum = eval -> acceptNumExpr( ERR_EXPECTED_WIN_ID, 1, MAX_WINDOWS );
        }
      
        tok -> checkEOS( );
    }

    glb -> winDisplay -> windowForward( winItems, internalWinNum( winNum ));
}

void SimCommandsWin::winBackwardCmd( ) {
   
    T64Word winItems    = 0;
    int     winNum      = 0;
    
    if ( tok -> tokId( ) != TOK_EOS ) {
      
        winItems = eval -> acceptNumExpr( ERR_INVALID_NUM, 0 );
      
        if ( tok -> isToken( TOK_COMMA )) {
            
            tok -> nextToken( );
            winNum = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WINDOWS );
        }
      
        tok -> checkEOS( );
    }
    
    glb -> winDisplay -> windowBackward( winItems, internalWinNum( winNum  ));
}

//----------------------------------------------------------------------------------------
// Window home. Each window has a home item address, which was set at window creation
// or trough a non-zero value previously passed to this command. The command sets the
// window item address to this value. The meaning of the item address is window 
// dependent. The window number is optional, used for user definable windows.
//
//  <win>H [ <pos> [ "," <winNum> ]]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winHomeCmd( ) {
   
    T64Word winPos  = 0;
    int     winNum  = -1;
    
    if ( tok -> tokId( ) != TOK_EOS ) {

        if ( tok -> isToken( TOK_COMMA )) {
            
            tok -> nextToken( );
            winNum = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WINDOWS );
        }
        else winPos = eval -> acceptNumExpr( ERR_INVALID_NUM ); 
      
        tok -> checkEOS( );
    }

    glb -> winDisplay -> windowHome( winPos, internalWinNum( winNum  ));
}

//----------------------------------------------------------------------------------------
// Window jump. The window jump command sets the item address to the position argument.
// The meaning of the item address is window dependent. The window number is optional,
// used for user definable windows.
//
//  WJ [ <pos> [ "," <winNum> ]]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winJumpCmd( ) {
   
    T64Word winPos  = 0;
    int     winNum  = -1;
    
    if ( tok -> tokId( ) != TOK_EOS ) {
      
        winPos = eval -> acceptNumExpr( ERR_INVALID_NUM );
      
        if ( tok -> isToken( TOK_COMMA )) {
            
            tok -> nextToken( );
            winNum = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WINDOWS );

            if ( ! glb -> winDisplay -> validWindowNum( internalWinNum( winNum ))) 
                throw ( ERR_INVALID_WIN_ID );
        }
      
        tok -> checkEOS( );
    }
    
    glb -> winDisplay -> windowJump( winPos, winNum );
}

//----------------------------------------------------------------------------------------
// Set window lines. This command sets the the number of rows for a window. The 
// number includes the banner line. If the "lines" argument is omitted, the window
// default value will be used. The window number is optional, used for user definable
// windows.
//
//  WL [ <lines> [ "," <winNum> ]]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winSetRowsCmd( ) {
   
    if ( tok -> isToken( TOK_EOS )) {
        
        glb -> winDisplay -> windowSetRows( 0, 0 );
    }
    else {

        int winLines = eval -> acceptNumExpr( ERR_INVALID_NUM );
        int winNum   = -1;
    
        if ( tok -> isToken( TOK_COMMA )) {
        
            tok -> nextToken( );
            winNum = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WINDOWS );

            if ( ! glb -> winDisplay -> validWindowNum( internalWinNum( winNum ))) 
                throw ( ERR_INVALID_WIN_ID );
        }

        tok -> checkEOS( );
        glb -> winDisplay -> windowSetRows( winLines, internalWinNum( winNum ));
        glb -> winDisplay -> setWinReFormat( );
    }    
}

//----------------------------------------------------------------------------------------
// Set command window lines. The command sets the the number of rows for the command
// window. The number includes the banner line. If the "lines" argument is omitted, 
// the window default value will be used. 
//
//  CWL <lines>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winSetCmdWinRowsCmd( ) {

    int winLines = eval -> acceptNumExpr( ERR_INVALID_NUM, 0, MAX_CMD_LINES );
    tok -> checkEOS( );
    glb -> winDisplay -> windowSetCmdWinRows( winLines );
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// Clear command window.
//  
//  CWC
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winClearCmdWinCmd( ) {

    tok -> checkEOS( );
    glb -> winDisplay -> windowClearCmdWin( );
}   

//----------------------------------------------------------------------------------------
// Window current command. User definable windows are controlled by their window 
// number. To avoid typing this number all the time for a user window command, a 
// user window can explicitly be set as the current command.
//
//  WC <winNum>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winCurrentCmd( ) {
    
    if ( tok -> isToken( TOK_EOS )) throw ( ERR_EXPECTED_WIN_ID );

    int winNum = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WINDOWS );
    
    tok -> checkEOS( );
    glb -> winDisplay -> setCurrentWindow( internalWinNum( winNum ));
}

//----------------------------------------------------------------------------------------
// This command toggles through alternate window content, if supported by the window.
// An example is the cache sets in a two-way associative cache. The toggle command 
// will just flip through the sets or set a specific set.
//
//  WT [ <winNum> [ "," <toggleVal> ]]
//
//----------------------------------------------------------------------------------------
void  SimCommandsWin::winToggleCmd( ) {
    
    if ( tok -> isToken( TOK_EOS )) {
        
        glb -> winDisplay -> 
            windowToggle( glb -> winDisplay -> getCurrentWindow( ), 0 );
    }
    else {

        int toggleVal = 0;
        int winNum    = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WINDOWS );
    
        if ( ! glb -> winDisplay -> validWindowNum( internalWinNum( winNum ))) 
            throw ( ERR_INVALID_WIN_ID );

        if ( tok -> isToken( TOK_COMMA )) {

            tok -> nextToken( );

            toggleVal = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WIN_TOGGLES );
            tok -> checkEOS( );
        }
    
        glb -> winDisplay -> windowToggle( internalWinNum( winNum ), toggleVal );
    }

    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// This command exchanges the current user window with the user window specified. It 
// allows to change the order of the user windows in a stacks.
//
//  WX <winNum>
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winExchangeCmd( ) {

    ensureWinModeOn( );
    
    if ( tok -> isToken( TOK_EOS )) throw ( ERR_EXPECTED_WIN_ID );

    int winNum = eval -> acceptNumExpr( ERR_INVALID_WIN_ID, 1, MAX_WINDOWS );
    
    tok -> checkEOS( );
    
    if ( ! glb -> winDisplay -> validWindowNum( internalWinNum( winNum ))) 
        throw ( ERR_INVALID_WIN_ID );

    glb -> winDisplay -> windowExchangeOrder( internalWinNum( winNum ));
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// This command creates a new window. The window is assigned a free index from 
// the windows list. This index is used in all the calls to this window. The window 
// type is determined by the keyword plus additional info such as module and 
// submodule number. Note that we do not create simulator module objects, we 
// merely attach a window to them. So they must exist. The general form of the
// command is:
//
//  WN <winType> [ "," <arg1> [ "," <arg2> ]]
//
//  WN  PROC    "," <mod>
//  WN  CPU     "," <mod>
//  WN  ICACHE  "," <mod>
//  WN  DCACHE  "," <mod>
//  WN  ITLB    "," <mod>
//  WN  DTLB    "," <mod>
//  WN  MEM     "," <adr>
//  WN  CODE    "," <adr>
//  WN  TEXT    "," <str>
// 
//----------------------------------------------------------------------------------------
void SimCommandsWin::winNewWinCmd( ) {
    
    SimTokId  winType = TOK_NIL;

    ensureWinModeOn( );
    winType = tok -> acceptTokSym( ERR_EXPECTED_WIN_ID );
 
    switch ( winType ) {

        case TOK_PROC: {

            tok -> acceptComma( );
            int modNum = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC );
            tok -> checkEOS( );

            glb -> winDisplay -> windowNewCpuState( modNum );
            glb -> winDisplay -> windowNewTlb( modNum, T64_TK_INSTR_TLB );  
            glb -> winDisplay -> windowNewTlb( modNum, T64_TK_DATA_TLB );
            glb -> winDisplay -> windowNewCache( modNum, T64_CK_INSTR_CACHE );
            glb -> winDisplay -> windowNewCache( modNum, T64_CK_DATA_CACHE ); 

        } break;

        case TOK_CPU: {

            tok -> acceptComma( );
            int modNum = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC );
            tok -> checkEOS( );

            glb -> winDisplay -> windowNewCpuState( modNum );  

        } break;

        case TOK_ITLB: {

            tok -> acceptComma( );
            int modNum = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC );
            tok -> checkEOS( );

            glb -> winDisplay -> windowNewTlb( modNum, T64_TK_INSTR_TLB );  

        } break;

        case TOK_DTLB: {

            tok -> acceptComma( );
            int modNum = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC );
            tok -> checkEOS( );

            glb -> winDisplay -> windowNewTlb( modNum, T64_TK_DATA_TLB );  

        } break;

        case TOK_ICACHE: {

            tok -> acceptComma( );
            int modNum = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC );
            tok -> checkEOS( );

            glb -> winDisplay -> windowNewCache( modNum, T64_CK_INSTR_CACHE );  

        } break;

        case TOK_DCACHE: {

            tok -> acceptComma( );
            int modNum = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC );
            tok -> checkEOS( );

            glb -> winDisplay -> windowNewCache( modNum, T64_CK_DATA_CACHE );  

        } break;

        case TOK_MEM: {

            tok -> acceptComma( );
            T64Word adr = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC, 
                                                 0,
                                                 T64_MAX_PHYS_MEM_LIMIT );
            tok -> checkEOS( );

            T64Module *mod = glb -> system -> lookupByAdr( adr );
            if ( mod != nullptr ) {

                glb -> winDisplay -> windowNewAbsMem( mod -> getModuleNum( ), adr );
            }
            else throw( 9997 );

        }  break;

        case TOK_CODE: {  

            tok -> acceptComma( );
            T64Word adr = eval -> acceptNumExpr( ERR_EXPECTED_NUMERIC, 
                                                 0,
                                                 T64_MAX_PHYS_MEM_LIMIT );
            tok -> checkEOS( );

             T64Module *mod = glb -> system -> lookupByAdr( adr );
            if ( mod != nullptr ) {

                glb -> winDisplay -> windowNewAbsCode( mod -> getModuleNum( ), adr );
            }
            else throw( 9997 );

        }  break;

        case TOK_TEXT: {

            tok -> acceptComma( );
        
            char *argStr = nullptr;
            if ( tok -> tokTyp( ) == TYP_STR ) argStr = tok -> tokStr( );
            else throw ( ERR_INVALID_ARG );

            tok -> nextToken( );
            tok -> checkEOS( );
            glb -> winDisplay -> windowNewText( argStr );

        } break;

        default: throw( ERR_INVALID_WIN_TYPE );
    }
    
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// This command removes  a user defined window or window range from the list of windows.
// A number of -1 will kill all user defined windows.
//
//  WK [[ <winNumStart> [ "," <winNumEnd]] || "ALL" ]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winKillWinCmd( ) {
    
    int winNumStart = -1;
    int winNumEnd   = -1;
    
    if ( ! glb -> winDisplay -> isWinModeOn( )) throw ( ERR_NOT_IN_WIN_MODE );

    parseWinNumRange( &winNumStart, &winNumEnd );
    tok -> checkEOS( );

    glb -> winDisplay -> windowKill( winNumStart, winNumEnd );
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// This command assigns a user window to a stack. User windows can be displayed in a
// separate stack of windows. The first stack is always the main stack, where the 
// predefined and command window can be found. Stacks are numbered from 1 to MAX.
//
//  WS <stackNum> [ , <winNumStart> [ , <winNumEnd ]]
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::winSetStackCmd( ) {
    
    int     winStack    = -1;
    int     winNumStart = -1;
    int     winNumEnd   = -1;
    
    if ( ! glb -> winDisplay -> isWinModeOn( )) throw ( ERR_NOT_IN_WIN_MODE );

    winStack = eval -> acceptNumExpr( ERR_EXPECTED_STACK_ID, 1, MAX_WIN_STACKS );
    
    if ( tok -> isToken( TOK_EOS )) {
        
        winNumStart = glb -> winDisplay -> getCurrentWindow( );
        winNumEnd   = winNumStart;
    }
    else if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );

        parseWinNumRange( &winNumStart, &winNumEnd );
        tok -> checkEOS( );
    }
    else throw ( ERR_EXPECTED_COMMA );

    if ( winStack >= MAX_WIN_STACKS ) throw ( ERR_INVALID_WIN_STACK_ID );
   
    glb -> winDisplay -> windowSetStack( winStack - 1, winNumStart, winNumEnd );
    glb -> winDisplay -> setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// Evaluate input line. There are commands, functions, expressions and so on. This 
// routine sets up the tokenizer and dispatches based on the first token in the input
// line. The commands are also added to the command history, with the exception of 
// the HITS, DO and REDOP commands.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::evalInputLine( char *cmdBuf ) {
    
    try {
        
        if ( strlen( cmdBuf ) > 0 ) {
            
            tok -> setupTokenizer( cmdBuf, (SimToken *) cmdTokTab );
            tok -> nextToken( );
            
            if (( tok -> isTokenTyp( TYP_CMD )) || ( tok -> isTokenTyp( TYP_WCMD ))) {
                
                currentCmd = tok -> tokId( );
                tok -> nextToken( );
                
                if (( currentCmd != CMD_HIST ) &&
                    ( currentCmd != CMD_DO ) &&
                    ( currentCmd != CMD_REDO )) {
                    
                    hist -> addCmdLine( cmdBuf );
                    glb -> env -> 
                        setEnvVar((char *) ENV_CMD_CNT, (T64Word) hist -> getCmdNum( ));
                }
                
                switch( currentCmd ) {
                        
                    case TOK_NIL:                                           break;
                    case CMD_EXIT:          exitCmd( );                     break;
                        
                    case CMD_HELP:          helpCmd( );                     break;
                    case CMD_ENV:           envCmd( );                      break;
                    case CMD_XF:            execFileCmd( );                 break;
                    case CMD_LF:            loadElfFileCmd( );              break;
                        
                    case CMD_WRITE_LINE:    writeLineCmd( );                break;
                        
                    case CMD_HIST:          histCmd( );                     break;
                    case CMD_DO:            doCmd( );                       break;
                    case CMD_REDO:          redoCmd( );                     break;
                        
                    case CMD_RESET:         resetCmd( );                    break;
                    case CMD_RUN:           runCmd( );                      break;
                    case CMD_STEP:          stepCmd( );                     break;

                    case CMD_NM:            addModuleCmd( );                break;
                    case CMD_RM:            removeModuleCmd( );             break;
                    case CMD_DM:            displayModuleCmd( );            break;   

                    case CMD_DW:            displayWindowCmd( );            break;  

                    case CMD_MR:            modifyRegCmd( );                break;
                        
                    case CMD_DA:            displayAbsMemCmd( );            break;
                    case CMD_MA:            modifyAbsMemCmd( );             break;
                        
                    case CMD_ITLB_I: 
                    case CMD_ITLB_D:        insertTLBCmd( );                break;

                    case CMD_PTLB_I:
                    case CMD_PTLB_D:        purgeTLBCmd( );                 break;
                        
                    case CMD_PCA_I:  
                    case CMD_PCA_D:         purgeCacheCmd( );               break;

                    case CMD_FCA_D:         flushCacheCmd( );               break;
                        
                    case CMD_WON:           winOnCmd( );                    break;
                    case CMD_WOFF:          winOffCmd( );                   break;
                    case CMD_WDEF:          winDefCmd( );                   break;
                    case CMD_WSE:           winStacksEnableCmd( true );     break;
                    case CMD_WSD:           winStacksEnableCmd( false );    break;
                        
                    case CMD_WC:            winCurrentCmd( );               break;
                    case CMD_WN:            winNewWinCmd( );                break;
                    case CMD_WK:            winKillWinCmd( );               break;
                    case CMD_WS:            winSetStackCmd( );              break;
                    case CMD_WT:            winToggleCmd( );                break;
                    case CMD_WX:            winExchangeCmd( );              break;
                    case CMD_WF:            winForwardCmd( );               break;
                    case CMD_WB:            winBackwardCmd( );              break;
                    case CMD_WH:            winHomeCmd( );                  break;
                    case CMD_WJ:            winJumpCmd( );                  break;
                    case CMD_WE:            winEnableCmd( true );           break;
                    case CMD_WD:            winEnableCmd( false );          break;
                    case CMD_WR:            winSetRadixCmd( );              break;    
                    case CMD_CWL:           winSetCmdWinRowsCmd( );         break;
                    case CMD_CWC:           winClearCmdWinCmd( );           break;
                    case CMD_WL:            winSetRowsCmd( );               break;
                        
                    default:                throw ( ERR_INVALID_CMD );
                }
            }
            else {
            
                hist -> addCmdLine( cmdBuf );
                glb -> env -> setEnvVar((char *) ENV_CMD_CNT, 
                                        (T64Word) hist -> getCmdNum( ));
                throw ( ERR_INVALID_CMD );
            }
        }
    }
    
    catch ( SimErrMsgId errNum ) {
        
        glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, (T64Word) -1 );
        cmdLineError( errNum );
    }
}

//----------------------------------------------------------------------------------------
// "cmdLoop" is the command line input interpreter. The basic loop is to prompt for
// the next input, read the input and evaluates it. If we are in windows mode, we also
// redraw the screen.
//
//----------------------------------------------------------------------------------------
void SimCommandsWin::cmdInterpreterLoop( ) {
    
    char cmdLineBuf[ MAX_CMD_LINE_SIZE ];
    char cmdPrompt[ MAX_CMD_LINE_SIZE ];

    glb -> winDisplay -> setWinReFormat( );
    glb -> winDisplay -> reDraw( );
   
    printWelcome( );
    glb -> winDisplay -> reDraw( );

    configureT64Sim( );
    glb -> winDisplay -> reDraw( );
    
    while ( true ) {
        
        buildCmdPrompt( cmdPrompt, sizeof( cmdPrompt ));
        int cmdLen = readCmdLine( cmdLineBuf, 0, cmdPrompt );

        if ( cmdLen > 0 ) evalInputLine( cmdLineBuf );
        glb -> winDisplay -> reDraw( );
    }
}