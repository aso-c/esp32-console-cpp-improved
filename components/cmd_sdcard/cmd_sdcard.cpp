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
#include "sdcard_ctrl.hpp"

//#include <stdio.h>

//using namespace idf;
using namespace std;


#define MOUNT_POINT_def "/sdcard"


/*
 * Предлагаемые команды:
 *    -	sd - main command for manipulation with SD-card
 *	+ m, mount	- mount sdcard, options: [<card>] [<mountpoint>];
 *	+ u, umount	- unmount sdcard, options: [<card>|<mountpiont>];
 *	+ pwd		- get current directory name, w/o options;
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


SDMMC::Server sd_server;


//--[ the filesystem operating commands ]----------------------------------------------------------

///////////////////////////////
// List of all fs cmd:
//	pwd, cd, ls, rm
///////////////////////////////


// The pwd command --------------------------------------------------------------------------------

static int pwd_act(int argc, char **argv)
{
    return sd_server.pwd();
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


// 'cd' command -----------------------------------------------------------------------------------

static int cd_act(int argc, char **argv)
{
    cout << "\"cd\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return sd_server.cd();
	break;

    case 2:
	cout << "...with one parameter - OK, specified the path name to change." << endl;
	return sd_server.cd(argv[1]);
	break;

    default:
	cout << "more than one parameter - unknown set of parameters." << endl;
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* cd_act */

