/*
 * SD-card control classes
 * Implementation file
 * 	File: sdcard_ctrl.cpp
 *	Author:  aso (Solomatov A.A.)
 *	Created: 14.07.2022
 *	Version: 0.6
 */

#define __PURE_C__


#include <cstdlib>
#include <iostream>
#include <iomanip>
//#include <stdarg.h>
#include <cstdarg>

//#include <string.h>
#include <cstring>
#include <sys/unistd.h>
#include <cerrno>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include <argtable3/argtable3.h>
//#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex>
#ifdef __PURE_C__
//#include <fcntl.h>
#include <dirent.h>
#else
#if __cplusplus < 201703L
#include <fcntl.h>
#include <dirent.h>
#else
#endif // __cplusplus < 201703L
#endif // ifdef __PURE_C__

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "sdcard_ctrl.hpp"

//#include <stdio.h>
//#include <cstdio>

//using namespace idf;
using namespace std;


#define MOUNT_POINT_def "/sdcard"


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

namespace SDMMC
{  //------------------------------------------------------------------------------------------------------------------

//--[ strust Host ]-------------------------------------------------------------------------------------------------

//int Control::slot_default_no;


//--[ strust Slot ]-------------------------------------------------------------------------------------------------

int Slot::def_num;


//--[ struct Mounting ]---------------------------------------------------------------------------------------------

    const char *Mounting::MOUNT_POINT_Default = MOUNT_POINT_def;


//--[ class Server ]------------------------------------------------------------------------------------------------

    // Mount SD-card with default parameters
    esp_err_t Server::mount()
    {
	cout << "Mount SD-card with default parameters" << endl;
	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"." << endl
		<< endl;
//	cout << TAG << ": " << "Procedure \"Mount\" is not yet released now" << endl;
//	cout  << endl;
	return mount(control.slot.default_num(), mounting.MOUNT_POINT_Default);
    }; /* Server::mount */

