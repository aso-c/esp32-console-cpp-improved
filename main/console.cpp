/* Console Improved project

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <cstdlib>
#include <iostream>
//#include <thread>
//#include "gpio_cxx.hpp"


//#include <cstdio>
//#include <cstring>
#include <string>
//#include <cunisd>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
//#include "esp_vfs_dev.h"
//#include "driver/uart.h"
//-//#include "driver/uart_vfs.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/soc_caps.h"	// --?
#include "cmd_system.h"	// --?
#include "cmd_wifi.h"	// --?
#include "cmd_nvs.h"	// --?
#include "console_settings.h"
#include "cmd_decl.h"

#include <argtable>
#include <console>

//#include "cmd_decl.h"
#include <astring.h>

//using namespace idf;
using namespace std;

/*
 * We warn if a secondary serial console is enabled. A secondary serial console is always output-only and
 * hence not very useful for interactive console applications. If you encounter this warning, consider disabling
 * the secondary serial console in menuconfig unless you know what you are doing.
 */
#if SOC_USB_SERIAL_JTAG_SUPPORTED
#if !CONFIG_ESP_CONSOLE_SECONDARY_NONE
#warning "A secondary serial console is not useful when using the console component. Please disable it in menuconfig."
#endif
#endif

#ifdef CONFIG_ESP_CONSOLE_USB_CDC
#error This example is incompatible with USB CDC console. Please try "console_usb" example instead.
#endif // CONFIG_ESP_CONSOLE_USB_CDC

#ifdef CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
#error This example is incompatible with USB serial JTAG console.
#endif // CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG


#define __INN_STR__(str) #str
#define STRING(str) __INN_STR__(str)
#pragma message("ESP32 Improved Console" ", version v." CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE " by " CONFIG_APP_PROJECT_AUTHOR " (" CONFIG_APP_PROJECT_AUTHOR_NICK ")")
#pragma message("C++ version is: " STRING(__cplusplus) )

//static const char* TAG = "example";
static const char* TAG = "improved console";
#define PROMPT_STR CONFIG_IDF_TARGET

// Hardware configuration detail
void initialize_hardware(void)
{
    // for SD-card connection

    gpio_pullup_en(GPIO_NUM_12);
}; /* initialize_hardware(void) */


/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_CONSOLE_STORE_HISTORY

#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

static void initialize_filesystem(void)
{
    static wl_handle_t wl_handle;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true,
//            .max_files = 4,
    };
#pragma GCC diagnostic pop
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
}
#else
#define HISTORY_PATH NULL
#endif // CONFIG_CONSOLE_STORE_HISTORY

static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

#if 0
static void initialize_console(void)
{
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    uart_vfs_dev_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    uart_vfs_dev_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
#if SOC_UART_SUPPORT_REF_TICK
        .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
        .source_clk = UART_SCLK_XTAL,
#endif
    };
#pragma GCC diagnostic pop
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(static_cast<uart_port_t>(CONFIG_ESP_CONSOLE_UART_NUM),
            256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config(static_cast<uart_port_t>(CONFIG_ESP_CONSOLE_UART_NUM), &uart_config) );

    /* Tell VFS to use UART driver */
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_length = 256,
            .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
#pragma GCC diagnostic pop
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

#if CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(HISTORY_PATH);
#endif
}
#endif //static void initialize_console(void)


///--[ Registering main infrastructure command - help & info ]---------------------------

/**
 * @brief Namespace of the fake command only for output version information in a 'help' command
 *
 */
namespace info
{
    arg::table::syntax syntax{ arg_str1(NULL, NULL, "Build Date:", __DATE__ " " __TIME__ ".") };

    /// definition of the act for the 'info' pseudo-command
    struct act: public arg::table::act_t<syntax>
    {

	/**
	 * @brief 'info' pseudo-command procedure about a version information of a project
	 *
	 * Printout version info & small description,
	 * about this project
	 *
	 * @return
	 *      - ESP_OK on success
	 *      - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
	 */
	static
	esp_err_t invoke(int argc, char * argv[]);

    }; /* class bt::act */


    /// @brief Info pseudo-command procedure about a version information of a project
    esp_err_t act::invoke(int argc, char * argv[])
    {
        cout << "ESP Console Example Project, Version: " CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE
    	    << ", builded with C++ version " << __cplusplus  << endl;
        return ESP_OK;
    }; /* act::invoke() */

    const esp::console::cmd_t<act> cmd("info",  version_str(), "about this project");

}; /* namespace info */

/// for unification only
inline void register_info(void) {
    info::cmd.enreg_check();
}; /* register_info */




