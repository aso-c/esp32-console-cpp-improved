/**
 * @file astring.cpp
 * @brief aso own string utils,
 * 	C++ body file
 *
 * @date Created on: 6 янв. 2024 г.
 *
 * @author:  aso (Solomatov A.A.)
 *
 * @version: 0.1
 */

#include <cstring>
#include <string>

#include <esp_log.h>

#include "astring.h"

/// @brief matching confirmation string value
bool astr::confirm(const std::string& bst)
{
	std::string buf = tolower(bst);

    return ((!is_zero(buf) && is_digitex(buf)) || buf == "on" || buf == "ok" || buf == "y" || buf == "yes");
}; /* astr::yes_str */

/// @brief match negation string value
bool astr::decline(const std::string& bst)
{
	std::string buf = tolower(bst);

    return (is_space(buf) || is_zero(buf) || buf == "n" || buf == "no" || buf == "cancel");
}; /* astr::no_str */


/// @brief match the confirmation string
/// pure C version
bool yes_str(const char str[])
{
    return astr::confirm(str);
}; /* yes_str() */

/// @brief match negation string
/// pure C version
bool no_str(const char str[])
{
    return astr::decline(str);
}; /* no_str() */

/// String manipulation utility ---------------------------

/// string to lower case
std::string astr::tolower(const std::string& str)
{
	std::string buf = str;

    for (char& c: buf)
	c = ::tolower(static_cast<unsigned char>(c));
    return buf;
}; /* astr::tolower */

/// string with only space chars or empty?
bool astr::is_space(const std::string& str)
{
        bool resf = true;

	ESP_LOGW(__PRETTY_FUNCTION__, "Input string is: [%s]", str.c_str());

    if (str == "")
    {
	ESP_LOGW(__func__, "Current string is empty string");
	return true;
    }

    for (unsigned char c: str)
	if (!std::isspace(c))
	    resf = false;

    return resf;
}; /* astr::is_space */


/// string with digit & optionsl spaces and/or underline
bool astr::is_digitex(const std::string& str)
{
	bool withdigit = false;	///< detected digit in string
	bool spdigit = true; ///< detected non_digit && non_space && no underline chars

    for (unsigned char c: str)
    {
	if (std::isdigit(c))
	    withdigit = true;
	else if (c != '_' || !std::isspace(c))
	    spdigit = false;
    }; /* for unsigned char c: str */
    return (withdigit && spdigit);
}; /* astr::is_digitex */


/// string is zero only?
bool astr::is_zero(const std::string& str)
{
	bool accept = false;
	bool decline = false;

    if (!is_digitex(str))
	return false;

    for (unsigned char c: str)
	if (c == '0')
	    accept = true;
	else if (!(std::isspace(c) || c == '.' || (std::tolower(static_cast<unsigned char>(c) == 'x'))))
	    decline = true;

    return accept && !decline;

}; /* astr::is_zero */


//--[ astring.cpp ]----------------------------------------------------------------------------------------------------


