/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <cstdlib>
#include <iostream>
#include <iomanip>
//#include <thread>
//#include "esp_log.h"
//#include "gpio_cxx.hpp"

//#define __WITH_STDIO__
//#define __WITH_BOOST__
//#define __MAX_UNFOLDED_OUTPUT__
	//#define __EXPRESSION_OUTPUT__


//#include <stdio.h>
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


//using namespace idf;
using namespace std;


#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define WITH_TASKS_INFO 1
#endif

static const char *TAG = "cmd_system";

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
    cout << "ESP Console Example, Version: " CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE << endl;
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
#endif

#endif
    return 0;
}

static void register_version(void)
{
    const esp_console_cmd_t cmd = {
        .command = "version",
        .help = "Get version of chip and SDK",
        .hint = NULL,
        .func = &get_version,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/** 'restart' command restarts the program */

static int restart(int argc, char **argv)
{
    ESP_LOGI(TAG, "Restarting");
    esp_restart();
}

static void register_restart(void)
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}


#ifdef __WITH_STDIO__
/* Print int value with pretty fprmat & in bytes/megabytes etc. */
static int pretty_size_prn(const char prompt[], uint32_t value);
#else

// typedef for iostream manipulator
typedef ostream& (streamer)(ostream&);

// i/o manipulator for calling pretty_bytes
// w/partial application for procedure pretty_bytesed(ostream, value),
// parameter 'value'
auto pretty_bytes(uint32_t value) -> streamer*;

// Partial application of prn_KMbytes: fixing value
auto prn_KMbytes(uint32_t val) -> streamer*;

#endif


/** 'free' command prints available heap memory */
static int free_mem(int argc, char **argv)
{
#ifdef __WITH_STDIO__
    printf("free memory size: %d\n", esp_get_free_heap_size());
    pretty_size_prn("Test free memory size prn", 15003748);
#else
    cout << "free memory size: " << prn_KMbytes(esp_get_free_heap_size());
    cout << " (" << pretty_bytes(esp_get_free_heap_size()) << " bytes)" << endl;
#endif
    return 0;
}; /* free_mem */

static void register_free(void)
{
    const esp_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}; /* register_free */

/* 'heap' command prints minumum heap size */
static int heap_size(int argc, char **argv)
{
    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
#ifdef __WITH_STDIO__
//    printf("min heap size: %u\n", heap_size);
    pretty_size_prn("min heap size", heap_size);
#else
    cout << "min heap size: " << prn_KMbytes(heap_size);
    cout << " (" << pretty_bytes(heap_size) << " bytes)" << endl;
#endif
    return 0;
}; /* heap_size */

static void register_heap(void)
{
    const esp_console_cmd_t heap_cmd = {
        .command = "heap",
        .help = "Get minimum size of free heap memory that was available during program execution",
        .hint = NULL,
        .func = &heap_size,
    };
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
    const esp_console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
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
#ifdef __WITH_STDIO__
        printf("%d\n", cause);
#else
        cout << cause << endl;
#endif
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


#ifdef __WITH_STDIO__
// Print bytes count in groups by 3 digits
void pretty_bytes(uint32_t value);

// Print bytes count in Kb, Mb as needed
void prn_KMbytes(uint32_t value);
#else
// Print bytes count in groups by 3 digits
ostream& pretty_bytes(ostream& out, uint32_t value);

// Print bytes count in Kb, Mb as needed
ostream& prn_KMbytes(ostream& out, uint32_t value);
#endif


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




// i/o manipulator for calling pretty_bytes
// w/partial application for procedure pretty_bytesed(ostream, value),
// parameter 'value'
auto pretty_bytes(uint32_t value) -> streamer*
{
	static uint32_t outvalue = 0;

    outvalue = value;
    return ([&outvalue] (ostream& ost) -> ostream& {return pretty_bytes(ost, outvalue);});
}; /* pretty_bytes */

// Partial application of prn_KMbytes: fixing value
auto prn_KMbytes(uint32_t val) -> streamer*
{
	static uint32_t value = 0;

    value = val;
    return ([&value] (ostream& ost) -> ostream& {return prn_KMbytes(ost, value);});
}; /* prn_KMbytes(uint32_t value) */

ostream& pretty_size_prn(ostream& out, const char prompt[], uint32_t value);

auto pretty_size_prn(const char prompt[], uint32_t value) -> streamer*
{
	static const char *outprompt = NULL;
	static uint32_t outval = 0;

    outprompt = prompt;
    outval = value;
    return ([&outprompt, &outval] (ostream& ost) -> ostream& {return pretty_size_prn(ost, outprompt, outval);});
};



#ifdef __WITH_STDIO__
/* Print int value with pretty fprmat & in bytes/megabytes etc. */
static int pretty_size_prn(const char prompt[], uint32_t size)
{
//    printf("%s: %d bytes (", prompt, size);
    printf("%s: ", prompt);
    prn_KMbytes(size);
    printf(" (");
    pretty_bytes(size);
    printf(" bytes)\n");
    return 0;
}; /* pretty_size_prn */
#else
/* Print int value with pretty fprmat & in bytes/megabytes etc. */
ostream& pretty_size_prn(ostream& out, const char prompt[], uint32_t value)
{
        out << prompt << ": " << prn_KMbytes(value);
        out << " (" << pretty_bytes(value) << " bytes)";
    return out;
}; /* pretty_size_prn */
#endif

#ifdef __WITH_STDIO__
// Print bytes count in groups by 3 digits
void pretty_bytes(uint32_t size)
{
	uint32_t head = size / 1000;

    if (head > 0)
    {
	pretty_bytes(head);
	printf("%c%03u", DIGDELIM, size % 1000);
    }
    else
	printf("%u", size);
}; /* pretty_bytes */
#else
// Print bytes count in groups by 3 digits
ostream& pretty_bytes(ostream& out, uint32_t value)
{
	uint32_t head = value / 1000;

    if (head > 0)
    {
	//	printf("%c%03u", DIGDELIM, value % 1000);
	out << pretty_bytes(head) << DIGDELIM << setw(3) << setfill('0') << value % 1000;
    }
    else
//	printf("%u", value);
	out << value;
    return out;
}; /* pretty_bytes */

#endif

#ifdef __WITH_STDIO__
// Print the units of numerical value
void prn_KMbytes(uint32_t size)
{


    if (size < 10 * Knum)
    {
	// Printout of bytes
	pretty_bytes(size);
	printf(" bytes");
    } /* if size < 10 * Knum */
    else if (size < Knum * Knum)
    {
	// Printout of Kbytes
//	printf("%u bytes", size);
	printf("%u Kbytes", size / Knum);
	;
    } /* else if size < Knum^2 */
    else if (size < Knum * Knum * Knum)
    {
	// Printout of Mbytes
//		printf("%u bytes", size);
		printf("%u Mbytes", size / Knum / Knum);
	;
    } /* else if size < Knum^3 */
    else
    {
	// all other
//	printf("%u bytes", size);
	pretty_bytes(size);
	printf(" bytes");
    }; /* else */

}; /* prn_KMbytes */
#else



// Print the units of numerical value
ostream& prn_KMbytes(ostream& out, uint32_t value)
{
    if (value < 10 * Knum)
    {
	// Printout of bytes
	out << pretty_bytes(value) << " bytes";
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
	out << pretty_bytes(value) << " bytes";
    }; /* else */
    return out;

}; /* prn_KMbytes */
#endif
