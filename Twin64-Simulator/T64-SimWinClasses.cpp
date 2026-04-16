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
// Local name space. We try to keep utility functions and constants local to the 
// file.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// Default row and column values for the window types.
// 
//----------------------------------------------------------------------------------------
const int DEF_WIN_COL_ABS_MEM   = 112;
const int DEF_WIN_ROW_ABS_MEM   = 4;

const int DEF_WIN_COL_CODE_MEM  = 80;
const int DEF_WIN_ROW_CODE_MEM  = 8;

const int DEF_WIN_COL_CPU_STATE = 96;
const int DEF_WIN_ROW_CPU_STATE = 5;

const int DEF_WIN_COL_TLB       = 88;
const int DEF_WIN_ROW_TLB       = 4;

const int DEF_WIN_COL_CACHE     = 112;
const int DEF_WIN_ROW_CACHE     = 4;

const int DEF_WIN_ROW_TEXT      = 10;

const int DEF_WIN_COL_CONSOLE   = 112;
const int DEF_WIN_ROW_CONSOLE   = 24;

//----------------------------------------------------------------------------------------
// Routine for creating the page type string.
//
//----------------------------------------------------------------------------------------
const char *pageTypeStr( uint8_t pTyp ) {

    switch ( pTyp ) {

        case 0: return( "R" );
        case 1: return( "W" );
        case 2: return( "X" );
        case 3: return( "G" );
        default: return( "*" );
    }
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
SimWinCpuState::SimWinCpuState( SimGlobals *glb, int modNum ) : SimWin( glb ) { 

    this -> modNum = modNum;
    this -> glb    = glb;

    T64ModuleType mType = glb -> system -> getModuleType( modNum );
    if ( mType != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    this -> proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );

    setDefaults( );
}

//----------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first 
// time, or for the WDEF command.
//
//----------------------------------------------------------------------------------------
void SimWinCpuState::setDefaults( ) {
    
    setWinType( WT_CPU_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 3 );
    setWinDefSize( 0, DEF_WIN_ROW_CPU_STATE ,DEF_WIN_COL_CPU_STATE );
    setWinDefSize( 1, DEF_WIN_ROW_CPU_STATE + 1, DEF_WIN_COL_CPU_STATE );
    setWinDefSize( 2, DEF_WIN_ROW_CPU_STATE, DEF_WIN_COL_CPU_STATE );
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
    setWinToggleLimit( 3 );
    setWinToggleVal( 0 );
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
void SimWinCpuState::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;

    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );

    printTextField((char *) "Mod:", fmtDesc );
    printNumericField( modNum, fmtDesc | FMT_DEC );

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

    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );

    setRows( getWinDefSize( getWinToggleVal( )).row );
}

