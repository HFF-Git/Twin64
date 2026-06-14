//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - CPU Core
//
//----------------------------------------------------------------------------------------
// The CPU core is a submodule of the processor. It implements the actual CPU 
// instruction set. There are routines for execution as well as routines for
// the simulator to access the CPU registers.
//
//----------------------------------------------------------------------------------------
//
// T64 - A 64-bit Processor - CPU Core
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
// program.  If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-Processor.h"

//----------------------------------------------------------------------------------------
// Name space for local routines.
//
//----------------------------------------------------------------------------------------
namespace {

};

//****************************************************************************************
//****************************************************************************************
//
// CPU
//
//----------------------------------------------------------------------------------------
// CPU object constructor. We keep a reference to the processor object for access
// the processor components and via the processor to the system.
//
// ??? need to set physical memory boundaries... we should get them from the 
// processor.
//
//----------------------------------------------------------------------------------------
T64Cpu::T64Cpu( T64Processor *proc, T64CpuType cpuType ) {

    this -> proc    = proc;
    this -> cpuType = cpuType;
    
    switch ( cpuType ) {

        default: ;
    }

    this -> reset( );
}

//----------------------------------------------------------------------------------------
// Destructor.
//
//----------------------------------------------------------------------------------------
T64Cpu:: ~T64Cpu( ) { }   

//----------------------------------------------------------------------------------------
// CPU reset method.
//
//----------------------------------------------------------------------------------------
void T64Cpu::reset( ) {

    for ( int i = 0; i < T64_MAX_CREGS; i++ ) cRegFile[ i ] = 0;
    for ( int i = 0; i < T64_MAX_GREGS; i++ ) gRegFile[ i ] = 0;
    
    psrReg          = 0;
    instrReg        = 0;
    physMemSize     = T64_MAX_PHYS_MEM_LIMIT;
}

//----------------------------------------------------------------------------------------
// The register access routines. They are used by the simulator to get/set their
// values.
//
//----------------------------------------------------------------------------------------
T64Word T64Cpu::getGeneralReg( int index ) {
    
    if ( index == 0 ) return( 0 );
    else              return( gRegFile[ index % T64_MAX_GREGS ] );
}

void T64Cpu::setGeneralReg( int index, T64Word val ) {
    
    if ( index != 0 ) gRegFile[ index % T64_MAX_GREGS ] = val;
}

T64Word T64Cpu::getControlReg( int index ) {
    
    return( cRegFile[ index % T64_MAX_CREGS ] );
}

void T64Cpu::setControlReg( int index, T64Word val ) {
    
    cRegFile[ index % T64_MAX_CREGS ] = val;
}

T64Word T64Cpu::getPsrReg( ) {
    
    return( psrReg );
}

void T64Cpu::setPsrReg( T64Word val ) {
    
    psrReg = val;
}

//----------------------------------------------------------------------------------------
// Get/Set the general register values. The register Id is obtained from the 
// register field position in the instruction word.
//
//----------------------------------------------------------------------------------------
T64Word T64Cpu::getRegR( uint32_t instr ) {
    
    return( getGeneralReg( extractInstrRegR( instr )));
}

T64Word T64Cpu::getRegB( uint32_t instr ) {
    
    return( getGeneralReg( extractInstrRegB( instr )));
}

T64Word T64Cpu::getRegA( uint32_t instr ) {
    
    return( getGeneralReg( extractInstrRegA( instr )));
}

void T64Cpu::setRegR( uint32_t instr, T64Word val ) {
    
    setGeneralReg( extractInstrRegR( instr ), val );
}

//----------------------------------------------------------------------------------------
// Trap code helpers. Each routine fills in the trap data and raises an exception.
//
//----------------------------------------------------------------------------------------
void T64Cpu::machineCheckTrap( T64Word adr ) {

    throw( T64Trap( MACHINE_CHECK, psrReg, instrReg, adr ));
}

void T64Cpu::dataMemTlbMissTrap( T64Word adr ) {

    throw( T64Trap( DATA_TLB_MISS_TRAP, psrReg, instrReg, adr ));
}

void T64Cpu::dataMemNonAccessTlbMissTrap( T64Word adr ) {

    throw( T64Trap( DATA_TLB_MISS_TRAP, psrReg, instrReg, adr ));
}

void T64Cpu::instrTlbMissTrap( T64Word adr ) {

    throw( T64Trap( INSTR_TLB_MISS_TRAP, psrReg, instrReg, adr ));
}

void T64Cpu::instrMemAccRightsTrap( T64Word adr ) {

    throw( T64Trap( INSTR_ACC_RIGHTS_TRAP, psrReg, 0, adr ));
}

void T64Cpu::instrMemProtectionTrap( T64Word adr ) {

    throw( T64Trap( INSTR_PROTECTION_TRAP, psrReg, 0, adr ));
}

void T64Cpu::instrMemAlignmentTrap( T64Word adr ) {

    throw( T64Trap( INSTR_ALIGNMENT_TRAP, psrReg, instrReg, adr ));
}

void T64Cpu::dataMemAccRightsTrap( T64Word adr ) {

    throw( T64Trap( DATA_ACC_RIGHTS_TRAP, psrReg, instrReg, adr ));
}

void T64Cpu::dataMemProtectionTrap( T64Word adr ) {

    throw( T64Trap( DATA_PROTECTION_TRAP, psrReg, instrReg, adr ));
}

void T64Cpu::dataMemAlignmentTrap( T64Word adr ) {

    throw( T64Trap( DATA_ALIGNMENT_TRAP, psrReg, instrReg, adr ));
}

void T64Cpu::privModeOperationTrap( ) {

    throw( T64Trap( PRIV_OPERATION_TRAP, psrReg, instrReg, 0 ));
}

void T64Cpu::overFlowTrap( ) {

    throw( T64Trap( OVERFLOW_TRAP, psrReg, instrReg, 0 ));
}

void T64Cpu::illegalInstrTrap( ) {

    throw( T64Trap( ILLEGAL_INSTR_TRAP, psrReg, instrReg, 0 ));
}

