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
// // Twin64 - A 64-bit CPU Monitor - Console IO
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
#include "T64-Common.h"
#include "T64-Util.h"
#include "T64-ConsoleIO.h"

//----------------------------------------------------------------------------------------
// Local name space.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// The previous terminal attribute settings. We restore them when the console IO object
// is deleted.
//
//----------------------------------------------------------------------------------------
#if __APPLE__
struct termios saveTermSetting;
#endif

//----------------------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------------------
char outputBuffer[ 1024 ];

//----------------------------------------------------------------------------------------
// Sometimes we need to delay a little, and sure enough WIN and Mac have different 
// routines to do so.
//
//----------------------------------------------------------------------------------------
#ifdef _WIN32
#include <windows.h>
#define sleepMilliSeconds (ms ) Sleep( ms )
#else
#include <unistd.h>
#define sleepMilliSeconds( ms ) usleep(( ms ) * 1000 )
#endif

}; // namespace


//----------------------------------------------------------------------------------------
// Object constructor. We will save the current terminal settings.
//
//----------------------------------------------------------------------------------------
SimConsoleIO::SimConsoleIO( ) {
  
#if __APPLE__
    tcgetattr( fileno( stdin ), &saveTermSetting );
#endif
    
}

SimConsoleIO::~SimConsoleIO( ) {
    
#if __APPLE__
    tcsetattr( fileno( stdin ), TCSANOW, &saveTermSetting );
#endif
    
}

//----------------------------------------------------------------------------------------
// The Simulator works in raw character mode. This is to support basic editing features
// and IO to the simulator console window when the simulation is active. There is a 
// price to pay in that there is no nice buffering of input and basic line editing 
// capabilities. On Mac/Linux the terminal needs to be set into raw character mode. On 
// windows, this seems to work without special setups. Hmm, anyway. This routine will
// set the raw mode attributes. For a windows system, these methods are a no operation.
//
// There is also a non-blocking IO mode. When the simulator hands over control to the 
// CPU, the console IO is mapped to the PDC console driver and output is directed to
// the console window. The console IO becomes part of the periodic processing and a key
// pressed will set the flags in the PDC console driver data. We act as"true" hardware.
// Non-blocking mode is enabled on entry to single step and run command and disabled 
// when we are back to the monitor.
//
//
// ??? perhaps a place to save the previous settings and restore them ?
//----------------------------------------------------------------------------------------
void SimConsoleIO::initConsoleIO( ) {

#if __APPLE__
    struct termios term;
    tcgetattr( fileno( stdin ), &term );
    term.c_lflag &= ~ ( ICANON | ECHO );
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr( STDIN_FILENO, TCSANOW, &term );
    tcflush( fileno( stdin ), TCIFLUSH );
#endif
    
    blockingMode  = true;
}

//----------------------------------------------------------------------------------------
// "isConsole" is used by the command interpreter to figure whether we have a true 
// terminal or just read from a file.
//
//----------------------------------------------------------------------------------------
bool  SimConsoleIO::isConsole( ) {
    
    #if __APPLE__
    return( isatty( fileno( stdin )));
    #else
    return( _isatty( _fileno( stdin )));
    #endif
}

//----------------------------------------------------------------------------------------
// "getConsoleSize" will return the current size of the terminal screen in rows
// and columns. Of course there are platform differences.
//
//----------------------------------------------------------------------------------------
int  SimConsoleIO::getConsoleSize( int *rows, int *cols ) {
    
    #if __APPLE__

    struct winsize w;
    if ( ioctl( STDOUT_FILENO, TIOCGWINSZ, &w ) == -1 ) {

        *rows = 24;
        *cols = 80;
        return ( -1 );
    }
    else {

        *rows = w.ws_row;
        *cols = w.ws_col;
        return ( 0 );
    }

    #else
    
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if ( GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi ) ) {

        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return ( 0 );
    }
    else {

        *rows = 24;
        *cols = 80;
        return ( -1 );
    }
    
    #endif
}

