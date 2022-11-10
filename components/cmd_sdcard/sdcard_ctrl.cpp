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
#include "esp_err.h"
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

#include <sdcard_io>
#include <sdcard_ctrl>


#include "extrstream"


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

//    static const char *TAG = "SD/MMC service";


//--[ class Server ]------------------------------------------------------------------------------------------------



const char* const Server::MOUNT_POINT_Default = SD_MOUNT_POINT;


#undef CMD_TAG_PRFX
#define CMD_TAG_PRFX "SD/MMC CMD server:"

    // Mount SD-card with default parameters
    esp_err_t Server::mount(SDMMC::Device& device, SDMMC::Card& card) { // @suppress("Type cannot be resolved") // @suppress("Member declaration not found")
	return device.mount(card, MOUNT_POINT_Default); // @suppress("Method cannot be resolved")
    }; /* Server::mount */

    // Mount default SD-card slot onto path "mountpoint"
    esp_err_t Server::mount(SDMMC::Device& device, SDMMC::Card& card, const char mountpoint[]) // @suppress("Type cannot be resolved") // @suppress("Member declaration not found")
    {
	if (isdigit(mountpoint[0]))
	    return mount(device, card, atoi(mountpoint)); // @suppress("Invalid arguments")
	return device.mount(card, mountpoint); // @suppress("Method cannot be resolved")
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto default mount path
    esp_err_t Server::mount(SDMMC::Device& device, SDMMC::Card &card, int slot_no) // @suppress("Member declaration not found") // @suppress("Type cannot be resolved")
    {
	device.slot_no(slot_no); // @suppress("Method cannot be resolved")
	return mount(device, card); // @suppress("Invalid arguments")
    }; /* Server::mount */

    // Mount SD-card slot "slot_no" onto specified mount path
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


//// print current directory name
//esp_err_t Server::pwd()
//{
//#ifdef __PURE_C__
//	char* buf = getcwd(NULL, 0);
////	size_t buflen = sizeof(buf) + 1;
//
//    if (!buf)
//	return errno;
//    cout << endl
//	<< "PWD is: \"" << buf << '"' << endl
//	<< endl;
//    free(buf);
//
////    buf = (char*)malloc(buflen);
////    FRESULT res = f_getcwd(buf, buflen);
////    if (res != FR_OK)
////    {
////	cout << "Name of the current working directory was not copied into buffer." << endl
////		<< "Return code is: " << res << endl;
////    }; /* if res != FR_OK */
////
////    cout << aso::format("Current dir by f_getcwd version is: \"%s\"") % buf << endl;
////    free(buf);
//
//    return ESP_OK;
//#else
//    cout << "Command \"pwd\" is not yet implemented now for C++ edition." << endl;
//    return ESP_ERR_INVALID_VERSION;
//#endif
//}; /* Server::pwd */



// print current directory name
esp_err_t Server::pwd(SDMMC::Device& device)
{
#ifdef __PURE_C__


	const char* buf = device.get_cwd();

    if (!buf)
	return errno;
    cout << endl
	<< "PWD is: \"" << buf << '"' << endl
	<< endl;

    return ESP_OK;
#else
    cout << "Command \"pwd\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::pwd */


#define CMD_NM "mkdir"
// create a new directory
esp_err_t Server::mkdir(SDMMC::Device& device, const char dirname[])
{
	struct stat statbuf;
	char *path = device.get_cwd(dirname);

    if (dirname == NULL || strcmp(dirname, "") == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, CMD_NM,
		"This command required the creating directory name.");
	return ESP_ERR_INVALID_ARG;
    }; /* if dirname == NULL || strcmp(dirname, "") */
#ifdef __PURE_C__

    ESP_LOGI(CMD_TAG_PRFX, "%s: Create directory with name \"%s\", real path is %s", __func__, dirname, path);

    if (stat(path, &statbuf) == 0)
        {
    	ESP_LOGE(CMD_TAG_PRFX, "%s: Invalid directory name - requested path \"%s\" is exist;\n", __func__, path);
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
    cout << "Change directory to " << '"' << dirname << '"' << endl;
    cout << "Command \"mkdir\" is not yet implemented now for C++ edition." << endl;
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::mkdir */

#undef CMD_NM
#define CMD_NM "rmdir"
// create a new directory
esp_err_t Server::rmdir(SDMMC::Device& device, const char dirname[])
{
	struct stat st;
	char *path = device.get_cwd(dirname);

    if (dirname == NULL || strcmp(dirname, "") == 0)
    {
	    ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, CMD_NM,
		     "This command required the name of the deleting directory.");
	    return ESP_ERR_INVALID_ARG;
    }; /* if dirname == NULL || strcmp(dirname, "") */
#ifdef __PURE_C__
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

	DIR *dir = opendir(/*dirname*/path);	// Directory descriptor

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
    cout << "Delete directory " << '<' << dirname << '>' << endl;
    //cout << aso::format("Command \"%s\" is not yet implemented now for C++ edition.") % CND_NM << endl;
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::rmdir */



#undef CMD_NM
#define CMD_NM "cd"

// change a current directory
esp_err_t Server::cd(SDMMC::Device& device, const char dirname[])
{
	esp_err_t err;

#ifdef __PURE_C__
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
esp_err_t Server::ls(SDMMC::Device& device, const char pattern[])
{
//#if defined(__PURE_C__) || __cplusplus < 201703L
//#ifdef __PURE_C__
    	int entry_cnt = 0;
	DIR *dir;	// Directory descriptor
	struct stat statbuf;	// buffer for stat
	char* in_pattern = device.get_cwd(pattern);

    ESP_LOGD(CMD_TAG_PRFX, "%s: pattern is             : \"%s\"", __func__, pattern);
    ESP_LOGD(CMD_TAG_PRFX, "%s: processed inner pattern: \"%s\"", __func__, in_pattern);

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

//    printf("Files in the directory <%s>\n",  pattern);
    ESP_LOGI(__func__, "Files in the directory <%s> (%s)",  pattern, in_pattern);
    printf("----------------\n");

#ifdef __PURE_C__
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



#undef CMD_NM
#define CMD_NM "cp"
#define __CP_OVER_EXIST_FILE__

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

	char *src = device.get_cwd(src_raw);

#ifdef __PURE_C__

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
    if (src_raw[strlen(src_raw) - 1] == '/' || (src_raw[strlen(src_raw) - 1] == '.' && src_raw[strlen(src_raw) - 2] == '/'))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: %s -\n\t\t\t\t%s; source file name \"%s\" is invalid", __func__,
		"The source file name must not be a directory",
		"and cannot end with a slash or a slash dot.", src_raw);
	return ESP_ERR_INVALID_ARG;
    }; /* if src_raw[strlen(src_raw) - 1] == '/' || (src_raw[strlen(src_raw) - 1] == '.' && src_raw[strlen(src_raw) - 2] == '/') */

	FILE* srcfile = fopen(device.curr_cwd(), "rb");

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

    if (strcmp(src, dest) == 0)
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: source & destination file name are same: \"%s\";\n\t\t\t copying file to iself is unsupported",
		__func__, dest);
	free(src);
	return ESP_ERR_NOT_SUPPORTED;
    }; /* if strcmp(src, dest) == 0 */

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

    // Retry Check if destination file exist,
    // destination - not a directory
    if (stat(dest, &st) == 0)
    {
#ifdef __CP_OVER_EXIST_FILE__
	ESP_LOGW(CMD_TAG_PRFX, "%s: overwrite an existing file \"%s\".", __func__, dest);
#else
	ESP_LOGE(CMD_TAG_PRFX, "%s: overwrite the existent file \"%s\" is prohibited; aborting.",
		__func__, dest);
	return ESP_ERR_NOT_SUPPORTED;
#endif	// __CP_OVER_EXIST_FILE__
    }; /* else if (S_ISDIR(st.st_mode)) */

    // destination file not exist or is overwrited
    ESP_LOGI(CMD_TAG_PRFX CMD_NM, "copy file %s to %s", src, dest);
    free(src);

	FILE* destfile = fopen(device.curr_cwd(), "wb");
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
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now for C++ edition.", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::cp */



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

#ifdef __PURE_C__

	char *src = device.get_cwd(src_raw);
	struct stat st;

    // Check if source file is not exist
    if (stat(src, &st) != 0)
    {
	// Source file must be exist
	ESP_LOGE(CMD_TAG_PRFX, "%s: file \"%s\" (%s) is not exist - renaming a non-existent file is not possible.\n%s",
		__func__, src_raw, src, esp_err_to_name(ESP_ERR_NOT_FOUND));
	return ESP_ERR_NOT_FOUND;
    }; /* if stat(src, &st) != 0 */

    cout << aso::format("Move file \"%s\" (%s) to \"%s\" (%s)", src_raw, device.get_cwd(src_raw),
			dest_raw, device.get_cwd(dest_raw)) << endl;
    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s (%s) to %s (%s)", src_raw, device.get_cwd(src_raw), dest_raw,
		    device.get_cwd(dest_raw));
    //if (rename(src, dest) != 0)
    //{
    //    ESP_LOGE(TAG, "Rename failed");
    //    return ESP_FAIL;
    //}; /* if rename(src, dest) != 0 */
    //return ESP_OK;
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "the command '%s' is not implemented now for C edition", "mv");
    return ESP_ERR_INVALID_VERSION;
