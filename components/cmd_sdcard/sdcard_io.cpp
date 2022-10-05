/*
 * SD-card control classes
 * Implementation file
 * 	File: sdcard_ctrl.cpp
 *	Author:  aso (Solomatov A.A.)
 *	Created: 14.07.2022
 *	Version: 0.6
 */

#define __PURE_C__


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

//#include <sdcard_ctrl>
#include <sdcard_io>

#include "extrstream"


//using namespace idf;
using namespace std;


#define MOUNT_POINT_def "/sdcard"


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

//int Control::slot_default_no;

// Default constructor
Host::Host()
{
    //slot.default_num(cfg.slot);
    ESP_LOGI(TAG, "Using SDMMC peripheral - default constructor");
    // Define my delay for SD/MMC command execution
   // cfg.command_timeout_ms = SDMMC_COMMAND_TIMEOUT;
}; /* Host::Host */

// Constructor with default slot configuration by number of the slot
Host::Host(Slot::number number):
	slot(number) {
	cfg.slot = number; };

// Custom slot configuration in temporary obj
// for desired slot number
// in lvalue object
//Host::Host(Slot::number number, const sdmmc_slot_config_t& slot_config)
Host::Host(Slot::number num, const Slot& aslot):
	slot(aslot)
{
    cfg.slot = num;
}; /* Host::Host(Slot::number, const Slot&) */
//}; /* Host::Host(Slot::number, const sdmmc_slot_config_t&) */

// in temporary object
//Host::Host(Slot::number number, sdmmc_slot_config_t&& slot_config)
Host::Host(Slot::number num, Slot&& aslot):
	Host(num, aslot)
{
    ;
}; /* Host::Host(Slot::number, Slot::number, Slot&&) */
//}; /* Host::Host(Slot::number, sdmmc_slot_config_t&&) */