void register_cd(void)
{
    static void* cdargs[] = {
	    arg_str1(NULL, NULL, "<dir>", "directory name to change"),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "cd",
	    .help = "Change current directory name",
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
	cout << "...without parameters - OK, list all files in the current directory." << endl;
	return sd_server.ls();
	break;

    case 2:
	cout << "...with one parameter - OK, print the files in the desired directory and/or according a pattern." << endl;
	return sd_server.ls(argv[1]);
	break;

    default:
	cout << "more than one parameter - unknown set of parameters." << endl;
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* ls_act */

void register_ls(void)
{
    static void* lsargs[] = {
	    arg_str0(NULL, NULL, "<pattern>", "pattern of the file for listing"),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "ls",
	    .help = "List contents of a directory according pattern",
	    .hint = NULL/*"enter the directory name for change"*/,
	    .func = ls_act,
	    .argtable = lsargs
    };

    register_cmd(&cmd);

}; /* register_ls */


// 'rm' command -----------------------------------------------------------------------------------

static int rm_act(int argc, char **argv)
{
    cout << "\"rm\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return sd_server.rm();
	break;

    case 2:
	cout << "...with one parameter - OK, specified the filename to delete." << endl;
	return sd_server.cd(argv[1]);
	break;

    default:
	cout << "more than one parameter - unknown set of parameters." << endl;
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* cd_act */

void register_rm(void)
{
    static void* rmargs[] = {
	    arg_str1(NULL, NULL, "<filename>", "filename name for delete"),
	    arg_end(1)
    };

    const esp_console_cmd_t cmd = {
	    .command = "rm",
	    .help = "Delete a file or set of files matching the pattern <filename>",
	    .hint = NULL,
	    .func = rm_act,
	    .argtable = rmargs
    };

    register_cmd(&cmd);

}; /* register_rm */


// register all fs commands -----------------------------------------------------------------------
void register_fs_cmd_all(void)
{

    register_pwd();
    register_cd();
    register_ls();
    register_rm();

}; /* register_fs_cmd_all */



//--[ SD card control commands]--------------------------------------------------------------------

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
    static esp_err_t exec(int argc, char **argv);	// execute the 'SD' command

    esp_err_t err_none();	// Handler for "subcommand missing" error.
    esp_err_t err_unknown();	// Handler for "subcommand unknown" error.

    esp_err_t act_mnt();	// action for 'mount' command
    esp_err_t act_umnt();	// action for 'unmount' command
    esp_err_t act_info();	// action for 'info' command
    esp_err_t act_pwd();	// action for 'pwd' command
    esp_err_t act_cd();		// action for 'cd' command
    esp_err_t act_ls();		// action for list/dir command
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
	enum cmd_id { none, mount, unmount, info, pwd, cd, ls, rm, cat, type, helping, unknown = -1 };

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
//    case SDctrl::Syntax::mount:
    case Syntax::mount:
	return instance.act_mnt();

//    case SDctrl::Syntax::unmount:
    case Syntax::unmount:
	return instance.act_umnt();

//    get current directory name
    case Syntax::info:
	return instance.act_info();

//    get current directory name
    case Syntax::pwd:
	return instance.act_pwd();

//    get current directory name
    case Syntax::cd:
	return instance.act_cd();

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
    cout << "Subcommand missing." << endl
	 << syntax.hint << endl;
    return ESP_OK;
}; /* SDcmd::err_none */

// Handler for "subcommand unknown" error.
esp_err_t SDctrl::err_unknown()
{
    cout << "Unknown options: \"" << argv[1] <<  "\"." << endl
	 << syntax.hint << endl;
    return ESP_OK;
}; /* SDcmd::err_unknown */


// action for 'mount' command
esp_err_t SDctrl::act_mnt()
{
    esp_err_t res;

    cout << "\"mount\" command execution" << endl;
    switch (argc)
    {
    case 2:
	cout << "...without parameters - use default values." << endl;
//	return sd_server.mount();
	res = sd_server.mount();
	break;

    case 3:
	cout << "...with one parameter - use device or mount point." << endl;
//	return sd_server.mount(argv[2]);
	res = sd_server.mount(argv[2]);
	break;

    case 4:
	cout << "...with two parameters - use device & mount point." << endl;
//	return sd_server.mount(atoi(argv[2]), argv[3]);
	res = sd_server.mount(atoi(argv[2]), argv[3]);
	break;

    default:
	cout << "more than two parameters - obscure set of parameters." << endl;
					//  unclear set of parameters
	res = ESP_FAIL;
    }; /* switch argc */
    cout << endl;

    if (res == ESP_OK)
	sd_server.card_info(stdout);

//    return 0;
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
	return sd_server.unmount();
	break;

    case 3:
	cout << "...with one parameter - use device or mount point." << endl;
	return sd_server.unmount(argv[2]);
	break;

    default:
	cout << "more than one parameter - unclear set of parameters." << endl;
    }; /* switch argc */
    cout << endl;

    return 0;
}; /* SDctrl::act_umnt */


// print info about the mounted SD-card
esp_err_t SDctrl::act_info()
{
//    cout << "\"info\" command execution" << endl;
    sd_server.info();
//    cout << endl;

    return 0;
}; /* SDctrl::act_info */


// action for pwd command
esp_err_t SDctrl::act_pwd()
{
//    cout << "\"pwd\" command execution" << endl;
    sd_server.pwd();
//    cout << endl;

    return 0;
}; /* SDctrl::act_pwd */


// action for 'cd' command
esp_err_t SDctrl::act_cd()
{
    cout << "\"cd\" command execution" << endl;
    switch (argc)
    {
    case 2:
	return sd_server.cd();
	break;

    case 3:
	cout << "...with one parameter - specified the path name to change." << endl;
	return sd_server.cd(argv[2]);
	break;

    default:
	cout << "more than one parameter - unknown set of parameters." << endl;
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
	return sd_server.ls();
	break;

    case 3:
	cout << "...with one parameter - use pattern or directory." << endl;
	return sd_server.ls(argv[2]);
	break;

    default:
	cout << "more than one parameter - unknown set of parameters." << endl;
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
	//cout << "...without parameters - error parameter values." << endl;
	return sd_server.cat();
	break;

    case 3:
	//cout << "...with one parameter - use file name." << endl;
	return sd_server.cat(argv[2]);
	break;

    default:
	cout << "Any parameters error." << endl;
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
	//cout << "...without parameters - type to screen only." << endl;
	return sd_server.type();
	break;

    case 3:
	cout << "...with one parameter - save type output to file & screen." << endl;
	return sd_server.type(argv[2]);
	break;

    default:
	cout << "Any parameters error." << endl;
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
    // syntax5: cd [<path>] "change current directory to <path>"
	static void* arg_cd[] = {
		arg_rex1(NULL, NULL, "cd", NULL, 0, "change current directory to <path>"),
		arg_str0(NULL, NULL, "<path>", "path to which the current directory is changed"),
		arg_end(2),
	};
//----------------------------------------------------------------------------------------------------------------------
    // syntax5: ls | dir [<pattern>] "print directory contents on SD-card"
	static void* arg_ls[] = {
		arg_rex1(NULL, NULL, "ls|dir", NULL, 0, "print directory contents on SD-card"),
		arg_str0(NULL, NULL, "<pattern>", "file pattern or path"),
		arg_end(2),
	};
    // syntax6: cat <filename> "print file to stdout (console output)"
	static void* arg_cat[] = {
		arg_rex1(NULL, NULL, "cat", NULL, 0, NULL),
		arg_str1(NULL, NULL, "<file>", NULL),
		arg_end(2),
	};
    // syntax7: type [filename] "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only"
	static void* arg_type[] = {
		arg_rex1(NULL, NULL, "type", NULL, 0, "type from the keyboard to file & screen or screen only; if file omitted - print to screen only"),
		arg_str0(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved"),
		arg_end(2),
	};

	static void* syntaxes[] = {
		arg_mnt,
		arg_umnt,
		arg_info,
		arg_pwd,
		arg_cd,
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
