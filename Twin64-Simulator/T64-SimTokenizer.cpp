//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command line tokenizer
//
//----------------------------------------------------------------------------------------
// The tokenizer will accept an input line and return one token at a time. 
// Upon an error, the tokenizer will raise an exception.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Command line tokenizer
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
// Local namespace. These routines are not visible outside this source file.
//
//----------------------------------------------------------------------------------------
namespace {

//----------------------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------------------
const int   TOK_NAME_SIZE   = 32;
const char  EOS_CHAR        = 0;

char        strTokenBuf[ MAX_TOK_STR_SIZE ] = { 0 };

//----------------------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------------------
void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) {
            
            str[ i ] = (char) toupper((int) str[ i ] );
        }
    }
}

//----------------------------------------------------------------------------------------
// The lookup function. We just do a linear search for now.
//
//----------------------------------------------------------------------------------------
int lookupToken( char *inputStr, SimToken *tokTab ) {

    char tempStr[ TOK_NAME_SIZE + 1 ] = { 0 };
    strncpy( tempStr, inputStr, TOK_NAME_SIZE );
    upshiftStr( tempStr );

    if (( strlen( tempStr ) == 0 ) || 
        ( strlen ( tempStr ) > TOK_NAME_SIZE )) return( -1 );
    
    for ( int i = 0; i < MAX_CMD_TOKEN_TAB; i++  ) {
        
        if ( strcmp( tempStr, tokTab[ i ].name ) == 0 ) return( i );
    }
    
    return( -1 );
}

//----------------------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------------------
void addChar( char *buf, int size, char ch ) {
    
    int len = (int) strlen( buf );
    
    if ( len + 1 < size ) {
        
        buf[ len ]     = ch;
        buf[ len + 1 ] = 0;
    }
}

}; // namespace

//----------------------------------------------------------------------------------------
// The object constructor, nothing to do for now.
//
//----------------------------------------------------------------------------------------
SimTokenizer::SimTokenizer( ) { }

//----------------------------------------------------------------------------------------
// helper functions for the current token.
//
//----------------------------------------------------------------------------------------
bool SimTokenizer::isToken( SimTokId tokId ) { 
    
    return( currentToken.tid == tokId ); 
}

bool SimTokenizer::isTokenTyp( SimTokTypeId typId )  { 
    
    return( currentToken.typ == typId ); 
}

bool SimTokenizer::isTokenIdent( char *name ) { 
    
    return( currentToken.typ == TYP_IDENT && 
            strcmp( currentToken.name, name ) == 0 );
}

SimToken SimTokenizer::token( ) { 
    
    return( currentToken );    
}

SimTokTypeId SimTokenizer::tokTyp( ) { 
    
    return( currentToken.typ ); 
}

SimTokId SimTokenizer::tokId( ) { 
    
    return( currentToken.tid ); 
}

T64Word SimTokenizer::tokVal( ) { 
    
    return( currentToken.u.val ); 
}

char *SimTokenizer::tokName( ) {

    return( currentToken.name );
}

char *SimTokenizer::tokStr( ) { 
    
    return( currentToken.u.str );
}

//----------------------------------------------------------------------------------------
// "parseNum" will parse a number. We accept decimals, hexadecimals and binary 
// numbers. The numeric string can also contain "_" characters for readability.
// Hex numbers start with "0x", binary with "0b", decimals just with numeric digits.
//----------------------------------------------------------------------------------------
void SimTokenizer::parseNum( ) {
    
    currentToken.tid    = TOK_NUM;
    currentToken.typ    = TYP_NUM;
    currentToken.u.val  = 0;
    
    int     base        = 10;
    int     maxDigits   = 22;
    int     digits      = 0;
    T64Word tmpVal      = 0;
    
    if ( currentChar == '0' ) {
        
        nextChar( );
        if (( currentChar == 'X' ) || ( currentChar == 'x' )) {
            
            base        = 16;
            maxDigits   = 16;
            nextChar( );
        }
        else if (( currentChar == 'B' ) || ( currentChar == 'b' )) {
            
            base        = 2;
            maxDigits   = 64;
            nextChar( );
        }
        else if ( !isdigit( currentChar )) {

            return;
        }
    }
    
    do {
        
        if ( currentChar == '_' ) {
            
            nextChar( );
            continue;
        }
        else {

            if ( isdigit( currentChar )) {

                int digit = currentChar - '0';
                if ( digit >= base ) throw ( ERR_INVALID_NUM );

                tmpVal = ( tmpVal * base ) + digit;
            }
            else if (( base == 16         ) && 
                     ( currentChar >= 'a' ) && 
                     ( currentChar <= 'f' )) {

                tmpVal = ( tmpVal * base ) + currentChar - 'a' + 10;            
            }
            else if (( base == 16         ) && 
                     ( currentChar >= 'A' ) && 
                     ( currentChar <= 'F' )) {

                tmpVal = ( tmpVal * base ) + currentChar - 'A' + 10;
            }      
            else throw ( ERR_INVALID_NUM );
            
            nextChar( );
            digits ++;
            
            if ( digits > maxDigits ) throw ( ERR_INVALID_NUM );
        }
    }
    while (
        ( currentChar == '_' ) ||
        ( isdigit( currentChar )) ||
        ( base == 16 &&
          (( currentChar >= 'a' && currentChar <= 'f' ) ||
           ( currentChar >= 'A' && currentChar <= 'F' ) ) 
        )
    );

    currentToken.u.val = tmpVal;
}

