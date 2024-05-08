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
#include "sdmmc_cmd.h"
#include <driver/sdmmc_host.h>

#include "cwd_emulate"
#include "sdcard_io"
#include "fs_ctrl"


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

namespace Exec	//-----------------------------------------------------------------------------------------------------
{

//--[ inner instance of the cwd_emulation ]----------------------------------------------------------------------------
    	char cwd_path_buff[PATH_MAX] = "";	// simulation current dir at this device
	fs::CWD_emulating fake_cwd(cwd_path_buff, sizeof(cwd_path_buff));


//    static const char *TAG = "SD/MMC service";

//--[ class Server ]------------------------------------------------------------------------------------------------



const char* const Server::MOUNT_POINT_Default = SD_MOUNT_POINT;


#undef CMD_TAG_PRFX
#define CMD_TAG_PRFX "SD/MMC CMD server:"


    /// Mount default SD-card slot onto path "mountpoint", default mountpoint is MOUNT_POINT_Default
    esp_err_t Server::mount(SDMMC::Device& device, SDMMC::Card& card, const char mountpoint[]) // @suppress("Type cannot be resolved") // @suppress("Member declaration not found")
    {
	if (isdigit(mountpoint[0]))
	    return mount(device, card, atoi(mountpoint)); // @suppress("Invalid arguments")

	ret = device.mount(card, mountpoint); // @suppress("Method cannot be resolved")
	if (ret == ESP_OK)
	{
#ifdef CONFIG_AUTO_CHDIR_BEHIND_MOUNTING
//	    change_currdir(mountpath());
	    fake_cwd.change(mountpoint);
	    ESP_LOGI(TAG, "Current directory autochanged to: %s", fake_cwd.current());
#else
//	    change_currdir("/");
	    fake_cwd.get(fake_cwd_path, sizeof(fake_cwd_path));	// set fake_cwd according system pwd (through get_cwd())
	    ESP_LOGI(TAG, "Current directory set to: %s,  according system pwd", fake_cwd.current());
#endif
	}; /* if ret == ESP_OK */
	return ret;
    }; /* Server::mount */


    /// Mount SD-card slot "slot_no" onto specified mount path, default mountpoint is MOUNT_POINT_Default
    esp_err_t Server::mount(SDMMC::Device& device, SDMMC::Card& card, int slot_no, const char mountpoint[]) // @suppress("Member declaration not found") // @suppress("Type cannot be resolved")
    {
	device.slot_no(slot_no); // @suppress("Method cannot be resolved")
	return device.mount(card, mountpoint); // @suppress("Method cannot be resolved")
    }; /* Server::mount */


    //------------------------------------------------------------------------------------------
    //    // All done, unmount partition and disable SDMMC peripheral
    //    esp_vfs_fat_sdcard_unmount(mount_point, card);
    //    ESP_LOGI(TAG, "Card unmounted");
    //------------------------------------------------------------------------------------------