//----------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after 
// the banner line. The program state window body lists the general registers.
//
// Format Ideas:
//
//  R0:  0x0000_0000_0000_0000 0x... 0x... 0x...
//  R4:  0x0000_0000_0000_0000 0x... 0x... 0x...
//  R8:  0x0000_0000_0000_0000 0x... 0x... 0x...
//  R12: 0x0000_0000_0000_0000 0x... 0x... 0x...
//
//  PID: 0x0000_0000_0000_0000 0x... 0x... 0x...
//
// The window supports the toggle concept. We should as default only the GRs.
// Toggle to CRs. Perhaps toggle to more formatted screens...
// 
//----------------------------------------------------------------------------------------
void SimWinCpuState::drawBody( ) {
    
    uint32_t fmtDesc   = FMT_DEF_ATTR | FMT_ALIGN_LFT;
    T64Cpu   *cpu      = proc -> getCpuPtr( );
    int      toggleVal = getWinToggleVal( );

    if (( toggleVal == 0 ) || ( toggleVal == 1 )) {

        int      numFlen        = glb -> console -> numberFmtLen( FMT_HEX_4_4_4_4 ) + 3;
        int      labelFlen      = 8;
        uint32_t numFmtField    = fmtDesc | FMT_HEX_4_4_4_4;
        uint32_t labelFmtField  = fmtDesc | FMT_BOLD;
        
        setWinCursor( 2, 1 );
        printTextField((char *) "GR0=", labelFmtField, labelFlen );
    
        for ( int i = 0; i < 4; i++ ) {

            printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
        setWinCursor( 3, 1 );
        printTextField((char *) "GR4=", labelFmtField, labelFlen );
    
        for ( int i = 4; i < 8; i++ ) {

            printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
        setWinCursor( 4, 1 );
        printTextField((char *) "GR8=", labelFmtField, labelFlen );
    
        for ( int i = 8; i < 12; i++ ) {

            printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
        setWinCursor( 5, 1 );
        printTextField((char *) "GR12=", labelFmtField, labelFlen );
    
        for ( int i = 12; i < 16; i++ ) {

            printNumericField( cpu -> getGeneralReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
    } 

    if ( toggleVal == 1 ) {

        int      numFlen        = glb -> console -> numberFmtLen( FMT_HEX_4_4_4_4 ) + 3;
        int      labelFlen      = 8;
        uint32_t numFmtField    = fmtDesc | FMT_HEX_4_4_4_4;
        uint32_t labelFmtField  = fmtDesc | FMT_BOLD;

        padLine( fmtDesc );
        setWinCursor( 6, 1 );
        printTextField((char *) "PID=", labelFmtField, labelFlen );
    
        for ( int i = 4; i < 8; i++ ) {

            printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
    }
    
    if ( toggleVal == 2 ) {

        int      numFlen        = glb -> console -> numberFmtLen( FMT_HEX_4_4_4_4 ) + 3;
        int      labelFlen      = 8;
        uint32_t numFmtField    = fmtDesc | FMT_HEX_4_4_4_4;
        uint32_t labelFmtField  = fmtDesc | FMT_BOLD;
        
        setWinCursor( 2, 1 );
        printTextField((char *) "CR0=", labelFmtField, labelFlen );
    
        for ( int i = 0; i < 4; i++ ) {

            printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
        setWinCursor( 3, 1 );
        printTextField((char *) "CR4=", labelFmtField, labelFlen );
    
        for ( int i = 4; i < 8; i++ ) {

            printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
        setWinCursor( 4, 1 );
        printTextField((char *) "CR8=", labelFmtField, labelFlen );
    
        for ( int i = 8; i < 12; i++ ) {

            printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
        setWinCursor( 5, 1 );
        printTextField((char *) "CR12=", labelFmtField, labelFlen );
    
        for ( int i = 12; i < 16; i++ ) {

            printNumericField( cpu -> getControlReg( i ), numFmtField, numFlen );
        }

        padLine( fmtDesc );
    }
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
SimWinAbsMem::SimWinAbsMem( SimGlobals *glb, int modNum, T64Word adr ) : 
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
void SimWinAbsMem::setDefaults( ) {
    
    setWinType( WT_MEM_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 4 );
    setWinDefSize( 0, DEF_WIN_ROW_ABS_MEM, DEF_WIN_COL_ABS_MEM );
    setWinDefSize( 1, DEF_WIN_ROW_ABS_MEM, DEF_WIN_COL_ABS_MEM );
    setWinDefSize( 2, DEF_WIN_ROW_ABS_MEM, DEF_WIN_COL_ABS_MEM );
    setWinDefSize( 3, DEF_WIN_ROW_ABS_MEM, DEF_WIN_COL_ABS_MEM );
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
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
void SimWinAbsMem::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;

    if ( getWinToggleVal( ) == 2 )  setRadix( 10 ); 
    else                            setRadix( 16 ); 

    T64Memory *mem = (T64Memory *) glb -> system -> lookupByAdr( getCurrentItemAdr( ));

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
    printRadixField( fmtDesc | FMT_LAST_FIELD );

    if ( ! isAlignedDataAdr( getCurrentItemAdr( ), 8 )) {
        
        setCurrentItemAdr( rounddown( getCurrentItemAdr( ), 8 ));
    }   

    // ??? correlate toggle val with radix....
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
void SimWinAbsMem::drawLine( T64Word itemAdr ) {

    uint32_t    fmtDesc     = FMT_DEF_ATTR;
    uint32_t    limit       = getLineIncrementItemAdr( ) - 1; // ??? why - 1?

    T64Memory   *mem = (T64Memory *) glb -> system -> lookupByAdr( getCurrentItemAdr( ));
    if ( mem == nullptr ) {

        printTextField((char *) "Invalid address" );
    }; 

    printTextField((char *) "(", fmtDesc );
    printNumericField( itemAdr, fmtDesc | FMT_HEX_2_4_4 );
    printTextField((char *) "): ", fmtDesc );

    if ( getWinToggleVal( ) == 0 ) {

        for ( int i = 0; i < limit; i = i + 4 ) {
        
            uint32_t val = 0;
            glb -> system -> readMem( itemAdr + i, (uint8_t *)&val, sizeof( val ));

            copyEndianAware((uint8_t *) &val, (uint8_t *) &val, sizeof( val ));
            printNumericField( val, fmtDesc | FMT_HEX_4_4 );

            printTextField((char *) "   " );
        }
    }
    else if ( getWinToggleVal( ) == 1 ) {

        for ( int i = 0; i < limit; i = i + 8 ) {
        
            T64Word val = 0;
            glb -> system -> readMem( itemAdr + i, (uint8_t *)&val, sizeof( val ));

            copyEndianAware((uint8_t *) &val, (uint8_t *) &val, sizeof( val ));

            printNumericField( val, fmtDesc | FMT_HEX_4_4_4_4 );
            printTextField((char *) "   " );
        }
    }
    else if ( getWinToggleVal( ) == 2 ) {

        for ( int i = 0; i < limit; i = i + 4 ) {
        
            uint32_t val = 0;
            glb -> system -> readMem( itemAdr + i, (uint8_t *)&val, sizeof( val ));

            copyEndianAware((uint8_t *) &val, (uint8_t *) &val, sizeof( val ));

            printNumericField( val, fmtDesc | FMT_DEC_32 );
            printTextField((char *) "   " );
        }
    }
    else if ( getWinToggleVal( ) == 3 ) {

        for ( int i = 0; i < limit; i = i + 4 ) {
        
            uint32_t val = 0;
            glb -> system -> readMem( itemAdr + i, (uint8_t *)&val, sizeof( val ));

            copyEndianAware((uint8_t *) &val, (uint8_t *) &val, sizeof( val ));
            
            printNumericField( val, fmtDesc | FMT_ASCII_4 );
            printTextField((char *) "   " );
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
    setWinDefSize( 0, DEF_WIN_ROW_CODE_MEM, DEF_WIN_COL_CODE_MEM );
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
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
    
    uint32_t    fmtDesc         = FMT_BOLD | FMT_INVERSE;
    T64Word     currentIa       = getCurrentItemAdr( );
    T64Word     currentIaLimit  = 
                currentIa + (( getRows( ) - 1 ) * getLineIncrementItemAdr( ));
    T64Word     currentIaOfs    = 0;
    
    SimTokId    currentCmd      = glb -> winDisplay -> getCurrentCmd( );
    bool        hasIaOfsAdr     = (( currentIaOfs >= currentIa ) && 
                                    ( currentIaOfs <= currentIaLimit ));
    
    if (( currentCmd == CMD_STEP ) && ( hasIaOfsAdr )) {
        
        if      ( currentIaOfs >= currentIaLimit ) winJump( currentIaOfs );
        else if ( currentIaOfs < currentIa )       winJump( currentIaOfs );
    }
    
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
// passed the item address and need to map this to the actual meaning of the 
// particular window. The disassembled format is printed in two parts, the first
// is the instruction and options, the second is the target and operand field. 
// We make sure that both parts are nicely aligned.
//
//----------------------------------------------------------------------------------------
void SimWinCode::drawLine( T64Word itemAdr ) {
    
    uint32_t    fmtDesc                     = FMT_DEF_ATTR;
    uint32_t    instr                       = 0x0;
    char        buf[ MAX_TEXT_LINE_SIZE ]   = { 0 };

    if ( ! glb -> system -> readMem( itemAdr, (uint8_t *) &instr, sizeof( uint32_t ))) {

        printNumericField( 0, fmtDesc | FMT_INVALID_NUM );
    } 

    copyEndianAware((uint8_t *) &instr, (uint8_t *) &instr, sizeof(uint32_t));
    
    printNumericField( itemAdr, fmtDesc | FMT_ALIGN_LFT | FMT_HEX_2_4_4, 14 );
  
    if ( itemAdr ==  0 ) printTextField((char *) "    >", fmtDesc, 5 );
    else                 printTextField((char *) "     ", fmtDesc, 5 );
   
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
// Methods for the TLB class.
//
//----------------------------------------------------------------------------------------
// Object constructor. All we do is to remember the reference to the TLB object.
//
//----------------------------------------------------------------------------------------
SimWinTlb::SimWinTlb( SimGlobals *glb, 
                      int        modNum, 
                      T64Tlb     *tlb ) : SimWinScrollable( glb ) { 

    this -> tlb = tlb;

    setWinModNum( modNum );
    setDefaults( );
}

//----------------------------------------------------------------------------------------
// We have a function to set reasonable default values for the window. The default 
// values are the initial settings when windows is brought up the first time, or for 
// the WDEF command. The TLB window is a window where the number of lines to display 
// can be set. However, the minimum is the default number of lines.
//
//----------------------------------------------------------------------------------------
void SimWinTlb::setDefaults( ) {
    
    setWinType( WT_TLB_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( 1 );
    setWinDefSize( 0, DEF_WIN_ROW_TLB, DEF_WIN_COL_TLB );
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
    setCurrentItemAdr( 0 );
    setLineIncrementItemAdr( 1 );
    setLimitItemAdr( tlb -> getTlbSize( ));
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The banner line is always shown in 
// inverse and contains summary or head data for the window. We also need to set 
// the item address limit. As this can change with some commands outside the windows
// system, better set it every time.
//
// Format:
//
//  <windId> Proc: n Tlb: n Set: n Current: 0x0000.  <rdx>
// 
//----------------------------------------------------------------------------------------
void SimWinTlb::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
   
    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );
    printTextField((char *) "Mod:" );
    printNumericField( getWinModNum( ), ( fmtDesc | FMT_DEC ));
    printTextField((char *) " ( ", fmtDesc );
    printTextField((char *) tlb -> getTlbTypeString( ), fmtDesc );
    printTextField((char *) " ) ", fmtDesc );
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//----------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after
// the banner line. The number of lines can vary. A line represents an entry in 
// the respective TLB.
//
// Format:
//
//  (0x0000) [xxxx][xx] : va: 0x00_0000_0000_0000 pa: 0x00_0000_0000 Pid: 0x0000_0000
//----------------------------------------------------------------------------------------
void SimWinTlb::drawLine( T64Word index ) {

    uint32_t  fmtDesc = FMT_DEF_ATTR;

    T64TlbEntry *ePtr = tlb -> getTlbEntry( index );

    printTextField((char *) "(", fmtDesc );
    printNumericField( index, fmtDesc | FMT_HEX_4 );
    printTextField((char *) "): [", fmtDesc );
    printTextField(( ePtr -> valid ) ? (char *) "V" : (char *) "v" );
    printTextField(( ePtr -> modified ) ? (char *) "M" : (char *) "m" );
    printTextField(( ePtr -> locked ) ? (char *) "L" : (char *) "l" );
    printTextField(( ePtr -> uncached ) ? (char *) "U" : (char *) "u" );
    printTextField((char *) "] [", fmtDesc );
    printTextField((char *) pageTypeStr( ePtr -> pageType ));
    printTextField((char *) ":", fmtDesc );
    printTextField(( ePtr -> pLev1 ) ? (char *) "P" : (char *) "p" );
    printTextField((char *) ":", fmtDesc );
    printTextField(( ePtr -> pLev2 ) ? (char *) "P" : (char *) "p" );
    printTextField((char *) "]", fmtDesc );
    printTextField((char *) "  vAdr: ", fmtDesc );
    printNumericField( ePtr -> vAdr, fmtDesc | FMT_HEX_2_4_4_4 );
    printTextField((char *) "  pAdr: ", fmtDesc );
    printNumericField( ePtr -> pAdr, fmtDesc | FMT_HEX_2_4_4 );
    printTextField((char *) "  len: ", fmtDesc );
    printNumericField( ePtr -> pageSize, fmtDesc | FMT_HEX_8 );
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the Cache class.
//
//----------------------------------------------------------------------------------------
// Object constructor. We are passed reference to global, the processor module number
// and the reference to the cache object.
//
//----------------------------------------------------------------------------------------
SimWinCache::SimWinCache( SimGlobals    *glb, 
                          int           modNum,
                          T64Cache      *cache  ) : SimWinScrollable( glb ) { 

    this -> cache = cache;

    setWinModNum( modNum );
    setDefaults( );
}

//----------------------------------------------------------------------------------------
// We have a function to set reasonable default values for the window. The default 
// values are the initial settings when windows is brought up the first time, or 
// for the WDEF command. The TLB window is a window  where the number of lines to
// display can be set. However, the minimum is the default number of lines.
//
//----------------------------------------------------------------------------------------
void SimWinCache::setDefaults( ) {

    setWinType( WT_CACHE_WIN );
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));

    setWinToggleLimit( cache -> getWays( ));

    for ( int i = 0; i < getWinToggleLimit( ); i++ ) 
        setWinDefSize( i, DEF_WIN_ROW_CACHE, DEF_WIN_COL_CACHE );
    
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
    setCurrentItemAdr( 0 );
    setLineIncrementItemAdr( 1 );

    if ( cache -> getCacheLineSize( ) == 32 ) 
        setLimitItemAdr( cache -> getSetSize( ));
    else setLimitItemAdr( cache -> getSetSize( ) * 2 );
    
    setWinToggleVal( 0 );
    setEnable( true );
}

//----------------------------------------------------------------------------------------
// Each window consist of a headline and a body. The banner line is always shown 
// in inverse and contains summary or head data for the window. We also need to 
// set the item address limit. As this can change with some commands outside the 
// windows system, better set it every time.
//
// Format:
//
//  <windId> Cpu: n Set: n Current: 0x0000.  <rdx>
//
//----------------------------------------------------------------------------------------
void SimWinCache::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;

    setWinCursor( 1, 1 );
    printWindowIdField( fmtDesc );
    printTextField((char *) "Mod:" );
    printNumericField( getWinModNum( ), ( fmtDesc | FMT_DEC ));
    printTextField((char *) " ( ", fmtDesc );
    printTextField((char *) cache -> getCacheTypeString( ), fmtDesc );
    printTextField((char *) " ) ", fmtDesc );
    printTextField((char *) "  Way: " );
    printNumericField( getWinToggleVal( ), ( fmtDesc | FMT_DEC ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//----------------------------------------------------------------------------------------
// The draw line methods for the cache lists a cache entry. There are various cache
// line sizes. And there are up to two sets of cache data. For the 32 byte size, we
// will print four words, all fits in one row. For the 64 byte size, two lines will
// be printed. We will still increment the lineItem index by one, but fetch only
// every other time the cache. On an even index a new cache line start and the first
// half of the line is printed, on an odd index the second half is printed.
//
// Format:
//
//  (0x0000): [xxx] [0x00_00000_0000] 0x0000_0000_0000_0000 0x... 0x... 0x... 
//----------------------------------------------------------------------------------------
void SimWinCache::drawLine( T64Word index ) {

    uint32_t fmtDesc = FMT_DEF_ATTR;

    T64CacheLineInfo *cInfo;
    uint8_t          *cData;
    bool             firstHalf = true;
    
    if ( cache -> getCacheLineSize( ) == 64 ) {
        
        firstHalf = ( index % 2 == 0 );
        index = index / 2;
    }

    if ( ! cache -> getCacheLineByIndex( getWinToggleVal( ),
                                  index,
                                  &cInfo, 
                                  &cData )) {

    }
    else ;

    printTextField((char *) "(", fmtDesc );
    printNumericField( index, fmtDesc | FMT_HEX_4 );
    printTextField((char *) "): ", fmtDesc );

    if ( index > getLimitItemAdr( )) {
  
        printTextField((char *) "[", fmtDesc );
        printTextField((char *) "Invalid Cache index", fmtDesc );
        printTextField((char *) "]", fmtDesc );
        padLine( );
    }
    else {

        if ( firstHalf ) {

            printTextField((char *) "[", fmtDesc );

            if ( cInfo -> valid ) printTextField((char *) "V", fmtDesc );
            else printTextField((char *) "v", fmtDesc );

            if ( cInfo -> modified ) printTextField((char *) "M", fmtDesc );
            else printTextField((char *) "m", fmtDesc );

            printTextField((char *) "] [", fmtDesc );
            printNumericField( cInfo -> tag, fmtDesc | FMT_HEX_2_4_4 );
            printTextField((char *) "] ", fmtDesc );
        }
        else printTextField(( char *) " ", fmtDesc, 20 );

        int start = ( firstHalf ? 0 : 4 );
       
        for ( int i = 0; i < 4; i++ ) { 

            T64Word tmp;
            copyEndianAware((uint8_t *) &tmp, 
                             cData + (( start + i ) * sizeof( T64Word )), 
                             sizeof( T64Word ));
            printNumericField( tmp, fmtDesc | FMT_HEX_4_4_4_4 );
            printTextField((char *) "  " ); 
        }         
    }
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for the text window class.
//
//----------------------------------------------------------------------------------------
// Object constructor. We are passed the globals and the file path. All we do right
// now is to remember the file name. The text window has a destructor method as 
// well. We need to close a potentially opened file.
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
    setWinDefSize( 0, DEF_WIN_ROW_TEXT, txWidth );
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
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
// we would now print an error message into the screen. This is also the time where
// we actually figure out how many lines are on the file so that we can set the 
// limitItemAdr field of the window object.
//
//----------------------------------------------------------------------------------------
void SimWinText::drawLine( T64Word index ) {
    
    uint32_t    fmtDesc = FMT_DEF_ATTR;
    char        lineBuf[ MAX_TEXT_LINE_SIZE ];
    int         lineSize = 0;
  
    if ( openTextFile( )) {

        printNumericField( index + 1, ( fmtDesc | FMT_DEC ));
        printTextField((char *) ": " );
  
        lineSize = readTextFileLine( index + 1, lineBuf, sizeof( lineBuf ));
        if ( lineSize > 0 ) {
            
            printTextField( lineBuf, fmtDesc, lineSize );
            padLine( fmtDesc );
        }
        else padLine( fmtDesc );
    }
    else printTextField((char *) "Error opening the text file", fmtDesc );
}

//----------------------------------------------------------------------------------------
// "openTextFile" is called every time we want to print a line. If the file is not
// opened yet, it will be opened now and while we are at it, we will also count the
// source lines for setting the limit in the scrollable window.
//
//----------------------------------------------------------------------------------------
bool SimWinText::openTextFile( ) {
    
    if ( textFile == nullptr ) {
        
        textFile = fopen( fileName, "r" );
        if ( textFile != nullptr ) {
           
            while( ! feof( textFile )) {
                
                if( fgetc( textFile ) == '\n' ) fileSizeLines++;
            }
            
            lastLinePos = 0;
            rewind( textFile );
            setLimitItemAdr( fileSizeLines );
        }
    }
    
    return( textFile != nullptr );
}

//----------------------------------------------------------------------------------------
// "readTextFileLine" will get a line from the text file. Unfortunately, we do not 
// have a line concept in a text file. In the worst case, we read from the beginning
// of the file, counting the lines read. To speed up a little, we remember the last
// line position read. If the requested line position is larger than the last position,
// we just read ahead. If it is smaller, no luck, we start to read from zero until we
// match. If equal, we just re-read the current line.
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
    setWinDefSize( 0, DEF_WIN_ROW_CONSOLE, DEF_WIN_COL_CONSOLE );
    setRows( getWinDefSize( 0 ).row );
    setColumns( getWinDefSize( 0 ).col );
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