//----------------------------------------------------------------------------------------
// Check routines that generate traps. We check for the condition and if not
// met, raise the corresponding trap. There are  couple of checks. The first
// is the privilege level and data alignment check. Next, we have a routine to
// check whether the region Id of the data address matches any of the protection
// Ids in the control registers. Finally, we perform the access rights check 
// based on the TLB info for the address. Note that instruction address are 
// checked for alignment and access rights but not for a region Id match.
//
//----------------------------------------------------------------------------------------
void T64Cpu::privModeCheck( ) {

    if ( extractPsrXbit( psrReg ) != 0 ) privModeOperationTrap( );
}

bool T64Cpu::regionIdCheck( uint32_t rId, bool wMode ) {

    for ( int i = 4; i < 8; i++ ) {

        if ((( extractField64( cRegFile[ i ],  0, 20 ) == rId   ) &&
             ( extractField64( cRegFile[ i ], 31,  1 ) == wMode )) ||
            (( extractField64( cRegFile[ i ], 32, 20 ) == rId   ) &&
             ( extractField64( cRegFile[ i ], 63,  1 ) == wMode ))) {

            return( true );
        }        
    }   

    return( false );  
}

void T64Cpu::instrAlignmentCheck( T64Word adr ) {

    if ( ! isAlignedAdr( adr, 4 )) instrMemAlignmentTrap( adr );
}

void T64Cpu::instrAccCheck( T64Word vAdr, uint16_t tlbInfo ) {

    uint8_t privMode = extractPsrXbit( psrReg );
    if ( privMode == 0 ) return;

    uint8_t pageType = tlbInfoPageType( tlbInfo );
    if (( pageType != PT_EXECUTE ) && ( pageType != PT_GATEWAY )) {

        instrMemAccRightsTrap( psrReg );
    } 
    
    if ( ! ( privMode <= tlbInfoPrivLevel1( tlbInfo ))) {
        
        instrMemAccRightsTrap( psrReg );
    }  
}

void T64Cpu::dataAlignmentCheck( T64Word adr, int len ) {

    if ( ! isAlignedAdr( adr, len )) dataMemAlignmentTrap( adr );
}

void T64Cpu::dataReadAccCheck( T64Word vAdr, uint16_t tlbInfo ) {

    uint8_t privMode = extractPsrXbit( psrReg );
    if ( privMode == 0 ) return;

    if ( ! ( privMode <= tlbInfoPrivLevel1( tlbInfo ))) { 
        
        dataMemAccRightsTrap( psrReg );
    }

    if ( ! regionIdCheck( vAdrRegionId( vAdr ), false )) {

        dataMemProtectionTrap( vAdr );  
    }
}

void T64Cpu::dataWriteAccCheck ( T64Word vAdr, uint16_t tlbInfo ) {

    uint8_t privMode = extractPsrXbit( psrReg );
    if ( privMode == 0 ) return;

    uint8_t pageType  = tlbInfoPageType( tlbInfo );
    uint8_t pLevel2   = tlbInfoPrivLevel2( tlbInfo );

    if ( pageType != PT_READ_WRITE ) {

        dataMemAccRightsTrap( psrReg );
    }

    if ( ! ( privMode <= pLevel2 )) {

            dataMemAccRightsTrap( psrReg );
    }

    if ( ! regionIdCheck( vAdrRegionId( vAdr ), true )) {

        dataMemProtectionTrap( vAdr );  
    }
}

void T64Cpu::addOverFlowCheck( T64Word val1, T64Word val2 ) {

    if ( willAddOverflow( val1, val2 )) overFlowTrap( );
}

void T64Cpu::subUnderFlowCheck( T64Word val1, T64Word val2 ) {

    if ( willSubOverflow( val1, val2 )) overFlowTrap( );
}

void T64Cpu::nextInstr( ) {

    psrReg = addAdrOfs32( psrReg, 4 );
}

//----------------------------------------------------------------------------------------
// Compare and conditional branch condition evaluation. We compare val1 and val2
// except for the OD and EV condition, which tests a bit in val1 for being zero
// or one.
//
//----------------------------------------------------------------------------------------
int T64Cpu::evalCond( int cond, T64Word val1, T64Word val2 ) {
    
    switch ( cond ) {
                        
        case 0: return (( val1          == val2 ) ? 1 : 0 ); 
        case 1: return (( val1          <  val2 ) ? 1 : 0 ); 
        case 2: return (( val1          >  val2 ) ? 1 : 0 );  
        case 3: return ((( val1 & 0x1 ) == 0    ) ? 1 : 0 );  
        case 4: return (( val1          != val2 ) ? 1 : 0 ); 
        case 5: return (( val1          <= val2 ) ? 1 : 0 ); 
        case 6: return (( val1          >= val2 ) ? 1 : 0 ); 
        case 7: return ((( val1 & 0x1 ) != 1    ) ? 1 : 0 ); 
        default: return( 0 );
    }
}

//----------------------------------------------------------------------------------------
// Diagnostic operations. None so far.
//
//----------------------------------------------------------------------------------------
T64Word T64Cpu::diagOpHandler( int opt, T64Word arg1, T64Word arg2 ) {

    return ( 0 );
}

//----------------------------------------------------------------------------------------
// Instruction memory read. This is the central routine that fetches an instruction
// word. We first check the address range. For a physical address we must be in 
// priv mode. For a virtual address, the TLB is consulted for address translation
// and access control data.
//
//----------------------------------------------------------------------------------------
T64Word T64Cpu::instrRead( T64Word vAdr ) {

    uint32_t instr = 0;
    T64Word  pAdr  = vAdr;

    instrAlignmentCheck( vAdr );

    if ( isInPhysMemAdrRange( vAdr )) { 

        privModeCheck( );
        pAdr = vAdr;  
    }
    else {

        if ( ! proc -> localTlb -> lookupItlb( vAdr, &pAdr, &instrTlbInfo )) {

            if ( proc -> getGlobalTlbPtr( ) == nullptr ) {
                
                machineCheckTrap( vAdr );
            }

            instrTlbMissTrap( vAdr );
        }

        instrAccCheck( vAdr, instrTlbInfo );      
    }

    if ( ! proc -> busOpRead( pAdr, (uint8_t *) &instr, 4 )) {

            machineCheckTrap( vAdr );
    }  

    copyEndianAware( ((uint8_t *) &instr ), ((uint8_t *) &instr ), 4 );
    return( instr );
}