    // Unmount SD-card, that mounted onto "mountpath"
    esp_err_t Server::unmount(SDMMC::Device& device/*const char mountpath[]*/) // @suppress("Type cannot be resolved") // @suppress("Member declaration not found")
    {
	ret = device.unmount(); // @suppress("Method cannot be resolved")
	//ESP_LOGI(TAG, "Card unmounted");
	if (ret != ESP_OK)
	    cout << TAG << ": "  << "Unmounting Error: " << ret
		<< ", " << esp_err_to_name(ret) << endl;
	else
	{
	    cout << TAG << ": " << "Card unmounted" << endl;
//	    fake_cwd_path[0] = '\0';	// set fake cwd path to: ""
	}

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


// print current directory name
esp_err_t Server::pwd(SDMMC::Device& device)
{
//#ifdef __PURE_C__
#if __cplusplus < 201703L

	const char* buf = device.get_cwd();

    if (!buf)
	return errno;
    cout << endl
	<< "PWD is: \"" << buf << '"' << endl
	<< endl;

    return ESP_OK;
#else
	const char* buf = fake_cwd.get();

if (!buf)
	return errno;
cout << endl
	<< "PWD is: \"" << buf << '"' << endl
	<< endl;

return ESP_OK;
#endif	//
}; /* Server::pwd */


// if the basename (the last part of the path) - has the characteristics
// of a directory name, and a dirname (the path prefix) -
// is an existing file, not a directory, or any other impossible variants
// of the full file/path name
bool Server::valid_path(const char path[])
{

	struct stat st;
	char *base = basename(path);	// get a filename of a path

    ESP_LOGD(__PRETTY_FUNCTION__, "basename of the path is: \"%s\"", base);
    ESP_LOGD(__PRETTY_FUNCTION__, "full path is: \"%s\"", path);
    ESP_LOGD(__PRETTY_FUNCTION__, "dirname path is: \"%.*s\"", base - path, path);

    // if path is empty
    if (empty(path))
	return true;	// ESP_LOGD("Device::valid_path", "path is empty, always valid");

    // if path - only base, not a dir
    if (strlen(path) == 1)
	return true;	// ESP_LOGD("Device::valid_path", "len of the path - is 1, always valid");

    // if dirname - empty or one symbol length (it can only be the slash)
    if ((base - path) < 2)
    {
	if (strcmp(path, "/..") == 0)	// if path == '/..' - it's invalid
	    return false;
	return true;	// ESP_LOGD("Device::valid_path", "Len of dirname is 1 or 0, then path is valid");
    }; /* if (base - path) < 2 */

    // if base is empty
    if (!empty(base))
	base--;	// set base to a last slash in the path
    else
    {
	if (stat(fake_cwd.compose(path), &st) == 0)
	    if (!S_ISDIR(st.st_mode))	// ESP_LOGD("Device::valid_path", "###!!! the path basename - is empty, test the path \"%s\" (real path is %s) exist and a directory... ###", path, fake_cwd.get_current());
		return false;	// the path is invalid (inconsist) // ESP_LOGE("Device::valid_path", "Path \"%s\" (real path %s) is a file, but marked as a directory, it's invalid!!!", path, fake_cwd.get_current());
	ESP_LOGD("Device::valid_path", "###!!! test dirname \"%s\" (real path is %s) preliminary is OK, seek to begin of last dir manually for continue test... ###", path, fake_cwd.current());
	for (base -= 2; base > path; base--)
	{
	    ESP_LOGD("Device::valid_path", "=== base[0] is \"%c\" ===", base[0]);
	    if (*base == '/')
		break;
	}; /* for base--; base > path; base-- */
    }; /* if empty(base) */

#define sign_place 0x2	// with of the place for the sign
#define point_sign 0x1	// mark a point symbol in a string
#define alpha_sign 0x2	// mark a non-point or a non-slash symbol in a string
#define initial_ctrl (0x3 << 4*sign_place)	// mark for the initial pass of the control of the path validity
#define alpha_present_mask (alpha_sign | (alpha_sign << 1*sign_place) | (alpha_sign << 2*sign_place))
#define three_point_mark (point_sign | (point_sign << 1*sign_place) | (point_sign << 2*sign_place))

	unsigned int ctrl_cnt = initial_ctrl;	// marked the firs pass of the control loop
	unsigned int idx_ctrl = 0;
    // scan the dirname of the path for found '/.' or '/..' sequence
    for (char* scan = base; scan >= path; scan--)
    {
	ESP_LOGD("Device::valid_path", "current char from the path is: '%c', ctrl_cnt is %2X", *scan, ctrl_cnt);

	switch (scan[0])
	{
	// solution point
	case '/':

	    idx_ctrl = 0;	// reset the idx_ctrl
	    ESP_LOGD("Device::valid_path", "###### Solution point: current path char ######");
	    switch (ctrl_cnt)
	    {
	    // double slash - prev symbol is slash
	    case 0:
		ESP_LOGD("Device::valid_path", "**** double slash and more - is not valid sequence in the path name ****");
		return false;

	    case three_point_mark:
		// if more then 3 point sequence in substring
		ESP_LOGD("Device::valid_path", "3 point or more sequence is present in current substring - nothing to do, continue");
		break;

	    case initial_ctrl:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
		ESP_LOGD(__PRETTY_FUNCTION__, "++++++ The first pass of the control loop ++++++");
#pragma GCC diagnostic pop

	    default:
		// if non point sign is present in tested substring
		if (ctrl_cnt & alpha_present_mask)
		{
		    ESP_LOGD("Device::valid_path", "alpha or other then point or slash symbol is present in current processing substring - test subpath for exist, continue");
		    ctrl_cnt = 0;
		    continue;
		}; /* if ctrl_cnt & alpha_present_mask */
		ESP_LOGD(__PRETTY_FUNCTION__, "====== One or two point sequence in the current meaning substring, ctrl_cnt is %2X, test current subpath for existing ======", ctrl_cnt);
		ESP_LOGD(__PRETTY_FUNCTION__, "### Testing the current substring \"%s\" for existing ###", fake_cwd.compose(path, scan - path));
		if ((stat(fake_cwd.compose(path, scan - path), &st) == 0)? !S_ISDIR(st.st_mode): (strcmp(fake_cwd.current(), "/") != 0))
		    return false;
	    }; /* switch ctrl_cnt */
	    ctrl_cnt = 0;
	    break;

	// point symbol handling
	case '.':
	// all other symbols
	default:

	    if (idx_ctrl < 3)
		ctrl_cnt |= (((scan[0] == '.')? point_sign: alpha_sign) << idx_ctrl++ * sign_place);	// ESP_LOGD("Device::valid_path", "%d symbol of the processing substring, symbol is \"%c\"", idx_ctrl, scan[0]);
	}; /* switch scan[0] */
    }; /* for char* scan = base; scan >= path; scan-- */

    return true;
}; /* Server::valid_path() */



#define CMD_NM "mkdir"
// create a new directory
esp_err_t Server::mkdir(SDMMC::Device& device, const char dirname[])
{
//    if (!device.valid_path(dirname))
    if (!fake_cwd.valid(dirname))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: the new directory name \"%s\" is invalid", __func__, dirname);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */

    if (empty(dirname))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, CMD_NM,
		"This command required the creating directory name.");
	return ESP_ERR_INVALID_ARG;
    }; /* if dirname == NULL || strcmp(dirname, "") */
//#ifdef __PURE_C__
#if __cplusplus < 201703L

	struct stat statbuf;
	char *path = device.get_cwd(dirname);

    ESP_LOGI(CMD_TAG_PRFX, "%s: Create directory with name \"%s\", real path is %s", __func__, dirname, path);

    if (stat(path, &statbuf) == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Invalid argument - requested path \"%s\" is exist; denied create duplication name\n", __func__, path);
	return ESP_ERR_INVALID_ARG;
    }; /* if stat(tmpstr, &statbuf) == -1 */
    errno = 0;
    ::mkdir(path, /*0777*/ S_IRWXU | S_IRWXG | S_IRWXO);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Error creating directory \"%s\": %s", __func__, dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if (errno) */
    return ESP_OK;

#else

	struct stat statbuf;
	char *path = fake_cwd.compose(dirname);

    ESP_LOGI(CMD_TAG_PRFX, "%s: Create directory with name \"%s\", real path is %s", __func__, dirname, path);

    if (stat(path, &statbuf) == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Invalid argument - requested path \"%s\" is exist; denied create duplication name\n", __func__, path);
	return ESP_ERR_INVALID_ARG;
    }; /* if stat(tmpstr, &statbuf) == -1 */
    errno = 0;
    ::mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Error creating directory \"%s\": %s", __func__, dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if (errno) */
    return ESP_OK;

#endif	// __cplusplus < 201703L
}; /* Server::mkdir */

