//
// iostream extentions for advanced formatting &
// additional compatibility with C stdio layer.
//
// extrstream.cpp
// Implementation file
//
// author: Solomatov A.A. (aso)
// ver.  : v.1.1
// date  : 05.07.22.
//


// Исходники:
// https://overcoder.net/q/54881/получение-файла-из-std-fstream
// https://overcoder.net/q/54881/%D0%BF%D0%BE%D0%BB%D1%83%D1%87%D0%B5%D0%BD%D0%B8%D0%B5-%D1%84%D0%B0%D0%B9%D0%BB%D0%B0-%D0%B8%D0%B7-std-fstream
// https://ask-dev.ru/info/113124/getting-a-file-from-a-stdfstream
//
// ОРИГИНАЛ:
// (Возможно, не перекрестная платформа, но простая)
//
// Упрощение взлома http://www.ginac.de/~kreckel/fileno/ (ответ dvorak) и просмотр этого расширения
// gcc http://gcc.gnu.org/onlinedocs/gcc-4.6.2/libstdc++/api/a00069.html#a59f78806603c619eafcd4537c920f859,
// У меня есть это решение, которое работает на GCC (не менее 4.8) и clang (по крайней мере, 3.3)
// [...]
//
//Usage:
//
//int main(){
//    std::ofstream ofs("file.txt");
//    fprintf(cfile(ofs), "sample1");
//    fflush(cfile(ofs)); // ofs << std::flush; doesn't help
//    ofs << "sample2\n";
//}



#include <stdarg.h>
#include <functional>
#include <iostream>
//#include <iomanip>
#include <fstream>
#include <sstream>
#include <ext/stdio_filebuf.h>

#include <stdio.h>

#include "include/extrstream"



using namespace std;

