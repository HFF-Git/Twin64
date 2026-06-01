//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Window classes
//
//----------------------------------------------------------------------------------------
// This module contains all of the methods for the different simulator windows. The 
// exception is the command window, which is in a separate file. A window generally
// consist of a banner line, shown in inverse video and a number of body lines.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Window classes
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
// ??? How about a module window. Shows the system state and module state.
//
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions and constants local to the 
// file.
//
//----------------------------------------------------------------------------------------
namespace {

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
                    } 
                    else continue;
                } 
                else break;
            } 
            else *dst++ = *src++;
        } 
        else *dst++ = *src++;
    }
    
    *dst = '\0';
}

//----------------------------------------------------------------------------------------
// "calculateStrLen" calculates the length of a string, taking into account that
// tabs are not just one character, but they move the cursor to the next tab stop.
// The tab stops are every "tabWidth" columns. So if we are at column 3 and we 
// encounter a tab, we move to column 8, which means that the tab has a length 
// of 5 in this case.
//
//----------------------------------------------------------------------------------------
size_t calculateStrLen( const char *s, int tabWidth  ) {

    size_t col = 0;

    while ( *s ) {

        if ( *s == '\t' ) col += tabWidth - ( col % tabWidth );
        else              col++;    

        s++;
    }

    return col;
}

//----------------------------------------------------------------------------------------
// "translateAdr" is a little helper function to translate a virtual address to
// a physical address. It first checks if the address is in the physical memory
// range. If so, it is already a physical address and we can return it. If not, 
// we look for the global TLB module and ask it to translate the address. If 
// there is no global TLB module, we cannot translate the address. Also, if the
// global TLB module cannot translate the address, we return false.
//
//----------------------------------------------------------------------------------------
bool translateAdr( T64System *sys, T64Word virtAdr, T64Word *physAdr ) {

    if ( isInPhysMemAdrRange( virtAdr )) {

        *physAdr = virtAdr;
        return ( true );
    }
    else {

        T64GlobalTlb *tlbModule = (T64GlobalTlb *)sys -> lookupByModuleType( MT_GTLB );
        if ( tlbModule == nullptr ) return ( false );

        return ( tlbModule -> translateAdr( virtAdr, physAdr ));
    }
}

//----------------------------------------------------------------------------------------
// "readMem" is a helper function to read memory content. It takes care of the
// address translation and the endian conversion. It first translates the virtual
// address to a physical address. If the translation succeeds, it performs a bus 
// read operation to read the memory content. If the bus read operation succeeds, 
// it converts endian aware the data and returns true.
//
//----------------------------------------------------------------------------------------
bool readMem( T64System *sys, T64Word adr, uint8_t *val, size_t size ) {

    T64Word physAdr = 0;

    if ( ! translateAdr( sys, adr, &physAdr )) return ( false );

    if ( sys -> busOpRead( -1, physAdr, (uint8_t *)val, size)) {

        copyEndianAware((uint8_t *) val, (uint8_t *) val, size);
        return ( true );    
    }

    return ( false );
}

}; // namespace


//****************************************************************************************
//****************************************************************************************
//
// Methods for the Program State Window class. This is the main window for a CPU.
//
//----------------------------------------------------------------------------------------
// Object creator.
//
//----------------------------------------------------------------------------------------
SimWinProcState::SimWinProcState( SimGlobals *glb, int modNum ) : SimWin( glb ) { 

    this -> glb             = glb;
    this -> disAsm          = new T64DisAssemble( );
    this -> codeWinBaseAdr  = 0;

    T64ModuleType mType = glb -> system -> getModuleType( modNum );
    if ( mType != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    this -> proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );

    setWinModNum( modNum );
    setDefaults( );
}

