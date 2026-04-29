//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Window subsystem
//
//----------------------------------------------------------------------------------------
// This module contains the window display routines. The window subsystem uses a 
// ton of escape sequences to create a terminal window screen and displays sub 
// windows on the screen.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Window subsystem
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
// Local name space. We try to keep utility functions local to the file. None 
// so far.
//
//----------------------------------------------------------------------------------------
namespace {

}; // namespace

//----------------------------------------------------------------------------------------
// Object constructor. We initialize the windows and create the command window. 
//
//----------------------------------------------------------------------------------------
SimWinDisplay::SimWinDisplay( SimGlobals *glb ) {
    
    this -> glb = glb;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) windowList[ i ] = nullptr;
    cmdWin = new SimCommandsWin( glb );
}

//----------------------------------------------------------------------------------------
// Get the window display system and command interpreter ready. 
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::setupWinDisplay( ) {
    
    glb -> winDisplay  -> windowDefaults( -1, -1 );
}

//----------------------------------------------------------------------------------------
// Start the window display. We start in screen mode and print the initial 
// screen. All left to do is to enter the command loop.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::startWinDisplay( ) {
    
    cmdWin -> cmdInterpreterLoop( );
}

SimTokId SimWinDisplay::getCurrentCmd( ) {
    
    return( cmdWin -> getCurrentCmd( ));
}

bool SimWinDisplay::isWinModeOn( ) {
    
    return( winModeOn );
}

void SimWinDisplay::setWinMode( bool winOn ) {
    
    winModeOn = winOn;
}

void SimWinDisplay::setWinReFormat( ) {

    winReFormatPending = true;
}

bool SimWinDisplay::isWinReFormat( ) {

    return( winReFormatPending );
}

//----------------------------------------------------------------------------------------
// The current window number defines which user window is marked "current" as the 
// default number to use in commands. Besides getting and setting the current window
// number, there are also routines that return the window type and associated
// module number.
//
//----------------------------------------------------------------------------------------
int  SimWinDisplay::getCurrentWindow( ) {
    
    return( currentWinNum );
}

void SimWinDisplay::setCurrentWindow( int winNum ) {

    if ( validWindowNum( winNum )) {
     
        previousWinNum = currentWinNum;
        currentWinNum  = winNum;
    }
    else throw( ERR_INVALID_WIN_ID );
}

SimWinType SimWinDisplay::getCurrentWinType( ) {

    if ( validWindowNum( currentWinNum )) {

        return( windowList[ currentWinNum ] -> getWinType( ));
    }
    else throw( ERR_INVALID_WIN_ID );
}

int SimWinDisplay::getCurrentWinModNum( ) {

     if ( validWindowNum( currentWinNum )) {

        return( windowList[ currentWinNum ] -> getWinModNum( ));
     } 
     else throw( ERR_INVALID_WIN_ID );
}

//----------------------------------------------------------------------------------------
// Attribute functions on window Id, stack and type. A window number is the index into
// the window list. It is valid when the number is within bounds and the window list 
// entry is actually used. Window numbers start at 1.
//
//----------------------------------------------------------------------------------------
bool SimWinDisplay::validWindowNum( int winNum ) {
    
    return(( winNum >= 0 ) && ( winNum < MAX_WINDOWS ) && 
           ( windowList[ winNum ] != nullptr ));
}

bool SimWinDisplay::validWindowStackNum( int stackNum ) {
    
    return(( stackNum >= 0 ) && ( stackNum < MAX_WIN_STACKS ));
}

bool SimWinDisplay::validWindowType( SimTokId winType ) {
    
    return( ( winType == TOK_CPU    ) ||
            ( winType == TOK_MEM    ) || 
            ( winType == TOK_TLB    ) ||
            ( winType == TOK_CODE   ) || 
            ( winType == TOK_TEXT   ));
}

char *SimWinDisplay::getWinName( int winNum ) {

    if ( validWindowNum( winNum )) {

        return( windowList[ winNum ] -> getWinName( ));
    }
    else throw( ERR_INVALID_WIN_ID );
}   

