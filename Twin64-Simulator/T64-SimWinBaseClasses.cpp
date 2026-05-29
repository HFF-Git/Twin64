//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Windows Base Classes
//
//----------------------------------------------------------------------------------------
// The simulator use a set of windows to show the system state. No, don't think 
// of modern windows. We have a terminal screen and use escape sequences to build
// windows. See the declaration include file for more details. This file contains
// the window base classes.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Windows Base Classes
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
#include "T64-SimDeclarations.h"

//----------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//----------------------------------------------------------------------------------------
namespace {

}; // namespace


//****************************************************************************************
//****************************************************************************************
//
// Methods for the ScreenWindow abstract class.
//
//----------------------------------------------------------------------------------------
// Object constructor and destructor. We need them as we create and destroy user
// definable windows.
//
//----------------------------------------------------------------------------------------
SimWin::SimWin( SimGlobals *glb ) { 
    
    this -> glb = glb; 
}

SimWin:: ~SimWin( ) { }

//----------------------------------------------------------------------------------------
// Getter/Setter methods for window attributes.
//
//----------------------------------------------------------------------------------------
SimWinType  SimWin::getWinType( ) { 
    
    return ( winType ); 
}

void SimWin::setWinType( SimWinType arg ) { 
    
    winType = arg; 
}

int SimWin::getWinIndex( ) { 
    
    return ( winIndex ); 
}

void SimWin::setWinIndex( int arg ) { 
    
    winIndex = arg; 
}

void SimWin::setWinName( char *name ) {

    strncpy( winName, name, MAX_WIN_NAME );
}

char *SimWin::getWinName( ) {

    return( winName );
}

void SimWin::setWinModNum( int modNum ) {

    winModNum = modNum;
}

int SimWin::getWinModNum( ) {

    return( winModNum );
}

bool SimWin::isEnabled( ) { 
    
    return ( winEnabled ); 
}

void SimWin::setEnable( bool arg ) { 

    winEnabled = arg; 
}

int SimWin::getDefRows( ) { 

    return ( winSizes[ winToggleVal ].actualRow ); 
}

int SimWin::getRows( ) { 

    return( winRows );
}

void SimWin::setRows( int arg ) { 

    int maxRows = winSizes[ winToggleVal ].maxRow;
    int minRows = winSizes[ winToggleVal ].minRow;

    if (( arg < minRows ) || ( arg > maxRows )) arg = minRows;

    winSizes[ winToggleVal ].actualRow = arg;

    winRows = arg;
}

int SimWin::getDefColumns( ) { 
    
    return ( winSizes[ winToggleVal ].actualCol );
}

int SimWin::getColumns( ) { 

    return( winColumns ); 
}

void SimWin::setColumns( int arg ) { 

    winColumns = (( arg > MAX_WIN_COL_SIZE ) ? MAX_WIN_COL_SIZE : arg );
}

void SimWin::setRadix( int rdx ) { 

    if (( rdx == 10 ) || ( rdx == 16 )) winRadix = rdx; 
    else winRadix = 16;
}

int SimWin::getRadix( ) { 
    
    return ( winRadix ); 
}

int SimWin::getWinStack( ) {  
    
    return ( winStack ); 
}

void SimWin::setWinStack( int wStack ) { 
    
    winStack = (( wStack > MAX_WIN_STACKS ) ? 0 : wStack ); 
}

//----------------------------------------------------------------------------------------
// Each window allows the toggling through different content. The implementation 
// of what the particular toggle value means is entirely up to the specific window. 
// When a window is created, the default values for the number of defined views is 
// set. The defined views are the limit where we wrap around. Routines that get and
// set window rows and column sizes are always referring to the actual toggled view.
// The "WT" command advances through the defined toggle view. When the toggle value
// changes, the default rows and columns change too.
//
//----------------------------------------------------------------------------------------
int SimWin::getWinToggleLimit( ) { 
    
    return ( winToggleLimit ); 
}

void SimWin::setWinToggleLimit( int limit ) { 

    if ( isInRange( limit, 1, MAX_WIN_TOGGLES )) winToggleLimit = limit; 
    else winToggleLimit = 1;
}