#undef CMD_NM
#define CMD_NM "rmdir"
// create a new directory
esp_err_t Server::rmdir(SDMMC::Device& device, const char dirname[])
{
//    if (!device.valid_path(dirname))
    if (!fake_cwd.valid(dirname))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: the directory name \"%s\" is invalid", __func__, dirname);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */

    if (empty(dirname))
    {
	    ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, CMD_NM,
		     "This command required the name of the deleting directory.");
	    return ESP_ERR_INVALID_ARG;
    }; /* if dirname == NULL || strcmp(dirname, "") */
#if __cplusplus < 201703L

	struct stat st;
	char *path = device.get_cwd(dirname);

    ESP_LOGI(CMD_TAG_PRFX, "%s: Delete directory <%s>, real path is %s", __func__, dirname, path);

    // Check if destination directory or file exists before deleting
    if (stat(path, &st) != 0)
    {
        // deleting a non-exist directory is not possible
	ESP_LOGE(CMD_TAG_PRFX, "%s: Directory \"%s\" is not exist - deleting a non-existent catalogue is not possible.\n%s", __func__, dirname, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(file_foo, &st) != 0 */
    if (!S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: The %s command delete directories, not the files.\n%s", __func__, CMD_NM, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_INVALID_ARG;
    }; /* if (S_ISDIR(st.st_mode)) */

	DIR *dir = opendir(path);	// Directory descriptor

    errno = 0;	// clear any possible errors

	struct dirent *entry = readdir(dir);

    closedir(dir);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when closing directory \"%s\": %s", __func__, dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */
    if (entry)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Directory \"%s\" is not empty, deletung non-emty directories "
		"is not supported.", __func__, dirname);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (entry) */

    errno = 0;
    //cout << aso::format("[[[ unlink the path [%s] ]]]") % path << endl;
    unlink(path);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when deleting \"%s\": %s", __func__, dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */

    return ESP_OK;

#else
	struct stat st;
//	char *path = device.get_cwd(dirname);
	char *path = fake_cwd.compose(dirname);

    ESP_LOGI(CMD_TAG_PRFX, "%s: Delete directory <%s>, real path is %s", __func__, dirname, path);

    // Check if destination directory or file exists before deleting
    if (stat(path, &st) != 0)
    {
	// deleting a non-exist directory is not possible
	ESP_LOGE(CMD_TAG_PRFX, "%s: Directory \"%s\" is not exist - deleting a non-existent catalogue is not possible.\n%s", __func__, dirname, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(file_foo, &st) != 0 */
    if (!S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: The %s command delete directories, not the files.\n%s", __func__, CMD_NM, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_INVALID_ARG;
    }; /* if (S_ISDIR(st.st_mode)) */

	DIR *dir = opendir(path);	// Directory descriptor

    errno = 0;	// clear any possible errors

    struct dirent *entry = readdir(dir);

    closedir(dir);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when closing directory \"%s\": %s", __func__, dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */
    if (entry)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Directory \"%s\" is not empty, deletung non-emty directories "
		"is not supported.", __func__, dirname);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (entry) */

    errno = 0;
    //cout << aso::format("[[[ unlink the path [%s] ]]]") % path << endl;
    unlink(path);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when deleting \"%s\": %s", __func__, dirname, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */

    return ESP_OK;

#endif // __cplusplus < 201703L

}; /* Server::rmdir */



#undef CMD_NM
#define CMD_NM "cd"

// change a current directory
esp_err_t Server::cd(SDMMC::Device& device, const char dirname[])
{
	esp_err_t err;

    if (!fake_cwd.valid(dirname))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: the directory name \"%s\" is invalid", __func__, dirname);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */

#if __cplusplus < 201703L
//    if (dirname != nullptr && dirname[0] != '\0')
    if (!empty(dirname))
	ESP_LOGI(CMD_TAG_PRFX, "%s: Change current dir to %s", __func__, dirname);
    else if (device.card != nullptr)
	    ESP_LOGI(CMD_TAG_PRFX, "%s: Not specified directory for jump to, change current dir to %s, [mountpoint].", __func__, device.mountpath());
	else
	{
	    ESP_LOGW(CMD_TAG_PRFX, "%s: Card is not mounted, mountpoint is not valid, nothing to do", __func__);
	    return ESP_ERR_NOT_SUPPORTED;
	}; /* else if device.card != nullptr */
    // change cwd dir
    //chdir(dirname);
    err = device.change_currdir(dirname);
    if (err != 0)
    //{
	ESP_LOGE(CMD_TAG_PRFX, "%s: fail change directory to %s\n%s", __func__, dirname, /*strerror(errno)*/ esp_err_to_name(err));
	//perror(CMD_TAG_PRFX CMD_NM);
//	return ESP_FAIL;
    //}; /* if errno != 0 */
    return err;
//    return ESP_OK;
#else
    if (!empty(dirname))
	ESP_LOGI(CMD_TAG_PRFX, "%s: Change current dir to %s", __func__, dirname);
    else if (device.card != nullptr)
	ESP_LOGI(CMD_TAG_PRFX, "%s: Not specified directory for jump to, change current dir to %s, [mountpoint].", __func__, device.mountpath());
	else
	{
    	    ESP_LOGW(CMD_TAG_PRFX, "%s: Card is not mounted, mountpoint is not valid, nothing to do", __func__);
    	    return ESP_ERR_NOT_SUPPORTED;
	}; /* else if device.card != nullptr */
    // change cwd dir: chdir(dirname);
//        err = device.change_currdir(dirname);
//	{ return mounted()? fake_cwd.change_dir( (path == nullptr || path[0] == '\0')? mountpath(): path): ESP_FAIL; }
    err = device.mounted()? fake_cwd.change( (dirname == nullptr || dirname[0] == '\0')? device.mountpath(): dirname): ESP_FAIL;

    if (err != 0)
    	ESP_LOGE(CMD_TAG_PRFX, "%s: fail change directory to %s\n%s", __func__, dirname, /*strerror(errno)*/ esp_err_to_name(err));
    return err;

#endif	// __cplusplus < 201703L
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
esp_err_t Server::ls(SDMMC::Device& device, const char pattern[])
{
    ESP_LOGD(CMD_TAG_PRFX, "%s: pattern is             : \"%s\"", __func__, pattern);
    ESP_LOGD(CMD_TAG_PRFX, "%s: processed inner pattern: \"%s\"", __func__, fake_cwd.compose(pattern));

    if (!fake_cwd.valid(pattern))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: pattern \"%s\" is invalid", __func__, pattern);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */

    	int entry_cnt = 0;
	DIR *dir;	// Directory descriptor
	struct stat statbuf;	// buffer for stat
	char* in_pattern = fake_cwd.compose(pattern);

    if (stat(in_pattern, &statbuf) == -1)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Listing dir is failed - pattern \"%s\" (%s) is not exist", __func__, pattern, in_pattern);
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(tmpstr, &statbuf) == -1 */
    if (!S_ISDIR(statbuf.st_mode))
    {
	if (pattern[strlen(pattern) - 1] == '/' || pattern[strlen(pattern) - 1] == '.')
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: %s -\n\t\t\t\t%s; pattern \"%s\" is invalid", __func__,
		    "Name of the file or other similar entity that is not a directory",
		    "cannot end with a slash or a dot", pattern);
	    return ESP_ERR_INVALID_ARG;
	}; /* if pattern[strlen(pattern) - 1] == '/' */

	ESP_LOGI(__func__, "\n%s %s, file size %ld bytes\n", pattern,
		    (S_ISLNK(statbuf.st_mode))? "[symlink]":
		    (S_ISREG(statbuf.st_mode))? "(file)":
		    (S_ISDIR(statbuf.st_mode))? "<DIR>":
		    (S_ISCHR(statbuf.st_mode))? "[char dev]":
		    (S_ISBLK(statbuf.st_mode))? "[blk dev]":
		    (S_ISFIFO(statbuf.st_mode))? "[FIFO]":
		    (S_ISSOCK(statbuf.st_mode))? "[socket]":
		    "[unknown type]", statbuf.st_size);
	return ESP_OK;
    }; /* if (!S_ISDIR(statbuf.st_mode)) */


    dir = opendir(in_pattern);
    if (!dir) {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Error opening directory <%s>, %s", __func__, pattern, strerror(errno));
	return ESP_FAIL;
    }; /* if !dir */

	esp_err_t ret = ESP_OK;

    ESP_LOGI(__func__, "Files in the directory <%s> (%s)",  pattern, in_pattern);
    printf("----------------\n");