char *SimWinDisplay::getWinTypeName( int winNum ) {  

    if ( validWindowNum( winNum )) {

        switch ( windowList[ winNum ]->getWinType( )) {
        
            case WT_CMD_WIN:       return((char *) "Command" );
            case WT_CONSOLE_WIN:   return((char *) "Console" );
            case WT_TEXT_WIN:      return((char *) "Text" );
            case WT_CPU_WIN:       return((char *) "CPU" );
            case WT_TLB_WIN:       return((char *) "TLB" );
            case WT_MEM_WIN:       return((char *) "Memory" );
            case WT_CODE_WIN:      return((char *) "Code" );
            
            default:               return((char *) "N/A" );
        }
    }
    else throw( ERR_INVALID_WIN_ID );
}

int SimWinDisplay::getWinStackNum( int winNum ) {

    return(( validWindowNum( winNum )) ? windowList[ winNum ] -> getWinStack( ) : -1 );
}

int SimWinDisplay::getWinModNum( int winNum ) {

    return(( validWindowNum( winNum )) ? windowList[ winNum ] -> getWinModNum( ) : -1 );
}

bool SimWinDisplay::isCurrentWin( int winNum ) {
    
    return(( validWindowNum( winNum ) && ( currentWinNum == winNum )));
}

bool SimWinDisplay::isScrollableWin ( int typ ) {

        return (( typ == WT_MEM_WIN     ) ||
                ( typ == WT_CODE_WIN    ) ||
                ( typ == WT_TLB_WIN     ) ||        
                ( typ == WT_TEXT_WIN    ));
}

bool SimWinDisplay::isWinEnabled( int winNum ) {

    if ( winNum == -1 ) winNum = getCurrentWindow( );
    return(( validWindowNum( winNum )) && ( windowList[ winNum ] -> isEnabled( )));
}

bool SimWinDisplay::isWindowsOn( ) {

    return( winModeOn );
}

//----------------------------------------------------------------------------------------
// Before drawing the screen content after the execution of a command line, we 
// compute the number of columns needed for a stack of windows. This function just 
// runs through the window list for a given stack and determines the widest default
// column of a window.
//
//----------------------------------------------------------------------------------------
int SimWinDisplay::computeColumnsNeeded( int winStack ) {
    
    int columnSize = 0;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            int columns = windowList[ i ] -> getDefColumns( );
            if ( columns > columnSize ) columnSize = columns;
        }
    }
    
    return( columnSize );
}

//----------------------------------------------------------------------------------------
// Once we know the maximum column size needed for the active windows in a stack, we 
// need to set this size in all those windows, so that they print nicely with a common 
// end of line picture.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::setWindowColumns( int winStack, int columnSize ) {
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            windowList[ i ] -> setColumns( columnSize );
        }
    }
}

//----------------------------------------------------------------------------------------
// Before drawing the screen content after the execution of a command line, we need to
// check whether the number of rows needed for a stack of windows has changed. This 
// function just runs through the window list and sums up the rows needed for a given
// stack.
//
//----------------------------------------------------------------------------------------
int SimWinDisplay::computeRowsNeeded( int winStack ) {
    
    int rowSize = 0;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            rowSize += windowList[ i ] -> getRows( );
        }
    }
    
    return( rowSize );
}

//----------------------------------------------------------------------------------------
// Content for each window is addressed in a window relative way. For this scheme 
// to work, each window needs to know the absolute position within in the overall
// screen. This routine will compute for each window of the passed stack the 
// absolute row and column position for the window in the terminal screen.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::setWindowOrigins( int winStack, int rowOffset, int colOffset ) {
    
    int tmpRow = rowOffset;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            windowList[ i ] -> setWinOrigin( tmpRow, colOffset );
            tmpRow += windowList[ i ] -> getRows( );
        }
        
        cmdWin -> setWinOrigin( tmpRow, colOffset );
    }
}

