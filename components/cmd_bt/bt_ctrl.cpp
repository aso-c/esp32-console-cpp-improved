/*!
 * @file bt_ctrl.cpp
 *
 * @brief bluetooth control code, implementation file
 *
 * @detail Definition of the bluetooth control procedures
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
 * @version 0.0.1
 * @date Created on: 16 дек. 2023 г.
 *	    Updated: 25.12.2024
 */

#if 0
#include <errno.h>
#include <stdlib.h>
#endif	// if 0

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "esp_console.h"
//FIXME line below - is not needed at this file, remove it?
// //#include "argtable3/argtable3.h"
//#include "freertos/event_groups.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_bt_device.h>
#include <esp_spp_api.h>

#include <time.h>
#include <sys/time.h>


#include "bt_ctrl"

#define SPP_TAG "BLUETOOTH_СTRL_MODULE"

/// if SSP is enabled
constexpr bool SSP_enabled = (CONFIG_EXAMPLE_SSP_ENABLED == true);

/// if SSP received data is dispayed?
constexpr bool SSP_Show_Data = true;


// TODO All defines must be removed in the final implementation, only procedural parameters must be used!!!
#define SPP_SERVER_NAME "ADVC_SPP_SRV"
#define EXAMPLE_DEVICE_NAME "ESP_ADVANCED_CONSOLE_SPP_ACCEPTOR"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
//#define SPP_SHOW_MODE SPP_SHOW_SPEED    /*Choose show mode: show data or speed*/
#define SPP_SHOW_MODE SPP_SHOW_DATA    /*Choose show mode: show data or speed*/


// TODO All constants - must be defined external or any specifical definition must be allied
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const bool esp_spp_enable_l2cap_ertm = true;

static struct timeval time_new, time_old;
static long data_num = 0;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;
//-----------------------------------------------------------------------







static char *bda2str(uint8_t * bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < 18) {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

[[maybe_unused]]
static void print_speed(void)
{
    float time_old_s = time_old.tv_sec + time_old.tv_usec / 1000000.0;
    float time_new_s = time_new.tv_sec + time_new.tv_usec / 1000000.0;
    float time_interval = time_new_s - time_old_s;
    float speed = data_num * 8 / time_interval / 1000.0;
    ESP_LOGI(SPP_TAG, "speed(%fs ~ %fs): %f kbit/s" , time_old_s, time_new_s, speed);
    data_num = 0;
    time_old.tv_sec = time_new.tv_sec;
    time_old.tv_usec = time_new.tv_usec;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    char bda_str[18] = {0};
#define STRRBUFLEN 63
    char *strbuf = static_cast<char *>(malloc(STRRBUFLEN + 1));
    int processed = 0;	// current pocessing position in the string

    switch (event) {
    case ESP_SPP_INIT_EVT:
        if (param->init.status == ESP_SPP_SUCCESS) {
            ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
            esp_spp_start_srv(sec_mask, role_slave, 0, SPP_SERVER_NAME);
        } else {
            ESP_LOGE(SPP_TAG, "ESP_SPP_INIT_EVT status:%d", param->init.status);
        }
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT status:%d handle:%" PRIu32 " close_by_remote:%d", param->close.status,
                 param->close.handle, param->close.async);
        break;
    case ESP_SPP_START_EVT:
        if (param->start.status == ESP_SPP_SUCCESS) {
            ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT handle:%" PRIu32 " sec_id:%d scn:%d", param->start.handle, param->start.sec_id,
                     param->start.scn);
            esp_bt_gap_set_device_name(EXAMPLE_DEVICE_NAME);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        } else {
            ESP_LOGE(SPP_TAG, "ESP_SPP_START_EVT status:%d", param->start.status);
        }
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:
	if constexpr (SSP_Show_Data)
	{
//#if (SPP_SHOW_MODE == SPP_SHOW_DATA)
	    /*
	     * We only show the data in which the data length is less than 128 here. If you want to print the data and
	     * the data rate is high, it is strongly recommended to process them in other lower priority application task
	     * rather than in this callback directly. Since the printing takes too much time, it may stuck the Bluetooth
	     * stack and also have a effect on the throughput!
	     */
	    ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len:%d handle:%" PRIu32,
		    param->data_ind.len, param->data_ind.handle);
	    if (param->data_ind.len < 128)
	    {
		esp_log_buffer_hex("", param->data_ind.data, param->data_ind.len);
	    }; /* if param->data_ind.len < 128 */
	} /* if constexpr (SSP_Show_Data) */
	else
	{
//#else
	    gettimeofday(&time_new, NULL);
	    data_num += param->data_ind.len;
	    if (time_new.tv_sec - time_old.tv_sec >= 3)
	    {
		print_speed();
	    }; /* if time_new.tv_sec - time_old.tv_sec >= 3 */
//#endif	// if (SPP_SHOW_MODE == SPP_SHOW_DATA)
	}; /* else if constexpr (SSP_Show_Data) */
        processed = (STRRBUFLEN > param->data_ind.len)? param->data_ind.len: STRRBUFLEN;
        strncpy(strbuf, (char*)(param->data_ind.data), processed);	// copy received string
        strbuf[processed] = '\0';	// terminate string
        ESP_LOGI(SPP_TAG, "===>> Received raw string: %s", strbuf);