#ifdef __PURE_C__
//#if __cplusplus < 201703L
    entry_cnt = listing_direntries_pureC(dir, in_pattern);
#else
    entry_cnt = listing_direntries_Cpp(dir, in_pattern);
#endif
    if (entry_cnt)
    {
	cout << "----------------" << endl;
	cout << aso::format("Total found %d files", entry_cnt) << endl;
    } /* if entry_cnt */
    else
    {
	ESP_LOGW(__func__, "Files or directory not found, directory is empty.");
	cout << "----------------" << endl;
    }; /* else if entry_cnt */

    if (errno != 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Error occured during reading of the directory <%s>, %s", __func__, pattern, strerror(errno));
	ret = ESP_FAIL;
    }; /* if errno != 0 */
    closedir(dir);
    cout << endl;
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
	char pathbuf[PATH_MAX + 1]; // @suppress("Symbol is not resolved")
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
	char pathbuf[PATH_MAX + 1]; // @suppress("Symbol is not resolved")
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
    printf("%s\tsize %ld bytes\n\t%s %s\n", fullpath, statbuf.st_size, name,
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


//#define __NOT_OVERWRITE__	// Deny overwrite cp & move destination files

#undef CMD_NM
#define CMD_NM "cp"
#define __CP_OVERWRITE_FILE__

// copy files according a pattern
esp_err_t Server::cp(SDMMC::Device& device, const char src_raw[], const char dest_raw[])
{
    if (empty(src_raw))
    {

	ESP_LOGE(CMD_TAG_PRFX, "%s: too few arguments: invoke command \"%s\" without parameters.\n%s", __func__, __func__,
		"Don't know what to copy.");
	return ESP_ERR_INVALID_ARG;
    }; /* if is_empty(src_raw) */
    if (empty(dest_raw))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: too few arguments: invoke command \"%s\" with one parameters.\n%s", __func__, __func__,
		"Don't know where to copy.");
	return ESP_ERR_INVALID_ARG;
    }; /* if is_empty(dest_raw) */

    if (!fake_cwd.valid(src_raw))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: the source file name \"%s\" is invalid", __func__, src_raw);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */

    if (!fake_cwd.valid(dest_raw))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: the destination file name \"%s\" is invalid", __func__, dest_raw);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */

	char *src = fake_cwd.compose(src_raw);

#if __cplusplus < 201703L

	struct stat st;

    // Check if source file is not exist
    if (stat(src, &st) != 0)
    {
	// Source file must be exist
	ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" (%s) is not exist - copyng a non-existent file is not possible.\n%s",
		__func__, src_raw, src, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(src, &st) != 0 */
    if (S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: copyng directories is unsupported.\n%s",
		__func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */

    /* or open source file at this point? */
    src = (char*)malloc(strlen(device.curr_cwd()) + 1);
    if (src == nullptr)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: not enought memory for store source file name \"%s\"", __func__, device.curr_cwd());
	return ESP_ERR_NO_MEM;
    }; /* if (src == nullptr) */
    strcpy(src, device.curr_cwd());

	char* srcbase = basename(src);
	char *dest = device.get_cwd(dest_raw);

    // Check if destination file is exist
    if (stat(dest, &st) == 0)
    {
	// Destination file is exist
	ESP_LOGI(CMD_TAG_PRFX, "%s: path \"%s\" (%s) is exist - copy is write to an existent file or directory.",
		__func__, dest_raw, dest);
	// if destination - exist path, not a directory
	if (S_ISDIR(st.st_mode))
	{
	    strcat(dest, "/");
	    strcat(dest, srcbase);
	} /* if S_ISDIR(st.st_mode) */
    }; /* if stat(dest, &st) == 0 */

    // Re-check the modified version of the
    // destination filename, that may be exist:
    if (stat(dest, &st) == 0)
    {
	// the final name of the target file
	// must not be a existing directory name
	if (S_ISDIR(st.st_mode))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist \"%s\" directory by the destination file is denied; aborting.",
		    __func__, dest);
	    free(src);
	    return ESP_ERR_NOT_SUPPORTED;
	} /* if S_ISDIR(st.st_mode) */

