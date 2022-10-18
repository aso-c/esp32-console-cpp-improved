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
#include <cmd_sdcard>

#include "sdcard_io"
#include "sdcard_ctrl"

//#include <cstdio>

#include <utility>
#include <functional>
#include <tuple>

#include "extrstream"



//using namespace idf;
using namespace std;


#define MOUNT_POINT_def "/sdcard"


/*
 * Предлагаемые команды:
 *    -	sd - main command for manipulation with SD-card
 *	+ m, mount	- mount sdcard, options: [<card>] [<mountpoint>];
 *	+ u, umount	- unmount sdcard, options: [<card>|<mountpiont>];
 *	+ pwd		- get current directory name, w/o options;
 *	+ mkdir		- create a new directory
 *	+ ls, dir	- list of files in sdcard, options: [<file pattern>];
 *	+ cd <dir>	- change a current directory;
 *	+ cat <file>	- print file to a console
 *	+ type [<file>]	- type text to console and store it in the file optionally
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


class esp_console_cmd
{
public:

    esp_console_cmd (const esp_console_cmd_t&& cmd_def): cmd(&cmd_def) {};

    void registrate() {ESP_ERROR_CHECK(esp_console_cmd_register(cmd));};

private:
    const esp_console_cmd_t* cmd;
}; /* esp_console_cmd */


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


SDMMC::Card sdmmc_card; // @suppress("Type cannot be resolved")
SDMMC::Device device(SDMMC::bus::width_4, SDMMC::Host::pullup); // @suppress("Type cannot be resolved")
Exec::Server exec_server;


//--[ the filesystem operating commands ]----------------------------------------------------------

///////////////////////////////
// List of all fs cmd:
//	pwd, mkdir, cd, ls, cp, mv, rm
///////////////////////////////


// The pwd command --------------------------------------------------------------------------------

static int pwd_act(int argc, char **argv)
{
    return exec_server.pwd();
}; /* pwd_act */

void register_pwd(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    const esp_console_cmd_t cmd = {
	    .command = "pwd",
	    .help = "Get current directory name",
	    .hint = NULL,
	    .func = pwd_act,
    };
#pragma GCC diagnostic pop

    register_cmd(&cmd);

}; /* register_pwd */


// mkdir command ----------------------------------------------------------------------------------