#define RETPROMPT "> Returned: "
        strcpy(strbuf, RETPROMPT);
        processed = strlen(RETPROMPT);
        for (int i = ((STRRBUFLEN > param->data_ind.len)? param->data_ind.len: STRRBUFLEN) - 1; i >= 0; i--)
            strbuf[processed++] = param->data_ind.data[i];
        strbuf[processed] = '\0';	// terminate string

        esp_spp_write(param->start.handle, processed, (uint8_t*)strbuf);


        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT");
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT status:%d handle:%" PRIu32 ", rem_bda:[%s]", param->srv_open.status,
                 param->srv_open.handle, bda2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
        gettimeofday(&time_old, NULL);
        break;
    case ESP_SPP_SRV_STOP_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_STOP_EVT");
        break;
    case ESP_SPP_UNINIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_UNINIT_EVT");
        break;
    default:
        break;
    }; /* switch event */

    free(strbuf);
}; /* esp_spp_cb() */



void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    char bda_str[18] = {0};

    switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(SPP_TAG, "authentication success: %s bda:[%s]", param->auth_cmpl.device_name,
                     bda2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
        } else {
            ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            ESP_LOGI(SPP_TAG, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }

    case ESP_BT_GAP_CFM_REQ_EVT:
	if constexpr (SSP_enabled) {
	    ESP_LOGI(SPP_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %" PRIu32, param->cfm_req.num_val);
	    esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
	}; /* if constexpr (SSP_enabled) */
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
	if constexpr (SSP_enabled) {
	    ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%" PRIu32, param->key_notif.passkey); };
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
	if constexpr (SSP_enabled) {
	    ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!"); };
        break;

    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode:%d bda:[%s]", param->mode_chg.mode,
                 bda2str(param->mode_chg.bda, bda_str, sizeof(bda_str)));
        break;

    default: {
        ESP_LOGI(SPP_TAG, "event: %d", event);
        break;
    }
    }
    return;
}; /* esp_bt_gap_cb() */


namespace esp {
    namespace bt
    {

	esp_err_t Controller::mem_release(esp_bt_mode_t btmode)
	{
	    return (res = esp_bt_controller_mem_release(btmode));
	}; /* esp::bt::Controller::mem_release() */

	esp_err_t Controller::init()
	{
	    return (res = esp_bt_controller_init(this));
	}; /* esp::bt::Controller::init() */

	esp_err_t Controller::enable(esp_bt_mode_t mode)
	{
	    return (res = esp_bt_controller_enable(mode));
	}; /* esp::bt::Controller::enable() */

	esp_err_t Controller::deinit()
	{
	    return (res = esp_bt_controller_deinit());
	}; /* esp::bt::Controller::deinit() */

	esp_err_t Controller::disable()
	{
	    return (res = esp_bt_controller_disable());
	}; /* esp::bt::Controller::disable() */



	esp_err_t Controller::res = ESP_OK;

	namespace droid
	{

	    esp_err_t init()
	    {
		return (config_t::res = esp_bluedroid_init());
	    }; /* esp::bt::droid::init() */

	    esp_err_t config_t::init()
	    {
		return (res = esp_bluedroid_init_with_cfg(this));
	    }; /* esp::bt::droid::config_t::init() */

	    esp_err_t config_t::deinit()
	    {
		return (res = esp_bluedroid_deinit());
	    }; /* esp::bt::droid::config_t::deinit() */

	    esp_err_t config_t::enable()
	    {
		return (res = esp_bluedroid_enable());
	    }; /* esp::bt::droid::config_t::enable() */

	    esp_err_t config_t::disable()
	    {
		return (res = esp_bluedroid_disable());
	    }; /* esp::bt::droid::config_t::disable() */

	    esp_bluedroid_status_t
	    config_t::status()
	    {
		return esp_bluedroid_get_status();
	    }; /* esp::bt::droid::config_t::status() */

	    esp_err_t config_t::res = ESP_OK;

	}; /* namespace esp::bt::droid */


//void app_main(void)
void bt_init(void)
{
    char bda_str[18] = {'\0'};
    // TODO Separate NVS initialization - needed it or not?
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    //----------------------------------------


    // TODO ESP_BT_MODE - must be parameter? - Yep!
    // Preliminary elease bt controller memory;
    // ESP_BT_MODE_BLE      - BLE not used, used only classic BT;
    // ESP_BT_MODE_CLASSIC_BT - used only BLE, BT classic is not used
    // ESP_BT_MODE_BTDM	    - bluetoos is not used in the current bootup cycle
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    // TODO esp_bt_controller_config_t bt_cfg - must be parameter?
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    // Initialize bt controller; pair procedure is esp_bt_controller_deinit(void)
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // TODO ESP_BT_MODE_CLASSIC_BT - must be parameter?
    // enable the BT controller; mode This mode must match the mode specified in the cfg of esp_bt_controller_init()
    // pair procedure - esp_bt_controller_disable(void)
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }





    // TODO esp_bluedroid_config_t bluedroid_cfg - must be parameter?
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
#if (CONFIG_EXAMPLE_SSP_ENABLED == false)
    bluedroid_cfg.ssp_en = false;
#endif
    if ((ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s gap register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp register failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // TODO esp_spp_cfg_t bt_spp_cfg - must be parameter?
    esp_spp_cfg_t bt_spp_cfg = {
        .mode = esp_spp_mode,
        .enable_l2cap_ertm = esp_spp_enable_l2cap_ertm,
        .tx_buffer_size = 0, /* Only used for ESP_SPP_MODE_VFS mode */
    };
    if ((ret = esp_spp_enhanced_init(&bt_spp_cfg)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp init failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    if constexpr (SSP_enabled)
    {
	/* Set default parameters for Secure Simple Pairing */
	esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
	esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
	esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
    }; /* if (SSP_enabled) */

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    ESP_LOGI(SPP_TAG, "Own address:[%s]", bda2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
}; /* initialize_bt() / none_the_app_main() */


    }; /* namespace esp::bt */

}; /* namesoace esp */
