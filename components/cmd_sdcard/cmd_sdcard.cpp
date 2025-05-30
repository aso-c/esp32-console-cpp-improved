/*!
 * @brief Implementation of SD-card manipulations commands
 *
 * 	@file	cmd_sdcard.cpp
 *	@author	Solomatov A.A. (aso)
 *	@date	Created 04.04.2022
 *		Modified 18.02.2025
 *	@version: 0.75
 */

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <list>
#include <vector>
#include <variant>
#include <stdarg.h>

#include <cstring>
//#include <sys/unistd.h>
#include <esp_log.h>
#include <esp_console.h>
#include <esp_system.h>
#include <argtable3/argtable3.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>

#include <argtable>
#include <argtable-legacy>
#include <console>
#include <cmd_sdcard>
#include <sdcard_io>
#include <fs_ctrl>

//#include <cstdio>

#include <utility>
#include <functional>
#include <tuple>

#include <extrstream>
#include <astring.h>

//using namespace idf;
using namespace std;


//#define MOUNT_POINT_def "/sdcard"
#define SD_MOUNT_POINT "/sdcard"


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


/// namespace for cmd-actions
namespace act
{

    /// Handler for "subcommand missing" error of the sd-command.
    esp_err_t none(std::vector<char*> args);

    /// Handler for "subcommand unknown" error of the sd-command.
    esp_err_t unknown(std::vector<char*> args);

    /// action for 'mount' command
    esp_err_t mnt(std::vector<char*> args);

    /// action for 'unmount' command
    esp_err_t umnt(std::vector<char*> args);

    /// print info about the mounted SD-card
    esp_err_t info(SD::MMC::Device&);

    /// action for pwd command
    esp_err_t pwd(std::vector<char*> args);

    /// action for 'mkdir' command
    esp_err_t mkdir(std::vector<char*> args);

    /// action for 'rmdir' command
    esp_err_t rmdir(std::vector<char*> args);

    /// action for 'cd' command
    esp_err_t cd(std::vector<char*> args);

    /// action for list/dir command
    esp_err_t ls(std::vector<char*> args);

    /// action for 'copy' command
    esp_err_t cp(std::vector<char*> args);

    /// action for 'rename/move' command
    esp_err_t mv(std::vector<char*> args);

    /// action for 'rm' command
    esp_err_t rm(std::vector<char*> args);

    /// action for 'cat' command
    esp_err_t cat(std::vector<char*> args);

    /// action for 'type' command
    esp_err_t type(std::vector<char*> args);

}; /* namespace act */



SD::Card sdmmc_card;
SD::MMC::Device device(SD::MMC::Slot(SD::MMC::Slot::flag::pullup, SD::MMC::bus::width::_4));
Exec::Cmd exec_server;


//--[ the filesystem operating commands ]----------------------------------------------------------

///////////////////////////////
// List of all fs cmd:
//	pwd, mkdir, cd, ls, cp, mv, rm
///////////////////////////////


/// The pwd command -----------------------------------------------------------------------------------------
namespace pwd
{
    static esp_err_t invoke(int argc, char* argv[]);
    const esp::console::cmd cmd("pwd", invoke, "Get current directory name");
}; /* namespace pwd */


/// mkdir command -------------------------------------------------------------------------------------------
namespace mk_dir
{