//----------------------------------------------------------------------------------------
// "setBlockingMode" will put the terminal into blocking or non-blocking mode. For
// the command interpreter we will use the blocking mode, i.e. we wait for character
// input. When the CPU runs, the console IO must be in non-blocking, and we check 
// for input on each CPU "tick".
//
//----------------------------------------------------------------------------------------
void SimConsoleIO::setBlockingMode( bool enabled ) {
    
#if __APPLE__
    
    int flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
    if ( flags == -1 ) {
        
        // ??? error ....
    }
    else {
        
        if ( enabled )  flags &= ~O_NONBLOCK;
        else            flags |= O_NONBLOCK;
        
        if ( fcntl( STDIN_FILENO, F_SETFL, flags ) == -1 ) {
               
            // ??? error ....
        }
    }
#endif
    
    blockingMode = enabled;
}

//----------------------------------------------------------------------------------------
// "readConsoleChar" is the single entry point to get a character from the terminal
// input. On Mac/Linux, this is the "read" system call. Whether the mode is blocking
// or non-blocking is set in the terminal settings. The read function is the same. 
// If there is no character available, a zero is returned, otherwise the character.
//
// On Windows there is a similar call, which does just return one character at a 
// time. However, there seems to be no real waiting function. Instead, the "_kbhit" 
// tests for a keyboard input. In blocking mode, we will loop for a keyboard input
// and then get the character. In non-blocking mode, we test the keyboard and return
// either the character typed or a zero.
//
// On Windows, we delay a little to avoid a busy loop.
// 
//----------------------------------------------------------------------------------------
int SimConsoleIO::readChar( ) {
    
#if __APPLE__
    char ch;
    if ( read( STDIN_FILENO, &ch, 1 ) == 1 ) return( ch );
    else return ( 0 );
#else
    if ( blockingMode ) {
        
        while ( ! _kbhit( )) Sleep( 50 );
        return( _getch( ));
    }
    else {
        
        if ( _kbhit( )) {
        
            int ch = _getch();
            return ( ch );
        }
        else return( 0 );
    }
#endif
    
}

//----------------------------------------------------------------------------------------
// "writeChars" is the single entry point to write to the terminal. On Mac/Linux, 
// we still try to send out the data in batches to the terminal emulator for better
// stability. In Windows this does not seems to be an issue, we send a single char
// at a time.
//
// I still get from time time garbled screens, which disappear on a redraw. Perhaps
// the single character write logic is too much for the terminal driver. So, let's
// accumulate larger sequences and only use "writeChars" for printing.
//
//----------------------------------------------------------------------------------------
int SimConsoleIO::writeChars( const char *format, ... ) {

    va_list args;
    va_start( args, format );
    int len = vsnprintf( outputBuffer, sizeof( outputBuffer ), format, args );
    va_end( args );

     if (len <= 0) return 0;

    #if __APPLE__ || __linux__

    const char  *p          = outputBuffer;
    size_t      remaining   = len;

    while ( remaining > 0   ) {

        ssize_t n = write( STDOUT_FILENO, p, remaining );

        if (( n < 0 ) && ( errno == EINTR )) continue;
        if ( n <= 0 ) break;
        
        p         += n;
        remaining -= n;
    }

    tcdrain( STDOUT_FILENO );
   
    #else

    for (int i = 0; i < len; i++) {

        _putch((unsigned char)outputBuffer[i]);
    }

    #endif

    return len;
}

//****************************************************************************************
//****************************************************************************************
//
// Console IO Formatter routines.
//
//----------------------------------------------------------------------------------------
// The Simulator features two distinct ways of text output. The first is the console
// I/O where a character will make its way directly to the terminal screen of the 
// application. The second output mode is a buffered I/O where all characters are 
// placed in an output buffer consisting of lines of text. When the command or console
// window is drawn, the text is taken form the this output buffer. Common to both
// output methods is the formatter.
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Escape code functions.
//
//----------------------------------------------------------------------------------------
void SimFormatter::eraseChar( ) {
    
    writeChars( "\033[D \033[P" );
}

void SimFormatter::writeCursorLeft( ) {
    
    writeChars( "\033[D" );
}

void SimFormatter::writeCursorRight( ) {
    
    writeChars( "\033[C" );
}

void SimFormatter::writeScrollUp( int n ) {
    
    writeChars( "\033[%dS", n );
}

void SimFormatter::writeScrollDown( int n ) {
    
    writeChars( "\033[%dT", n );
}