    // Mount default SD-card slot onto path "mountpoint"
    esp_err_t Server::mount(char mountpoint[])
    {
	if (isdigit(mountpoint[0]))
	    return mount(atoi(mountpoint));

	cout << "Mount default SD-card slot onto specified path" << endl;
	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
		<< endl;
//	cout << TAG << ": " << "Procedure \"Mount(<mountpath>)\" is not yet released now" << endl;
//	cout << endl;
	return mount(control.slot.default_num(), mountpoint);
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto default mount path
    esp_err_t Server::mount(int slot_no)
    {
//	cout << "Mount SD-card in specified slot onto default mount path" << endl;
//	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
//		<< endl;
	cout << TAG << ": " << "Procedure \"Mount(<slot_number>)\" is not yet released now" << endl;
	cout /*<< "Exit..."*/ << endl;
	return mount(slot_no, mounting.MOUNT_POINT_Default);
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto specified mount path
    esp_err_t Server::mount(int slot_no, const char mountpoint[])
    {
	mounting.target = mountpoint;
	control.host.slot = slot_no;

	// Enable internal pullups on enabled pins. The internal pullups
	// are insufficient however, please make sure 10k external pullups are
	// connected on the bus. This is for debug / example purpose only.
	// slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
	//ESP_LOGI(TAG, "Mounting filesystem");

	cout << "Mount SD-card in specified slot onto specified mount path" << endl;
//	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
//		<< endl;
	cout << TAG << ": " "Mounting filesystem in card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
		<< endl;
	ret = esp_vfs_fat_sdmmc_mount(mounting.target, &control.host, &control.slot.cfg, &mounting.cfg, &card);
	if (ret != ESP_OK)
	{
	    if (ret == ESP_FAIL)
		cout << TAG << ": " << "Failed to mount filesystem. "
			"If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.";
	    else
		cout << TAG << ": " << "Failed to initialize the card (error " << ret << ", " << esp_err_to_name(ret) << "). "
			<< "Make sure SD card lines have pull-up resistors in place.";
	    return ret;
	}; /* if ret != ESP_OK */

	//ESP_LOGI(TAG, "Filesystem mounted");
	cout << TAG << ": " "Filesystem mounted";

	return ret;
    }; /* Server::mount */



    //------------------------------------------------------------------------------------------
    //    // All done, unmount partition and disable SDMMC peripheral
    //    esp_vfs_fat_sdcard_unmount(mount_point, card);
    //    ESP_LOGI(TAG, "Card unmounted");
    //------------------------------------------------------------------------------------------

    // Unmount default mounted SD-card
    esp_err_t Server::unmount()
    {
//	return ESP_ERR_NOT_SUPPORTED;
	cout << TAG << ": " << "Call: unmount(" << mounting.target << ");" << endl;
	ret = unmount(mounting.target);
	return ret;
    }; /* Server::unmount */

    // Unmount SD-card, that mounted onto "mountpath"
    esp_err_t Server::unmount(const char mountpath[])
    {
	ret = esp_vfs_fat_sdcard_unmount(mountpath, card);
	//ESP_LOGI(TAG, "Card unmounted");
	if (ret == ESP_OK)
	    cout << TAG << ": " << "Card unmounted" << endl;
	else
	    cout << TAG << ": "  << "Error: " << ret
		<< ", " << esp_err_to_name(ret) << endl;
	return ret;
    }; /* Server::unmount */

//------------------------------------------------------------------------------------------
//    // All done, unmount partition and disable SDMMC peripheral
//    esp_vfs_fat_sdcard_unmount(mount_point, card);
//    ESP_LOGI(TAG, "Card unmounted");
//------------------------------------------------------------------------------------------

//    // Unmount SD-card "card", mounted onto default mountpath
//    esp_err_t Server::unmount(sdmmc_card_t *card)
//    {
//	cout << TAG << ": " << "Procedure \"Unmount(<card>)\" is not yet released now" << endl;
//	cout << "Exit..." << endl;
//	return ESP_ERR_INVALID_VERSION;
//    }; /* Server::unmount */
//
//    // Unmount mounted SD-card "card", mounted onto mountpath
//    esp_err_t Server::unmount(const char *base_path, sdmmc_card_t *card)
//    {
//	cout << TAG << ": " << "Procedure \"Unmount(<mountpath, ><card>)\" is not yet released now" << endl;
//	cout << "Exit..." << endl;
//	return ESP_ERR_INVALID_VERSION;
//    }; /* Server::unmount */


// Print the card info
void Server::card_info(FILE* outfile)
{
    sdmmc_card_print_info(outfile, card);
}; /* Server::card_info */


// print the SD-card info (wrapper for the external caller)
esp_err_t Server::info()
{
    //cout << "Command \"SD-card info\" is not yet implemented now." << endl;
    card_info(stdout);
    return ESP_OK;
}; /* Server::info */


// print current directory name
esp_err_t Server::pwd()
{
#ifdef __PURE_C__
	char* buf = getcwd(NULL, 0);

    if (!buf)
	return errno;
    cout << endl
	<< "PWD is: \"" << buf << '"' << endl
	<< endl;
    free(buf);
    return ESP_OK;
#else
    cout << "Command \"pwd\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::pwd */


// change a current directory - error handler
esp_err_t Server::cd()
{
    ESP_LOGE("Console::cd", "invoke command \"%s\" without parameters.\n%s", "cd",
	     "This command required directory name to change.");

    return ESP_ERR_INVALID_ARG;
}; /* Server::cd */

// change a current directory
esp_err_t Server::cd(const char dirname[])
{
#ifdef __PURE_C__
    cout << "Change current dir to " << dirname << endl;
    chdir(dirname);
    if (errno != 0)
    {
	ESP_LOGE("Console::cd", "fail change directory to %s", dirname);
	perror("Console::cd");
	return ESP_FAIL;
    }; /* if errno != 0 */
    return ESP_OK;
#else
    cout << "Change directory to " << '"' << dirname << '"' << endl;
    cout << "Command \"cd\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::cd */


// print a list of files in the current directory
esp_err_t Server::ls()
{
    return ls(".");
}; /* Server::ls */

// print a list of files in the specified directory
esp_err_t Server::ls(const char pattern[])
{
    cout << "List files in " << '"' << pattern << '"' << endl;
#if defined(__PURE_C__) || __cplusplus < 201703L
	DIR *dir;	// Directory descriptor

    dir = opendir(pattern);
    if (!dir) {
	ESP_LOGE("Console::ls", "Error opening directory %s", pattern);
	perror("Console::ls");
	return ESP_FAIL;
    }; /* if !dir */

	esp_err_t ret = ESP_OK;

    for ( struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
	cout << "inode: " << entry->d_ino << ", name: " << entry->d_name
		<< "[type " << entry->d_type << "]; record length is: " /*<< entry->d_reclen*/ << endl;
    }; /* for entry = readdir(dir); entry != NULL; entry = readdir(dir) */
    if (errno != 0)
    {
	cerr << "Any error occured during reading of the current directory" << endl;
	perror("Console::ls");
	ret = ESP_FAIL;
    }; /* if errno != 0 */
    closedir(dir);
#else
#endif
    return ret;
}; /* Server::ls */


// remove files - error handler
esp_err_t Server::rm()
{
    ESP_LOGE("Console::rm", "invoke command \"%s\" without parameters.\n%s", "rm",
	     "Missing filename to remove.");
    return ESP_ERR_INVALID_ARG;
}; /* Server::rm */

// remove files according a pattern
esp_err_t Server::rm(const char pattern[])
{
    cout << "Delete file " << '"' << pattern << '"' << endl;
#ifdef __PURE_C__
    ESP_LOGW("Console::rm", "Command \"%s\" is not yet implemented now for C edition.", "rm");
    return ESP_ERR_INVALID_VERSION;
    //return ESP_OK;
#else
    ESP_LOGW("Console::rm", "Command \"%s\" is not yet implemented now for C++ edition.", "rm");
//    cout << "Command \"rm\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::rm */



// type file contents - error, file name is absent
esp_err_t Server::cat()
{
    cout << endl
	 << "*** Printing contents of the file <XXXX fname>. ***" << endl
	 << endl;
    ESP_LOGE("Console::cat", "invoke command \"%s\" without parameters.\n%s", "cat",
	     "Missing filename for print to output.");

    cout << "*** End of printing file XXXX. ** ******************" << endl;
    return ESP_ERR_INVALID_ARG;
}; /* cat */

// type file contents
esp_err_t Server::cat(const char fname[])
{
    cout << endl
	 << "*** Printing contents of the file <" << fname << ">. ***" << endl
	 << endl;
//    cout << TAG << ": " << "Procedure \"cat(" << fname << ")\" is not yet released now" << endl;
    ESP_LOGW("Console::cat", "Command \"%s %s \" is not yet implemented now", "cat", fname);
    cout << "Exit..." << endl;
    cout << endl
	 << "*** End of printing file " << fname << ". **************" << endl
	 << endl;
    return ESP_ERR_INVALID_VERSION;
}; /* cat */


// type text from keyboard to screen
esp_err_t Server::type()
{
    cout << endl
	 << "**** Type the text on keyboard to screen *****" << endl
//	 << "Press 'q' or <Enter> twice for exit..." << endl
	 << "Press <Enter> twice for exit..." << endl
	 << endl;

	char c = '\0', prevc;
    do {
	prevc = c;
	cin >> noskipws >> c;
	cout << c;
//	if (c == '\n')
//	    cout << "<LF>" << endl;
//	if (c == '\r')
//	    cout << "<CR>" << endl;
    } while (c != prevc || c != '\n');

    cout << endl << endl
	 << "**** End of typing the text on keyboard. *****" << endl
	 << endl;
    return ESP_OK;
}; /* type */

// type text from keyboard to file and to screen
esp_err_t Server::type(const char fname[])
{
    cout << endl
	 << "**** Type the text on keyboard to screen and file <" << fname << ">. ****" << endl
	 << endl;
//    cout << TAG << ": " << "Procedure \"cat(fname)\" is not yet released now" << endl;
    ESP_LOGW("Console::type", "Command \"%s %s \" is not yet implemented now", "type", fname);
    cout << "Exit..." << endl;
    cout << endl
	 << "**** End of typing the text on keyboard for the screen&file. ****" << endl
	 << endl;
    return ESP_ERR_INVALID_VERSION;
}; /* type */


    const char* Server::TAG = "SD/MMC service";

}; /* namespace SDMMC */  //-------------------------------------------------------------------------------------------



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

//--[ sdcard_ctrl.cpp ]----------------------------------------------------------------------------
