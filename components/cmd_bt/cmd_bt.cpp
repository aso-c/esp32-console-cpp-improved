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
//#include <esp_bt.h>
//#include <esp_bt_main.h>
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



const esp_console_cmd_t bt_cmd = {
	.command = "bt",
        .help = "Main Bluetooth command",
        .hint = "Bluetooth command execution",
        .func = &bt_exec,
	.argtable = nullptr,
	.func_w_context = nullptr,
	.context = nullptr
}; /* bt_cmd */

/// Register bluetooth command
void register_bt_cmd(void)
{
    ESP_ERROR_CHECK( esp_console_cmd_register(&bt_cmd) );
}; /* register_bt() */


