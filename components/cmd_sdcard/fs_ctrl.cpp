/*!
 * @brief Filesystem on storege device (SD-card etc) control/navigation classes
 * Implementation file
 * 	@file: fs_ctrl.cpp
 *	@author: Solomatov A.A. aso
 *	@date 14.07.2022 - 27.04.2024
 *	@version: 0.7
 */

//#define __PURE_C__


#include <limits>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdarg>

#include <string>
#include <cstring>
#include <cctype>
#include <sys/unistd.h>
#include <cerrno>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_console.h>
#include <esp_system.h>
#include <argtable3/argtable3.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
#include <regex>
//#ifdef __PURE_C__
////#include <fcntl.h>
//#include <dirent.h>
//#else
  //#if __cplusplus < 201703L
#ifndef __PURE_C__
#include <fcntl.h>
#endif // ifndef __PURE_C__
#include <dirent.h>
  //#else
  //#endif // __cplusplus < 201703L
//#endif // ifdef __PURE_C__

#include <esp_vfs_fat.h>
#include "sdkconfig.h"



#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>

#include <cwd_emulate>
#include <sdcard_io>
#include "fs_ctrl"


#include <extrstream>
#include <astring.h>

//using namespace idf;
using namespace std;


#define SD_MOUNT_POINT CONFIG_UNIT_SD_CARD_MOUNT_POINT


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

namespace Exec	//-----------------------------------------------------------------------------------------------------
{

//--[ instance of the cwd_emulation ]----------------------------------------------------------------------------------
	CWD artificial_cwd(SD_MOUNT_POINT);


//    static const char *TAG = "SD/MMC service";

//--[ class Cmd ]------------------------------------------------------------------------------------------------------



const char* const Cmd::MOUNT_POINT_Default = SD_MOUNT_POINT;


#undef CMD_TAG_PRFX
#define CMD_TAG_PRFX "SD/MMC CMD Exec server:"


    /// Mount default SD-card slot onto path "mountid", or onto slot no ""mountid, if it's a number, default mountpoint is MOUNT_POINT_Default
    esp_err_t Cmd::mount(SD::MMC::Device& device, SD::MMC::Card& card, std::string mountid)
    {
	ESP_LOGI(TAG, "Mounting SD-Cart to a mountpoint %s", mountid.c_str());

	if (astr::is_digitex(mountid))
	    return mount(device, card, atoi(mountid.c_str())); // @suppress("Invalid arguments")

	ESP_LOGI(TAG, "Mounting SD-Cart to a directory!!!");
	ret = device.mount(card, std::move(mountid)); // @suppress("Method cannot be resolved")
	if (ret == ESP_OK)
	{
#ifdef CONFIG_AUTO_CHDIR_BEHIND_MOUNTING
	    artificial_cwd.change(device.mountpath());
	    ESP_LOGI(TAG, "Current directory autochanged to: %s", artificial_cwd.get().c_str());
#else
//	    change_currdir("/");
	    artificial_cwd.get();	// set fake_cwd according system pwd (through get_cwd())
	    ESP_LOGI(TAG, "Current directory set to: %s,  according system pwd", artificial_cwd.get().c_str());
#endif
	}; /* if ret == ESP_OK */
	return ret;
    }; /* Exec::Cmd::mount() */


    /// Mount SD-card slot "slot_no" onto specified mount path, default mountpoint is MOUNT_POINT_Default
    esp_err_t Cmd::mount(SD::MMC::Device& device, SD::MMC::Card& card, int slot_no, std::string mountpoint)
    {
	device.slot_no(slot_no); // @suppress("Method cannot be resolved")
	return device.mount(card, std::move(mountpoint)); // @suppress("Method cannot be resolved")
    }; /* Exec::Cmd::mount() */


    //------------------------------------------------------------------------------------------
    //    // All done, unmount partition and disable SDMMC peripheral
    //    esp_vfs_fat_sdcard_unmount(mount_point, card);
    //    ESP_LOGI(TAG, "Card unmounted");
    //------------------------------------------------------------------------------------------