void SimWin::setWinLimitsForToggle( int toggleVal, 
                                    int minRow, 
                                    int maxRow, 
                                    int minCol,
                                    int maxCol ) {

    toggleVal = toggleVal % MAX_WIN_TOGGLES;

    winSizes[ toggleVal ].minRow = minRow;
    winSizes[ toggleVal ].maxRow = maxRow;
    winSizes[ toggleVal ].minCol = minCol;
    winSizes[ toggleVal ].maxCol = maxCol;

    winSizes[ toggleVal ].actualRow = minRow;
    winSizes[ toggleVal ].actualCol = minCol;
}

void SimWin::setWinSizeForToggle( int toggleVal, int row, int col ) {

    toggleVal = toggleVal % MAX_WIN_TOGGLES;

    if ( row < winSizes[ toggleVal ].minRow ) row = winSizes[ toggleVal ].minRow;
    if ( row > winSizes[ toggleVal ].maxRow ) row = winSizes[ toggleVal ].maxRow;

    if ( col < winSizes[ toggleVal ].minCol ) col = winSizes[ toggleVal ].minCol;
    if ( col > winSizes[ toggleVal ].maxCol ) col = winSizes[ toggleVal ].maxCol;

    winSizes[ toggleVal ].actualRow = row;
    winSizes[ toggleVal ].actualCol = col;
}

SimWinSize SimWin::getWinSize( int toggleVal ) {

    return( winSizes[ toggleVal % MAX_WIN_TOGGLES ]);
}

int  SimWin::getWinToggleVal( ) { 
    
    return ( winToggleVal ); 
}

void SimWin::setWinToggleVal( int val ) { 
    
    winToggleVal = ( val >= winToggleLimit ) ? winToggleLimit - 1 : val; 
}

void SimWin::toggleWin( int toggleVal ) { 

    winToggleVal = (winToggleVal + 1) % winToggleLimit;

    winRows    = winSizes[ winToggleVal ].actualRow; 
    winColumns = winSizes[ winToggleVal ].actualCol; 
}

//----------------------------------------------------------------------------------------
// "setWinOrigin" sets the absolute cursor position for the terminal screen. We 
// maintain absolute positions, which only may change when the terminal screen 
// is redrawn with different window sizes. The window relative rows and column 
// cursor position are set at (1,1).
//
//----------------------------------------------------------------------------------------
void SimWin::setWinOrigin( int row, int col ) {
    
    winAbsCursorRow = row;
    winAbsCursorCol = col;
    lastRowPos      = 1;
    lastColPos      = 1;
}

//----------------------------------------------------------------------------------------
// "setWinCursor" sets the cursor to a windows relative position if row and column
// are non-zero. If they are zero, the last relative cursor position is used. The 
// final absolute position is computed from the windows absolute row and column on 
// the terminal screen plus the window relative row and column. Rows and numbers 
// are values starting with 1.
//
// ??? add a check that we do not go past the window column size ?
//----------------------------------------------------------------------------------------
void SimWin::setWinCursor( int row, int col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;
  
    if ( row > MAX_WIN_ROW_SIZE ) row = MAX_WIN_ROW_SIZE;
    if ( col > MAX_WIN_COL_SIZE ) col = MAX_WIN_COL_SIZE;

    glb -> console -> setAbsCursor( winAbsCursorRow + row - 1, 
                                    winAbsCursorCol + col );
    
    lastRowPos = row;
    lastColPos = col;
}

int SimWin::getWinCursorRow( ) { 
    
    return ( lastRowPos ); 
}

int SimWin::getWinCursorCol( ) { 
    
    return ( lastColPos ); 
}

//----------------------------------------------------------------------------------------
// Fields that have a larger size than the actual argument length in the field 
// need to be padded left or right. This routine is just a simple loop emitting 
// blanks in the current format set.
//
// ??? add a check that we do not go past the window column size ?
//----------------------------------------------------------------------------------------
void SimWin::padField( int dLen, int fLen ) {
    
    while ( fLen > dLen ) {
        
        glb -> console -> writeChars((char *) " " );
        fLen --;
    }
}

