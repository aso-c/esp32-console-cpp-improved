/*!
 * @file cmd_bt.cpp
 *
 * @brief The bluetooth command implementation
 *
 * @detail Implementation of the bluetooth command for the Advanced Console project demo
 * Implementation file.
 *
 * @section LICENCE
 *
 * This code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *
 * @author: Solomatov A.A. (aso)
 * @version 0.7.0
 * @date Created on: 11 дек. 2024 г.
 *	Updated 24.01.2025
 */

#if 0
#include <errno.h>
#include <stdlib.h>
#endif	// if 0

#include <iostream>
#include <iomanip>

#include <cstdint>
#include <cstring>
#include <stdbool.h>
#include <cstdio>
#include <cinttypes>

#include <nvs.h>
#include <nvs_flash.h>

#include "esp_console.h"
#include "argtable3/argtable3.h"
//#include "freertos/event_groups.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
//#include <esp_gap_bt_api.h>
//#include <esp_bt_device.h>
//#include <esp_spp_api.h>

//#include <time.h>
#include <ctime>
#include <sys/time.h>

#include <argtable>
#include <console>

#include "bt_ctrl"
#include "cmd_bt.h"


#define SPP_TAG "ADVANCED_CONSOLE_BT_DEMO"
#define SPP_SERVER_NAME "ADVC_SPP_SRV"
#define EXAMPLE_DEVICE_NAME "ESP_ADVANCED_CONSOLE_SPP_ACCEPTOR"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
//#define SPP_SHOW_MODE SPP_SHOW_SPEED    /*Choose show mode: show data or speed*/
#define SPP_SHOW_MODE SPP_SHOW_DATA    /*Choose show mode: show data or speed*/



//namespace bt
//{
//    extern const esp::console::cmd cmd;
//    extern const esp::console::cmd longcmd;
//}; /* namespace bt */


/// Register bluetooth command
void register_bt_cmd(void);


namespace bt
{

    // Desired syntax:
    // variant 0: regexp's
    // [bt | bluetooth] [-h | --help] [help] { [classic] | [le] | lowenergy } [spp] [start | stop | status]
    //
    //   - variant 1: Multisyntax w/options
    // [bt | bluetooth] [-h | --help]
    // [bt | bluetooth] [--start | --stop | --status] [classic]|[le]
    //    extended variant:
    //    [bt | bluetooth] [--start | --stop | --status] [classic] | [le] | [ble] spp
    //
    //   - variant 2: Single syntax w/options
    // [bt | bluetooth] []
    // [bt | bluetooth] [-h | --help | --start | --stop | --status] [classic] | [le] [spp]
    //    extended variant:
    //    [bt | bluetooth] [-h | --help | --start | --stop | --status] [classic]|[le]|[ble] [spp]
    // (flags -h | --help with command - help about selected command)
    //

    // defined if argtable3.c: #define TREX_ICASE ARG_REX_ICASE : flag for regexp, ignore casing for the matches
    //	must be defined in my header argtable? As below??? :
    // #define TREX_ICASE ARG_REX_ICASE

    // And in argtable3.h - defined this:
    // #define ARG_REX_ICASE 1
    //
    //
    // or my must define:
    // #define REG_EXTENDED 1
    // #define REG_ICASE (REG_EXTENDED << 1)
    // ???

    arg::table::syntax::def syntax = {
		//	help    = arg_litn(NULL, "help", 0, 1, "display this help and exit"),
		    // origin->>	arg_lit0("hH", "help", /*nullptr*/ "help options for command or subcommand"),
			arg_rex0("hH", "Help", "classic|le|lowenergy", "<stack>", ARG_REX_ICASE/*Arg::Rex::ICase*/, "help options for command or subcommand"),
		//	arg_litn(NULL, "help,start,stop", 1, 1, "help or start or stop or status"),
			//arg_reg0(nullptr, "classic,ble,le", /*nullptr*/ "classic bluetooth or Low Energy BT (BLE)"),
		//	arg_lit0(nullptr, "classic,ble,le", /*nullptr*/ "classic bluetooth or Low Energy BT (BLE)"),
			arg_rex0("sS", "stack,Stack", "classic|ble|le|lowenergy", "<stack_type>", ARG_REX_ICASE/*Arg::Rex::ICase*/, "kind of bluetooth stack for operating"),
			arg_rex0("cC", "command,Command,cmd,Cmd", "start|stop|status", "<command_string>", ARG_REX_ICASE/*Arg::Rex::ICase*/, "command for operating with desired bluetooth stack")/*,*/
//			arg_end(20)/*,*/
    }; /* bt::syntax */;


    /// definition of the act for the bt/bluetooth command
    struct act: public arg::table::act_t<&syntax>
    {