#else
    ESP_LOGW(CMD_TAG_PRFX CMD_NM, "Command \"%s\" is not yet implemented now for C++ edition.", CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::mv */



#undef CMD_NM
#define CMD_NM "rm"

// remove files according a pattern
esp_err_t Server::rm(SDMMC::Device& device, const char pattern[])
{

	struct stat st;
	char *path = device.get_cwd(pattern);

//    if (pattern == NULL || strcmp(pattern, "") == 0)
    if (empty(pattern))
    {
	ESP_LOGE(CMD_TAG_PRFX, "%s: invoke command \"%s\" without parameters.\n%s", __func__, __func__,
		"Missing filename to remove.");
	return ESP_ERR_INVALID_ARG;
    }; /* if empty(pattern) */ /* if pattern == NULL || strcmp(pattern, "") */

//    cout << "Delete file " << '"' << pattern << '"' << endl;
    ESP_LOGI(CMD_TAG_PRFX, "%s: delete file \"%s\" (%s)", __func__,  pattern, path);
#ifdef __PURE_C__

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
#else
    ESP_LOGW(CMD_TAG_PRFX, "%s: Command \"%s\" is not yet implemented now for C++ edition.", __func__, CMD_NM);
    return ESP_ERR_INVALID_VERSION;
#endif
}; /* Server::rm */



#undef CMD_NM
#define CMD_NM "cat"

// type file contents
esp_err_t Server::cat(SDMMC::Device& device, const char fname[])
{
	struct stat st;
	char *fullname = device.get_cwd(fname);
	FILE *text = nullptr; // file for type to screen

//    if (fname == NULL || strcmp(fullname, "") == 0)
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

#ifdef __PURE_C__

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
#endif

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
	struct stat st;
	char *fullname = device.get_cwd(fname);
	FILE *storage = NULL;

#if 0
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
#endif


    // Test file 'fname' for existing
    errno = 0;	// clear all error state
//    if (access(fname, F_OK) == -1)
    if (access(fullname, F_OK) == -1)
    {
	if (errno == ENOENT)	// error "file does not exist"
	{
//		struct stat statbuf;

//	    if (!stat(fname, &statbuf))	// but if the fname still exists here, then it is a directory // @suppress("Symbol is not resolved")
	    if (!stat(fullname, &st))	// but if the fname still exists here, then it is a directory // @suppress("Symbol is not resolved")
//		return err4existent(fname, &statbuf);
		return err4existent(fname, &st);
	    ESP_LOGI(CMD_TAG_PRFX CMD_NM, "OK, file \"%s\" does not exist, opening this file.", fname);
	    cout << aso::format("Open file %s for the write") % fullname << endl;
	    errno = 0;	// clear error state
	    storage = fopen(fullname, "w");
	}
	else	// error other than "file does not exist"
	{
	    ESP_LOGE(CMD_TAG_PRFX CMD_NM, "Error test existing file %s: %s", fullname , strerror(errno));
	    return ESP_FAIL;
	}
    } /* if stat(fname, &statbuf) == -1 */
    else
    {	// Error - file fname is exist

//	    struct stat statbuf;
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


namespace Exec	//-----------------------------------------------------------------------------------------------------
{
    ;
}; //--[ namespace Exec ]----------------------------------------------------------------------------------------------



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