void SimFormatter::writeCarriageReturn( ) {

    #if __APPLE__
        writeChars( "\n" );
    #else 
        writeChars( "\r\n" );
    #endif
}

void SimFormatter::writeCharAtLinePos( int ch, int pos ) {
    
    writeChars( "\033[%dG\033[1@%c", pos, ch );
}

void SimFormatter::clearScreen( ) {
    
    writeChars((char *) "\x1b[2J" );
    writeChars((char *) "\x1b[3J" );
}

void SimFormatter::clearLine( ) {
    
    writeChars((char *) "\x1b[2K" );
}

void SimFormatter::clearToEndOfLine( ) {
    
    writeChars((char *) "\x1b[K" );
}

void SimFormatter::setAbsCursor( int row, int col ) {
    
    writeChars((char *) "\x1b[%d;%dH", row, col );
}

void SimFormatter::setCursorInLine( int col ) {
    
    writeChars((char *) "\x1b[%dG", col );
}

void SimFormatter::setWindowSize( int row, int col ) {
    
    writeChars((char *) "\x1b[8;%d;%dt", row, col );
}

void SimFormatter::setScrollArea( int start, int end ) {
    
    writeChars((char *) "\x1b[%d;%dr", start, end );
}

void SimFormatter::clearScrollArea( ) {
    
    writeChars((char *) "\x1b[r" );
}

//----------------------------------------------------------------------------------------
// Console output is also used to print out window forms. A window will consist of lines
// with lines having fields on them. A field has a set of attributes such as foreground
// and background colors, bold characters and so on. This routine sets the attributes 
// based on the format descriptor. If the descriptor is zero, we will just stay where 
// are with their attributes.
//
//----------------------------------------------------------------------------------------
void SimFormatter::setFmtAttributes( uint32_t fmtDesc ) {
    
    if ( fmtDesc != 0 ) {
        
        writeChars((char *) "\x1b[0m" );
        if ( fmtDesc & FMT_BOLD )           writeChars((char *) "\x1b[1m" );
        if ( fmtDesc & FMT_HALF_BRIGHT )    writeChars((char *) "\x1b[2m" );
        if ( fmtDesc & FMT_UNDER_LINE )     writeChars((char *) "\x1b[4m" );
        if ( fmtDesc & FMT_BLINK )          writeChars((char *) "\x1b[5m" );
        if ( fmtDesc & FMT_INVERSE )        writeChars((char *) "\x1b[7m" );
        
        switch ( fmtDesc & 0xF ) { // BG Color
                
            case 1:     writeChars((char *) "\x1b[41m"); break;
            case 2:     writeChars((char *) "\x1b[42m"); break;
            case 3:     writeChars((char *) "\x1b[43m"); break;
            default:    writeChars((char *) "\x1b[49m");
        }
        
        switch (( fmtDesc >> 4 ) & 0xF ) { // FG Color
                
            case 1:     writeChars((char *) "\x1b[31m"); break;
            case 2:     writeChars((char *) "\x1b[32m"); break;
            case 3:     writeChars((char *) "\x1b[33m"); break;
            default:    writeChars((char *) "\x1b[39m");
        }
    }
}

//----------------------------------------------------------------------------------------
// Just emit blanks.
//
//----------------------------------------------------------------------------------------
int SimFormatter::printBlanks( int len ) {

    for ( int i = 0; i < len; i++ ) writeChars((char *) " " );
    return( len );
}

//----------------------------------------------------------------------------------------
// Routine for putting out simple text. We make sure that the string length is in the
// range of what the text size could be.
//
//----------------------------------------------------------------------------------------
int SimFormatter::printText( char *text, int maxLen ) {
    
    if ( strlen( text ) <= maxLen ) {
        
        return( writeChars((char *) "%s", text ));
    }
    else {
     
        if ( maxLen > 4 ) {

            for ( int i = 0; i < maxLen - 3; i++  ) writeChars( "%c", text[ i ] );
            writeChars((char *) "..." );

            return( maxLen );
        }
        else {

            for ( int i = 0; i < maxLen; i++  ) writeChars((char *) "." );
            return( maxLen );
        }
    }
}