//----------------------------------------------------------------------------------------
// Window screen drawing. This routine is perhaps the heart of the window system. 
// Each time we read in a command input, the terminal screen must be updated. A 
// terminal screen consist of a list of stacks and in each stack a list of windows. 
// 
// Only if we have windows assigned to another stack and at least one of the windows
// in that stack is enabled, will this stack show up in the terminal screen. 
//
// We first compute the number of rows and columns needed for all windows to show 
// in their assigned stack. Only enabled screens will participate in the overall
// screen size computation. Within a stack, the window with the largest columns
// needed determines the stack column size. The total number of columns required
// is the sum of the columns computed per stack plus a margin between the stacks.  
// If the columns needed is less than the minimum columns for the command window, 
// we correct the column sizes of all windows to match the command window default.
//
// Rows are determined by adding the required rows of all windows in a given stack.
// The final number is the rows needed by the largest stack plus the rows needed
// for the command window. The data is used to set the absolute origin coordinates
// of each window.
//
// The overall screen size is at least the column and row numbers computed. If the
// number of rows required for the windows and command window is less than the 
// defined minimum number of rows, the command window is enlarged accordingly.  
// When the screen size changed, we just redraw the screen with the command screen
// going last. The command screen will have a columns size across all visible stacks.
//
// If the terminal screen is not large enough to hold all windows, we set a flag
// to reformat the screen on the next command input. This gives the terminal a
// chance to resize itself. However, the resetting of the terminal size is only
// done after the next command input.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::reDraw( ) {
    
    int winStackColumns[ MAX_WIN_STACKS ]   = { 0 };
    int winStackRows[ MAX_WIN_STACKS ]      = { 0 };
    int stackCount                          = 0;
    int maxRowsNeeded                       = 0;
    int maxColumnsNeeded                    = 0;
    int stackColumnGap                      = 2;
    int minRowSize = glb -> env -> getEnvVarInt((char *) ENV_WIN_MIN_ROWS );
    
    if ( winModeOn ) {
       
        for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {
            
            winStackColumns[ i ] = computeColumnsNeeded( i );
            winStackRows[ i ]    = computeRowsNeeded( i );

            if ( winStackColumns[ i ] > 0 ) {

                    maxColumnsNeeded += winStackColumns[ i ];
                    maxColumnsNeeded += stackColumnGap;
                    stackCount ++;
                }
                
                if ( winStackRows[ i ] > maxRowsNeeded ) 
                    maxRowsNeeded = winStackRows[ i ];
        }

        if ( maxColumnsNeeded > 0 ) maxColumnsNeeded -= stackColumnGap;

        if ( maxColumnsNeeded < cmdWin -> getDefColumns( )) {

            maxColumnsNeeded = cmdWin -> getDefColumns( );
        }

        if ( stackCount == 1 ) {

            for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {

                if (( winStackColumns[ i ] > 0 ) && 
                    ( winStackColumns[ i ] < maxColumnsNeeded ))
                    winStackColumns[ i ] = maxColumnsNeeded;
            }
        }  

        int curColumn = 1;
        int curRows   = 1;
        
        for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {

            setWindowColumns( i, winStackColumns[ i ] );
            setWindowOrigins( i, curRows, curColumn );

            curColumn += winStackColumns[ i ];
            if (( curColumn > 1 ) && ( winStackColumns[ i ] > 0 ))
               curColumn += stackColumnGap;
        }
        
        if (( maxRowsNeeded + cmdWin -> getRows( )) < minRowSize ) {
            
            cmdWin -> setRows( minRowSize - maxRowsNeeded );
            maxRowsNeeded += cmdWin -> getRows( );
        }
        else maxRowsNeeded += cmdWin -> getRows( );
        
        cmdWin -> setColumns( maxColumnsNeeded ); 
        cmdWin -> setWinOrigin( maxRowsNeeded - cmdWin -> getRows( ) + 1, 1 );
    }
    else {
        
        maxRowsNeeded       = cmdWin -> getDefRows( );
        maxColumnsNeeded    = cmdWin -> getDefColumns( );
        
        cmdWin -> setWinOrigin( 1, 1 );
    }

    int actualRows = 0;
    int actualCols = 0;
    glb -> console -> getConsoleSize( &actualRows, &actualCols );

    if  (( actualRows < maxRowsNeeded ) || ( actualCols < maxColumnsNeeded )) {

        winReFormatPending = true;
    }
   
    if ( winReFormatPending ) {
       
        glb -> console -> setWindowSize( maxRowsNeeded, maxColumnsNeeded );
        glb -> console -> setAbsCursor( 1, 1 );
        glb -> console -> clearScrollArea( );
        glb -> console -> clearScreen( );
        
        if ( winModeOn )
            glb -> console -> setScrollArea( maxRowsNeeded - 1, maxRowsNeeded );
        else
            glb -> console -> setScrollArea( 2, maxRowsNeeded );
    }
    
    if ( winModeOn ) {

        for ( int i = 0; i < MAX_WINDOWS; i++ ) {
            
            if (( windowList[ i ] != nullptr ) && 
                ( windowList[ i ] -> isEnabled( ))) windowList[ i ] -> reDraw( );
        }
    }
    
    cmdWin -> reDraw( );
    glb -> console -> setAbsCursor( maxRowsNeeded, 1 );
    winReFormatPending = false;
}

