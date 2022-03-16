/* Console example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <cstdlib>
#include <iostream>
//#include <thread>
//#include "esp_log.h"
#include "gpio_cxx.hpp"

//#define __WITH_STDIO__
//#define __WITH_BOOST__
#define __MAX_UNFOLDED_OUTPUT__

//#include <stdio.h>
//#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "cmd_decl.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"

using namespace idf;
using namespace std;

#ifdef CONFIG_ESP_CONSOLE_USB_CDC
#error This example is incompatible with USB CDC console. Please try "console_usb" example instead.
#endif // CONFIG_ESP_CONSOLE_USB_CDC

static const char* TAG = "example";
#define PROMPT_STR CONFIG_IDF_TARGET

/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_STORE_HISTORY

#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

static void initialize_filesystem(void)
{
    static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 4,
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
}
#endif // CONFIG_STORE_HISTORY

static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

static void initialize_console(void)
{
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
        .source_clk = UART_SCLK_REF_TICK,
#else
        .source_clk = UART_SCLK_XTAL,
#endif
    };
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_length = 256,
            .max_cmdline_args = 8,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
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

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

#if CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(HISTORY_PATH);
#endif
}



/**
 * @brief Info command about a version information of a project
 *
 * Printout version info $ small description,
 * about this project
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
 */

/* 'info' pseudo-command */
extern "C" {
static int get_info(int argc, char **argv)
{
#ifdef __WITH_STDIO__
//    printf("ESP Console Example Project, Version: %s of %s\r\n", CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
    printf("ESP Console Example Project, Version: %s-%s of %s\r\n", CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
#elif __WITH_BOOST__
    printf("ESP Console Example Project, Version: %s-%s of %s\r\n", CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR, CONFIG_APP_PROJECT_DATE);
#else
    cout << "ESP Console Example Project, Version: " CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE "\r\n";
#endif
    return ESP_OK;
}; /* get_info */
}; /* extern C */


/**
 * @brief Fake command only for output version information in a 'help' command
 *
 * Own 'help' command implementation first run default 'help' command,
 * and then prints the version string of a program.
 */

static void register_info(void)
{

	static struct {
	    struct arg_str *name_space;
	    struct arg_end *end;
	} info_args;


    info_args.name_space = arg_str1(NULL, NULL, "Build Date:", __DATE__ " " __TIME__ ".");
    info_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "info",
        .help = version_str(),
        .hint = "about this project",
        .func = &get_info,
	.argtable = &info_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}; /* register_info */



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
esp_err_t console_example_register_help_command(void)
{
    return (esp_err_t)esp_console_register_help_command();
}; /* console_example_register_help_command */


extern "C" void app_main(void)
{
    initialize_nvs();

#if CONFIG_STORE_HISTORY
    initialize_filesystem();
    ESP_LOGI(TAG, "Command history enabled");
#else
    ESP_LOGI(TAG, "Command history disabled");
#endif

    initialize_console();

    /* Register commands */
    //esp_console_register_help_command();
    console_example_register_help_command();
    register_system();
    register_wifi();
    register_nvs();
    register_info();

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char* prompt = LOG_COLOR_I PROMPT_STR "> " LOG_RESET_COLOR;

#ifdef __WITH_STDIO__
    printf("\n"
           "This is an example of ESP-IDF console component.\n"
	   "Version %s-%s of %s, modified by %s.\n"
  	   "Builded %s %s\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n"
	   "Press Enter or Ctrl+C will terminate the console environment.\n",
	   CONFIG_APP_PROJECT_VER, CONFIG_APP_PROJECT_FLAVOUR,
	   CONFIG_APP_PROJECT_DATE, CONFIG_APP_PROJECT_MODIFICATOR,
	   __DATE__, __TIME__);
#elif __WITH_BOOST__
    printf("\n"
           "This is an example of ESP-IDF console component.\n"
	   "%s\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n"
	   "Press Enter or Ctrl+C will terminate the console environment.\n",
	   version_str());
#else
    cout << endl
	<< "This is an example of ESP-IDF console component.\n"
	<< version_str() << endl
	<< "Type 'help' to get the list of commands.\n"
	<< "Use UP/DOWN arrows to navigate through command history.\n"
	<< "Press TAB when typing command name to auto-complete.\n"
	<< "Press Enter or Ctrl+C will terminate the console environment.\n";
#endif

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
#ifdef __WITH_STDIO__
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
#elif __WITH_BOOST__
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
#else
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
#endif

        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = PROMPT_STR "> ";
#endif //CONFIG_LOG_COLORS
    }

    /* Main loop */
    while(true) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Break on EOF or error */
            break;
        }
        /* Add the command to the history if not empty*/
        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
            /* Save command history to filesystem */
            linenoiseHistorySave(HISTORY_PATH);
#endif
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