    /// Unmount SD-card, that mounted onto "mountpath"
    esp_err_t Cmd::unmount(SD::MMC::Device& device/*const char mountpath[]*/)
    {
	if ((ret = device.unmount()) != ESP_OK) // @suppress("Method cannot be resolved")
	    cout << TAG << ": "  << "Unmounting Error: " << ret
		<< ", " << esp_err_to_name(ret) << endl;
	else
	{
	    cout << TAG << ": " << "Card unmounted" << endl;
	    artificial_cwd.clear();	// set fake cwd path to: ""
	}; /* else if device.unmount() != ESP_OK */

	return ret;
    }; /* Exec::Cmd::unmount() */

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


    /// print current working directory name
    esp_err_t Cmd::pwd()
    {
	cout << endl
	    << "PWD is: \"" << artificial_cwd.get() << '"' << endl
	    << endl;
	return ESP_OK;

    }; /* Exec::Cmd::pwd() */



#define CMD_NM "mkdir"
    /// create a new directory
    esp_err_t Cmd::mkdir(std::string dirname)
    {
	dirname = astr::trim(dirname);

	if (!artificial_cwd.valid(dirname))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: the new directory name \"%s\" is invalid", __func__, dirname.c_str());
	    return ESP_ERR_NOT_FOUND;
	}; /* if !artificial_cwd.valid(dirname) */

	if (dirname.empty())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, CMD_NM,
		    "This command required the creating directory name.");
	    return ESP_ERR_INVALID_ARG;
	}; /* if dirname.empty() */

	ESP_LOGI(CMD_TAG_PRFX, "%s: Create directory with name \"%s\"", __func__, dirname.c_str());

	dirname = artificial_cwd / dirname;
	ESP_LOGI(CMD_TAG_PRFX, "Real path is:\t%s", dirname.c_str());

	if (CWD::last::exist())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Invalid argument - requested path \"%s\" is exist; denied create duplication name\n", __func__, dirname.c_str());
	    return ESP_ERR_INVALID_ARG;
	}; /* if CWD::last::exist() */

	errno = 0;
	::mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	if (errno)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Error creating directory \"%s\": %s", __func__, dirname.c_str(), strerror(errno));
	    return ESP_FAIL;
	}; /* if (errno) */
	return ESP_OK;

    }; /* Exec::Cmd::mkdir() */


#undef CMD_NM
#define CMD_NM "rmdir"
    /// delete empty directory
    esp_err_t Cmd::rmdir(std::string dirname)
    {
	dirname = astr::trim(dirname);

	if (!artificial_cwd.valid(dirname))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: the directory name \"%s\" is invalid", __func__, dirname.c_str());
	    return ESP_ERR_NOT_FOUND;
	}; /* if !artificial_cwd.valid(dirname.c_str()) */

	if (dirname.empty())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, CMD_NM,
		     "This command required the name of the deleting directory.");
	    return ESP_ERR_INVALID_ARG;
	}; /* if dirname.empty() */

	ESP_LOGI(CMD_TAG_PRFX, "%s: Delete directory <%s>,", __func__, dirname.c_str());
	dirname = artificial_cwd / dirname;
	ESP_LOGI(CMD_TAG_PRFX, "Real path is:\t%s", dirname.c_str());

	// Check if destination directory or file exists before deleting
	if (!CWD::last::exist())
	{
	    // deleting a non-exist directory is not possible
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Directory \"%s\" is not exist - deleting a non-existent catalogue is not possible.\n%s", __func__, dirname.c_str(), esp_err_to_name(ESP_ERR_NOT_FOUND));
	    return ESP_ERR_NOT_FOUND;
	}; /* if !CWD::last::exist() */
	if (!CWD::last::is_dir())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: The %s command delete directories, not the files.\n%s", __func__, CMD_NM, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	    return ESP_ERR_INVALID_ARG;
	}; /* if !CWD::last::is_dir() */

	    DIR *dir = opendir(dirname.c_str());	// Directory descriptor

	errno = 0;	// clear any possible errors

	    struct dirent *entry = readdir(dir);

	closedir(dir);
	if (errno)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when closing directory \"%s\": %s", __func__, dirname.c_str(), strerror(errno));
	    return ESP_FAIL;
	}; /* if errno */
	if (entry)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Directory \"%s\" is not empty, deletung non-emty directories "
		    "is not supported.", __func__, dirname.c_str());
	    return ESP_ERR_NOT_SUPPORTED;
	}; /* if (entry) */

	errno = 0;
	unlink(dirname.c_str());
	if (errno)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when deleting \"%s\": %s", __func__, dirname.c_str(), strerror(errno));
	    return ESP_FAIL;
	}; /* if errno */

	return ESP_OK;

    }; /* Exec::Cmd::rmdir() */



