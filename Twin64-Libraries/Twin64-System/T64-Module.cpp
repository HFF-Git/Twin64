//----------------------------------------------------------------------------------------
//
// Twin-64 - Module Base Class
//
//----------------------------------------------------------------------------------------
// The T64-System represent the system consisting of several modules. A module is
// an object plugged into the imaginary system bus. It has a type and a module 
// number, which is the slot in that bus. Each module has a dedicated memory
// page page in the IO HPA space. The address is easily computed from the slot 
// number. In addition, a module can have several SPA regions. This is however
// module specific and not stored at the common module level.
//
//----------------------------------------------------------------------------------------
//
//  Twin-64 - System
// Copyright (C) 2020 - 2026 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under 
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <http://www.gnu.org/licenses/>.
//
//----------------------------------------------------------------------------------------
#include "T64-System.h"

//----------------------------------------------------------------------------------------
// Object constructor.
//
//----------------------------------------------------------------------------------------
T64Module::T64Module( T64ModuleType    modType, 
                      int              modNum,
                      T64Word          spaAdr,
                      int              spaLen ) {

    this -> moduleTyp   = modType;
    this -> moduleNum   = modNum;
    this -> hpaAdr      =  T64_IO_HPA_MEM_START + ( modNum * T64_PAGE_SIZE_BYTES );
    this -> hpaLen      = T64_PAGE_SIZE_BYTES;
    this -> spaAdr      = spaAdr;
    this -> spaLen      = spaLen;
    this -> spaLimit    = spaAdr + spaLen - 1;
}

//----------------------------------------------------------------------------------------
// Object constructor.
//
//----------------------------------------------------------------------------------------
int T64Module::getModuleNum( ) {

    return ( moduleNum );
}

//----------------------------------------------------------------------------------------
// Module properties.
//
//----------------------------------------------------------------------------------------
T64ModuleType T64Module::getModuleType( ) {

    return ( moduleTyp );
}

const char *T64Module::getModuleTypeName( ) {

    switch ( moduleTyp ) {

        case MT_PROC:       return ((char *) "PROC" );
        case MT_CPU_CORE:   return ((char *) "CPU" );
        case MT_CPU_TLB:    return ((char *) "TLB"  );
        case MT_IO:         return ((char *) "IO" );
        case MT_MEM:        return ((char *) "MEM" );

        case MT_NIL:
        default:            return ((char *) "NIL" );
    }
}

T64Word T64Module::getHpaAdr( ) {

    return ( hpaAdr );
}

int T64Module::getHpaLen( ) {

    return ( hpaLen );
}

T64Word T64Module::getSpaAdr( )  {

    return ( spaAdr );
}

int T64Module::getSpaLen( )  {

    return ( spaLen );
}