    arg::table::syntax syntax = { 1,
	    arg_str1(NULL, NULL, "<dir>", NULL),
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct mk_dir::act */
    const esp::console::cmd_t<act> cmd("mkdir", "Create new directory with name <dir>");
}; /* namespace mk_dir */


/// rmdir command -------------------------------------------------------------------------------------------

namespace rm_dir
{
    arg::table::syntax syntax = { 1,
	    arg_str1(NULL, NULL, "<dir>", NULL),
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct rm_dir::act */
    const esp::console::cmd_t<act> cmd("rmdir", "Delete empty existing directory <dir>");
}; /* namespace rm_dir */


/// 'cd' command --------------------------------------------------------------------------------------------
namespace cd
{
    arg::table::syntax syntax = { 1,
	    arg_str1(NULL, NULL, "<dir>", NULL),
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct cd::act */
    const esp::console::cmd_t<act> cmd("cd", "Change current directory to a <path>");
}; /* namespace cd */


/// 'ls' command --------------------------------------------------------------------------------------------

namespace ls
{
    arg::table::syntax syntax = { 1,
	    arg_str0(NULL, NULL, "<pattern>", NULL),
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct ls::act */
    const esp::console::cmd_t<act> cmd("ls", "List contents of a directory or a file according <pattern>");
}; /* namespace ls */


/// 'cp' command, pure C wrapper ----------------------------------------------------------------------------
namespace cp
{
    arg::table::syntax syntax = { 2,
	    arg_str1(NULL, NULL, "<src>", NULL),
	    arg_str1(NULL, NULL, "<dest>", NULL),
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct cp::act */
    const esp::console::cmd_t<act> cmd("cp", "Copy a file \"<src>\" to \"<dest>\".");
}; /* namespace cp */


/// 'mv' command --------------------------------------------------------------------------------------------

namespace mv
{
    arg::table::syntax syntax = { 2,
	    arg_str1(NULL, NULL, "<src>", NULL),
	    arg_str1(NULL, NULL, "<dest>", NULL),
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct mv::act */
    const esp::console::cmd_t<act> cmd("mv", "Rename/move a file \"<src>\" to \"<dest>\".");
}; /* namespace mv */


/// 'rm' command, pure C wrapper ----------------------------------------------------------------------------
namespace rm
{
    arg::table::syntax syntax = { 1,
	    arg_str1(NULL, NULL, "<filename>", NULL),
//	    arg_end(1)
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct rm::act */
    const esp::console::cmd_t<act> cmd("rm", "Delete a file or fileset matching the pattern <filename>");
}; /* namespace rm */


/// 'cat' command -------------------------------------------------------------------------------------------
namespace cat
{
    arg::table::syntax syntax = { 1,
	    arg_str1(NULL, NULL, "<filename>", NULL),
//	    arg_end(1)
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct cat::act */
    const esp::console::cmd_t<act> cmd("cat", "Type contents of the file <filename> to standard output (default - to screen)");
}; /* namespace cat */


//! 'type' command ------------------------------------------------------------------------------------------
namespace type
{
    arg::table::syntax syntax = { 1,
	    arg_str0(NULL, NULL, "<filename>", NULL),
//	    arg_end(1)
    };
    struct act: public arg::table::act_t<syntax>
    {
	static esp_err_t invoke(int argc, char* argv[]);
    }; /* struct type::act */
    const esp::console::cmd_t<act> cmd("type", "Type from a keyboard to standard output (default - to screen) and storing keyboard typing to the file <filename> (if specified)");
}; /* namespace type */


//! register all fs commands ----------------------------------------------------------------------
void register_fs_cmd_all(void)
{

    pwd::cmd.enreg_chk();
    cd::cmd.enreg_chk();
    mk_dir::cmd.enreg_chk();
    rm_dir::cmd.enreg_chk();
    ls::cmd.enreg_chk();
    cp::cmd.enreg_chk();
    mv::cmd.enreg_chk();
    rm::cmd.enreg_chk();
    cat::cmd.enreg_chk();
    type::cmd.enreg_chk();

    //device.host().io_int_enable();
    //sdmmc_card.io.enable_int();

}; /* register_fs_cmd_all */



//--[ 'sd' command ]-------------------------------------------------------------------------------


namespace sdcard
{
    arg::table::syntax syntax = { 2,
	    arg_rex1(NULL, NULL, "h|help", "h | help", 0/*REG_ICASE*/, "help for command 'sdcard'"),
	    arg_rem ("|", NULL),
	    arg_rex1(NULL, NULL, "<subcommand>", NULL, 0/*REG_ICASE*/, "other subcommand of command 'sdcard'"),
	    arg_strn(NULL, NULL, "<options>", 0, 2, "subcommand options"),
    };
    struct act: public arg::table::act_t<syntax>
    {
	/// execution procedure for the sd/sdcard command
	static esp_err_t invoke(int argc, char* argv[]);
	/// action for 'mount' command
	static esp_err_t mnt(int argc, char* argv[]);
	/// action for 'unmount' command
	static esp_err_t umnt(int argc, char* argv[]);
	/// print info about the mounted SD-card
	static esp_err_t info(SD::MMC::Device& dev);
    }; /* struct sdcard::act */
    const esp::console::cmd_t<act> cmd("sdcard", "SD card manipulating main command" /*, .hint = "enter subcommand for Sd card operations" */);
    const esp::console::cmd_t<act> cmd2("sd", "shortcut for 'sdcard' command" /*, .hint = "enter subcommand for Sd card operations" */);
}; /* namespace sdcard */




//--[ command action wrapper class ]----------------------------------------------------------------

// standart subcommand action wrapper
class act_cmd
{
public:
    act_cmd(std::function<bool(std::string_view)> cmp_def,
	    esp_err_t (*exec_action)(std::vector<char*>),
	    const char cmd_name[] = "<spared>",
	    void** args = nullptr):
		argtable(args),
		name(cmd_name),
		cmp_core(cmp_def),
		action(exec_action)
    {};

    virtual ~act_cmd() {};

    // compare inner name of command with extname
    virtual bool cmp(char extname[]) const;
    virtual bool cmp(const char extname[]) const;
    virtual bool cmp(std::string_view extname) const;
    virtual bool cmp(const act_cmd& extcmd) const;

    // exec stored command
    virtual esp_err_t exec(int argc, char* argv[]) const;

    std::string_view title() const { return name; };

    void** argtable;

protected:
    /*std::string_view*/const char* name;

    std::function<bool(std::string_view)> cmp_core;

    /// execute action pointer
    esp_err_t (*action)(std::vector<char*> args);

}; /* class act_cmd */





inline bool operator == (const act_cmd& lf, const act_cmd& rt) {
    return lf.cmp(rt); };

inline bool operator == (const act_cmd& lf, const std::string_view extnm) {
    return lf.cmp(extnm); };

inline bool operator == (const std::string_view extnm, const act_cmd& rt) {
    return rt.cmp(extnm); };

inline bool operator != (const act_cmd& lf, const act_cmd& rt) {
    return !lf.cmp(rt); };

inline bool operator != (const act_cmd& lf, const std::string_view extnm) {
    return !lf.cmp(extnm); };

inline bool operator != (const std::string_view extnm, const act_cmd& rt) {
    return !rt.cmp(extnm); };


inline bool act_cmd::cmp(char str[]) const {
    return cmp_core(str);
}; /* act_cmd::cmp() */

inline bool act_cmd::cmp(const char str[]) const {
    return /*(std::string_view(name) == str)*/ cmp_core(str);
}; /* act_cmd::cmp() */

inline bool act_cmd::cmp(std::string_view str) const {
    return cmp_core(str);
}; /* act_cmd::cmp() */

inline bool act_cmd::cmp(const act_cmd& extcmd) const {
    return cmp_core(extcmd.title());
}

// standard execution (non-shifted) of the operated command in class act_cmd
esp_err_t act_cmd::exec(int argc, char* argv[]) const
{
    return action(astr::makestor<std::vector<char*>>(argc, argv));
}; /* act_cmd::exec() */



//--[ SD card control commands]--------------------------------------------------------------------

// classs of the SD control command implementation
class SDctrl
{
public:
    /// get unigue single instance of the SDcmd object
    static SDctrl& cmd();
    /// execute the 'SD' command
    static esp_err_t exec(int argc, char **argv);

