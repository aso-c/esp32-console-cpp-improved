/*
 * SD-card control classes
 * Implementation file
 * 	File: sdcard_ctrl.cpp
 *	Author:  aso (Solomatov A.A.)
 *	Created: 14.07.2022
 *	Version: 0.6
 */

#define __PURE_C__


#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG	// 4 - set 'DEBUG' logging level

#include <limits>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cstdarg>

#include <cstring>
#include <cctype>
#include <sys/unistd.h>
#include <cerrno>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include <argtable3/argtable3.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
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

#include "sdcard_io"

#include "extrstream"
#include "astring.h"

//using namespace idf;
using namespace std;


#define SD_MOUNT_POINT "/sdcard"


/*
 * Предлагаемые команды:
 *    -	sd - main command for manipulation with SD-caed
 *	+ m, mount	- mount sdcard, options: [<card>] [<mountpoint>];
 *	+ u, umount	- unmount sdcard, options: [<card>|<mountpiont>];
 *	+ mkdir		- create new directory
 *	+ ls, dir	- list of files in sdcard, options: [<file pattern>];
 *	+ cd <dir>	- change dir;
 *	+ cat <file>	- print file to console
 *	+ type [<file>]	- type text to cinsile and store it in the file optionally
 *	+ cp, copy	- copy file, options: [<src file>|<dest file>];
 *	+ mv, move	- move or rename file, options: [<src file>|<dest file>];
 */

namespace SDMMC	//-----------------------------------------------------------------------------------------------------
{