//----------------------------------------------------------------------------------------
// Print out a numeric field. Each call will set the format options passed via 
// the format descriptor. If the field length is larger than the the data in the
// field, the data will be printed left or right justified in the field.
//
// ??? add a check that we do not go past the window column size ?
//----------------------------------------------------------------------------------------
void SimWin::printNumericField( T64Word     val, 
                                uint32_t    fmtDesc, 
                                int         fLen, 
                                int         row, 
                                int         col ) {
    
    if ( row == 0 )                 row = lastRowPos;
    if ( col == 0 )                 col = lastColPos;
    if ( fmtDesc & FMT_LAST_FIELD ) col = getColumns( );

    int maxLen = glb -> console -> numberFmtLen( fmtDesc, val );
   
    if ( fLen == 0 ) fLen = maxLen;

    glb -> console -> setFmtAttributes( fmtDesc );
    setWinCursor( row, col );
   
    if ( fLen > maxLen ) {
        
        if ( fmtDesc & FMT_ALIGN_LFT ) {

            glb -> console -> printNumber( val, fmtDesc );
            padField( maxLen, fLen );
        }
        else {
            
            padField( maxLen, fLen );
            glb -> console -> printNumber( val, fmtDesc );
        }
    }
    else glb -> console -> printNumber( val, fmtDesc );
    
    lastRowPos  = row;
    lastColPos  = col + fLen;
}

//----------------------------------------------------------------------------------------
// Print out a text field. Each call will set the format options passed via the 
// format descriptor. If the field length is larger than the positions needed to
// print the data in the field, the data will be printed left or right justified
// in the field. If the data is larger than the field, it will be truncated.
//
// ??? add a check that we do not go past the window column size ?
//----------------------------------------------------------------------------------------
void SimWin::printTextField( char       *text, 
                             uint32_t   fmtDesc, 
                             int        fLen, 
                             int        row, 
                             int        col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;
    
    int dLen = (int) strlen( text );
    if ( dLen > MAX_TEXT_FIELD_LEN ) dLen = MAX_TEXT_FIELD_LEN;
    
    if ( fLen == 0 ) fLen = dLen;
    if ( fmtDesc & FMT_LAST_FIELD ) col = getColumns( ) - fLen;

    setWinCursor( row, col );
    glb -> console -> setFmtAttributes( fmtDesc );
    
    if ( fLen > dLen ) {
        
        if ( fmtDesc & FMT_ALIGN_LFT ) {
            
            glb -> console -> printText( text, dLen );
            padField( dLen, fLen );
        }
        else {
            
            padField( dLen, fLen );
            glb -> console -> printText( text, dLen );
        }
    }
    else if ( fLen < dLen ) {
        
        if ( fmtDesc & FMT_TRUNC_LFT ) {
            
            glb -> console -> printText(( char *) "...", 3 );
            glb -> console -> printText( text + ( dLen - fLen ) + 3, fLen - 3 );
        }
        else {
            
            glb -> console -> printText( text, fLen - 3 );
            glb -> console -> printText(( char *) "...", 3 );
        }
    }
    else glb -> console -> printText( text, dLen );
    
    lastRowPos  = row;
    lastColPos  = col + fLen;
}

//----------------------------------------------------------------------------------------
// Print out a bit character. When the bit in the word is set, it is upper case, 
// else lower case.
//
//----------------------------------------------------------------------------------------
void SimWin::printBitField( T64Word val, 
                            int pos,
                            char printChar,
                            uint32_t fmtDesc,
                            int fLen,
                            int row,
                            int col ) {

    if ( isInRange( pos, 0, 63 )) {
        
        char buf[ 4 ];
        if (( val >> pos ) & 0x1 ) buf[ 0 ] = toupper( printChar );
        else                       buf[ 0 ] = tolower( printChar );

        buf[ 1 ] = '\0';

        printTextField( buf, fmtDesc, fLen, row, col );
    }
    else printTextField((char *) "*", fmtDesc, fLen, row, col );
}

//----------------------------------------------------------------------------------------
// It is a good idea to put the current radix into the banner line to show in what 
// format the data in the body is presented. This field is when used always printed
// as the last field in the banner line.
//
//----------------------------------------------------------------------------------------
void SimWin::printRadixField( uint32_t fmtDesc, int fLen, int row, int col ) {
    
    glb -> console -> setFmtAttributes( fmtDesc );
    
    if ( fmtDesc & FMT_LAST_FIELD ) col = getColumns( ) - fLen;

    switch ( winRadix ) {
            
        case 10: printTextField((char *) "dec", fmtDesc, 3, row, col ); break;
        case 16: printTextField((char *) "hex", fmtDesc, 3, row, col ); break;
        default: printTextField((char *) "***", fmtDesc, 3, row, col );
    }
}

