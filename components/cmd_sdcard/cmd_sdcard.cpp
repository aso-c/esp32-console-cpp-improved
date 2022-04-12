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
static int sdcard_cmd(int argc, char **argv)
//{
//    cout << "Run the command \"sdcard\'" << endl
//	 << endl;
//    cout << "argc is   : " << argc << endl;
//    for (int i = 0; i < argc; i++)
//	cout << "argv[" << i << "] is: " << argv[i] << endl;
//    cout << endl
//	 << " . . ." << endl
//	 << " <<   <<" << endl
//	 << " . . ." << endl
//	 << endl;
//    cout << "Command " << argv[0] << "' is not yet implemented now." << endl
//	 << endl;
//    return 0;
/*}*/; /* sd_cmd */
}


// argument tables for any subcommand of sdcard commqand
static void
    *args_help[1+1],	// h | help
    *args_mount[3+1],	// m | mount [<device>] [<mountpoint>]
    *args_umount[2+1],	// u | umount [ <device> | <mountpoint> ]
    *args_ls[2+1],	// ls | dir [<pattern>]
    *args_cat[2+1],	// cat <filename>
    *args_type[2+1];	// type [filename]


// Register all SD-card commands
void register_sdcard_cmd(void)
{

#define _MULTISYNTAX_
#ifndef _MULTISYNTAX_
#if 0
	static void *args[] = {
		arg_str0("m", "mount", "[<device>|<mountpoint>]", "mount SD card, optionally the device name or mount point path can by specified; if omitted - used default value..."),
//		arg_rex1(NULL, NULL, "m|mount", NULL, 0/*REG_ICASE*/, "mount SD-card <device> to <mountpoint>, parameters is optionally"),
//		arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ..."),
//		arg_str0(NULL, NULL, "<path>", "mountpoint path for SD card, if omitted - use ..."),
		arg_str0("u", "umount", "[<device>|<mountpoint>]", "unmount SD card, optionally the device name or mount point path can by specified; if omitted - used default value..."),
//		arg_rex1(NULL, NULL, "u|umount", NULL, 0/*REG_ICASE*/, "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."),
//		arg_str0(NULL, NULL, "<device>|<path>", "SD card device or mountpoint"),
		arg_str0("l", "ls,dir", "[<pattern>]", "list directory contents on SD-card; if omitted - used current dir"),
//		arg_rex1(NULL, NULL, "ls", NULL, 0, "list directory contents on SD-card"),
//		arg_str0(NULL, NULL, "<pattern>", "pattern or path in SD-card of the listed files in directory"),
		arg_str0("c", "cat", "<file name>", "print file to stdout (console output)"),
//		arg_rex1(NULL, NULL, "cat", NULL, 0, "print file to stdout (console output)"),
//		arg_str1(NULL, NULL, "<file name>", "print file content to stdout"),
		arg_str0("t", "type", "[<file name>]", "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only"),
//		arg_rex1(NULL, NULL, "type", NULL, 0, "type from the keyboard to file & screen or screen only, if file name is omitted"),
//		arg_str0(NULL, NULL, "<file name>", "file name to be printed"),
		arg_rem(NULL, "\nNote! Use only one options/command at one time!"),
		arg_end(2),
	};
#else
	static void *args[] = {
//		arg_str0("m", "mount", "[<device>|<mountpoint>]", "mount SD card, optionally the device name or mount point path can by specified; if omitted - used default value..."),
		arg_rex1(NULL, NULL, "m|mount", NULL, 0/*REG_ICASE*/, "mount SD-card <device> to <mountpoint>, parameters is optionally"),
		arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ..."),
		arg_str0(NULL, NULL, "<path>", "mountpoint path for SD card, if omitted - use ..."),
		arg_rem("\nsdcard  u|umount", "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."),
//		arg_rex1(NULL, NULL, "u|umount", NULL, 0/*REG_ICASE*/, "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."),
		arg_str0(NULL, NULL, "[<device>|<path>]", "SD card device or mountpoint"),
//		arg_rex1(NULL, NULL, "ls", NULL, 0, "list directory contents on SD-card"),
		arg_rem("\nsdcard  l|ls", "list directory contents on SD-card"),
		arg_str0(NULL, NULL, "[<pattern>]", "pattern or path in SD-card of the listed files in directory"),
//		arg_str0("c", "cat", "<file name>", "print file to stdout (console output)"),
		arg_rem("\nsdcard  c|cat", "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."),
		print file to stdout (console output)
		arg_str0(NULL, NULL, "<pattern>", "pattern or path in SD-card of the listed files in directory"),
//		arg_rem(NULL, "\nNote! Use only one options/command at one time!"),
		arg_end(2),
	};
#endif

#else

	static void *args[] = {
		arg_rex1(NULL, NULL, "h|help", "h | help", 0/*REG_ICASE*/, "help for command 'sdcard'"),
		arg_rem ("|", NULL),
		arg_rex1(NULL, NULL, "<subcommand>", NULL, 0/*REG_ICASE*/, "other subcommand of command 'sdcard'"),
		arg_strn(NULL, NULL, "<options>", 0, 2, "subcommand options"),
		arg_end(2),
	};

    // syntax0: h | help
    args_help[0] = arg_rex1(NULL, NULL, "h|help", "h | help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'");
    args_help[1] = arg_end(2);
    // syntax1: m | mount [<device>] [<mountpoint>]
    args_mount[0] = arg_rex1(NULL, NULL, "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters is optionally"),
    args_mount[3] = arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ...");
    args_mount[2] = arg_str0(NULL, NULL, "<mountpoint>", "mountpoint path for SD card, if omitted - use ...");
    args_mount[3] = arg_end(2);
    // syntax2: u | umount [ <device> | <mountpoint> ]
    args_umount[0] = arg_rex1(NULL, NULL, "u|umount", NULL, 0, "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ...");
    args_umount[1] = arg_str0(NULL, NULL, "[<device>|<path>]", "SD card device or mountpoint, if omitted - use defaul value"),
    args_umount[2] = arg_end(2);
    // syntax3: ls | dir [<pattern>]
    args_ls[0] = arg_rex1(NULL, NULL, "ls|dir", NULL, 0, "list directory contents on SD-card");
    args_ls[1] = arg_str0(NULL, NULL, "[<pattern>]", "pattern or path in SD-card of the listed files in directory");
    args_ls[2] = arg_end(2);
    // syntax4: cat <filename>
    args_cat[0] = arg_rex1(NULL, NULL, "cat", NULL, 0, "print file to stdout (console output)");
    args_cat[1] = arg_str1(NULL, NULL, "<file name>", "file name to be printed");
    args_cat[2] = arg_end(2);
    // syntax5: type [filename]
    args_type[0] = arg_rex1(NULL, NULL, "type", NULL, 0, "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only");
    args_type[1] = args_cat[1] = arg_str0(NULL, NULL, "<file name>", "the name of the file where the typed text is saved");
    args_type[2] = arg_end(2);

#endif

	const esp_console_cmd_t cmd = {
	    .command = "sdcard",
	    .help = "SD card manipulating main command",
    //        .hint = "enter subcommand for Sd card operations",
	    .hint = NULL,
	    .func = &sdcard_cmd,
	    .argtable = &args
	};
    register_cmd(&cmd);



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

//const esp_console_cmd_t cmd = {
//    .command = "sdcard",
//    .help = "SD card manipulating main command",
////        .hint = "enter subcommand for Sd card operations",
//    .hint = NULL,
//    .func = &sdcard_cmd,
//    .argtable = &args
//};


// Register command procedure
static void register_cmd(const esp_console_cmd_t* cmd)
{
    ESP_ERROR_CHECK(esp_console_cmd_register(cmd));
}; /* register_cmd */


// sucommand id
enum SD_Subcommands_id {
    sd_none,
    sd_mount,
    sd_unmount,
    sd_ls,
    sd_cat,
    sd_type,
    sd_help,
    sd_unknown = -1
}; /* sd_subcommands_id */

// Test for empty string or NULL pointer
bool isempty(const char *str)
{
    if (str == NULL || !str[0])
	return true;
    for (int i = 0; i < strlen(str); i++)
	if (!isspace(str[i]))
	    return false;
    return true;
}; /* isempty */

// Convert subcommand string to subcommand_id
SD_Subcommands_id
string2subcommand(char subcmd_str[])
{
//    if (subcmd_str == NULL || !subcmd_str[0])
//	return sd_none;
    if (isempty(subcmd_str))
	return sd_none;
    if (strcmp(subcmd_str, "help") == 0 || strcmp(subcmd_str, "h") == 0)
	return sd_help;
    if (strcmp(subcmd_str, "mount") == 0 || strcmp(subcmd_str, "m") == 0)
	return sd_mount;
    if (strcmp(subcmd_str, "umount") == 0 || strcmp(subcmd_str, "u") == 0)
	return sd_unmount;
    if (strcmp(subcmd_str, "ls") == 0 || strcmp(subcmd_str, "dir") == 0)
	return sd_ls;
    if (strcmp(subcmd_str, "cat") == 0)
	return sd_ls;
    if (strcmp(subcmd_str, "type") == 0)
	return sd_type;

    return sd_unknown;
}; /* string2subcommand */


/* help_action implements the actions for syntax 0 */
int help_action(int help, const char *progname, void *argtable0[], void *argtable1[],
	void *argtable2[], void *argtable3[], void *argtable4[], void *argtable5[]);


//extern "C" {
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
//    cout << "Command " << argv[0] << "' is not yet implemented now." << endl
    if (argc == 1)
	return help_action(0, argv[0], args_help, args_mount, args_umount, args_ls, args_cat, args_type);

    switch (string2subcommand(argv[1]))
    {
    case sd_mount:
    case sd_unmount:
    case sd_ls:
    case sd_cat:
    case sd_type:
	cout << "Command " << argv[0] << "' is not yet implemented now." << endl;
	break;

    case sd_help:
	return help_action(0, argv[0], args_help, args_mount, args_umount, args_ls, args_cat, args_type);

    case sd_unknown:
    default:
	return help_action(1, argv[0], args_help, args_mount, args_umount, args_ls, args_cat, args_type);
    }; /* switch string2subcommand(argv[1]) */

    cout << endl;
    return 0;
}; /* sd_cmd */
//}