#if !defined(__NOT_OVERWRITE__) && defined(__CP_OVERWRITE_FILE__)
		ESP_LOGW(CMD_TAG_PRFX, "%s: overwrite an existing file \"%s\".", __func__, dest);
#else
		ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite the existent file \"%s\" is denied; aborting.",
			__func__, dest);
		free(src);
		return ESP_ERR_NOT_SUPPORTED;
#endif	// __CP_OVER_EXIST_FILE__
    }; /* if stat(dest, &st) == 0 */

    // check the source and destination file are same
    if (strcmp(src, dest) == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: source & destination file name are same: \"%s\";\n\t\t\t copying file to iself is unsupported",
		__func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if strcmp(src, dest) == 0 */

    // destination file - OK, it's not exist or is may be overwrited
    ESP_LOGI(CMD_TAG_PRFX ":" CMD_NM, "copy file %s to %s", src, dest);

	FILE* srcfile = fopen(src, "rb");
	FILE* destfile = fopen(dest, "wb");

    free(src);

    if (!destfile)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "failed creating file %s, aborting ", device.curr_cwd());
	fclose(srcfile);
	return ESP_ERR_NOT_FOUND;
    }; /* if !destfile */

#define CP_BUFSIZE 512
	char buf[CP_BUFSIZE];
	size_t readcnt;

    while (!feof(srcfile))
    {
	readcnt = fread(buf, 1, CP_BUFSIZE, srcfile);
	if (readcnt == 0)
	    break;
	fwrite(buf, 1, readcnt, destfile);
    }; /* while !feof(srcfile) */

    fflush(destfile);
    fsync(fileno(destfile));
    fclose(destfile);
    fclose(srcfile);

    return ESP_OK;
#else

	struct stat st;

    // Check if source file is not exist
    if (stat(src, &st) != 0)
    {
	// Source file must be exist
	ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" (%s) is not exist - copyng a non-existent file is not possible.\n%s",
		__func__, src_raw, src, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(src, &st) != 0 */
    if (S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: copyng directories is unsupported.\n%s",
		__func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */

    /* or open source file at this point? */
    src = (char*)malloc(strlen(fake_cwd.current()) + 1);
    if (src == nullptr)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: not enought memory for store source file name \"%s\"", __func__, fake_cwd.current());
	return ESP_ERR_NO_MEM;
    }; /* if (src == nullptr) */
    strcpy(src, fake_cwd.current());

	char* srcbase = basename(src);
	char *dest = fake_cwd.compose(dest_raw);

    // Check if destination file is exist
    if (stat(dest, &st) == 0)
    {
	// Destination file is exist
	ESP_LOGI(CMD_TAG_PRFX, "%s: path \"%s\" (%s) is exist - copy is write to an existent file or directory.",
		__func__, dest_raw, dest);
	// if destination - exist path, not a directory
	if (S_ISDIR(st.st_mode))
	{
	    strcat(dest, "/");
	    strcat(dest, srcbase);
	} /* if S_ISDIR(st.st_mode) */
    }; /* if stat(dest, &st) == 0 */

    // Re-check the modified version of the
    // destination filename, that may be exist:
    if (stat(dest, &st) == 0)
    {
	// the final name of the target file
	// must not be a existing directory name
	if (S_ISDIR(st.st_mode))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist \"%s\" directory by the destination file is denied; aborting.",
		    __func__, dest);
	    free(src);
	    return ESP_ERR_NOT_SUPPORTED;
	} /* if S_ISDIR(st.st_mode) */

#if !defined(__NOT_OVERWRITE__) && defined(__CP_OVERWRITE_FILE__)
	ESP_LOGW(CMD_TAG_PRFX, "%s: overwrite an existing file \"%s\".", __func__, dest);
#else
	ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite the existent file \"%s\" is denied; aborting.", __func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
#endif	// __CP_OVER_EXIST_FILE__
    }; /* if stat(dest, &st) == 0 */

    // check the source and destination file are same
    if (strcmp(src, dest) == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: source & destination file name are same: \"%s\";\n\t\t\t copying file to iself is unsupported",
		__func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if strcmp(src, dest) == 0 */

    // destination file - OK, it's not exist or is may be overwrited
    ESP_LOGI(CMD_TAG_PRFX ":" CMD_NM, "copy file %s to %s", src, dest);

	FILE* srcfile = fopen(src, "rb");
	FILE* destfile = fopen(dest, "wb");

    free(src);

    if (!destfile)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "failed creating file %s, aborting ", fake_cwd.current());
	fclose(srcfile);
	return ESP_ERR_NOT_FOUND;
    }; /* if !destfile */

#define CP_BUFSIZE 512
	char buf[CP_BUFSIZE];
	size_t readcnt;

    while (!feof(srcfile))
    {
	readcnt = fread(buf, 1, CP_BUFSIZE, srcfile);
	if (readcnt == 0)
	    break;
	fwrite(buf, 1, readcnt, destfile);
    }; /* while !feof(srcfile) */

    fflush(destfile);
    fsync(fileno(destfile));
    fclose(destfile);
    fclose(srcfile);

    return ESP_OK;
#endif	// __cplusplus < 201703L

}; /* Server::cp */



