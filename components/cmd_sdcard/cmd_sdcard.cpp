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
#include <stdarg.h>

#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include <argtable3/argtable3.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex>
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
 *	+ m, mount	- mount sdcard, options: [<card>] [<mountpoint>];
 *	+ u, umount	- unmount sdcard, options: [<card>|<mountpiont>];
 *	+ ls, dir	- list of files in sdcard, options: [<file pattern>];
 *	+ cd <dir>	- change dir;
 *	+ cat <file>	- print file to console
 *	+ type [<file>]	- type text to cinsile and store it in the file optionally
 *	+ cp, copy	- copy file, options: [<src file>|<dest file>];
 *	+ mv, move	- move or rename file, options: [<src file>|<dest file>];
 */

// Register command procedure
static void register_cmd(const esp_console_cmd_t*);

extern "C" {
// Procedure of the 'sd' command
static int sdcard_cmd(int argc, char **argv);
}


// argument tables for any subcommand of sdcard commqand
//static void
//    *args_help[1+1],	// h | help
//    *args_mount[3+1],	// m | mount [<device>] [<mountpoint>]
//    *args_umount[6+1],	// u | umount [ <device> | <mountpoint> ]
//    *args_ls[2+1],	// ls | dir [<pattern>]
//    *args_cat[2+1],	// cat <filename>
//    *args_type[4+1];	// type [<filename>]

static struct
{
    void *help[1+1],	// h | help
	*mount[3+1],	// m | mount [<device>] [<mountpoint>]
	*umount[6+1],	// u | umount [ <device> | <mountpoint> ]
	*ls[2+1],	// ls | dir [<pattern>]
	*cat[2+1],	// cat <filename>
	*type[4+1];	// type [<filename>]
} args;


