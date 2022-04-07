/*
 * SD-card manipulations command definition
 * Implementation file
 * 	File: cmd_sdcard.cpp
 *	Author:  aso (Solomatov A.A.)
 *	Created: 04.04.2022
 *	Version: 0.1
 */

#include <cstdlib>
#include <iostream>
#include <iomanip>

#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "cmd_sdcard.hpp"

//#include <stdio.h>

//using namespace idf;
using namespace std;


/*
 * Предлагаемые команды:
 *    -	sd - main command for manipulation with SD-caed
 *	+ m, mount  - mount sdcard;
 *	+ u, umount - unmount sdcard;
 *	+ ls, dir   - list of files in sdcard;
 *	+ cd	    - change dir;
 *	+ cat	    - print file to console
 *	+ type      - type text to cinsile and store it in the file optionally
 *	+ cp, copy  - copy file
 *	+ mv, move  - move & rename file
 */

static void register_sdcard_cmd(void);
static void register_sd_cmd(void);

// Register all SD-card commands
void register_sdcard_all(void)
{
    register_sdcard_cmd();
    register_sd_cmd();
}; /* register_sdcard_all */


extern "C" {
// Procedure of the 'sd' command
static int sdcard_cmd(int argc, char **argv)
{
    cout << "Run the command \"sdcard\'" << endl
	 << endl;
    cout << "argc is   : " << argc << endl;
    for (int i = 0; i < argc; i++)
	cout << "argv[" << i << "] is: " << argv[i] << endl;
    cout << endl
	 << " . . ." << endl
	 << " <<   <<" << endl
	 << " . . ." << endl
	 << endl;
    cout << "Command " << argv[0] << "' is not yet implemented now." << endl
	 << endl;
    return 0;
}; /* sd_cmd */
}


//static struct {
//    struct arg_str *key;
//    struct arg_str *type;
//    struct arg_str *value;
//    struct arg_end *end;
//} sd_args;

static struct {
    struct arg_str *subcommand;
    struct arg_str *option;
    struct arg_str *type;
    struct arg_end *end;
} sd_args;


class TConsoleCmd
{
public:
    TConsoleCmd(esp_console_cmd_func_t proc, const char cmd[], const char help[],
	    const char hint[] = NULL, void* args = NULL);

    const esp_console_cmd_t* get();

    void set(esp_console_cmd_func_t proc, const char cmd[], const char help[],
	    const char hint[] = NULL, void* args = NULL);
    void command(const char cmd[]);
    void help(const char help[]);
    void hint(const char hint[]);
    void procedure(esp_console_cmd_func_t proc);
    void argtable(void* args);

private:
    static esp_console_cmd_t console_cmd;
}; /* TConsoleCmd */


inline const esp_console_cmd_t*
TConsoleCmd::get() {
    return &console_cmd; };

void inline
TConsoleCmd::command(const char cmd[]) {
    TConsoleCmd::console_cmd.command = cmd;};

void inline
TConsoleCmd::help(const char hlp[]) {
    TConsoleCmd::console_cmd.help = hlp;};

void inline
TConsoleCmd::hint(const char hnt[]) {
    TConsoleCmd::console_cmd.hint = hnt;};

void inline
TConsoleCmd::procedure(esp_console_cmd_func_t proc) {
    TConsoleCmd::console_cmd.func = proc;};

void inline
TConsoleCmd::argtable(void* args) {
    TConsoleCmd::console_cmd.argtable = args;};


// Register command 'sdcard'
static void register_sdcard_cmd(void)
{
#if 01
    const esp_console_cmd_t cmd = {
        .command = "sdcard",
        .help = "SD card manipulating generic command",
//        .hint = "enter subcommand for Sd card operations",
        .hint = NULL,
        .func = &sdcard_cmd,
	.argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
#else
    TConsoleCmd Cmd(sdcard_cmd, "sdcard", "SD card manipulating generic command"/*,
	"enter subcommand for Sd card operations"*/);
    ESP_ERROR_CHECK(esp_console_cmd_register(Cmd.get()));
#endif
}; /* register_sdcard_cmd */


// Register command 'sd'
static void register_sd_cmd(void)
{
    const esp_console_cmd_t cmd = {
        .command = "sd",
        .help = "shortcut for 'sdcard' command",
//        .hint = "enter subcommand for Sd card operations",
        .hint = NULL,
        .func = &sdcard_cmd,
	.argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}; /* register_sd_cmd */



TConsoleCmd::TConsoleCmd(esp_console_cmd_func_t proc, const char cmd[],
	const char help[], const char hint[], void* args) {
    set(proc, cmd, help, hint, args); }

void TConsoleCmd::set(esp_console_cmd_func_t proc, const char cmd[],
	const char hlp[], const char hnt[], void* args)
{
    command(cmd);
    help(hlp);
    hint(hnt);
    procedure(proc);
    argtable(args);
}; /* TConsoleCmd::set */




#if 0

/********** Content of the file sd_card_example.nain.c **********/

/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// This example uses SDMMC peripheral to communicate with SD card.

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"


void app_main(void)
{
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.

    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // To use 1-line SD mode, change this to 1:
    slot_config.width = 4;

    // On chips where the GPIOs used for SD card can be configured, set them in
    // the slot_config structure:
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = GPIO_NUM_14;
    slot_config.cmd = GPIO_NUM_15;
    slot_config.d0 = GPIO_NUM_2;
    slot_config.d1 = GPIO_NUM_4;
    slot_config.d2 = GPIO_NUM_12;
    slot_config.d3 = GPIO_NUM_13;
#endif

    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files:

    // First create a file.
    const char *file_hello = MOUNT_POINT"/hello.txt";

    ESP_LOGI(TAG, "Opening file %s", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    const char *file_foo = MOUNT_POINT"/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", file_foo);
    f = fopen(file_foo, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SDMMC peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
}

#endif

//--[ cmd_sdcard.cpp ]-----------------------------------------------------------------------------
