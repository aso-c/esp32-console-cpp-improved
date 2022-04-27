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

#include "cmd_sdcard.hpp"

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




// Register command procedure
static void register_cmd(const esp_console_cmd_t*);

extern "C" {
// Procedure of the 'sd' command
static int sdcard_cmd(int argc, char **argv);
}


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


namespace SDMMC
{

static const char *TAG = "SD/MMC service";

//// SD/MMC card class
//class card
//{
//public:
//    card() {card = NULL;};
//    void Init() {};
//    ~card();
//private:
//    sdmmc_card_t *card;
//}; /* card */


//class SDMMC_host
struct Host
{
    Host()
    {
	ESP_LOGI(TAG, "Using SDMMC peripheral");
    }; /* Host */

//    // Use settings defined above to initialize SD card and mount FAT filesystem.
//    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
//    // Please check its source code and implement error recovery when developing
//    // production applications.
//    void Init() {
//	ESP_LOGI(TAG, "Using SDMMC peripheral");
//	instance = SDMMC_HOST_DEFAULT();
//    }; /* Init() */

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_host_t instance = SDMMC_HOST_DEFAULT();
}; /* struct Host */


struct Slot
{
    Slot()
    {
	    // This initializes the slot without card detect (CD) and write protect (WP) signals.
	    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	    //config = SDMMC_SLOT_CONFIG_DEFAULT();

	    // To use 1-line SD mode, change this to 1:
	    config.width = SLOT_WIDTH;

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
	//    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
	    config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    }; /* Slot */

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    // use field name 'config' instead 'slot_config'
    sdmmc_slot_config_t config = SDMMC_SLOT_CONFIG_DEFAULT();

private:
    // To use 1-line SD mode, change this to 1:
    static const int SLOT_WIDTH = 4;

}; /* struct Slot */


// Options for mounting the filesystem.
// If format_if_mount_failed is set to true, SD card will be partitioned and
// formatted in case when mounting fails.
struct Mounter
{

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    Mounter():
	point(MOUNT_POINT)
    {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
	config.format_if_mount_failed = true;
#else
	config.format_if_mount_failed = false;
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
	config.max_files = 5;
	config.allocation_unit_size = 16 * 1024;
	//ESP_LOGI(TAG, "Initializing SD card");
    }; /* Mount */
//    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
//    #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
//        .format_if_mount_failed = true,
//    #else
//        .format_if_mount_failed = false,
//    #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
//        .max_files = 5,
//        .allocation_unit_size = 16 * 1024
//    };

    esp_vfs_fat_sdmmc_mount_config_t config;
    const char* point;
    //ESP_LOGI(TAG, "Initializing SD card");

private:
    static const char* MOUNT_POINT;

}; /* struct Mounter */

const char *Mounter::MOUNT_POINT = MOUNT_POINT_def;


class Server
{

    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
//    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
//    #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
//        .format_if_mount_failed = true,
//    #else
//        .format_if_mount_failed = false,
//    #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
//        .max_files = 5,
//        .allocation_unit_size = 16 * 1024
//    };
    //
//    char* mount_point/* = MOUNT_POINT*/;
//    //ESP_LOGI(TAG, "Initializing SD card");

private:
//    static const char* MOUNT_POINT;
    static const char* TAG;
    sdmmc_card_t *card;

}; /* class Server */

//const char *server::MOUNT_POINT = MOUNT_POINT_def;
const char* Server::TAG = "SD/MMC service";

}; /* namespace SDMMC */


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






// argument tables for any subcommand of sdcard commqand
//static void
//    *args_help[1+1],	// h | help
//    *args_mount[3+1],	// m | mount [<device>] [<mountpoint>]
//    *args_umount[6+1],	// u | umount [ <device> | <mountpoint> ]
//    *args_ls[2+1],	// ls | dir [<pattern>]
//    *args_cat[2+1],	// cat <filename>
//    *args_type[4+1];	// type [<filename>]

//static struct
//{
//    void *help[1+1],	// h | help
//	*mount[3+1],	// m | mount [<device>] [<mountpoint>]
//	*umount[6+1],	// u | umount [ <device> | <mountpoint> ]
//	*ls[2+1],	// ls | dir [<pattern>]
//	*cat[2+1],	// cat <filename>
//	*type[4+1];	// type [<filename>]
//} args;