/**
 * @brief Register a 'help' command
 *
 * Default 'help' command prints the list of registered commands along with
 * hints and help strings if no additional argument is given. If an additional
 * argument is given, the help command will look for a command with the same
 * name and only print the hints and help strings of that command.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
 */


/**
 * @brief Register a 'help' command for a console example project
 *
 * Own 'help' command implementation first run default 'help' command,
 * and then prints the version string of a program.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
 */
esp_err_t console_register_help_command(void)
{
    return (esp_err_t)esp_console_register_help_command();
}; /* console_example_register_help_command */

//--[ End of command registering - help & info ]-----------------------------------------



extern "C" void app_main(void)
{
    initialize_hardware();

    initialize_nvs();

#if CONFIG_CONSOLE_STORE_HISTORY
    initialize_filesystem();
    ESP_LOGI(TAG, "Command history enabled");
#else
    ESP_LOGI(TAG, "Command history disabled");
#endif

    /* Initialize console output periheral (UART, USB_OTG, USB_JTAG) */
    initialize_console_peripheral();

    /* Initialize linenoise library and esp_console*/
    initialize_console_library(HISTORY_PATH);

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
//    const char *prompt = setup_prompt(PROMPT_STR ">");
    std::string prompt = setup_prompt(PROMPT_STR ">");

    /* Register commands */
    //esp_console_register_help_command();
    console_register_help_command();
    register_system_common();
#if 0	// No - acessible sleep mode must selected in the cmd_system component
#if SOC_LIGHT_SLEEP_SUPPORTED
    register_system_light_sleep();
#endif
#if SOC_DEEP_SLEEP_SUPPORTED
    register_system_deep_sleep();
#endif
#endif	// if 0
    register_system_sleep();
#if (CONFIG_ESP_WIFI_ENABLED || CONFIG_ESP_HOST_WIFI_ENABLED)
    register_wifi();
#endif
    register_nvs();
    register_fs_cmd_all();
    register_sdcard_cmd();
    register_bt_cmd();

    register_info();
//    info::cmd.enreg_chked();

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
//    const char* prompt = LOG_COLOR_I PROMPT_STR "> " LOG_RESET_COLOR;

    cout << endl
	<< "This is a ESP-IDF improved console project, that using appropriate component." << endl
	<<   "Version " << CONFIG_APP_PROJECT_VER << '-' << CONFIG_APP_PROJECT_FLAVOUR
	<< " of " << CONFIG_APP_PROJECT_DATE << ',' << " modified by "
	<< CONFIG_APP_PROJECT_AUTHOR << '.' << endl
	<< "Builded " << __DATE__ << " " << __TIME__ << endl
	<< "Type 'help' to get the list of commands." << endl
	<< "Use UP/DOWN arrows to navigate through command history." << endl
	<< "Press TAB when typing command name to auto-complete." << endl
	<< "Press Enter or Ctrl+C will terminate the console environment." << endl;

    /* Figure out if the terminal supports escape sequences */
    if (linenoiseIsDumbMode()) {
        cout << endl
	    << "Your terminal application does not support escape sequences." << endl
	    << "Line editing and history features are disabled." << endl
	    << "On Windows, try using Putty instead." << endl;

#if 0
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = PROMPT_STR "> ";
#endif //CONFIG_LOG_COLORS
#endif
    }

    /* Main loop */
    while(true) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt.data());

#if CONFIG_CONSOLE_IGNORE_EMPTY_LINES
        if (line == NULL) { /* Ignore empty lines */
            continue;;
        }
#else
        if (line == NULL) { /* Break on EOF or error */
            break;
        }
#endif // CONFIG_CONSOLE_IGNORE_EMPTY_LINES

        /* Add the command to the history if not empty*/
//        if (strlen(line) > 0) {
        if (std::string_view(line).length() > 0) {
            linenoiseHistoryAdd(line);
#if CONFIG_CONSOLE_STORE_HISTORY
            /* Save command history to filesystem */
            linenoiseHistorySave(HISTORY_PATH);
#endif // CONFIG_CONSOLE_STORE_HISTORY
        }

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);

        switch (err)
	{
        case ESP_ERR_NOT_FOUND:
            printf("Unrecognized command\n");
            break;

        case ESP_ERR_INVALID_ARG:
            // command was empty
            printf("Command was empty or invalid argument of execution\n");
            break;

        case ESP_OK:
            if (ret != ESP_OK)
		printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
            break;

        default:
            // if err != ESP_OK
            printf("Internal error: %s\n", esp_err_to_name(err));
	}; /* switch err */

        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }

    ESP_LOGE(TAG, "Error or end-of-input, terminating console");
    esp_console_deinit();
}; /* app_main */
