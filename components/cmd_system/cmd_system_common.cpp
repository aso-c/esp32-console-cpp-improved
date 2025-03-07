/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <iostream>
#include <iomanip>
// #include <fstream>
//#include <ext/stdio_filebuf.h>


//#include <thread>
#include <utility>
#include <functional>
#include <tuple>


#include <cstdlib>
#include <cctype>
#include <cinttypes>
#include <unistd.h>


#include <esp_log.h>
#include <esp_console.h>
#include <esp_chip_info.h>
#include <esp_sleep.h>
#include <esp_flash.h>
#include <driver/rtc_io.h>
#include <driver/uart.h>
#include <argtable3/argtable3.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//#include "gpio_cxx.hpp"

#include "cmd_system.h"
#include "sdkconfig.h"

#include <argtable>
#include <console>
#include <extrstream>



//using namespace idf;
using namespace std;


#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define WITH_TASKS_INFO 1
#endif

static const char *TAG = "cmd_system_common";


#define __PRETTY_CLASSES__
//#define __WITH_MY_FORMAT__


// Print bytes count in groups by 3 digits
ostream& pretty_bytes(ostream& out, uint32_t value);

// Print bytes count in Kb, Mb as needed
ostream& prn_KMbytes(ostream& out, uint32_t value);


// i/o manipulator for calling pretty_bytes
// w/partial application of procedure pretty_bytesed(ostream, value),
// parameter 'value'
ostream& (*prettynumber(uint32_t val))(ostream&);

// Partial application of prn_KMbytes: fixing value
ostream& (*prn_KMbytes(uint32_t val))(ostream&);
//auto prn_KMbytes(uint32_t val) -> streamer*;





//static void register_log_level(void);



/** 'version' command */
namespace version
{
    static esp_err_t invoke(int argc, char* argv[]);
    const esp::console::cmd cmd("version", invoke, "Get version of chip and SDK");
}; /* namespace version */

/** 'restart' command */
namespace restart
{
    [[noreturn]]
    static esp_err_t invoke(int argc, char* argv[]);
    const esp::console::cmd cmd ("restart", invoke, "Software reset of the chip");
}; /* namespace restart */

/** 'free' command prints available heap memory */
namespace freemem
{
    /// Execute command procedure without the syntax object (zero syntax)
    static esp_err_t invoke(int argc, char* argv[]);
    const esp::console::cmd cmd("free", invoke, "Get the current size of free heap memory");
}; /* namespace freemem */

/** 'heap' command prints minumum heap size */
namespace heap
{
    /// Execute command procedure without the syntax object (zero syntax)
    static esp_err_t invoke(int argc, char* argv[]);
    const esp::console::cmd cmd("heap", invoke, "Get minimum size of free heap memory that was available during program execution");
}; /* namespace heap */

#if WITH_TASKS_INFO
/** 'tasks' command prints the list of tasks and related information */
namespace tasks
{
    /// Execute command procedure without the syntax object (zero syntax)
    static esp_err_t invoke(int argc, char* argv[]);
    const esp::console::cmd cmd("tasks", invoke, "Get information about running tasks");
}; /* namespace tasks */
#endif // WITH_TASKS_INFO


/** log_level command changes log level via esp_log_level_set */
namespace loglevel
{
    arg::table::syntax syntax = { 2,
		arg_str1(NULL, NULL, "<tag|*>", "Log tag to set the level for, or * to set for all tags"),
		arg_str1(NULL, NULL, "<none|error|warn|debug|verbose>", "Log level to set. Abbreviated words are accepted."),
    }; /* loglevel::syntax */
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct loglevel::act */
    const esp::console::cmd_t<act> cmd("log_level", "Set log level for all tags or a specific tag.");

}; /* namespace loglevel */


void register_system_common(void)
{
    freemem::cmd.enreg_check();
    heap::cmd.enreg_check();
    version::cmd.enreg_check();
    restart::cmd.enreg_check();
#if WITH_TASKS_INFO
    tasks::cmd.enreg_check();
#endif
    loglevel::cmd.enreg_check();
}; /* register_system_common() */