//----------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first 
// time, or for the WDEF command. The window limits are set for the different 
// toggle values. The minimum row count specifies how many lines we need for the
// banner line, the respective register lines and the minimum number of code
// lines plus the subwindow banner.
//
//----------------------------------------------------------------------------------------
void SimWinProcState::setDefaults( ) {

    const int MIN_ROW_BANNERS               = 2;
    const int MIN_ROW_GREG_SUBWINDOW        = 4; 
    const int MIN_ROW_CREG_SUBWINDOW        = 4; 
    const int MIN_ROW_CREG_SUBSET_SUBWINDOW = 1;
    const int MIN_ROW_CODE_SUBWINDOW        = 4; 
    const int MAX_ROWS                      = 32;
    const int MAX_COLS                      = 96;
    
    setWinType( WT_CPU_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 3 );
    setWinToggleVal( 0 );

    setWinLimitsForToggle( 0, 
                           MIN_ROW_BANNERS + 
                           MIN_ROW_GREG_SUBWINDOW + 
                           MIN_ROW_CODE_SUBWINDOW,
                           MAX_ROWS,
                           MAX_COLS, 
                           MAX_COLS );

    setWinLimitsForToggle( 1, 
                           MIN_ROW_BANNERS + 
                           MIN_ROW_GREG_SUBWINDOW + 
                           MIN_ROW_CREG_SUBSET_SUBWINDOW +
                           MIN_ROW_CODE_SUBWINDOW,
                           MAX_ROWS,
                           MAX_COLS, 
                           MAX_COLS );

    setWinLimitsForToggle( 2, 
                           MIN_ROW_BANNERS + 
                           MIN_ROW_CREG_SUBWINDOW + 
                           MIN_ROW_CODE_SUBWINDOW,
                           MAX_ROWS,
                           MAX_COLS, 
                           MAX_COLS );

    setRows( getWinSize( 0 ).actualRow );
    setColumns( getWinSize( 0 ).actualCol );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// Each window consist of a headline and a body. The banner line is always shown 
// in inverse and contains summary or head data for the window. The program state
// banner lists the instruction address and the status word.
//
// Format:
//
//  <winId> Proc: n IA: 0x00_0000_0000 ST: [xxxxxxx] <rdx>
//
//----------------------------------------------------------------------------------------
void SimWinProcState::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;

    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );

    printTextField((char *) "Mod:", fmtDesc );
    printNumericField( getWinModNum( ), fmtDesc | FMT_DEC );

    T64Word psw = proc -> getCpuPtr( ) -> getPsrReg( );

    printTextField((char *) " IA: ", fmtDesc );
    printNumericField( psw, fmtDesc | FMT_HEX_2_4_4_4 );

    printTextField((char *) " ST: [", fmtDesc );
    printBitField( psw, 63, 'A', fmtDesc );
    printBitField( psw, 62, 'B', fmtDesc );
    printBitField( psw, 61, 'C', fmtDesc );
    printBitField( psw, 60, 'D', fmtDesc );
    printBitField( psw, 59, 'E', fmtDesc );
    printBitField( psw, 58, 'F', fmtDesc );
    printTextField((char *) "]", fmtDesc );

    printTextField((char *) " State: ", fmtDesc );
    printTextField( proc -> getProcStateStr( ), fmtDesc );

    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );

    setRows( getWinSize( getWinToggleVal( )).actualRow );
}