#undef CMD_NM
#define CMD_NM "cd"

    /// change a current wirking directory
    esp_err_t Cmd::cd(SD::MMC::Device& device, std::string dirname)
    {
	    esp_err_t err;

	dirname = astr::trim(dirname);

	if (!artificial_cwd.valid(dirname))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: the directory name \"%s\" is invalid", __func__, dirname.c_str());
	    return ESP_ERR_NOT_FOUND;
	}; /* if !artificial_cwd.valid(dirname) */

	// change cwd dir: chdir(dirname);
	if (device.mounted())
	{
	    if (dirname.empty())
	    {
		ESP_LOGI(CMD_TAG_PRFX, "%s: Not specified directory for jump to, change current dir to %s, [mountpoint].", __func__, device.mountpath_c());
		artificial_cwd/= device.mountpath();
		err = CWD::last::state();
	    } /* if dirname.empty() */
	    else
	    {
		ESP_LOGI(CMD_TAG_PRFX, "%s: Change current dir to %s", __func__, dirname.c_str());
		artificial_cwd/= dirname;
		err = CWD::last::state();
	    }; /* else if dirname.empty() */
	}
	else
	{
	    ESP_LOGW(CMD_TAG_PRFX, "%s: Card is not mounted, mountpoint is not valid, nothing to do", __func__);
	    return ESP_ERR_NOT_SUPPORTED;
	}; /* else if device.card != nullptr */
	// change cwd dir: chdir(dirname);

	if (err != 0)
	    ESP_LOGE(CMD_TAG_PRFX, "%s: fail change directory to %s\n%s", __func__, dirname.c_str(), esp_err_to_name(err));
	return err;

    }; /* Exec::Cmd::cd() */



#undef CMD_NM
#define CMD_NM "ls"


    /// print a list of files in the specified directory
    esp_err_t Cmd::ls(std::string pattern)
    {
	ESP_LOGI(CMD_TAG_PRFX, "Listing the %s", pattern.c_str());

	    //esp_log_level_set(CMD_TAG_PRFX, ESP_LOG_DEBUG);	/* for debug purposes */
	ESP_LOGD(CMD_TAG_PRFX, "%s: pattern is             : \"%s\"", __func__, pattern.c_str());

	if (!artificial_cwd.valid(pattern))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Listing is failed - pattern \"%s\" is invalid or impossible", __func__, pattern.c_str());
	    return ESP_ERR_INVALID_ARG;
	}; /* if !artificial_cwd.valid(pattern) */
	pattern = artificial_cwd / pattern;
	ESP_LOGI(CMD_TAG_PRFX, "(real path is: %s)", pattern.c_str());
	ESP_LOGD(CMD_TAG_PRFX, "%s: processed inner pattern: \"%s\"", __func__, pattern.c_str());

	cout << "----------------" << endl;

	if (!CWD::last::exist())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Listing is failed - pattern \"%s\" is not exist", __func__, pattern.c_str());
	    return ESP_ERR_NOT_FOUND;
	}; /* if CWD::last::exist() */
	if (!CWD::last::is_dir())
	{
	    if (pattern.back() == '/' || pattern.back() == '.')
	    {
		ESP_LOGE(CMD_TAG_PRFX, "%s: %s -\n\t\t\t\t%s; pattern \"%s\" is invalid", __func__,
			"Name of the file or other similar entity that is not a directory",
			"cannot end with a slash or a dot", pattern.c_str());
		return ESP_ERR_INVALID_ARG;
	    }; /* if pattern.back() == '/' || pattern.back() == '.' */

	    ESP_LOGI(__func__, "\n%s %s, file size %ld bytes", pattern.c_str(), CWD::last::type(), CWD::last::size());
	    cout << "----------------" << endl;
	    return ESP_OK;
	}; /* if CWD::last::is_dir() */

    	    int entry_cnt = 0;
    	errno = 0;	// clear any possible errors
	for (auto &entry : fs::Directory(pattern))
	{
	    ++entry_cnt;
	    artificial_cwd / (pattern + CWD::refine(entry.d_name));
	    cout << aso::format("  %-42s\t%s") % entry.d_name % CWD::last::type() << endl;
	}; /* for auto &entry = dir.begin(); entry != dir.end(); dir++ */

	if (errno != 0)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Error occured during reading of the directory <%s>, %s", __func__, pattern.c_str(), strerror(errno));
	    return ESP_FAIL;
	}; /* if errno != 0 */
	if (entry_cnt == 0)
	    ESP_LOGW(__func__, "Files or directory not found, directory is empty.");

	cout << "----------------" << endl;
	cout << aso::format("Total found %d files", entry_cnt) << endl;

	cout << endl;
	return ESP_OK;
    }; /* Exec::Cmd::ls() */


