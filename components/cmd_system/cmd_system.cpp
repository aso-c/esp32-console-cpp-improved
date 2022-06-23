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
#include "gpio_cxx.hpp"

//#define __WITH_STDIO__
//#define __WITH_BOOST__
//#define __MAX_UNFOLDED_OUTPUT__


#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_spi_flash.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cmd_system.h"
#include "sdkconfig.h"

#include <utility>
#include <functional>
#include <tuple>

#include "include/extrstream"



//using namespace idf;
using namespace std;


#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define WITH_TASKS_INFO 1
#endif

static const char *TAG = "cmd_system";


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
static void register_deep_sleep(void);
static void register_light_sleep(void);
#if WITH_TASKS_INFO
static void register_tasks(void);
#endif

void register_system_common(void)
{
    register_free();
    register_heap();
    register_version();
    register_restart();
#if WITH_TASKS_INFO
    register_tasks();
#endif
}

void register_system_sleep(void)
{
    register_deep_sleep();
    register_light_sleep();
}

void register_system(void)
{
    register_system_common();
    register_system_sleep();
}

/* 'version' command */
static int get_version(int argc, char **argv)
{
	esp_chip_info_t info;

    esp_chip_info(&info);
#ifdef __WITH_STDIO__

#ifdef __MAX_UNFOLDED_OUTPUT__
    printf("ESP Console Example, Version: %s-%s of %s,\r\n", CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
    printf("\t\t\t\t\t      modified by %s\r\n", CONFIG_APP_PROJECT_MODIFICATOR);
    printf("IDF Version: %s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);
#else
    printf("ESP Console Example, Version: %s of %s\r\n", CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
    printf("\t\t\t\t\t      modified by %s\r\n", CONFIG_APP_PROJECT_MODIFICATOR);
    printf("IDF Version: %s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);
#endif

#else

#ifdef __WITH_BOOST__
    printf("ESP Console Example, Version: %s-%s of %s,\r\n", CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
    printf("\t\t\t\t\t      modified by %s\r\n", CONFIG_APP_PROJECT_MODIFICATOR);
    printf("IDF Version: %s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);
#elif defined(__WITH_MY_FORMAT__)
    fprintf(cfile(cout), "ESP Console Example, Version: %s-%s of %s,\r\n", CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
    fprintf(cfile(cout), "\t\t\t\t\t      modified by %s\r\n", CONFIG_APP_PROJECT_MODIFICATOR);
    fprintf(cfile(cout), "IDF Version: %s\r\n", esp_get_idf_version());
    fprintf(cfile(cout), "Chip info:\r\n");
    fprintf(cfile(cout), "\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    fprintf(cfile(cout), "\tcores:%d\r\n", info.cores);
    fprintf(cfile(cout), "\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    fprintf(cfile(cout), "\trevision number:%d\r\n", info.revision);
#elif defined(__MAX_UNFOLDED_OUTPUT__)
    cout << "ESP Console Example, Version: " << CONFIG_APP_PROJECT_VER << '-' << CONFIG_APP_PROJECT_FLAVOUR
	 << " of " << CONFIG_APP_PROJECT_DATE << endl;
    cout << "\t\t\t\t\t      modified by " << CONFIG_APP_PROJECT_MODIFICATOR << endl;
    cout << "IDF Version: " << esp_get_idf_version() << endl;
    cout << "Chip info: " << endl;
    cout << "\tmodel: " << (info.model == CHIP_ESP32 ? "ESP32" : "Unknow") << endl;
    cout << "\tcores: " << (int)info.cores << endl;
    cout << "\tfeature:"
	<< (info.features & CHIP_FEATURE_WIFI_BGN ? "802.11bgn/" : "")
	<< (info.features & CHIP_FEATURE_BLE ? "BLE/" : "")
	<< (info.features & CHIP_FEATURE_BT ? "BT/" : "")
	<< (info.features & CHIP_FEATURE_EMB_FLASH ? "Embedded-Flash:" : "External-Flash:")
	<< spi_flash_get_chip_size() / (1024 * 1024) << " MB" << endl;
    cout << "\trevision number: " << (int)info.revision << endl;
#else
    fprintf(aso::cfile(cout), "ESP Console Example, Version: %s-%s of %s,\r\n", CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
    cout << "=== format implementation ======================================================" << endl;
//    cout << aso::format("ESP Console Example, Version: %s-%s of %s,\r\n");
    aso::format_impl(cout, "ESP Console Example, Version: %s-%s of %s,\r\n", CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
//    cout << "ESP Console Example, Version: " CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE << endl;
    cout << "=== format implementation ======================================================" << endl;
    cout << "=== aso::format test call ======================================================" << endl;
    cout << aso::format</*std::string, std::string, std::string*/>( "ESP Console Example, Version: %s-%s of %s,\r\n"/*, CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE*/);
  //   operator << (cout, aso::format( "ESP Console Example, Version: %%s-%%s of %%s,\r\n"));
    //cout << aso::format<>("ESP Console Example, Version: %s-%s of %s,\r\n");
//     cout << aso::format( "ESP Console Example, Version: %s-%s of %s,\r\n");
//     cout << aso::formatter_impl<"ESP Console Example, Version: %%s-%%s of %%s,\r\n">;
     cout << "=== aso::format test call ======================================================" << endl;
    fprintf(aso::cfile(cout), "\t\t\t\t\t      modified by %s\r\n", CONFIG_APP_PROJECT_MODIFICATOR);
//    cout << "\t\t\t\t\t      modified by " << CONFIG_APP_PROJECT_MODIFICATOR << endl;
//    fprintf(cfile(cout), "IDF Version: %s\r\n", esp_get_idf_version());
    cout << "IDF Version: " << esp_get_idf_version() << endl;
//    fprintf(cfile(cout), "Chip info:\r\n");
    cout << "Chip info: " << endl;
//    fprintf(cfile(cout), "\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    cout << "\tmodel: " << (info.model == CHIP_ESP32 ? "ESP32" : "Unknow") << endl;
//    fprintf(cfile(cout), "\tcores:%d\r\n", info.cores);
    cout << "\tcores: " << (int)info.cores << endl;
#if 0
    fprintf(cfile(cout), "\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
#else
    cout << "\tfeature:"
	<< (info.features & CHIP_FEATURE_WIFI_BGN ? "802.11bgn/" : "")
	<< (info.features & CHIP_FEATURE_BLE ? "BLE/" : "")
	<< (info.features & CHIP_FEATURE_BT ? "BT/" : "")
	<< (info.features & CHIP_FEATURE_EMB_FLASH ? "Embedded-Flash:" : "External-Flash:")
	<< spi_flash_get_chip_size() / (1024 * 1024) << " MB" << endl;
#endif
//    fprintf(cfile(cout), "\trevision number:%d\r\n", info.revision);
    cout << "\trevision number: " << (int)info.revision << endl;
#endif

#endif
    return 0;
}

static void register_version(void)
{
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
}

/** 'restart' command restarts the program */

static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
    return -1;	// stub for supress warning only
}

static void register_restart(void)
{
#pragma GCC diagnostic push
//#pragma -Wmissing-field-initializers
//#pragma GCC diagnostic [warning|error|ignored] OPTION
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
#pragma GCC diagnostic pop
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}


/** 'free' command prints available heap memory */
static int free_mem(int argc, char **argv)
{
    cout << "free memory size: " << prn_KMbytes(esp_get_free_heap_size());
//    cout << " (" << prettybytes(esp_get_free_heap_size()) << " bytes)" << endl;
    cout << " (" << prettynumber(esp_get_free_heap_size()) << " bytes)" << endl;
    return 0;
}; /* free_mem */

static void register_free(void)
{
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
}; /* register_free */

/* 'heap' command prints minumum heap size */
static int heap_size(int argc, char **argv)
{
    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    cout << "min heap size: " << prn_KMbytes(heap_size);
//    cout << " (" << prettybytes(heap_size) << " bytes)" << endl;
    cout << " (" << prettynumber(heap_size) << " bytes)" << endl;
    return 0;
}; /* heap_size */

static void register_heap(void)
{
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

}; /* register_heap */

/** 'tasks' command prints the list of tasks and related information */
#if WITH_TASKS_INFO

static int tasks_info(int argc, char **argv)
{
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = (char*)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        ESP_LOGE(TAG, "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    fputs("\tAffinity", stdout);
#endif
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}

static void register_tasks(void)
{
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
}

#endif // WITH_TASKS_INFO

/** 'deep_sleep' command puts the chip into deep sleep mode */

static struct {
    struct arg_int *wakeup_time;
#if SOC_PM_SUPPORT_EXT_WAKEUP
    struct arg_int *wakeup_gpio_num;
    struct arg_int *wakeup_gpio_level;
#endif
    struct arg_end *end;
} deep_sleep_args;


static int deep_sleep(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &deep_sleep_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, deep_sleep_args.end, argv[0]);
        return 1;
    }
    if (deep_sleep_args.wakeup_time->count) {
        uint64_t timeout = 1000ULL * deep_sleep_args.wakeup_time->ival[0];
        ESP_LOGI(TAG, "Enabling timer wakeup, timeout=%lluus", timeout);
        ESP_ERROR_CHECK( esp_sleep_enable_timer_wakeup(timeout) );
    }

#if SOC_PM_SUPPORT_EXT_WAKEUP
    if (deep_sleep_args.wakeup_gpio_num->count) {
        int io_num = deep_sleep_args.wakeup_gpio_num->ival[0];
        if (!esp_sleep_is_valid_wakeup_gpio((gpio_num_t)io_num)) {
            ESP_LOGE(TAG, "GPIO %d is not an RTC IO", io_num);
            return 1;
        }
        int level = 0;
        if (deep_sleep_args.wakeup_gpio_level->count) {
            level = deep_sleep_args.wakeup_gpio_level->ival[0];
            if (level != 0 && level != 1) {
                ESP_LOGE(TAG, "Invalid wakeup level: %d", level);
                return 1;
            }
        }
        ESP_LOGI(TAG, "Enabling wakeup on GPIO%d, wakeup on %s level",
                 io_num, level ? "HIGH" : "LOW");

        ESP_ERROR_CHECK( esp_sleep_enable_ext1_wakeup(1ULL << io_num, (esp_sleep_ext1_wakeup_mode_t)level) );
        ESP_LOGE(TAG, "GPIO wakeup from deep sleep currently unsupported on ESP32-C3");
    }
#endif // SOC_PM_SUPPORT_EXT_WAKEUP

#if CONFIG_IDF_TARGET_ESP32
    rtc_gpio_isolate(GPIO_NUM_12);
#endif //CONFIG_IDF_TARGET_ESP32

    esp_deep_sleep_start();
}

static void register_deep_sleep(void)
{
    int num_args = 1;
    deep_sleep_args.wakeup_time =
        arg_int0("t", "time", "<t>", "Wake up time, ms");
#if SOC_PM_SUPPORT_EXT_WAKEUP
    deep_sleep_args.wakeup_gpio_num =
        arg_int0(NULL, "io", "<n>",
                 "If specified, wakeup using GPIO with given number");
    deep_sleep_args.wakeup_gpio_level =
        arg_int0(NULL, "io_level", "<0|1>", "GPIO level to trigger wakeup");
    num_args += 2;
#endif
    deep_sleep_args.end = arg_end(num_args);

    const esp_console_cmd_t cmd = {
        .command = "deep_sleep",
        .help = "Enter deep sleep mode. "
#if SOC_PM_SUPPORT_EXT_WAKEUP
        "Two wakeup modes are supported: timer and GPIO. "
#else
        "Timer wakeup mode is supported. "
#endif
        "If no wakeup option is specified, will sleep indefinitely.",
        .hint = NULL,
        .func = &deep_sleep,
        .argtable = &deep_sleep_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/** 'light_sleep' command puts the chip into light sleep mode */

static struct {
    struct arg_int *wakeup_time;
    struct arg_int *wakeup_gpio_num;
    struct arg_int *wakeup_gpio_level;
    struct arg_end *end;
} light_sleep_args;

static int light_sleep(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &light_sleep_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, light_sleep_args.end, argv[0]);
        return 1;
    }
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    if (light_sleep_args.wakeup_time->count) {
        uint64_t timeout = 1000ULL * light_sleep_args.wakeup_time->ival[0];
        ESP_LOGI(TAG, "Enabling timer wakeup, timeout=%lluus", timeout);
        ESP_ERROR_CHECK( esp_sleep_enable_timer_wakeup(timeout) );
    }
    int io_count = light_sleep_args.wakeup_gpio_num->count;
    if (io_count != light_sleep_args.wakeup_gpio_level->count) {
        ESP_LOGE(TAG, "Should have same number of 'io' and 'io_level' arguments");
        return 1;
    }
    for (int i = 0; i < io_count; ++i) {
        int io_num = light_sleep_args.wakeup_gpio_num->ival[i];
        int level = light_sleep_args.wakeup_gpio_level->ival[i];
        if (level != 0 && level != 1) {
            ESP_LOGE(TAG, "Invalid wakeup level: %d", level);
            return 1;
        }
        ESP_LOGI(TAG, "Enabling wakeup on GPIO%d, wakeup on %s level",
                 io_num, level ? "HIGH" : "LOW");

        ESP_ERROR_CHECK( gpio_wakeup_enable((gpio_num_t)io_num, level ? GPIO_INTR_HIGH_LEVEL : GPIO_INTR_LOW_LEVEL) );
    }
    if (io_count > 0) {
        ESP_ERROR_CHECK( esp_sleep_enable_gpio_wakeup() );
    }
    if (CONFIG_ESP_CONSOLE_UART_NUM >= 0 && CONFIG_ESP_CONSOLE_UART_NUM <= UART_NUM_1) {
        ESP_LOGI(TAG, "Enabling UART wakeup (press ENTER to exit light sleep)");
        ESP_ERROR_CHECK( uart_set_wakeup_threshold(CONFIG_ESP_CONSOLE_UART_NUM, 3) );
        ESP_ERROR_CHECK( esp_sleep_enable_uart_wakeup(CONFIG_ESP_CONSOLE_UART_NUM) );
    }
    fflush(stdout);
    fsync(fileno(stdout));
    esp_light_sleep_start();
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    const char *cause_str;
    switch (cause) {
    case ESP_SLEEP_WAKEUP_GPIO:
        cause_str = "GPIO";
        break;
    case ESP_SLEEP_WAKEUP_UART:
        cause_str = "UART";
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        cause_str = "timer";
        break;
    default:
        cause_str = "unknown";
        cout << cause << endl;
    }
    ESP_LOGI(TAG, "Woke up from: %s", cause_str);
    return 0;
}

static void register_light_sleep(void)
{
    light_sleep_args.wakeup_time =
        arg_int0("t", "time", "<t>", "Wake up time, ms");
    light_sleep_args.wakeup_gpio_num =
        arg_intn(NULL, "io", "<n>", 0, 8,
                 "If specified, wakeup using GPIO with given number");
    light_sleep_args.wakeup_gpio_level =
        arg_intn(NULL, "io_level", "<0|1>", 0, 8, "GPIO level to trigger wakeup");
    light_sleep_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "light_sleep",
        .help = "Enter light sleep mode. "
        "Two wakeup modes are supported: timer and GPIO. "
        "Multiple GPIO pins can be specified using pairs of "
        "'io' and 'io_level' arguments. "
        "Will also wake up on UART input.",
        .hint = NULL,
        .func = &light_sleep,
        .argtable = &light_sleep_args
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
	    " modified by " CONFIG_APP_PROJECT_MODIFICATOR "."/* "\r\n"
	    "Build Date: " __DATE__ " " __TIME__ "."*/;
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
	//	printf("%c%03u", DIGDELIM, value % 1000);
	out << prettynumber(head) << DIGDELIM << setw(3) << setfill('0') << value % 1000;
    }
    else
//	printf("%u", value);
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
//	printf("%u Kbytes", value / Knum);
	out << value / Knum << " Kbytes";
	;
    } /* else if size < Knum^2 */
    else if (value < Knum * Knum * Knum)
    {
	// Printout of Mbytes
//	printf("%u Mbytes", value / Knum / Knum);
	out << value / Knum / Knum << " Mbytes";
    } /* else if size < Knum^3 */
    else
    {
	// all other
	out << prettynumber(value) << " bytes";
    }; /* else */
    return out;

}; /* prn_KMbytes */