//----------------------------------------------------------------------------------------
// Data memory read. We read a data item from memory. Valid lengths are 1, 2, 4 
// and 8, aligned accordingly. The data is read from memory in the length given
// and stored right justified and sign extended in the return argument. We first
// check the address range. For a physical address we must be in priv mode. For
// a virtual address, the TLB is consulted for the translation and security 
// checking. 
//
//----------------------------------------------------------------------------------------
T64Word T64Cpu::dataRead( T64Word vAdr, int len, bool sExt, bool rsv ) {

    T64Word pAdr    = 0;
    T64Word data    = 0;
    int     wordOfs = sizeof( T64Word ) - len;

    dataAlignmentCheck( vAdr, len );
           
    if ( vAdr < physMemSize ) { 
        
        privModeCheck( );
        pAdr = vAdr;
    }
    else {

        uint16_t tlbInfo;
       
        if ( ! proc -> localTlb -> lookupDtlb( vAdr, &pAdr, &tlbInfo )) {

            if ( proc -> getGlobalTlbPtr( ) == nullptr ) {
                
                machineCheckTrap( vAdr );
            } 
        
            dataMemTlbMissTrap( vAdr );
        }

        dataReadAccCheck( vAdr, tlbInfo );      
    }

    if ( rsv ) {

        if ( ! proc -> busOpReadRsv( pAdr, ((uint8_t *) &data ) + wordOfs, len )) {

            machineCheckTrap( pAdr );
        }
    }
    else {
           
        if ( ! proc -> busOpRead( pAdr, ((uint8_t *) &data ) + wordOfs, len )) {

            machineCheckTrap( pAdr );
        }

    }

    copyEndianAware(((uint8_t *) &data ) + wordOfs, 
                    ((uint8_t *) &data ) + wordOfs, 
                    len );

    if ( sExt ) {

        switch ( len ) {

            case 1: data = extractSignedField64( data, 0, 8  ); break;
            case 2: data = extractSignedField64( data, 0, 16 ); break;
            case 4: data = extractSignedField64( data, 0, 32 ); break;
            default: ;
        }
    }

    return( data );
}

//----------------------------------------------------------------------------------------
// Data memory write. We write the data item to memory. Valid lengths are 1, 2,
// 4 and 8, aligned accordingly. The data is stored in memory in the length 
// given. We first check the address range. For a physical address we must be 
// in priv mode. For a virtual address, the TLB is consulted for the translation
// and security checking. 
//
//----------------------------------------------------------------------------------------
bool T64Cpu::dataWrite( T64Word vAdr, T64Word data, int len, bool cond ) {

    T64Word pAdr    = 0;
    int     wordOfs = sizeof( T64Word ) - len;

    dataAlignmentCheck( vAdr, len );

    copyEndianAware(((uint8_t *) &data ) + wordOfs, 
                    ((uint8_t *) &data ) + wordOfs, 
                    len );
  
    if ( vAdr < physMemSize ) {
        
        privModeCheck( );
        pAdr = vAdr;
    }
    else {

        uint16_t tlbInfo;

        if ( ! proc -> localTlb -> lookupDtlb( vAdr, &pAdr, &tlbInfo )) {

            if ( proc -> getGlobalTlbPtr( ) == nullptr ) {
                
                machineCheckTrap( vAdr );
            }

            dataMemTlbMissTrap( vAdr );
        }
        
        dataWriteAccCheck( vAdr, tlbInfo ); 
    }

    if ( cond ) {

        if ( ! proc -> busOpWriteCond( pAdr, ((uint8_t *) &data ) + wordOfs, len )) {

            machineCheckTrap( pAdr );
        }
    }
    else {

        if ( ! proc -> busOpWrite( pAdr, ((uint8_t *) &data ) + wordOfs, len )) {

            machineCheckTrap( pAdr );
        }
    }

    return( true );
}

//----------------------------------------------------------------------------------------
// Read memory data based using RegB and the IMM-13 offset to form the address.
//
//----------------------------------------------------------------------------------------
T64Word T64Cpu::dataReadRegBOfsImm13( uint32_t instr, bool sExt, bool rsv ) {
    
    T64Word     adr     = getRegB( instr );
    int         dw      = extractInstrDwField( instr ); 
    T64Word     ofs     = extractInstrSignedScaledImm13( instr );
    int         len     = 1U << dw;
    
    return( dataRead( addAdrOfs32( adr, ofs ), len, sExt, rsv ));
}

//----------------------------------------------------------------------------------------
// Read memory data based using RegB and the RegX offset to form the address.
//
//----------------------------------------------------------------------------------------
T64Word T64Cpu::dataReadRegBOfsRegX( uint32_t instr, bool sExt ) {
    
    T64Word     adr     = getRegB( instr );
    int         dw      = extractInstrDwField( instr );
    T64Word     ofs     = getRegA( instr ) << dw;
    int         len     = 1U << dw;

    return( dataRead( addAdrOfs32( adr, ofs ), len, sExt ));
}

//----------------------------------------------------------------------------------------
// Write data to memory based using RegB and the IMM-13 offset to form the 
// address.
//
//----------------------------------------------------------------------------------------
bool T64Cpu::dataWriteRegBOfsImm13( uint32_t instr, bool cond ) {
    
    T64Word adr       = getRegB( instr );
    int     dw        = extractInstrDwField( instr );
    T64Word ofs       = extractInstrSignedScaledImm13( instr );
    T64Word targetAdr = addAdrOfs32( adr, ofs );
    int     len     = 1 << dw;
    T64Word val     = getRegR( instr );
    
    return( dataWrite( targetAdr, val, len, cond ));
}

//----------------------------------------------------------------------------------------
// Write data to memory based using RegB and the RegX offset to form the
// address. In addition, we return the computed target address.
//
//----------------------------------------------------------------------------------------
bool T64Cpu:: dataWriteRegBOfsRegX( uint32_t instr ) {
    
    T64Word adr     = getRegB( instr );
    int     dw      = extractInstrDwField( instr );
    T64Word ofs     = getRegA( instr ) << dw;
    T64Word targetAdr = addAdrOfs32( adr, ofs );
    int     len     = 1U << dw;
    T64Word val     = getRegR( instr );
  
    return( dataWrite( targetAdr, val, len ));
}

