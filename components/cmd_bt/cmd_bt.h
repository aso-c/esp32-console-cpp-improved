/*!
 * @file cmd_bt.h
 *
 * @brief Registering of the bluetooth command
 *
 * @detail Declaration of the bluetooth command registrating procedure
 * Header file.
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



#ifndef __CMD_BT__
#define __CMD_BT__


#ifdef __cplusplus
extern "C" {
#endif

/// Register bluetooth command
void register_bt_cmd(void);

#ifdef __cplusplus
}
#endif


namespace bt
{

    /// Bluetooth subsystem initialization (one-time on the boot)
    esp_err_t init(void);

}; /* namespace bt */



#endif // __CMD_BT__