// Copy constructors
// for lvalue object (defined variable)
Host::Host(const Host& host):
	Host(host.cfg)
{
////    typedef struct {
////        uint32_t flags;             /*!< flags defining host properties */
//    cfg.flags = host.cfg.flags;	/*!< flags defining host properties */
////    #define SDMMC_HOST_FLAG_1BIT    BIT(0)      /*!< host supports 1-line SD and MMC protocol */
////    #define SDMMC_HOST_FLAG_4BIT    BIT(1)      /*!< host supports 4-line SD and MMC protocol */
////    #define SDMMC_HOST_FLAG_8BIT    BIT(2)      /*!< host supports 8-line MMC protocol */
////    #define SDMMC_HOST_FLAG_SPI     BIT(3)      /*!< host supports SPI protocol */
////    #define SDMMC_HOST_FLAG_DDR     BIT(4)      /*!< host supports DDR mode for SD/MMC */
////    #define SDMMC_HOST_FLAG_DEINIT_ARG BIT(5)      /*!< host `deinit` function called with the slot argument */
////        int slot;                   /*!< slot number, to be passed to host functions */
//    slot = host.slot;	/*!< slot number, to be passed to host functions */
////        int max_freq_khz;           /*!< max frequency supported by the host */
//    max_freq_khz = host.max_freq_khz;	/*!< max frequency supported by the host */
////    #define SDMMC_FREQ_DEFAULT      20000       /*!< SD/MMC Default speed (limited by clock divider) */
////    #define SDMMC_FREQ_HIGHSPEED    40000       /*!< SD High speed (limited by clock divider) */
////    #define SDMMC_FREQ_PROBING      400         /*!< SD/MMC probing speed */
////    #define SDMMC_FREQ_52M          52000       /*!< MMC 52MHz speed */
////    #define SDMMC_FREQ_26M          26000       /*!< MMC 26MHz speed */
////        float io_voltage;           /*!< I/O voltage used by the controller (voltage switching is not supported) */
//    io_voltage = host.io_voltage;	/*!< I/O voltage used by the controller (voltage switching is not supported) */
////        esp_err_t (*init)(void);    /*!< Host function to initialize the driver */
//    init = host.init;	/*!< Host function to initialize the driver */
////        esp_err_t (*set_bus_width)(int slot, size_t width);    /*!< host function to set bus width */
//    set_bus_width = host.set_bus_width;	/*!< host function to set bus width */
////        size_t (*get_bus_width)(int slot); /*!< host function to get bus width */
//    get_bus_width = host.get_bus_width;	/*!< host function to get bus width */
////        esp_err_t (*set_bus_ddr_mode)(int slot, bool ddr_enable); /*!< host function to set DDR mode */
////        esp_err_t (*set_bus_ddr_mode)(int slot, bool ddr_enable); /*!< host function to set DDR mode */
////        esp_err_t (*set_card_clk)(int slot, uint32_t freq_khz); /*!< host function to set card clock frequency */
////        esp_err_t (*do_transaction)(int slot, sdmmc_command_t* cmdinfo);    /*!< host function to do a transaction */
////        union {
////            esp_err_t (*deinit)(void);  /*!< host function to deinitialize the driver */
////            esp_err_t (*deinit_p)(int slot);  /*!< host function to deinitialize the driver, called with the `slot` */
////        };
////        esp_err_t (*io_int_enable)(int slot); /*!< Host function to enable SDIO interrupt line */
////        esp_err_t (*io_int_wait)(int slot, TickType_t timeout_ticks); /*!< Host function to wait for SDIO interrupt line to be active */
////        int command_timeout_ms;     /*!< timeout, in milliseconds, of a single command. Set to 0 to use the default value. */
////    } sdmmc_host_t;    ;
    slot = host.slot;
}; /* Host::Host(const Host&) */
// for lvalue object (defined variable)
Host::Host(const sdmmc_host_t& host)
{
    //    typedef struct {
    //        uint32_t flags;             /*!< flags defining host properties */
    cfg.flags = host.flags;	/*!< flags defining host properties */
    //    #define SDMMC_HOST_FLAG_1BIT    BIT(0)      /*!< host supports 1-line SD and MMC protocol */
    //    #define SDMMC_HOST_FLAG_4BIT    BIT(1)      /*!< host supports 4-line SD and MMC protocol */
    //    #define SDMMC_HOST_FLAG_8BIT    BIT(2)      /*!< host supports 8-line MMC protocol */
    //    #define SDMMC_HOST_FLAG_SPI     BIT(3)      /*!< host supports SPI protocol */
    //    #define SDMMC_HOST_FLAG_DDR     BIT(4)      /*!< host supports DDR mode for SD/MMC */
    //    #define SDMMC_HOST_FLAG_DEINIT_ARG BIT(5)      /*!< host `deinit` function called with the slot argument */
    //        int slot;                   /*!< slot number, to be passed to host functions */
    cfg.slot = host.slot;	/*!< slot number, to be passed to host functions */
    //        int max_freq_khz;           /*!< max frequency supported by the host */
    cfg.max_freq_khz = host.max_freq_khz;	/*!< max frequency supported by the host */
    //    #define SDMMC_FREQ_DEFAULT      20000       /*!< SD/MMC Default speed (limited by clock divider) */
    //    #define SDMMC_FREQ_HIGHSPEED    40000       /*!< SD High speed (limited by clock divider) */
    //    #define SDMMC_FREQ_PROBING      400         /*!< SD/MMC probing speed */
    //    #define SDMMC_FREQ_52M          52000       /*!< MMC 52MHz speed */
    //    #define SDMMC_FREQ_26M          26000       /*!< MMC 26MHz speed */
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
    //    } sdmmc_host_t;    ;
    if (host.slot == 0)
	slot = Slot(Slot::_0);
}; /* Host::Host(const sdmmc_host_t&) */

// for rvalue oblect (e.g. temporary object)
Host::Host(sdmmc_host_t&& host) noexcept:
	Host(host)
{
    ;
}; /* Host::Host(sdmmc_host_t&&) noexcept */


//Host();	// Default constructor
//Host(Slot::number);	// Constructor with default slot configuration by number of the slot
//// Custom slot configuration in temporary obj for desired slot number
//Host(Slot::number, Slot&&);	// in temporary object
//Host(Slot::number, const Slot&);	// in lvalue object
//// Copy constructors
//Host(const Host&);		// for lvalue object (defined variable)
//Host(const sdmmc_host_t&);	// for lvalue object (defined variable)
//Host(sdmmc_host_t&&) noexcept; // for rvalue oblect (e.g. temporary object)

//sdmmc_host_t& operator =(const Host&);
//sdmmc_host_t& operator =(const sdmmc_host_t&);
//sdmmc_host_t& operator =(sdmmc_host_t&&) noexcept;


//esp_err_t Host::init(int slotno, const sdmmc_slot_config_t *slot_config)
esp_err_t Host::init(Slot::number slotno, const Slot& extern_slot)
{
    cfg.slot = slotno;
    slot = extern_slot;
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

//    typedef struct {
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
//    } sdmmc_slot_config_t;

}; /* Slot::Slot(const sdmmc_slot_config_t&) */

// temporary object copy constructor
Slot::Slot(sdmmc_slot_config_t&& config):
	Slot(config)
{
    /*;*/
}; /* Slot::Slot(sdmmc_slot_config_t&&) */

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


//--[ struct Device ]-----------------------------------------------------------------------------------------------