namespace aso
{


typedef std::basic_ofstream<char>::__filebuf_type  buffer_t;

typedef __gnu_cxx::stdio_filebuf<char>             io_buffer_t;


//
// Implementation generationg of the C-file, assiciated with passed C++ I/O fstream
//
FILE* cfile_impl(buffer_t* const fb)
{
    return (static_cast<io_buffer_t* /*const*/>(fb))->file(); //type std::__c_file
}; /* cfile_impl */



//
// Get C File, tied with passed ofstream
//
FILE* cfile(std::ofstream const& ofs)
{
    return cfile_impl(ofs.rdbuf());
}; /* cfile(std::ofstream const&) */


//
// Get C File, tied with passed ofstream
//
FILE* cfile(std::ifstream const& ifs)
{
    return cfile_impl(ifs.rdbuf());
}; /* cfile(std::ifstream) */



//
// Text from this source:
// https://overcoder.net/q/54881/получение-файла-из-std-fstream
// https://overcoder.net/q/54881/%D0%BF%D0%BE%D0%BB%D1%83%D1%87%D0%B5%D0%BD%D0%B8%D0%B5-%D1%84%D0%B0%D0%B9%D0%BB%D0%B0-%D0%B8%D0%B7-std-fstream
// https://ask-dev.ru/info/113124/getting-a-file-from-a-stdfstream
//
// Limitations: (комментарии приветствуются)
//
//    Я считаю, что после fprintf печатать до std::ofstream важно fflush, иначе "sample2" появится перед "sample1" в приведенном выше примере. Я не знаю, есть ли лучшее решение для этого, чем использование fflush. В частности ofs << flush не помогает.
//
//    Невозможно извлечь файл * из std::stringstream, я даже не знаю, возможно ли это. (см. ниже для обновления).
//
//    Я до сих пор не знаю, как извлечь C stderr из std::cerr и т.д., например, для использования в fprintf(stderr, "sample"), в гипотетическом коде, подобном этому fprintf(cfile(std::cerr), "sample").
//
// Что касается последнего ограничения, единственным обходным решением, которое я нашел,
// является добавление этих перегрузок:
//

//
// Overloads for cout, cerr & clog iostreams
//
FILE* cfile(std::ostream const& os)
{
    if(std::ofstream const* ofsP = dynamic_cast<std::ofstream const*>(&os)) return cfile(*ofsP);
    if(&os == &std::cerr) return stderr;
    if(&os == &std::cout) return stdout;
    if(&os == &std::clog) return stderr;
//    if(dynamic_cast<std::ostringstream const*>(&os) != 0){
       throw std::runtime_error("don't know cannot extract FILE pointer from std::ostringstream");
//    }
//    return 0; // stream not recognized
    return nullptr; // stream not recognized
}; /* cfile(std::ostream) */

//
// Overloads for cin iostreams
//
FILE* cfile(std::istream const& is)
{
    if(std::ifstream const* ifsP = dynamic_cast<std::ifstream const*>(&is)) return cfile(*ifsP);
    if(&is == &std::cin) return stdin;
//    if(dynamic_cast<std::ostringstream const*>(&is) != 0){
        throw std::runtime_error("don't know how to extract FILE pointer from std::istringstream");
//    }
//    return 0; // stream not recognized
    return nullptr; // stream not recognized
}; /* cfile(std::istream) */


//--[ class Formatter ]----------------------------------------------------------------------------


// helpers for aso::Formatter::output
// additional procedures can be defined by needed


// Format string output helper for std::ostream
std::ostream& output_helper(std::ostream& ostrm, const std::string& str ...)
{
	va_list vargs;

    va_start(vargs, str);
    vfprintf(cfile(ostrm), str.c_str(), vargs);
    fflush(cfile(ostrm)); // ofs << std::flush; doesn't help
    va_end(vargs);
    return ostrm;
}; /* std::ostream& output_helper(std::ostream&, const std::string& ...) */


// Format string output helper for std::ofstream
std::ofstream& output_helper(std::ofstream& ofs, const std::string& str...)
{
	va_list vargs;

    va_start(vargs, str);
    vfprintf(cfile(ofs), str.c_str(), vargs);
    fflush(cfile(ofs)); // ofs << std::flush; doesn't help
    va_end(vargs);
    return ofs;
}; /* std::ofstream& output_helper(std::ofstream&, const std::string&...) */

// Format string output helper for std::ostringstream
std::ostringstream& output_helper(std::ostringstream& ostr, const std::string& formatstr...)
{
	va_list vargs, vargs_pre;
	char* outbuf = nullptr;

    va_start(vargs, formatstr);
    va_copy(vargs_pre, vargs);
    outbuf = (char*)malloc(vsnprintf(nullptr, 0, formatstr.c_str(), vargs_pre) * sizeof(char) / sizeof(std::size_t) + 1);
    vsprintf(outbuf, formatstr.c_str(), vargs);
    ostr << outbuf;
    free(outbuf);
    va_end(vargs_pre);
    va_end(vargs);

    return ostr;
}; /* std::ostringstream& output_helper(std::ostringstream& ofs, const std::string&...) */

// Format string output helper for std::stringstream
std::stringstream& output_helper(std::stringstream& strstr, const std::string& formatstr...)
{
	va_list vargs, vargs_pre;
	char* outbuf = nullptr;

    va_start(vargs, formatstr);
    va_copy(vargs_pre, vargs);
    outbuf = (char*)malloc(vsnprintf(nullptr, 0, formatstr.c_str(), vargs_pre) * sizeof(char) / sizeof(std::size_t) + 1);
    vsprintf(outbuf, formatstr.c_str(), vargs);
    strstr << outbuf;
    free(outbuf);
    va_end(vargs_pre);
    va_end(vargs);

    return strstr;
}; /* std::stringstream& output_helper(std::stringstream& ofs, const std::string&...) */


//--[ end of class Formatter ]---------------------------------------------------------------------



}; /* namespace aso */



//--[ End of file ]--------------------------------------------------------------------------------