//----------------------------------------------------------------------------------------
// "parseString" gets a string. We manage special characters inside the string 
// with the "\" prefix. Right now, we do not use strings, so the function is 
// perhaps for the future. We will just parse it, but record no result. One day,
// the entire simulator might use the string functions. Then we need it.
//
//----------------------------------------------------------------------------------------
void SimTokenizer::parseString() {

    currentToken.tid   = TOK_STR;
    currentToken.typ   = TYP_STR;
    currentToken.u.str = nullptr;

    strTokenBuf[0] = '\0';

    do {
       
        nextChar();

        while ((currentChar != EOS_CHAR) && (currentChar != '"')) {

            if (currentChar == '\\') {

                nextChar();

                if (currentChar == EOS_CHAR)
                    throw(ERR_EXPECTED_CLOSING_QUOTE);

                switch (currentChar) {

                    case 'n':  strcat( strTokenBuf, "\n"); break;
                    case 't':  strcat( strTokenBuf, "\t"); break;
                    case '\\': strcat( strTokenBuf, "\\"); break;
                    case '"':  strcat( strTokenBuf, "\""); break;
                    
                    default:   addChar( strTokenBuf, sizeof(strTokenBuf), currentChar);
                } 
            }
            else addChar( strTokenBuf, sizeof(strTokenBuf), currentChar );

            nextChar();
        }

        if ( currentChar != '"' ) throw( ERR_EXPECTED_CLOSING_QUOTE );

        nextChar( );
        while ( isspace( currentChar )) nextChar( );

    } while ( currentChar == '"' );

    currentToken.u.str = strTokenBuf;
}