Device::Device():
	target(MOUNT_POINT_Default)
{
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
	mnt.format_if_mount_failed = true;
#else
	mnt.format_if_mount_failed = false;
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
	mnt.max_files = 5;
	mnt.allocation_unit_size = 16 * 1024;
	//ESP_LOGI(TAG, "Initializing SD card");
}; /* Device::Device */

const char *Device::MOUNT_POINT_Default = MOUNT_POINT_def;


//--[ class Card ]------------------------------------------------------------------------------------------------

#define CMD_TAG_PRFX "SD/MMC Card::"


// Print the card info
void Card::print_info(FILE* outfile)
{
    sdmmc_card_print_info(outfile, data);
    fprintf(outfile, "Sector: %d Bytes\n\n", data->csd.sector_size);
}; /* Card::print_info */


// print the SD-card info (wrapper for the external caller)
esp_err_t Card::info()
{
    print_info(stdout);
    return ESP_OK;
}; /* Card::info */

const char* Card::TAG = "SD/MMC Card";



#if 0
//--[ class Server ]------------------------------------------------------------------------------------------------

#undef CMD_TAG_PRFX
#define CMD_TAG_PRFX "SD/MMC CMD server::"

    // Mount SD-card with default parameters
    esp_err_t Server::mount(SDMMC::Card& mcard)
    {
	mounting.target_reset();
	host.cfg.slot = host.slot.default_num();
	cout << "Mount SD-card with default parameters" << endl;
	cout << "Mount card slot #" << host.cfg.slot << " to path \"" << mounting.target << "\"." << endl
		<< endl;
//	cout << TAG << ": " << "Procedure \"Mount\" is not yet released now" << endl;
//	cout  << endl;
	//return mount(mcard, host.slot.default_num(), mounting.MOUNT_POINT_Default);
	return _mount_implement(mcard);
    }; /* Server::mount */

    // Mount default SD-card slot onto path "mountpoint"
    esp_err_t Server::mount(SDMMC::Card &mcard, char mountpoint[])
    {
	if (isdigit(mountpoint[0]))
	    return mount(mcard, atoi(mountpoint));

	mounting.target = mountpoint;
	host.cfg.slot = host.slot.default_num();

	cout << "Mount default SD-card slot onto specified path" << endl;
	cout << "Mount card slot #" << host.cfg.slot << " to path \"" << mounting.target << "\"" << endl
		<< endl;
//	cout << TAG << ": " << "Procedure \"Mount(<mountpath>)\" is not yet released now" << endl;
//	cout << endl;
	//return mount(mcard, host.slot.default_num(), mountpoint);
	return _mount_implement(mcard);
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto default mount path
    esp_err_t Server::mount(SDMMC::Card &mcard, int slot_no)
    {
	mounting.target_reset();
	host.cfg.slot = slot_no;
//	cout << "Mount SD-card in specified slot onto default mount path" << endl;
//	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
//		<< endl;
	cout << TAG << ": " << "Procedure \"Mount(<slot_number>)\" is not yet released now" << endl;
	cout /*<< "Exit..."*/ << endl;
	//return mount(slot_no, mounting.MOUNT_POINT_Default);
	return _mount_implement(mcard);
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto specified mount path
    esp_err_t Server::mount(SDMMC::Card &mcard, int slot_no, const char mountpoint[])
    {
//	card = &mcard;
	mounting.target = mountpoint;
	host.cfg.slot = slot_no;

	// Enable internal pullups on enabled pins. The internal pullups
	// are insufficient however, please make sure 10k external pullups are
	// connected on the bus. This is for debug / example purpose only.
	// slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
	//ESP_LOGI(TAG, "Mounting filesystem");

	cout << "Mount SD-card in specified slot onto specified mount path" << endl;
//	cout << "Mount card slot #" << control.host.slot << " to path \"" << mounting.target << "\"" << endl
//		<< endl;
	cout << TAG << ": " "Mounting filesystem in card slot #" << host.cfg.slot << " to path \"" << mounting.target << "\"" << endl
		<< endl;
	return _mount_implement(mcard);

//	ret = esp_vfs_fat_sdmmc_mount(mounting.target, &host.cfg, &host.slot.cfg, &mounting.cfg, &card);
//	if (ret != ESP_OK)
//	{
//	    if (ret == ESP_FAIL)
//		cout << TAG << ": " << "Failed to mount filesystem. "
//			"If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.";
//	    else
//		cout << TAG << ": " << "Failed to initialize the card (error " << ret << ", " << esp_err_to_name(ret) << "). "
//			<< "Make sure SD card lines have pull-up resistors in place.";
//	    return ret;
//	}; /* if ret != ESP_OK */
//
//	//ESP_LOGI(TAG, "Filesystem mounted");
//	cout << TAG << ": " "Filesystem mounted";
//
//	return ret;
    }; /* Server::mount */

    esp_err_t Server::_mount_implement(SDMMC::Card &mcard)
    {
	card = &mcard;

	ret = esp_vfs_fat_sdmmc_mount(mounting.target, &host.cfg, &host.slot.cfg, &mounting.cfg, &card->data);
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
    }; /*  */



    //------------------------------------------------------------------------------------------
    //    // All done, unmount partition and disable SDMMC peripheral
    //    esp_vfs_fat_sdcard_unmount(mount_point, card);
    //    ESP_LOGI(TAG, "Card unmounted");
    //------------------------------------------------------------------------------------------

    // Unmount SD-card, that mounted onto "mountpath"
    esp_err_t Server::unmount(const char mountpath[])
    {
	if (mountpath == NULL || strcmp(mountpath, "") == 0)
	{
	    cout << TAG << ": " << "Call: unmount(" << mounting.target << ");" << endl;
	    ret = unmount(mounting.target);
	    return ret;
	}; /*  */
	ret = esp_vfs_fat_sdcard_unmount(mountpath, card->data);
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
void Server::print_info(FILE* outfile)
{
    sdmmc_card_print_info(outfile, card->data);
    fprintf(outfile, "Sector: %d Bytes\n\n", card->data->csd.sector_size);
}; /* Server::print_info */


// print the SD-card info (wrapper for the external caller)
esp_err_t Server::info()
{
    print_info(stdout);
    return ESP_OK;
}; /* Server::info */



// print current directory name
esp_err_t Server::pwd()
{
#ifdef __PURE_C__
	char* buf = getcwd(NULL, 0);
//	size_t buflen = sizeof(buf) + 1;

    if (!buf)
	return errno;
    cout << endl
	<< "PWD is: \"" << buf << '"' << endl
	<< endl;
    free(buf);

//    buf = (char*)malloc(buflen);
//    FRESULT res = f_getcwd(buf, buflen);
//    if (res != FR_OK)
//    {
//	cout << "Name of the current working directory was not copied into buffer." << endl
//		<< "Return code is: " << res << endl;
//    }; /* if res != FR_OK */
//
//    cout << aso::format("Current dir by f_getcwd version is: \"%s\"") % buf << endl;
//    free(buf);

    return ESP_OK;
#else
    cout << "Command \"pwd\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::pwd */


#define CMD_NM "mkdir"
// create a new directory
esp_err_t Server::mkdir(const char dirname[])
{
    if (dirname == NULL || strcmp(dirname, "") == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "invoke command \"%s\" without parameters.\n%s", CMD_NM,
		"This command required the creating directory name.");
	return ESP_ERR_INVALID_ARG;
    }; /* if dirname == NULL || strcmp(dirname, "") */
#ifdef __PURE_C__

#define __SIMPLE_MK__
#ifdef __SIMPLE_MK__
    errno = 0;
    ::mkdir(dirname, /*0777*/ S_IRWXU | S_IRWXG | S_IRWXO);
#else
	const char *buf;
	char *extrbuf = NULL;
    if (dirname[strlen(dirname)-1] == '/')
	buf = dirname;
    else
    {
	extrbuf = (char*)malloc(strlen(dirname) + 2);
	strcpy(extrbuf, dirname);
	extrbuf[strlen(dirname)] = '/';
	extrbuf[strlen(dirname)+1] = '\0';
	buf = extrbuf;
    }; /* else if (dirname[strlen(dirname)] == '/' */
    cout << aso::format("Create directory \"%s\"") % buf << endl;
    errno = 0;
    ::mkdir(buf, /*0777*/ S_IRWXU | S_IRWXG | S_IRWXO);
    free(extrbuf);
#endif // __SIMPLE_MK__
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error creating directory \"%s\": %s", dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if (errno) */
    return ESP_OK;

#else
    cout << "Change directory to " << '"' << dirname << '"' << endl;
    cout << "Command \"mkdir\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::mkdir */

#undef CMD_NM
#define CMD_NM "rmdir"
// create a new directory
esp_err_t Server::rmdir(const char dirname[])
{
    if (dirname == NULL || strcmp(dirname, "") == 0)
    {
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "invoke command \"%s\" without parameters.\n%s", CMD_NM,
		     "This command required the deleting directory name.");
	    return ESP_ERR_INVALID_ARG;
    }; /* if dirname == NULL || strcmp(dirname, "") */
#ifdef __PURE_C__
    cout << aso::format("Delete directory <%s>") % dirname << endl;

    // Check if destination directory or file exists before deleting
    struct stat st;
    if (stat(dirname, &st) != 0)
    {
        // deleting a non-existent directory is not possible
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Directory \"%s\" is not exist - deleting a non-existent catalogue is not possible.\n%s", dirname, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(file_foo, &st) != 0 */
    if (!S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "The %s command delete directories, not the files.\n%s", CMD_NM, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */

	DIR *dir = opendir(dirname);	// Directory descriptor

    errno = 0;	// clear any possible errors

	struct dirent *entry = readdir(dir);

    closedir(dir);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Fail when closing directory \"%s\": %s", dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */
    if (entry)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Directory \"%s\" is not empty, deletung non-emty directories is not supported.", dirname);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (entry) */

//    errno = 0;
    unlink(dirname);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Fail when deleting \"%s\": %s", CMD_NM, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */

    return ESP_OK;

#else
    cout << "Delete directory " << '<' << dirname << '>' << endl;
    //cout << aso::format("Command \"%s\" is not yet implemented now for C++ edition.") % CND_NM << endl;
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::rmdir */



#undef CMD_NM
#define CMD_NM "cd"

// change a current directory
esp_err_t Server::cd(const char dirname[])
{
    if (dirname == NULL || strcmp(dirname, "") == 0)
    {
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "invoke command \"%s\" without parameters.\n%s", CMD_NM,
		     "This command required directory name to change.");
	    return ESP_ERR_INVALID_ARG;
    }; /* if dirname == NULL || strcmp(dirname, "") */
#ifdef __PURE_C__
    cout << "Change current dir to " << dirname << endl;
    chdir(dirname);
    if (errno != 0)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "fail change directory to %s\n%s", dirname, strerror(errno));
	//perror(CMD_TAG_PRFX CMD_NM);
	return ESP_FAIL;
    }; /* if errno != 0 */
    return ESP_OK;
#else
    cout << "Change directory to " << '"' << dirname << '"' << endl;
    cout << "Command \"" CMD_NM "\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::cd */




#undef CMD_NM
#define CMD_NM "ls"

//
// Listing the entries of the opened directory
// Parameters:
//	dir  - opened directory stream;
//	path - full path of this directory
// Return:
//	>=0 - listed entries counter;
//	<0  - error - -1*(ESP_ERR_xxx) or ESP_FAIL (-1) immediately
// Pure C edition
static int listing_direntries_pureC(DIR *dir, const char path[]);
// C++ edition
static int listing_direntries_Cpp(DIR *dir, const char path[]);


// print a list of files in the specified directory
esp_err_t Server::ls(const char pattern[])
{
//#if defined(__PURE_C__) || __cplusplus < 201703L
//#ifdef __PURE_C__
    	int entry_cnt = 0;
	DIR *dir;	// Directory descriptor

    dir = opendir(pattern);
    if (!dir) {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error opening directory <%s>, %s", pattern, strerror(errno));
	return ESP_FAIL;
    }; /* if !dir */

	esp_err_t ret = ESP_OK;

    printf("Files in the directory <%s>\n",  pattern);
    printf("----------------\n");

#ifdef __PURE_C__
    entry_cnt = listing_direntries_pureC(dir, pattern);
#else
    entry_cnt = listing_direntries_Cpp(dir, pattern);
#endif
    if (entry_cnt)
    {
	cout << "----------------" << endl;
	cout << aso::format("Total found %d files", entry_cnt) << endl;
    }
    else
	cout << "Files or directory not found, directory is empty." << endl;

    if (errno != 0)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error occured during reading of the directory <%s>, %s", pattern, strerror(errno));
	ret = ESP_FAIL;
    }; /* if errno != 0 */
    closedir(dir);
    cout << endl;
//#else
//#endif
    return ret;
}; /* Server::ls */


// Printout one entry of the dir, pure C edition
static void ls_entry_printout_pure_C(const char fullpath[], const char name[]);
// Printout one entry of the dir, C++ edition
static void ls_entry_printout_Cpp(const char fullpath[], const char name[]);


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
// Listing the entries of the opened directory
// Pure C edition
// Parameters:
//	dir  - opened directory stream;
//	path - full path of this directory
// Return:
//	>=0 - listed entries counter;
//	<0  - error - -1*(ESP_ERR_xxx) or ESP_FAIL (-1) immediately
int listing_direntries_pureC(DIR *dir, const char path[])
{
	char pathbuf[PATH_MAX + 1];
	char * fnbuf;
	int cnt = 0;

    if (realpath(path, pathbuf) == NULL)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error canonicalizing path \"<%s>\", %s", path, strerror(errno));
	return ESP_FAIL;
    }; /* if realpath(pattern, pathbuf) == NULL */

    errno = 0;	// clear any possible errors
    fnbuf = pathbuf + strlen(pathbuf);
    fnbuf[0] = '/';
    fnbuf++;

    for ( struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
	cnt++;
	strcpy(fnbuf, entry->d_name);

	ls_entry_printout_pure_C(pathbuf, entry->d_name);
    }; /* for entry = readdir(dir); entry != NULL; entry = readdir(dir) */
    return cnt;
}; /* listing_direntries_pureC */


