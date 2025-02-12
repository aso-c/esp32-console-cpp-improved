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


#include <cstdlib>
// #include <stdio.h>
#include <iostream>
#include <iomanip>
// #include <fstream>
//#include <ext/stdio_filebuf.h>


//#include <thread>
//#include "esp_log.h"
//#include "gpio_cxx.hpp"

//#define __WITH_STDIO__
//#define __WITH_BOOST__
//#define __MAX_UNFOLDED_OUTPUT__


#include <string.h>
#include <ctype.h>
#include <inttypes.h>
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

#include "cmd_system.h"
#include "sdkconfig.h"

#include <utility>
#include <functional>
#include <tuple>

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





static void register_free(void);
static void register_heap(void);
static void register_version(void);
static void register_restart(void);
#if WITH_TASKS_INFO
static void register_tasks(void);
#endif
static void register_log_level(void);

void register_system_common(void)
{
    register_free();
    register_heap();
    register_version();
    register_restart();
#if WITH_TASKS_INFO
    register_tasks();
#endif
    register_log_level();
}


/** 'version' command */
namespace version
{
    struct act: public arg::table::act
    {
	/// Execute command procedure without the syntax object (zero syntax)
	static
	esp_err_t invoke(int argc, char* argv[]);
    }; /* struct version::act */

    const esp::console::cmd_t<act> cmd ("version", "Get version of chip and SDK");

}; /* namespace version */

static void register_version(void)
{
#if 0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t cmd = {
        .command = "version",
        .help = "Get version of chip and SDK",
        .hint = NULL,
        .func = &get_version,
    };
#pragma GCC diagnostic pop
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
#endif

    version::cmd.enreg_check();
}; /* register_version */

/** 'version' command */
/// Execute command procedure without the syntax object (zero syntax)
esp_err_t version::act::invoke(int argc, char* argv[])
//static int get_version(int argc, char **argv)
{
    const char *model;
    esp_chip_info_t info;
    uint32_t flash_size;
    esp_chip_info(&info);

    switch(info.model) {
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
        default:
            model = "Unknown";
            break;
    }

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
}; /* get_version */ /* version::act::invoke() */



/** 'version' command */
namespace restart
{
    struct act: public arg::table::act
    {
	/// Execute command procedure without the syntax object (zero syntax)
	[[noreturn]]
	static
	esp_err_t invoke(int argc, char* argv[]);
    }; /* struct restart::act */

    const esp::console::cmd_t<act> cmd ("restart", "Software reset of the chip");

}; /* namespace restart */

static void register_restart(void)
{
#if 0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
#pragma GCC diagnostic pop
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
#endif
    restart::cmd.enreg_check();

}; /* register_restart() */

/** 'restart' command restarts the program */
[[noreturn]]
esp_err_t restart::act::invoke(int argc, char* argv[])
//static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
//    return -1;	// stub for supress warning only
}; /* restart::act::invoke() */



/** 'free' command prints available heap memory */
namespace freemem
{
    struct act: public arg::table::act
    {
	/// Execute command procedure without the syntax object (zero syntax)
	static
	esp_err_t invoke(int argc, char* argv[]);
    }; /* struct freemem::act */

    const esp::console::cmd_t<act> cmd ("free", "Get the current size of free heap memory");

}; /* namespace freemem */

static void register_free(void)
{
#if 0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &free_mem,
    };
#pragma GCC diagnostic pop
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
#endif
    freemem::cmd.enreg_check();
}; /* register_free */

/** 'free' command prints available heap memory */
//static
esp_err_t freemem::act::invoke(int argc, char* argv[])
//static int free_mem(int argc, char **argv)
{
    cout << "free memory size: " << prn_KMbytes(esp_get_free_heap_size());
//    cout << " (" << prettybytes(esp_get_free_heap_size()) << " bytes)" << endl;
    cout << " (" << prettynumber(esp_get_free_heap_size()) << " bytes)" << endl;
//    return 0;
    return ESP_OK;
}; /* free_mem */ /* freemem::act::invoke() */




/** 'heap' command prints minumum heap size */
namespace heap
{
    struct act: public arg::table::act
    {
	/// Execute command procedure without the syntax object (zero syntax)
	static
	esp_err_t invoke(int argc, char* argv[]);
    }; /* struct heap::act */