//----------------------------------------------------------------------------------------
// A user defined window has a field that shows the window number as well as this 
// is the current window. We show wether it is the current window, the window stack
// and the window number.
//
//----------------------------------------------------------------------------------------
void SimWin::printWindowIdField( uint32_t fmtDesc, int row, int col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;

    bool isCurrent  = glb -> winDisplay -> isCurrentWin( winIndex );
    int  len        = 0;
    
    glb -> console -> setFmtAttributes( fmtDesc );
    
    if ( winIndex <= MAX_WINDOWS ) {

        if ( isCurrent ) len += glb -> console -> writeChars((char *) "*(" ); 
        else             len += glb -> console -> writeChars((char *) " (" );

        len += glb -> console -> writeChars((char *) "%1d:%02d)", 
                                            winStack + 1, winIndex + 1); 
    }    
    else len = glb -> console -> writeChars((char *) "(-***-)" );

    len += glb -> console -> writeChars((char *) " %.8s  ", winName );
   
    lastRowPos  = row;
    lastColPos  = col + len;
}

//----------------------------------------------------------------------------------------
// Padding a line will write a set of blanks with the current format setting to the 
// end of the line. It is intended to fill for example a banner line that is in inverse
// video with the inverse format until the end of the screen column size.
//
//----------------------------------------------------------------------------------------
void SimWin::padLine( uint32_t fmtDesc ) {
    
    glb -> console -> setFmtAttributes( fmtDesc );
    padField( lastColPos, getColumns( ));
}

//----------------------------------------------------------------------------------------
// Clear out a field.
//
//----------------------------------------------------------------------------------------
void SimWin::clearField( int len, uint32_t fmtDesc ) {
    
    int pos = lastColPos;
    
    if ( pos + len > getColumns( )) len = getColumns( ) - pos;     

    glb -> console -> setFmtAttributes( fmtDesc );
    padField( lastColPos, lastColPos + len );
    
    setWinCursor( 0,  pos );
}