// Listing the entries of the opened directory
// C++ edition
// Parameters:
//	dir  - opened directory stream;
//	path - full path of this directory
// Return:
//	>=0 - listed entries counter;
//	<0  - error - -1*(ESP_ERR_xxx) or ESP_FAIL (-1) immediately
//-Wunused-function
int listing_direntries_Cpp(DIR *dir, const char path[])
{
	char pathbuf[PATH_MAX + 1];
	char * fnbuf;
	int cnt = 0;

    if (realpath(path, pathbuf) == NULL)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error canonicalizing path \"<%s>\", %s", path, strerror(errno));
	return ESP_FAIL;
    }; /* if realpath(pattern, pathbuf) == NULL */

    errno = 0;	// clear any possible errors
    fnbuf = pathbuf + strlen(pathbuf);
    fnbuf[0] = '/';
    fnbuf++;

    for ( struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
	cnt++;
	strcpy(fnbuf, entry->d_name);
	ls_entry_printout_Cpp(pathbuf, entry->d_name);
    }; /* for entry = readdir(dir); entry != NULL; entry = readdir(dir) */
    return cnt;
}; /* listing_direntries_Cpp */
#pragma GCC diagnostic pop


void ls_entry_printout_pure_C(const char fullpath[], const char name[])
{
	struct stat statbuf;

    stat(fullpath, &statbuf);
    printf("%s\n\t%s %s\n", fullpath, name,
	    (S_ISLNK(statbuf.st_mode))? "[symlink]":
	    (S_ISREG(statbuf.st_mode))? "(file)":
	    (S_ISDIR(statbuf.st_mode))? "<DIR>":
	    (S_ISCHR(statbuf.st_mode))? "[char dev]":
	    (S_ISBLK(statbuf.st_mode))? "[blk dev]":
	    (S_ISFIFO(statbuf.st_mode))? "[FIFO]":
	    (S_ISSOCK(statbuf.st_mode))? "[socket]":
	    "[unknown type]");
}; /* ls_entry_printout_pure_C */

