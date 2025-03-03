/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Console example — various sleep-related commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cinttypes>
#include <unistd.h>

#include "esp_log.h"
#include "esp_console.h"
#include "esp_chip_info.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "cmd_system.h"
#include "sdkconfig.h"

#include <argtable>
#include <console>


static const char *TAG = "cmd_system_sleep";

namespace sys
{
    /** The 'sleep' commands: - puts the chip into deep or light sleep mode */
    namespace sleep
    {
	arg::table::syntax syntax = {
#if 0
//---------------------------------------------------
	    deep_sleep_args.wakeup_time =
	        arg_int0("t", "time", "<t>", "Wake up time, ms");
	#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
	    deep_sleep_args.wakeup_gpio_num =
	        arg_int0(NULL, "io", "<n>",
	                 "If specified, wakeup using GPIO with given number");
	    deep_sleep_args.wakeup_gpio_level =
	        arg_int0(NULL, "io_level", "<0|1>", "GPIO level to trigger wakeup");
	    num_args += 2;
	#endif
	    deep_sleep_args.end = arg_end(num_args);
//---------------------------------------------------
	    static struct {
	        struct arg_int *wakeup_time;
	    #if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
	        struct arg_int *wakeup_gpio_num;
	        struct arg_int *wakeup_gpio_level;
	    #endif
	        struct arg_end *end;
	    } deep_sleep_args;
//---------------------------------------------------
	    static struct {
	        struct arg_int *wakeup_time;
	        struct arg_int *wakeup_gpio_num;
	        struct arg_int *wakeup_gpio_level;
	        struct arg_end *end;
	    } light_sleep_args;
//---------------------------------------------------
	    light_sleep_args.wakeup_time =
	        arg_int0("t", "time", "<t>", "Wake up time, ms");
	    light_sleep_args.wakeup_gpio_num =
	        arg_intn(NULL, "io", "<n>", 0, 8,
	                 "If specified, wakeup using GPIO with given number");
	    light_sleep_args.wakeup_gpio_level =
	        arg_intn(NULL, "io_level", "<0|1>", 0, 8, "GPIO level to trigger wakeup");
	    light_sleep_args.end = arg_end(3);
//---------------------------------------------------
#endif
	    5,
	    arg_rex0("hH", "help,usage", "help", nullptr/*"<sleep_mode>"*/, ARG_REX_ICASE/*Arg::Rex::ICase*/, "Help/usage (short help) about this command"),
	    arg_rex1(nullptr, nullptr, "light|deep", "<sleep_mode>", ARG_REX_ICASE/*Arg::Rex::ICase*/, "Setting required sleep mode"),
	    arg_int0("t", "time", "<t>", "Wake up time, ms"),
	    arg_intn(NULL, "io", "<n>", 0, 8,
			"If specified, wakeup using GPIO with given number"),
	    arg_intn(NULL, "io_level", "<0|1>", 0, 8, "GPIO level to trigger wakeup"),
//	    5
//	    light_sleep_args.end = arg_end(3);
	}; /* sys::sleep::syntax */

	struct act: public arg::table::act_t<syntax>
	{
	    static esp_err_t invoke(int argc, char* argv[]);
	}; /* struct type::act */
	const esp::console::cmd_t<act> cmd("sleep", "Enter the sleep mode - light or a deep sleep.\n "
//	        "Wakeup is possible by the timer or GPIO (optionally for deep sleep mode). "
//	        "For light sleep - multiple GPIO pins can be specified using pairs of "
//	        "'io' and 'io_level' arguments, "
//		"and will also wake up on UART input. "
		"For details - use help <light|deep> form.\n"
		"'light_sleep' and a 'deep_sleep' - is a shortcuts for 'sleep light' or a 'sleep deep' commands."/*,
		"sleep mode: deep | light"*/);