//#define __NOT_OVERWRITE__	// Deny overwrite cp & move destination files

#undef CMD_NM
#define CMD_NM "cp"
#define __CP_OVERWRITE_FILE__

    /// copy files according a pattern
    esp_err_t Cmd::cp(std::string src, std::string dest)
    {
	dest = astr::trim(dest);    // trim the destination file name
	if (dest.empty())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: too few arguments: invoke command \"%s\" with less than one parameter.\n%s", __func__, __func__,
		    "Don't know where to copy.");
	    ret = ESP_ERR_INVALID_ARG;
	    return ret;
	}; /* if dest.empty() */

	src = astr::trim(src);
	if (src.empty())    //
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: too few arguments: invoke command \"%s\" without src parameter.\n%s", __func__, __func__,
		    "Don't know what to copy.");
	    ret = ESP_ERR_INVALID_ARG;
	    return ret;
	}; /* if src.empty() */

	ESP_LOGI(CMD_TAG_PRFX, "%s: copy source file \"%s\".",	__func__, src.c_str());
	if (!artificial_cwd.valid(src))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: the source file name is invalid", __func__);
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if !artificial_cwd.valid(src) */

	src = artificial_cwd / src;

	// Check if source file is not exist
	if (!CWD::last::exist())
	{
	    // Source file must be exist
	    ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" is not exist - copyng a non-existent file is not possible.\n%s",
		    __func__, src.c_str(), esp_err_to_name(ESP_ERR_NOT_FOUND));

	    ret = ESP_ERR_NOT_FOUND;
	    return ret;
	}; /* if !CWD::last::exist() */
	if (CWD::last::is_dir())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: copyng directories is unsupported.\n%s",
		    __func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	    ret = ESP_ERR_NOT_FOUND;
	    return ret;
	}; /* if if CWD::last::is_dir() */

	ESP_LOGI(CMD_TAG_PRFX, "%s: to destination \"%s\".", __func__, dest.c_str());
	if (!artificial_cwd.valid(dest))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: destination file name \"%s\" is invalid\n%s", __func__, dest.c_str(), esp_err_to_name(ESP_ERR_NOT_FOUND));
	    ret = ESP_ERR_NOT_FOUND;
	    return ret;
	}; /* if !artificial_cwd.valid(dest) */


	dest = artificial_cwd / dest;

	// Check if destination file is exist
	if (CWD::last::exist())
	{
	    // Destination file is exist
	    ESP_LOGI(CMD_TAG_PRFX, "%s: file \"%s\" is exist - copy is write to an existent file or directory.",
		    __func__, dest.c_str());
	    // if destination - exist path, is a directory
	    if (CWD::last::is_dir())
		dest = dest + "/" + basename(src.c_str());
	}; /* CWD::last::exist() */

	// Re-check final version of the
	// destination filename, that may be exist:
	artificial_cwd.compose(dest);	// Check the dest file
//	artificial_cwd / dest;	// Check the dest file
	if (CWD::last::exist())
	{
	    // the final name of the target file
	    // must not be a existing directory name
	    if (CWD::last::is_dir())
	    {
		ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist \"%s\" directory by the destination file is denied; aborting.",
			__func__, dest.c_str());
		ret = ESP_ERR_NOT_SUPPORTED;
		return ret;
	    } /* if CWD::last::is_dir() */