    static const char *TAG = "SD/MMC service";


//--[ strust Host ]----------------------------------------------------------------------------------------------------


// Default constructor
Host::Host(bus::width width, Pullup pullupst) // @suppress("Member declaration not found") // @suppress("Type cannot be resolved")
{
    //slot.default_num(cfg.slot);
    ESP_LOGI(TAG, "Using SDMMC peripheral - default constructor");
    // Define my delay for SD/MMC command execution
   // cfg.command_timeout_ms = SDMMC_COMMAND_TIMEOUT;
    bus_width(width);
    set_pullup(pullupst);
}; /* Host::Host */

// Constructor with default slot configuration by number of the slot
Host::Host(Slot::number number, bus::width width, Host::Pullup pullupst): // @suppress("Member declaration not found") // @suppress("Type cannot be resolved")
	_slot(number) // @suppress("Symbol is not resolved")
{
    cfg.slot = number; // @suppress("Field cannot be resolved")
    ((sdmmc_slot_config_t*)_slot)->width = width;
    bus_width(width);
    set_pullup(pullupst);
}; /* Host::Host(Slot::number, bus::width, Host::Pullup) */

// Custom slot configuration in temporary obj
// for desired slot number
// in lvalue object
Host::Host(Slot::number num, const Slot& slot, bus::width width, Pullup pullupst):
	_slot(slot)
{
    cfg.slot = num;
    ((sdmmc_slot_config_t*)_slot)->width = width;
    bus_width(width);
    set_pullup(pullupst);
}; /* Host::Host(Slot::number, const Slot&) */

// in temporary object
Host::Host(Slot::number num, Slot&& slot, bus::width width, Pullup pullupst):
	Host(num, slot, width, pullupst)
    {};

// Copy constructors
// for lvalue object (defined variable)
Host::Host(const Host& host, bus::width width, Pullup pullupst):
	Host(host.cfg, width, pullupst)
{
    _slot = host._slot;
}; /* Host::Host(const Host&) */

// for lvalue object (defined variable)
//Host(const sdmmc_host_t&, bus::width = bus::width_def, Pullup = nopullup);
Host::Host(const sdmmc_host_t& host, bus::width width, Pullup pullupst)
{
    cfg = host;
    if (host.slot == 0)
	_slot = Slot(Slot::_0);
    bus_width(width);
    set_pullup(pullupst);
}; /* Host::Host(const sdmmc_host_t&) */

// for rvalue oblect (e.g. temporary object)
Host::Host(sdmmc_host_t&& host, bus::width width, Pullup pullupst) noexcept:
	Host(host, width, pullupst)
{ };


Host& Host::operator =(const Host& host)
{
    *this = host.cfg;
    _slot = host._slot;
    return *this;
}; /* Host::operator =(const Host& */

Host& Host::operator =(const sdmmc_host_t& host)
{
    // uint32_t flags;             /*!< flags defining host properties */
    cfg.flags = host.flags;	/*!< flags defining host properties */
    //        int slot;                   /*!< slot number, to be passed to host functions */
    cfg.slot = host.slot;	/*!< slot number, to be passed to host functions */
    //        int max_freq_khz;           /*!< max frequency supported by the host */
    cfg.max_freq_khz = host.max_freq_khz;	/*!< max frequency supported by the host */
    //        float io_voltage;           /*!< I/O voltage used by the controller (voltage switching is not supported) */
    cfg.io_voltage = host.io_voltage;	/*!< I/O voltage used by the controller (voltage switching is not supported) */
    //        esp_err_t (*init)(void);    /*!< Host function to initialize the driver */
    cfg.init = host.init;	/*!< Host function to initialize the driver */
    //        esp_err_t (*set_bus_width)(int slot, size_t width);    /*!< host function to set bus width */
    cfg.set_bus_width = host.set_bus_width;	/*!< host function to set bus width */
    //        size_t (*get_bus_width)(int slot); /*!< host function to get bus width */
    cfg.get_bus_width = host.get_bus_width;	/*!< host function to get bus width */
    //        esp_err_t (*set_bus_ddr_mode)(int slot, bool ddr_enable); /*!< host function to set DDR mode */
    cfg.set_bus_ddr_mode = host.set_bus_ddr_mode;	/*!< host function to set DDR mode */
    //        esp_err_t (*set_card_clk)(int slot, uint32_t freq_khz); /*!< host function to set card clock frequency */
    cfg.set_card_clk = host.set_card_clk;	/*!< host function to set card clock frequency */
    //        esp_err_t (*do_transaction)(int slot, sdmmc_command_t* cmdinfo);    /*!< host function to do a transaction */
    cfg.do_transaction = host.do_transaction;	/*!< host function to do a transaction */
    //        union {
    //            esp_err_t (*deinit)(void);  /*!< host function to deinitialize the driver */
    //            esp_err_t (*deinit_p)(int slot);  /*!< host function to deinitialize the driver, called with the `slot` */
    //        };
    cfg.deinit = host.deinit;	/*!< host function to deinitialize the driver */
    //        esp_err_t (*io_int_enable)(int slot); /*!< Host function to enable SDIO interrupt line */
    cfg.io_int_enable = host.io_int_enable;	/*!< Host function to enable SDIO interrupt line */
    //        esp_err_t (*io_int_wait)(int slot, TickType_t timeout_ticks); /*!< Host function to wait for SDIO interrupt line to be active */
    cfg.io_int_wait = host.io_int_wait;	/*!< Host function to wait for SDIO interrupt line to be active */
    //        int command_timeout_ms;     /*!< timeout, in milliseconds, of a single command. Set to 0 to use the default value. */
    cfg.command_timeout_ms = host.command_timeout_ms;	/*!< timeout, in milliseconds, of a single command. Set to 0 to use the default value. */
    if (host.slot == 0)
	_slot = Slot(Slot::_0);
    return *this;
}; /* Host::operator =(const sdmmc_host_t&) */


//esp_err_t Host::init(int slotno, const sdmmc_slot_config_t *slot_config)
esp_err_t Host::init(Slot::number slotno, const Slot& extern_slot)
{
    cfg.slot = slotno;
    _slot = extern_slot;
//    return sdmmc_host_init_slot(slot, slot_config);
    return init();
}; /* Host::init(Slot::number, const Slot&) */

esp_err_t Host::init(Slot::number slotno, Slot&& extern_slot)
{
    //cfg.slot = slotno;
    //slot = slot_config;
//    return sdmmc_host_init_slot(slot, &slot_config);
//    return init();
    return init(slotno, extern_slot);
}; /* Host::init(Slot::number, Slot&&) */


//--[ strust Slot ]----------------------------------------------------------------------------------------------------


//=================================================================================================
//
// ESP32’s SDMMC host peripheral has two slots. Each slot can be used independently to connect to an SD card, SDIO device or eMMC chip.
//
//    Slot 0 (SDMMC_HOST_SLOT_0) is an 8-bit slot. It uses HS1_* signals in the PIN MUX.
//
//    Slot 1 (SDMMC_HOST_SLOT_1) is a 4-bit slot. It uses HS2_* signals in the PIN MUX.
//
// The slots are connected to ESP32 GPIOs using IO MUX. Pin mappings of these slots are given in the table below.
//
// Signal    Slot 0	Slot 1
//
// CMD	    GPIO11	GPIO15
// CLK	    GPIO6	GPIO14
// D0	    GPIO7	GPIO2
// D1	    GPIO8	GPIO4
// D2	    GPIO9	GPIO12
// D3	    GPIO10	GPIO13
// D4	    GPIO16
// D5	    GPIO17
// D6	    GPIO5
// D7	    GPIO18
// CD	    any input via GPIO matrix
// WP	    any input via GPIO matrix
//
//=================================================================================================


Slot::Slot()
{
    // To use 1-line SD mode, change this to 1:
//    cfg.width = SLOT_WIDTH;

    // On chips where the GPIOs used for SD card can be configured, set them in

#if 0
    // the slot_config structure:
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
    cfg.clk = GPIO_NUM_14;
    cfg.cmd = GPIO_NUM_15;
    cfg.d0 = GPIO_NUM_2;
    cfg.d1 = GPIO_NUM_4;
    cfg.d2 = GPIO_NUM_12;
    cfg.d3 = GPIO_NUM_13;
#endif
#endif

    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
//    cfg.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

}; /* Slot::Slot */

//Slot();
// really copy constructor
Slot::Slot(const Slot& slot):
    Slot(slot.cfg)
{};

// from struct sdmmc_slot_config_t copy constructor
Slot::Slot(const sdmmc_slot_config_t& config)
{

#ifdef SOC_SDMMC_USE_GPIO_MATRIX
//        gpio_num_t clk;         ///< GPIO number of CLK signal.
    cfg.clk = config.clk;	///< GPIO number of CLK signal.
//        gpio_num_t cmd;         ///< GPIO number of CMD signal.
    cfg.cmd = config.cmd;	///< GPIO number of CMD signal.
//        gpio_num_t d0;          ///< GPIO number of D0 signal.
    cfg.d0 = config.d0;	///< GPIO number of D0 signal.
//        gpio_num_t d1;          ///< GPIO number of D1 signal.
    cfg.d1 = config.d1;	///< GPIO number of D1 signal.
//        gpio_num_t d2;          ///< GPIO number of D2 signal.
    cfg.d2 = config.d2;	///< GPIO number of D2 signal.
//        gpio_num_t d3;          ///< GPIO number of D3 signal.
    cfg.d3 = config.d3;	///< GPIO number of D3 signal.
//        gpio_num_t d4;          ///< GPIO number of D4 signal. Ignored in 1- or 4- line mode.
    cfg.d4 = config.d4;	///< GPIO number of D4 signal. Ignored in 1- or 4- line mode.
//        gpio_num_t d5;          ///< GPIO number of D5 signal. Ignored in 1- or 4- line mode.
    cfg.d5 = config.d5;	///< GPIO number of D5 signal. Ignored in 1- or 4- line mode.
//        gpio_num_t d6;          ///< GPIO number of D6 signal. Ignored in 1- or 4- line mode.
    cfg.d6 = config.d6;	///< GPIO number of D6 signal. Ignored in 1- or 4- line mode.
//        gpio_num_t d7;          ///< GPIO number of D7 signal. Ignored in 1- or 4- line mode.
    cfg.d7 = config.d7;	///< GPIO number of D7 signal. Ignored in 1- or 4- line mode.
#endif // SOC_SDMMC_USE_GPIO_MATRIX
//        union {
//            gpio_num_t gpio_cd;     ///< GPIO number of card detect signal
//            gpio_num_t cd;          ///< GPIO number of card detect signal; shorter name.
//        };
    cfg.cd = config.cd;	///< card detect signal
//        union {
//            gpio_num_t gpio_wp;     ///< GPIO number of write protect signal
//            gpio_num_t wp;          ///< GPIO number of write protect signal; shorter name.
//        };
    cfg.wp = config.wp;	///< write protect signal
//        uint8_t width;          ///< Bus width used by the slot (might be less than the max width supported)
    cfg.width = config.width;	///< Bus width used by the slot (might be less than the max width supported)
//        uint32_t flags;         ///< Features used by this slot
    cfg.flags = config.flags;         ///< Features used by this slot
//    #define SDMMC_SLOT_FLAG_INTERNAL_PULLUP  BIT(0)
            /**< Enable internal pullups on enabled pins. The internal pullups
             are insufficient however, please make sure external pullups are
             connected on the bus. This is for debug / example purpose only.
             */

}; /* Slot::Slot(const sdmmc_slot_config_t&) */

// temporary object copy constructor
Slot::Slot(sdmmc_slot_config_t&& config):
	Slot(config)
{};

// initializing Slot cfg by its number
//enum number {_0 = SDMMC_HOST_SLOT_0, null=_0, _1 = SDMMC_HOST_SLOT_1, one=_1};
Slot::Slot(number num)
{
    switch (num)
    {
	// configurate slot to Slot0
    case null:

#ifdef SOC_SDMMC_USE_GPIO_MATRIX
	//Signal Slot0	Slot 1
	// CMD	GPIO11	GPIO15
	cfg.cmd = GPIO_NUM_11;
	// CLK	GPIO6	GPIO14
	cfg.clk = GPIO_NUM_6;
	// D0	GPIO7	GPIO2
	cfg.d0 = GPIO_NUM_7;
	// D1	GPIO8	GPIO4
	cfg.d1 = GPIO_NUM_8;
	// D2	GPIO9	GPIO12
	cfg.d2 = GPIO_NUM_9;
	// D3	GPIO10	GPIO13
	cfg.d3 = GPIO_NUM_10;
	// D4	GPIO16
	cfg.d4 = GPIO_NUM_16;
	// D5	GPIO17
	cfg.d5 = GPIO_NUM_17;
	// D6	GPIO5
	cfg.d6 = GPIO_NUM_5;
	// D7	GPIO18
	cfg.d7 = GPIO_NUM_18;
	// CD	any input via GPIO matrix
	// WP	any input via GPIO matrix
#endif
	// Other params - initialized by default
//	    cfg.cd = SDMMC_SLOT_NO_CD;
//	    cfg.wp = SDMMC_SLOT_NO_WP;
	//    cfg.width   = SDMMC_SLOT_WIDTH_DEFAULT;
	    //cfg.width   = bus::width_8;
//	    cfg.flags = 0;
	break;

	// for Slot 1 - initialized by default values
    default:
	;
    }; /* switch num */
}; /* Slot::Slot(number) */

// Set or clear SDDMMC internal pullup bit
void Slot::internal_pullup(bool pullup)
{
    if (pullup)
	cfg.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;	///> set internal pullap bit
    else
	cfg.flags &= ~SDMMC_SLOT_FLAG_INTERNAL_PULLUP;	///> clear internal pullup bit
}; /* Slot::internal_pullup */



// Assignment operators
//    sdmmc_slot_config_t& operator =(const Slot&);
Slot& Slot::operator =(const Slot& slot)
{
    *this = slot.cfg;
    return *this;
}; /* Slot::operator =(const Slot&) */

//    sdmmc_slot_config_t& operator =(const sdmmc_slot_config_t&);
Slot& Slot::operator =(const sdmmc_slot_config_t& config)
{
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
    //        gpio_num_t clk;         ///< GPIO number of CLK signal.
    cfg.clk = config.clk;	///< GPIO number of CLK signal.
    //        gpio_num_t cmd;         ///< GPIO number of CMD signal.
    cfg.cmd = config.cmd;	///< GPIO number of CMD signal.
    //        gpio_num_t d0;          ///< GPIO number of D0 signal.
    cfg.d0 = config.d0;	///< GPIO number of D0 signal.
    //        gpio_num_t d1;          ///< GPIO number of D1 signal.
    cfg.d1 = config.d1;	///< GPIO number of D1 signal.
    //        gpio_num_t d2;          ///< GPIO number of D2 signal.
    cfg.d2 = config.d2;	///< GPIO number of D2 signal.
    //        gpio_num_t d3;          ///< GPIO number of D3 signal.
    cfg.d3 = config.d3;	///< GPIO number of D3 signal.
    //        gpio_num_t d4;          ///< GPIO number of D4 signal. Ignored in 1- or 4- line mode.
    cfg.d4 = config.d4;	///< GPIO number of D4 signal. Ignored in 1- or 4- line mode.
    //        gpio_num_t d5;          ///< GPIO number of D5 signal. Ignored in 1- or 4- line mode.
    cfg.d5 = config.d5;	///< GPIO number of D5 signal. Ignored in 1- or 4- line mode.
    //        gpio_num_t d6;          ///< GPIO number of D6 signal. Ignored in 1- or 4- line mode.
    cfg.d6 = config.d6;	///< GPIO number of D6 signal. Ignored in 1- or 4- line mode.
    //        gpio_num_t d7;          ///< GPIO number of D7 signal. Ignored in 1- or 4- line mode.
    cfg.d7 = config.d7;	///< GPIO number of D7 signal. Ignored in 1- or 4- line mode.
#endif // SOC_SDMMC_USE_GPIO_MATRIX
    cfg.cd = config.cd;	///< card detect signal
    cfg.wp = config.wp;	///< write protect signal
    cfg.width = config.width;	///< Bus width used by the slot (might be less than the max width supported)
    cfg.flags = config.flags;         ///< Features used by this slot
    return *this;
}; /* Slot::operator =(const sdmmc_slot_config_t&) */


//    sdmmc_slot_config_t& operator =(sdmmc_slot_config_t&&) noexcept;
Slot& Slot::operator =(sdmmc_slot_config_t&& config) noexcept
{
    *this = config;
    return *this;
}; /* Slot::operator =(sdmmc_slot_config_t&&) */


//int Slot::def_num;


//--[ class CWD_emulating ]-----------------------------------------------------------------------------------------


// get current dir (if path == NULL or "") or generate fullpath for sended path
// absent trailing slash in returned string is guaranteed

// return current pwd (current dir) only
char* CWD_emulating::get()
{
    ESP_LOGD("CWD_emulating:", "%s: \"path\" argument is absent",  __func__);
    ESP_LOGD("CWD_emulating:", "%s: \"len\" argument is absent too",  __func__);
    strcpy(operative_path_buff, pwd);
    return operative_path_buff;
}; /* char* CWD_emulating::get() */

//// return full path appling current dir
//char* get(const char path[]);

// return full path appling current dir, use desired part of the passed path
// return current dir (if path == NULL or "") or generate fullpath for sended path
// trailing slash in returned string is absent always
char* CWD_emulating::get(const char path[], size_t len)
{
    ESP_LOGD("CWD_emulating:", "%s: \"path\" argument is %s",  __func__, path);
    ESP_LOGD("CWD_emulating:", "%s: \"len\" argument is %d",  __func__, len);
    // if len not defined
    if (len == 0)
	return get(path);

    // argument - absolute path
    if (!empty(path) && absolute_path(path))
    {
	if (len > sizeof(operative_path_buff) / sizeof(char) - 1)
	    clearbuff();	// path don't fit in operative_path_buff - error, return empty str
	else
	{
		char* tmpstr = (char*)malloc(len + 1);
	    strncpy(tmpstr, path, len/* + 1*/);
	    tmpstr[len] = '\0';	// complete the NULL-terminated string
	    realpath(tmpstr, operative_path_buff);
	    free(tmpstr);
	}; /* if len > (sizeof(operative_path_buff) / sizeof(char) - 1) */
	ESP_LOGD("CWD_emulating:", "%s: operative_path_buff is \"%s\"", __func__, operative_path_buff);
	return operative_path_buff;
    }; /* if path[0] != '/' */

    get();
    // pwd == "" --> catch it
    if (empty(pwd))
	return get_current();

    // argument - NULL or empty string
    if (empty(path))
	return get_current();

    // relative path - finalize processing
    ESP_LOGD("CWD_emulating:", "%s: processing relative path: updating path on top of the current pwd", __func__);
    // add a trailing slash at end of the relative path base
    if (operative_path_buff[strlen(operative_path_buff) - 1] != '/')
    {
	ESP_LOGD("CWD_emulating:", "%s: operative_path_buff before adding trailing slash is: \"%s\"", __func__, operative_path_buff);
	// add EOL behind the string data in the operative_path_buff
	// add trailing '/' at the operative_path_buff
	operative_path_buff[strlen(operative_path_buff) + 1] = '\0';
	operative_path_buff[strlen(operative_path_buff)] = '/';
	//strcat(operative_path_buff, "/");
	ESP_LOGD("CWD_emulating:", "%s: operative_path_buff after adding trailing slash is: \"%s\"", __func__, operative_path_buff);
    }; /* if operative_path_buffer[strlen(operative_path_buff) - 1] != '/' */

    // copy path on top of base bath
    if (strlen(operative_path_buff) + len < sizeof(operative_path_buff) / sizeof(char))
	strncat(operative_path_buff, path, len);
    else
	return clearbuff();

    ESP_LOGD("CWD_emulating:", "%s: final operative_path_buff after drop it's trailing slash: \"%s\"", __func__, operative_path_buff);

	 char* src = realpath(operative_path_buff, NULL);	// resolve dirty path
    strcpy(operative_path_buff, src);
    free(src);

    return operative_path_buff;
}; /* CWD_emulating::get */


// raw_get path with current dir:
// only concatenate path with current dir,
// not processing output with realpath().
char* CWD_emulating::raw_get(const char path[])
{
    if (empty(path))
	return strcpy(operative_path_buff, pwd);
    if(absolute_path(path))
	return strcpy(operative_path_buff, path);
    raw_get();
    strcat(operative_path_buff, "/");	// add directory separator at end of the default path
//    strcat(operative_path_buff, path);	// add the path above to default path
//    return operative_path_buff;
    return strcat(operative_path_buff, path);	// add the path above to default path
}; /* CWD_emulating::raw_get */


// change cwd dir
esp_err_t CWD_emulating::change_dir(const char path[])
{
	const char* tmpstr = get(path);
	struct stat statbuf;

    ESP_LOGD("CWD_emulating::change_dir", "The \"path\" parameter is: \"%s\"", path);
    ESP_LOGD("CWD_emulating::change_dir", "The \"tmpstr\" variable is: \"%s\"", tmpstr);

    if (tmpstr == nullptr || tmpstr[0] == '\0')
    {
	ESP_LOGE("CWD_emulating::change_dir", "Change dir is failed");
	return ESP_FAIL;
    }; /* if tmpstr == nullptr || tmpstr[0] == '\0' */
    // if dir changed to root - exclusively change dir
    if (strcmp(tmpstr,"/") == 0)
	goto final_copy;
    if (stat(tmpstr, &statbuf) == -1)
    {
	ESP_LOGE("CWD_emulating::change_dir", "Change dir is failed - requested path to change \"%s\" is not exist;\n"
		"\t\t\t\tcurrent directory was not changing", tmpstr);
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(tmpstr, &statbuf) == -1 */
    ESP_LOGI("CWD_emulating::change_dir", "to %s which is a %s\n", tmpstr,
	    (S_ISLNK(statbuf.st_mode))? "[symlink]":
	    (S_ISREG(statbuf.st_mode))? "(file)":
	    (S_ISDIR(statbuf.st_mode))? "<DIR>":
	    (S_ISCHR(statbuf.st_mode))? "[char dev]":
	    (S_ISBLK(statbuf.st_mode))? "[blk dev]":
	    (S_ISFIFO(statbuf.st_mode))? "[FIFO]":
	    (S_ISSOCK(statbuf.st_mode))? "[socket]":
	    "[unknown type]");
    if (!S_ISDIR(statbuf.st_mode))
    {
	ESP_LOGE("CWD_emulating::change_dir", "Change dir is failed - requested path to change \"%s\" is not directory;\n"
		"\t\t\t\tleave current directory without changing", tmpstr);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if stat(tmpstr, &statbuf) == -1 */

final_copy:
    // copy tmpstr to pwd at the final
    strcpy(pwd, tmpstr);
    return ESP_OK;
}; /* CWD_emulating::change_dir */

// temporary buffer for file fullpath composing
char CWD_emulating::operative_path_buff[PATH_MAX];

//--[ struct Device ]-----------------------------------------------------------------------------------------------


Device::Device(bus::width width, Host::Pullup pullst, esp_vfs_fat_sdmmc_mount_config_t&& mnt_cfg):
	/*card(nullptr),*/
	_host(width, pullst),
	fake_cwd(fake_cwd_path, sizeof(fake_cwd_path))
{
    selective_log_level_set("Device::valid_path", ESP_LOG_DEBUG);	/* for debug purposes */
//#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
//	mnt.format_if_mount_failed = true;
//#else
//	mnt.format_if_mount_failed = false;
//#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
    mnt.format_if_mount_failed = mnt_cfg.format_if_mount_failed;
//	mnt.max_files = 5;
    mnt.max_files = mnt_cfg.max_files;
//	mnt.allocation_unit_size = 16 * 1024;
    mnt.allocation_unit_size = mnt_cfg.allocation_unit_size;
	//ESP_LOGI(TAG, "Initializing SD card");
}; /* Device::Device(bus::width, Host::Pullup, esp_vfs_fat_sdmmc_mount_config_t&) */

Device::Device(Card::format::mntfail autofmt, int max_files, size_t size,
	    bus::width width, Host::Pullup pull):
		_host(width, pull),
		fake_cwd(fake_cwd_path, sizeof(fake_cwd_path))
{
    selective_log_level_set("Device::valid_path", ESP_LOG_DEBUG);	/* for debug purposes */
        mnt.format_if_mount_failed = (autofmt == Card::format::yes)? true: false;
    //	mnt.max_files = 5;
        mnt.max_files = max_files;
    //	mnt.allocation_unit_size = 16 * 1024;
        mnt.allocation_unit_size = size;
    	//ESP_LOGI(TAG, "Initializing SD card");
}; /* Device::Device(Card::format::mntfail, int, size_t, bus::width, Host::Pullup) */




// Mount default SD-card slot onto path "mountpoint"
esp_err_t Device::mount(Card& excard, const char mountpoint[])
{
	esp_err_t ret;

    // if card already mounted - exit with error
    if (card)
    {
	ESP_LOGE(TAG, "%s: card already mounted at the %s, refuse to mount again", __func__, mountpath());
//	return ESP_ERR_INVALID_STATE;
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if card */

    card  = &excard;
    target = mountpoint;

    ret = esp_vfs_fat_sdmmc_mount(mountpoint, _host, _host.slot(), &mnt, &card->self);
    if (ret != ESP_OK)
    {
	if (ret == ESP_FAIL)
//	    cout << TAG << ": " << "Failed to mount filesystem. "
//		<< "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.";
	    ESP_LOGI(TAG, "Failed to mount filesystem. %s",
		"If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
	else
//	    cout << TAG << ": " << "Failed to initialize the card (error " << ret << ", " << esp_err_to_name(ret) << "). "
//		<< "Make sure SD card lines have pull-up resistors in place.";
	    ESP_LOGI(TAG, "Failed to initialize the card (error %d, %s). %s", ret,  esp_err_to_name(ret),
		"Make sure SD card lines have pull-up resistors in place.");
	return ret;
    }; /* if ret != ESP_OK */

    ESP_LOGI(TAG, "Filesystem mounted at the %s", mountpath());

#ifdef CONFIG_AUTO_CHDIR_BEHIND_MOUNTING
//    change_currdir(DIRECTORY_FOR_AUTOCHANGE);
    change_currdir(mountpath());
    ESP_LOGI(TAG, "Current directory autochanged to: %s", fake_cwd_path);
#else
//    change_currdir("/");
    getcwd(fake_cwd_path, sizeof(fake_cwd_path));	// set fake_cwd according system pwd (through get_cwd())
    ESP_LOGI(TAG, "Current directory set to: %s,  according system pwd", fake_cwd_path);
#endif

    return ret;
}; /* Device::mount(Card&, char[]) */



//esp_err_t mount(Card&, int slot_no);		// Mount SD-card slot "slot_no" onto default mount path
//esp_err_t mount(Card&, int slot_no, const char mountpoint[]);  // Mount SD-card slot "slot_no" onto default mount path

// unmount();
//esp_err_t unmount(const char mountpath[] = NULL);	// Unmount SD-card, that mounted onto "mountpath", default - call w/o mountpath
//------------------------------------------------------------------------------------------
//    // All done, unmount partition and disable SDMMC peripheral
//    esp_vfs_fat_sdcard_unmount(mount_point, card);
//    ESP_LOGI(TAG, "Card unmounted");
//------------------------------------------------------------------------------------------

// Unmount SD-card, that mounted onto "mountpath"
esp_err_t Device::unmount()
{

	esp_err_t ret;

    // if card already mounted - exit with error
    if (!card)
    {
	ESP_LOGE(TAG, "%s: mounted card is absent, nothing to unmount", __func__);
	return ESP_ERR_NOT_FOUND;
    }; /* if card */

//    if (empty(mountpath))
//    {
//	cout << TAG << ": " << "Call: unmount(" << mounting.target << ");" << endl;
//	ret = unmount(mounting.target);
//	return ret;
//    }; /* if mountpath == NULL || strcmp(mountpath, "") == 0 */
    ret = esp_vfs_fat_sdcard_unmount(target, card->self);
    //ESP_LOGI(TAG, "Card unmounted");
    if (ret != ESP_OK)
    {
//	cout << TAG << ": "  << "Error: " << ret
//	    << ", " << esp_err_to_name(ret) << endl;
	ESP_LOGE(TAG, "Error: %d, %s", ret, esp_err_to_name(ret));
	return ret;
    }; /* if ret != ESP_OK */

//    cout << TAG << ": " << "Card unmounted" << endl;
    ESP_LOGI(TAG, "Card at %s unmounted", mountpath());
    card = nullptr;	// card is unmounted - clear this field as unmounted sign
    clean_mountpath();
    fake_cwd_path[0] = '\0';	// set fake cwd path to: ""
    return ret;
}; /* Device::unmount */
//    esp_err_t unmount(sdmmc_card_t *card);	// Unmount SD-card "card", mounted onto default mountpath
//    esp_err_t unmount(const char *base_path, sdmmc_card_t *card);	// Unmount mounted SD-card "card", mounted onto mountpath


// if the basename (the last part of the path) - has the characteristics
// of a directory name, and a dirname (the path prefix) -
// is an existing file, not a directory, or any other impossible variants
// of the full file/path name
bool Device::valid_path(const char path[])
{

	struct stat st;
	char *base = basename(path);	// get a filename of a path

/*esp_log_level_set("Device::valid_path", ESP_LOG_DEBUG);*//* for debug purposes */
    ESP_LOGD("Device::valid_path", "basename of the path is: \"%s\"", base);
    ESP_LOGD("Device::valid_path", "full path is: \"%s\"", path);
    ESP_LOGD("Device::valid_path", "dirname path is: \"%.*s\"", base - path, path);

//    strcpy(given_path, fake_cwd.curr_get());
    // if path is empty
    if (empty(path))
    {
	ESP_LOGD("Device::valid_path", "path is empty, always valid");
	return true;
    }; /* if empty(path) */
    // if path - only base, not a dir
    if (strlen(path) == 1)
    {
	ESP_LOGD("Device::valid_path", "len of the path - is 1, always valid");
	return true;
    }; /* if strlen(path) == 1 */
    // if dirname - empty or one symbol length (it can only be the slash)
    if ((base - path) < 2)
    {
	// if path == '/..' - it's invalid
	if (strcmp(path, "/..") == 0)
	    return false;
	ESP_LOGD("Device::valid_path", "Len of dirname is 1 or 0, then path is valid");
	return true;
    }; /* if (base - path) < 2 */

    // if base is empty
    if (empty(base))
    {
	if (stat(fake_cwd.get(path), &st) == 0)
	{
	    ESP_LOGD("Device::valid_path", "###!!! the path basename - is empty, test the path \"%s\" (real path is %s) exist and a directory... ###", path, fake_cwd.get_current());
	    if (!S_ISDIR(st.st_mode))
	    {
		ESP_LOGE("Device::valid_path", "Path \"%s\" (real path %s) is a file, but marked as a directory, it's invalid!!!", path, fake_cwd.get_current());
		return false;	// the path is invalid (inconsist)
	    }; /* path is not dir */
	}; /* if stat(real_path, &st) == 0 */
	ESP_LOGD("Device::valid_path", "###!!! test dirname \"%s\" (real path is %s) preliminary is OK, seek to begin of last dir manually for continue test... ###", path, fake_cwd.get_current());
	for (base -= 2; base > path; base--)
	{
	    ESP_LOGD("Device::valid_path", "=== base[0] is \"%c\" ===", base[0]);
	    if (*base == '/')
	    {
//		base++;
		break;
	    }; /* if base[0] == '/' */
	}; /* for base--; base > path; base-- */
    }; /* if (empty(base)) */

#if 0
    // dirname of the path must be exist
    // and be a directory, not a file
    if (stat(fake_cwd.get(path, base - path - 1), &st) != 0)
    {
	ESP_LOGD("Device::valid_path", "###!!! 1'st subdir (is \"%.*s\") of the path \"%s\" is not exist, but should be... ###", base - path - ((base==path)? 0: 1), path, path);
	return false;
    }
    else
    {
	ESP_LOGD("Device::valid_path", "###!!! 1'st subdir (\"%.*s\") of the path \"%s\" is exist, must be a directory... ###", base - path - ((base==path)? 0: 1), path, path);
	if (!S_ISDIR(st.st_mode))
	{
	    ESP_LOGE("Device::valid_path", "Path \"%.*s\" is a file, but must be a directory, it's invalid!!!", base - path - ((base==path)? 0:  1), path);
	    return false;	// the path is invalid (inconsist)
	}; /* subpath is exist */
    }; /* if stat(real_path, &st) == 0 */
#endif

#define sign_place 0x2	// with of the place for the sign
#define point_sign 0x1	// mark a point symbol in a string
#define alpha_sign 0x2	// mark a non-point or a non-slash symbol in a string
#define initial_ctrl (0x3 << 3*sign_place)	// mark for the initial pass of the control of the path validity
#define alpha_present_mask (alpha_sign | (alpha_sign << 1*sign_place) | (alpha_sign << 2*sign_place) | (alpha_sign << 3*sign_place))
#define three_point_mark (point_sign | (point_sign << 1*sign_place) | (point_sign << 2*sign_place))

	unsigned int ctrl_cnt = initial_ctrl;	// marked the firs pass of the control loop
	unsigned int idx_ctrl = 0;
    // scan the dirname of the path for found '/.' or '/..' sequence
    for ( char* scan = base - 2; scan >= path; scan--)
    {
	ESP_LOGD("Device::valid_path", "current char from the path is: '%c', ctrl_cnt is %2X", *scan, ctrl_cnt);

	    unsigned int prev_ctrl = ctrl_cnt;

	switch (scan[0])
	{
	// solution point
	case '/':

	    ctrl_cnt = 0;
	    idx_ctrl = 0;	// reset the idx_ctrl
	    ESP_LOGD("Device::valid_path", "###### Solution point: current path char ######");
	    // double slash - prev symbol is slash
	    if (prev_ctrl == 0)
	    {
		ESP_LOGD("Device::valid_path", "**** double slash and more - is not valid sequence in the path name ****");
		return false;
	    }; /* if prev_ctrl == 0 */
	    // if non point sign is present in tested substring
	    if (prev_ctrl & alpha_present_mask)
	    {
		ESP_LOGD("Device::valid_path", "alpha or other then point or slash symbol is present in current processing substring - test subpath for exist, continue");
		ctrl_cnt = 0;
		continue;
	    }; /* if prev_ctrl & alpha_present_mask */
	    // if more then 3 point sequence in substring
	    if (prev_ctrl == three_point_mark)
	    {
		ESP_LOGD("Device::valid_path", "3 point or more sequence is present in current substring - nothing to do, continue");
		ctrl_cnt = 0;
		continue;
	    }; /* if prev_ctrl & alpha_present_mask */

	    ESP_LOGD("Device::valid_path", "====== One or two point sequence in the current meaning substring, ctrl_cnt is %2X, test current subpath for exist ======", prev_ctrl);
	    if (stat(fake_cwd.get(path, scan - path - 1), &st) == 0)
	    {
	    	if (!S_ISDIR(st.st_mode))
	    	{
		    ESP_LOGE("Device::valid_path", "!!! Subpath is non exist, it's invalid!!!");
		    return false;	// the path is invalid (inconsist)
	    	}; /* subpath is exist */
	    }; /* if stat(real_path, &st) == 0 */

	    break;

	// point symbol handling
	case '.':

	    if (idx_ctrl <= 3)
	    {
		ESP_LOGD("Device::valid_path", "%d symbol of the processing substring", idx_ctrl);
		ctrl_cnt |= (point_sign << (idx_ctrl++) * sign_place);
	    } /* if idx_ctrl <= 3 */
	    else
		ESP_LOGD("Device::valid_path", "more then 3'd symbol of the processing substring, nothing to do");
	    continue;
	    break;

	// all other symbols
	default:

	    if (idx_ctrl <= 3)
	    {
		ESP_LOGD("Device::valid_path", "%d symbol of the processing substring", idx_ctrl);
		ctrl_cnt |= (alpha_sign << (idx_ctrl++) * sign_place);
	    } /* if idx_ctrl <= 3 */
	    else
		ESP_LOGD("Device::valid_path", "more then 3'd symbol of the processing substring, nothing to do");
	    //continue;

	}; /* switch scan[0] */
    }; /* for char* scan = base - 1; scan > path; scan-- */

    return true;
}; /* Device::valid_path */


#if 0	// simply_dir_verify
// is the filename ending
// really like as a directory name end?
extern "C"
bool dir_tail(const char fname[])
{
	size_t len = strlen(fname);
    if (len == 0)
	return false;
    // path == "blablabla/"
    if (fname[len-1] == '/')
	return true;
    // path == "."
    if (len == 1)
    {
	if (fname[0] == '.')
	    return true;
	else return false;
    }; /* if len == 1 */
    // path == "blablabla/."
    if (fname[len-2] == '/' && fname[len-1] == '.')
	return true;
    if (len == 2)
    {
	// path == ".."
	if (fname[len-2] == '.' && fname[len-1] == '.')
	    return true;
	// path != ".."
	else return false;
    }; /* if len == 2 */
    // path == "blablabla/.."
    if (fname[len-3] == '/' && fname[len-2] == '.' && fname[len-1] == '.')
	return true;
    return false;
}; /* dir_tail */
#endif



//const char *Device::MOUNT_POINT_Default = MOUNT_POINT_def;


//--[ class Card ]------------------------------------------------------------------------------------------------

#define CMD_TAG_PRFX "SD/MMC Card::"


// Print the card info
void Card::print_info(FILE* outfile)
{
    sdmmc_card_print_info(outfile, self);
    fprintf(outfile, "Sector: %d Bytes\n\n", self->csd.sector_size);
}; /* Card::print_info */


// print the SD-card info (wrapper for the external caller)
esp_err_t Card::info()
{
    print_info(stdout);
    return ESP_OK;
}; /* Card::info */

const char* Card::TAG = "SD/MMC Card";

// Print CIS info to outfile -
// High level procedure
esp_err_t Card::cis_info(FILE* outfile)
{
	size_t cisize = 0;
	size_t bsize = 16;
	uint8_t* outbuf = (uint8_t*)malloc(bsize);
	esp_err_t err;

    err = io.get_cis_data(outbuf, bsize, &cisize); // @suppress("Method cannot be resolved") // @suppress("Field cannot be resolved")
    if (err != ESP_OK)
    {
	ESP_LOGE("sdcard info command", "Error %i in get get CIS data first time: %s", err, esp_err_to_name(err));
	free(outbuf);
	switch (err)
	{
	case ESP_ERR_INVALID_RESPONSE:
	    return err;
	case ESP_ERR_INVALID_SIZE:	// CIS_CODE_END found, but buffer_size is less than required size, which is stored in the inout_cis_size then.
	case ESP_ERR_NOT_FOUND:		// if the CIS_CODE_END not found. Increase input value of inout_cis_size or set it to 0,
	    bsize = cisize;
//	    cout << aso::format("The new size of the CIS data buffer is: %i") % bsize << endl;
	    ESP_LOGD("sdcard info command", "Error %i in get get CIS data first time: %s", err, esp_err_to_name(err));
	    ESP_LOGD("sdcard cis info command", "The new size of the CIS data buffer is: %i", bsize);
	    cisize = 0;
	    outbuf = (uint8_t*)malloc(bsize);
	    err = io.get_cis_data(outbuf, bsize, &cisize); // @suppress("Field cannot be resolved") // @suppress("Method cannot be resolved")
	}; /* switch err */
    }; /* if err != ESP_ERR_INVALID_SIZE */

    if (err != ESP_OK)
    {
	//ESP_RETURN_ON_ERROR(err, "sdcard info command", "Error in the get CIS data");
	ESP_LOGD("sdcard info command", "Error %i in the get CIS data: %s", err, esp_err_to_name(err));
	return err;
    }; /* if err != ESP_OK */

    err = io.print_cis_info(outbuf, bsize, stdout); // @suppress("Method cannot be resolved") // @suppress("Field cannot be resolved")
    free(outbuf);
    if (err != ESP_OK)
	ESP_LOGD("sdcard info command", "Error %i in the print of the CIS info: %s", err, esp_err_to_name(err));

    return err;
}; /* Card::cis_info */



//--[ class IO ]--------------------------------------------------------------------------------------------------



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
    sdmmc_host_t cfg = SDMMC_HOST_DEFAULT();

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
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &cfg, &slot_config, &mount_config, &card);

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