	/** 'deep_sleep' command puts the chip into deep sleep mode */
	namespace deep
	{
	    arg::table::syntax syntax = {
#if !(SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP)
		    1,
#else
			3,
#endif	//  !(SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP)
//			    deep_sleep_args.wakeup_time =
		    arg_int0("t", "time", "<t>", "Wake up time, ms"),
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
//			    deep_sleep_args.wakeup_gpio_num =
		        arg_int0(NULL, "io", "<n>",
			                 "If specified, wakeup using GPIO with given number"),
//			    deep_sleep_args.wakeup_gpio_level =
		        arg_int0(NULL, "io_level", "<0|1>", "GPIO level to trigger wakeup"),
#endif	//  SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
//		    /*deep_sleep_args.end =*/ arg_end(num_args)
	    }; /* sys::sleep::deep::syntax */
#if 0
	    static struct {
	        struct arg_int *wakeup_time;
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
	        struct arg_int *wakeup_gpio_num;
	        struct arg_int *wakeup_gpio_level;
#endif
	        struct arg_end *end;
	    } deep_sleep_args;
#endif

	    struct act: public arg::table::act_t<syntax>
	    {
		static esp_err_t invoke(int argc, char* argv[]);

		struct wakeup
		{
//		    static auto& time() { return syntax[0].asint(); };
		    static inline auto& time = syntax[0].asint();
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
		    struct gpio
		    {
//			static auto& num() { return syntax[1].asint(); };
			static inline auto& num = syntax[1].asint();
//			static auto& level() { return syntax[2].asint(); };
			static inline auto& level = syntax[2].asint();
		    }; /*  struct sys::sleep::deep::act::wakeup::gpio  */
#endif
		}; /* struct sys::sleep::deep::act::wakeup  */

	    }; /* struct sys::sleep::deep::act */
	    const esp::console::cmd_t<act> cmd("deep_sleep", "Enter deep sleep mode. "
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
						"Two wakeup modes are supported: timer and GPIO. "
#else
						"Timer wakeup mode is supported. "
#endif	// SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
			"If no wakeup option is specified, will sleep indefinitely."
			);
	}; /* namespace sys::sleep::deep */



	namespace light
	{
	    arg::table::syntax syntax = {
		        arg_int0("t", "time", "<t>", "Wake up time, ms"),
		        arg_intn(NULL, "io", "<n>", 0, 8,
		                 "If specified, wakeup using GPIO with given number"),
		        arg_intn(NULL, "io_level", "<0|1>", 0, 8, "GPIO level to trigger wakeup")
//		    light_sleep_args.end = arg_end(3);
	    }; /* sys::sleep::syntax */
#if 0
	    static struct {
	        struct arg_int *wakeup_time;
	        struct arg_int *wakeup_gpio_num;
	        struct arg_int *wakeup_gpio_level;
	        struct arg_end *end;
	    } light_sleep_args;
#endif

	    struct act: public arg::table::act_t<syntax>
	    {
		static esp_err_t invoke(int argc, char* argv[]);

		struct wakeup
		{
//		    static auto& time() { return syntax[0].asint(); };
//		    static inline auto& timep = arg::table::item::get<struct arg_int>/*std::get<std::shared_ptr<arg_int>>*/(syntax[0]);
		    static inline auto& time = syntax[0].asint();
		    struct gpio
		    {
//			static auto& num() { return syntax[1].asint(); };
			static inline auto& num = syntax[1].asint();
//			static auto& level() { return syntax[2].asint(); };
			static inline auto& level = syntax[2].asint();
		    }; /* struct sys::sleep::light::act::wakeup::gpio */
		}; /* struct sys::sleep::light::act::wakeup */
	    }; /* struct sys::sleep::light::act */
	    const esp::console::cmd_t<act> cmd("light_sleep", "Enter light sleep mode. "
							"Two wakeup modes are supported: timer and GPIO. "
							"Multiple GPIO pins can be specified using pairs of "
							"'io' and 'io_level' arguments. "
							"Will also wake up on UART input.");
	}; /* namespace sys::sleep::light */
    }; /* namespace sys::sleep */
}; /* namespace sys */





//static void register_deep_sleep(void);
//static void register_light_sleep(void);

void register_system_sleep(void)
{
    sys::sleep::cmd.enreg_check();
    sys::sleep::deep::cmd.enreg_check();
    sys::sleep::light::cmd.enreg_check();
}; /* register_system_sleep() */


/** 'deep_sleep' command puts the chip into deep sleep mode */

#if 0
static struct {
    struct arg_int *wakeup_time;
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
    struct arg_int *wakeup_gpio_num;
    struct arg_int *wakeup_gpio_level;
#endif
    struct arg_end *end;
} deep_sleep_args;
#endif


