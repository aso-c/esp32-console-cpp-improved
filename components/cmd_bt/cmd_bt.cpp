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
 * @version 0.0.2
 * @date Created on: 11 дек. 2024 г.
 *	Updated 14.12.2024
 */

#if 0
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#endif	// if 0

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
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

#include <time.h>
#include <sys/time.h>


#include "bt_ctrl"
#include "cmd_bt.h"


#define SPP_TAG "ADVANCED_CONSOLE_BT_DEMO"
#define SPP_SERVER_NAME "ADVC_SPP_SRV"
#define EXAMPLE_DEVICE_NAME "ESP_ADVANCED_CONSOLE_SPP_ACCEPTOR"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
//#define SPP_SHOW_MODE SPP_SHOW_SPEED    /*Choose show mode: show data or speed*/
#define SPP_SHOW_MODE SPP_SHOW_DATA    /*Choose show mode: show data or speed*/




// Execute the bt command
esp_err_t bt_exec(int argc, char* argv[])
{
    ESP_LOGI(SPP_TAG, "===>> The Bluetooth command execution!!!");
    return ESP_OK;
}; /* bt_exec() */



// Desired syntax:
// variant 0: regexp's
// [bt | bluetooth] [-h | --help] | [classic] | [le] [spp] [start | stop | status]
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


namespace bt
{

void* syntax[] = {
//	help    = arg_litn(NULL, "help", 0, 1, "display this help and exit"),
	arg_lit1("hH", "help,start,stop,status", /*nullptr*/ "help or start or stop or status"),
//	arg_litn(NULL, "help,start,stop", 1, 1, "help or start or stop or status"),
	arg_lit0(nullptr, "classic,ble,le", /*nullptr*/ "classic bluetooth or Low Energy BT (BLE)"),
//	/*version = */arg_litn(NULL, "version", 0, 1, "display version info and exit"),
//	/*level   = */arg_intn(NULL, "level", "<n>", 0, 1, "foo value"),
//	/*verb    = */arg_litn("v", "verbose", 0, 1, "verbose output"),
//	/*o       = */arg_filen("o", NULL, "myfile", 0, 1, "output file"),
//	/*file    = */arg_filen(NULL, NULL, "<file>", 1, 100, "input files"),
	/*end     = */arg_end(20),
}; /* void* bt_syntax */

}; /* namespace bt */


const esp_console_cmd_t bt_cmd = {
	.command = "bt"/* | bluetooth"*/,
        .help = "General Bluetooth command",
        .hint = nullptr/*"Bluetooth command exec"*/,
        .func = &bt_exec,
//	.argtable = nullptr,
	.argtable = bt::syntax,
	.func_w_context = nullptr,
	.context = nullptr
}; /* bt_cmd */




/// Register bluetooth command
void register_bt_cmd(void)
{
    ESP_ERROR_CHECK( esp_console_cmd_register(&bt_cmd) );
}; /* register_bt() */


namespace bt
{

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

