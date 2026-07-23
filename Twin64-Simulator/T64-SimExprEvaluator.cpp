//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command line expression parser
//
//----------------------------------------------------------------------------------------
// The command interpreter features expression evaluation for command arguments. 
// It is a straightforward recursive top down interpreter.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command line expression parser
// Copyright (C) 2020 - 2026 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details. 
// You should  have received a copy of the GNU General Public License along with
// this program. If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-SimDeclarations.h"
#include "T64-SimTables.h"

//----------------------------------------------------------------------------------------
// The command line features an expression evaluator for the arguments. The 
// overall syntax is as follows:
//
//      <command>   ->  <cmdId> [ <argList> ]
//      <argList>   ->  <expr> { <expr> }
//
// Expression have a type, which are NUM, BOOL, STR, SREG, GREG and CREG.
//
//      <factor> -> <number>                        |
//                  <boolean>                       |
//                  <string>                        |
//                  <envId>                         |
//                  <pswId>  [ ":" <proc> ]         |     
//                  <gregId> [ ":" <proc> ]         |
//                  <cregId> [ ":" <proc> ]         |
//                  "~" <factor>                    |
//                  <funcId> “(“ [ <argList> ] ")"  |
//                  "(" <expr> ")"
//
//      <term>          ->  <factor> { <termOp> <factor> }
//      <termOp>        ->  "*" | "/" | "%" | "&"
//
//      <simpleExpr>    ->  [ ( "+" | "-" ) ] <term> { <exprOp> <term> }
//      <simpleExprOp>  ->  "+" | "-" | "|" | "^"
//
//      <expr>          -> <simpleExpr> <relOp> <simpleExpr>
//      <relOp>         -> "==" | "!=" | "<" | "<=" | ">" | ">="   
//
// Most of the arguments to a command are evaluated as expressions. Boolean
// expressions are evaluated in short circuit form. For example, and "&&"
// operator will evaluate the left side and if false skip the right side of 
// the logical and operation. However, the parsing of the non-evaluated side
// still needs to take place, as we may have nested boolean operations.
//
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
// BitOp operation.
//
//----------------------------------------------------------------------------------------
enum BitOpId : int {
    
    AND_OP  = 0,
    OR_OP   = 1,
    XOR_OP  = 2
};