//static int deep_sleep(int argc, char **argv)
esp_err_t sys::sleep::deep::act::invoke(int argc, char* argv[])
{
#if 0
	constexpr int time_idx = 0;
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
	constexpr int num_idx = 1;
	constexpr int  level_idx = 2;
#endif
#endif // if 0

//    int nerrors = arg_parse(argc, argv, (void **) &deep_sleep_args);
    syntax.parse(argc, argv);
    if (syntax.err())
    {
       syntax.errors(stderr, argv[0]);
       return syntax.err();
    }; /* if syntax.err() */
//    if (syntax[time_idx].asint().count) {
    if (wakeup::time.count) {
//        uint64_t timeout = 1000ULL * syntax[time_idx].asint().ival[0];
        uint64_t timeout = 1000ULL * wakeup::time.ival[0];
        ESP_LOGI(TAG, "Enabling timer wakeup, timeout=%lluus", timeout);
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(timeout));
    }; /* if (syntax[time_idx].asint().count) */

#if SOC_PM_SUPPORT_EXT1_WAKEUP
    //if (std::get<arg::table::integer>(syntax[num_idx])->count) {
    if (wakeup::gpio::num.count)
    {
//        int io_num = std::get<arg::table::integer>(syntax[num_idx])->ival[0];
        int io_num = wakeup::gpio::num.ival[0];
        if (!esp_sleep_is_valid_wakeup_gpio(static_cast<gpio_num_t>(io_num))) {
            ESP_LOGE(TAG, "GPIO %d is not an RTC IO", io_num);
            return 1;
        }; /* if !esp_sleep_is_valid_wakeup_gpio(static_cast<gpio_num_t>(io_num)) */
        int level = 0;
//        if (std::get<arg::table::integer>(syntax[level_idx])->count)
        if (wakeup::gpio::level.count)
        {
//            level = std::get<arg::table::integer>(syntax[level_idx])->ival[0];
            level = wakeup::gpio::level.ival[0];
//            if (level != 0 && level != 1) {
            if (!(level == 0 || level == 1))
            {
                ESP_LOGE(TAG, "Invalid wakeup level: %d", level);
                return 1;
            }; /* if !(level == 0 || level == 1) */
        }; /* if wakeup::gpio::level().count */
        ESP_LOGI(TAG, "Enabling wakeup on GPIO%d, wakeup on %s level",
                 io_num, level ? "HIGH" : "LOW");

        ESP_ERROR_CHECK( esp_sleep_enable_ext1_wakeup(1ULL << io_num, static_cast<esp_sleep_ext1_wakeup_mode_t>(level)) );
        ESP_LOGE(TAG, "GPIO wakeup from deep sleep currently unsupported on ESP32-C3");
    }; /* if wakeup::gpio::num().count */
#endif // SOC_PM_SUPPORT_EXT1_WAKEUP

#if CONFIG_IDF_TARGET_ESP32
    rtc_gpio_isolate(GPIO_NUM_12);
#endif //CONFIG_IDF_TARGET_ESP32

    esp_deep_sleep_start();
    return 1;
}; /* sys::sleep::deep::act::invoke() */


#if 0
static void register_deep_sleep(void)
{
    int num_args = 1;
    deep_sleep_args.wakeup_time =
        arg_int0("t", "time", "<t>", "Wake up time, ms");
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
    deep_sleep_args.wakeup_gpio_num =
        arg_int0(NULL, "io", "<n>",
                 "If specified, wakeup using GPIO with given number");
    deep_sleep_args.wakeup_gpio_level =
        arg_int0(NULL, "io_level", "<0|1>", "GPIO level to trigger wakeup");
    num_args += 2;
#endif
    deep_sleep_args.end = arg_end(num_args);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t cmd = {
        .command = "deep_sleep",
        .help = "Enter deep sleep mode. "
#if SOC_PM_SUPPORT_EXT0_WAKEUP || SOC_PM_SUPPORT_EXT1_WAKEUP
        "Two wakeup modes are supported: timer and GPIO. "
#else
        "Timer wakeup mode is supported. "
#endif
        "If no wakeup option is specified, will sleep indefinitely.",
        .hint = NULL,
        .func = &deep_sleep,
        .argtable = &deep_sleep_args
    }; /* const esp_console_cmd_t cmd */
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
#pragma GCC diagnostic pop
}; /* register_deep_sleep() */
#endif

