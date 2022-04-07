/*
 * SD-card manipulations command definition
 * Implementation file
 * 	File: cmd_sdcard.h
 *	Author:  aso (Solomatov A.A.)
 *	Created: 04.04.2022
 *	Version: 0.1
 */


// prevent multyple inclusion
#pragma once

#ifndef __cplusplus
#error "File cmd_sdcard.hpp is included for C++ only"
#endif


// Register all SD-card commands
void register_sdcard_cmd(void);

//--[ cmd_sdcard.hpp ]-----------------------------------------------------------------------------