// Printout one entry of the dir, C++ edition
void ls_entry_printout_Cpp(const char fullpath[], const char name[])
{
	struct stat statbuf;

    stat(fullpath, &statbuf);
    cout << fullpath << endl
	<< aso::format("\t%s ") % name
	<< aso::format((S_ISDIR(statbuf.st_mode))? "<DIR>":
	    (S_ISREG(statbuf.st_mode))? "(file)": "[%s]",
		(S_ISLNK(statbuf.st_mode))? "symlink":
		(S_ISCHR(statbuf.st_mode))? "char dev":
		(S_ISBLK(statbuf.st_mode))? "blk dev":
		(S_ISFIFO(statbuf.st_mode))? "FIFO":
		(S_ISSOCK(statbuf.st_mode))? "socket":
			"unknown/other type") << endl;
}; /* ls_entry_printout_Cpp */



#undef CMD_NM
#define CMD_NM "cp"

// copy files according a pattern
esp_err_t Server::cp(const char src[], const char dest[])
{
    if (dest == NULL || strcmp(dest, "") == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "too few arguments: invoke command \"%s\" with one parameters.\n%s", CMD_NM,
		"Don't know where to copy?");
	return ESP_ERR_INVALID_ARG;
    }; /* if dest == NULL || strcmp(dest, "") */
    if (src == NULL || strcmp(src, "") == 0)
    {
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "too few arguments: invoke command \"%s\" without parameters.\n%s", CMD_NM,
		     "Don't know what to copy?");
	    return ESP_ERR_INVALID_ARG;
    }; /* if src == NULL || strcmp(src, "") */

    cout << aso::format("Copy file \"%s\" to \"%s.\"", src, dest) << endl;
