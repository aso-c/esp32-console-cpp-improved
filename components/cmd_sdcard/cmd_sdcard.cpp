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
    int act_help();	// help_action was implements actions for help

    // sucommand id
    enum cmd_id { none, mount, unmount, ls, cat, type, help, unknown = -1 };

    // Return subcommand id
    cmd_id id();

    // Recommendation to see a help
    class HelpHint
    {
    public:
	HelpHint(SDctrl*);
	void operator()(ostream&) const;

    private:
	SDctrl* ownsd;
    }; /* HelpHint */

    HelpHint help_hint;

    friend ostream&
	operator << (ostream&, const HelpHint&);

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
	static Syntax& get();
	int help();
	static void** tables();
    private:
	Syntax();
//	int InitSyntaxes();	    // Initialize all syntax tables

	int help_action(void* hlp_arg[],...);	// inner release of the help action implements

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
	help_hint(this),
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

    switch (instance.id())
    {
    case SDctrl::mount:
    case SDctrl::unmount:
    case SDctrl::ls:
    case SDctrl::cat:
    case SDctrl::type:
	cout << "Command" << '\'' << argv[0] << ' ' << argv[1] << '\''
	    << " is not yet implemented now." << endl;
	break;

    case SDctrl::help:
//	return instance.act_help();
	return syntax.help();

    case SDctrl::unknown:
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


// syntax table storage
//void** SDctrl::all_syntaxes = NULL;
SDctrl::Syntax& SDctrl::syntax = SDctrl::Syntax::get();


// help_action was implements actions for help
int SDctrl::act_help()
{
    return syntax.help();
}; /* SDcmd::act_help */



// Return command id
SDctrl::cmd_id SDctrl::id()
{
    if (argc < 2)
	return none;
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
}; /* SDctrl::id */



// Handler for "subcommand missing" error.
int SDctrl::err_none()
{
    cout << "Subcommand missing." << endl
	 << help_hint << endl;
    return 0;
}; /* SDcmd::err_none */

// Handler for "subcommand unknown" error.
int SDctrl::err_unknown()
{
    cout << "Unknown options: \"" << argv[1] <<  "\"." << endl
	 << help_hint << endl;
    return 0;
}; /* SDcmd::err_unknown */


ostream& operator << (ostream& out, const SDctrl::HelpHint& hint)
{
    hint(out);
    return out;
}; /* ostream& operator << */



//--[ Inner class of prompter, that recommend to see a help ]-------------------

// Constructor
SDctrl::HelpHint::HelpHint(SDctrl* owner):
	ownsd(owner)
{};

void
SDctrl::HelpHint::operator()(ostream& out) const
{
    out << "Try \"" << ownsd->argv[0] << " help\" for more information.";
}; /* SDcmd::HelpHint::operator() */



//--[ Inner class of the Syntax Contains Syntax tables for subcommand for 'sdcard' command ]---

//SDctrl::Syntax::Syntax(SDctrl& pater): parent(pater)
SDctrl::Syntax::Syntax()
{
    cout << "<<< In Constructor SDcmd::Syntax::Syntax()                          >>>" << endl;
    cout << "<<< Create the singleton object of the Syntax class (SDcmd::Syntax) >>>" << endl;
    alltables = tables();
}; /* SDctrl::Syntax::Syntax */


// Rererence to parent SDcmd object, that is the singleton
SDctrl& SDctrl::Syntax::parent = SDctrl::cmd();


SDctrl::Syntax& SDctrl::Syntax::get()
{
	static Syntax instance;
    return instance;
}; /* SDctrl::Syntax::get */



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
//    args_help[0] = arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'");
		arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'"),
//    args_help[1] = arg_end(2);
		arg_end(2),
	};

    // syntax1: m | mount [<device>] [<mountpoint>] "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters are optional"
	static void* arg_mnt[] = {
//    args_mount[0] = arg_rex1(NULL, NULL, "m|mount", NULL, 0, NULL);
		arg_rex1(NULL, NULL, "m|mount", NULL, 0, NULL),
//    args_mount[1] = arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ...");
		arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use default value"),
//    args_mount[2] = arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use ...");
		arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use default value"),
//    args_mount[3] = arg_end(2);
		arg_end(2),
	};
    // syntax2: u | umount [ <device> | <mountpoint> ] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
	static void* arg_umnt[] = {
//    args_umount[0] = arg_rex1(NULL, NULL, "u|umount", NULL, 0, NULL);
		arg_rex1(NULL, NULL, "u|umount", NULL, 0, NULL),
//    args_umount[1] = arg_rem ("[", NULL);
		arg_rem ("[", NULL),
//    args_umount[2] = args_mount[1]; // arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ...");
		arg_mnt[1], // arg_str0(NULL, NULL, "<device>", "SD card device name, if omitted - use ...");
//    args_umount[3] = arg_rem ("|", NULL);
		arg_rem ("|", NULL),
//    args_umount[4] = args_mount[2]; // arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use ...");
		arg_mnt[2], // arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, if omitted - use ...");
//    args_umount[5] = arg_rem ("]", NULL);
		arg_rem ("]", NULL),
//    args_umount[6] = arg_end(2);
		arg_end(2),
	};
    // syntax3: ls | dir [<pattern>] "list directory contents on SD-card"
	static void* arg_ls[] = {
//    args_ls[0] = arg_rex1(NULL, NULL, "ls|dir", NULL, 0, NULL);
		arg_rex1(NULL, NULL, "ls|dir", NULL, 0, NULL),
//    args_ls[1] = arg_str0(NULL, NULL, "<pattern>", "pattern or path in SD-card of the listed files in directory");
		arg_str0(NULL, NULL, "<pattern>", "pattern or path in SD-card of the listed files in directory"),
//    args_ls[2] = arg_end(2);
		arg_end(2),
	};
    // syntax4: cat <filename> "print file to stdout (console output)"
	static void* arg_cat[] = {
//    args_cat[0] = arg_rex1(NULL, NULL, "cat", NULL, 0, NULL);
		arg_rex1(NULL, NULL, "cat", NULL, 0, NULL),
//    args_cat[1] = arg_str1(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved");
		arg_str1(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved"),
//    args_cat[2] = arg_end(2);
		arg_end(2),
	};
    // syntax5: type [filename] "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only"
	static void* arg_type[] = {
//    args_type[0] = arg_rex1(NULL, NULL, "type", NULL, 0, NULL);
		arg_rex1(NULL, NULL, "type", NULL, 0, NULL),
//    args_type[1] = args_umount[1];  // arg_rem ("[", NULL);
		arg_umnt[1],  // arg_rem ("[", NULL);
//    args_type[2] = args_cat[1];
		arg_cat[1],
//    args_type[3] = args_umount[5];  // arg_rem ("]", NULL);
		arg_umnt[5],  // arg_rem ("]", NULL);
//    args_type[4] = arg_end(2);
		arg_end(2),
	};

	static void* syntaxes[] = {
		arg_help,
		arg_mnt,
		arg_umnt,
		arg_ls,
		arg_cat,
		arg_type,
		NULL
	};

    cout << "**  Get the Syntax Tables in SDctrl::Syntax::tables().  **" << endl;
    return syntaxes;

}; /* SDctrl::Syntax::tables */


