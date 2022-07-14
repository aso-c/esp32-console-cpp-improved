/*
 * SD-card control classes
 * Include file
 * 	File: sdcard_ctrl.cpp
 *	Author:  aso (Solomatov A.A.)
 *	Created: 28.04.2022
 *	Version: 0.1
 */


//using namespace idf;
//using namespace std;


#pragma once

/*
 * Предлагаемые команды:
 *    -	sd - main command for manipulation with SD-caed
 *	+ m, mount	- mount sdcard, options: [<card>] [<mountpoint>];
 *	+ u, umount	- unmount sdcard, options: [<card>|<mountpiont>];
 *	+ pwd		- get current directory name, w/o options;
 *	+ ls, dir	- list of files in sdcard, options: [<file pattern>];
 *	+ cd <dir>	- change dir;
 *	+ cat <file>	- print file to console
 *	+ type [<file>]	- type text to cinsile and store it in the file optionally
 *	+ cp, copy	- copy file, options: [<src file>|<dest file>];
 *	+ mv, move	- move or rename file, options: [<src file>|<dest file>];
 */


#ifdef __cplusplus


namespace SDMMC	//-----------------------------------------------------------------------------------------------------
{

static const char *TAG = "SD/MMC service";

struct Slot
{
    Slot()
    {
	    // To use 1-line SD mode, change this to 1:
	    cfg.width = SLOT_WIDTH;

	    // On chips where the GPIOs used for SD card can be configured, set them in
	    // the slot_config structure:
	#ifdef SOC_SDMMC_USE_GPIO_MATRIX
	    cfg.clk = GPIO_NUM_14;
	    cfg.cmd = GPIO_NUM_15;
	    cfg.d0 = GPIO_NUM_2;
	    cfg.d1 = GPIO_NUM_4;
	    cfg.d2 = GPIO_NUM_12;
	    cfg.d3 = GPIO_NUM_13;
	#endif

	    // Enable internal pullups on enabled pins. The internal pullups
	    // are insufficient however, please make sure 10k external pullups are
	    // connected on the bus. This is for debug / example purpose only.
	    cfg.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    }; /* Slot */

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t cfg = SDMMC_SLOT_CONFIG_DEFAULT();

    int  default_num() {return def_num;};

private:

    friend class Control;
    void default_num(int num) {def_num = num;};

    // To use 1-line SD mode, change this to 1:
    static const int SLOT_WIDTH = 4;
    static int def_num;

}; /* struct Slot */


//
// SD/MMC Host
struct Control
{
    Control()
    {
	//slot_default_no = host.slot;
	slot.default_num(host.slot);
	ESP_LOGI(TAG, "Using SDMMC peripheral");
    }; /* Control */

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    Slot slot;

private:
//    static int slot_default_no;
}; /* struct Control */




// Options for mounting the filesystem.
// If format_if_mount_failed is set to true, SD card will be partitioned and
// formatted in case when mounting fails.
struct Mounting
{

    Mounting():
	target(MOUNT_POINT_Default)
    {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
	cfg.format_if_mount_failed = true;
#else
	cfg.format_if_mount_failed = false;
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
	cfg.max_files = 5;
	cfg.allocation_unit_size = 16 * 1024;
	//ESP_LOGI(TAG, "Initializing SD card");
    }; /* Mount */

    void target_reset() {target = Mounting::MOUNT_POINT_Default;};

    esp_vfs_fat_sdmmc_mount_config_t cfg;
    const char* target;
    //ESP_LOGI(TAG, "Initializing SD card");

    static const char* MOUNT_POINT_Default;

}; /* struct Mountштп */

// const char *Mounting::MOUNT_POINT = MOUNT_POINT_def;


class Server
{
public:
    esp_err_t mount();				// Mount SD-card with default parameters
    esp_err_t mount(char mountpoint[]);		// Mount default SD-card slot onto path "mountpoint"
    esp_err_t mount(int slot_no);		// Mount SD-card slot "slot_no" onto default mount path
    esp_err_t mount(int slot_no, const char mountpoint[]);  // Mount SD-card slot "slot_no" onto default mount path

    esp_err_t unmount();			// Unmount default mounted SD-card
    esp_err_t unmount(const char mountpath[]);	// Unmount SD-card, that mounted onto "mountpath"
//    esp_err_t unmount(sdmmc_card_t *card);	// Unmount SD-card "card", mounted onto default mountpath
//    esp_err_t unmount(const char *base_path, sdmmc_card_t *card);	// Unmount mounted SD-card "card", mounted onto mountpath
    void card_info(FILE* outfile);		// Print the card info

    esp_err_t pwd();	// print current directory name
    esp_err_t ls();	// print a list of files in the required directory

    esp_err_t cat();	// type file contents - error, if file name is absent
    esp_err_t cat(const char fname[]);	// type file contents

    esp_err_t type();	// type text from keyboard to screen
    esp_err_t type(const char fname[]);	// type text from keyboard to file and to screen


private:
    esp_err_t ret;

    static const char* TAG;
    sdmmc_card_t *card;

    Control control;
//    Slot slot;
    Mounting mounting;

}; /* class Server */

//const char* Server::TAG = "SD/MMC service";

}; /* namespace SDMMC */  //-------------------------------------------------------------------------------------------


#if 0

/********** Content of the file sd_card_example.nain.c **********/



void app_main(void)
{


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

#else
#error "The sdcard_ctrl file usable in C++ projects only."
#endif

//--[ sdcard_ctrl.hpp ]----------------------------------------------------------------------------