//----------------------------------------------------------------------------------------
// The entry point to showing windows is to draw the screen on the "windows on" 
// command and to clean up when we switch back to line mode. 
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowsOn( ) {

    winModeOn = true;
    setWinReFormat( );
}

void SimWinDisplay::windowsOff( ) {

    if ( winModeOn ) {

        winModeOn = false;
        glb -> console -> clearScrollArea( );
        glb -> console -> clearScreen( );
    
        cmdWin -> setDefaults( );
        setWinReFormat( );
    }
}

//----------------------------------------------------------------------------------------
// The window defaults method will set a range of windows to their preconfigured
// default value. This is quite useful when we messed up our screens. Also, if the
// screen is displayed garbled after some windows mouse based screen window changes, 
// just do WON again to set it straight. The command window is reset in any case.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowDefaults( int winNumStart, int winNumEnd ) {

    if (( winNumStart < 0 ) || ( winNumEnd >= MAX_WINDOWS ))  return;
    if ( winNumStart > winNumEnd ) winNumEnd = winNumStart;

    if (( ! validWindowNum( winNumStart )) && ( ! validWindowNum( winNumEnd ))) 
        throw ( ERR_INVALID_WIN_ID );
    
    for ( int i = winNumStart; i <= winNumEnd; i++ ) {

        if ( windowList[ i ] != nullptr ) windowList[ i ] -> setDefaults( );
    }

    cmdWin -> setDefaults( );
}

//----------------------------------------------------------------------------------------
// "winStacksEnable" enables or disabled a stack. This allows to move windows to a
// stack and then show all of them by referring to their stack number. A stack number
// of -1 means all stacks. 
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::winStacksEnable( int stackNum, bool enable ) {

    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );

    for ( int i = 0; i < MAX_WINDOWS; i++ ) {

        SimWin *wPtr = windowList[ i ];

        if ( wPtr != nullptr ) {

            if ( stackNum == -1 ) {
                
                wPtr -> setEnable( enable );
            }
            else {

                if ( wPtr -> getWinStack( ) == stackNum ) 
                    wPtr -> setEnable( enable );
            }
        }
    }
}

//----------------------------------------------------------------------------------------
// The routine sets the stack attribute for a user window. The setting is not 
// allowed for the predefined window. They are always in the main window stack, 
// which has the stack Id of one externally. Theoretically we could have many 
// stacks, numbered 1 to MAX_STACKS. Realistically, 2 to 3 stacks will fit on a 
// screen. The last window moved to a stack is made the current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowSetStack( int winStack, int winNumStart, int winNumEnd ) {

    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNumStart < 0 ) throw ( ERR_INVALID_WIN_ID );
    if ( winNumEnd >= MAX_WINDOWS ) throw ( ERR_INVALID_WIN_ID );
   
    for ( int i = winNumStart; i <= winNumEnd; i++ ) {
        
        if ( windowList[ i ] != nullptr ) {

            windowList[ i ] -> setWinStack( winStack );
        }
    }

    previousWinNum = currentWinNum;
    currentWinNum  = winNumEnd;
}