// Register all SD-card commands
void register_sdcard_cmd(void)
{

	static void *args[] = {
		arg_rex1(NULL, NULL, "h|help", "h | help", 0/*REG_ICASE*/, "help for command 'sdcard'"),
		arg_rem ("|", NULL),
		arg_rex1(NULL, NULL, "<subcommand>", NULL, 0/*REG_ICASE*/, "other subcommand of command 'sdcard'"),
		arg_strn(NULL, NULL, "<options>", 0, 2, "subcommand options"),
		arg_end(2),
	};

	const esp_console_cmd_t cmd = {
	    .command = "sdcard",
	    .help = "SD card manipulating main command",
    //        .hint = "enter subcommand for Sd card operations",
	    .hint = NULL,
	    .func = &sdcard_cmd,
	    .argtable = &args
	};
    register_cmd(&cmd);


    // syntax0: h | help
//    args_help[0] = arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'");
    ::args.help[0] = arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'");
//    args_help[1] = arg_end(2);
    ::args.help[1] = arg_end(2);
    // syntax1: m | mount [<device>] [<mountpoint>] "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters are optional"
//    args_mount[0] = arg_rex1(NULL, NULL, "m|mount", NULL, 0, NULL);
    ::args.mount[0] = arg_rex1(NULL, NULL, "m|mount", NULL, 0, NULL);
//    args_mount[1] = arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ...");
    ::args.mount[1] = arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use default value");
//    args_mount[2] = arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use ...");
    ::args.mount[2] = arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use default value");
//    args_mount[3] = arg_end(2);
    ::args.mount[3] = arg_end(2);
    // syntax2: u | umount [ <device> | <mountpoint> ] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
//    args_umount[0] = arg_rex1(NULL, NULL, "u|umount", NULL, 0, NULL);
    ::args.umount[0] = arg_rex1(NULL, NULL, "u|umount", NULL, 0, NULL);
//    args_umount[1] = arg_rem ("[", NULL);
    ::args.umount[1] = arg_rem ("[", NULL);
//    args_umount[2] = args_mount[1]; // arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ...");
    ::args.umount[2] = ::args.mount[1]; // arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ...");
//    args_umount[3] = arg_rem ("|", NULL);
    ::args.umount[3] = arg_rem ("|", NULL);
//    args_umount[4] = args_mount[2]; // arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use ...");
    ::args.umount[4] = ::args.mount[2]; // arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use ...");
//    args_umount[5] = arg_rem ("]", NULL);
    ::args.umount[5] = arg_rem ("]", NULL);
//    args_umount[6] = arg_end(2);
    ::args.umount[6] = arg_end(2);
    // syntax3: ls | dir [<pattern>] "list directory contents on SD-card"
//    args_ls[0] = arg_rex1(NULL, NULL, "ls|dir", NULL, 0, NULL);
    ::args.ls[0] = arg_rex1(NULL, NULL, "ls|dir", NULL, 0, NULL);
//    args_ls[1] = arg_str0(NULL, NULL, "<pattern>", "pattern or path in SD-card of the listed files in directory");
    ::args.ls[1] = arg_str0(NULL, NULL, "<pattern>", "pattern or path in SD-card of the listed files in directory");
//    args_ls[2] = arg_end(2);
    ::args.ls[2] = arg_end(2);
    // syntax4: cat <filename> "print file to stdout (console output)"
//    args_cat[0] = arg_rex1(NULL, NULL, "cat", NULL, 0, NULL);
    ::args.cat[0] = arg_rex1(NULL, NULL, "cat", NULL, 0, NULL);
//    args_cat[1] = arg_str1(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved");
    ::args.cat[1] = arg_str1(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved");
//    args_cat[2] = arg_end(2);
    ::args.cat[2] = arg_end(2);
    // syntax5: type [filename] "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only"
//    args_type[0] = arg_rex1(NULL, NULL, "type", NULL, 0, NULL);
    ::args.type[0] = arg_rex1(NULL, NULL, "type", NULL, 0, NULL);
//    args_type[1] = args_umount[1];  // arg_rem ("[", NULL);
    ::args.type[1] = ::args.umount[1];  // arg_rem ("[", NULL);
//    args_type[2] = args_cat[1];
    ::args.type[2] = ::args.cat[1];
//    args_type[3] = args_umount[5];  // arg_rem ("]", NULL);
    ::args.type[3] = ::args.umount[5];  // arg_rem ("]", NULL);
//    args_type[4] = arg_end(2);
    ::args.type[4] = arg_end(2);


    const esp_console_cmd_t cmd2 = {
        .command = "sd",
        .help = "shortcut for 'sdcard' command",
//        .hint = "enter subcommand for Sd card operations",
        .hint = NULL,
        .func = &sdcard_cmd,
	.argtable = NULL
    };
    register_cmd(&cmd2);

}; /* register_sdcard_all */


// Register command procedure
static void register_cmd(const esp_console_cmd_t* cmd)
{
    ESP_ERROR_CHECK(esp_console_cmd_register(cmd));
}; /* register_cmd */




// Check if a string is empty
bool isempty(const char *str)
{
    if (str == NULL || !str[0])
	return true;
    for (int i = 0; i < strlen(str); i++)
	if (!isspace(str[i]))
	    return false;
    return true;
}; /* isempty */




class SDcmd
{
public:

    void Init(int argc, char *argv[]);	// Initializing current command environment

    // sucommand id
    enum cmd_id {
        none,
        mount,
        unmount,
        ls,
        cat,
        type,
        help,
        unknown = -1
    }; /* cmd_id */

//    // Convert subcommand string to id
//    static cmd_id
//    id(char subcmd_str[]);
    // Return subcommand id
    cmd_id id();

private:
    int argc;
    char **argv;

}; /* SDcmd */


static SDcmd sdcommand;

// help_action implements the actions for syntax 0
// Multisyntax!
// Call variants:
// help_actions(int act = 0, const char cmdname[]) - call, if omittes cmd options
// help_actions(int act = -1, const char cmdname, char option[]) - call, if cmd option is unknown
// help_action(int act = 1, const char cmdname[], int argcnt, void *argtable1[][, void* argtable2[]]...)
int help_action(int act, const char cmdname[], ...);


//extern "C" {
// Procedure of the 'sd' command
static int sdcard_cmd(int argc, char **argv)
{
    cout << "Run the command \"sdcard\'" << endl
	 << endl;
    cout << "argc is   : " << argc << endl;
    for (int i = 0; i < argc; i++)
	cout << "argv[" << i << "] is: " << argv[i] << endl;
    cout << "..............................................." /*<< endl*/
	 << endl;

    sdcommand.Init(argc, argv);

    if (argc == 1)
	return help_action(0, argv[0]);

//    switch (SDcmd::id(argv[1]))
    switch (sdcommand.id())
    {
    case SDcmd::mount:
    case SDcmd::unmount:
    case SDcmd::ls:
    case SDcmd::cat:
    case SDcmd::type:
	cout << "Command" << '\'' << argv[0] << ' ' << argv[1] << '\''
	    << " is not yet implemented now." << endl;
	break;

    case SDcmd::help:
	return help_action(1, argv[0], 6, args.help, args.mount, args.umount, args.ls, args.cat, args.type);

    case SDcmd::unknown:
    default:
	return help_action(-1, argv[0], argv[1]);
    }; /* switch string2subcommand(argv[1]) */

    cout << endl;
    return 0;
}; /* sd_cmd */
//}





//--[ class SDcmd ]-------------------------------------------------------------



// Initializing current command environment
void SDcmd::Init(int argcnt, char *argvalue[])
{
    argc = argcnt;
    argv = argvalue;
}; /* SDcmd::Init */

// help_action implements the actions for syntax 0
// Multisyntax!
// Call variants:
// help_actions(int act = 0, const char cmdname[]) - call, if omittes cmd options
// help_actions(int act = -1, const char cmdname, char option[]) - call, if cmd option is unknown
// help_action(int act = 1, const char cmdname[], int argcnt, void *argtable1[][, void* argtable2[]]...)
int help_action(int act, const char cmdname[], ...)
{

	va_list arglst;

    va_start(arglst, cmdname);

    /* help subcommand */
    if (act == 1)
    {
	    int argcnt = va_arg(arglst, int);

	printf("Usage: %s", cmdname);
	arg_print_syntax(stdout, va_arg(arglst, void**), "\n");
	argcnt--;
	for (int i = 0; i < argcnt; i++)
	{
	    printf("       %s", cmdname);
	    arg_print_syntax(stdout, va_arg(arglst, void**), "\n");
	}; /* for int i = 0; i < argcnt; i++ */
	va_end(arglst);

//	printf("This program demonstrates the use of the argtable2 library\n");
	printf("The command '%s' is provide operating with SD-card in ESP32\n", cmdname);
	printf("for parsing multiple command line syntaxes.\n");

	va_start(arglst, cmdname);  // reset arglist pointer
        va_arg(arglst, int);	// drop unneded first variadic parameter from the list
        for (int i = 0; i < argcnt; i++)
	    arg_print_glossary(stdout, va_arg(arglst, void**), "      %-20s %s\n");
        va_end(arglst);
        return 0;
    }; /* help */

#if 0
    /* --version option */
    if (version)
    {
        printf("'%s' example program for the \"argtable\" command line argument parser.\n",cmdname);
        return 0;
    }; /* version */
#endif

    switch (act)
    {
    case 0:
	printf("Options is absent.\n");
	break;
    case -1:
    default:
	printf ("Unknown options: \"%s\".\n", va_arg(arglst, char*));
    }; /* switch act */
    /* no command line options at all */
    va_end(arglst);
    printf("Try '%s help' for more information.\n", cmdname);
    va_end(arglst);
    return 0;
}; /* help_action */


#if 0
// Convert subcommand string to cmd_id
SDcmd::cmd_id
SDcmd::id(char subcmd_str[])
{
//    if (isempty(subcmd_str))
    if (isempty(subcmd_str))
	return none;
    if (strcmp(subcmd_str, "help") == 0 || strcmp(subcmd_str, "h") == 0)
    	return help;
    if (strcmp(subcmd_str, "mount") == 0 || strcmp(subcmd_str, "m") == 0)
    	return mount;
    if (strcmp(subcmd_str, "umount") == 0 || strcmp(subcmd_str, "u") == 0)
	return unmount;
    if (strcmp(subcmd_str, "ls") == 0 || strcmp(subcmd_str, "dir") == 0)
	return ls;
    if (strcmp(subcmd_str, "cat") == 0)
    	return ls;
    if (strcmp(subcmd_str, "type") == 0)
    	return type;

    return unknown;
}; /* SDcommand::id */
#else
// Return command id
SDcmd::cmd_id
SDcmd::id()
{
    if (isempty(argv[1]))
	return none;
    if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "h") == 0)
    	return help;
    if (strcmp(argv[1], "mount") == 0 || strcmp(argv[1], "m") == 0)
    	return mount;
    if (strcmp(argv[1], "umount") == 0 || strcmp(argv[1], "u") == 0)
	return unmount;
    if (strcmp(argv[1], "ls") == 0 || strcmp(argv[1], "dir") == 0)
	return ls;
    if (strcmp(argv[1], "cat") == 0)
    	return ls;
    if (strcmp(argv[1], "type") == 0)
    	return type;

    return unknown;
}; /* SDcommand::id */
#endif





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
