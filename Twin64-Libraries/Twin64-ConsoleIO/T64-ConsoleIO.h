//----------------------------------------------------------------------------------------
//
// Twin64 - A 64-bit CPU Monitor - Console IO
//
//----------------------------------------------------------------------------------------
// Console IO is the piece of code that provides a single character interface for the 
// terminal screen. For the simulator, it is just plain character IO to the terminal 
// screen.For the simulator running in CPU mode, the characters are taken from and 
// place into the virtual console declared on the IO space.
//
// Unfortunately, PCs and Macs differ. The standard system calls typically buffer the 
// input up to the carriage return. To avoid this, the terminal needs to be place in
// "raw" mode. And this is different for the two platforms.
//
//----------------------------------------------------------------------------------------
//
// Twin64 - A 64-bit CPU Monitor - Console IO
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
#ifndef T64_ConsoleIO_h
#define T64_ConsoleIO_h

#include "T64-Common.h"

//----------------------------------------------------------------------------------------
// Format descriptor for putting out a field. The options are simply ORed. The 
// idea is that a format descriptor can be assembled once and used for many 
// fields. A value of zero will indicate to simply use the previously 
// established descriptor.
//
//  FMT_USE_ACTUAL_ATTR -> Use previously established settings.
//    
//  FMT_BG_COL_DEF      -> Set default background color setting.
//  FMT_BG_COL_RED      -> Set RED background color setting.
//  FMT_BG_COL_GREEN    -> Set GREEN background color setting.
//  FMT_BG_COL_YELLOW   -> Set YELLOW background color setting.
//    
//  FMT_FG_COL_DEF      -> Set default foreground color setting.
//  FMT_FG_COL_RED      -> Set RED foreground color setting.
//  FMT_FG_COL_GREEN    -> Set GREEN foreground color setting.
//  FMT_FG_COL_YELLOW   -> Set YELLOW foreground color setting.
//
//  FMT_DEC             -> Print numeric data as decimal "vvvv..."
//  FMT_HEX_2           -> Print numeric data as "0xvv"
//  FMT_HEX_4           -> Print numeric data as "0xvvvv"
//  FMT_HEX_8           -> Print numeric data as "0xvvvvvvvv"
//  FMT_HEX_16          -> Print numeric data as "0xvvvvvvvvvvvvvvvv"
//  FMT_HEX_2_4         -> Print numeric data as "0xvv_vvvv"
//  FMT_HEX_4_4         -> Print numeric data as "0xvvvv_vvvv"
//  FMT_HEX_2_4_4       -> Print numeric data as "0xvv_vvvv_vvvv"
//  FMT_HEX_4_4_4       -> Print numeric data as "0xvvvv_vvvv_vvvv"
//  FMT_HEX_2_4_4_4     -> Print numeric data as "0xvv_vvvv_vvvv_vvvv"
//  FMT_HEX_4_4_4_4     -> Print numeric data as "0xvvvv_vvvv_vvvv_vvvv"
//
//  FMT_BOLD            -> Set BOLD character mode.
//  FMT_BLINK           -> Set BLINK character mode.
//  FMT_INVERSE         -> Set INVERSE character mode.
//  FMT_UNDER_LINE      -> Set UNDER-LINE character mode.
//  FMT_HALF_BRIGHT     -> Set HALF-BRIGHT character mode.      
//
//  FMT_ALIGN_LFT       -> Align data left in field.
//  FMT_TRUNC_LFT       -> Align data left and optionally truncate.
//  FMT_LAST_FIELD      -> repeat last setting to end of line.
//  FMT_INVALID_NUM     -> Print numeric field as invalid number.
//  FMT_DEF_ATTR        -> Use default attributes.
//
// Note that some options are encoded in a field as a numeric value, e.g the
// number format, and some options are encoded as individual bits which can be
// used in combination. In any case, the options are ORed to form the final 
// format descriptor.
// 
//----------------------------------------------------------------------------------------
enum FmtDescOptions : uint32_t {
    
    FMT_USE_ACTUAL_ATTR = 0x0,
    