//----------------------------------------------------------------------------------------
// "parseIdent" parses an identifier. It is a sequence of characters starting 
// with an alpha character. An identifier found in the token table will assume 
// the type and value of the token found. Any other identifier is just an 
// identifier symbol. There is one more thing. There are qualified constants 
// that begin with a character followed by a percent character, followed by a 
// numeric value. During the character analysis, We first check for these kind
// of qualifiers and if found hand over to parse a number.
//
//----------------------------------------------------------------------------------------
void SimTokenizer::parseIdent( ) {
    
    currentToken.tid    = TOK_IDENT;
    currentToken.typ    = TYP_IDENT;

    char identBuf[ MAX_TOK_NAME_SIZE ] = "";
    
    if (( currentChar == 'L' ) || ( currentChar == 'l' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.u.val &= 0x00000000FFFFFC00;
                currentToken.u.val >>= 10;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    else if (( currentChar == 'R' ) || ( currentChar == 'r' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.u.val &= 0x00000000000003FF;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    else if (( currentChar == 'S' ) || ( currentChar == 's' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.u.val &= 0x000FFFFF00000000;
                currentToken.u.val >>= 32;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    else if (( currentChar == 'U' ) || ( currentChar == 'u' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.u.val &= 0xFFF0000000000000;
                currentToken.u.val >>= 52;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    
    while (( isalnum( currentChar )) || ( currentChar == '_' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
    }
    
    int index = lookupToken( identBuf, tokTab );
    
    if ( index == - 1 ) {
        
        strncpy( currentToken.name, identBuf, MAX_TOK_NAME_SIZE );
        currentToken.typ    = TYP_IDENT;
        currentToken.tid    = TOK_IDENT;
    }
    else currentToken = tokTab[ index ];
}

//----------------------------------------------------------------------------------------
// "nextToken" is the entry point to the token business. It returns the next token from
// the input string.
//
//----------------------------------------------------------------------------------------
void SimTokenizer::nextToken( ) {

    currentToken.typ    = TYP_NIL;
    currentToken.tid    = TOK_NIL;
    currentToken.u.val  = 0;
    
    while (( currentChar == ' '  ) || ( currentChar == '\n' )) nextChar( );
    
    if ( isalpha( currentChar )) {
        
       parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
       parseNum( );
    }
    else if ( currentChar == '"' ) {
        
        parseString( );
    }
    else if ( currentChar == '.' ) {
        
        currentToken.typ   = TYP_SYM;
        currentToken.tid   = TOK_PERIOD;
        nextChar( );
    }
    else if ( currentChar == ':' ) {
        
        currentToken.typ   = TYP_SYM;
        currentToken.tid   = TOK_COLON;
        nextChar( );
    }
    else if ( currentChar == '=' ) {
        
        currentToken.typ   = TYP_SYM;
        currentToken.tid   = TOK_EQUAL;
        nextChar( );
    }
    else if ( currentChar == '+' ) {
        
        currentToken.tid = TOK_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MOD;
        nextChar( );
    }
    else if ( currentChar == '&' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_AND;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_COMMA;
        nextChar( );
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentToken.typ    = TYP_NIL;
        currentToken.tid    = TOK_EOS;
    }
    else {
    
        currentToken.tid = TOK_ERR;
        printf( "invalid ch: %d\n", currentChar );
        throw ( ERR_INVALID_CHAR_IN_IDENT );
    }
}

//----------------------------------------------------------------------------------------
// Helper functions.
//
//----------------------------------------------------------------------------------------
void SimTokenizer::checkEOS( ) {
    
    if ( ! isToken( TOK_EOS )) throw ( ERR_TOO_MANY_ARGS_CMD_LINE );
}

void SimTokenizer::acceptComma( ) {
    
    if ( isToken( TOK_COMMA )) nextToken( );
    else throw ( ERR_EXPECTED_COMMA );
}

void SimTokenizer::acceptColon( ) {
    
    if ( isToken( TOK_COLON )) nextToken( );
    else throw ( ERR_EXPECTED_COLON );
}

void SimTokenizer::acceptEqual( ) {
    
    if ( isToken( TOK_EQUAL )) nextToken( );
    else throw ( ERR_EXPECTED_COLON );
}

void SimTokenizer::acceptLparen( ) {
    
    if ( isToken( TOK_LPAREN )) nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
}

void SimTokenizer::acceptRparen( ) {
    
    if ( isToken( TOK_RPAREN )) nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
}

SimTokId SimTokenizer::acceptTokSym( SimErrMsgId errId ) {

    if ( isTokenTyp( TYP_SYM )) {
        
        SimTokId tmp = tokId( );
        nextToken( );
        return( tmp );
    }
    else throw ( errId );
}


//****************************************************************************************
//****************************************************************************************
//
// Methods for SimTokenizerFromString class.   
//
//----------------------------------------------------------------------------------------
SimTokenizerFromString::SimTokenizerFromString(  ) : SimTokenizer( ) { }

//----------------------------------------------------------------------------------------
// We initialize a couple of globals that represent the current state of the 
// parsing process. This call is the first before any other method can be 
// called.
//
//----------------------------------------------------------------------------------------
void SimTokenizerFromString::setupTokenizer( char *lineBuf, SimToken *tokTab ) {

    strncpy( tokenLine, lineBuf, strlen( lineBuf ) + 1 );
    
    this -> tokTab                  = tokTab;
    this -> currentLineLen          = (int) strlen( tokenLine );
    this -> currentCharIndex        = 0;
    this -> currentChar             = ' ';
}

//----------------------------------------------------------------------------------------
// "nextChar" returns the next character from the token line string.
//
//----------------------------------------------------------------------------------------
void SimTokenizerFromString::nextChar( ) {

    if ( currentCharIndex < currentLineLen ) {
        
        currentChar = tokenLine[ currentCharIndex ];
        currentCharIndex ++;
    }
    else currentChar = EOS_CHAR;
}

//****************************************************************************************
//****************************************************************************************
//
// Methods for SimTokenizerFromFile class.   
//
//----------------------------------------------------------------------------------------
SimTokenizerFromFile::SimTokenizerFromFile(  ) : SimTokenizer( ) { }

SimTokenizerFromFile::~SimTokenizerFromFile( ) {

    closeFile( );
}

//----------------------------------------------------------------------------------------
// We initialize a couple of globals that represent the current state of the 
// parsing process. This call is the first before any other method can be 
// called.
//
 
void SimTokenizerFromFile::setupTokenizer( char *filePath, SimToken *tokTab ) {

    this -> tokTab          = tokTab;
    this -> currentChar     = ' ';

    openFile( filePath );
}

//----------------------------------------------------------------------------------------
// "openFile" opens the source file for reading.
//
//----------------------------------------------------------------------------------------
void SimTokenizerFromFile::openFile( char *filePath ) {
    
    srcFile = fopen( filePath, "r" );
    
    if ( srcFile == nullptr ) throw ( ERR_FILE_NOT_FOUND );
}

//----------------------------------------------------------------------------------------
// "closeFile" closes the source file.
//
//----------------------------------------------------------------------------------------
void SimTokenizerFromFile::closeFile( ) {

    if ( srcFile != nullptr ) {
        
        fclose( srcFile );
        srcFile = nullptr;
    }
}   

 
//----------------------------------------------------------------------------------------
// "nextChar" returns the next character from the source file. We also maintain a 
// character and line index.
//
//----------------------------------------------------------------------------------------
void SimTokenizerFromFile::nextChar( ) {

    if ( srcFile != nullptr ) {
        
        int ch = fgetc( srcFile );
      
        if ( ch == EOF ) {

            currentChar = EOS_CHAR;
        }
        else if ( ch == '\n' ) {
            
            currentLineIndex ++;
            currentCharIndex = 0;
            currentChar      = ' ';
        }
        else {
            
            currentCharIndex ++;
            currentChar = (char) ch;
        }
    }
    else currentChar = EOS_CHAR;
}   