#if !defined(__NOT_OVERWRITE__) && defined(__CP_OVERWRITE_FILE__)
	    ESP_LOGW(CMD_TAG_PRFX, "%s: overwrite an existing file \"%s\".", __func__, dest.c_str());
#else
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite the existent file \"%s\" is denied; aborting.", __func__, dest.c_str());
	    ret = ESP_ERR_NOT_SUPPORTED;
	    return ret;
#endif	// __CP_OVER_EXIST_FILE__
	}; /* if stat(dest, &st) == 0 */

	if (src == dest)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: source & destination file name are same: \"%s\";\n\t\t\t copying file to iself is unsupported",
		    __func__, dest.c_str());
	    ret = ESP_ERR_NOT_SUPPORTED;
	    return ret;
	}; /* if strcmp(src, dest) == 0 */

	// destination file - OK, it's not exist or is may be overwrited
	ESP_LOGI(CMD_TAG_PRFX ":" CMD_NM, "copy file %s to %s", src.c_str(), dest.c_str());

	    std::ifstream ifs(src);

	if(!ifs)
	{
	    // Error opening the source file
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Any Error opening the file \"%s\" is not exist - copyng from a not opened file is impossible.\n",
		    __func__, src.c_str());
	    return (ret = ESP_FAIL);
	}; /* if!ifs */

	    std::ofstream ofs(dest);

	if (!ofs)
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Failed creating file %s, aborting ", dest.c_str());
	    return (ret = ESP_ERR_NOT_SUPPORTED);
	}; /* if !ofs */


	    constexpr size_t CP_BUFSIZE = 512;
	    std::vector<char> buf(CP_BUFSIZE);
//	    char buf[CP_BUFSIZE];
//	    std::streamsize cnt;

	while (!ifs.eof())
	{
	    ifs.read(buf.data(), CP_BUFSIZE);
//	    cnt = ifs.gcount();
	    ofs.write(buf.data(), /*cnt*/ifs.gcount());
	}; /* while !ifs.eof() */
	ofs.flush();

	if (!ifs.good())
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "I/O Error during reading from the file [%s] to output"/*, %s"*/, src.c_str()/*, strerror(errno)*/);
	    return (ret = ESP_FAIL);
	}; /* if errno */
	if (!ofs.good())
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "I/O Error during writing to file [%s]"/*, %s"*/, dest.c_str()/*, strerror(errno)*/);
	    return (ret = ESP_FAIL);
	}; /* if errno */

	return (ret = ESP_OK);

    }; /* Exec::Cmd::cp() */



#define __MV_OVERWRITE_FILE__