//----------------------------------------------------------------------------------------
// "drawGeneralRegSubWindow" draws the general registers set in the body of the 
// window. We show 4 registers per line, with the format "GRn=0x0000_0000_0000_0000". 
//
//----------------------------------------------------------------------------------------
void SimWinProcState::drawGeneralRegSubWindow( int linePos ) {

    uint32_t fmtDesc        = FMT_DEF_ATTR | FMT_ALIGN_LFT;
    T64Cpu   *cpu           = proc -> getCpuPtr( );
    int      numFlen        = glb -> console -> numberFmtLen( FMT_HEX_4_4_4_4 ) + 3;
    int      labelFlen      = 8;
    uint32_t numFmtField    = fmtDesc | FMT_HEX_4_4_4_4;
    uint32_t labelFmtField  = fmtDesc | FMT_BOLD;
    
    setWinCursor( linePos, 1 );
    printTextField((char *) "GR0=", labelFmtField, labelFlen );

    for ( int i = 0; i < 4; i++ ) {

        printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
    setWinCursor( linePos + 1, 1 );
    printTextField((char *) "GR4=", labelFmtField, labelFlen );

    for ( int i = 4; i < 8; i++ ) {

        printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
    setWinCursor( linePos + 2, 1 );
    printTextField((char *) "GR8=", labelFmtField, labelFlen );

    for ( int i = 8; i < 12; i++ ) {

        printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
    setWinCursor( linePos + 3, 1 );
    printTextField((char *) "GR12=", labelFmtField, labelFlen );

    for ( int i = 12; i < 16; i++ ) {

        printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
} 

//----------------------------------------------------------------------------------------
// "drawControlRegSubWindow" draws the control registers set in the body of the 
// window. We show 4 registers per line, with the format "CRn=0x0000_0000_0000_0000". 
//
//----------------------------------------------------------------------------------------
void SimWinProcState::drawControlRegSubWindow( int linePos ) {

    uint32_t fmtDesc        = FMT_DEF_ATTR | FMT_ALIGN_LFT;
    T64Cpu   *cpu           = proc -> getCpuPtr( );
    int      numFlen        = glb -> console -> numberFmtLen( FMT_HEX_4_4_4_4 ) + 3;
    int      labelFlen      = 8;
    uint32_t numFmtField    = fmtDesc | FMT_HEX_4_4_4_4;
    uint32_t labelFmtField  = fmtDesc | FMT_BOLD;

    setWinCursor( linePos, 1 );
    printTextField((char *) "CR0=", labelFmtField, labelFlen );

    for ( int i = 0; i < 4; i++ ) {

        printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
    setWinCursor( linePos + 1, 1 );
    printTextField((char *) "CR4=", labelFmtField, labelFlen );

    for ( int i = 4; i < 8; i++ ) {

        printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
    setWinCursor( linePos + 2, 1 );
    printTextField((char *) "CR8=", labelFmtField, labelFlen );

    for ( int i = 8; i < 12; i++ ) {

        printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
    setWinCursor( linePos + 3, 1 );
    printTextField((char *) "CR12=", labelFmtField, labelFlen );

    for ( int i = 12; i < 16; i++ ) {

        printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
}

//----------------------------------------------------------------------------------------
// "drawCregSubsetRegSubWindow" draws a subset of the control registers. The 
// registers are related to user visible registers and registers that can be 
// modified in user mode. 
//
//----------------------------------------------------------------------------------------
void SimWinProcState::drawCregSubsetRegSubWindow( int linePos ) {

    uint32_t fmtDesc        = FMT_DEF_ATTR | FMT_ALIGN_LFT;
    T64Cpu   *cpu           = proc -> getCpuPtr( );
    int      numFlen        = glb -> console -> numberFmtLen( FMT_HEX_4_4_4_4 ) + 3;
    int      labelFlen      = 8;
    uint32_t numFmtField    = fmtDesc | FMT_HEX_4_4_4_4;
    uint32_t labelFmtField  = fmtDesc | FMT_BOLD;

    padLine( fmtDesc );
    setWinCursor( linePos, 1 );
    printTextField((char *) "PID=", labelFmtField, labelFlen );

    for ( int i = 4; i < 8; i++ ) {

        printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
    }

    padLine( fmtDesc );
}
    
//----------------------------------------------------------------------------------------
// "drawCodeSubWindow" draws the code area in the body of the window. We show a
// banner and a small number of code lines, with the instruction address and the
// instruction in hex. We also show the disassembled instruction text. Also, the
// code line where the instruction address points to is marked.
//
// Whenever the current instruction address comes near or is is outside the 
// current code window, we move the code window so that the current instruction 
//address is in the visible range.
//
//----------------------------------------------------------------------------------------
void SimWinProcState::drawCodeSubWindow( int linePos, int linesLeft ) {

    uint32_t    fmtDesc     = FMT_DEF_ATTR | FMT_BOLD | FMT_INVERSE;
    T64Word     currentIa   = proc -> getCpuPtr( ) -> getPsrReg( );
    T64Word     windowSize  = linesLeft * 4;
    T64Word     windowEnd   = codeWinBaseAdr + windowSize - 4;
    uint32_t    instr       = 0x0;
    char        instrBuf[ MAX_TEXT_LINE_SIZE ] = { 0 };

    if ( currentIa < codeWinBaseAdr + 4 ) {

        codeWinBaseAdr = ( currentIa >= 4 ) ? currentIa - 4 : 0;

    } else if ( currentIa >= windowEnd - 4 ) {

        codeWinBaseAdr = currentIa - 4;
    }

    setWinCursor( linePos, 1 );
    printTextField((char *) "Code", fmtDesc );
    printTextField((char *) " ", fmtDesc );
    padLine( fmtDesc );

    fmtDesc = FMT_DEF_ATTR;
    for ( int i = 0; i < linesLeft; i++ ) {

        T64Word ia = codeWinBaseAdr + ( i * 4 );

        setWinCursor( linePos + i + 1, 1 );
        printNumericField( ia, fmtDesc | FMT_HEX_2_4_4_4 );
        printTextField((char *) ": ", fmtDesc );

        if ( readMem( glb -> system, ia, (uint8_t *) &instr, sizeof( instr ))) {

            if ( currentIa ==  ia ) printTextField((char *) "    >", fmtDesc, 5 );
            else                    printTextField((char *) "     ", fmtDesc, 5 );

            printNumericField( instr, fmtDesc | FMT_HEX_8 );
            printTextField((char *) "    ", fmtDesc );

            int pos          = getWinCursorCol( );
            int opCodeField  = disAsm -> getOpCodeFieldWidth( );
            int operandField = disAsm -> getOperandsFieldWidth( );
            
            clearField( opCodeField );
            disAsm -> formatOpCode( instrBuf, sizeof( instrBuf ), instr );
            printTextField( instrBuf, fmtDesc, (int) strlen( instrBuf ));
            setWinCursor( 0, pos + opCodeField );
            
            clearField( operandField );
            disAsm -> formatOperands( instrBuf, sizeof( instrBuf ), instr, 16 );
            printTextField( instrBuf, fmtDesc, (int) strlen( instrBuf ));
            setWinCursor( 0, pos + opCodeField + operandField );

            padLine( fmtDesc );
        }
        else {

            printTextField((char *) "????_????", fmtDesc );
            padLine( fmtDesc );
        }
    }
}

//----------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after 
// the banner line. The window supports the toggle concept and shows in the upper
// part the selected register view. The lower part will display the code area
// where the instruction address register points to. This window will eat up 
// the remaining line sin the processor window.
// 
//----------------------------------------------------------------------------------------
void SimWinProcState::drawBody( ) {
    
    int     toggleVal = getWinToggleVal( );
    int     linePos   = 2;

    if (( toggleVal == 0 ) || ( toggleVal == 1 )) {

        drawGeneralRegSubWindow( linePos );
        linePos += 4;
    }    
       
    if ( toggleVal == 1 ) {

        drawCregSubsetRegSubWindow( linePos );
        linePos += 1;
    }
    
    if ( toggleVal == 2 ) {

        drawControlRegSubWindow( linePos );
        linePos += 4;
    }

    int linesLeft = getRows( ) - linePos + 1;
    drawCodeSubWindow( linePos, linesLeft );
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the TLB class.
//
//----------------------------------------------------------------------------------------
// Object constructor. All we do is to remember the reference to the TLB object.
//
//----------------------------------------------------------------------------------------
SimWinTlb::SimWinTlb( SimGlobals    *glb, 
                      int           modNum ) : SimWinScrollable( glb ) { 

    this -> glb = glb;

    T64ModuleType mType = glb -> system -> getModuleType( modNum );
    if ( mType != MT_GTLB ) throw ( ERR_INVALID_MODULE_TYPE );

    this -> tlb = (T64GlobalTlb *) glb -> system -> lookupByModNum( modNum );
    if ( tlb == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );

    setWinModNum( modNum );
    setDefaults( );
}

//----------------------------------------------------------------------------------------
// We have a function to set reasonable default values for the window. These  
// values are the initial settings when windows is brought up the first time, 
// or for the WDEF command. The TLB window is a window where the number of 
// lines to display can be set. However, the minimum is the default number of
// lines.
//
//----------------------------------------------------------------------------------------
void SimWinTlb::setDefaults( ) {
    
    setWinType( WT_TLB_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 1 );
    setWinLimitsForToggle( 0, 8, tlb -> getTlbSize( ), 96, 96 );
    
    setRows( getWinSize( 0 ).actualRow );
    setColumns( getWinSize( 0 ).actualCol );

    setCurrentItemAdr( 0 );
    setLineIncrementItemAdr( 1 );
    setLimitItemAdr( tlb -> getTlbSize( ));
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The banner line is always shown in 
// inverse and contains summary or head data for the window. In addition we show
// the TLB type, which is derived from the module type. The toggle value will 
// decide on which TLB type we are showing the entries, the UTLB, the ITLB or 
// the DTLB.
// 
//----------------------------------------------------------------------------------------
void SimWinTlb::drawBanner( ) {
    
    uint32_t fmtDesc   = FMT_BOLD | FMT_INVERSE;
   
    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );
    printTextField((char *) "Mod:" );
    printNumericField( getWinModNum( ), ( fmtDesc | FMT_DEC ));
    printTextField((char *) " ( ", fmtDesc );
    printTextField((char *) tlb -> getTlbTypeStr( ), fmtDesc );
    printTextField((char *) " ):", fmtDesc );

    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//----------------------------------------------------------------------------------------
// The body of the TLB window shows the TLB entries. We are passed a pointer
// to the TLB entry, which we need to decode and print.
//
//----------------------------------------------------------------------------------------
void SimWinTlb::drawTlbEntry( T64TlbEntry *ePtr ) {

    uint32_t  fmtDesc = FMT_DEF_ATTR;

    printTextField((char *) "vAdr: ", fmtDesc );
    printNumericField( ePtr -> vAdr, fmtDesc | FMT_HEX_2_4_4_4 );

    printTextField((char *) "  pAdr: ", fmtDesc );
    printNumericField( ePtr -> pAdr, fmtDesc | FMT_HEX_2_4_4 );

    printTextField((char *) "  info: ", fmtDesc );
    printNumericField( ePtr -> tlbInfo, fmtDesc | FMT_HEX_4 );

    printTextField((char *) "  pMask: ", fmtDesc );
    printNumericField( ePtr -> pageMask, fmtDesc | FMT_HEX_4_4_4_4 );
}

//----------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after
// the banner line. The number of lines can vary. A line represents an entry in 
// the respective TLB. We have three TLBs, the UTLB, the ITLB and the DTLB. The
// toggle value will decide on which TLB entries to display.
//
//----------------------------------------------------------------------------------------
void SimWinTlb::drawLine( T64Word index ) {

    uint32_t    fmtDesc     = FMT_DEF_ATTR;
    T64TlbEntry *ePtr       = tlb -> getTlbEntry( index );

    printTextField((char *) "(", fmtDesc );
    printNumericField( index, fmtDesc | FMT_HEX_4 );
    printTextField((char *) "): ", fmtDesc );

    if ( ePtr != nullptr ) drawTlbEntry( ePtr );
    padLine( fmtDesc );
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the physical memory window class.
//
//----------------------------------------------------------------------------------------
// Object constructor.
//
//----------------------------------------------------------------------------------------
SimWinMem::SimWinMem( SimGlobals *glb, int modNum, T64Word adr ) : 
                                                         SimWinScrollable( glb ) {

    this -> adr = rounddown( adr, 8 );
    setWinModNum( modNum );
    setDefaults( );
 }

//----------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the 
// first time, or for the WDEF command. The memory window is a window where the 
// number of lines to display can be set. However, the minimum is the default 
// number of lines.
//
//----------------------------------------------------------------------------------------
void SimWinMem::setDefaults( ) {
    
    setWinType( WT_MEM_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 4 );
    setWinLimitsForToggle( 0, 5, MAX_WIN_ROW_SIZE, 112, 112 );
    setWinLimitsForToggle( 1, 5, MAX_WIN_ROW_SIZE, 112, 112 );
    setWinLimitsForToggle( 2, 5, MAX_WIN_ROW_SIZE, 112, 112 );
    setWinLimitsForToggle( 3, 5, MAX_WIN_ROW_SIZE, 112, 112 );
    setRows( getWinSize( 0 ).actualRow );
    setColumns( getWinSize( 0 ).actualCol );
    setHomeItemAdr( adr );
    setCurrentItemAdr( adr );
    setLineIncrementItemAdr( 8 * 4 );
    setLimitItemAdr( T64_MAX_PHYS_MEM_LIMIT );
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// In addition to the module and window info, the banner line shows the item home
// address.
//
//----------------------------------------------------------------------------------------
void SimWinMem::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;

    if ( getWinToggleVal( ) == 2 )  setRadix( 10 ); 
    else                            setRadix( 16 ); 

    T64Memory *mem = (T64Memory *) 
            glb -> system -> lookupByAdr( getCurrentItemAdr( ));

    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );
    printTextField((char *) "Mod:", fmtDesc );
    printNumericField( getWinModNum( ), fmtDesc | FMT_DEC );

    if ( mem != nullptr ) {

       printTextField((char *) " ( ", fmtDesc );
       printTextField( mem -> getMemTypeString( ), fmtDesc );
       printTextField((char *) " ) ", fmtDesc );
    }

    printTextField((char *) "  Home: " );
    printNumericField( getHomeItemAdr( ), fmtDesc | FMT_HEX_2_4_4 );
    padLine( fmtDesc );

    // ??? if toggleVal is ascii should say ascii...

    printRadixField( fmtDesc | FMT_LAST_FIELD );

    if ( ! isAlignedAdr( getCurrentItemAdr( ), 8 )) {
        
        setCurrentItemAdr( rounddown( getCurrentItemAdr( ), 8 ));
    }   
}

//----------------------------------------------------------------------------------------
// A scrollable window needs to implement a routine for displaying a row. We are 
// passed the item address and need to map this to the actual meaning of the 
// particular window. The "itemAdr" value is the byte offset into physical memory,
// the line increment is 8 * 4 = 32 bytes. We show eight 32-bit words or 4 64-bit 
// words. The toggle value will decide on the format:
//
// Toggle 0: (0x00_0000_0000) 0x0000_0000             ... 8 times HEX.
// Toggle 1: (0x00_0000_0000) 0x0000_0000_0000_0000   ... 4 times HEX.
// Toggle 2: (0x00_0000_0000)           0             ... 8 times DEC.
// Toggle 3: (0x00_0000_0000) "...." "...."           ... 8 times ASCII in "" 
//
//----------------------------------------------------------------------------------------
void SimWinMem::drawLine( T64Word itemAdr ) {

    uint32_t    fmtDesc     = FMT_DEF_ATTR;
    uint32_t    limit       = getLineIncrementItemAdr( ) - 1; // ??? why - 1?

    T64Memory   *mem = (T64Memory *) 
                            glb -> system -> lookupByAdr( getCurrentItemAdr( ));
    if ( mem == nullptr ) {

        printTextField((char *) "Invalid address" );
    }; 

    printTextField((char *) "(", fmtDesc );
    printNumericField( itemAdr, fmtDesc | FMT_HEX_2_4_4 );
    printTextField((char *) "): ", fmtDesc );

    if ( getWinToggleVal( ) == 0 ) {

        for ( int i = 0; i < limit; i = i + 4 ) {

            uint32_t val = 0;
            if ( readMem( glb -> system, itemAdr + i, (uint8_t *)&val, sizeof( val ))) {

                printNumericField( val, fmtDesc | FMT_HEX_4_4 );
                printTextField((char *) "   " );
            }
        }
    }
    else if ( getWinToggleVal( ) == 1 ) {

        for ( int i = 0; i < limit; i = i + 8 ) {

            T64Word val = 0;
            if ( readMem( glb -> system, itemAdr + i, (uint8_t *)&val, sizeof( val ))) {

                printNumericField( val, fmtDesc | FMT_HEX_4_4_4_4 );
                printTextField((char *) "   " );
            }
        }
    }
    else if ( getWinToggleVal( ) == 2 ) {

        for ( int i = 0; i < limit; i = i + 4 ) {

            uint32_t val = 0;
            if ( readMem( glb -> system, itemAdr + i, (uint8_t *)&val, sizeof( val ))) {

                printNumericField( val, fmtDesc | FMT_DEC_32 );
                printTextField((char *) "   " );
            }
        }
    }
    else if ( getWinToggleVal( ) == 3 ) {

        for ( int i = 0; i < limit; i = i + 4 ) {

            uint32_t val = 0;
            if ( readMem( glb -> system, itemAdr + i, (uint8_t *)&val, sizeof( val ))) {

                printNumericField( val, fmtDesc | FMT_ASCII_4 );
                printTextField((char *) "   " );
            }
        }
    }
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the code memory window class.
//
//----------------------------------------------------------------------------------------
// Object constructor. We create a disassembler object for displaying the decoded 
// instructions and also need to remove it when the window is killed. 
//
//----------------------------------------------------------------------------------------
SimWinCode::SimWinCode( SimGlobals *glb, int modNum, T64Word adr ) : 
                                                    SimWinScrollable( glb ) {

    disAsm = new T64DisAssemble( );

    setWinModNum( modNum );
    this -> adr = rounddown( adr, 8 );
    setDefaults( );
}

SimWinCode::  ~  SimWinCode( ) {

    if ( disAsm != nullptr ) delete ( disAsm );
}

//----------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the 
// first time, or for the WDEF command. The code memory window is a window where
// the number of lines to display can be set. However, the minimum is the default 
// number of lines.
//
//----------------------------------------------------------------------------------------
void SimWinCode::setDefaults( ) {
     
    setWinType( WT_CODE_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 1 );
    setWinLimitsForToggle( 0, 8, MAX_WIN_ROW_SIZE, 80, 80 );
    setRows( getWinSize( 0 ).actualRow );
    setColumns( getWinSize( 0 ).actualCol );
    setHomeItemAdr( adr );
    setCurrentItemAdr( adr );
    setLineIncrementItemAdr( 4 );
    setLimitItemAdr( T64_MAX_PHYS_MEM_LIMIT );
    setWinToggleLimit( 0 );
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// The banner for the code window shows the code address. We automatically scroll 
// the window for the single step command. We detect this by examining the current
// command and adjust the current item address to scroll to the next lines to show.
//
//----------------------------------------------------------------------------------------
void SimWinCode::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;

    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );
    printTextField((char *) "Mod:", fmtDesc );
    printNumericField( getWinModNum( ), fmtDesc | FMT_DEC );
    printTextField((char *) " ", fmtDesc );
    printTextField((char *) "Current: " );
    printNumericField( getCurrentItemAdr( ), fmtDesc | FMT_HEX_2_4_4 );
    printTextField((char *) "  Home: " );
    printNumericField( getHomeItemAdr( ), fmtDesc | FMT_HEX_2_4_4 );
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//----------------------------------------------------------------------------------------
// A scrollable window needs to implement a routine for displaying a row. We are 
// passed the item address and will print the instruction address and value as
// well as the disassembled format.
// 
//----------------------------------------------------------------------------------------
void SimWinCode::drawLine( T64Word itemAdr ) {
    
    uint32_t    fmtDesc                     = FMT_DEF_ATTR;
    uint32_t    instr                       = 0x0;
    char        buf[ MAX_TEXT_LINE_SIZE ]   = { 0 };

    printNumericField( itemAdr, fmtDesc | FMT_ALIGN_LFT | FMT_HEX_2_4_4, 18 );
   
    if ( ! readMem( glb -> system, 
                    itemAdr, (uint8_t *) &instr, 
                    sizeof( uint32_t ))) {

        printTextField((char *) "Invalid address", fmtDesc );
        return;
    }

    printNumericField( instr, fmtDesc | FMT_ALIGN_LFT | FMT_HEX_8, 12 );
    
    int pos          = getWinCursorCol( );
    int opCodeField  = disAsm -> getOpCodeFieldWidth( );
    int operandField = disAsm -> getOperandsFieldWidth( );
    
    clearField( opCodeField );
    disAsm -> formatOpCode( buf, sizeof( buf ), instr );
    printTextField( buf, fmtDesc, (int) strlen( buf ));
    setWinCursor( 0, pos + opCodeField );
    
    clearField( operandField );
    disAsm -> formatOperands( buf, sizeof( buf ), instr, 16 );
    printTextField( buf, fmtDesc, (int) strlen( buf ));
    setWinCursor( 0, pos + opCodeField + operandField );

    padLine( fmtDesc );
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the text window class.
//
//----------------------------------------------------------------------------------------
// Object constructor. We are passed the globals and the file path. All we do 
// right now is to remember the file name. The text window has a destructor 
// method as well. We need to close a potentially opened file.
//
//----------------------------------------------------------------------------------------
SimWinText::SimWinText( SimGlobals *glb, char *fName ) : SimWinScrollable( glb ) {
    
    if ( fName != nullptr ) strcpy( fileName, fName );
    else throw ( ERR_EXPECTED_FILE_NAME );

    setWinModNum ( -1 );
    setDefaults( );
}

SimWinText:: ~SimWinText( ) {
    
    if ( textFile != nullptr ) fclose( textFile );
}

//----------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first 
// time, or for the WDEF command.
//
//----------------------------------------------------------------------------------------
void SimWinText::setDefaults( ) {

    int txWidth = glb -> env -> getEnvVarInt((char *) ENV_WIN_TEXT_LINE_WIDTH );
    
    setWinType( WT_TEXT_WIN );
    
    setWinToggleLimit( 1 );
    setWinLimitsForToggle( 0, 10, MAX_WIN_ROW_SIZE, txWidth, txWidth );
    setRows( getWinSize( 0 ).actualRow );
    setColumns( getWinSize( 0 ).actualCol );
    setRadix( 10 );
    setCurrentItemAdr( 0 );
    setLineIncrementItemAdr( 1 );
    setLimitItemAdr( 1 );
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// The banner line for the text window. It contains the open file name and the 
// current line and home line number. The file path may be a bit long for listing
// it completely, so we will truncate it on the left side. The routine will print
// the filename, and the position into the file. The banner method also sets the 
// preliminary line size of the window. This value is used until we know the actual
// number of lines in the file. Lines shown on the display start with one, 
// internally we start at zero.
//
//----------------------------------------------------------------------------------------
void SimWinText::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );
    printTextField((char *) "Text: ", ( fmtDesc | FMT_ALIGN_LFT ));
    printTextField((char *) fileName, ( fmtDesc | FMT_ALIGN_LFT | FMT_TRUNC_LFT ), 48 );
    printTextField((char *) "  Line: " );
    printNumericField( getCurrentItemAdr( ) + 1, ( fmtDesc | FMT_DEC ));
    padLine( fmtDesc );
}

//----------------------------------------------------------------------------------------
// The draw line method for the text file window. We print the file content line 
// by line. A line consists of the line number followed by the text. This routine
// will first check whether the file is already open. If we cannot open the file, 
// we would now print an error message into the screen. This is also the time 
// where we actually figure out how many lines are on the file so that we can set
// the limitItemAdr field of the window object.
//
// A final twist is the problems with tabs in a text file. We have no idea what
// the actual size of a line is until we read it. We need to read the line, 
// expand the tabs and calculate the actual line size. If the line size is larger 
// than the window width, we need to truncate the line.  There are two global
// ENV variables that control the tab size and the text line width. We need to
// take these into account when calculating the line size and truncating the line.
//
//----------------------------------------------------------------------------------------
void SimWinText::drawLine( T64Word index ) {
    
    uint32_t    fmtDesc = FMT_DEF_ATTR;
    int         tabSize = glb -> env -> getEnvVarInt((char *) ENV_WIN_TEXT_TAB_SIZE );
    char        lineBuf[ MAX_TEXT_LINE_SIZE ];
    int         lineSize = 0;

    if ( openTextFile( )) {

        printNumericField( index + 1, ( fmtDesc | FMT_DEC ));
        printTextField((char *) ": " );
        setWinCursor( 0, 8 );
  
        lineSize = readTextFileLine( index + 1, lineBuf, sizeof( lineBuf ));
        if ( lineSize > 0 ) {

            lineSize = calculateStrLen( lineBuf, tabSize ); 
           
            if ( lineSize > getWinSize( 0 ).actualCol )
                lineSize = getWinSize( 0 ).actualCol;
            
            printTextField( lineBuf, fmtDesc, lineSize );
            padLine( fmtDesc );
        }
        else padLine( fmtDesc );
    }
    else printTextField((char *) "Error opening the text file", fmtDesc );
}

//----------------------------------------------------------------------------------------
// "openTextFile" is called every time we want to print a line. If the file is 
// not opened yet, it will be opened now and while we are at it, we will also 
// count the source lines for setting the limit in the scrollable window.
//
//----------------------------------------------------------------------------------------
bool SimWinText::openTextFile( ) {
    
    if ( textFile == nullptr ) {
        
        textFile = fopen( fileName, "r" );
        if ( textFile != nullptr ) {
           
            while( ! feof( textFile )) {

                int c = fgetc( textFile );
                
                if(( c == '\n' ) || ( c == '\r' )) fileSizeLines++;
            }
            
            lastLinePos = 0;
            rewind( textFile );
            setLimitItemAdr( fileSizeLines );
        }
    }
    
    return( textFile != nullptr );
}



//----------------------------------------------------------------------------------------
// "readTextFileLine" will get a line from the text file. Unfortunately, we do 
// not have a line concept in a text file. In the worst case, we read from the 
// beginning of the file, counting the lines read. To speed up a little, we 
// remember the last line position read. If the requested line position is larger
// than the last position, we just read ahead. If it is smaller, no luck, we 
// start to read from zero until we match. If equal, we just re-read the current
// line.
//
//----------------------------------------------------------------------------------------
int SimWinText::readTextFileLine( int linePos, char *lineBuf, int bufLen  ) {
 
    if ( textFile != nullptr ) {
        
        if ( linePos > lastLinePos ) {
            
            while ( lastLinePos < linePos ) {
                
                lastLinePos ++;
                fgets( lineBuf, bufLen, textFile );
            }
        }
        else if ( linePos < lastLinePos ) {
            
            lastLinePos = 0;
            rewind( textFile );
            
            while ( lastLinePos < linePos ) {
                
                lastLinePos ++;
                fgets( lineBuf, bufLen, textFile );
            }
        }
        else fgets( lineBuf, bufLen, textFile );
            
        lineBuf[ strcspn( lineBuf, "\r\n") ] = 0;
        return((int) strlen ( lineBuf ));
    }
    else return ( 0 );
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the console window class.
//
//----------------------------------------------------------------------------------------
// Object constructor.
//
//----------------------------------------------------------------------------------------
SimWinConsole::SimWinConsole( SimGlobals *glb ) : SimWin( glb ) {
    
    this -> glb = glb;
    winOut      = new SimWinOutBuffer( );

    setDefaults( );
}

//----------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first
// time, or for the WDEF command.
//
//----------------------------------------------------------------------------------------
void SimWinConsole::setDefaults( ) {
    
    setWinType( WT_CONSOLE_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 1 );
    setWinLimitsForToggle( 0, 24, MAX_WIN_ROW_SIZE, 112, 112 );
    setRows( getWinSize( 0 ).actualRow );
    setColumns( getWinSize( 0 ).actualCol );
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// The console offers two basic routines to read a character and to write a 
// character. The write function will just add the character to the output buffer.
//
//----------------------------------------------------------------------------------------
void SimWinConsole::putChar( char ch ) {
    
    winOut -> writeChar( ch );
}


// ??? what about the read part. Do we just get a character from the terminal input 
// and add it to the output side ? Or is this a function of the console driver code
// written for the simulator ? should we add the switch to and from the console in 
// this class ?

//----------------------------------------------------------------------------------------
// The banner line for console window.
//
//----------------------------------------------------------------------------------------
void SimWinConsole::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Console ", fmtDesc );
    padLine( fmtDesc );
}

//----------------------------------------------------------------------------------------
// The body lines of the console window are displayed after the banner line. Each
// line is "sanitized" before we print it out. This way, dangerous escape sequences
// are simply filtered out.
//
//----------------------------------------------------------------------------------------
void SimWinConsole::drawBody( ) {
    
    char lineOutBuf[ MAX_WIN_OUT_LINE_SIZE ];
    
    glb -> console -> setFmtAttributes( FMT_DEF_ATTR );
    
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