/** 'version' command */
static esp_err_t version::invoke(int argc, char* argv[])
{
    const char *model;
    esp_chip_info_t info;
    uint32_t flash_size;
    esp_chip_info(&info);

    switch(info.model)
    {
        case CHIP_ESP32:
            model = "ESP32";
            break;
        case CHIP_ESP32S2:
            model = "ESP32-S2";
            break;
        case CHIP_ESP32S3:
            model = "ESP32-S3";
            break;
        case CHIP_ESP32C3:
            model = "ESP32-C3";
            break;
        case CHIP_ESP32H2:
            model = "ESP32-H2";
            break;
        case CHIP_ESP32C2:
            model = "ESP32-C2";
            break;
        case CHIP_ESP32P4:
            model = "ESP32-P4";
            break;
        case CHIP_ESP32C5:
            model = "ESP32-C5";
            break;
        default:
            model = "Unknown";
            break;
    }; /* switch info.model */

    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        cout << "Get flash size failed" << endl;
        return 1;
    }; /* if esp_flash_get_size(NULL, &flash_size) != ESP_OK */

    cout << aso::format("ESP Console Example, Version: %s-%s of %s,")
	  % CONFIG_APP_PROJECT_VER
	  % CONFIG_APP_PROJECT_FLAVOUR
	  % CONFIG_APP_PROJECT_DATE /*<< std::endl;*/
	  << aso::format(" modified by %s") % CONFIG_APP_PROJECT_AUTHOR << std::endl;
//    cout << aso::format("\t\t\t\t\t      modified by %s") % CONFIG_APP_PROJECT_AUTHOR << std::endl;
    cout << "IDF Version\t" << esp_get_idf_version() << endl;
    cout << "Build w/C++\t" << __cplusplus << endl;
    cout << "Chip info: " << endl;
//    cout << "\tmodel: " << (info.model == CHIP_ESP32 ? "ESP32" : "Unknown") << endl;
    cout << "\tmodel: " << model << endl;
    cout << "\tcores: " << (int)info.cores << endl;
    cout << aso::format("\tfeature:%s%s%s%s%d%s")
         % (info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "")
         % (info.features & CHIP_FEATURE_BLE ? "/BLE" : "")
         % (info.features & CHIP_FEATURE_BT ? "/BT" : "")
         % (info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:")
//         % (spi_flash_get_chip_size() / (1024 * 1024)) % " MB" << std::endl;
         % (flash_size / (1024 * 1024)) % " MB" << std::endl;
    cout << "\trevision number: " << (int)info.revision << endl;

    return 0;
}; /* version::invoke() */



/** 'restart' command restarts the program */
[[noreturn]]
static esp_err_t restart::invoke(int argc, char* argv[])
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
}; /* restart::invoke() */



/** 'free' command prints available heap memory */
static esp_err_t freemem::invoke(int argc, char* argv[])
{
    cout << "free memory size: " << prn_KMbytes(esp_get_free_heap_size());
//    cout << " (" << prettybytes(esp_get_free_heap_size()) << " bytes)" << endl;
    cout << " (" << prettynumber(esp_get_free_heap_size()) << " bytes)" << endl;
    return ESP_OK;
}; /* freemem::invoke() */


/* 'heap' command prints minumum heap size */
esp_err_t static heap::invoke(int argc, char* argv[])
{
    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    cout << "min heap size: " << prn_KMbytes(heap_size);
    cout << " (" << prettynumber(heap_size) << " bytes)" << endl;
    return 0;
}; /* heap::invoke() */



#if WITH_TASKS_INFO