//----------------------------------------------------------------------------------------
// A window can be added to or removed from the window list shown. We are passed 
// an optional windows number, which is used when there are user defined windows
// for locating the window object. If we remove a window and the window is the
// current window, we select the previous current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowEnable( int winNumStart, int winNumEnd, bool enable ) {
    
    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNumStart < 0 ) throw ( ERR_INVALID_WIN_ID );
    if ( winNumEnd >= MAX_WINDOWS ) throw ( ERR_INVALID_WIN_ID );
   
    for ( int i = winNumStart; i <= winNumEnd; i++ ) {

        if ( windowList[ i ] != nullptr ) {

            if (( i == currentWinNum ) && ( ! enable )) {

                currentWinNum = previousWinNum;
            }

            windowList[ i ] -> setEnable( enable );
        } 
    }
}

//----------------------------------------------------------------------------------------
// For the numeric values in a window, we can set the radix. The token ID for the 
// format option is mapped to the actual radix value.We are passed an optional 
// windows number, which is used when there are user defined windows for locating 
// the window object. Changing the radix potentially means that the window layout
// needs to change. In case of a user window, the window is made the current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowRadix( int rdx, int winNum ) {

    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNum == -1 ) winNum = currentWinNum;
    if ( ! validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
         
    windowList[ winNum ] -> setRadix( rdx );
    
    setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// "setRows" is the method to set the number if lines in a window. The number 
// includes the banner. We are passed an optional windows number, which is used 
// when there are user defined windows for locating the window object. The window
// is made the current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowSetRows( int rows, int winNum ) {

    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNum == -1 ) winNum = currentWinNum;
    if ( ! validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
      
    if ( rows == 0 ) rows = windowList[ winNum ] -> getRows( );
                
    windowList[ winNum ] -> setRows( rows );

    previousWinNum = currentWinNum;
    currentWinNum  = winNum;

    setWinReFormat( );
}

void SimWinDisplay::windowSetCmdWinRows( int rows ) {

    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );

    if ( rows == 0 ) rows = cmdWin -> getRows( );
    cmdWin -> setRows( rows );

    setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// The command window can be cleared with the "windowClearCmdWin" method. This
// will reset the content of the command window to empty.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowClearCmdWin( ) {      

    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );

   cmdWin -> clearCmdWin( );
    setWinReFormat( );
}

//----------------------------------------------------------------------------------------
// "winHome" will set the current position to the home index, i.e. the position 
// with which the window was cleared. If the position passed is non-zero, it will
// become the new home position. The position meaning is window dependent and the
// actual window will sort it out. We are passed an optional windows number, which
// is used when there are user defined windows for locating the window object. 
// The window is made the current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowHome( int pos, int winNum ) {
    
    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNum == -1 ) winNum = getCurrentWindow( );
    if ( ! validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );

    ((SimWinScrollable *) windowList[ winNum ] ) -> winHome( pos );

    previousWinNum = currentWinNum;
    currentWinNum  = winNum;
}

//----------------------------------------------------------------------------------------
// A window is scrolled forward with the "windowForward" method. The meaning of 
// the amount is window dependent and the actual window will sort it out. We are
// passed an optional windows number, which is used when there are user defined 
// windows for locating the window object. The window is made the current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowForward( int amt, int winNum ) {
    
    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNum == -1 ) winNum = getCurrentWindow( );
    if ( ! validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );

    if (SimWinScrollable* w = dynamic_cast<SimWinScrollable*>( windowList[ winNum ])) {

        w -> winForward( amt ); 

        previousWinNum = currentWinNum;
        currentWinNum  = winNum;
    } 
    else throw ( ERR_INVALID_WIN_ID );
}

//----------------------------------------------------------------------------------------
// A window is scrolled backward with the "windowBackward" methods. The meaning 
// of the amount is window dependent and the actual window will sort it out. We
// are passed an optional windows number, which is used when there are user 
// defined windows for locating the window object. The window is made the current
// window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowBackward( int amt, int winNum ) {
    
    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNum == -1 ) winNum = getCurrentWindow( );
    if ( ! validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );

    if (SimWinScrollable* w = dynamic_cast<SimWinScrollable*>( windowList[ winNum ])) {

        w -> winBackward( amt ); 

        previousWinNum = currentWinNum;
        currentWinNum  = winNum;
    } 
    else throw ( ERR_INVALID_WIN_ID );
}