#ifdef __PURE_C__
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now for C edition.", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
    //return ESP_OK;
#else
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now for C++ edition.", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::cp */



#undef CMD_NM
#define CMD_NM "mv"

// move files according a pattern
esp_err_t Server::mv(const char src[], const char dest[])
{
    if (dest == NULL || strcmp(dest, "") == 0)
    {
	ESP_LOGE("CMD_TAG_PRFX CMD_NM", "too few arguments: invoke command \"%s\" without parameters.\n%s", CMD_NM,
		"Don't know what to move?");
	return ESP_ERR_INVALID_ARG;
    }; /* if dest == NULL || strcmp(dest, "") */

    if (src == NULL || strcmp(src, "") == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "too few arguments: invoke command \"%s\" with one parameters.\n%s", CMD_NM,
		"Don't know where to move?");
	return ESP_ERR_INVALID_ARG;
    }; /* if src == NULL || strcmp(src, "") */

    cout << aso::format("Move file \"%s\" to \"%s.\"", src, dest) << endl;
#ifdef __PURE_C__
//    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now for C edition.", CMD_NM);
//    return ESP_ERR_INVALID_VERSION;
    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", src, dest);
    if (rename(src, dest) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return ESP_FAIL;
    }
    return ESP_OK;
#else
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now for C++ edition.", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::mv */