//----------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The reDraw routine will invoke these 
// mandatory routines of the child classes.
//
//----------------------------------------------------------------------------------------
void SimWin::reDraw( ) {
    
    if ( winEnabled ) {
        
        drawBanner( );
        drawBody( );
    }
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the scrollable window abstract class.
//
//----------------------------------------------------------------------------------------
// Object creator / destructor.
//
//----------------------------------------------------------------------------------------
SimWinScrollable::SimWinScrollable( SimGlobals *glb ) : SimWin( glb ) { }

SimWinScrollable:: ~ SimWinScrollable( ) { }

//----------------------------------------------------------------------------------------
// Getter/Setter methods for scrollable window attributes.
//
//----------------------------------------------------------------------------------------
T64Word SimWinScrollable::getHomeItemAdr( ) { 
    
    return ( homeItemAdr ); 
}

void SimWinScrollable::setHomeItemAdr( T64Word adr ) { 
    
    homeItemAdr = adr; 
}

T64Word SimWinScrollable::getCurrentItemAdr( ) { 
    
    return ( currentItemAdr ); 
}

void SimWinScrollable::setCurrentItemAdr( T64Word adr ) { 
    
    currentItemAdr = adr; 
}

T64Word SimWinScrollable::getLimitItemAdr( ) { 
    
    return ( limitItemAdr ); 
}

void SimWinScrollable::setLimitItemAdr( T64Word adr ) { 
    
    limitItemAdr = adr; 
}

int SimWinScrollable::getLineIncrementItemAdr( ) { 
    
    return ( lineIncrement ); 
}

void SimWinScrollable::setLineIncrementItemAdr( int arg ) { 
    
    lineIncrement = arg; 
}

//----------------------------------------------------------------------------------------
// The scrollable window inherits from the general window. While the banner part 
// of a window is expected to be implemented by the inheriting class, the body is
// done by this class, which will call the "drawLine" method implemented by the 
// inheriting class. The "drawLine" method is passed the current item address which
// is the current line start of the item of whatever the window is displaying. The
// item address value is incremented by the itemsPerLine value each time the drawLine
// routine is called. The cursor position for "drawLine" method call is incremented
// by the linesPerItem amount. Note that the window system thinks in lines. 
//
// Some items fill more than one row. In this case the number of itemLines we can 
// draw is the number of rows in the window divided by rows per item line. In most 
// cases there is a one to one mapping between rows and item lines.
//
//----------------------------------------------------------------------------------------
void SimWinScrollable::drawBody( ) {
    
    int numOfItemLines = ( getRows( ) - 1 ) / rowsPerItemLine;

    for ( int line = 0; line < numOfItemLines; line++ ) {
        
        setWinCursor( line + 2, 1 );
        drawLine( currentItemAdr + ( line * lineIncrement ));
    }
}

//----------------------------------------------------------------------------------------
// The "winHome" method set the starting item address of a window within the 
// defined boundaries. An argument of zero will set the window back to the original
// home address. If the address is larger than the limit address of the window, 
// the position will be the limit address minus the number of lines times the 
// number of items on the line.
//
//----------------------------------------------------------------------------------------
void SimWinScrollable::winHome( T64Word pos ) {
    
    if ( pos > 0 ) {
        
        int itemsPerWindow = ( getRows( ) - 1 ) * lineIncrement;
        
        if ( pos > limitItemAdr - itemsPerWindow ) {
            
            homeItemAdr = limitItemAdr - itemsPerWindow;
        } 
        
        homeItemAdr       = pos;
        currentItemAdr    = pos;
    }
    else currentItemAdr = homeItemAdr;
}

//----------------------------------------------------------------------------------------
// The "winJump" method moves the starting item address of a window within the 
// boundaries zero and the limit address.
//
//----------------------------------------------------------------------------------------
void SimWinScrollable::winJump( T64Word pos ) {
    
   currentItemAdr = pos;
}

//----------------------------------------------------------------------------------------
// Window move implements the forward / backward moves of a window. The amount is 
// added to the current window body position, also making sure that we stay inside 
// the boundaries of the address range for the window. If the new position would 
// point beyond the limit address, we set the new item address to limit minus the
// window lines times the line increment. Likewise of the new item address would 
// be less than zero, we just set it to zero.
//
//----------------------------------------------------------------------------------------
void SimWinScrollable::winForward( T64Word amt ) {
    
    if ( amt == 0 ) amt = ( getRows( ) - 1 ) * lineIncrement;
    
    if (((uint64_t) currentItemAdr + amt ) > (uint64_t) limitItemAdr ) {
        
        currentItemAdr = limitItemAdr - (( getRows( ) - 1 ) * lineIncrement );
    }
    else currentItemAdr = currentItemAdr + amt;
}

void SimWinScrollable::winBackward( T64Word amt ) {
    
    if ( amt == 0 ) amt = ( getRows( ) - 1 ) * lineIncrement;
    
    if ( amt <= currentItemAdr ) {
        
        currentItemAdr = currentItemAdr - amt;
    }
    else currentItemAdr = 0;
}


//****************************************************************************************
//****************************************************************************************
//
// Object methods - SimCmdWinOutBuffer
//
//----------------------------------------------------------------------------------------
// Command window output buffer. We cannot directly print to the command window when 
// we want to support scrolling of the command window data. Instead, all printing is 
// routed to a command window buffer. The buffer is a circular structure, the oldest 
// lines are removed when we need room. When it comes to printing the window body 
// content, the data is taken from the windows output buffer.
//
//----------------------------------------------------------------------------------------
SimWinOutBuffer::SimWinOutBuffer( ) { 

    initBuffer( );
}

void SimWinOutBuffer::initBuffer( ) {
    
    for ( int i = 0; i < MAX_WIN_OUT_LINES; i++ ) buffer[i][0] = '\0';
    
    topIndex     = 0;
    cursorIndex  = 0;
    charPos      = 0;
    screenLines  = 0;
}

//----------------------------------------------------------------------------------------
// Add new data to the output buffer. Note that we do not add entire lines, but 
// rather add what ever is in the input buffer. When we encounter a "\n", the 
// current line string is terminated with the zero character and a new line is 
// started. When we are adding to the buffer, we always set the cursor to the 
// line below topIndex.
//
//----------------------------------------------------------------------------------------
void SimWinOutBuffer::addToBuffer( const char *buf ) {
    
    size_t  bufLen         = strlen( buf );
    char    *currentLine   = buffer[ topIndex ];
    
    if ( bufLen > 0 ) {
        
        for ( int i = 0; i < bufLen; i++ ) {
            
            if (( buf[ i ]  == '\n' ) || ( charPos >= MAX_WIN_OUT_LINE_SIZE - 1 )) {
                
                currentLine[ charPos ]      = '\0'; 
                charPos                     = 0;
                topIndex                    = ( topIndex + 1 ) % MAX_WIN_OUT_LINES;
                currentLine                 = buffer[topIndex];
                buffer[ topIndex ] [ 0 ]    = '\0'; 
                
            } 
            else currentLine[ charPos++ ] = buf[ i ];
        }
    }
    
    cursorIndex = ( topIndex - 1 + MAX_WIN_OUT_LINES ) % MAX_WIN_OUT_LINES;
}

//----------------------------------------------------------------------------------------
// "printChar and "printChars" will add data to the window output buffer. The 
// resulting print string is just added to the window output buffer. The actual 
// printing to screen is performed in the "drawBody" routine of the command window.
//
//----------------------------------------------------------------------------------------
int SimWinOutBuffer::writeChar( const char ch ) {
    
    char buf[ 2 ];
    buf[0] = ch;
    buf[1] = '\0';

    addToBuffer(buf);
    return 1;
}

int SimWinOutBuffer::writeChars( const char *format, ... ) {
    
    char    lineBuf[ MAX_WIN_OUT_LINE_SIZE ];
    va_list args;
    
    va_start( args, format );
    int len = vsnprintf( lineBuf, MAX_WIN_OUT_LINE_SIZE, format, args );
    va_end(args);
    
    if ( len > 0 ) {
        
        if ( len >= MAX_WIN_OUT_LINE_SIZE ) {
            
            len = MAX_WIN_OUT_LINE_SIZE - 1;
            lineBuf[ len ] = '\0';
        }
        
        addToBuffer( lineBuf );
    }
    
    return ( len );
}

//----------------------------------------------------------------------------------------
// Cursor up / down movements refer to the output line buffer. There is the top 
// index, which will always point the next output line to use in our circular 
// buffer. The cursor index is normally one below this index, i.e. pointing to the
// last active line. This is the line from which we start for example printing 
// downward to fill the command window. The scroll up function will move the cursor
// away from the top up to the oldest entry in the output line buffer. The scroll
// down function will move the cursor toward the top index. Both directions stop
// when either oldest or last entry is reached. We cannot move logically above the
// current top index, and we cannot move below the last valid line plus the current
// line display screen. This is due to the logic that we print the screen content
// from top line by line away from the top.
//
//----------------------------------------------------------------------------------------
void SimWinOutBuffer::scrollUp( int lines ) {
    
    int oldestValid   = ( topIndex - ( MAX_WIN_OUT_LINES - lines ) + MAX_WIN_OUT_LINES )
                          % MAX_WIN_OUT_LINES;

    int scrollUpLimit = ( oldestValid + screenLines ) % MAX_WIN_OUT_LINES;
    
    if ( cursorIndex != scrollUpLimit ) {
        
        cursorIndex = ( cursorIndex - lines + MAX_WIN_OUT_LINES )  % MAX_WIN_OUT_LINES;
    }
}

void SimWinOutBuffer::scrollDown( int lines ) {
    
    int lastActive = ( topIndex - lines + MAX_WIN_OUT_LINES ) % MAX_WIN_OUT_LINES;
    
    if ( cursorIndex != lastActive ) {
        
        cursorIndex = ( cursorIndex + lines ) % MAX_WIN_OUT_LINES;
    }
}

//----------------------------------------------------------------------------------------
// For printing the output buffer lines, we will get a line pointer relative to 
// the actual cursor. In the typical case the cursor is identical with the top if
// the output buffer. If it was moved, the we just get the lines from that actual
// position. The line argument is referring to the nth line below the cursor.
//
//----------------------------------------------------------------------------------------
char *SimWinOutBuffer::getLineRelative( int lineBelowTop ) {
    
    int lineToGet = ( cursorIndex + MAX_WIN_OUT_LINES - lineBelowTop ) 
                    % MAX_WIN_OUT_LINES;

    return ( &buffer[ lineToGet ][ 0 ] );
}

int SimWinOutBuffer::getCursorIndex( ) {
    
    return ( cursorIndex );
}

int SimWinOutBuffer::getTopIndex( ) {
    
    return ( topIndex );
}

void SimWinOutBuffer::resetLineCursor( ) {
    
    cursorIndex = topIndex;
}

void SimWinOutBuffer::setScrollWindowSize( int size ) {
    
    screenLines = size;
}