// classs of the SD control command implementation
class SDctrl
{
public:

    void store(int argc, char *argv[]);	// Initializing current command environment
    static SDctrl& cmd();// get unigue single instance of the SDcmd object
    static int exec(int argc, char **argv);	// execute the 'SD' command

    int err_none();	// Handler for "subcommand missing" error.
    int err_unknown();	// Handler for "subcommand unknown" error.

    int act_mnt();	// action for 'mount' command
    int act_umnt();	// action for 'unmount' command
    int act_ls();	// action for list/dir command
    int act_cat();	// action for 'cat' command
    int act_type();	// action for 'type' command

private:
    int argc;
    char **argv;


    SDctrl();		// Default constructor - private for singleton
    SDctrl(SDctrl&) = delete;	// copy constructor forbidden for singleton
    SDctrl& operator =(const SDctrl&) = delete;	// operator "=" - forbidden for singleton

    // Contains Syntax tables for subcommand for 'sdcard' command
    class Syntax
    {
    public:

	// sucommand id
	enum cmd_id { none, mount, unmount, ls, cat, type, helping, unknown = -1 };

	static Syntax& get();
	cmd_id id();	// Return subcommand id
	int help();
	static void** tables();
	static ostream& hint(ostream&);	// hint for the command - suggest to see help

    private:
	Syntax();
	~Syntax();

	Syntax(Syntax&) = delete;	// blocking of copy constructor
	Syntax& operator =(const Syntax&) = delete;	// blocking of operator '='

	static SDctrl& parent;
	static void** alltables;	// for initializing singleton at the initial phase of programm
    };

    static SDctrl& instance;    // Unique single instance of the SDcmd object
    static Syntax& syntax;	// reference to inner 'syntax' object, contain`s all syntax tables

}; /* SDctrl */



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

//    SDctrl::cmd().store(argc, argv);
    return SDctrl::exec(argc, argv);

}; /* sd_cmd */
//}



//--[ class SDcmd ]-------------------------------------------------------------


// Default constructor
SDctrl::SDctrl():
	argc(0),
	argv(nullptr)
{ /*InitSyntaxs();*/ };


// get unigue single instance of the SDcmd object
SDctrl& SDctrl::cmd()
{
	static SDctrl instance;
    return instance;
}

// Unique single instance of the SDcmd object
SDctrl& SDctrl::instance = SDctrl::cmd();


// execute the 'SD' command
int SDctrl::exec(int argc, char **argv)
{

    instance.store(argc, argv);

    if (argc == 1)
	return instance.err_none();

    switch (syntax.id())
    {
//    case SDctrl::Syntax::mount:
    case Syntax::mount:
	return instance.act_mnt();

//    case SDctrl::Syntax::unmount:
    case Syntax::unmount:
	return instance.act_umnt();

//    case SDctrl::Syntax::ls:
    case Syntax::ls:
	return instance.act_ls();

//    case SDctrl::Syntax::cat:
    case Syntax::cat:
	return instance.act_cat();

//    case SDctrl::Syntax::type:
    case Syntax::type:
	return instance.act_type();

//    case SDctrl::Syntax::helping:
    case Syntax::helping:
	return syntax.help();

//    case SDctrl::Syntax::unknown:
    case Syntax::unknown:
    default:
	return instance.err_unknown();
    }; /* switch instance.id() */

    cout << endl;
    return 0;
}; /* SDctrl::exec */



// Initializing current command environment
void SDctrl::store(int argcnt, char *argvalue[])
{
    argc = argcnt;
    argv = argvalue;
}; /* SDcmd::store */


// Handler for "subcommand missing" error.
int SDctrl::err_none()
{
    cout << "Subcommand missing." << endl
	 << syntax.hint << endl;
    return 0;
}; /* SDcmd::err_none */

// Handler for "subcommand unknown" error.
int SDctrl::err_unknown()
{
    cout << "Unknown options: \"" << argv[1] <<  "\"." << endl
	 << syntax.hint << endl;
    return 0;
}; /* SDcmd::err_unknown */