#define __MV_OVERWRITE_FILE__

#undef CMD_NM
#define CMD_NM "mv"

// move files according a pattern
esp_err_t Server::mv(SDMMC::Device& device, const char src_raw[], const char dest_raw[])
{

    if (empty(src_raw))
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "too few arguments: invoke command \"%s\" with one parameters.\n%s", CMD_NM,
		"Don't know what to move?");
	return ESP_ERR_INVALID_ARG;
    }; /* if empty(src) */

    if (empty(dest_raw))
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "too few arguments: invoke command \"%s\" without parameters.\n%s", CMD_NM,
		"Don't know where to move?");
	return ESP_ERR_INVALID_ARG;
    }; /* if empty(dest) */

//    if (!device.valid_path(src_raw))
    if (!fake_cwd.valid(src_raw))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: the souce file name \"%s\" is invalid", __func__, src_raw);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */
    if (!fake_cwd.valid(dest_raw))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: the destination file name \"%s\" is invalid", __func__, dest_raw);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */



#if __cplusplus < 201703L

	char *src = device.get_cwd(src_raw);
	struct stat st_src;

    // Check if source file is not exist
    if (stat(src, &st_src) != 0)
    {
	// Source file must be exist
	ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" (%s) is not exist - renaming a non-existent file is not possible.\n%s",
		__func__, src_raw, src, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(src_stat, &st) != 0 */

    src = (char*)malloc(strlen(src) * sizeof(char) + 1);
    if (!src)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Not enought memory for store souce file name", __func__);
	return ESP_ERR_NO_MEM;
    }; /* if src == NULL*/
    strcpy(src, device.curr_cwd());
//    src = strcpy((char*)malloc(strlen(src) * sizeof(char)) + 1, src);


    	char *dest = NULL;
    	struct stat st_dest;

    dest = device.get_cwd(dest_raw);

    cout << aso::format("Move file \"%s\" (%s) to \"%s\" (%s)") %src_raw %src
			%dest_raw %dest << endl;

    if (stat(dest, &st_dest) == 0)
    {
	// Target file exist
	ESP_LOGW(CMD_TAG_PRFX, "%s: target file name \"%s\" (%s) exist",
		__func__, dest_raw, dest);
	// if destination is existing directory
	if (S_ISDIR(st_dest.st_mode))
	{
		char *basenm = basename(src);

	    ESP_LOGD(CMD_TAG_PRFX, "%s: destination file is exist directory,\n\t\t\tbasename of src is: %s ", __func__,
		    basenm);
	    strcat(dest, "/");
	    ESP_LOGD(CMD_TAG_PRFX, "%s: adding trailing slash to a destination file: %s", __func__, dest);
	    strcat(dest, basenm);
	    ESP_LOGD(CMD_TAG_PRFX, "%s: adding src basename to a destination file: %s", __func__, dest);
	} /* if S_ISDIR(st.st_mode) */
    } /* if stat(dest, &st) != 0 */

    // Re-check the modified version of the
    // destination filename, that may be exist:
    if (stat(dest, &st_dest) == 0)
    {
	// the final name of the target file
	// must not be a existing directory name
	if (S_ISDIR(st_dest.st_mode))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist directory \"%s\" by the destination file from the %s is not allowed; aborting.",
		    __func__, dest, src);
	    free(src);
	    return ESP_ERR_NOT_SUPPORTED;
	} /* if S_ISDIR(st.st_mode) */

	// if source - is dir, but destination - ordinary file
	if (S_ISDIR(st_src.st_mode))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist file \"%s\" by renaming the source directory %s to it - is not allowed; aborting.",
		    __func__, dest, src);
	    return ESP_ERR_NOT_SUPPORTED;
	}; /* if S_ISDIR(st.st_mode) */

#if !defined(__NOT_OVERWRITE__) && defined(__CP_OVERWRITE_FILE__)
	ESP_LOGW(CMD_TAG_PRFX, "%s: overwrite an existing file \"%s\".", __func__, dest);
#else
	ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite the existent file \"%s\" is denied; aborting.",
		__func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
#endif	// __CP_OVER_EXIST_FILE__
    }; /* if stat(dest, &st) == 0 */

    // Names of the moving/renaming file
    ESP_LOGI(CMD_TAG_PRFX, "%s: Moving/renaming file %s (%s) to %s (%s)", __func__, src_raw, src, dest_raw, dest);
    // check the source and destination file are same
    if (strcmp(src, dest) == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: source & destination file name are same: \"%s\";\n\t\t\t copying file to iself is unsupported",
		__func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if strcmp(src, dest) == 0 */


//    // Rename original file
//    cout << aso::format("Move/rename file %s (%s) to %s (%s)") %src_raw %src
//			%dest_raw %dest << endl;
    if (rename(src, dest/*device.curr_cwd()*/) != 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Error %d: %s", __func__, errno, strerror(errno));
    	free(src);
    	return ESP_FAIL;
    }; /* if rename(src, dest) != 0 */
    free(src);
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "the command '%s' now is partyally implemented for C edition", __func__);
    return ESP_OK;