	/// Execute command procedure with the pointer to the own syntax object
	static
	esp_err_t invoke(int argc, char * argv[]);

//	/// Run the Help procedure with the pointer to the own syntax object
//	static
//	esp_err_t help_impl(int argc, char* argv[]);

    }; /* class bt::act */


esp_err_t act::invoke(int argc, char* argv[])
{
    ESP_LOGI(SPP_TAG, "===>> The Bluetooth command execution!!!");

    std::clog << "Passed " << argc << " arguments" << std::endl;
    std::clog << "Args is:" << std::endl;
    for (int i = 0; i < argc; i++)
	std::clog << '\t' << argv[i] << std::endl;

    /*bt::*/syntax.parse(argc, argv);
    if (/*bt::*/syntax.err())
    {
	/*bt::*/syntax.error(stdout, argv[0]);
	return bt::syntax.err();
    }; /* if (syntax.err() != 0) */
#if 0
		for (int i = 0; i < argc; i++)
		    std::clog << '\t' << argv[i] << std::endl;
#endif

#if 0
			void** sntxtble = static_cast<void**>(cmd_cntxt->argtable);
		for (int i = 0; sntxtble[i] != nullptr && static_cast<arg_hdr*>(sntxtble[i]) != reinterpret_cast<void*>(ARG_TERMINATOR); i++)
		{
		    ;
		}; /* for void** currsntx */
#endif

    for (auto opt: /*bt::*/syntax.description)
    {
	;
    }; /* for opt: syntax.description */

#if 0
		int i;
	    printf("-a = %d\n", a->count);
	    printf("-b = %d\n", b->count);
	    printf("-c = %d\n", c->count);
	    printf("--verbose = %d\n", verb->count);

	    if (scal->count > 0)
		printf("--scalar=%d\n", scal->ival[0]);

	    if (o->count > 0)
		printf("-o %s\n", o->filename[0]);

	    for (i = 0; i < file->count; i++)
		printf("file[%d]=%s\n", i, file->filename[i]);
#endif

	return ESP_OK;
    }; /* act::invoke(int, char* []) */


    // short name (main) for the bluetooth command
    const esp::console::cmd<act> cmd ("bt",  "General Bluetooth command");

    // long name alias for the bluetooth command
    const esp::console::cmd<act> longcmd ("bluetooth");
    //const esp::console::cmd bluetooth_cmd ({
    //	.command = "bluetooth",
    //        .help = nullptr,
    //        .hint = nullptr,
    //        .func = global_lambda,
    //	.argtable = nullptr,
    //	.func_w_context = nullptr,
    //	.context = nullptr
    //}); /* bluetooth_cmd */


    // TODO esp_bt_controller_config_t bt_cfg - must be parameter?
//    esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp::bt::Controller controller (BT_CONTROLLER_INIT_CONFIG_DEFAULT());

//    // XXX If implemented a copy-constructor with esp_bt_controller_config_t& or esp_bt_controller_config_t&&,
//    // XXX then assignment initialization is possible as is below:
//    esp::bt::Controller controller = esp_bt_controller_config_t{(BT_CONTROLLER_INIT_CONFIG_DEFAULT())};


    /// One-time at the boot bluetooth subsystem initialization, application specific
    esp_err_t init(void)
    {
//        char bda_str[18] = {'\0'};
#if 0
        // TODO Separate NVS initialization - needed it or not?

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK( ret );
#endif
        //----------------------------------------


        // TODO ESP_BT_MODE - must be parameter? - Yep!
        // Preliminary elease bt controller memory;
        // ESP_BT_MODE_BLE      - BLE not used, used only classic BT;
        // ESP_BT_MODE_CLASSIC_BT - used only BLE, BT classic is not used
        // ESP_BT_MODE_BTDM	    - bluetoos is not used in the current bootup cycle
//        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
        ESP_ERROR_CHECK(controller.mem_release(ESP_BT_MODE_BLE));
        return controller.err();

    }; /* bt::init() */


    /// Start the bluetooth subsystem, application specific
    esp_err_t start()
    {
	ESP_LOGI(SPP_TAG, "+++>> Start the ###Bluetooth subsystem###");

		// FXIME Temporarily!!!
//		esp_err_t ret = ESP_OK;

	    // TODO esp_bt_controller_config_t bt_cfg - must be parameter?
//	    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	    // Initialize bt controller; pair procedure is esp_bt_controller_deinit(void)
//	    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
//	        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
//	        return ret;
//	    }
	    controller.init();
	    if (controller.err() != ESP_OK)
	    {
	        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(controller.err()));
	        return controller.err();
	    }; /* if controller.err() != ESP_OK */

	    // TODO ESP_BT_MODE_CLASSIC_BT - must be parameter?
	    // enable the BT controller; mode This mode must match the mode specified in the cfg of esp_bt_controller_init()
	    // pair procedure - esp_bt_controller_disable(void)
//	    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
//	        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
//	        return ret;
//	    }
	    controller.enable(ESP_BT_MODE_CLASSIC_BT);
	    if (controller.err() != ESP_OK)
	    {
	        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(controller.err()));
	        return controller.err();
	    }; /* if controller.err() != ESP_OK */

	return ESP_OK;

    }; /*  bt::start() */


    /// Stop the bluetooth subsystem, application specific
    esp_err_t stop()
    {
	ESP_LOGI(SPP_TAG, "--->> Stop the ***Bluetooth subsystem***");

	controller.disable();
	if (controller.err() != ESP_OK)
	{
	    ESP_LOGE(SPP_TAG, "%s disable controller failed: %s", __func__, esp_err_to_name(controller.err()));
	    return controller.err();
	}; /* if controller.err() != ESP_OK */

	controller.deinit();
	if (controller.err() != ESP_OK)
	{
	    ESP_LOGE(SPP_TAG, "%s deinitialize controller failed: %s", __func__, esp_err_to_name(controller.err()));
	    return controller.err();
	}; /* if controller.err() != ESP_OK */

	return ESP_OK;

    }; /*  bt::stop() */

}; /* namespace bt */


/// Register bluetooth command
void register_bt_cmd(void)
{
//    ESP_ERROR_CHECK(esp_console_cmd_register(&bt_cmd));
    ESP_ERROR_CHECK(bt::cmd.enreg());
//    ESP_ERROR_CHECK(esp_console_cmd_register(&bluetooth_cmd));
    ESP_ERROR_CHECK(bt::longcmd.enreg());
}; /* register_bt() */