    /// register sub-command aka options of the 'SD' command
    esp_err_t enroll(const act_cmd& subcmd);

    /// the help action static wrapper procedure for singleton object
    static esp_err_t help_act(std::vector<char*> args);


    /// error handler action if subcommand is absent, argc/argv version
    static esp_err_t err_none(int argc, char* argv[]);
    /// error handler action if subcommand is absent
    static esp_err_t err_none(std::vector<char*> args) {
	//	ESP_LOGW("act_none::exec()", "None of subcommand action execution, command is: \"%s\"", argv[0]);
    	return err_none(args.size(), args.data());
    }; /* SDctrl::err_none() */

    /// error handler action if subcommand unknown
    static esp_err_t err_unknown(std::vector<char*> args);


    class act_shft: public act_cmd
    {
    public:
        act_shft(std::function<bool(std::string_view)> accept,
    	    esp_err_t (*exec_act)(std::vector<char*>),
    	    const char cmd_name[] = "<spared>", void**args = nullptr):
    	act_cmd(accept, exec_act, cmd_name, args)
        {};

        // exec stored command
        esp_err_t exec(int argc, char* argv[]) const override;

    }; /* class act_shft */


    /// Storage wrapper for reference to class act_cmd
    class act_ref
    {
    public:
        act_ref(const act_cmd& cmd): hold(cmd) {};
        const act_cmd& get() const { return hold;}
        operator const act_cmd&() const { return get();}

        const act_cmd& hold;
    }; /* class act_ref */


private:

    /// object help for object execution procedure
    esp_err_t help(int argc, char* argv[]);

    // syntax0: h | help
    static const void* arg_help[];

    /// list of the defined commands
    std::list<act_ref> syntax;

    SDctrl();		// Default constructor - private for singleton
    SDctrl(SDctrl&) = delete;	// copy constructor forbidden for singleton
    SDctrl& operator =(const SDctrl&) = delete;	// operator "=" - forbidden for singleton

    /// Initialization the Help subsystem
    void InitHelp();

    Arg::table argtable;


    //-- temporary - only for development time -----------
    static void** alltables;	// for initializing singleton at the initial phase of programm
    static void** tables();