#else

	char *src = fake_cwd.compose(src_raw);
	struct stat st_src;

    // Check if source file is not exist
    if (stat(src, &st_src) != 0)
    {
	// Source file must be exist
	ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" (%s) is not exist - renaming a non-existent file is not possible.\n%s",
		__func__, src_raw, src, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(src_stat, &st) != 0 */

    src = (char*)malloc(strlen(src) * sizeof(char) + 1);
    if (!src)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Not enought memory for store souce file name", __func__);
	return ESP_ERR_NO_MEM;
    }; /* if src == NULL*/
    strcpy(src, fake_cwd.current());


	char *dest = NULL;
	struct stat st_dest;

    dest = fake_cwd.compose(dest_raw);

    cout << aso::format("Move file \"%s\" (%s) to \"%s\" (%s)") %src_raw %src
			%dest_raw %dest << endl;

    if (stat(dest, &st_dest) == 0)
    {
	// Target file exist
	ESP_LOGW(CMD_TAG_PRFX, "%s: target file name \"%s\" (%s) exist",
		__func__, dest_raw, dest);
	// if destination is existing directory
	if (S_ISDIR(st_dest.st_mode))
	{
		char *basenm = basename(src);

	    ESP_LOGD(CMD_TAG_PRFX, "%s: destination file is exist directory,\n\t\t\tbasename of src is: %s ", __func__,
		    basenm);
	    strcat(dest, "/");
	    ESP_LOGD(CMD_TAG_PRFX, "%s: adding trailing slash to a destination file: %s", __func__, dest);
	    strcat(dest, basenm);
	    ESP_LOGD(CMD_TAG_PRFX, "%s: adding src basename to a destination file: %s", __func__, dest);
	} /* if S_ISDIR(st.st_mode) */
    } /* if stat(dest, &st) != 0 */

    // Re-check the modified version of the
    // destination filename, that may be exist:
    if (stat(dest, &st_dest) == 0)
    {
	// the final name of the target file
	// must not be a existing directory name
	if (S_ISDIR(st_dest.st_mode))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist directory \"%s\" by the destination file from the %s is not allowed; aborting.",
		    __func__, dest, src);
	    free(src);
	    return ESP_ERR_NOT_SUPPORTED;
	} /* if S_ISDIR(st.st_mode) */

	// if source - is dir, but destination - ordinary file
	if (S_ISDIR(st_src.st_mode))
	{
	    ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite exist file \"%s\" by renaming the source directory %s to it - is not allowed; aborting.",
		    __func__, dest, src);
	    return ESP_ERR_NOT_SUPPORTED;
	}; /* if S_ISDIR(st.st_mode) */

#if !defined(__NOT_OVERWRITE__) && defined(__CP_OVERWRITE_FILE__)
	ESP_LOGW(CMD_TAG_PRFX, "%s: overwrite an existing file \"%s\".", __func__, dest);
#else
	ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite the existent file \"%s\" is denied; aborting.",
		__func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
#endif	// __CP_OVER_EXIST_FILE__
    }; /* if stat(dest, &st) == 0 */

    // Names of the moving/renaming file
    ESP_LOGI(CMD_TAG_PRFX, "%s: Moving/renaming file %s (%s) to %s (%s)", __func__, src_raw, src, dest_raw, dest);
    // check the source and destination file are same
    if (strcmp(src, dest) == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: source & destination file name are same: \"%s\";\n\t\t\t copying file to iself is unsupported",
		__func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if strcmp(src, dest) == 0 */


//    // Rename original file
//    cout << aso::format("Move/rename file %s (%s) to %s (%s)") %src_raw %src
//			%dest_raw %dest << endl;
    if (rename(src, dest/*device.curr_cwd()*/) != 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Error %d: %s", __func__, errno, strerror(errno));
	free(src);
	return ESP_FAIL;
    }; /* if rename(src, dest) != 0 */
    free(src);
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "the command '%s' now is partyally implemented for C edition", __func__);
    return ESP_OK;
    #endif
}; /* Server::mv */



#undef CMD_NM
#define CMD_NM "rm"

// remove files according a pattern
esp_err_t Server::rm(SDMMC::Device& device, const char pattern[])
{

//    if (!device.valid_path(pattern))
    if (!fake_cwd.valid(pattern))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: pattern \"%s\" is invalid", __func__, pattern);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(pattern) */

	struct stat st;
	char *path = fake_cwd.compose(pattern);

    if (empty(pattern))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, __func__,
		"Missing filename to remove.");
	return ESP_ERR_INVALID_ARG;
    }; /* if empty(pattern) */ /* if pattern == NULL || strcmp(pattern, "") */

    ESP_LOGI(CMD_TAG_PRFX, "%s: delete file \"%s\" (%s)", __func__,  pattern, path);
//#ifdef __PURE_C__
#if __cplusplus < 201703L

    // Check if destination file exists before deleting
    if (stat(path, &st) != 0)
    {
        // deleting a non-existent file is not possible
	ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" is not exist - deleting a non-existent file is not possible.\n%s",
		__func__, pattern, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(file_foo, &st) != 0 */
    if (S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: deleting directories unsupported.\n%s",
		__func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */
    errno = 0;
    //cout << "Now exec: ===>> " << aso::format("unlink(%s)") % path << "<<===" << endl;
    unlink(path);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when deleting \"%s\": %s", __func__, pattern, strerror(errno));

//	ESP_LOGE(CMD_TAG_PRFX, "%s: Error %d: %s", __func__, errno, strerror(errno));
	return ESP_FAIL;
    }; /* if errno */

    return ESP_OK;
#else
    // Check if destination file exists before deleting
    if (stat(path, &st) != 0)
    {
        // deleting a non-existent file is not possible
	ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" is not exist - deleting a non-existent file is not possible.\n%s",
		__func__, pattern, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(file_foo, &st) != 0 */
    if (S_ISDIR(st.st_mode))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: deleting directories unsupported.\n%s",
		__func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */
    errno = 0;
    //cout << "Now exec: ===>> " << aso::format("unlink(%s)") % path << "<<===" << endl;
    unlink(path);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: Fail when deleting \"%s\": %s", __func__, pattern, strerror(errno));

	return ESP_FAIL;
    }; /* if errno */

    return ESP_OK;

#endif // __cplusplus < 201703L

}; /* Server::rm */



#undef CMD_NM
#define CMD_NM "cat"