#undef CMD_NM
#define CMD_NM "rm"

// remove files according a pattern
esp_err_t Server::rm(const char pattern[])
{
    if (pattern == NULL || strcmp(pattern, "") == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "invoke command \"%s\" without parameters.\n%s", CMD_NM,
		"Missing filename to remove.");
	return ESP_ERR_INVALID_ARG;
    }; /* if pattern == NULL || strcmp(pattern, "") */

    cout << "Delete file " << '"' << pattern << '"' << endl;
#ifdef __PURE_C__

    // Check if destination file exists before deleting
    struct stat st;
    if (stat(pattern, &st) != 0)
    {
        // deleting a non-existent file is not possible
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "File \"%s\" is not exist - deleting a non-existent file is not possible.\n%s", pattern, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(file_foo, &st) != 0 */
    if (S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Deleting directories by command \"%s\" is impossible.\n%s", CMD_NM, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */
    errno = 0;
//    cout << "Now exec: ===>> " << "unlink(pattern)" << "<<===" << endl;
    unlink(pattern);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Faile when deleting \"%s\": %s", CMD_NM, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */

    return ESP_OK;
#else
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now for C++ edition.", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::rm */



#undef CMD_NM
#define CMD_NM "cat"

// type file contents
esp_err_t Server::cat(const char fname[])
{
    if (fname == NULL || strcmp(fname, "") == 0)
    {
	cout << endl
	     << "*** Printing contents of the file <XXXX fname>. ***" << endl
	     << endl;
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "invoke command \"%s\" without parameters.\n%s", CMD_NM,
		"Missing filename for print to output.");

	cout << "*** End of printing file XXXX. ** ******************" << endl;
	return ESP_ERR_INVALID_ARG;
    }; /* if fname == NULL || strcmp(fname, "") */

    cout << endl
	 << "*** Printing contents of the file <" << fname << ">. ***" << endl
	 << endl;

#ifdef __PURE_C__

	FILE *text = fopen(fname, "r"); // open the file for type to screen

    errno = 0;	// clear possible errors
    text = fopen(fname, "r"); // open the file for type to screen
    if (!text)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error opening file <%s>, %s", fname, strerror(errno));
	return ESP_FAIL;
    }; /* if !FILE */

    for (char c = getc(text); !feof(text); c = getc(text))
	putchar(c);

//    putchar('\n');
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error during type the file %s to output, %s", fname, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */

#else
#endif

    cout << endl
	 << "*** End of printing file " << fname << ". **************" << endl
	 << endl;
//    return ESP_ERR_INVALID_VERSION;
    return ESP_OK;
}; /* cat */



#undef CMD_NM
#define CMD_NM "type"

