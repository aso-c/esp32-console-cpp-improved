/*!
 * @file console.cpp
 *
 * @brief Improved Console project
 *
 * @detail Improve the Advanced Console ESP Example project for using any extended features
 * and dicover & testing additional modules provided the ESP-IDF SDK.
 * Main project file.
 *
 * @section LICENCE
 *
 * This code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *
 * @author: Solomatov A.A. (aso)
 * @version 2.2.12.2
 * @date Created on: 26 янв. 2022 г.
 *	Updated 30.05.2025
 */


#include <cstdlib>
#include <iostream>

#include <ranges>
#include <string>
#include <tuple>
#include <cstdint>


#include <esp_system.h>
#include <esp_log.h>
#include <esp_console.h>
//#include "esp_vfs_dev.h"
//#include "driver/uart.h"
//-//#include "driver/uart_vfs.h"
#include <linenoise/linenoise.h>
#include <argtable3/argtable3.h>
#include <esp_vfs_fat.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <soc/soc_caps.h>	// --?
#include "cmd_system.h"	// --?
#include "cmd_wifi.h"	// --?
#include "cmd_nvs.h"	// --?
#include "console_init"
#include "cmd_decl.h"

#include <argtable>
#include <console>

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


#pragma message("ESP32 Improved Console" ", version v." CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE " by " CONFIG_APP_PROJECT_AUTHOR " (" CONFIG_APP_PROJECT_AUTHOR_NICK ")")
#pragma message("C++ version is: " STRING(__cplusplus) )

static const char TAG[] = "improved console";
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
}; /* initialize_nvs() */


///--[ Registering main infrastructure command - help & info ]---------------------------

/**
 * @brief Namespace of the fake command only for output version information in a 'help' command
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

    }; /* class info::act */


    /// @brief Info pseudo-command procedure about a version information of a project
    esp_err_t act::invoke(int argc, char * argv[])
    {
        cout << "ESP Console Example Project, Version: " CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE
    	    << ", builded with C++ version " << __cplusplus  << endl;
        return ESP_OK;
    }; /* info::act::invoke() */

    const esp::console::cmd_t<act> cmd("info",  version_str(), "about this project");

}; /* namespace info */



/**
 * @brief Namespace of help command
 */
namespace help
{
    arg::table::syntax syntax{ arg_str0(NULL, NULL, "<string>", "Name of command"),
				arg_intn("v", "verbose", "<0|1>", 0, 1,
						"If specified, list console commands with given verbose level"), };

    /// definition of the act for the 'info' pseudo-command
    struct act: public arg::table::act_t<syntax>
    {
	/**
	 * @brief 'help' command - now is emulate envelope over the standard help command
	 *
	 * Printout help about commands of this project
	 *
	 * @return
	 *      - ESP_OK on success
	 *      - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
	 */
	static
	esp_err_t invoke(int argc, char * argv[]);

    }; /* class bt::act */


    /// @brief Stub for the 'Help' command - now will is not used
    esp_err_t act::invoke(int argc, char * argv[])
    {
//        cout << "ESP Console Example Project, Version: " CONFIG_APP_PROJECT_VER "-" CONFIG_APP_PROJECT_FLAVOUR " of " CONFIG_APP_PROJECT_DATE
//    	    << ", builded with C++ version " << __cplusplus  << endl;
	return ESP_OK;
    }; /* help::act::invoke() */

    struct cmd_def: public esp::console::cmd_t<act>
    {
	cmd_def(const char name[], const char help_str[] = nullptr):
	    esp::console::cmd_t<act>(name, help_str)
	{};

	/**
	 * @brief Register a 'help' command
	 */
	esp_err_t enreg() const override;

    }; /* struct help::cmd_def */

    const cmd_def cmd("help",  "Print the summary of all registered commands if no arguments "
						"are given, otherwise print summary of given command.");
}; /* namespace help */

/**
 * @brief Register a 'help' command
 *
 * Default 'help' command prints the list of registered commands along with
 * hints and help strings if no additional argument is given. If an additional
 * argument is given, the help command will look for a command with the same
 * name and only print the hints and help strings of that command.
 *
 * Own 'help' command implementation first run default 'help' command,
 * and then prints the version string of a program.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_STATE, if esp_console_init wasn't called
 */
//esp_err_t console_register_help_command(void)
esp_err_t help::cmd_def::enreg() const
{
    //info::cmd.enreg_check();
//    return help::cmd.enreg();
    return (esp_err_t)esp_console_register_help_command();
//    override { return (esp_err_t)esp_console_register_help_command(); };
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
    std::string prompt = create_prompt(PROMPT_STR + ">"s);
//        const /*auto*/ std::string parts[] = {/*STRLIT(*/PROMPT_STR/*)*/, ">"s };
//    std::string prompt = simple_prompt_gen(parts | std::ranges::views::join | std::ranges::to<const std::string>());

    /* Register commands */
    //console_register_help_command();
    help::cmd.enreg_tst();
    register_system_common();
    register_system_sleep();
#if (CONFIG_ESP_WIFI_ENABLED || CONFIG_ESP_HOST_WIFI_ENABLED)
    register_wifi();
#endif
    register_nvs();
    register_fs_cmd_all();
    register_sdcard_cmd();
    register_bt_cmd();

    info::cmd.enreg_tst();

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
    if (linenoiseIsDumbMode())
    {
        cout << endl
	    << "Your terminal application does not support escape sequences." << endl
	    << "Line editing and history features are disabled." << endl
	    << "On Windows, try using Putty instead." << endl;
    }; /* if linenoiseIsDumbMode() */

    /* Main loop */
    while(true)
    {
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
    }; /* while(true) */

    ESP_LOGE(TAG, "Error or end-of-input, terminating console");
    esp_console_deinit();
}; /* app_main */