// for initializing singleton at the initial phase of programm
void** SDctrl::Syntax::alltables = nullptr;



int SDctrl::Syntax::help()
{
    return help_action((void**)alltables[0], alltables[1], alltables[2], alltables[3], alltables[4], alltables[5], NULL);
}; /* SDctrl::Syntax::help */



// inner release of the help action implements
int SDctrl::Syntax::help_action(void* hlp_arg[],...)
{

//	va_list arglst;
//	va_start(arglst, hlp_arg);
	void** curr_arg;


    cout << "#### Help action, implemented in the SDcmd::Syntax class. ####" << endl;

//    if (!hlp_arg)
    if (!tables() || !tables()[0])
    {
	cout << "!!! Error: syntax tables is undefined. !!!" << endl;
	cout << "Abort command" << endl;
	return -1;
    }; /* if !hlp_arg */

//    cout << "Usage: " << parent.argv[0];
//    curr_arg = va_arg(arglst, void**);
    cout << "Usage: " << parent.argv[0];
//    if (curr_arg)
//        arg_print_syntax(stdout, curr_arg, "\n");
//    else
//    {
//	arg_print_syntax(stdout, hlp_arg, "\n");
//	return 0;
//    }; /* else if curr_arg */
    arg_print_syntax(stdout, (void**)alltables[0], "\n");
#if 0
    curr_arg = va_arg(arglst, void**);
    while (curr_arg)
    {
	cout << "       " << parent.argv[0];
	arg_print_syntax(stdout, curr_arg, "\n");
	curr_arg = va_arg(arglst, void**);
    }; /* while curr_arg */
    va_end(arglst);
#else
    for (void **currcmd = tables() + 1; *currcmd != NULL; currcmd++)
    {
	cout << "       " << parent.argv[0];
	arg_print_syntax(stdout, (void**)*currcmd, "\n");
    }; /* for void **currcmd */
#endif
    cout << "       " << parent.argv[0];
//    arg_print_syntax(stdout, hlp_arg, "\n");
    arg_print_syntax(stdout, (void**)alltables[0], "\n");

    cout << "Command \"" << parent.argv[0] << "\" supports the ESP32 operation with an SD card." << endl;
    cout << "Use subcommands to invoke individual operations; operation are: mount, unmount, ls, cat, type, help." << endl;

#if 0
    va_start(arglst, hlp_arg);  // reset arglist pointer
    curr_arg = va_arg(arglst, void**);
    while (curr_arg)
    {
	arg_print_glossary(stdout, curr_arg, "      %-20s %s\n");
	curr_arg = va_arg(arglst, void**);
    }; /* while curr_arg */
    va_end(arglst);
    arg_print_glossary(stdout, hlp_arg, "      %-20s %s\n");
#else
    for (void **currcmd = tables(); *currcmd != NULL; currcmd++)
    {
//	cout << "       " << parent.argv[0];
//	arg_print_syntax(stdout, (void**)*currcmd, "\n");

	arg_print_glossary(stdout, (void**)*currcmd, "      %-20s %s\n");
    }; /* for void **currcmd */
#endif
    return 0;

}; /* SDcmd::Syntax::help_action */





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