static int mkdir_act(int argc, char **argv)
{
    cout << "\"mkdir\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.mkdir();
	break;

    case 2:
	return exec_server.mkdir(argv[1]);
	break;

    default:
	ESP_LOGE("mkdir command", "too many parameters (%d), don't know what directory to create", argc);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* mkdir_act */

void register_mkdir(void)
{
    static void* mkdargs[] = {
	    arg_str1(NULL, NULL, "<dir>", NULL/*"creating directory name"*/),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "mkdir",
	    .help = "Create new directory with name <dir>",
	    .hint = NULL,
	    .func = mkdir_act,
	    .argtable = mkdargs
    };

    register_cmd(&cmd);

}; /* register_mkdir */


// rmdir command ----------------------------------------------------------------------------------

static int rmdir_act(int argc, char **argv)
{
    cout << "\"rmdir\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.rmdir();
	break;

    case 2:
	return exec_server.rmdir(argv[1]);
	break;

    default:
	ESP_LOGE("rmdir command", "too many parameters (%d), deleting multiple directories at once is not allowed", argc);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* rmdir_act */

void register_rmdir(void)
{
    static void* rmdargs[] = {
	    arg_str1(NULL, NULL, "<dir>", NULL/*"name of deleting directory"*/),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "rmdir",
	    .help = "Delete empty existing directory <dir>",
	    .hint = NULL,
	    .func = rmdir_act,
	    .argtable = rmdargs
    };

    register_cmd(&cmd);

}; /* register_mkdir */


// 'cd' command -----------------------------------------------------------------------------------

static int cd_act(int argc, char **argv)
{
    cout << "\"cd\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.cd();
	break;

    case 2:
	return exec_server.cd(argv[1]);
	break;

    default:
	ESP_LOGE("cd command", "too many parameters (%d), where to go?", argc);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* cd_act */

void register_cd(void)
{
    static void* cdargs[] = {
	    arg_str1(NULL, NULL, "<dir>", NULL/*"directory name to change"*/),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "cd",
	    .help = "Change current directory to <dir>",
	    .hint = NULL,
	    .func = cd_act,
	    .argtable = cdargs
    };

    register_cmd(&cmd);

}; /* register_cd */


// 'ls' command -----------------------------------------------------------------------------------
static int ls_act(int argc, char **argv)
{
    cout << "\"ls\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.ls();
	break;

    case 2:
	return exec_server.ls(argv[1]);
	break;

    default:
	ESP_LOGE("ls command", "too many parameters (%d), which directory is to be printed?", argc);

    }; /* switch argc */
    cout << endl;
    return ESP_ERR_INVALID_ARG;
}; /* ls_act */

void register_ls(void)
{
    static void* lsargs[] = {
	    arg_str0(NULL, NULL, "<pattern>", NULL/*"pattern of the file for listing"*/),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "ls",
	    .help = "List contents of a directory according <pattern>, list non-directory files  is not available now. ((( Sorry.",
	    .hint = NULL,
	    .func = ls_act,
	    .argtable = lsargs
    };

    register_cmd(&cmd);

}; /* register_ls */


// 'cp' command -----------------------------------------------------------------------------------
static int cp_act(int argc, char **argv)
{
    cout << "\"cp\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.cp();
	break;

    case 2:
	return exec_server.cp(argv[1]);
	break;

    case 3:
	return exec_server.cp(argv[1], argv[2]);
	break;

    default:
	ESP_LOGE("cp command", "too many parameters (%d) for copy file(s), don't know to do.", argc);
    }; /* switch argc */

    cout << endl;
    return ESP_ERR_INVALID_ARG;
}; /* cp_act */

void register_cp(void)
{
    static void* cpargs[] = {
	    arg_str1(NULL, NULL, "<src>", NULL/*"source filename to copy"*/),
	    arg_str1(NULL, NULL, "<dest>", NULL/*"where to copy file"*/),
	    arg_end(2)
    };

    const esp_console_cmd_t cmd = {
	    .command = "cp",
	    .help = "Copy a file \"<src>\" to \"<dest>\".",
	    .hint = NULL,
	    .func = cp_act,
	    .argtable = cpargs
    };

    register_cmd(&cmd);

}; /* register_cp */


// 'mv' command -----------------------------------------------------------------------------------
static int mv_act(int argc, char **argv)
{
    cout << "\"mv\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.mv();
	break;

    case 2:
	return exec_server.mv(argv[1]);
	break;

    case 3:
	return exec_server.mv(argv[1], argv[2]);
	break;

    default:
	ESP_LOGE("mv command", "too many parameters (%d) for move/renaming file, don't know to do.", argc);
    }; /* switch argc */

    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* mv_act */

void register_mv(void)
{
    static void* cpargs[] = {
	    arg_str1(NULL, NULL, "<src>", NULL/*"source filename to rename/move"*/),
	    arg_str1(NULL, NULL, "<dest>", NULL/*"where to copy file"*/),
	    arg_end(2)
    };

    const esp_console_cmd_t cmd = {
	    .command = "mv",
	    .help = "Rename/move a file \"<src>\" to \"<dest>\".",
	    .hint = NULL,
	    .func = mv_act,
	    .argtable = cpargs
    };

    register_cmd(&cmd);

}; /* register_mv */


// 'rm' command -----------------------------------------------------------------------------------

static int rm_act(int argc, char **argv)
{
    cout << "\"rm\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.rm();
	break;

    case 2:
	cout << "...with one parameter - OK, specified the filename to delete." << endl;
	return exec_server.rm(argv[1]);
	break;

    default:
	ESP_LOGE("rm command", "too many parameters (%d), unable select file to remove.", argc);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* rm_act */

void register_rm(void)
{
    static void* rmargs[] = {
	    arg_str1(NULL, NULL, "<filename>", NULL/*"name of the file to delete"*/),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "rm",
	    .help = "Delete a file or fileset matching the pattern <filename>",
	    .hint = NULL,
	    .func = rm_act,
	    .argtable = rmargs
    };

    register_cmd(&cmd);

}; /* register_rm */


// 'cat' command -----------------------------------------------------------------------------------

static int cat_act(int argc, char **argv)
{
    cout << "\"cat\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.cat();
	break;

    case 2:
	return exec_server.cat(argv[1]);
	break;

    default:
	ESP_LOGE("cat command", "too many parameters (%d), unable select file to print.", argc);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* cat_act */

void register_cat(void)
{
    static void* catargs[] = {
	    arg_str1(NULL, NULL, "<filename>", NULL/*"name of the files for type to output"*/),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "cat",
	    .help = "Type contents of the file <filename> to standard output (default - to screen)",
	    .hint = NULL,
	    .func = cat_act,
	    .argtable = catargs
    };

    register_cmd(&cmd);

}; /* register_cat */


// 'type' command -----------------------------------------------------------------------------------

static int type_act(int argc, char **argv)
{
    cout << "\"type\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.type();
	break;

    case 2:
	cout << "...with one parameter - OK, save type output to file & output to screen." << endl;
	return exec_server.type(argv[1]);
	break;

    default:
//	cout << "more than one parameter - unknown set of parameters." << endl;
	ESP_LOGE("type command", "too many parameters (%d), in which file the output to be saved?", argc);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* type_act */

void register_type(void)
{
    static void* typeargs[] = {
	    arg_str0(NULL, NULL, "<filename>", NULL/*"filename for storage keyboard type output"*/),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	     .command = "type",
	    .help = "Type from a keyboard to standard output (default - to screen) and storing keyboard typing to the file <filename> (if specified)",
	    .hint = NULL,
	    .func = type_act,
	    .argtable = typeargs
    };

    register_cmd(&cmd);

}; /* register_type */


// register all fs commands -----------------------------------------------------------------------
void register_fs_cmd_all(void)
{

    register_pwd();
    register_cd();
    register_mkdir();
    register_rmdir();
    register_ls();
    register_cp();
    register_mv();
    register_rm();
    register_cat();
    register_type();

    //device.host().io_int_enable();
    //sdmmc_card.io.enable_int();

}; /* register_fs_cmd_all */



//--[ SD card control commands]--------------------------------------------------------------------

// classs of the SD control command implementation
class SDctrl
{
public:

    void store(int argc, char *argv[]);	// Initializing current command environment
    static SDctrl& cmd();// get unigue single instance of the SDcmd object
    static esp_err_t exec(int argc, char **argv);	// execute the 'SD' command

    esp_err_t err_none();	// Handler for "subcommand missing" error.
    esp_err_t err_unknown();	// Handler for "subcommand unknown" error.

    esp_err_t act_mnt();	// action for 'mount' command
    esp_err_t act_umnt();	// action for 'unmount' command
    esp_err_t act_info();	// action for 'info' command
    esp_err_t act_pwd();	// action for 'pwd' command
    esp_err_t act_mkdir();	// action for mkdir command
    esp_err_t act_rmdir();	// action for rmdir command
    esp_err_t act_cd();		// action for 'cd' command
    esp_err_t act_ls();		// action for list/dir command
    esp_err_t act_cp();		// action for 'copy file' command
    esp_err_t act_mv();		// action for rename/move file command
    esp_err_t act_rm();		// action for remove/delete file command
    esp_err_t act_cat();	// action for 'cat' command
    esp_err_t act_type();	// action for 'type' command

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
	enum cmd_id { none, mount, unmount, info, pwd, mkdir, rmdir, cd, ls, cp, mv, rm, cat, type, helping, unknown = -1 };

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
esp_err_t SDctrl::exec(int argc, char **argv)
{

    instance.store(argc, argv);

    if (argc == 1)
	return instance.err_none();

    switch (syntax.id())
    {
    case Syntax::mount:
	return instance.act_mnt();

    case Syntax::unmount:
	return instance.act_umnt();

    case Syntax::info:
	return instance.act_info();

    case Syntax::pwd:
	return instance.act_pwd();

    case Syntax::mkdir:
	return instance.act_mkdir();

    case Syntax::rmdir:
	return instance.act_rmdir();

    case Syntax::cd:
	return instance.act_cd();

    case Syntax::ls:
	return instance.act_ls();

    case Syntax::cp:
	return instance.act_cp();

    case Syntax::mv:
	return instance.act_mv();

    case Syntax::rm:
	return instance.act_rm();

    case Syntax::cat:
	return instance.act_cat();

    case Syntax::type:
	return instance.act_type();

    case Syntax::helping:
	return syntax.help();

    case Syntax::unknown:
    default:
	return instance.err_unknown();
    }; /* switch instance.id() */

    cout << endl;
    return ESP_OK;
}; /* SDctrl::exec */



// Initializing current command environment
void SDctrl::store(int argcnt, char *argvalue[])
{
    argc = argcnt;
    argv = argvalue;
}; /* SDcmd::store */


// Handler for "subcommand missing" error.
esp_err_t SDctrl::err_none()
{
    ESP_LOGE("sdcard command", "subcommand missing, what to run?");
    cout << syntax.hint << endl;
    return ESP_OK;
}; /* SDcmd::err_none */

// Handler for "subcommand unknown" error.
esp_err_t SDctrl::err_unknown()
{
    ESP_LOGE("sdcard command", "Unknown options: \"%s\".", argv[1]);
    cout << syntax.hint << endl;
    return ESP_OK;
}; /* SDcmd::err_unknown */


// action for 'mount' command
esp_err_t SDctrl::act_mnt()
{
    esp_err_t res;

    device.host().set_card_clk(40000);	// test for low speed

    switch (argc)
    {
    case 2:
	res = exec_server.mount(device, sdmmc_card); // @suppress("Invalid arguments")
	break;

    case 3:
	cout << "...with one parameter - use device or mount point." << endl;
	res = exec_server.mount(device, sdmmc_card, argv[2]); // @suppress("Invalid arguments")
	break;

    case 4:
	cout << "...with two parameters - use device & mount point." << endl;
	res = exec_server.mount(device, sdmmc_card, atoi(argv[2]), argv[3]); // @suppress("Invalid arguments")
	break;

    default:
	ESP_LOGE("sdcard mount command", "more than two parameters (%d) is not allowed", argc - 2);
	res = ESP_FAIL;
    }; /* switch argc */
    cout << endl;

    if (res == ESP_OK)
    {
	device.host().io_int_enable();
	sdmmc_card.io.enable_int();
	device.card->info(); // @suppress("Field cannot be resolved") // @suppress("Method cannot be resolved")
    }

    return res;
}; /* SDctrl::act_mnt */


// action for 'unmount' command
esp_err_t SDctrl::act_umnt()
{
    cout << "\"unmount\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - use default values." << endl;
	return exec_server.unmount(device); // @suppress("Invalid arguments")
	break;

//    case 3:
//	cout << "...with one parameter - use device or mount point." << endl;
//	return exec_server.unmount(argv[2]);
//	break;

    default:
	ESP_LOGE("sdcard umount command", "more than one parameters (%d) - is not allowed", argc - 2);
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_umnt */


// print info about the mounted SD-card
esp_err_t SDctrl::act_info()
{
//    exec_server.info();
//    if (device.card == nullptr)
    if (!device.card) // @suppress("Field cannot be resolved")
    {
	ESP_LOGW("sdcard info command", "SD-card now is not mounted!!!");
	return ESP_ERR_NOT_FOUND;
//    }; //* if device.card == nullptr */
    }; //* if !device.card */
    device.card->info(); // @suppress("Field cannot be resolved") // @suppress("Method cannot be resolved")
    cout << "Pullup is: " << ((device.host().slot().pullup_state())? "Enabled": "Absent") << endl;
    cout << "###############################################" << endl;

#define TAG "SD Command Service"

    ESP_LOGI(TAG, "SD card info:");
        ESP_LOGI(TAG, "\tBus width (log2): %d", sdmmc_card.self->log_bus_width);
        ESP_LOGI(TAG, "\tFreq (kHz): %'d", sdmmc_card.self->max_freq_khz);
        ESP_LOGI(TAG, "\tDDR: %d", sdmmc_card.self->is_ddr);
        ESP_LOGI(TAG, "\tCID: Date %d, MFG_ID %d, Name %s, OEM ID %d, Rev %d, Serial %d", sdmmc_card.self->cid.date, sdmmc_card.self->cid.mfg_id, sdmmc_card.self->cid.name, sdmmc_card.self->cid.oem_id, sdmmc_card.self->cid.revision, sdmmc_card.self->cid.serial);
        ESP_LOGI(TAG, "\tCSD: Capacity %'d, Card Common Class %d, CSD version %d, MMC version %d, read block len %d, sector size %d, tr speed %'d", sdmmc_card.self->csd.capacity, sdmmc_card.self->csd.card_command_class, sdmmc_card.self->csd.csd_ver, sdmmc_card.self->csd.mmc_ver, sdmmc_card.self->csd.read_block_len, sdmmc_card.self->csd.sector_size, sdmmc_card.self->csd.tr_speed);
        ESP_LOGI(TAG, "\tCSD: Ease mem state <undef>%d, Power class %d, Revision <undef>%d, Sec feature <undef>%d", /*sdmmc_card.self->ext_csd.erase_mem_state*/-1, sdmmc_card.self->ext_csd.power_class, /*sdmmc_card.self->ext_csd.rev*/-1, /*sdmmc_card.self->ext_csd.sec_feature*/-1);
        ESP_LOGI(TAG, "\tSCR: bus width %d, erase mem state <undef%d>, reserved <undef%d>, rsvd_mnf <undef%d>, sd_spec %d", sdmmc_card.self->scr.bus_width, /*sdmmc_card.self->scr.erase_mem_state*/-1, /*sdmmc_card.self->scr.reserved*/-1, /*sdmmc_card.self->scr.rsvd_mnf*/-1, sdmmc_card.self->scr.sd_spec);
        //ESP_LOGI(TAG, "\tSSR: cur_bus_width %d, discard_support %d, fule_support %d, reserved %d", sdmmc_card.self->ssr.cur_bus_width, sdmmc_card.self->ssr.discard_support, sdmmc_card.self->ssr.fule_support, sdmmc_card.self->ssr.reserved);

        cout << "###############################################" << endl;

#if 1
    //return device.card->cis_info(stdout);
//        size_t cisize = 0;
//	size_t bsize = 16;
//	uint8_t* outbuf = (uint8_t*)malloc(bsize);
	esp_err_t err;
//    err = device.card->io.get_cis_data(outbuf, bsize, &cisize); // @suppress("Method cannot be resolved") // @suppress("Field cannot be resolved")
    err = device.card->cis_info(stdout);
    ESP_LOGE("sdcard info command", "Error %i in the get or print CIS data: %s", err, esp_err_to_name(err));
////    if (err != ESP_OK)
////    {
//	//ESP_LOGE("sdcard info command", "Error %i in get get CIS data first time: %s", err, esp_err_to_name(err));
////	free(outbuf);
//	switch (err)
//	{
//	case ESP_ERR_INVALID_RESPONSE:
//		ESP_LOGE("sdcard info command", "Error %i in the get CIS data: %s", err, esp_err_to_name(err));
//	    return err;
//	case ESP_ERR_INVALID_SIZE:	// CIS_CODE_END found, but buffer_size is less than required size, which is stored in the inout_cis_size then.
//	case ESP_ERR_NOT_FOUND:		// if the CIS_CODE_END not found. Increase input value of inout_cis_size or set it to 0,
////	    bsize = cisize;
////	    cout << aso::format("The new size of the CIS data buffer is: %i") % bsize << endl;
////	    cisize = 0;
////	    outbuf = (uint8_t*)malloc(bsize);
////	    err = device.card->io.get_cis_data(outbuf, bsize, &cisize); // @suppress("Field cannot be resolved") // @suppress("Method cannot be resolved")
//		ESP_LOGE("sdcard info command", "Error %i in the get CIS data: %s", err, esp_err_to_name(err));
//	    return err;
//	}; /* switch err */
////    }; /* if err != ESP_ERR_INVALID_SIZE */
////    if (err != ESP_OK)
////    {
////	//ESP_RETURN_ON_ERROR(err, "sdcard info command", "Error in the get CIS data");
////	ESP_LOGE("sdcard info command", "Error %i in the get CIS data: %s", err, esp_err_to_name(err));
////	return err;
////    }; /* if err != ESP_OK */
////    err = device.card->io.print_cis_info(outbuf, bsize, stdout); // @suppress("Method cannot be resolved") // @suppress("Field cannot be resolved")
////    free(outbuf);
////    if (err != ESP_OK)
////	ESP_LOGE("sdcard info command", "Error %i in the print of the CIS info: %s", err, esp_err_to_name(err));

    return err;
#else
        size_t cisize = 0;
	size_t bsize = 16;
	uint8_t* outbuf = (uint8_t*)malloc(bsize);
	esp_err_t err;
    // esp_err_t sdmmc_io_get_cis_data(sdmmc_card_t *card, uint8_t *out_buffer, size_t buffer_size, size_t *inout_cis_size)
    err = device.card->io.get_cis_data(outbuf, bsize, &cisize); // @suppress("Method cannot be resolved") // @suppress("Field cannot be resolved")
    if (err != ESP_OK)
    {
//	    if (err != ESP_ERR_INVALID_SIZE)
	ESP_LOGE("sdcard info command", "Error %i in get get CIS data first time: %s", err, esp_err_to_name(err));
	free(outbuf);
	switch (err)
	{
	case ESP_ERR_INVALID_RESPONSE:
	    return err;
	case ESP_ERR_INVALID_SIZE:	// CIS_CODE_END found, but buffer_size is less than required size, which is stored in the inout_cis_size then.
	case ESP_ERR_NOT_FOUND:		// if the CIS_CODE_END not found. Increase input value of inout_cis_size or set it to 0,
	    bsize = cisize;
	    cout << aso::format("The new size of the CIS data buffer is: %i") % bsize << endl;
	    cisize = 0;
	    outbuf = (uint8_t*)malloc(bsize);
	    err = device.card->io.get_cis_data(outbuf, bsize, &cisize); // @suppress("Field cannot be resolved") // @suppress("Method cannot be resolved")
	}; /* switch err */
    }; /* if err != ESP_ERR_INVALID_SIZE */
    if (err != ESP_OK)
    {
	//ESP_RETURN_ON_ERROR(err, "sdcard info command", "Error in the get CIS data");
	ESP_LOGE("sdcard info command", "Error %i in the get CIS data: %s", err, esp_err_to_name(err));
	return err;
    }; /* if err != ESP_OK */
    err = device.card->io.print_cis_info(outbuf, bsize, stdout); // @suppress("Method cannot be resolved") // @suppress("Field cannot be resolved")
    free(outbuf);
    if (err != ESP_OK)
	ESP_LOGE("sdcard info command", "Error %i in the print of the CIS info: %s", err, esp_err_to_name(err));

    return err;
#endif
}; /* SDctrl::act_info */


// action for pwd command
esp_err_t SDctrl::act_pwd()
{
    exec_server.pwd();

    return 0;
}; /* SDctrl::act_pwd */


// action for 'mkdir' command
esp_err_t SDctrl::act_mkdir()
{
    cout << "\"mkdir\" command execution" << endl;
    switch (argc)
    {
    case 2:
	return exec_server.mkdir();
	break;

    case 3:
	cout << "...with one parameter - specified the name of the new directory." << endl;
	return exec_server.mkdir(argv[2]);
	break;

    default:
	ESP_LOGE("sdcard mkdir command", "more than one parameters (%d) - don't know what directory to create.", argc - 2);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* SDctrl::act_mkdir */


// action for 'rmdir' command
esp_err_t SDctrl::act_rmdir()
{
    cout << "\"rmdir\" command execution" << endl;
    switch (argc)
    {
    case 2:
	return exec_server.rmdir();
	break;

    case 3:
	cout << "...with one parameter - specified the name directory to delete." << endl;
	return exec_server.rmdir(argv[2]);
	break;

    default:
	ESP_LOGE("sdcard rmdir command", "more than one parameters (%d) - deleting multiple directories at once is not allowed", argc - 2);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* SDctrl::act_mkdir */


// action for 'cd' command
esp_err_t SDctrl::act_cd()
{
    cout << "\"cd\" command execution" << endl;
    switch (argc)
    {
    case 2:
	return exec_server.cd();
	break;

    case 3:
	cout << "...with one parameter - specified the path name to change." << endl;
	return exec_server.cd(argv[2]);
	break;

    default:
	ESP_LOGE("sdcard cd command", "more than one parameters (%d) - where to go?", argc - 2);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* SDctrl::act_cd */


// action for list/dir command
esp_err_t SDctrl::act_ls()
{
    cout << "\"ls\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - use current dir." << endl;
	return exec_server.ls();
	break;

    case 3:
	cout << "...with one parameter - use pattern or directory." << endl;
	return exec_server.ls(argv[2]);
	break;

    default:
	ESP_LOGE("sdcard ls command", "more than one parameters (%d) - what directory to listing?", argc - 2);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* SDctrl::act_ls */


// action for 'copy' command
esp_err_t SDctrl::act_cp()
{
    cout << "\"cp\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - error." << endl;
	return exec_server.cp();
	break;

    case 3:
	cout << "...with one parameter - error." << endl;
	return exec_server.cp(argv[2]);
	break;

    case 4:
	cout << "...with two parameter - copy files." << endl;
	return exec_server.cp(argv[2], argv[3]);
	break;

    default:
	ESP_LOGE("sdcard ls command", "more than two parameters (%d) - don't know what to copy", argc - 2);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* SDctrl::act_cp */


// action for 'rename/move' command
esp_err_t SDctrl::act_mv()
{
    cout << "\"mv\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - error." << endl;
	return exec_server.mv();
	break;

    case 3:
	cout << "...with one parameter - error." << endl;
	return exec_server.mv(argv[2]);
	break;

    case 4:
	cout << "...with two parameter - move/rename files." << endl;
	return exec_server.mv(argv[2], argv[3]);
	break;

    default:
	ESP_LOGE("sdcard mv command", "more than two parameters (%d) - don't know what to rename/move", argc - 2);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* SDctrl::act_mv */


// action for list/dir command
esp_err_t SDctrl::act_rm()
{
    cout << "\"rm\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - error." << endl;
	return exec_server.rm();
	break;

    case 3:
	cout << "...with one parameter - remove file." << endl;
	return exec_server.rm(argv[2]);
	break;

    default:
	ESP_LOGE("sdcard rm command", "more than one parameters (%d) - don't know what to remove", argc - 2);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* SDctrl::act_ls */


// action for 'cat' command
esp_err_t SDctrl::act_cat()
{
    cout << "\"cat\" command execution" << endl;
    switch (argc)
    {
    case 2:
	return exec_server.cat();
	break;

    case 3:
	return exec_server.cat(argv[2]);
	break;

    default:
	ESP_LOGE("sdcard cat command", "more than one parameters (%d) - what file is to be printed?", argc - 2);
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_cat */


// action for 'type' command
esp_err_t SDctrl::act_type()
{
    cout << "\"type\" command execution" << endl;
    switch (argc)
    {
    case 2:
	return exec_server.type();
	break;

    case 3:
	cout << "...with one parameter - save type output to file & screen." << endl;
	return exec_server.type(argv[2], device.card->self->csd.sector_size); // @suppress("Invalid arguments") // @suppress("Field cannot be resolved")
	break;

    default:
	ESP_LOGE("sdcard type command", "more than one parameters (%d) - what file save the type output?", argc - 2);
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

#define innerqte(a) #a
#define qte(a) innerqte(a)
    // syntax1: m | mount [<device>] [<mountpoint>] "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters are optional"
    // syntax1: corrected m | mount [<slot>] [<mountpoint>] "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters are optional"
	static void* arg_mnt[] = {
		arg_rex1(NULL, NULL, "m|mount", NULL, 0, "mount SD-card [<device>] to [<mountpoint>], parameters are optional"),
		arg_str0(NULL, NULL, "<slot>", "SD card slot (device) number, used slot #"  qte(SDMMC_HOST_SLOT_1)  " default value if omitted"),
		arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, used path \"" MOUNT_POINT_def "\" if omitted"),
		arg_end(2),
	};
    // syntax2: u | umount [ <device> | <mountpoint> ] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
    // syntax2 corrected: u | umount [<mountpoint>] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
	static void* arg_umnt[] = {
		arg_rex1(NULL, NULL, "u|umount", NULL, 0, "unmount SD-card [<path>] where the SD card is mounted; if parameters omitted - use \"" MOUNT_POINT_def "\"" ),
		arg_str0(NULL, NULL, "<mountpoint>", NULL),
		arg_end(2),
	};
//----------------------------------------------------------------------------------------------------------------------
    // syntax3: info "information about mounted SD-card"
	static void* arg_info[] = {
		arg_rex1(NULL, NULL, "i|info", NULL, 0, "information about mounted SD-card"),
//		arg_str0(NULL, NULL, "<pattern>", "file pattern or path"),
		arg_end(2),
	};
    // syntax4: pwd "current directory name"
	static void* arg_pwd[] = {
		arg_rex1(NULL, NULL, "p|pwd", NULL, 0, "current directory name"),
//		arg_str0(NULL, NULL, "<pattern>", "file pattern or path"),
		arg_end(2),
	};
    // syntax5: mkdir [<path>] "make new directory with name <path>"
	static void* arg_mkdir[] = {
		arg_rex1(NULL, NULL, "mkdir", NULL, 0, "make new directory with name \"<path>\""),
		arg_str0(NULL, NULL, "<path>", NULL/*"name of the new directory"*/),
		arg_end(2),
	};
	// syntax6: rmdir [<path>] "delete existing empty directory with name <path>"
	static void* arg_rmdir[] = {
		arg_rex1(NULL, NULL, "rmdir", NULL, 0, "delete existing empty directory with name \"<path>\""),
		arg_str0(NULL, NULL, "<path>", NULL/*"name of the new directory"*/),
		arg_end(2),
		};
    // syntax7: cd [<path>] "change current directory to <path>"
	static void* arg_cd[] = {
		arg_rex1(NULL, NULL, "cd", NULL, 0, "change current directory to a <path>"),
		arg_str0(NULL, NULL, "<path>", NULL/*"path to which the current directory is changed"*/),
		arg_end(2),
	};
//----------------------------------------------------------------------------------------------------------------------
    // syntax8: cp <src> <dest> "copy file <src> to <dest>"
	static void* arg_cp[] = {
		arg_rex1(NULL, NULL, "cp|copy", NULL, 0, "copy file <src> to <dest>"),
		arg_str1(NULL, NULL, "<src>", NULL/*"file name to copy"*/),
		arg_str1(NULL, NULL, "<dest>", NULL/*"where file to copy"*/),
		arg_end(3),
	};
    // syntax9: mv <src> <dest> "rename/move file <src> to <dest>"
	static void* arg_mv[] = {
		arg_rex1(NULL, NULL, "mv|ren", NULL, 0, "rename/move file <src> to <dest>"),
		arg_str1(NULL, NULL, "<src>", NULL/*"source file name to copy or rename/move"*/),
		arg_str1(NULL, NULL, "<dest>", NULL/*"where file to copy or rename/move"*/),
		arg_end(3),
		};
    // syntax10: rm [<pattern>] "delete file <pattern>"
	static void* arg_rm[] = {
		arg_rex1(NULL, NULL, "rm", NULL, 0, "delete file according <pattern>"),
		arg_str1(NULL, NULL, "<pattern>", NULL/*"file name to delete"*/),
		arg_end(2),
	};
    // syntax11: ls | dir [<pattern>] "print directory contents on SD-card"
	static void* arg_ls[] = {
		arg_rex1(NULL, NULL, "ls|dir", NULL, 0, "print directory contents on SD-card according <pattern>"),
		arg_str0(NULL, NULL, "<pattern>", NULL/*"file pattern or path for lising"*/),
		arg_end(2),
	};
    // syntax12: cat <filename> "print file to stdout (console output)"
	static void* arg_cat[] = {
		arg_rex1(NULL, NULL, "cat", NULL, 0, "print content of the file \"<filename>\" to screen"),
		arg_str1(NULL, NULL, "<filename>", NULL),
		arg_end(2),
	};
    // syntax  : type [filename] "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only"
	static void* arg_type[] = {
		arg_rex1(NULL, NULL, "type", NULL, 0, "type from the keyboard to a file & screen or screen only if the file omitted"),
//		"Type from a keyboard to standard output (default - to screen) and storing keyboard typing to the file <filename> (if specified)"		arg_str0(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved"),
		arg_end(2),
	};

	static void* syntaxes[] = {
		arg_mnt,
		arg_umnt,
		arg_info,
		arg_pwd,
		arg_mkdir,
		arg_rmdir,
		arg_cd,
		arg_cp,
		arg_mv,
		arg_rm,
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
    if (strcmp(parent.argv[1], "info") == 0 || strcmp(parent.argv[1], "i") == 0)
	return info;
    if (strcmp(parent.argv[1], "pwd") == 0 || strcmp(parent.argv[1], "p") == 0)
	return pwd;
    if (strcmp(parent.argv[1], "cd") == 0)
	return cd;
    if (strcmp(parent.argv[1], "ls") == 0 || strcmp(parent.argv[1], "dir") == 0)
	return ls;
    if (strcmp(parent.argv[1], "cat") == 0 || strcmp(parent.argv[1], "c") == 0)
    	return cat;
    if (strcmp(parent.argv[1], "type") == 0 || strcmp(parent.argv[1], "t") == 0)
    	return type;

    return unknown;
}; /* SDctrl::Syntax::id */



int SDctrl::Syntax::help()
{
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


//--[ cmd_sdcard.cpp ]-----------------------------------------------------------------------------