// type text from keyboard to screen
esp_err_t Server::type()
{
    cout << endl
	 << "**** Type the text on keyboard to screen *****" << endl
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

// Generates an error message if the struct stat
// refers to an object, other than a file.
// fname - the name of the object referenced by the struct stat.
static esp_err_t err4existent(const char fname[], const struct stat* statbuf);


// type text from keyboard to file and to screen
esp_err_t Server::type(const char fname[])
{
	FILE *storage = NULL;

    // Test file 'fname' for existing
    errno = 0;	// clear all error state
    if (access(fname, F_OK) == -1)
    {
	if (errno == ENOENT)	// error "file does not exist"
	{
		struct stat statbuf;

	    if (!stat(fname, &statbuf))	// but if the fname still exists here, then it is a directory
		return err4existent(fname, &statbuf);
	    ESP_LOGI(CMD_TAG_PRFX CMD_NM, "OK, file \"%s\" does not exist, opening this file.", fname);
	    cout << aso::format("Open file %s for the write") % fname << endl;
	    errno = 0;	// clear error state
	    storage = fopen(fname, "w");
	}
	else	// error other than "file does not exist"
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error test existing file %s: %s", fname , strerror(errno));
	    return ESP_FAIL;
	}
    } /* if stat(fname, &statbuf) == -1 */
    else
    {	// Error - file fname is exist

	    struct stat statbuf;
	    char c;

	// fname exists, check that is a regular file
	if (!stat(fname, &statbuf) && !S_ISREG(statbuf.st_mode))
	    return err4existent(fname, &statbuf);

	printf("File %s is exist.\nDo you want use this file? [yes(add)/over(write)/No]: ", fname);
	cin >> noskipws >> c;
	cout << c;

	if (c == '\n')
	    cout << "<LF>";
	cout << endl;

	switch (tolower(c))
	{
	case 'a':
	case 'y':
	    ESP_LOGI(CMD_TAG_PRFX CMD_NM, "OK, open the file %s to add.", fname);
	    cout << aso::format("File %s is opened for add+write.") % fname << endl;
	    storage = fopen(fname, "a");
	    break;

	case 'o':
	case 'w':
	    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "OK, open the file %s to owerwrite.", fname);
	    cout << aso::format("File %s is opened to truncate+write (overwrite).") % fname << endl;
	    storage = fopen(fname, "w");
	    break;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
	case '\n':
	    cout << "Enter char '\\n'" << endl;
	case 'n':
	    ESP_LOGW("console::type <filename>", "User cancel opening file %s.", fname);
	    return ESP_ERR_NOT_FOUND;
	    break;
#pragma GCC diagnostic pop

	default:
	    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Error: H.z. cho in input, input char value is: [%d]", (int)c);
	    return ESP_ERR_INVALID_ARG;
	}; /* switch tolower(c) */
    }; /* else if stat(fname, &statbuf) == -1 */

    if (storage == NULL)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Any error occured when opening the file %s: %s.", fname, strerror(errno));
	return ESP_ERR_NOT_FOUND;
    }; /* if storage == NULL */

//#define TYPEBUFSIZE (8 * 4*BUFSIZ)
#define TYPEBUFSIZE (4 * card->data->csd.sector_size)
//	char typebuf[TYPEBUFSIZE];
	char *typebuf = (char*)malloc(TYPEBUFSIZE);
//	char *typebuf = (char*)malloc( card->data->csd.sector_size);

    errno = 0;
    setvbuf(storage, typebuf, _IOFBF, TYPEBUFSIZE);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error when setting buffering mode for file %s: %s.", fname, strerror(errno));
	fclose(storage);
	return ESP_FAIL;
    }; /* if (errno) */


    cout << endl
	 << aso::format("**** Type the text on keyboard to screen and file [%s]. ****") % fname  << endl
	 << "Press <Enter> twice for exit..." << endl
	 << endl;

	char c = '\0', prevc;
    do {
	prevc = c;
	cin >> noskipws >> c;
	cout << c;
	fputc(c, storage);
    } while (c != prevc || c != '\n');

    cout << endl << endl
	 << "**** End of typing the text on keyboard. *****" << endl
	 << endl;


    cout << aso::format("Flush&write cache buffer of the file %s.") % fname << endl;
    fflush(storage);
    fsync(fileno(storage));
    cout << aso::format("Close the file %s.") % fname << endl;
    fclose(storage);
    free(typebuf);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Any error occured when closing the file %s: %s.", fname, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */


    cout << endl
	 << aso::format("**** End of typing the text on keyboard for the screen and the file %s. ****") % fname << endl
	 << endl;
    return ESP_OK;
//    return ESP_ERR_INVALID_VERSION;
}; /* Server::type <file> */


// Generates an error message if the struct stat
// refers to an object, other than a file.
// fname - the name of the object referenced by the struct stat.
esp_err_t err4existent(const char fname[], const struct stat* statbuf)
{
#define EXIST_FN_TAG "console::type exist chechk"
    ESP_LOGE(EXIST_FN_TAG, "Error: path %s exist, and is not a file, but a %s.\nOperation is not permitted.",
	    fname, (S_ISLNK(statbuf->st_mode))? "symlink":
		    (S_ISDIR(statbuf->st_mode))? "directory":
		    (S_ISCHR(statbuf->st_mode))? "character device":
		    (S_ISBLK(statbuf->st_mode))? "block device":
		    (S_ISFIFO(statbuf->st_mode))? "FIFO channel":
		    (S_ISSOCK(statbuf->st_mode))? "socket":
			    "(unknown type)");
    return ESP_ERR_NOT_SUPPORTED;
#undef EXIST_FN_TAG
}; /* err4existent */

#undef CMD_TAG_PRFX


    const char* Server::TAG = "SD/MMC service";
#endif

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