// action for 'mount' command
int SDctrl::act_mnt()
{
    cout << "\"mount\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - use default values." << endl;
	break;

    case 3:
	cout << "...with one parameter - use device or mount point." << endl;
	break;

    case 4:
	cout << "...with two parameters - use device & mount point." << endl;
	break;

    default:
	cout << "more than two parameters - unknown configuration of parameters." << endl;
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_mnt */


// action for 'unmount' command
int SDctrl::act_umnt()
{
    cout << "\"unmount\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - use default values." << endl;
	break;

    case 3:
	cout << "...with one parameter - use device or mount point." << endl;
	break;

    default:
	cout << "more than one parameter - unknown configuration of parameters." << endl;
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_umnt */


// action for list/dir command
int SDctrl::act_ls()
{
    cout << "\"ls\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - use current dir." << endl;
	break;

    case 3:
	cout << "...with one parameter - use pattern or directory." << endl;
	break;

    default:
	cout << "more than one parameter - unknown parameters config." << endl;
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_ls */


// action for 'cat' command
int SDctrl::act_cat()
{
    cout << "\"cat\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - error parameter values." << endl;
	break;

    case 3:
	cout << "...with one parameter - use file name." << endl;
	break;

    default:
	cout << "more than one parameter - error in parameters value." << endl;
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_cat */


// action for 'type' command
int SDctrl::act_type()
{
    cout << "\"type\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - type to screen only." << endl;
	break;

    case 3:
	cout << "...with one parameter - save type output to file & screen." << endl;
	break;

    default:
	cout << "more than one parameter - error in parameters value." << endl;
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_type */




// syntax table storage
SDctrl::Syntax& SDctrl::syntax = SDctrl::Syntax::get();


//--[ Inner class of the Syntax Contains Syntax tables for subcommand for 'sdcard' command ]---

SDctrl::Syntax::Syntax()
{
    cout << "<<< In Constructor SDcmd::Syntax::Syntax()                          >>>" << endl;
    cout << "<<< Create the singleton object of the Syntax class (SDcmd::Syntax) >>>" << endl;
    alltables = tables();
}; /* SDctrl::Syntax::Syntax */


SDctrl::Syntax::~Syntax()
{
//    // free the alltables
////    for (void** tbl = alltables; tbl; tbl++)
//    for (int i = 0; alltables[i] != nullptr; i++)
//	arg_freetable(alltables[i],sizeof(argtable1)/sizeof(argtable1[0]));
////    arg_freetable(argtable1,sizeof(argtable1)/sizeof(argtable1[0]));
}; /* SDctrl::Syntax::~Syntax() */

// Rererence to parent SDcmd object, that is the singleton
SDctrl& SDctrl::Syntax::parent = SDctrl::cmd();


SDctrl::Syntax& SDctrl::Syntax::get()
{
	static Syntax instance;
    return instance;
}; /* SDctrl::Syntax::get */


// hint for the command - suggest to see help
ostream& SDctrl::Syntax::hint(ostream& out)
{
    out << "Try \"" << parent.argv[0] << " help\" for more information.";
    return out;
}; /* SDctrl::Syntax::hint */




void** SDctrl::Syntax::tables()
{
    if (!alltables)
    {
    cout << "***********************************************************************" << endl;
    cout << "*** Initializing the Syntax Tables at the Start                     ***" << endl;
    cout << "***********************************************************************" << endl;


    cout << "***                                                                 ***" << endl
	 << "*** Start the Initializing the Syntax Tables in tables().           ***" << endl
	 << "***                                                                 ***" << endl;
    }; /* if !alltables */

    // syntax0: h | help
	static void* arg_help[] = {
		arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'"),
		arg_end(2),
	};

    // syntax1: m | mount [<device>] [<mountpoint>] "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters are optional"
	static void* arg_mnt[] = {
		arg_rex1(NULL, NULL, "m|mount", NULL, 0, "mount SD-card [<device>] to [<mountpoint>], parameters are optional"),
		arg_str0(NULL, NULL, "<device>", NULL),
		arg_str0(NULL, NULL, "<mountpoint>", NULL),
		arg_end(2),
	};
    // syntax2: u | umount [ <device> | <mountpoint> ] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
	static void* arg_umnt[] = {
		arg_rex1(NULL, NULL, "u|umount", NULL, 0, "unmount SD-card <device> or <path> where the SD card is mounted; if parameters omitted - use \"" MOUNT_POINT_def "\"" ),
		arg_rem ("[", NULL),
		arg_str1(NULL, NULL, "<device>", "SD card device name, used default value if omitted"),
		arg_rem ("|", NULL),
		arg_str1(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, used default value if omitted"),
		arg_rem ("]", NULL),
		arg_end(2),
	};
    // syntax3: ls | dir [<pattern>] "list directory contents on SD-card"
	static void* arg_ls[] = {
	//	arg_rex1(NULL, NULL, "ls|dir", NULL, 0, NULL),
		arg_rex1(NULL, NULL, "ls|dir", NULL, 0, "list directory contents on SD-card"),
	//	arg_str0(NULL, NULL, "<pattern>", "file pattern or path whose contents are printed"),
		arg_str0(NULL, NULL, "<pattern>", "file pattern or path"),
		arg_end(2),
	};
    // syntax4: cat <filename> "print file to stdout (console output)"
	static void* arg_cat[] = {
		arg_rex1(NULL, NULL, "cat", NULL, 0, NULL),
		arg_str1(NULL, NULL, "<file>", NULL),
		arg_end(2),
	};
    // syntax5: type [filename] "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only"
	static void* arg_type[] = {
		arg_rex1(NULL, NULL, "type", NULL, 0, "type from the keyboard to file & screen or screen only; if file omitted - print to screen only"),
		arg_str0(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved"),
		arg_end(2),
	};

	static void* syntaxes[] = {
		arg_mnt,
		arg_umnt,
		arg_ls,
		arg_cat,
		arg_type,
		arg_help,
		NULL
	};

//    cout << "**  Get the Syntax Tables in SDctrl::Syntax::tables().  **" << endl;
    return syntaxes;

}; /* SDctrl::Syntax::tables */

// Return subcommand id
SDctrl::Syntax::cmd_id
SDctrl::Syntax::id()
{
    if (parent.argc < 2)
	return none;
    if (isempty(parent.argv[1]))
	return none;
    if (strcmp(parent.argv[1], "help") == 0 || strcmp(parent.argv[1], "h") == 0)
    	return helping;
    if (strcmp(parent.argv[1], "mount") == 0 || strcmp(parent.argv[1], "m") == 0)
    	return mount;
    if (strcmp(parent.argv[1], "umount") == 0 || strcmp(parent.argv[1], "u") == 0)
	return unmount;
    if (strcmp(parent.argv[1], "ls") == 0 || strcmp(parent.argv[1], "dir") == 0)
	return ls;
    if (strcmp(parent.argv[1], "cat") == 0)
    	return cat;
    if (strcmp(parent.argv[1], "type") == 0)
    	return type;

    return unknown;
}; /* SDctrl::Syntax::id */



int SDctrl::Syntax::help()
{
//    return help_action((void**)alltables[0], alltables[1], alltables[2], alltables[3], alltables[4], alltables[5], NULL);

    cout << "#### Help action, implemented in the SDcmd::Syntax class, method help(). ####" << endl;

    if (!tables())
    {
	cout << "!!! Error: syntax tables is undefined. !!!" << endl;
	cout << "Abort command" << endl;
	return -2;
    }; /* if !hlp_arg */

    if (!tables()[0])
    {
	cout << "!!! Error: syntax tables for 1'st command is undefined. !!!" << endl;
	cout << "Abort command" << endl;
	return -1;
    }; /* if !tables()[0] */

    cout << "Usage: " << parent.argv[0];
    arg_print_syntax(stdout, (void**)alltables[0], "\n");

    for (void **currcmd = tables() + 1; *currcmd != NULL; currcmd++)
    {
	cout << "       " << parent.argv[0];
	arg_print_syntax(stdout, (void**)*currcmd, "\n");
    }; /* for void **currcmd */

    cout << "Command \"" << parent.argv[0] << "\" supports the ESP32 operation with an SD card." << endl;
    cout << "Use subcommands to invoke individual operations; operation are: mount, unmount, ls, cat, type, help." << endl;

    for (void **currcmd = tables(); *currcmd != NULL; currcmd++)
	arg_print_glossary(stdout, (void**)*currcmd, "      %-20s %s\n");

    return 0;

}; /* SDctrl::Syntax::help */


// for initializing singleton at the initial phase of programm
void** SDctrl::Syntax::alltables = nullptr;






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