/** 'light_sleep' command puts the chip into light sleep mode */
#if 0
static struct {
    struct arg_int *wakeup_time;
    struct arg_int *wakeup_gpio_num;
    struct arg_int *wakeup_gpio_level;
    struct arg_end *end;
} light_sleep_args;
#endif

//static int light_sleep(int argc, char **argv)
/** invoke the 'light_sleep' command, that puts the chip into light sleep mode */
esp_err_t sys::sleep::light::act::invoke(int argc, char* argv[])
{
//    int nerrors = arg_parse(argc, argv, (void **) &light_sleep_args);
    syntax.parse(argc, argv);
//    if (nerrors != 0) {
//        arg_print_errors(stderr, light_sleep_args.end, argv[0]);
//        return 1;
//    }
    if (syntax.err())
    {
       syntax.errors(stderr, argv[0]);
       return syntax.err();
    }; /* if syntax.err() */
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
//    if (light_sleep_args.wakeup_time->count) {
    if (wakeup::time.count) {
//        uint64_t timeout = 1000ULL * light_sleep_args.wakeup_time->ival[0];
        uint64_t timeout = 1000ULL * wakeup::time.ival[0];
        ESP_LOGI(TAG, "Enabling timer wakeup, timeout=%lluus", timeout);
        ESP_ERROR_CHECK( esp_sleep_enable_timer_wakeup(timeout) );
    }; /* if (wakeup::time->count) */
//    int io_count = light_sleep_args.wakeup_gpio_num->count;
    int io_count = wakeup::gpio::num.count;
//    if (io_count != light_sleep_args.wakeup_gpio_level->count) {
    if (io_count != wakeup::gpio::level.count)
    {
        ESP_LOGE(TAG, "Should have same number of 'io' and 'io_level' arguments");
        return 1;
    }; /* if io_count != wakeup::gpio::level->count */

    for (int i = 0; i < io_count; ++i)
    {
//        int io_num = light_sleep_args.wakeup_gpio_num->ival[i];
        int io_num = wakeup::gpio::num.ival[i];
//        int level = light_sleep_args.wakeup_gpio_level->ival[i];
        int level = wakeup::gpio::level.ival[i];
        if (level != 0 && level != 1) {
            ESP_LOGE(TAG, "Invalid wakeup level: %d", level);
            return 1;
        }; /* if level != 0 && level != 1 */
        ESP_LOGI(TAG, "Enabling wakeup on GPIO%d, wakeup on %s level",
                 io_num, level ? "HIGH" : "LOW");

        ESP_ERROR_CHECK( gpio_wakeup_enable(static_cast<gpio_num_t>(io_num), level ? GPIO_INTR_HIGH_LEVEL : GPIO_INTR_LOW_LEVEL) );
    }; /* for int i = 0; i < io_count; ++i */

    if (io_count > 0)
    {
        ESP_ERROR_CHECK( esp_sleep_enable_gpio_wakeup() );
    }
    if (CONFIG_ESP_CONSOLE_UART_NUM >= 0 && CONFIG_ESP_CONSOLE_UART_NUM <= UART_NUM_1) {
        ESP_LOGI(TAG, "Enabling UART wakeup (press ENTER to exit light sleep)");
        ESP_ERROR_CHECK( uart_set_wakeup_threshold(static_cast<uart_port_t>(CONFIG_ESP_CONSOLE_UART_NUM), 3) );
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
        printf("%d\n", cause);
        // cout << cause << endl;	// Intended for migration to C++ 
    }

    ESP_LOGI(TAG, "Woke up from: %s", cause_str);
    return 0;
}; /* sys::sleep::light::act::invoke() */

#if 0
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
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
    }; /* const esp_console_cmd_t cmd */
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
#pragma GCC diagnostic push
}; /* register_light_sleep(void) */
#endif

esp_err_t sys::sleep::act::invoke(int argc, char* argv[])
{
    return ESP_OK;
}; /* sys_sleep::act::invoke() */
