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


#ifdef __cplusplus
extern "C"
{
#endif


// is string 'str' is empty or NULL?
inline bool empty(const char* const str)
{
    return str == NULL || str[0] == '\0';
}; /*  empty */


// is the path terminated a slash
// as a directory name
inline bool is_dirname(const char path[])
{
    return !empty(path) && path[strlen(path)] == '/';
}; /* is_dirname */


#ifdef __cplusplus
}; /* extern "C" */
#endif



#endif	// __ASTRING__

//--[ astring.h ]----------------------------------------------------------------------------------
