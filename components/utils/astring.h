/*
 * aso own string utils
 * Include file
 * 	File: astring
 *	Author:  aso (Solomatov A.A.)
 *	Created: 17.11.2022
 *	Date:	 17.11.2022
 *	Version: 0.9
 */


#ifndef __ASTRING__
#define __ASTRING__


// is string 'str' is empty or NULL?
inline bool empty(const char* const str)
{
    return str == NULL || str[0] == '\0';
}; /*  empty */

#endif	// __ASTRING__

//--[ astring.h ]----------------------------------------------------------------------------------