//----------------------------------------------------------------------------------------
// The current index can also directly be set to another location. The position
// meaning is window dependent and the actual window will sort it out. The window
// is made the current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowJump( int pos, int winNum ) {
    
    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNum == -1 ) winNum = getCurrentWindow( );
    if ( ! validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
   
    if (SimWinScrollable* w = dynamic_cast<SimWinScrollable*>( windowList[ winNum ])) {

        w ->  winJump( pos ); 

        previousWinNum = currentWinNum;
        currentWinNum  = winNum;
    } 
    else throw ( ERR_INVALID_WIN_ID );
}

//----------------------------------------------------------------------------------------
// The current window index can also directly be set to another location. The 
// position meaning is window dependent and the actual window will sort it out. 
// The window is made the current window.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowToggle( int winNum, int toggleVal ) {
    
    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( winNum == -1 ) winNum = getCurrentWindow( );
    if ( ! validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
   
    windowList[ winNum ] -> toggleWin( toggleVal );

    previousWinNum = currentWinNum;
    currentWinNum  = winNum;
}

//----------------------------------------------------------------------------------------
// The display order of the windows is determined by the window index. It would 
// however be convenient to modify the display order. The window exchange command
// will exchange the current window with the window specified by the index of
// another window. If the windows are from different window stacks, we also 
// exchange the winStack.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowExchangeOrder( int winNum ) {
    
    if ( ! winModeOn ) throw( ERR_NOT_IN_WIN_MODE );
    if ( ! validWindowNum( winNum )) return;

    int currentWindow = getCurrentWindow( );
    if ( winNum == currentWindow ) return;

    int winStackA = windowList[ winNum ] -> getWinStack( );
    int winStackB = windowList[ currentWindow ] -> getWinStack( );

    if ( winStackA != winStackB ) {

        windowList[ winNum ] -> setWinStack( winStackB );
        windowList[ currentWindow ] -> setWinStack( winStackA );
    }
   
    std::swap( windowList[ winNum ], windowList[ currentWindow ]);
}

//----------------------------------------------------------------------------------------
// "Window New" family of routines create a new window for certain window type. 
// The newly created window also becomes the current user window. The window 
// number is stored from 1 to MAX, the initial stack number is zero.
//
//----------------------------------------------------------------------------------------
int SimWinDisplay::getFreeWindowSlot( ) {

    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if ( windowList[ i ] == nullptr ) return( i );
    }

    throw( ERR_OUT_OF_WINDOWS );
}

void SimWinDisplay::windowNewAbsMem( int modNum, T64Word adr ) {

    int slot = getFreeWindowSlot( );

    windowList[ slot ] = (SimWin *) new SimWinAbsMem( glb, modNum, adr );
    windowList[ slot ] -> setWinName(( char *) "MEM" );
    windowList[ slot ] -> setDefaults( );
    windowList[ slot ] -> setWinIndex( slot );
    windowList[ slot ] -> setWinStack( 0 );
    windowList[ slot ] -> setEnable( true );
    currentWinNum = slot;
}

void SimWinDisplay::windowNewAbsCode( int modNum, T64Word adr ){

    int slot = getFreeWindowSlot( );

    windowList[ slot ] = (SimWin *) new SimWinCode( glb, modNum, adr ); 
    windowList[ slot ] -> setWinName(( char *) "CODE" );
    windowList[ slot ] -> setDefaults( );
    windowList[ slot ] -> setWinIndex( slot );
    windowList[ slot ] -> setWinStack( 0 );
    windowList[ slot ] -> setEnable( true );
    currentWinNum = slot;
}

void SimWinDisplay::windowNewCpuState( int modNum ) {

    int slot = getFreeWindowSlot( );

    windowList[ slot ] = (SimWin *) new SimWinCpuState( glb, modNum  );
    windowList[ slot ] -> setWinName(( char *) "CPU" );
    windowList[ slot ] -> setWinModNum( modNum );
    windowList[ slot ] -> setDefaults( );
    windowList[ slot ] -> setWinIndex( slot );
    windowList[ slot ] -> setWinStack( 0 );
    windowList[ slot ] -> setEnable( true );
    currentWinNum = slot;
}