// type file contents
esp_err_t Server::cat(SDMMC::Device& device, const char fname[])
{

    if (!fake_cwd.valid(fname))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: pattern \"%s\" is invalid", __func__, fname);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(fname) */

	struct stat st;
	char *fullname = fake_cwd.compose(fname);
	FILE *text = nullptr; // file for type to screen

    if (empty(fname))
    {
	cout << endl
	     << "*** Printing contents of the file <XXXX fname>. ***" << endl
	     << endl;
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "invoke command \"%s\" without parameters.\n%s", CMD_NM,
		"Missing filename for print to output.");

	cout << "*** End of printing file XXXX. ** ******************" << endl;
	return ESP_ERR_INVALID_ARG;
    }; /* if empty(fname) */ /* if fname == NULL || strcmp(fname, "") */

    cout << endl
	 << aso::format("*** Printing contents of the file <%s> (realname '%s'). ***") % fname % fullname  << endl
	 << endl;

#if __cplusplus < 201703L

    // Check if destination file exists before deleting
    if (stat(fullname, &st) != 0)
    {
        // typing a non-exist file is not possible
    	ESP_LOGE(CMD_TAG_PRFX, "%s: \"%s\" file does not exist - printing of the missing file is not possible.\n%s",
    		__func__, fname, esp_err_to_name(ESP_ERR_NOT_FOUND));
    	return ESP_ERR_NOT_FOUND;
    }; /* if stat(path, &st) != 0 */

    if (S_ISDIR(st.st_mode))
    {
    	ESP_LOGE(CMD_TAG_PRFX, "%s: Typing directories unsupported, use the 'ls' command instead.\n%s",
    		__func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
    	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */

    errno = 0;	// clear possible errors
    text = fopen(fullname, "r"); // open the file for type to screen
    if (!text)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error opening file <%s> (%s), %s", fname, fullname, strerror(errno));
	return ESP_FAIL;
    }; /* if !FILE */

    for (char c = getc(text); !feof(text); c = getc(text))
	putchar(c);

//    putchar('\n');
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error during type the file %s (%s) to output, %s", fname, fullname, strerror(errno));
	fclose(text);
	return ESP_FAIL;
    }; /* if errno */


#else
    // Check if destination file exists before deleting
    if (stat(fullname, &st) != 0)
    {
        // typing a non-exist file is not possible
    	ESP_LOGE(CMD_TAG_PRFX, "%s: \"%s\" file does not exist - printing of the missing file is not possible.\n%s",
    		__func__, fname, esp_err_to_name(ESP_ERR_NOT_FOUND));
    	return ESP_ERR_NOT_FOUND;
    }; /* if stat(path, &st) != 0 */

    if (S_ISDIR(st.st_mode))
    {
    	ESP_LOGE(CMD_TAG_PRFX, "%s: Typing directories unsupported, use the 'ls' command instead.\n%s",
    		__func__, esp_err_to_name(ESP_ERR_NOT_SUPPORTED));
    	return ESP_ERR_NOT_SUPPORTED;
    }; /* if (S_ISDIR(st.st_mode)) */

    errno = 0;	// clear possible errors
    text = fopen(fullname, "r"); // open the file for type to screen
    if (!text)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error opening file <%s> (%s), %s", fname, fullname, strerror(errno));
	return ESP_FAIL;
    }; /* if !FILE */

    for (char c = getc(text); !feof(text); c = getc(text))
	putchar(c);

    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error during type the file %s (%s) to output, %s", fname, fullname, strerror(errno));
	fclose(text);
	return ESP_FAIL;
    }; /* if errno */
#endif	// __cplusplus < 201703L

    cout << endl
	 << "*** End of printing file " << fname << ". **************" << endl
	 << endl;

    fclose(text);
    if (errno)
    {
	ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error during closing the file %s (%s) to output, %s", fname, fullname, strerror(errno));
	fclose(text);
	return ESP_ERR_INVALID_STATE;
    }; /* if errno */

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
esp_err_t Server::type(SDMMC::Device& device, const char fname[], size_t sector_size)
{
    if (!fake_cwd.valid(fname))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: pattern \"%s\" is invalid", __func__, fname);
	return ESP_ERR_NOT_FOUND;
    }; /* !device.valid_path(fname) */

	struct stat st;
	char *fullname = fake_cwd.compose(fname);
	FILE *storage = NULL;

    // Test file 'fname' for existing
    errno = 0;	// clear all error state
    if (access(fullname, F_OK) == -1)
    {
	if (errno == ENOENT)	// error "file does not exist"
	{

	    if (!stat(fullname, &st))	// but if the fname still exists here, then it is a directory // @suppress("Symbol is not resolved")
		return err4existent(fname, &st);
	    ESP_LOGI(CMD_TAG_PRFX CMD_NM, "OK, file \"%s\" does not exist, opening this file.", fname);
	    cout << aso::format("Open file %s for the write") % fullname << endl;
	    errno = 0;	// clear error state
	    storage = fopen(fullname, "w");
	} /* if errno == ENOENT */
	else	// error other than "file does not exist"
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error test existing file %s: %s", fullname , strerror(errno));
	    return ESP_FAIL;
	} /* else if errno == ENOENT */
    } /* if stat(fname, &statbuf) == -1 */
    else
    {	// Error - file fname is exist

	    char c;

	// fname exists, check that is a regular file
	if (!stat(fullname, &st) && !S_ISREG(st.st_mode)) // @suppress("Symbol is not resolved")
	    return err4existent(fname, &st);

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
	    storage = fopen(fullname, "a");
	    break;

	case 'o':
	case 'w':
	    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "OK, open the file %s to owerwrite.", fname);
	    cout << aso::format("File %s is opened to truncate+write (overwrite).") % fname << endl;
	    storage = fopen(fullname, "w");
	    break;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
	case '\n':
	    cout << "Enter char '\\n'" << endl; // @suppress("No break at end of case")
	case 'n':
	    ESP_LOGW(CMD_TAG_PRFX ":" CMD_NM " <filename>", "User cancel opening file %s.", fname);
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
//#define TYPEBUFSIZE (4 * card->self->csd.sector_size)
#define TYPEBUFSIZE (4 * sector_size)
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

}; //--[ namespace Exec ]----------------------------------------------------------------------------------------------


//--[ sdcard_ctrl.cpp ]----------------------------------------------------------------------------