//----------------------------------------------------------------------------------------
// Add operation.
//
//----------------------------------------------------------------------------------------
void addOp( SimExpr *rExpr, SimExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: {
                    
                    if ( willAddOverflow( rExpr -> u.val, lExpr -> u.val ))
                        throw ( ERR_NUMERIC_OVERFLOW );

                    rExpr -> u.val += lExpr -> u.val; 
                    
                } break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
                
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//----------------------------------------------------------------------------------------
// Sub operation.
//
//----------------------------------------------------------------------------------------
void subOp( SimExpr *rExpr, SimExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: {
                
                    if ( willSubOverflow( rExpr -> u.val, lExpr -> u.val ))
                        throw ( ERR_NUMERIC_OVERFLOW );

                    rExpr -> u.val -= lExpr -> u.val; 
                
                } break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//----------------------------------------------------------------------------------------
// Multiply operation.
//
//----------------------------------------------------------------------------------------
void multOp( SimExpr *rExpr, SimExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: { 
                    
                    if ( willMultOverflow( rExpr -> u.val, lExpr -> u.val ))
                        throw ( ERR_NUMERIC_OVERFLOW );

                    rExpr -> u.val *= lExpr -> u.val;
                    
                } break;
                    
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//----------------------------------------------------------------------------------------
// Divide Operation.
//
//----------------------------------------------------------------------------------------
void divOp( SimExpr *rExpr, SimExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: { 

                    if ( willDivOverflow( rExpr -> u.val, lExpr -> u.val ))
                        throw ( ERR_NUMERIC_OVERFLOW );
                    
                    rExpr -> u.val /= lExpr -> u.val; 
                    
                } break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//----------------------------------------------------------------------------------------
// Modulo operation.
//
//----------------------------------------------------------------------------------------
void modOp( SimExpr *rExpr, SimExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: { 
                    
                    if ( willDivOverflow( rExpr -> u.val, lExpr -> u.val ))
                        throw ( ERR_NUMERIC_OVERFLOW );

                    rExpr -> u.val %= lExpr -> u.val; 
                    
                } break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
                
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//----------------------------------------------------------------------------------------
// Boolean operation.
//
//----------------------------------------------------------------------------------------
void bitOp( SimExpr *rExpr, SimExpr *lExpr, BitOpId op ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_BOOL: {
            
            if ( lExpr -> typ == TYP_BOOL ) {
                
                switch ( op ) {
                        
                    case AND_OP:    rExpr -> u.bVal &= lExpr -> u.bVal; break;
                    case OR_OP:     rExpr -> u.bVal |= lExpr -> u.bVal; break;
                    case XOR_OP:    rExpr -> u.bVal ^= lExpr -> u.bVal; break;
                }
            }
            else throw ( ERR_EXPR_TYPE_MATCH );
            
        } break;
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: {
                    
                    switch ( op ) {
                            
                        case AND_OP:    rExpr -> u.val &= lExpr -> u.val; break;
                        case OR_OP:     rExpr -> u.val |= lExpr -> u.val; break;
                        case XOR_OP:    rExpr -> u.val ^= lExpr -> u.val; break;
                    }
                    
                } break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//----------------------------------------------------------------------------------------
// A little helper for comparing types.
//
//----------------------------------------------------------------------------------------
bool equalTypes( SimExpr *rExpr, SimExpr *lExpr ) {

    return ( rExpr -> typ == lExpr -> typ );
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

    if ( sys -> busOpRead( nullptr, physAdr, (uint8_t *)val, size)) {

        copyEndianAware((uint8_t *) val, (uint8_t *) val, size);
        return ( true );    
    }

    return ( false );
}

}; // namespace


//----------------------------------------------------------------------------------------
// Evaluation Expression Object constructor.
//
//----------------------------------------------------------------------------------------
SimExprEvaluator::SimExprEvaluator( SimGlobals *glb, SimTokenizer *tok ) {
    
    this -> glb         = glb;
    this -> tok         = tok;
    this -> inlineAsm   = new T64Assemble( );
    this -> disAsm      = new T64DisAssemble( );
}

//----------------------------------------------------------------------------------------
//
//  ??? skipEval ? What exactly to bypass ?
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseRegister( SimExpr *rExpr ) {

    SimTokTypeId regType    = tok -> tokTyp( );
    int          regId      = tok -> tokVal( );
    int          modNum     = -1;

    rExpr -> typ      = TYP_NIL;
    rExpr -> u.val    = 0;

    tok -> nextToken( );
    if ( tok -> isToken( TOK_COLON )) {

        tok -> nextToken( );
        if ( tok -> isTokenTyp( TYP_NUM )) modNum = tok -> tokVal( );
        else throw( ERR_EXPECTED_NUM_VALUE ); 

        tok -> nextToken( );
    }
    else modNum = glb -> winDisplay -> getCurrentWinModNum( );
        
    T64ModuleType mType = glb -> system -> getModuleType( modNum );
    if ( mType != MT_PROC ) throw ( ERR_INVALID_MODULE_TYPE );

    T64Processor *proc = (T64Processor *) glb -> system -> lookupByModNum( modNum );
    if ( proc == nullptr ) throw ( ERR_INVALID_MODULE_TYPE );

    if ( regType == TYP_GREG ) {

        rExpr -> u.val =  proc -> getCpuPtr( ) -> getGeneralReg( regId );
        rExpr -> typ = TYP_NUM;
    }
    else if ( regType == TYP_CREG ) {

        rExpr -> u.val =  proc -> getCpuPtr( ) -> getControlReg( regId );
        rExpr -> typ = TYP_NUM;
    }
    else if ( regType == TYP_PREG ) {

        T64Word tmp = proc -> getCpuPtr( ) -> getPsrReg( );
        if      ( regId == 1 ) rExpr -> u.val = extractField64( tmp, 0, 52 );
        else if ( regId == 2 ) rExpr -> u.val = extractField64( tmp, 52, 12 );  
        rExpr -> typ = TYP_NUM;
    }
    else ;
}

//----------------------------------------------------------------------------------------
//
// ??? skipEval ? What exactly to bypass ?
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseMemData( SimExpr *rExpr ) {

    int  len  = sizeof( T64Word );
    bool sExt = true;

    tok -> nextToken( );

    if ( tok -> isToken( TOK_BYTE ) ||
         tok -> isToken( TOK_SHORT  ) ||
         tok -> isToken( TOK_HALF ) ||
         tok -> isToken( TOK_WORD  ) ||
         tok -> isToken( TOK_DWORD ) ||
         tok -> isToken( TOK_DOUBLE )) {

        len = tok -> tokVal( );
        tok -> nextToken( );
    }
    else if ( tok -> isToken( TOK_UBYTE ) ||
                tok -> isToken( TOK_USHORT  ) ||
                tok -> isToken( TOK_UHALF ) ||
                tok -> isToken( TOK_UWORD  )) {

        sExt = false;
        len  = tok -> tokVal( );
        tok -> nextToken( );
    }
    else ;

    parseExpr( rExpr );
    if ( rExpr -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUM_VALUE );

    if ( ! isAlignedAdr( rExpr -> u.val, len )) throw ( ERR_UNALIGNED_ADDR );

    T64Word data = 0;
    if ( readMem( glb -> system, rExpr -> u.val, (uint8_t *) &data, len )) {

        rExpr -> typ = TYP_NUM;
        rExpr -> u.val = data;   
        
            if ( sExt ) {

            switch ( len ) {

                case 1: rExpr -> u.val = signExtend( rExpr -> u.val, 7 );
                        break;

                case 2: rExpr -> u.val = signExtend( rExpr -> u.val, 15 );
                        break;

                case 4: rExpr -> u.val = signExtend( rExpr -> u.val, 31 );
                        break;
            }
        }
    }
    else throw ( ERR_MEM_OP_FAILED );
                                                
    if ( tok -> isToken( TOK_RBRACK )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_RBRACK );
}

//----------------------------------------------------------------------------------------
// "parseFactor" parses the factor syntax part of an expression. The expression directly
// ties into the value providers, i.e. a register of a processor or an environment 
// variable.
//
//      <factor> -> <number>                        |
//                  <pswRegId>  [ ":" <proc> ]      |
//                  <gRegId>    [ ":" <proc> ]      |
//                  <cRegId>    [ ":" <proc> ]      |
//                  "[" <expr "]"                   |
//                  "~" <factor>                    |
//                  "(" <expr> ")"
//
// ??? how to handle skipEval and negated ?
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseFactor( SimExpr *rExpr ) {

    rExpr -> typ       = TYP_NIL;
    rExpr -> u.val    = 0;

    if ( tok -> isTokenTyp( TYP_NIL )) {

        if ( tok -> isToken( TOK_NIL )) {

            rExpr -> typ = TYP_NIL;
            rExpr -> u.val    = 0;
        }
        else throw ( ERR_EXPR_FACTOR );
    }
    else if ( tok -> isTokenTyp( TYP_BOOL )) {

        rExpr -> typ = TYP_BOOL;
        rExpr -> u.bVal = tok -> tokVal( );
        tok -> nextToken( );
    }
    else if ( tok -> isTokenTyp( TYP_NUM )) {
        
        rExpr -> typ     = TYP_NUM;
        rExpr -> u.val  = tok -> tokVal( );
        tok -> nextToken( );
    }
    else if ( tok -> isTokenTyp( TYP_STR ))  {
        
        rExpr -> typ   = TYP_STR;
        rExpr -> u.str = tok -> tokStr( );
        tok -> nextToken( );
    }
    else if (( tok -> isTokenTyp( TYP_GREG )) || 
             ( tok -> isTokenTyp( TYP_CREG )) ||
             ( tok -> isTokenTyp( TYP_PREG )))  {

        parseRegister( rExpr );
    }
    else if ( tok -> isToken( TOK_LBRACK )) {

        parseMemData( rExpr );
    }
    else if ( tok -> isToken( TOK_NEG )) {
        
        tok -> nextToken( );
        parseFactor( rExpr );

        if ( rExpr -> typ != TYP_BOOL ) throw( ERR_EXPECTED_BOOL_VALUE );
        rExpr -> u.bVal = ! rExpr -> u.bVal;
    }
    else if ( tok -> isToken( TOK_LNOT )) {

        // ??? maybe we do not need this... NEG converts already a bool value ...

        tok -> nextToken( );
        parseExpr( rExpr );
        if ( rExpr -> typ != TYP_BOOL ) throw( ERR_EXPECTED_BOOL_VALUE ); 

        rExpr -> u.bVal = ! rExpr -> u.bVal;
    }
    else if ( tok -> isToken( TOK_LPAREN )) {
        
        tok -> nextToken( );
        parseExpr( rExpr );
            
        if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
    }
     else if ( tok -> isTokenTyp( TYP_P_FUNC )) {

        parsePredefinedFunction( tok -> token( ), rExpr );
    }
    else if ( tok -> isToken( TOK_IDENT )) {
    
        SimEnvTabEntry *entry = glb -> env -> getEnvEntry( tok -> tokName( ));
        if ( entry == nullptr ) throw( ERR_ENV_VAR_NOT_FOUND );
            
        rExpr -> typ = entry -> typ;
        rExpr -> u.str = entry -> u.strVal;
            
        switch( rExpr -> typ ) {
                    
            case TYP_NIL:                                                break;
            case TYP_BOOL:  rExpr -> u.bVal =  entry -> u.bVal;          break;
            case TYP_NUM:   rExpr -> u.val  =  entry -> u.iVal;          break;
            case TYP_STR:   strcpy( rExpr -> u.str, entry -> u.strVal ); break;
            default: throw( ERR_INVALID_EXPR );
        }
       
        tok -> nextToken( );
    }
    else if ( tok -> isToken( TOK_EOS )) {
        
        rExpr -> typ = TYP_NIL;
    }
    else throw ( ERR_EXPR_FACTOR );
}

//----------------------------------------------------------------------------------------
// "parseTerm" parses the term syntax.
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
// ??? type mix options ?
// ??? skipEval ? What exactly to bypass ?
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseTerm( SimExpr *rExpr ) {
    
    SimExpr lExpr = INIT_EXPR;

    parseFactor( rExpr );
    
    while (( tok -> tokId( ) == TOK_MULT )   ||
           ( tok -> tokId( ) == TOK_DIV  )   ||
           ( tok -> tokId( ) == TOK_MOD  )   ||
           ( tok -> tokId( ) == TOK_AND  )   ||
           ( tok -> tokId( ) == TOK_LAND ))  {

        switch ( tok -> tokId( )) {

            case TOK_MULT:   {

                tok -> nextToken( );
                parseFactor( &lExpr );
                if ( lExpr.typ == TYP_NIL ) throw ( ERR_UNEXPECTED_EOS );
                
                multOp( rExpr, &lExpr );   
                           
            } break;

            case TOK_DIV:   {

                tok -> nextToken( );
                parseFactor( &lExpr );
                if ( lExpr.typ == TYP_NIL ) throw ( ERR_UNEXPECTED_EOS );
                
                divOp( rExpr, &lExpr ); 

            } break;

            case TOK_MOD:   {

                tok -> nextToken( );
                parseFactor( &lExpr );
                if ( lExpr.typ == TYP_NIL ) throw ( ERR_UNEXPECTED_EOS );
                
                modOp( rExpr, &lExpr ); 

            } break;

            case TOK_AND:   {

                tok -> nextToken( );
                parseFactor( &lExpr );
                if ( lExpr.typ == TYP_NIL ) throw ( ERR_UNEXPECTED_EOS );
                
                bitOp( rExpr, &lExpr, AND_OP );

            } break;

            default: ;
        }
    }
}

//----------------------------------------------------------------------------------------
// "parseSimpleExpr" parses the expression syntax. 
//
//      <simpleExpr>    ->  [ ( "+" | "-" ) ] <term> { <exprOp> <term> }
//      <exprOp>        ->  "+" | "-" | "|" | "^"
//
// ??? type mix options ?
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseSimpleExpr( SimExpr *rExpr ) {
    
    SimExpr lExpr = INIT_EXPR;
    
    if ( tok -> isToken( TOK_PLUS )) {
        
        tok -> nextToken( );
        parseTerm( rExpr );
        
        if ( rExpr -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUM_VALUE );
    }
    else if ( tok -> isToken( TOK_MINUS )) {
        
        tok -> nextToken( );
        parseTerm( rExpr );
        
        if ( rExpr -> typ == TYP_NUM ) rExpr -> u.val = - (int32_t) rExpr -> u.val;
        else throw ( ERR_EXPECTED_NUM_VALUE );
    }
    else parseTerm( rExpr );

    // ??? isn't that a while loop ?

    switch ( tok -> tokId( )) {

        case TOK_PLUS: {

            tok -> nextToken( );
            parseTerm( &lExpr );
            if ( lExpr.typ != TYP_NUM ) throw ( ERR_EXPECTED_NUM_VALUE );

            addOp( rExpr, &lExpr );

        } break;

        case TOK_MINUS: {

            tok -> nextToken( );
            parseTerm( &lExpr );
            if ( lExpr.typ != TYP_NUM ) throw ( ERR_EXPECTED_NUM_VALUE );

            subOp( rExpr, &lExpr );

        } break;

        case TOK_OR: {

            tok -> nextToken( );
            parseTerm( &lExpr );
            if ( lExpr.typ != TYP_NUM ) throw ( ERR_EXPECTED_NUM_VALUE );

            bitOp( rExpr, &lExpr, OR_OP );

        } break;

        case TOK_XOR: {

            tok -> nextToken( );
            parseTerm( &lExpr );
            if ( lExpr.typ != TYP_NUM ) throw ( ERR_EXPECTED_NUM_VALUE );

            bitOp( rExpr, &lExpr, XOR_OP );

        } break;

        default: ;
    }
}

//----------------------------------------------------------------------------------------
//
//  <relationExpr>  ->  <simpleExpr> <relOp> <simpleExpr>
//  <relOp>.        ->  "==" | "!=" | "<" | "<=" | ">" | ">=" 
//
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseRelationExpr( SimExpr *rExpr ) {

    SimExpr     lExpr = INIT_EXPR;
    SimTokId    relOp;

    parseSimpleExpr( rExpr );

    relOp = tok -> tokId( );

    if (( relOp != TOK_EQ ) && 
        ( relOp != TOK_NE ) && 
        ( relOp != TOK_LT ) && 
        ( relOp != TOK_LE ) && 
        ( relOp != TOK_GT ) && 
        ( relOp != TOK_GE )) return;

    // ??? set skipEval flag ?

    tok -> nextToken( );
    parseSimpleExpr( &lExpr );

    if ( ! ( equalTypes( rExpr, &lExpr ))) throw ( ERR_EXPR_TYPE_MATCH );

    // ??? check that both are numerics ?
   
    switch ( relOp ) {

        case TOK_EQ: { 
            
            rExpr -> typ    = TYP_BOOL;
            rExpr -> u.bVal = ( rExpr -> u.val == lExpr.u.val );

        } break;

        case TOK_NE: { 
            
            rExpr -> typ    = TYP_BOOL;
            rExpr -> u.bVal = ( rExpr -> u.val != lExpr.u.val );

        } break;

        case TOK_LT: { 
            
            rExpr -> typ    = TYP_BOOL;
            rExpr -> u.bVal = ( rExpr -> u.val < lExpr.u.val );

        } break;

        case TOK_LE: { 
            
            rExpr -> typ    = TYP_BOOL;
            rExpr -> u.bVal = ( rExpr -> u.val >= lExpr.u.val );

        } break;

        case TOK_GT: { 
            
            rExpr -> typ    = TYP_BOOL;
            rExpr -> u.bVal = ( rExpr -> u.val > lExpr.u.val );

        } break;

        case TOK_GE: { 
            
            rExpr -> typ    = TYP_BOOL;
            rExpr -> u.bVal = ( rExpr -> u.val >= lExpr.u.val );

        } break;

        default: {
            
            rExpr -> typ    = TYP_NIL;
            rExpr -> u.val  = 0;
            throw ( ERR_EXPECTED_REL_OP );
        }
    }
}

//----------------------------------------------------------------------------------------
// "parseNotExpr" implements the NOT operator. Not operators can be repeated,
// which result in a recursive call to this routine.
//
//  <notExpr> -> <notOp> <relationExpr>
//  <notOp>   -> "NOT" | "!"
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseNotExpr( SimExpr *rExpr ) {

    if ( tok -> tokId( ) == TOK_LNOT ) {

        parseNotExpr( rExpr );
        if ( ! rExpr -> skipEval ) rExpr -> u.bVal = ! rExpr -> u.bVal;
    }
    else parseRelationExpr( rExpr );
}

//----------------------------------------------------------------------------------------
// "parseAndExpr" implements the logical AND operation. If the left hand side
// evaluates to "false", the right side is not evaluated but still parsed. 
//
//  <andExpr> -> <notExpr> <andOP> <notExpr>
//  <andOp>   -> "AND" | "&&"
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseAndExpr( SimExpr *rExpr ) {

    SimExpr lExpr = INIT_EXPR;

    parseNotExpr( rExpr );

    if ( ! rExpr -> u.bVal ) lExpr.skipEval = true;
    
    while ( tok -> tokId( ) == TOK_LAND ) {

        tok -> nextToken( );
        parseNotExpr( &lExpr );
                    
        *rExpr = lExpr;
    }

}

//----------------------------------------------------------------------------------------
// "parseOrExpr" implements the logical OR operation. If the left hand side
// evaluates to "true", the right side is not evaluated but still parsed. 
//
//  <orExpr> -> <andExpr> <orOP> <andExpr>
//  <orOp>   -> "OR" | "||"
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseOrExpr( SimExpr *rExpr ) {

    SimExpr lExpr = INIT_EXPR;

    parseAndExpr( rExpr );
     if ( ! rExpr -> u.bVal ) lExpr.skipEval = true;
    
    while ( tok -> tokId( ) == TOK_LOR ) {

        tok -> nextToken( );
        parseAndExpr( &lExpr );
       
        *rExpr = lExpr;
    }
}

//----------------------------------------------------------------------------------------
// "parseExpr" is the entry point to parse the expression syntax. The overall
// hierarchy is:
//
//  <expr>
//      <orExpr>
//          <andExpr>
//              <notExpr>
//                  <relationExpr>
//                      <simpleExpr>
//                          <term>
//                              <factor>
//
// The evaluation of logical "OR", "AND" and "NOT" is implemented as a short
// circuit evaluation. That is, when for example and AND operation evaluates
// the left hand side as false, the right side is not evaluated but still parsed
// without effect.
//
//----------------------------------------------------------------------------------------
void SimExprEvaluator::parseExpr( SimExpr *rExpr ) {

    parseOrExpr( rExpr );
}

//----------------------------------------------------------------------------------------
// We often expect expressions of a certain type. Little helper functions.
//
//----------------------------------------------------------------------------------------
T64Word SimExprEvaluator::acceptNumExpr( SimErrMsgId errCode, 
                                         T64Word low, 
                                         T64Word high ) {

     SimExpr rExpr = INIT_EXPR;
     parseExpr( &rExpr );
     
     if ( rExpr.typ == TYP_NUM ) {
        
        if ( ! isInRange( rExpr.u.val, low, high )) throw( errCode );
        return ( rExpr.u.val );
     }
     else throw ( errCode );
}

bool SimExprEvaluator::acceptBoolExpr( SimErrMsgId errCode ) {

    SimExpr rExpr = INIT_EXPR;
    parseExpr( &rExpr );

    if ( rExpr.typ == TYP_BOOL ) {

        return( rExpr.u.bVal );
    }
    else throw ( errCode );
 }

char *SimExprEvaluator::acceptStringExpr( SimErrMsgId errCode ) {

    SimExpr rExpr = INIT_EXPR;
    parseExpr( &rExpr );

    if ( rExpr.typ == TYP_STR ) {

        return( rExpr.u.str );
    }
    else throw ( errCode );
 }