//----------------------------------------------------------------------------------------
// ALU:NOP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluNopOp( T64Instr instr ) {

    if ( instr != 0 ) illegalInstrTrap( );

    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:ADD operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluAddOp( T64Instr instr ) {

    T64Word val1 = getRegB( instr );
    T64Word val2 = 0;

    switch ( extractInstrFieldU( instr, 19, 3 )) {
                        
        case 0: { 
            
            if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
            if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
            val2 = getRegA( instr ); 

        } break;       
        
        case 1: { 
            
            val2 = extractInstrSignedImm15( instr ); 
            
        } break;

        default: illegalInstrTrap( );
    }

    addOverFlowCheck( val1, val2 );
    setRegR( instr, val1 + val2 );            
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:ADD operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemAddOp( T64Instr instr ) {

    T64Word val1 = getRegR( instr );
    T64Word val2 = 0;
               
    switch ( extractInstrFieldU( instr, 19, 3 )) {
                   
        case 0: { 
            
            val2 = extractInstrSignedScaledImm13( instr );
         
        } break;

        case 1: { 

            if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
            val2 = dataReadRegBOfsRegX( instr, true );
        
        } break;

        default: illegalInstrTrap( );
    }

    addOverFlowCheck( val1, val2 );
    setRegR( instr, val1 + val2 );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU_SUB operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluSubOp( T64Instr instr ) {

    T64Word val1 = getRegR( instr );
    T64Word val2 = 0;
    
    switch ( extractInstrFieldU( instr, 19, 3 )) {
                        
        case 0: { 
            
            if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
            if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
            val2 = getRegA( instr ); 

        } break;       
        
        case 1: { 
            
            val2 = extractInstrSignedImm15( instr ); 
            
        } break;

        default: illegalInstrTrap( );
    }
            
    subUnderFlowCheck( val1, val2 );
    setRegR( instr, val1 - val2 );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:SUB operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemSubOp( T64Instr instr ) {

    T64Word val1 = getRegR( instr );
    T64Word val2 = 0;

    switch ( extractInstrFieldU( instr, 19, 3 )) {
                   
        case 0: { 
            
            val2 = extractInstrSignedScaledImm13( instr );
         
        } break;

        case 1: { 

            if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
            val2 = dataReadRegBOfsRegX( instr, true );
        
        } break;

        default: illegalInstrTrap( );
    }

    subUnderFlowCheck( val1, val2 );
    setRegR( instr, val1 - val2 );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:AND operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluAndOp( T64Instr instr ) {

    T64Word val1 = getRegB( instr );
    T64Word val2 = 0;

    if ( extractInstrBit( instr, 19 )) {
        
        val2 = extractInstrSignedImm15( instr );
    }
    else {

        if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = getRegA( instr );
    }
    
    if ( extractInstrBit( instr, 20 )) val1 = ~ val1;
    T64Word res = val1 & val2;
    if ( extractInstrBit( instr, 21 )) res = ~ res;
    setRegR( instr, res );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:AND operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemAndOp( T64Instr instr ) {

    T64Word val1 = getRegB( instr );
    T64Word val2 = 0;

    if ( extractInstrBit( instr, 19 )) {
        
        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = dataReadRegBOfsRegX( instr, true );
    }
    else val2 = dataReadRegBOfsImm13( instr, true );
    
    if ( extractInstrBit( instr, 20 )) val1 = ~ val1;
    T64Word res = val1 & val2;
    if ( extractInstrBit( instr, 21 )) res = ~ res;
    setRegR( instr, res );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:OR operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluOrOp( T64Instr instr ) {

    T64Word val1 = getRegB( instr );
    T64Word val2 = 0;

    if ( extractInstrBit( instr, 19 )) {
        
        val2 = extractInstrSignedImm15( instr );
    }
    else {

        if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = getRegA( instr );
    }
    
    if ( extractInstrBit( instr, 20 )) val1 = ~ val1;
    T64Word res = val1 | val2;
    if ( extractInstrBit( instr, 21 )) res = ~ res;
    setRegR( instr, res );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:OR operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemOrOp( T64Instr instr ) {

    T64Word val1 = getRegB( instr );
    T64Word val2 = 0;

    if ( extractInstrBit( instr, 19 )) {
        
        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = dataReadRegBOfsRegX( instr, true );
    }
    else val2 = dataReadRegBOfsImm13( instr, true );
    
    if ( extractInstrBit( instr, 20 )) val1 = ~ val1;
    T64Word res = val1 | val2;
    if ( extractInstrBit( instr, 21 )) res = ~ res;
    setRegR( instrReg, res );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:XOR operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluXorOp( T64Instr instr ) {

    T64Word val1 = getRegB( instr );
    T64Word val2 = 0;

    if ( extractInstrBit( instr, 20 )) illegalInstrTrap( );

    if ( extractInstrBit( instr, 19 )) {
        
        val2 = extractInstrSignedImm15( instr );
    } 
    else {

        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = getRegA( instr );
    }                      
    
    T64Word res  = val1 ^ val2;
    if ( extractInstrBit( instr, 21 )) res = ~ res;
    setRegR( instr, res );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:XOR operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemXorOp( T64Instr instr ) {

    T64Word val1 = getRegB( instr );
    T64Word val2 = 0;

    if ( extractInstrBit( instr, 20 )) illegalInstrTrap( );
    
    if ( extractInstrBit( instr, 19 )) {
        
        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = dataReadRegBOfsRegX( instr, true );
    }
    else val2 = dataReadRegBOfsImm13( instr, true );
    
    T64Word res = val1 ^ val2;
    if ( extractInstrBit( instr, 21 )) res = ~ res;
    setRegR( instr, res );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:CMP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluCmpOp( T64Instr instr ) {

    T64Word val1   = getRegB( instr );
    T64Word val2   = 0;
    int     opCode = extractInstrOpNum( instr );
    
    if ( opCode == OPC_CMP_A ) {
        
        if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = getRegA( instr );
    }
    else if ( opCode == OPC_CMP_B ) val2 = extractInstrSignedImm15( instr );
    else illegalInstrTrap( );
    
    setRegR( instr, evalCond( extractInstrFieldU( instr, 19, 3 ), val1, val2 ));
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:CMP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemCmpOp( T64Instr instr ) {

    T64Word val1   = getRegB( instrReg );
    T64Word val2   = 0;
    int     opCode = extractInstrOpNum( instr );
    
    if ( opCode == OPC_CMP_A ) { 
        
        val2 = dataReadRegBOfsImm13( instr, true );
    }
    else if ( opCode == OPC_CMP_B ) {
        
        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        val2 = dataReadRegBOfsRegX( instr, true );
    }
    else illegalInstrTrap( );
    
    setRegR( instr, evalCond( extractInstrFieldU( instr, 19, 3 ), val1, val2 ));
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:BITOP operation.
// 
//  0 -> EXTR
//  1 -> DEP
//  2 -> DSR
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluBitOp( T64Instr instr ) {

    switch ( extractInstrFieldU( instr, 19, 3 )) {
                        
        case 0: { 
            
            T64Word val = getRegB( instr );
            T64Word res  = 0;
            int     pos  = 0;
            int     len  = extractInstrFieldU( instr, 0, 6 );

            if ( extractInstrBit( instr, 14 )) illegalInstrTrap( );
            
            if ( extractInstrBit( instr, 13 ))   
                pos = (int) cRegFile[ CTL_REG_SHAMT ] & 0x3F;
            else                               
                pos = extractInstrFieldU( instr, 6, 6 );
            
            if ( extractInstrBit( instr, 12 ))  
                res = extractSignedField64( val, pos, len );
            else                               
                res = extractField64( val, (int) pos, len );
            
            setRegR( instr, res );
            
        } break;
            
        case 1: {
            
            T64Word val1 = 0;
            T64Word val2 = 0;
            T64Word res  = 0;
            int     pos  = 0;
            int     len  = (int) extractInstrFieldU( instr, 0, 6 );
            
            if ( extractInstrBit( instr, 13 ))    
                pos = (int) cRegFile[ CTL_REG_SHAMT ] & 0x3F;
            else                                
                pos = (int) extractInstrFieldU( instr, 6, 6 );
            
            if ( ! extractInstrBit( instr, 12 )) val1 = getRegR( instr );
            
            if ( extractInstrBit( instr, 14 ))    
                val2 = extractInstrFieldU( instr, 15, 4 );
            else                                
                val2 = getRegB( instr );
            
            res = depositField64( val1, pos, len , val2 );
            setRegR( instr, res );
            
        } break;
            
        case 3: { 
            
            T64Word val1    = getRegB( instr );
            T64Word val2    = getRegA( instr );
            int     shamt   = 0;
            T64Word res     = 0;

            if ( extractInstrBit( instr, 14 )) illegalInstrTrap( );
            if ( extractInstrFieldU( instr, 6, 3 )) illegalInstrTrap( );
            
            if ( extractInstrBit( instr, 13 ))    
                shamt = (int) cRegFile[ CTL_REG_SHAMT ] & 0x3F;
            else                                
                shamt = (int) extractInstrFieldU( instr, 0, 6 );
            
            res = shiftRight128( val1, val2, shamt );
            setRegR( instr, res );
            
        } break;
            
        default: illegalInstrTrap( );
    }

    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:SHAOP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluShaOP( T64Instr instr ) {

    T64Word val1  = getRegB( instr );
    T64Word val2  = 0;
    T64Word res   = 0;
    int     shamt = extractInstrFieldU( instr, 13, 2 );
    int     opt   = extractInstrFieldU( instr, 19, 3 );

    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
    
    switch (  opt ) {

        case 0: 
        case 2: val2 = getRegA( instr ); break;

        case 1: 
        case 3: val2 = extractInstrSignedImm13( instr ); break;

        default: illegalInstrTrap( );
    }

    switch ( opt ) {

        case 0:
        case 1: {

            if ( willShiftLeftOverflow( val1, shamt )) overFlowTrap( );
            res = val1 << shamt;

        } break;

        case 2:
        case 3: {

            res = val1 >> shamt;    

        } break;

        default: illegalInstrTrap( );
    }

    addOverFlowCheck( res, val2 );
    setRegR( instrReg, res + val2 );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU_IMMOP operation.
//
//  0 -> ADDIL 
//  1 -> LDIL.L
//  2 -> LDIL.R
//  3 -> LDIL.U
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluImmOp( T64Instr instr ) {

    T64Word val = extractInstrImm20( instr );
    T64Word res = getRegR( instr );
    
    switch ( extractInstrFieldU( instr, 20, 2 )) {
            
        case 0: res = addAdrOfs32( res, val );              break;
        case 1: res = val << 12;                            break;
        case 2: res = depositField64( res, 32, 20, val );   break;
        case 3: res = depositField64( res, 52, 12, val );   break;
    }
    
    setRegR( instr, res );
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// ALU:LDO operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrAluLdoOp( T64Instr instr ) {

  
    T64Word base = getRegB( instr );
    T64Word ofs  = 0;

    switch ( extractInstrFieldU( instr, 19, 3 )) {

        case 0: ofs = extractInstrSignedScaledImm13( instr ); break;
        case 1: {
            
            if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
            ofs = getRegA( instr ); 

        } break;

        default: illegalInstrTrap( );
    }
    
    setRegR( instr, addAdrOfs32( base, ofs ));
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:LD operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemLdOp( T64Instr instr ) {

    if ( extractInstrBit( instr, 21 )) illegalInstrTrap( );

    bool sExt = ! extractInstrBit( instr, 20 );

    if( extractInstrBit( instr, 19 )) {

        if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
        setRegR( instr, dataReadRegBOfsRegX( instr, sExt ));
    }
    else setRegR( instr, dataReadRegBOfsImm13( instr, sExt ));
   
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:LDR operation.
//
// The load reserved instruction loads the value from address and remembers
// the address. It will set a reservation valid flag, encoded in bit 63 of 
// the reserved register.
//
// ??? enforce an alignment larger than 8 bytes ?
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemLdrOp( T64Instr instr ) {

    if ( extractInstrBit( instr, 19 )) illegalInstrTrap( );
    if ( extractInstrBit( instr, 21 )) illegalInstrTrap( );
    if ( extractInstrDwField( instr ) != 3 ) illegalInstrTrap( );

    bool sExt = ( extractInstrBit( instr, 20 ) == 0 );
          
    setRegR( instr, dataReadRegBOfsImm13( instr, sExt, true ));
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:ST operation.
//
// The ST instruction stores a data item to memory. In addition, the reservation
// is cleared if the address matches. In addition, we need to broadcast this
// event to all processors.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemStOp( T64Instr instr ) {

    switch ( extractInstrFieldU( instr, 19, 3 )) {

        case 0: dataWriteRegBOfsImm13( instr ); break;
        case 1: {
            
            if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );
            dataWriteRegBOfsRegX( instr );
        
        } break;

        default: illegalInstrTrap( );
    }

    nextInstr( );
}

//----------------------------------------------------------------------------------------
// MEM:STC operation.
//
// The store conditional instruction computes the target address and compares 
// it against the reserved register data. We pass the cond flag to indicate
// that we do a conditional store bus operation.
//
// ??? enforce an alignment larger than 8 bytes ? 
//----------------------------------------------------------------------------------------
void T64Cpu::instrMemStcOp( T64Instr instr ) {

    if ( extractInstrFieldU( instr, 19, 3 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrDwField( instr ) != 3 ) illegalInstrTrap( );

    if ( dataWriteRegBOfsImm13( instr, true )) {

        setRegR( instr, 1 );
    }
    else setRegR( instr, 0 );

    nextInstr( );
}

//----------------------------------------------------------------------------------------
// BR:B_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrBOp( T64Instr instr ) {

    T64Word ofs     = extractInstrSignedImm19( instr ) << 2;
    T64Word rl      = addAdrOfs32( psrReg, 4 );
    T64Word newIA   = addAdrOfs32( psrReg, ofs );

    if ( extractInstrFieldU( instr, 20, 2 ) != 0 ) illegalInstrTrap( );
    
    if ( extractInstrBit( instr, 19 )) { 

        if ( tlbInfoPageType( instrTlbInfo ) == PT_GATEWAY ) {

            depositPsrXbit( &psrReg, tlbInfoPrivLevel1( instrTlbInfo ));
        }
    }
   
    psrReg = newIA;
    setRegR( instr, rl );
}

//----------------------------------------------------------------------------------------
// BR:BE_OP operation.
//
// ??? priv check ?
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrBeOp( T64Instr instr ) {

    T64Word newIABase = getRegB( instr );
    T64Word ofs       = extractInstrSignedImm15( instr ) << 2;
    T64Word newIA     = addAdrOfs32( newIABase, ofs );
    T64Word rl        = addAdrOfs32( psrReg, 4 );

    if ( extractInstrFieldU( instr, 19, 3 ) != 0 ) illegalInstrTrap( ); 
    
    psrReg = newIA;
    setRegR( instr, rl );
}

//----------------------------------------------------------------------------------------
// BR:BR_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrBrOp( T64Instr instr ) {

    int     scale = extractInstrFieldU( instr, 13, 2 ) + 2;
    T64Word newIA = addAdrOfs32( psrReg, getRegB( instr ) << scale );
    T64Word rl    = addAdrOfs32( psrReg, 4 );

    if ( extractInstrFieldU( instr, 19, 3 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrFieldU( instr, 0, 13 ) != 0 ) illegalInstrTrap( );

    instrAlignmentCheck( newIA );
    psrReg = newIA;
    setRegR( instr, rl );
}

//----------------------------------------------------------------------------------------
// BR:BV_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrBvOp( T64Instr instr ) {

    int     scale   = extractInstrFieldU( instr, 13, 2 ) + 2;
    T64Word base    = getRegB( instr );
    T64Word rl      = addAdrOfs32( psrReg, 4 );
    T64Word newIA   = addAdrOfs32( base, getRegA( instr ) << scale );

    if ( extractInstrFieldU( instr, 19, 3 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );

    instrAlignmentCheck( newIA );
    psrReg = newIA;
    setRegR( instr, rl );
}

//----------------------------------------------------------------------------------------
// BR:BB_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrBbOp( T64Instr instr ) {

    bool    testVal = extractInstrBit( instr, 19 );
    bool    testBit = 0;
    int     pos     = 0;
    
    if ( extractInstrBit( instr, 21 )) illegalInstrTrap( );

    if ( extractInstrBit( instr, 20 )) {

        pos = cRegFile[ CTL_REG_SHAMT ] & 0x3F;
    }    
    else pos = (int) extractInstrFieldU( instr, 13, 6 );
    
    testBit = extractInstrBit( getRegR( instr ), pos );
    
    if ( testVal ^ testBit ) { 
        
        psrReg = addAdrOfs32( psrReg, extractInstrSignedImm13( instr ) << 2 );
    }
    else nextInstr( );
}

//----------------------------------------------------------------------------------------
// BR:ABR_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrAbrOp( T64Instr instr ) {

    T64Word val1    = getRegR( instr );
    T64Word val2    = getRegB( instr );
    T64Word sum     = 0;

    addOverFlowCheck( val1, val2 );
    sum = val1 + val2;
    setRegR( instr, sum );

    if ( evalCond( extractInstrFieldU( instr, 19, 3 ), sum, 0 )) {

        psrReg = addAdrOfs32( psrReg, extractInstrSignedImm15( instr ));
    } 
    else nextInstr( );
}

//----------------------------------------------------------------------------------------
// BR:CBR_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrCbrOp( T64Instr instr ) {

    T64Word val1    = getRegR( instr );
    T64Word val2    = getRegB( instr );

    if ( evalCond( extractInstrFieldU( instr, 19, 3 ), val1, val2 )) {

        psrReg = addAdrOfs32( psrReg, extractInstrSignedImm15( instr ));
    } 
    else nextInstr( );
}

//----------------------------------------------------------------------------------------
// BR:MBR_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrBrMbrOp( T64Instr instr ) {

    T64Word val = getRegB( instr );
        
    setRegR( instr, val );
    
    if ( evalCond( extractInstrFieldU( instr, 19, 3 ), val, 0 )) {

        psrReg = addAdrOfs32( psrReg, extractInstrSignedImm15( instr ));
    }
    else nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:MR_OP operation.
//
//  0       -> MFCR
//  1       -> MTCR
//  2 .. 3  -> undefined.
//  4       -> MFIA: psrReg
//  5       -> MFIA: psrReg.[ 31..12 ] 
//  6       -> MFIA: psrReg.[ 51..32 ] 
//  7       -> MFIA: psrReg.[ 63..52 ] 
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysMrOp( T64Instr instr ) {

    switch ( extractInstrFieldU( instr, 19, 3 )) {
                        
        case 0: {

            if ( extractInstrFieldU( instr, 4, 15 ) != 0 ) illegalInstrTrap( );
            
            int cReg = extractInstrFieldU( instr, 0, 4 );
            setRegR( instr, cRegFile[ cReg ] ); 
            
        } break;

        case 1: {

            if ( extractInstrFieldU( instr, 4, 15 ) != 0 ) illegalInstrTrap( );
            int cReg = extractInstrFieldU( instr, 0, 4 );
            setRegR( instr, cRegFile[ cReg ] );
            cRegFile[ cReg ] = getRegB( instr );

        } break;

        case 4: { 
            
            if ( extractInstrFieldU( instr, 0, 19 ) != 0 ) illegalInstrTrap( );
            setRegR( instr, psrReg ); 
            
        } break;
        
        case 5: {
            
            if ( extractInstrFieldU( instr, 0, 19 ) != 0 ) illegalInstrTrap( );
            setRegR( instr, extractField64( psrReg, 12, 20 )); 
            
        } break;
        
        case 6: { 
            
            if ( extractInstrFieldU( instr, 0, 19 ) != 0 ) illegalInstrTrap( );
            setRegR( instr, extractField64( psrReg, 32, 20 )); 
        
        } break;

        case 7: { 
            
            if ( extractInstrFieldU( instr, 0, 19 ) != 0 ) illegalInstrTrap( );
            setRegR( instr, extractField64( psrReg, 52, 12 )); 
            
        } break;

        default: illegalInstrTrap( );
    }
    
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:LPA_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysLpaOp( T64Instr instr ) {

    privModeCheck( );

    T64Word base = getRegB( instr );
    T64Word ofs  = getRegA( instr );
    T64Word vAdr = addAdrOfs32( base, ofs );

    if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrFieldU( instr, 19, 3 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );

    T64Word     pAdr    = 0;
    uint16_t    tlbInfo = 0;

    if ( proc -> getLocalTlbPtr( ) -> lookupDtlb( vAdr, &pAdr, &tlbInfo  )) {

        setRegR( instr, pAdr );
    }
    else if ( proc -> getLocalTlbPtr( ) -> lookupItlb( vAdr, &pAdr, &tlbInfo  )) {

        setRegR( instr, pAdr );
    }
    else setRegR( instr, 0 );

    nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:PRB operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysPrbOp( T64Instr instr ) {

    T64Word vAdr        = getRegB( instr );
    int     mode        = extractInstrFieldU( instr, 13, 2 );
    int     privLevel   = extractPsrXbit( psrReg );
    
    if ( extractInstrFieldU( instr, 19, 3 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );

    if ( mode == 3 ) mode = extractField64( getRegA( instr ), 0, 2 );

    T64Word     pAdr    = 0;
    uint16_t    tlbInfo = 0;

    if ( proc -> getLocalTlbPtr( ) -> lookupDtlb( vAdr, &pAdr, &tlbInfo  )) {

        if ( proc -> getGlobalTlbPtr( ) == nullptr ) {
                
                machineCheckTrap( vAdr );
            }

        dataMemNonAccessTlbMissTrap( vAdr );
    }
    else if ( proc -> getLocalTlbPtr( ) -> lookupItlb( vAdr, &pAdr, &tlbInfo  )) {

        if ( proc -> getGlobalTlbPtr( ) == nullptr ) {
                
                machineCheckTrap( vAdr );
            }

        // ??? non-access trap ? Is there an instruction non-access... ?
    }
    else setRegR( instr, 0 );

    if ( privLevel == 1 ) {

        // ??? compare the ACC field with mode...

    }
    else setRegR( instr, 1 );

    nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:TLB_OP operation.
//
//  0 -> IITLB
//  1 -> IDTLB
//  2 -> PITLB
//  3 -> PDTLB
//
// So far, we map both TLB types to our unified TLB.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysTlbOp( T64Instr instr ) {

    if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );

    switch ( extractInstrFieldU( instr, 19, 3 )) {

        case 0: 
        case 1: {

            proc -> globalTlb -> insertTlbEntry( getRegB( instr ), getRegA( instr ));
            setRegR( instr, 1 );

        } break;

        case 2: 
        case 3: {

            T64Word vAdr = addAdrOfs32( getRegB( instr ), getRegA( instr ));
            proc -> localTlb -> purgeTlb( vAdr );
            
            proc -> busOpControl( T64_CNTRL_EVENT_TLB_PURGE, vAdr, 0 ); 

            setRegR( instr, 1 );

        } break;

        default: illegalInstrTrap( );
    }
    
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:CA_OP operation.
//
//  0 -> FICA
//  1 -> FDCA
//  2 -> PICA
//  3 -> PDCA
//
// The cache instructions are right now a NOP. They become relevant when we 
// have true hardware and a real cache. We do however still check that the
// instruction is well formed. 
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysCaOp( T64Instr instr ) {

    if ( extractInstrFieldU( instr, 13, 2 ) != 0 ) illegalInstrTrap( );
    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );

    switch ( extractInstrFieldU( instr, 19, 3 )) {

        case 0: 
        case 1: 
        case 2:
        case 3: {

            setRegR( instr, 1 );

        } break;

        default: illegalInstrTrap( );
    }
    
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:MST_OP operation.
//
//  0 -> RSM
//  1 -> SSM
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysMstOp( T64Instr instr ) {

    if ( extractInstrFieldU( instr, 0, 8 ) != 0 ) illegalInstrTrap( );

    switch ( extractInstrFieldU( instr, 19, 3 )) {

        case 0: {

            // RSM

        } break;

        case 1: {

            // SSM
            
        } break;

        default: illegalInstrTrap( );
    }
    
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:RFI_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysRfiOp( T64Instr instr ) {

    if ( extractInstrFieldU( instr, 0, 22 ) != 0 ) illegalInstrTrap( );

    setRegR( instr, addAdrOfs32( psrReg, 4 ));
    psrReg = cRegFile[ CTL_REG_IPSR ];
}

//----------------------------------------------------------------------------------------
// SYS:DIAG_OP operation.
//
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysDiagOp( T64Instr instr ) {

    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );

    int diagOpt = ( extractInstrFieldU( instr, 19, 3 ) * 4 ) + 
                    extractInstrFieldU( instr, 13, 2 );

    setRegR( instr, diagOpHandler( diagOpt, getRegB( instr ), getRegA( instr )));
    nextInstr( );
}

//----------------------------------------------------------------------------------------
// SYS:TRAP_OP operation.
//
//  what exactly do we do here ? Can we pass arguments ? Or do we just move 
// them to control registers scratch 1 and 2 ????
//----------------------------------------------------------------------------------------
void T64Cpu::instrSysTrapOp( T64Instr instr ) {

    if ( extractInstrFieldU( instr, 0, 9 ) != 0 ) illegalInstrTrap( );

    int trapOpt = ( extractInstrFieldU( instr, 19, 3 ) * 4 ) + 
                    extractInstrFieldU( instr, 13, 2 );

    throw( T64Trap( USER_DEFINED_TRAP, psrReg, instrReg, trapOpt ));
}

//----------------------------------------------------------------------------------------
// Execute an instruction at instruction address location. This is the key 
// routine of the simulator. Essentially a big case statement. Each instruction
// is encoded based on the instruction group and the opcode family. 
//
// When traps happen, the control registers are set with the trap information 
// and execution continuous at the IVA address slot for the respective trap.
//
//----------------------------------------------------------------------------------------
T64TrapCode T64Cpu::executeInstr( ) {

    uint32_t instr = instrRead( extractField64( psrReg, 0, 52 ));

    try {
        
        switch ( extractInstrOpCode( instr ) ) {
                
            case ( OPC_GRP_ALU * 16 + OPC_NOP ):    instrAluNopOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_ADD ):    instrAluAddOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_ADD ):    instrMemAddOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_SUB ):    instrAluSubOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_SUB ):    instrMemSubOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_AND ):    instrAluAndOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_AND ):    instrMemAndOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_OR ):     instrAluOrOp( instr );    break;
            case ( OPC_GRP_MEM * 16 + OPC_OR ):     instrMemOrOp( instr );    break;
            case ( OPC_GRP_ALU * 16 + OPC_XOR ):    instrAluXorOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_XOR ):    instrMemXorOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_CMP_A ):  instrAluCmpOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_CMP_B ):  instrAluCmpOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_CMP_A ):  instrMemCmpOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_CMP_B ):  instrMemCmpOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_BITOP ):  instrAluBitOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_SHAOP ):  instrAluShaOP( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_IMMOP ):  instrAluImmOp( instr );   break;
            case ( OPC_GRP_ALU * 16 + OPC_LDO ):    instrAluLdoOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_LD ):     instrMemLdOp( instr );    break;
            case ( OPC_GRP_MEM * 16 + OPC_LDR ):    instrMemLdrOp( instr );   break;
            case ( OPC_GRP_MEM * 16 + OPC_ST ):     instrMemStOp( instr );    break;
            case ( OPC_GRP_MEM * 16 + OPC_STC ):    instrMemStcOp( instr );   break;
            case ( OPC_GRP_BR * 16 + OPC_B ):       instrBrBOp( instr );      break;
            case ( OPC_GRP_BR * 16 + OPC_BE ):      instrBrBeOp( instr );     break;
            case ( OPC_GRP_BR * 16 + OPC_BR ):      instrBrBrOp( instr );     break;
            case ( OPC_GRP_BR * 16 + OPC_BB ):      instrBrBbOp( instr );     break;
            case ( OPC_GRP_BR * 16 + OPC_ABR ):     instrBrAbrOp( instr );    break;
            case ( OPC_GRP_BR * 16 + OPC_CBR ):     instrBrCbrOp( instr );    break;
            case ( OPC_GRP_BR * 16 + OPC_MBR ):     instrBrMbrOp( instr );    break;
            case ( OPC_GRP_SYS * 16 + OPC_MR ):     instrSysMrOp( instr );    break;
            case ( OPC_GRP_SYS * 16 + OPC_LPA ):    instrSysLpaOp( instr );   break;
            case ( OPC_GRP_SYS * 16 + OPC_PRB ):    instrSysPrbOp( instr );   break;
            case ( OPC_GRP_SYS * 16 + OPC_TLB ):    instrSysTlbOp( instr  );  break;
            case ( OPC_GRP_SYS * 16 + OPC_CA ):     instrSysCaOp( instr );    break;
            case ( OPC_GRP_SYS * 16 + OPC_MST ):    instrSysMstOp( instr );   break;
            case ( OPC_GRP_SYS * 16 + OPC_RFI ):    instrSysRfiOp( instr );   break;
            case ( OPC_GRP_SYS * 16 + OPC_DIAG ):   instrSysDiagOp( instr );  break;
            case ( OPC_GRP_SYS * 16 + OPC_TRAP ):   instrSysTrapOp( instr );  break;
            
            default: illegalInstrTrap( );
        }

        return ( NO_TRAP );
    }
    catch ( const T64Trap t ) {

        proc -> setRsvInfo( 0, false );

        T64TrapCode code = t.trapCode;

        cRegFile[ CTL_REG_IPSR   ] = t.instrAdr;
        cRegFile[ CTL_REG_IARG_0 ] = t.arg0;
        cRegFile[ CTL_REG_IARG_1 ] = t.arg1;  

        T64Word ivaAdr  = cRegFile[ CTL_REG_IVA ];
        psrReg          = ivaAdr + ( code * 32 );

        return( code );
    }
}