    static SDctrl& instance;    // Unique single instance of the SDcmd object

}; /* class SDctrl */



#if 0
//--[ 'sd' command ]-------------------------------------------------------------------------------


namespace sdcard
{
    arg::table::syntax::def syntax = { 2,
	    arg_rex1(NULL, NULL, "h|help", "h | help", 0/*REG_ICASE*/, "help for command 'sdcard'"),
	    arg_rem ("|", NULL),
	    arg_rex1(NULL, NULL, "<subcommand>", NULL, 0/*REG_ICASE*/, "other subcommand of command 'sdcard'"),
	    arg_strn(NULL, NULL, "<options>", 0, 2, "subcommand options"),
//	    arg_end(2),
    };
    struct act: public arg::table::act_t<syntax>
    {
	/// execution procedure for the sd/sdcard command
	static esp_err_t invoke(int argc, char* argv[]);
	/// action for 'mount' command
	static esp_err_t mnt(int argc, char* argv[]);

    }; /* struct sdcard::act */
    const esp::console::cmd_t<act> cmd("sdcard", "SD card manipulating main command" /*, .hint = "enter subcommand for Sd card operations" */);
    const esp::console::cmd_t<act> cmd2("sd", "shortcut for 'sdcard' command" /*, .hint = "enter subcommand for Sd card operations" */);

}; /* namespace sdcard */
#endif



// Register all SD-card commands
void register_sdcard_cmd(void)
{
	/// command action definitions
	static const act_cmd help_cmd([](std::string_view str) {return str == "help"|| str == "h";}, SDctrl::help_act, "help");
	static const SDctrl::act_shft  mnt_cmd([](std::string_view str) {return str == "mount" || str == "m";}, act::mnt, "mount");
	static const SDctrl::act_shft umnt_cmd([](std::string_view str) {return str == "umount"|| str == "u";},act::umnt, "umount");
	static const SDctrl::act_shft info_cmd([](std::string_view str) {return str == "info"  || str == "i";},
					[](std::vector<char*>) -> esp_err_t { return act::info(device);}, "info");
	static const SDctrl::act_shft  pwd_cmd([](std::string_view str) {return str == "pwd"   || str == "p";}, act::pwd, "pwd");
	static const SDctrl::act_shft  mkd_cmd([](std::string_view str) {return str == "mkdir";}, act::mkdir, "mkdir");
	static const SDctrl::act_shft  rmd_cmd([](std::string_view str) {return str == "rmdir";}, act::rmdir, "rmdir");
	static const SDctrl::act_shft   cd_cmd([](std::string_view str) {return str == "cd";}, act::cd, "cd");
	static const SDctrl::act_shft   ls_cmd([](std::string_view str) {return str == "ls" || str == "dir";}, act::ls, "ls");
	static const SDctrl::act_shft   cp_cmd([](std::string_view str) {return str == "cp" || str == "copy";}, act::cp, "cp");
	static const SDctrl::act_shft   mv_cmd([](std::string_view str) {return str == "mv" || str == "move";}, act::mv, "mv");
	static const SDctrl::act_shft   rm_cmd([](std::string_view str) {return str == "rm" || str == "del";}, act::rm, "rm");
	static const SDctrl::act_shft  cat_cmd([](std::string_view str) {return str == "cat" || str == "c";}, act::cat, "cat");
	static const SDctrl::act_shft type_cmd([](std::string_view str) {return str == "type"|| str == "t";}, act::type, "type");


    SDctrl::cmd().enroll(mnt_cmd);
    SDctrl::cmd().enroll(umnt_cmd);
    SDctrl::cmd().enroll(info_cmd);
    SDctrl::cmd().enroll(pwd_cmd);
    SDctrl::cmd().enroll(mkd_cmd);
    SDctrl::cmd().enroll(rmd_cmd);
    SDctrl::cmd().enroll(cd_cmd);
    SDctrl::cmd().enroll(ls_cmd);
    SDctrl::cmd().enroll(cp_cmd);
    SDctrl::cmd().enroll(mv_cmd);
    SDctrl::cmd().enroll(rm_cmd);
    SDctrl::cmd().enroll(cat_cmd);
    SDctrl::cmd().enroll(type_cmd);
    SDctrl::cmd().enroll(help_cmd);

    //TODO Fimally - this registering must be moved into procedure register_fs_cmd_all() or not
    sdcard::cmd.enreg_chk();
    sdcard::cmd2.enreg_chk();

}; /* register_sdcard_all */



//--[ class SDctrl ]------------------------------------------------------------


// Default constructor
SDctrl::SDctrl()
{
    // Initialize base part of subcommand list with terminal cmd obj:
    // error_none & error_unknown subcommand ojects
	static const act_cmd none_cmd([](std::string_view str) {return str == "";}, SDctrl::err_none, "none");
	static const act_cmd unknown_cmd([](std::string_view str) {return true;}, SDctrl::err_unknown, "unknown");

    syntax.push_back(none_cmd);
    syntax.push_back(unknown_cmd);

    // temporary - only for development time of moving help subsysten into SDctrl class
    alltables = tables();

    InitHelp();

}; /* SDctrl::SDctrl() */


/// Initialization the Help subsystem (Temporariliy, only for devel?)
void SDctrl::InitHelp()
{
    cout << "***********************************************************************" << endl;
    cout << "*** Initializing Syntax Tables for the Help at the Start            ***" << endl;
    cout << "***********************************************************************" << endl;


    cout << "***                                                                 ***" << endl
	 << "*** Start Initializing the Help subsystem.                          ***" << endl
	 << "***                                                                 ***" << endl;

    // syntax0: h | help
    argtable.add(arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'"));
//	static void* arg_help[] = {
//		arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'"),
//		arg_end(2),
//	};

#define innerqte(a) #a
#define qte(a) innerqte(a)
    // syntax1: m | mount [<device>] [<mountpoint>] "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters are optional"
    // syntax1: corrected m | mount [<slot>] [<mountpoint>] "m|mount", NULL, 0, "mount SD-card <device> to <mountpoint>, parameters are optional"
    argtable.add(arg_rex1(NULL, NULL, "m|mount", NULL, 0, "mount SD-card [<device>] to [<mountpoint>], parameters are optional"));
//	static void* arg_mnt[] = {
//		arg_rex1(NULL, NULL, "m|mount", NULL, 0, "mount SD-card [<device>] to [<mountpoint>], parameters are optional"),
//		arg_str0(NULL, NULL, "<slot>", "SD card slot (device) number, used slot #"  qte(SDMMC_HOST_SLOT_1)  " default value if omitted"),
//		arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, used path \"" SD_MOUNT_POINT "\" if omitted"),
//		arg_end(2),
//	};
    // syntax2: u | umount [ <device> | <mountpoint> ] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
    // syntax2 corrected: u | umount [<mountpoint>] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
    argtable.add(arg_rex1(NULL, NULL, "u|umount", NULL, 0, "unmount SD-card [<path>] where the SD card is mounted; if parameters omitted - use \"" SD_MOUNT_POINT "\"" ));
//	static void* arg_umnt[] = {
//		arg_rex1(NULL, NULL, "u|umount", NULL, 0, "unmount SD-card [<path>] where the SD card is mounted; if parameters omitted - use \"" SD_MOUNT_POINT "\"" ),
//		arg_str0(NULL, NULL, "<mountpoint>", NULL),
//		arg_end(2),
//	};
//----------------------------------------------------------------------------------------------------------------------
    // syntax3: info "information about mounted SD-card"
    argtable.add(arg_rex1(NULL, NULL, "i|info", NULL, 0, "information about mounted SD-card"));
//	static void* arg_info[] = {
//		arg_rex1(NULL, NULL, "i|info", NULL, 0, "information about mounted SD-card"),
////		arg_str0(NULL, NULL, "<pattern>", "file pattern or path"),
//		arg_end(2),
//	};
    // syntax4: pwd "current directory name"
    argtable.add(arg_rex1(NULL, NULL, "p|pwd", NULL, 0, "current directory name"));
//	static void* arg_pwd[] = {
//		arg_rex1(NULL, NULL, "p|pwd", NULL, 0, "current directory name"),
////		arg_str0(NULL, NULL, "<pattern>", "file pattern or path"),
//		arg_end(2),
//	};
    // syntax5: mkdir [<path>] "make new directory with name <path>"
    argtable.add(arg_rex1(NULL, NULL, "mkdir", NULL, 0, "make new directory with name \"<path>\""));
//	static void* arg_mkdir[] = {
//		arg_rex1(NULL, NULL, "mkdir", NULL, 0, "make new directory with name \"<path>\""),
//		arg_str0(NULL, NULL, "<path>", NULL/*"name of the new directory"*/),
//		arg_end(2),
//	};
	// syntax6: rmdir [<path>] "delete existing empty directory with name <path>"
    argtable.add(arg_rex1(NULL, NULL, "rmdir", NULL, 0, "delete existing empty directory with name \"<path>\""));
//	static void* arg_rmdir[] = {
//		arg_rex1(NULL, NULL, "rmdir", NULL, 0, "delete existing empty directory with name \"<path>\""),
//		arg_str0(NULL, NULL, "<path>", NULL/*"name of the new directory"*/),
//		arg_end(2),
//		};
    // syntax7: cd [<path>] "change current directory to <path>"
    argtable.add(arg_rex1(NULL, NULL, "cd", NULL, 0, "change current directory to a <path>"));
//	static void* arg_cd[] = {
//		arg_rex1(NULL, NULL, "cd", NULL, 0, "change current directory to a <path>"),
//		arg_str0(NULL, NULL, "<path>", NULL/*"path to which the current directory is changed"*/),
//		arg_end(2),
//	};
//----------------------------------------------------------------------------------------------------------------------
    // syntax8: cp <src> <dest> "copy file <src> to <dest>"
    argtable.add(arg_rex1(NULL, NULL, "cp|copy", NULL, 0, "copy file <src> to <dest>"));
//	static void* arg_cp[] = {
//		arg_rex1(NULL, NULL, "cp|copy", NULL, 0, "copy file <src> to <dest>"),
//		arg_str1(NULL, NULL, "<src>", NULL/*"file name to copy"*/),
//		arg_str1(NULL, NULL, "<dest>", NULL/*"where file to copy"*/),
//		arg_end(3),
//	};
    // syntax9: mv <src> <dest> "rename/move file <src> to <dest>"
    argtable.add(arg_rex1(NULL, NULL, "mv|ren", NULL, 0, "rename/move file <src> to <dest>"));
//	static void* arg_mv[] = {
//		arg_rex1(NULL, NULL, "mv|ren", NULL, 0, "rename/move file <src> to <dest>"),
//		arg_str1(NULL, NULL, "<src>", NULL/*"source file name to copy or rename/move"*/),

    //		arg_str1(NULL, NULL, "<dest>", NULL/*"where file to copy or rename/move"*/),
//		arg_end(3),
//		};
    // syntax10: rm [<pattern>] "delete file <pattern>"
    argtable.add(arg_rex1(NULL, NULL, "rm", NULL, 0, "delete file according <pattern>"));
//	static void* arg_rm[] = {
//		arg_rex1(NULL, NULL, "rm", NULL, 0, "delete file according <pattern>"),
//		arg_str1(NULL, NULL, "<pattern>", NULL/*"file name to delete"*/),
//		arg_end(2),
//	};
    // syntax11: ls | dir [<pattern>] "print directory contents on SD-card"
    argtable.add(arg_rex1(NULL, NULL, "ls|dir", NULL, 0, "print directory contents on SD-card according <pattern>"));
//	static void* arg_ls[] = {
//		arg_rex1(NULL, NULL, "ls|dir", NULL, 0, "print directory contents on SD-card according <pattern>"),
//		arg_str0(NULL, NULL, "<pattern>", NULL/*"file pattern or path for lising"*/),
//		arg_end(2),
//	};
    // syntax12: cat <filename> "print file to stdout (console output)"
    argtable.add(arg_rex1(NULL, NULL, "cat", NULL, 0, "print content of the file \"<filename>\" to screen"));
//	static void* arg_cat[] = {
//		arg_rex1(NULL, NULL, "cat", NULL, 0, "print content of the file \"<filename>\" to screen"),
//		arg_str1(NULL, NULL, "<filename>", NULL),
//		arg_end(2),
//	};
    // syntax  : type [filename] "type from the keyboard to file & screen or screen only; <file name> - name of the file is to be printed; if omitted - print to screen only"
    argtable.add(arg_rex1(NULL, NULL, "type", NULL, 0, "type from the keyboard to a file & screen or screen only if the file omitted"));
//	static void* arg_type[] = {
//		arg_rex1(NULL, NULL, "type", NULL, 0, "type from the keyboard to a file & screen or screen only if the file omitted"),
////		"Type from a keyboard to standard output (default - to screen) and storing keyboard typing to the file <filename> (if specified)"		arg_str0(NULL, NULL, "<file>", "file name to be printed or the name of where the typed text is saved"),
//		arg_end(2),
//	};

#if 0
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
#endif
    ;
}; /* SDctrl::InitHelp() */


/// object "help" execution procedure
inline esp_err_t SDctrl::help(int argc, char* argv[])
{
    //ESP_LOGW("SDctrl::help", "Help wrapper call: exec syntax.help");
    cout << "#### Help action, implemented in the SDctrl class, method help(int argc, char* argv[]), argc=" << argc << ", argv[0]='" << argv[0] << "'. ####" << endl;
#if 0
    if (!tables())
    {
	cout << "!!! Error: syntax tables is undefined. !!!" << endl;
	cout << "Abort command" << endl;
	return ESP_ERR_INVALID_SIZE;
    }; /* if !hlp_arg */

    if (!tables()[0])
    {
	cout << "!!! Error: syntax tables for 1'st command is undefined. !!!" << endl;
	cout << "Abort command" << endl;
	return ESP_ERR_INVALID_ARG;
    }; /* if !tables()[0] */
#endif
    cout << "Usage: " << argv[0];
#if 0
//    arg_print_syntax(stdout, (void**)alltables[0], "\n");
    arg_print_syntax(stdout, (void**)arg_help, "\n");
#endif

//    for (void **currcmd = tables() + 1; *currcmd != NULL; currcmd++)
//    {
//	cout << "       " << argv[0];
//	arg_print_syntax(stdout, (void**)*currcmd, "\n");
//    }; /* for void **currcmd */
    void** dcc = argtable.syntax().data();
//    arg_print_syntax(stdout, argtable.syntax().data(), "\n");
    arg_print_syntax(stdout, dcc, "\n");

    cout << "Command \"" << argv[0] << "\" supports the ESP32 operation with an SD card." << endl;
    cout << "Use subcommands to invoke individual operations; operation are: mount, unmount, ls, cat, type, help." << endl;

//#if 0 // --
    for (void **currcmd = tables(); *currcmd != NULL; currcmd++)
	arg_print_glossary(stdout, (void**)*currcmd, "      %-20s %s\n");
//#endif


	//arg_print_glossary(stdout, (void**)argtable.syntax().data(), "      %-20s %s\n");

    return ESP_OK;

}; /* SDctrl::help() */


/// the help action static wrapper procedure for singleton object
esp_err_t SDctrl::help_act(std::vector<char*> args)
{
    return instance.help(args.size(), args.data());
}; /* SDctrl::help_act() */



/// error handler action if subcommand is absent, argc/argv version
esp_err_t SDctrl::err_none(int argc, char* argv[])
{
    ESP_LOGE("sdcard command", "subcommand missing, what to run?");
    cout << "Try \"" << argv[0] << " help\" for more information." ;
    return ESP_OK;
}; /* SDctrl::err_none() */


/// error handler action if subcommand unknown
esp_err_t SDctrl::err_unknown(std::vector<char*> args)
{
//    	ESP_LOGW("act_unknown::exec()", "Unknown subcommand is present, command is: \"%s\", subcommand: \"%s\"", argv[0], argv[1]);
    ESP_LOGE("sdcard command", "Unknown options: \"%s\".", args[1]);
    cout << "Try \"" << args[0] << " help\" for more information." ;
    return ESP_OK;
}; /* SDctrl::err_unknown() */


// get unigue single instance of the SDcmd object
SDctrl& SDctrl::cmd()
{
	static SDctrl instance;

    return instance;
}; /* SDctrl::cmd() */

// Unique single instance of the SDcmd object
SDctrl& SDctrl::instance = SDctrl::cmd();


// shifted execution - drop the first argument of the calling command
esp_err_t SDctrl::act_shft::exec(int argc, char* argv[]) const
{
    return action(astr::makestor<std::vector<char*>>(argc - 1, argv + 1));
}; /* act_cmd::exec() */



// execute the 'SD' command
esp_err_t SDctrl::exec(int argc, char **argv)
{

//    instance.store(argc, argv);

    if (argc == 1)
	return err_none(argc, argv);

    return std::find(instance.syntax.begin(), instance.syntax.end(), argv[1])->hold.exec(argc, argv);

//    cout << endl;
//    return ESP_OK;
}; /* SDctrl::exec */


/// register sub-command aka options of the 'SD' command
esp_err_t SDctrl::enroll(const act_cmd& subcmd)
{
    syntax.insert(std::prev(syntax.end(), 2) , subcmd);
    return ESP_OK;
}; /* SDctrl::enroll() */



void** SDctrl::tables()
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
		arg_str0(NULL, NULL, "<mountpoint>", "path to mountpoint SD card, used path \"" SD_MOUNT_POINT "\" if omitted"),
		arg_end(2),
	};
    // syntax2: u | umount [ <device> | <mountpoint> ] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
    // syntax2 corrected: u | umount [<mountpoint>] "unmount SD-card <device> or that was mounted to <path>; if all parameters omitted - use default values - ..."
	static void* arg_umnt[] = {
		arg_rex1(NULL, NULL, "u|umount", NULL, 0, "unmount SD-card [<path>] where the SD card is mounted; if parameters omitted - use \"" SD_MOUNT_POINT "\"" ),
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

#if 0
	// Initializing the Syntax Tables at the Start
	if (!alltables)
	{
	    cout << "*** Initialize the new Argtable field" << endl;

	    cout << "--- Add arg_mnt option" << endl;
	    instance.argtable.addoption(arg_mnt[0]);
	    cout << "--- Add arg_umnt option" << endl;
	    instance.argtable.addoption(arg_umnt[0]);
	    cout << "--- Add arg_info option" << endl;
	    instance.argtable.addoption(arg_info[0]);
	    cout << "--- Add arg_pwd option" << endl;
	    instance.argtable.addoption(arg_pwd[0]);
	    cout << "--- Add arg_mkdir option" << endl;
	    instance.argtable.addoption(arg_mkdir[0]);
	    cout << "--- Add arg_rmdir option" << endl;
	    instance.argtable.addoption(arg_rmdir[0]);
	    cout << "--- Add arg_cd option" << endl;
	    instance.argtable.addoption(arg_cd[0]);
	    cout << "--- Add arg_cp option" << endl;
	    instance.argtable.addoption(arg_cp[0]);
	    cout << "--- Add arg_mv option" << endl;
	    instance.argtable.addoption(arg_mv[0]);
	    cout << "--- Add arg_rm option" << endl;
	    instance.argtable.addoption(arg_rm[0]);
	    cout << "--- Add arg_ls option" << endl;
	    instance.argtable.addoption(arg_ls[0]);
	    cout << "--- Add arg_cat option" << endl;
	    instance.argtable.addoption(arg_cat[0]);
	    cout << "--- Add arg_type option" << endl;
	    instance.argtable.addoption(arg_type[0]);
	    cout << "--- Add arg_help option" << endl;
	    instance.argtable.addoption(arg_help[0]);
	}; /* if !alltables */
#endif


//    cout << "**  Get the Syntax Tables in SDctrl::Syntax::tables().  **" << endl;
    return syntaxes;

}; /* SDctrl::Syntax::tables */

// syntax0: h | help
const void* SDctrl::arg_help[] = {
	arg_rex1(NULL, NULL, "h|help", "h|help", 0/*REG_ICASE*/, "help by subcommand of command 'sdcard'"),
	arg_end(2),
}; /* void* SDctrl::arg_help[] */


// test variant while full SDctrl singleton object help initialization
void** SDctrl::alltables = nullptr;







//--[ command execution procedures definition ]------------------------------------------------------------------------


/// Execute procedure of the 'sd' command
esp_err_t sdcard::act::invoke(int argc, char* argv[])
{
    cout << "Run the command \"sdcard\'" << endl
	 << endl;
    cout << "argc is   : " << argc << endl;
    for (int i = 0; i < argc; i++)
	cout << "argv[" << i << "] is: " << argv[i] << endl;
    cout << "..............................................."
	 << endl;

    return SDctrl::exec(argc, argv);

}; /* sdcard::act::invoke() */


/// action for 'mount' command
esp_err_t sdcard::act::mnt(int argc, char* argv[])
{
    esp_err_t res = ESP_FAIL;

    //device.host().set_card_clk(40000);	// test for low speed

    switch (argc)
    {
    case 1/*2*/:
	res = exec_server.mount(device, sdmmc_card); // @suppress("Invalid arguments")
	break;

    case 2/*3*/:
	cout << "...with one parameter - use device or mount point." << endl;
	res = exec_server.mount(device, sdmmc_card, argv[1/*2*/]); // @suppress("Invalid arguments")
	break;

    case 3/*4*/:
	cout << "...with two parameters - use device & mount point." << endl;
	res = exec_server.mount(device, sdmmc_card, atoi(argv[1/*2*/]), argv[2/*3*/]); // @suppress("Invalid arguments")
	break;

    default:
	ESP_LOGE("sdcard mount command", "more than two parameters (%d) is not allowed", argc - 1/*2*/);
//	res = ESP_FAIL;
    }; /* switch argc */
    cout << endl;

    if (res == ESP_OK)
    {
	device.host.io.interrupt.enable();
	sdmmc_card.io.interrupt.enable();
	device.card->info();
    }; /* if res == ESP_OK */

    return res;
}; /* sdcard::act::mnt() */

/// action for 'mount' command
esp_err_t act::mnt(std::vector<char*> args) {
    return sdcard::act::mnt(args.size(), args.data());
}; /* act::mnt() */


/// action for 'unmount' command
esp_err_t sdcard::act::umnt(int argc, char* argv[])
{
    cout << "\"unmount\" command execution" << endl;
    switch (argc)
    {
    case 1/*2*/:
	cout << "...without parameters - use default values." << endl;
	return exec_server.unmount(device); // @suppress("Invalid arguments")
	break;

//    case 2/*3*/:
//	cout << "...with one parameter - use device or mount point." << endl;
//	return exec_server.unmount(args[1/*2*/]);
//	break;

    default:
	ESP_LOGE("sdcard umount command", "more than one parameters (%d) - is not allowed", argc - 1/*2*/);
    }; /* switch args.size() */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* sdcard::act::umnt() */

/// action for 'unmount' command
esp_err_t act::umnt(std::vector<char*> args)
{
    return sdcard::act::umnt(args.size(), args.data());
}; /* act::umnt() */


/// print info about the mounted SD-card
esp_err_t sdcard::act::info(SD::MMC::Device& dev)
{
    if (!dev.card)
    {
	ESP_LOGW("sdcard info command", "SD-card now is not mounted!!!");
	return ESP_ERR_NOT_FOUND;
    }; //* if !dev.card */
    dev.card->info();
    cout << "Pullup is: " << ((dev.slot.flags() & SD::MMC::Slot::flag::pullup)? "Enabled": "Absent") << endl;
    cout << "###############################################" << endl;

#define TAG "SD Command Service"

    ESP_LOGI(TAG, "SD card info:");
        ESP_LOGI(TAG, "\tBus width (log2): %d", sdmmc_card.self->log_bus_width);
        ESP_LOGI(TAG, "\tFreq (kHz): %'" PRIu32, sdmmc_card.self->max_freq_khz);
        ESP_LOGI(TAG, "\tDDR: %d", sdmmc_card.self->is_ddr);
        ESP_LOGI(TAG, "\tCID: Date %d, MFG_ID %d, Name %s, OEM ID %d, Rev %d, Serial %d", sdmmc_card.self->cid.date, sdmmc_card.self->cid.mfg_id, sdmmc_card.self->cid.name, sdmmc_card.self->cid.oem_id, sdmmc_card.self->cid.revision, sdmmc_card.self->cid.serial);
        ESP_LOGI(TAG, "\tCSD: Capacity %'d, Card Common Class %d, CSD version %d, MMC version %d, read block len %d, sector size %d, tr speed %'d", sdmmc_card.self->csd.capacity, sdmmc_card.self->csd.card_command_class, sdmmc_card.self->csd.csd_ver, sdmmc_card.self->csd.mmc_ver, sdmmc_card.self->csd.read_block_len, sdmmc_card.self->csd.sector_size, sdmmc_card.self->csd.tr_speed);
        ESP_LOGI(TAG, "\tCSD: Ease mem state <undef>%d, Power class %d, Revision <undef>%d, Sec feature <undef>%d", /*sdmmc_card.self->ext_csd.erase_mem_state*/-1, sdmmc_card.self->ext_csd.power_class, /*sdmmc_card.self->ext_csd.rev*/-1, /*sdmmc_card.self->ext_csd.sec_feature*/-1);
        ESP_LOGI(TAG, "\tSCR: bus width %d, erase mem state <undef%d>, reserved <undef%d>, rsvd_mnf <undef%d>, sd_spec %d", sdmmc_card.self->scr.bus_width, /*sdmmc_card.self->scr.erase_mem_state*/-1, /*sdmmc_card.self->scr.reserved*/-1, /*sdmmc_card.self->scr.rsvd_mnf*/-1, sdmmc_card.self->scr.sd_spec);
        //ESP_LOGI(TAG, "\tSSR: cur_bus_width %d, discard_support %d, fule_support %d, reserved %d", sdmmc_card.self->ssr.cur_bus_width, sdmmc_card.self->ssr.discard_support, sdmmc_card.self->ssr.fule_support, sdmmc_card.self->ssr.reserved);

    cout << "###############################################" << endl;

	esp_err_t err;
    err = dev.card->print_cis();
    ESP_LOGE("sdcard info command", "Error %i in the get or print CIS data: %s", err, esp_err_to_name(err));
    return err;
}; /* sdcard::act::info() */

/// print info about the mounted SD-card
esp_err_t act::info(SD::MMC::Device& dev)
{
    return sdcard::act::info(dev);
}; /* act::info() */



/// action for pwd command
static int pwd::invoke(int argc, char *argv[])
{
    exec_server.pwd();
    return ESP_OK;
}; /* pwd::invoke() */

/// action for pwd command
esp_err_t act::pwd(std::vector<char*> args)
{
    return pwd::invoke(args.size(), args.data());
}; /* act::pwd() */


/// action for 'mkdir' command
esp_err_t mk_dir::act::invoke(int argc, char *argv[])
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
}; /* mk_dir::act::invoke() */

/// action for 'mkdir' command
esp_err_t act::mkdir(std::vector<char*> args) {
    return mk_dir::act::invoke(args.size(), args.data());
}; /* act::mkdir() */


/// action for 'rmdir' command
esp_err_t rm_dir::act::invoke(int argc, char *argv[])
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
}; /* rm_dir::act::invoke() */

/// action for 'rmdir' command
esp_err_t act::rmdir(std::vector<char*> args) {
    return rm_dir::act::invoke(args.size(), args.data());
}; /* act::rmdir() */


/// action for 'cd' command
esp_err_t cd::act::invoke(int argc, char *argv[])
{
    cout << "\"cd\" command execution" << endl;
    switch (argc)
    {
    case 1:
	return exec_server.cd(device);
	break;

    case 2:
	return exec_server.cd(device, argv[1]);
	break;

    default:
	ESP_LOGE("cd command", "too many parameters (%d), where to go?", argc);
    }; /* switch argc */
    cout << endl;

    return ESP_ERR_INVALID_ARG;
}; /* cd::act::invoke() */

/// action for 'cd' command
esp_err_t act::cd(std::vector<char*> args) {
    return cd::act::invoke(args.size(), args.data());
}; /* act::cd() */


/// action for list/dir command
esp_err_t ls::act::invoke(int argc, char *argv[])
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
}; /* ls::act::invoke() */

/// action for list/dir command
esp_err_t act::ls(std::vector<char*> args) {
    return ls::act::invoke(args.size(), args.data());
}; /* act::ls() */


// action for 'copy' command
esp_err_t cp::act::invoke(int argc, char* argv[])
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
}; /* cp::act::invoke() */

// action for 'copy' command
esp_err_t act::cp(std::vector<char*> args) {
    return cp::act::invoke(args.size(), args.data());
}; /* act::cp() */


/// action for 'rename/move' command
esp_err_t mv::act::invoke(int argc, char* argv[])
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
}; /* mv::act::invoke() */

/// action for 'rename/move' command
esp_err_t act::mv(std::vector<char*> args) {
    return mv::act::invoke(args.size(), args.data());
}; /* act::mv() */


/// action for 'rm' command
esp_err_t rm::act::invoke(int argc, char* argv[])
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
}; /* rm::act::invoke() */

/// action for 'rm' command
esp_err_t act::rm(std::vector<char*> args) {
    return rm::act::invoke(args.size(), args.data());
}; /* act::rm() */


/// action for the 'cat' command
esp_err_t cat::act::invoke(int argc, char* argv[])
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
}; /* cat::act::invoke() */

/// action for 'cat' command
esp_err_t act::cat(std::vector<char*> args) {
    return cat::act::invoke(args.size(), args.data());
}; /* act::cat() */


/// action for 'type' command
esp_err_t type::act::invoke(int argc, char* argv[])
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
    }; /* switch args.size() */
    cout << endl;
    return ESP_ERR_INVALID_ARG;
}; /* type::act::invoke() */

/// action for 'type' command
esp_err_t act::type(std::vector<char*> args) {
    return cat::act::invoke(args.size(), args.data());
}; /* act::type() */


//--[ cmd_sdcard.cpp ]-----------------------------------------------------------------------------