    const esp::console::cmd_t<act> cmd ("heap", "Get minimum size of free heap memory that was available during program execution");

}; /* namespace heap */

static void register_heap(void)
{
#if 0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t heap_cmd = {
        .command = "heap",
        .help = "Get minimum size of free heap memory that was available during program execution",
        .hint = NULL,
        .func = &heap_size,
    };
#pragma GCC diagnostic pop
    ESP_ERROR_CHECK( esp_console_cmd_register(&heap_cmd) );
#endif
    heap::cmd.enreg_check();

}; /* register_heap */

/* 'heap' command prints minumum heap size */
esp_err_t heap::act::invoke(int argc, char* argv[])
//static int heap_size(int argc, char **argv)
{
    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    cout << "min heap size: " << prn_KMbytes(heap_size);
    cout << " (" << prettynumber(heap_size) << " bytes)" << endl;
    return 0;
}; /* heap_size */ /* heap::act::invoke() */




#if WITH_TASKS_INFO

/** 'tasks' command prints the list of tasks and related information */
namespace tasks
{
    struct act: public arg::table::act
    {
	/// Execute command procedure without the syntax object (zero syntax)
	static
	esp_err_t invoke(int argc, char* argv[]);
    }; /* struct tasks::act */

    const esp::console::cmd_t<act> cmd ("tasks", "Get information about running tasks");

}; /* namespace tasks */

static void register_tasks(void)
{
#if 0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
#pragma GCC diagnostic pop
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
#endif
    tasks::cmd.enreg_check();
}; /* register_tasks() */

/** 'tasks' command prints the list of tasks and related information */
/// Execute command procedure without the syntax object (zero syntax)
esp_err_t tasks::act::invoke(int argc, char* argv[])
//static int tasks_info(int argc, char **argv)
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
}; /* tasks::act::invoke() */

#endif // WITH_TASKS_INFO

/** log_level command changes log level via esp_log_level_set */

static struct {
    struct arg_str *tag;
    struct arg_str *level;
    struct arg_end *end;
} log_level_args;

static const char* s_log_level_names[] = {
    "none",
    "error",
    "warn",
    "info",
    "debug",
    "verbose"
};

template <typename T>
inline T next(T& item)
{
    item = static_cast<T>(static_cast<int>(item) + 1);
    return item;
}; /* next() */


static int log_level(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &log_level_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, log_level_args.end, argv[0]);
        return 1;
    }
    assert(log_level_args.tag->count == 1);
    assert(log_level_args.level->count == 1);
    const char* tag = log_level_args.tag->sval[0];
    const char* level_str = log_level_args.level->sval[0];
    esp_log_level_t level;
    size_t level_len = strlen(level_str);
    for (level = ESP_LOG_NONE; level <= ESP_LOG_VERBOSE; next(level))
    {
        if (memcmp(level_str, s_log_level_names[level], level_len) == 0) {
            break;
        }
    }; /* for level = ESP_LOG_NONE; level <= ESP_LOG_VERBOSE; next(level) */
    if (level > ESP_LOG_VERBOSE) {
        printf("Invalid log level '%s', choose from none|error|warn|info|debug|verbose\n", level_str);
        return 1;
    }
    if (level > CONFIG_LOG_MAXIMUM_LEVEL) {
        printf("Can't set log level to %s, max level limited in menuconfig to %s. "
               "Please increase CONFIG_LOG_MAXIMUM_LEVEL in menuconfig.\n",
               s_log_level_names[level], s_log_level_names[CONFIG_LOG_MAXIMUM_LEVEL]);
        return 1;
    }
    esp_log_level_set(tag, level);
    return 0;
}

static void register_log_level(void)
{
    log_level_args.tag = arg_str1(NULL, NULL, "<tag|*>", "Log tag to set the level for, or * to set for all tags");
    log_level_args.level = arg_str1(NULL, NULL, "<none|error|warn|debug|verbose>", "Log level to set. Abbreviated words are accepted.");
    log_level_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "log_level",
        .help = "Set log level for all tags or a specific tag.",
        .hint = NULL,
        .func = &log_level,
        .argtable = &log_level_args,
	.func_w_context = nullptr,
	.context = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}


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