#undef CMD_NM
#define CMD_NM "mv"

    /// move files according a pattern
    esp_err_t Cmd::mv(std::string src, std::string dest)
    {

	src = astr::trim(src);
	if (src.empty())
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "too few arguments: invoke command \"%s\" with one parameters.\n%s", CMD_NM,
		    "Don't know what to move?");
	    return (ret = ESP_ERR_INVALID_ARG);
	}; /* if src.empty() */

	dest = astr::trim(dest);
	if (dest.empty())
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "too few arguments: invoke command \"%s\" without parameters.\n%s", CMD_NM,
		    "Don't know where to move?");
	    return (ret = ESP_ERR_INVALID_ARG);
	}; /* if dest_raw.empty() */

	cout << aso::format("Move file \"%s\" to \"%s\"") %src %dest << endl;

	if (!artificial_cwd.valid(src))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: the souce file name \"%s\" is invalid", __func__, src.c_str());
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if !artificial_cwd.valid(src) */
	if (!artificial_cwd.valid(dest))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: the destination file name \"%s\" is invalid", __func__, dest.c_str());
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if !artificial_cwd.valid(dest) */


	src = artificial_cwd / src;

	// Check if source file is not exist
	if (!CWD::last::exist())
	{
	    // Source file must be exist
	    ESP_LOGE(CMD_TAG_PRFX, "%s: file [%s] is not exist - renaming a non-existent file is not possible.\n%s",
		    __func__, src.c_str(), esp_err_to_name(ESP_ERR_NOT_FOUND));
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if stat(src.c_str(), &st_src) != 0 */

	dest = artificial_cwd / dest;

	cout << aso::format("\t(%s) to (%s)") %src %dest << endl;

	if (CWD::last::exist())
	{
	    // Target file exist
	    ESP_LOGW(CMD_TAG_PRFX, "%s: target file name \"%s\" is exist",
		    __func__, dest.c_str());
	    // if destination is existing directory
	    if (CWD::last::is_dir())
	    {
		ESP_LOGD(CMD_TAG_PRFX, "%s: destination file is exist directory,\n\t\t\tbasename of src is: %s ", __func__,
			basename(src.c_str()));
		ESP_LOGD(CMD_TAG_PRFX, "%s: adding trailing slash to a destination file: %s", __func__, (dest + '/').c_str());
		dest = dest + '/' + basename(src.c_str());
		ESP_LOGD(CMD_TAG_PRFX, "%s: adding src basename to a destination file: %s", __func__, dest.c_str());
	    } /* if CWD::last::is_dir() */
	} /* if CWD::last::exist() */

	// Re-check the modified version of the
	// destination filename, that may be exist:
	artificial_cwd.compose(dest);
	if (CWD::last::exist())
	{
	    // the final name of the target file
	    // must not be a existing directory name
	    if (CWD::last::is_dir())
	    {
		ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist directory \"%s\" by the destination file from the %s is not allowed; aborting.",
			__func__, dest.c_str(), src.c_str());
		return (ret = ESP_ERR_NOT_SUPPORTED);
	    } /* if CWD::last::is_dir() */

	    // if source - is dir, but destination - ordinary file
	    artificial_cwd.compose(src);
	    if (CWD::last::is_dir())
	    {
		ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist file \"%s\" by renaming the source directory %s to it - is not allowed; aborting.",
			__func__, dest.c_str(), src.c_str());
		return (ret = ESP_ERR_NOT_SUPPORTED);
	    }; /* CWD::last::is_dir() */

#if !defined(__NOT_OVERWRITE__) && defined(__CP_OVERWRITE_FILE__)
	    ESP_LOGW(CMD_TAG_PRFX, "%s: overwrite an existing file \"%s\".", __func__, dest.c_str());
#else
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite the existent file \"%s\" is denied; aborting.",
		    __func__, dest.c_str());
	    return (ret = ESP_ERR_NOT_SUPPORTED);
#endif	// __CP_OVER_EXIST_FILE__
	}; /*     if stat(dest.c_str(), &st_dest) == 0 */

	// Names of the moving/renaming file
	ESP_LOGI(CMD_TAG_PRFX, "%s: Moving/renaming file [%s] to [%s]", __func__, src.c_str(), dest.c_str());
	// check the source and destination file are same
	if (src == dest)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: source & destination file name are same: \"%s\";\n\t\t\t copying file to iself is unsupported",
		    __func__, dest.c_str());
	    return (ret = ESP_ERR_NOT_SUPPORTED);
	}; /* if src == dest */


	if (rename(src.c_str(), dest.c_str()) != 0)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Error %d: %s", __func__, errno, strerror(errno));
	    return (ret = ESP_FAIL);
	}; /* if rename(src.c_str(), dest.c_str()) != 0 */
	return (ret = ESP_OK);

    }; /* Exec::Cmd::mv() */



#undef CMD_NM
#define CMD_NM "rm"

    /// remove files according a pattern
    esp_err_t Cmd::rm(std::string pattern)
    {
	pattern = astr::trim(pattern);
	if (pattern.empty())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, __func__,
		    "Missing filename to remove.");
	    return (ret = ESP_ERR_INVALID_ARG);
	}; /* if pattern.empty() */

	if (!artificial_cwd.valid(pattern))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: pattern \"%s\" is invalid", __func__, pattern.c_str());
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if !artificial_cwd.valid(pattern) */

	ESP_LOGI(CMD_TAG_PRFX, "%s: delete file \"%s\"", __func__,  pattern.c_str());

	pattern = artificial_cwd / pattern;

	ESP_LOGI(CMD_TAG_PRFX, "full name is: %s", pattern.c_str());

	// Check if destination file exists before deleting
	if (!CWD::last::exist())
	{
	    // deleting a non-existent file is not possible
	    ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" is not exist - deleting a non-existent file is not possible.\n%s",
		    __func__, pattern.c_str(), esp_err_to_name(ESP_ERR_NOT_FOUND));
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if CWD::last::exist() */
	if (CWD::last::is_dir())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: deleting directories unsupported.\n%s",
		    __func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	    return (ret = ESP_ERR_NOT_SUPPORTED);
	}; /* if CWD::last::is_dir() */
	errno = 0;
	unlink(pattern.c_str());
	if (errno)
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when deleting \"%s\": %s", __func__, pattern.c_str(), strerror(errno));
	    return (ret = ESP_FAIL);
	}; /* if errno */

	return ESP_OK;

    }; /* Exec::Cmd::rm() */