/** 'tasks' command prints the list of tasks and related information */
esp_err_t static tasks::invoke(int argc, char* argv[])
{
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = (char*)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        ESP_LOGE(TAG, "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    //fputs("\tAffinity", stdout);
    cout << "\tAffinity";
#endif
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}; /* tasks::invoke() */

#endif // WITH_TASKS_INFO





static const std::array<const char*, 7> log_level_names = {
	    "none",
	    "error",
	    "warn",
	    "info",
	    "debug",
	    "verbose",
//	    "out_of_range"
	}; /* log_level_names */


//static int log_level(int argc, char **argv)
esp_err_t loglevel::act::invoke(int argc, char* argv[])
{
	constexpr int tag_idx = 0;
	constexpr int level_idx = 1;

    syntax.parse(argc, argv);
    if (syntax.err())
    {
       syntax.errors(stderr, argv[0]);
       return syntax.err();
    }; /* if syntax.err() */
    assert(std::get<arg::table::str>(syntax.description[tag_idx])->count == 1);
    assert(std::get<arg::table::str>(syntax.description[level_idx])->count == 1);
    const string_view tag = std::get<arg::table::str>(syntax.description[tag_idx])->sval[0];
    const string_view level_str = std::get<arg::table::str>(syntax.description[level_idx])->sval[0];
    auto level_ptgt = std::find(log_level_names.begin(), log_level_names.end(), level_str);
    if (level_ptgt == log_level_names.end()) {
        ESP_LOGE("log_level command", "Invalid log level '%s', choose from none|error|warn|info|debug|verbose\n", level_str.data());
        return 1;
    }; /* if level_ptgt == log_level_names.end() */

    esp_log_level_t level = static_cast<esp_log_level_t>(std::distance(log_level_names.begin(), level_ptgt));
    if (level > CONFIG_LOG_MAXIMUM_LEVEL) {
        ESP_LOGE("log_level command", "Can't set log level to %s, max level limited in menuconfig to %s. "
               "Please increase CONFIG_LOG_MAXIMUM_LEVEL in menuconfig.\n",
	       log_level_names[level], log_level_names[CONFIG_LOG_MAXIMUM_LEVEL]);
        return 1;
    }
    esp_log_level_set(tag.data(), level);
    return ESP_OK;
}; /* loglevel::act::invoke() */


/*
 * @brief Get string with version information of project current state
 * @return string containing the current version of project.
 */
const char* version_str(void)
{
    return "Version " CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR
	    " of " CONFIG_APP_PROJECT_DATE ","
	    " modified by " CONFIG_APP_PROJECT_AUTHOR ".";
}; /* get_version */



// Procedures for output memory size/numbers
// in pretty format: group digits by 3 cifer,
// and divide fractional part from integer

//#define DIGDELIM '.'
#define DIGDELIM '_'
#define FRACTDELIM ','
#define Knum 1024
//#ifdef __EXPRESSION_OUTPUT__
//typedef ostream& (streamer)(ostream&);
//#endif


// Template for the iostream manipulator
// that implemented partial application of the function callfunc
// with fixing ParamType parameter
template <typename ParamType, ostream& (*callfunc)(ostream&, ParamType)>
ostream& (*paramanip(ParamType value))(ostream&)
{
	static ParamType val = 0;
	struct PartApp { static ostream& exec(ostream& out) {return callfunc(out, val);}; };

    val = value;
    return PartApp::exec;
}; /* paramanip*/


// Call the procedure 'pretty_bytes'
// in iostream manipulator environment
// partial application
ostream& (*prettynumber(uint32_t value))(ostream&)
{
    return paramanip<uint32_t, pretty_bytes>(value);
}; /* prettynumber */


// Partial application of prn_KMbytes: fixing value
ostream& (*prn_KMbytes(uint32_t value))(ostream&)
{
    return paramanip<uint32_t, prn_KMbytes>(value);
}; /* prn_KMbytes(uint32_t value) */


ostream& pretty_size_prn(ostream& out, const char prompt[], uint32_t value);

ostream& (*pretty_size_prn(const char prompt[], uint32_t value))(ostream&)
{
	static const char *outprompt = NULL;
	static uint32_t outvalue = 0;
	struct PartDef { static ostream& exec(ostream& out) {return pretty_size_prn(out, outprompt, outvalue);}; };

    outprompt = prompt;
    outvalue = value;
    return PartDef::exec;
};


/* Print int value with pretty fprmat & in bytes/megabytes etc. */
ostream& pretty_size_prn(ostream& out, const char prompt[], uint32_t value)
{
        out << prompt << ": " << prn_KMbytes(value);
        out << " (" << prettynumber(value) << " bytes)";
    return out;
}; /* pretty_size_prn */

// Print bytes count in groups by 3 digits
ostream& pretty_bytes(ostream& out, uint32_t value)
{
	uint32_t head = value / 1000;

    if (head > 0)
    {
	out << prettynumber(head) << DIGDELIM << setw(3) << setfill('0') << value % 1000;
    }
    else
	out << value;
    return out;
}; /* pretty_bytes */



// Print the units of numerical value
ostream& prn_KMbytes(ostream& out, uint32_t value)
{
    if (value < 10 * Knum)
    {
	// Printout of bytes
	out << prettynumber(value) << " bytes";
    } /* if size < 10 * Knum */
    else if (value < Knum * Knum)
    {
	// Printout of Kbytes
	out << value / Knum << " Kbytes";
	;
    } /* else if size < Knum^2 */
    else if (value < Knum * Knum * Knum)
    {
	// Printout of Mbytes
	out << value / Knum / Knum << " Mbytes";
    } /* else if size < Knum^3 */
    else
    {
	// all other
	out << prettynumber(value) << " bytes";
    }; /* else */
    return out;

}; /* prn_KMbytes */