    FMT_BG_COL_DEF      = 0x00000001,
    FMT_BG_COL_RED      = 0x00000002,
    FMT_BG_COL_GREEN    = 0x00000003,
    FMT_BG_COL_YELLOW   = 0x00000004,
    
    FMT_FG_COL_DEF      = 0x00000010,
    FMT_FG_COL_RED      = 0x00000020,
    FMT_FG_COL_GREEN    = 0x00000030,
    FMT_FG_COL_YELLOW   = 0x00000040,

    FMT_HEX             = 0x00000100,
    FMT_HEX_2           = 0x00000200,
    FMT_HEX_4           = 0x00000300,
    FMT_HEX_8           = 0x00000400,
    FMT_HEX_16          = 0x00000500,
    FMT_HEX_2_4         = 0x00000600,
    FMT_HEX_4_4         = 0x00000700,
    FMT_HEX_2_4_4       = 0x00000800,
    FMT_HEX_4_4_4       = 0x00000900,
    FMT_HEX_2_4_4_4     = 0x00000A00,
    FMT_HEX_4_4_4_4     = 0x00000B00, 
    
    FMT_DEC             = 0x00001000,
    FMT_DEC_32          = 0x00002000,

    FMT_BOLD            = 0x00010000,
    FMT_BLINK           = 0x00020000,
    FMT_INVERSE         = 0x00040000,
    FMT_UNDER_LINE      = 0x00080000,
    FMT_HALF_BRIGHT     = 0x00100000,

    FMT_ALIGN_LFT       = 0x00200000,
    FMT_TRUNC_LFT       = 0x00400000,
    FMT_LAST_FIELD      = 0x00800000,

    FMT_ASCII_4         = 0x01000000,
    FMT_ASCII_8         = 0x02000000,
    FMT_PREFIX_0X       = 0x04000000,
    FMT_INVALID_NUM     = 0x08000000,
   
    FMT_DEF_ATTR        = 0x80000000
};

//----------------------------------------------------------------------------------------
// The formatter abstract class contains all the routines the generate the output 
// characters, including escape sequences and so on. It is used by the console I/O 
// written to the terminal screen but also the output window buffer used for the command
// and console window of the simulator.
//
//----------------------------------------------------------------------------------------
struct SimFormatter {

    virtual int     writeChars( const char *format, ... ) = 0;
    
    void            writeCarriageReturn( );
    void            eraseChar( );
    void            writeCursorLeft( );
    void            writeCursorRight( );
    void            writeScrollUp( int n );
    void            writeScrollDown( int n );
    void            writeCharAtLinePos( int ch, int pos );
  
    void            clearScreen( );
    void            clearLine( );
    void            clearToEndOfLine( );
    void            setAbsCursor( int row, int col );
    void            setCursorInLine( int col ); 
    void            setWindowSize( int row, int col );
    void            setScrollArea( int start, int end );
    void            clearScrollArea( );

    void            setFmtAttributes( uint32_t fmtDesc );
    int             printBlanks( int len );
    int             printText( char *text, int len );
    int             printNumber( T64Word val, uint32_t fmtDesc );
    int             numberFmtLen( uint32_t fmtDesc, T64Word val = 0 );
    char            printBit( T64Word val, int pos, char printChar );

};

//----------------------------------------------------------------------------------------
// Console IO object. The simulator is a character based interface. The typical terminal
// IO functionality such as buffered data input and output needs to be disabled. We run
// a bare bone console so to speak. There are two modes. In the first mode, the simulator
// runs and all IO is for command lines, windows and so on. When control is given to the
// CPU code, the console IO is mapped to a virtual console configured in the IO address
// space. This interface will also write and read a character at a time.
//
//----------------------------------------------------------------------------------------
struct SimConsoleIO : SimFormatter {
    
    public:
    
    SimConsoleIO( );
    ~SimConsoleIO( );
    
    void    initConsoleIO( );
    void    setBlockingMode( bool enabled );
    bool    isConsole( );
    int     getConsoleSize( int *rows, int *cols );
    int     readChar( );
    int     writeChars( const char *format, ... );
    
    private:
    
    bool    blockingMode = false;
};

#endif // T64_ConsoleIO_h
