/*
 * SD-card control classes
 * Implementation file
 * 	File: sdcard_ctrl.cpp
 *	Author:  aso (Solomatov A.A.)
 *	Created: 28.04.2022
 *	Version: 0.1
 */

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <stdarg.h>

#include <string.h>
#include <sys/unistd.h>
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

#include "sdcard_ctrl.hpp"


//#include <stdio.h>

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

    const char *Mounting::MOUNT_POINT = MOUNT_POINT_def;


//--[ class Server ]------------------------------------------------------------------------------------------------

    // Mount SD-card with default parameters
    esp_err_t Server::mount()
    {
//	mounting.target = Mounting::MOUNT_POINT;
	mounting.target_reset();
//	control.slot_num_reset();
	control.host.slot = control.slot.default_num();
	cout << "Mount SD-card with default parameters" << endl;
	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"." << endl
		<< endl;
	cout << TAG << ": " << "Procedure \"Mount\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::mount */

    // Mount default SD-card slot onto path "mountpoint"
    esp_err_t Server::mount(char mountpoint[])
    {
	// if parameter - digit, than call mount(int)
	if (isdigit(mountpoint[0]))
	    return mount(atoi(mountpoint));

	mounting.target = mountpoint;
//	control.slot_num_reset();
	control.host.slot = control.slot.default_num();
	cout << "Mount default SD-card slot onto specified path" << endl;
	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
		<< endl;
	cout << TAG << ": " << "Procedure \"Mount(<mountpath>)\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto default mount path
    esp_err_t Server::mount(int slot_no)
    {
	mounting.target_reset();
	control.host.slot = slot_no;
	cout << "Mount SD-card in specified slot onto default mount path" << endl;
	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
		<< endl;
	cout << TAG << ": " << "Procedure \"Mount(<slot_number>)\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto specified mount path
    esp_err_t Server::mount(int slot_no, char mountpoint[])
    {
	mounting.target = mountpoint;
//	control.host.slot = slot_no;
	control.host.slot = control.slot.default_num();
	cout << "Mount SD-card in specified slot onto specified mount path" << endl;
	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
		<< endl;
	cout << TAG << ": " << "Procedure \"Mount(<slot_number>, <mountpath>)\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::mount */

    // Unmount default mounted SD-card
    esp_err_t Server::unmount()
    {
	cout << TAG << ": " << "Procedure \"Unmount()\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::unmount */

    // Unmount SD-card, that mounted onto "mountpath"
    esp_err_t Server::unmount(const char mountpath[])
    {
	cout << TAG << ": " << "Procedure \"Unmount(<mountpath>)\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::unmount */

    // Unmount SD-card "card", mounted onto default mountpath
    esp_err_t Server::unmount(sdmmc_card_t *card)
    {
	cout << TAG << ": " << "Procedure \"Unmount(<card>)\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::unmount */

    // Unmount mounted SD-card "card", mounted onto mountpath
    esp_err_t Server::unmount(const char *base_path, sdmmc_card_t *card)
    {
	cout << TAG << ": " << "Procedure \"Unmount(<mountpath, ><card>)\" is not yet released now" << endl;
	cout << "Exit..." << endl;
	return ESP_ERR_INVALID_VERSION;
    }; /* Server::unmount */


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