/* help_action implements the actions for syntax 0 */
int help_action(int help, const char *progname, void *argtable0[], void *argtable1[],
	void *argtable2[], void *argtable3[], void *argtable4[], void *argtable5[])
{
    /* help subcommand */
    if (help)
    {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout,argtable1,"\n");
        printf("       %s", progname);
        arg_print_syntax(stdout,argtable2,"\n");
        printf("       %s", progname);
        arg_print_syntax(stdout,argtable3,"\n");
        printf("       %s", progname);
        arg_print_syntax(stdout,argtable4,"\n");
        printf("       %s", progname);
        arg_print_syntax(stdout,argtable5,"\n");
        printf("       %s", progname);
        arg_print_syntax(stdout,argtable0,"\n");
        printf("This program demonstrates the use of the argtable2 library\n");
        printf("for parsing multiple command line syntaxes.\n");
        arg_print_glossary(stdout,argtable1,"      %-20s %s\n");
        arg_print_glossary(stdout,argtable2,"      %-20s %s\n");
        arg_print_glossary(stdout,argtable3,"      %-20s %s\n");
        arg_print_glossary(stdout,argtable4,"      %-20s %s\n");
        arg_print_glossary(stdout,argtable5,"      %-20s %s\n");
        arg_print_glossary(stdout,argtable0,"      %-20s %s\n");
        return 0;
    }; /* help */

#if 0
    /* --version option */
    if (version)
    {
        printf("'%s' example program for the \"argtable\" command line argument parser.\n",progname);
        return 0;
    }; /* version */
#endif

    /* no command line options at all */
    printf("Try '%s help' for more information.\n", progname);
    return 0;
}; /*  */



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
