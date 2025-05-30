/*!
 * @file console_init.cpp
 *
 * @brief Console inoitialization
 *
 * @detail Inoitialization console in the Improved Console project
 * Implementation file.
 *
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 * SPDX-FileCopyrightText: 2025 Solomatov A.A. (aso)
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @author: Solomatov A.A. (aso)
 * @version 0.2
 * @date Created on: 28.05.2025.
 *	Updated 30.05.2025
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_console.h>
#include "sdkconfig.h"
#include <soc/soc_caps.h>
#include <driver/uart_vfs.h>
#include <driver/uart.h>
#include <driver/usb_serial_jtag.h>
#include <driver/usb_serial_jtag_vfs.h>
#include <esp_vfs_cdcacm.h>
#include <linenoise/linenoise.h>
#include <argtable3/argtable3.h>

#include "console_init"

//#define CONSOLE_MAX_CMDLINE_ARGS 8
constexpr size_t CONSOLE_MAX_CMDLINE_ARGS = 8;
//#define CONSOLE_MAX_CMDLINE_LENGTH 256
constexpr size_t CONSOLE_MAX_CMDLINE_LENGTH = 256;
//#define CONSOLE_PROMPT_MAX_LEN (32)
constexpr size_t CONSOLE_PROMPT_MAX_LEN = 32;

char prompt[CONSOLE_PROMPT_MAX_LEN]; // Prompt to be printed before each line

void initialize_console_peripheral()
{
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    uart_vfs_dev_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    uart_vfs_dev_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
    /* warning: missing initializer for member 'uart_config_t::flow_ctrl' */
	    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//	    .rx_flow_ctrl_thresh = 122,
#if SOC_UART_SUPPORT_REF_TICK
            .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
            .source_clk = UART_SCLK_XTAL,
#endif
    }; /* const uart_config_t uart_config */
#pragma GCC diagnostic pop

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(static_cast<uart_port_t>(CONFIG_ESP_CONSOLE_UART_NUM), 256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config(static_cast<uart_port_t>(CONFIG_ESP_CONSOLE_UART_NUM), &uart_config) );

    /* Tell VFS to use UART driver */
    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

#elif defined(CONFIG_ESP_CONSOLE_USB_CDC)
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_cdcacm_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_cdcacm_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Enable blocking mode on stdin and stdout */
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Enable blocking mode on stdin and stdout */
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

    usb_serial_jtag_driver_config_t jtag_config = {
        .tx_buffer_size = 256,
        .rx_buffer_size = 256,
    };

    /* Install USB-SERIAL-JTAG driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( usb_serial_jtag_driver_install(&jtag_config));

    /* Tell vfs to use usb-serial-jtag driver */
    usb_serial_jtag_vfs_use_driver();

#else
#error Unsupported console type
#endif

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);
}; /* initialize_console_peripheral() */

void initialize_console_library(const char *history_path)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_length = CONSOLE_MAX_CMDLINE_LENGTH,
            .max_cmdline_args	= CONSOLE_MAX_CMDLINE_ARGS,
#if CONFIG_LOG_COLORS
            .hint_color 	= atoi(LOG_COLOR_CYAN)
#endif
    }; /* esp_console_config_t console_config */
    ESP_ERROR_CHECK( esp_console_init(&console_config) );
#pragma GCC diagnostic pop

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

#if CONFIG_CONSOLE_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(history_path);
#endif // CONFIG_CONSOLE_STORE_HISTORY

    /* Figure out if the terminal supports escape sequences */
    const int probe_status = linenoiseProbe();
    if (probe_status) {         /* zero indicates success */
        linenoiseSetDumbMode(1);
    }
}; /* initialize_console_library(history_path) */