//----------------------------------------------------------------------------------------
// We often need to print a bit of a machine word. If set in upper case, if cleared in 
// lower case.
// 
//----------------------------------------------------------------------------------------
char SimFormatter::printBit( T64Word val, int pos, char printChar ) {

    if ( isInRange( pos, 0, 63 )) {

        if (( val >> pos ) & 0x1 )  return ( toupper( printChar ));
        else                        return ( tolower( printChar ));
    }
    else return ( '*' );
}

//----------------------------------------------------------------------------------------
// "printNumber" will print the number in the selected format. There quite a few HEX
// format to ease the printing of large numbers as we have in 64-bit system. If the 
// "invalid number" option is set in addition to the number format, the format is filled 
// with asterisks instead of numbers.
//
//----------------------------------------------------------------------------------------
int SimFormatter::printNumber( T64Word val, uint32_t fmtDesc ) {

    if ((( fmtDesc >> 8 ) & 0xF ) > 0 ) {

        switch (( fmtDesc >> 8 ) & 0xF ) {

            case 1: { // HEX as is

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X ) 
                        len += writeChars((char *) "0x" ); 

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "**" );          
                else                             
                    len += writeChars((char *) "%x", val );

                return ( len );

            } break;

            case 2: { // HEX_2

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                        len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM )
                    len += writeChars((char *) "**" );          
                else 
                    len += writeChars((char *) "%02x", val & 0xFF );
                
                return( len );

            } break;

            case 3: { // HEX_4

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM )
                    len += writeChars((char *) "****" );          
                else 
                    len += writeChars((char *) "%04x", val & 0xFFFF );

                return( len );

            } break;

            case 4: { // HEX_8

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM )
                    len += writeChars((char *) "****" "****" );         
                else 
                    len += writeChars((char *) "%08x", val & 0xFFFFFFFF );
                
                return( len );

            } break;

            case 5: { // HEX_16

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "****" "****" "****" "****" );
                else 
                    len += writeChars((char *) "%016x", val );
                
                    return( len );

            } break;

            case 6: { // FMT_HEX_2_4

                int len = 0;
                
                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "**_****" );
                else
                    len += writeChars((char *) "%02x_%04x", 
                                      (( val >> 16 ) & 0xFF   ),
                                      (( val       ) & 0xFFFF ));
                
                return( len );

            } break;

            case 7: { // FMT_HEX_4_4

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "****_****" );
                else
                    len += writeChars((char *) "%04x_%04x", 
                                      (( val >> 16 ) & 0xFFFF ),
                                      (( val       ) & 0xFFFF ));
                return( len );

            } break;

            case 8: { // FMT_HEX_2_4_4

                int len = 0;
                
                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "**_****_****" );
                else 
                    len += writeChars((char *) "%02x_%04x_%04x", 
                                      (( val >> 32 ) & 0xFF   ),
                                      (( val >> 16 ) & 0xFFFF ),
                                      (( val       ) & 0xFFFF ));

                return( len );

            } break;

            case 9: { // FMT_HEX_4_4_4

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "****_****_****" );
                else 
                    len += writeChars((char *) "%04x_%04x_%04x", 
                                      (( val >> 32 ) & 0xFFFF ),
                                      (( val >> 16 ) & 0xFFFF ),
                                      (( val       ) & 0xFFFF ));

                return( len );

            } break;

            case 10: { // FMT_HEX_2_4_4_4

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "**_****_****_****" );
                else
                    len += writeChars((char *) "%02X_%04x_%04x_%04x", 
                                      (( val >> 48 ) & 0xFF   ),
                                      (( val >> 32 ) & 0xFFFF ),
                                      (( val >> 16 ) & 0xFFFF ),
                                      (( val       ) & 0xFFFF ));

                return( len );

            } break;

            case 11: { // FMT_HEX_4_4_4_4

                int len = 0;

                if ( fmtDesc & FMT_PREFIX_0X )   
                    len += writeChars((char *) "0x" );

                if ( fmtDesc & FMT_INVALID_NUM ) 
                    len += writeChars((char *) "****_****_****_****" );
                else
                    len += writeChars((char *) "%04x_%04x_%04x_%04x", 
                                      (( val >> 48 ) & 0xFFFF ),
                                      (( val >> 32 ) & 0xFFFF ),
                                      (( val >> 16 ) & 0xFFFF ),
                                      (( val       ) & 0xFFFF ));
                return( len );

            } break;

            default: return ( writeChars ((char *) "*num*" ));
        }
    }
    else  if ((( fmtDesc >> 12 ) & 0xF ) > 0 ) {

         switch (( fmtDesc >> 12 ) & 0xF ) {

            case 1: { // DEC as is

                return ( writeChars((char *) "%d", val));

            } break;

            case 2: { // DEC_32

                return ( writeChars((char *) "%10d", val ));

            } break;

            default: return ( writeChars ((char *) "*num*" ));
         }

    }
    else if ( fmtDesc & FMT_ASCII_4 ) {

        int           len = 0;
        unsigned char bytes[ 4 ];

        for ( int i = 0; i < 4; i++ )
            bytes[ i ] = ( val >> ( 8 * ( 3 - i ))) & 0xFF;

        len += writeChars( "\"");
        for ( int i = 0; i < 4; i++ ) {
            
            unsigned char c = bytes[i];
            
            if ( isprint(c)) len += writeChars( "%c", c );
            else             len += writeChars( "." );
        }
        
        len += writeChars( "\"" );

        return( len );
    }
    else if ( fmtDesc & FMT_ASCII_8 ) {

        int           len = 0;
        unsigned char bytes[ 8 ];

        for ( int i = 0; i < 8; i++ )
            bytes[ i ] = ( val >> (8 * ( 7 - i ))) & 0xFF;

        len += writeChars( "\"" );
        for ( int i = 0; i < 8; i++ ) {
            
            unsigned char c = bytes[ i ];
            
            if (isprint(c)) len += writeChars( "%c", c );
            else            len += writeChars( "." );
        }
        len += writeChars( "\"" );

        return( len );
    }
    else return( writeChars( "*num*" ));
}