#undef CMD_NM
#define CMD_NM "cat"

    /// type file contents
    esp_err_t Cmd::cat(std::string fname)
    {
	fname = astr::trim(fname);
	if (empty(fname))
	{
	    cout << endl
		<< "*** Printing contents of the file <XXXX fname>. ***" << endl
		<< endl;
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "invoke command \"%s\" without parameters.\n%s", CMD_NM,
		    "Missing filename for print to output.");

	    cout << "*** End of printing file XXXX. ** ******************" << endl;
	    return (ret = ESP_ERR_INVALID_ARG);
	}; /* if empty(fname) */ /* if fname == NULL || strcmp(fname, "") */

	if (!artificial_cwd.valid(fname))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: pattern \"%s\" is invalid", __func__, fname.c_str());
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if !artificial_cwd.valid(fname) */

	cout << endl
	    << aso::format("*** Printing contents of the file <%s> ") % fname;

	fname = artificial_cwd / fname;

	cout << aso::format("(realname '%s'). ***") % fname << endl
	    << endl;

	// Check if destination file exists before typing
	if (!CWD::last::exist())
	{
	    // typing a non-exist file is not possible
	    ESP_LOGE(CMD_TAG_PRFX, "%s: \"%s\" file does not exist - printing of the missing file is not possible.\n%s",
		    __func__, fname.c_str(), esp_err_to_name(ESP_ERR_NOT_FOUND));
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if !CWD::last::exist() */

	if (CWD::last::is_dir())
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: Typing directories unsupported, use the 'ls' command instead.\n%s",
		    __func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	    return (ret = ESP_ERR_NOT_SUPPORTED);
	}; /* if CWD::last::is_dir() */

	    ifstream text(fname, ios::in | ios::binary);

	if (!text)
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error opening file <%s>, %s", fname.c_str(), strerror(errno));
	    return (ret = ESP_FAIL);
	}; /* if !FILE */

	    constexpr size_t CAT_BUFSIZE = 512;
//	    char buf[CP_BUFSIZE];
	    std::vector<char> buf(CAT_BUFSIZE);
//	    std::streamsize cnt;

	while (!text.eof())
	{
	    text.read(buf.data(), CAT_BUFSIZE);
//	    cnt = text.gcount();
	    cout.write(buf.data(), /*cnt*/text.gcount());
	}; /* while !text.eof() */

	if (text.bad())
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "I/O Error during type the file [%s] to output, %s", fname.c_str(), strerror(errno));
	    return (ret = ESP_FAIL);
	}; /* if errno */

	cout << endl
	    << "*** End of printing file " << fname << ". **************" << endl
	    << endl;

	return (ret = ESP_OK);
    }; /* Exec::Cmd::cat() */



