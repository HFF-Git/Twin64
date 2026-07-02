//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command Expression Parser Predefined Functions
//
//----------------------------------------------------------------------------------------
// The command interpreter features expression evaluation for command arguments. It is
// a straightforward recursive top down interpreter.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command Expression Parser Predefined Functions
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
#include "T64-SimTables.h"

//----------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//----------------------------------------------------------------------------------------
namespace {

}; // namespace

//----------------------------------------------------------------------------------------
// Assemble function.
//
// ASSEMBLE "(" <str> ")"
//----------------------------------------------------------------------------------------
void SimExprEvaluator::pFuncAssemble( SimExpr *rExpr ) {
    
    SimExpr     lExpr;
    T64Instr    instr   = 0;
    int         ret     = 0;
    
    tok -> nextToken( );
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
        
    parseExpr( &lExpr );
    if ( lExpr.typ == TYP_STR ) {
        
        ret = inlineAsm -> assembleInstr( lExpr.u.str, &instr );
        
        if ( ret == 0 ) {
            
            rExpr -> typ   = TYP_NUM;
            rExpr -> u.val = instr;
        }
        else {
            
            // ??? how do we map the ASM error codes to out error code ?
            throw ( ERR_IN_ASM_PFUNC );
        }
    }
    else throw ( ERR_EXPECTED_STR );

    if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_RPAREN );
}

//----------------------------------------------------------------------------------------
// Dis-assemble function. We take the value and produce a string. 
//
// WARNING: the expression evaluator returns a pointer to a string. This is typically
// a token and the string is value until the next token is read. This is bit ugly for
// our disassembler which returns an assembled string. When we just set the expression
// pointer and return, the string is lost. Therefore the disassemble string is a static
// variable.
//
// DISASSEMBLE "(" <str> [ "," <rdx> ] ")"
//----------------------------------------------------------------------------------------
void SimExprEvaluator::pFuncDisAssemble( SimExpr *rExpr ) {
    
    SimExpr     lExpr;
    uint32_t    instr = 0;
    int         rdx   = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    static char        asmStr[ MAX_CMD_LINE_SIZE ];
    
    tok -> nextToken( );
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
    
    parseExpr( &lExpr );
    
    if ( lExpr.typ == TYP_NUM ) {
        
        instr = lExpr.u.val;
        
        if ( tok -> tokId( ) == TOK_COMMA ) {
            
            tok -> nextToken( );
            
            if (( tok -> tokId( ) == TOK_HEX ) ||
                ( tok -> tokId( ) == TOK_DEC )) {
                
                rdx = tok -> tokVal( );
                tok -> nextToken( );
            }
            else if ( tok -> tokId( ) == TOK_EOS ) {
                
                throw ( ERR_UNEXPECTED_EOS );
            }
            else throw ( ERR_INVALID_FMT_OPT );
        }
        
        if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
        
        disAsm -> formatInstr( asmStr, sizeof( asmStr ), instr, rdx );
        
        rExpr -> typ   = TYP_STR;
        rExpr -> u.str = asmStr; 
    }
    else throw ( ERR_EXPECTED_INSTR_VAL );
}

//----------------------------------------------------------------------------------------
// Add Offset function. We take the value and return the virtual address plus 
// the offset.
//
//----------------------------------------------------------------------------------------
void SimExprEvaluator::pFuncAddOffset( SimExpr *rExpr ) {
    
    SimExpr     lExpr;
    T64Word     base   = 0;
    T64Word     offset = 0;
    
    tok -> nextToken( );
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
    
    parseExpr( &lExpr );
    if ( lExpr.typ == TYP_NUM ) {
        
        base = lExpr.u.val;
    }
    else throw ( ERR_EXPECTED_AN_OFFSET_VAL );

    if ( tok -> isToken( TOK_COMMA )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_COMMA );

    parseExpr( &lExpr );
    if ( lExpr.typ == TYP_NUM ) {
        
        offset = lExpr.u.val;
        
        if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
        
        rExpr -> typ   = TYP_NUM;
        rExpr -> u.val = addAdrOfs32( base, offset );
    }
    else throw ( ERR_EXPECTED_AN_OFFSET_VAL );
}

//----------------------------------------------------------------------------------------
// Region function. We take the value and return the virtual region portion.
//
//----------------------------------------------------------------------------------------
void SimExprEvaluator::pFuncRegion( SimExpr *rExpr ) {
    
    SimExpr lExpr;
    
    tok -> nextToken( );
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
    
    parseExpr( &lExpr );
    
    if ( lExpr.typ == TYP_NUM ) {
        
        if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
        
        rExpr -> typ   = TYP_NUM;
        rExpr -> u.val = vAdrRegionId( lExpr.u.val );
    }
    else throw ( ERR_EXPECTED_NUM_VALUE );
}

//----------------------------------------------------------------------------------------
// Offset function. We take the value and return the virtual offset portion.
//
//----------------------------------------------------------------------------------------
void SimExprEvaluator::pFuncOffset( SimExpr *rExpr ) {
    
    SimExpr lExpr;
    
    tok -> nextToken( );
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
    
    parseExpr( &lExpr );
    
    if ( lExpr.typ == TYP_NUM ) {
    
        if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
        
        rExpr -> typ   = TYP_NUM;
        rExpr -> u.val = vAdrRegionOfs( lExpr.u.val );
    }
    else throw ( ERR_EXPECTED_AN_OFFSET_VAL );
}

//----------------------------------------------------------------------------------------
// Page function. We take the value and return the virtual page portion.
//
//----------------------------------------------------------------------------------------
void SimExprEvaluator::pFuncPage( SimExpr *rExpr ) {
    
    SimExpr lExpr;
   
    tok -> nextToken( );
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
    
    parseExpr( &lExpr );
    
    if ( lExpr.typ == TYP_NUM ) {
      
        if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
        
        rExpr -> typ   = TYP_NUM;
        rExpr -> u.val = vAdrPageNum( lExpr.u.val );
    }
    else throw ( ERR_EXPECTED_NUM_VALUE );
}

//----------------------------------------------------------------------------------------
// Entry point to the predefined functions. We dispatch based on the predefined function
// token Id.
//
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parsePredefinedFunction( SimToken funcId, SimExpr *rExpr ) {
    
    switch( funcId.tid ) {
            
        case PF_ASSEMBLE:       pFuncAssemble( rExpr );     break;
        case PF_DIS_ASM:        pFuncDisAssemble( rExpr );  break;
        case PF_ADD_OFS:        pFuncAddOffset( rExpr );    break;
        case PF_REGION:         pFuncRegion( rExpr );       break;
        case PF_OFS:            pFuncOffset( rExpr );       break;
        case PF_PAGE:           pFuncPage( rExpr );         break;
            
        default: throw ( ERR_UNDEFINED_PFUNC );
    }
}