//----------------------------------------------------------------------------------------
// The window system sometimes prints numbers in a field with a given length. This
// routine returns based format descriptor and optional value the necessary field length.
//
//----------------------------------------------------------------------------------------
int SimFormatter::numberFmtLen( uint32_t fmtDesc, T64Word val ) {
    
    if ((( fmtDesc >> 8 ) & 0xF ) > 0 ) {

        int prefixLen = (( fmtDesc & FMT_PREFIX_0X ) ? 2 : 0 );

        switch (( fmtDesc >> 8 ) & 0xF ) {

            case 1: { // HEX

                int len = prefixLen + 1;

                val = abs( val );
                
                while ( val & 0xF ) {

                    val >>= 4;
                    len ++;
                }
                
                return( len );

            } break;

            case 2:     return (  2 + prefixLen );   // HEX_2
            case 3:     return (  4 + prefixLen );   // HEX_4
            case 4:     return (  8 + prefixLen );   // HEX_8
            case 5:     return ( 16 + prefixLen );   // HEX_16
            case 6:     return (  7 + prefixLen );   // FMT_HEX_2_4
            case 7:     return (  9 + prefixLen );   // FMT_HEX_4_4
            case 8:     return ( 12 + prefixLen );   // FMT_HEX_2_4_4
            case 9:     return ( 14 + prefixLen );   // FMT_HEX_4_4_4
            case 10:    return ( 17 + prefixLen );   // FMT_HEX_2_4_4_4
            case 11:    return ( 19 + prefixLen );   // FMT_HEX_4_4_4_4
            default:    return( 0 );
        }
    }
    else if ((( fmtDesc >> 12 ) & 0xF ) > 0 ) { 

        switch (( fmtDesc >> 12 ) & 0xF ) {

            case 1: { // DEC

                int len = (( val < 0 ) ? 2 : 1 );

                val = abs( val );
                
                while ( val >= 10 ) {

                    val /= 10;
                    len ++;
                }
                
                return( len );

            } break;

            case 2: return( 10 ); // FMT_DEC_32

            default: return( 0 );
        }
    }
    else if ( fmtDesc & FMT_ASCII_4 ) {

        return( 6 );
    }
    else if (  fmtDesc & FMT_ASCII_8 ) {

        return( 10 );
    }
    else return ( 0 );
}