#undef CMD_NM
#define CMD_NM "type"

    /// type text from keyboard to screen
    esp_err_t Cmd::type()
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
//	    if (c == '\n')
//		cout << "<LF>" << endl;
//	    if (c == '\r')
//		cout << "<CR>" << endl;
	} while (c != prevc || c != '\n');

	cout << endl << endl
	     << "**** End of typing the text on keyboard. *****" << endl
	     << endl;
	return ESP_OK;
    }; /* Exec::Cmd::type() */


    /// type text from keyboard to file and to screen
    esp_err_t Cmd::type(std::string fname, const size_t sector_size)
    {
	fname = astr::trim(fname);
	if (!artificial_cwd.valid(fname))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: pattern \"%s\" is invalid", __func__, fname.c_str());
	    return (ret = ESP_ERR_NOT_FOUND);
	}; /* if !artificial_cwd.valid(fname) */

	    ios::openmode mode = ios::out | ios::binary;
	    ofstream storage;

	fname = artificial_cwd / fname;
	if (CWD::last::exist())	// if file - exist
	{	// Status - file "fname" is exist

		char c;

	    // fname exists, check that is a regular file
	    if (!CWD::last::is_file())
	    {
		ESP_LOGE("console::type exist chechk", "Error: path %s exist, and is not a file, but a %s.\nOperation is not permitted.",
			fname.c_str(), CWD::last::type());
		return (ret = ESP_ERR_NOT_SUPPORTED);
	    }; /* if !CWD::last::is_file() */

	    cout << aso::format("File %s is exist.\nDo you want use this file? [yes[append]/over(write)/No]: ") % fname;
	    cin >> noskipws >> c;
	    cout << c;

	    if (c == '\n')
		cout << "<LF>";
	    cout << endl;

	    switch (tolower(c))
	    {
	    case 'a':
	    case 'y':
		ESP_LOGI(CMD_TAG_PRFX CMD_NM, "OK, open the file %s to add.", fname.c_str());
		cout << aso::format("File %s is opened for add+write.") % fname << endl;
		mode |= ios::app;
		storage.open(fname, mode);
		break;

	    case 'o':
	    case 'w':
		ESP_LOGW(CMD_TAG_PRFX CMD_NM, "OK, open the file %s to owerwrite.", fname.c_str());
		cout << aso::format("File %s is opened to truncate+write (overwrite).") % fname << endl;
		mode |= ios::trunc;
		storage.open(fname, mode);
		break;

	    case '\n':
		cout << "Enter char '\\n'" << endl; // @suppress("No break at end of case")
		[[fallthrough]];
	    case 'n':
		ESP_LOGW(CMD_TAG_PRFX ":" CMD_NM " <filename>", "User cancel opening the file %s.", fname.c_str());
		return (ret = ESP_ERR_NOT_FOUND);
		break;

	    default:
		ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Error: Unrecognized input, received char value: \"%c\" [%d]", c, (int)c);
		return (ret = ESP_ERR_INVALID_ARG);
	    }; /* switch tolower(c) */
	    ;
	} /* if CWD::last::exist() */
	else
	{
	    ESP_LOGI(CMD_TAG_PRFX CMD_NM, "OK, file \"%s\" does not exist, opening this file.", fname.c_str());
	    cout << aso::format("Open file %s for the write") % fname << endl;
	    storage.open(fname, mode);
	}; /* else if CWD::last::exist() */

	if (!storage)
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Any error occured when opening the file %s: %s.", fname.c_str(), strerror(errno));
	    return ESP_ERR_NOT_FOUND;
	}; /* if storage == NULL */

	cout << endl
	     << aso::format("**** Type the text on keyboard to screen and file [%s]. ****") % fname  << endl
	     << "Press <Enter> twice for exit..." << endl
	     << endl;

	    char c = '\0', prevc;
	do {
	    prevc = c;
	    cin >> noskipws >> c;
	    cout << c;
	    storage << c;
	} while (c != prevc || c != '\n');

	cout << endl << endl
	     << "**** End of typing the text on keyboard. *****" << endl
	     << endl;


	cout << aso::format("Flush&write cache buffer of the file %s.") % fname << endl;
	storage << flush;
	cout << aso::format("Close the file %s.") % fname << endl;
	storage.close();
	if (!storage.good())
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Any error occured when closing the file %s: %s.", fname.c_str(), strerror(errno));
	    return ESP_FAIL;
	}; /* if errno */

	cout << endl
	     << aso::format("**** End of typing the text on keyboard for the screen and the file %s. ****") % fname << endl
	     << endl;
	return ESP_OK;

    }; /* Cmd::type <file> */

#undef CMD_TAG_PRFX

    const char* Cmd::TAG = "SD/MMC Exec Cmd service";

}; //--[ namespace Exec ]----------------------------------------------------------------------------------------------


//--[ sdcard_ctrl.cpp ]----------------------------------------------------------------------------
