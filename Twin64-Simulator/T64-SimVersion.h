//----------------------------------------------------------------------------------------
//
//  Twin64Sim - A 64-bit CPU Simulator - Version ID
//
//----------------------------------------------------------------------------------------
// The whole purpose of  this file is to define the current version String.
// We also set a constant to use for APPle vs. Windows specific coding. We use
// it in the command handler. It is not designed for compiling different code
// sequence, but rather make logical decisions on some output specifics, such
// as carriage return handling.
//
//----------------------------------------------------------------------------------------
//
// Twin64Sim - A 64-bit CPU Simulator - Version ID
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
#pragma once

const char SIM_VERSION[ ] = "A.00.01";
const char SIM_GIT_BRANCH[ ] = "main";
const int  SIM_PATCH_LEVEL = 29;

#if __APPLE__
const bool SIM_IS_APPLE = true;
#else
const bool SIM_IS_APPLE = false;
#endif
