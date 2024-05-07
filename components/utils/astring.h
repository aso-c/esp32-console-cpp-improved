/**
 * @file: astring.h
 * @brief aso own string utils
 * Include file
 *
 * @author  aso (Solomatov A.A.)
 *
 * @date Created 17.11.2022
 *	Updated  07.01.2024
 *
 * @version 0.95
 */


#ifndef __ASTRING__
#define __ASTRING__


#ifdef __cplusplus
extern "C"
{
#endif


/// @brief String 'str' is empty or NULL?
inline bool empty(const char* const str)
{
    return str == NULL || str[0] == '\0';
}; /*  empty */


/// @brief is the path terminated a slash
/// as a directory name
inline bool is_dirname(const char path[])
{
    return !empty(path) && path[strlen(path)] == '/';
}; /* is_dirname */


/// @brief match confirmation string
/// pure C version
bool yes_str(const char[]);

/// @brief match negation string
/// pure C version
bool no_str(const char[]);



#ifdef __cplusplus
}; /* extern "C" */


namespace astr
{


    /// @brief matching the confirmation string
    /// Is the string math the "Yes" string
    bool confirm(const std::string&);

    /// matching the negation string
    ///Is the string math the "No" string
    bool decline(const std::string&);

    /// @brief Return the "Yes" string
    inline const char* yes() { return "y";};

    /// @brief Return the "No" string
    inline const char* no() { return "n"; };

    ///------- String manipulation utility

    /// @brief string to lower case
    std::string tolower(const std::string&);

    /// @brief string with only space chars or empty?
    bool is_space(const std::string&);

    /// @brief string with digit & optionsl spaces and/or underline
    bool is_digitex(const std::string&);

    /// @brief string is zero only?
    bool is_zero(const std::string&);

}; /* astr */

#endif



#endif	// __ASTRING__

//--[ astring.h ]----------------------------------------------------------------------------------