void SimWinDisplay::windowNewTlb( int modNum, T64TlbKind tKind ) {

    int slot = getFreeWindowSlot( );

     T64ModuleType mType = glb -> system -> getModuleType( modNum );
    if ( mType != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );

    if ( tKind == T64_TK_UNIFIED_TLB ) {

        windowList[ slot ] = 
            (SimWin *) new SimWinTlb( glb, modNum, proc -> getTlbPtr( ));
        windowList[ slot ] -> setWinName(( char *) "U-TLB" );
    }
    else throw( ERR_INVALID_WIN_TYPE );

    windowList[ slot ] -> setDefaults( );
    windowList[ slot ] -> setWinIndex( slot );
    windowList[ slot ] -> setWinStack( 0 );
    windowList[ slot ] -> setEnable( true );
    currentWinNum = slot;
}

// ??? will go out ...
void SimWinDisplay::windowNewCache( int modNum, T64CacheKind cType ) {

    T64ModuleType mType = glb -> system -> getModuleType( modNum );
    if ( mType != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );

    #if 0 // ???

    int slot = getFreeWindowSlot( );

    if ( cType == T64_CK_INSTR_CACHE ) {

        windowList[ slot ] = 
            (SimWin *) new SimWinCache( glb, modNum, proc -> getICachePtr( ));
        
        windowList[ slot ] -> setWinName(( char *) "I-CACHE" );
    }
    else if ( cType == T64_CK_DATA_CACHE ) {
        
        windowList[ slot ] = 
            (SimWin *) new SimWinCache( glb, modNum, proc -> getDCachePtr( ));
        
        windowList[ slot ] -> setWinName(( char *) "D-CACHE" );
    }
    else throw( ERR_INVALID_WIN_TYPE );

    windowList[ slot ] -> setDefaults( );
    windowList[ slot ] -> setWinIndex( slot );
    windowList[ slot ] -> setWinStack( 0 );
    windowList[ slot ] -> setEnable( true );
    currentWinNum = slot;

    #endif
}
   
void SimWinDisplay::windowNewText( char *pathStr ) {

    int slot = getFreeWindowSlot( );

    windowList[ slot ] = (SimWin *) new SimWinText( glb, pathStr );
    windowList[ slot ] -> setWinName(( char *) "TEXT" );
    windowList[ slot ] -> setDefaults( );
    windowList[ slot ] -> setWinIndex( slot );
    windowList[ slot ] -> setWinStack( 0 );
    windowList[ slot ] -> setEnable( true );
    currentWinNum = slot;
}

//----------------------------------------------------------------------------------------
// "Window Kill" is the counter part to user windows creation. The method supports 
// removing a range of user windows. When the start is greater than the end, the 
// end is also set to the start window Id. When we kill a window that was the 
// current window, we need to set a new one. We just pick the first used entry 
// in the user range.
//
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowKill( int winNumStart, int winNumEnd ) {

    if ( winNumStart < 0 ) throw ( ERR_INVALID_WIN_ID );
    if ( winNumEnd >= MAX_WINDOWS ) throw ( ERR_INVALID_WIN_ID );
    
    for ( int i = winNumStart; i <= winNumEnd; i++ ) {

        delete ( SimWin * ) windowList[ i ];
        windowList[ i ] = nullptr;
                
        if ( currentWinNum == i ) {
                 
            for ( int i = 1; i < MAX_WINDOWS; i++ ) {
                        
                if ( validWindowNum( i )) {
                            
                    currentWinNum = i;
                    break;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------
// When we remove a window, we need to also remove all windows associated with a 
// module. This is what the "windowKillByModNum" method does. It runs through the
// window list and removes all windows that are associated with the passed module
// number.
//----------------------------------------------------------------------------------------
void SimWinDisplay::windowKillByModNum( int modNum ) {      

    for ( int i = 0; i < MAX_WINDOWS; i++ ) {

        if (( windowList[ i ] != nullptr ) && 
            ( windowList[ i ] -> getWinModNum( ) == modNum )) {

            delete ( SimWin * ) windowList[ i ];
            windowList[ i ] = nullptr;
                
            if ( currentWinNum == i ) {
                 
                for ( int i = 1; i < MAX_WINDOWS; i++ ) {
                        
                    if ( validWindowNum( i )) {
                            
                        currentWinNum = i;
                        break;
                    }
                }
            }
        }
    }
}   