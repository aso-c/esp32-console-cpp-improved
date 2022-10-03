# ESP32 Console Example C++ Improved

An experimental project for explore porting the "Console" ESP32 example to C++ (where possibility) and adding some features, provided by ESP-IDF, for research them.

### Branch:
- main&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: initial branch of the GitHub repository, default by github settings.
- master&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: initial state of the project, original source from the ESP-IDF [repository](https://github.com/espressif/esp-idf/tree/master/examples/system/console/advanced), early development of the project.
- unencrypted : base branch for project developing at this stage; not prepared for flash encryption.
- encrypted&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: branch for testing encryption flash features on ESP32; implied prepare confuguratuion for flash encryption; codebase merged from the branch 'unencrypted'.
- unencrypted-cpp&nbsp;: late development of the project, main dev branch; configuration not prepared for flash encryption. Added SD-card support.
- encrypted-cpp&nbsp;: branch for testing encryption flash features on ESP32; implied prepare confuguratuion for flash encryption; codebase merged from the branch 'unencrypted-cpp'.

*On branch 'main'*.


#### *Contents of the original ESP32 example [console](https://github.com/espressif/esp-idf/tree/master/examples/system/console/advanced) project README.md [file](https://github.com/espressif/esp-idf/tree/master/examples/system/console/advanced/README.md)*:

================================================================================
# Console Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example illustrates the usage of the [Console Component](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/console.html#console) to create an interactive shell on the ESP32. The interactive shell running on the ESP32 can then be controlled/interacted with over a serial port (UART).

The interactive shell implemented in this example contains a wide variety of commands, and can act as a basis for applications that require a command-line interface (CLI).

## How to use example

### Hardware Required

This example should be able to run on any commonly available ESP32 development board.

### Configure the project

```
idf.py menuconfig
```

* Enable/disable `Example Configuration > Store command history in flash` as necessary

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

Enter the `help` command get a full list of all available commands. The following is a sample session of the Console Example where a variety of commands provided by the Console Example are used. Note that GPIO15 is connected to GND to remove the boot log output. 

```
This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
[esp32]> help
help 
  Print the list of registered commands

free 
  Get the total size of heap memory available

restart 
  Restart the program

deep_sleep  [-t <t>] [--io=<n>] [--io_level=<0|1>]
  Enter deep sleep mode. Two wakeup modes are supported: timer and GPIO. If no
  wakeup option is specified, will sleep indefinitely.
  -t, --time=<t>  Wake up time, ms
      --io=<n>  If specified, wakeup using GPIO with given number
  --io_level=<0|1>  GPIO level to trigger wakeup

join  [--timeout=<t>] <ssid> [<pass>]
  Join WiFi AP as a station
  --timeout=<t>  Connection timeout, ms
        <ssid>  SSID of AP
        <pass>  PSK of AP

[esp32]> free
257200
[esp32]> deep_sleep -t 1000
I (146929) deep_sleep: Enabling timer wakeup, timeout=1000000us
I (619) heap_init: Initializing. RAM available for dynamic allocation:
I (620) heap_init: At 3FFAE2A0 len 00001D60 (7 KiB): DRAM
I (626) heap_init: At 3FFB7EA0 len 00028160 (160 KiB): DRAM
I (645) heap_init: At 3FFE0440 len 00003BC0 (14 KiB): D/IRAM
I (664) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (684) heap_init: At 40093EA8 len 0000C158 (48 KiB): IRAM

This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
[esp32]> join --timeout 10000 test_ap test_password
I (182639) connect: Connecting to 'test_ap'
I (184619) connect: Connected
[esp32]> free
212328
[esp32]> restart
I (205639) restart: Restarting
I (616) heap_init: Initializing. RAM available for dynamic allocation:
I (617) heap_init: At 3FFAE2A0 len 00001D60 (7 KiB): DRAM
I (623) heap_init: At 3FFB7EA0 len 00028160 (160 KiB): DRAM
I (642) heap_init: At 3FFE0440 len 00003BC0 (14 KiB): D/IRAM
I (661) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (681) heap_init: At 40093EA8 len 0000C158 (48 KiB): IRAM

This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
[esp32]> 

```

## Troubleshooting

### Line Endings

The line endings in the Console Example are configured to match particular serial monitors. Therefore, if the following log output appears, consider using a different serial monitor (e.g. Putty for Windows) or modify the example's [UART configuration](#Configuring-UART-and-VFS).

```
This is an example of ESP-IDF console component.
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
Your terminal application does not support escape sequences.
Line editing and history features are disabled.
On Windows, try using Putty instead.
esp32>
```

## Example Breakdown

### Configuring UART

The ``initialize_console()`` function in the example configures some aspects of UART relevant to the operation of the console. 

- **Line Endings**: The default line endings are configured to match those expected/generated by common serial monitor programs, such as `screen`, `minicom`, and the `idf_monitor.py` included in the SDK. The default behavior for these commands are:
    - When 'enter' key is pressed on the keyboard, `CR` (0x13) code is sent to the serial device.
    - To move the cursor to the beginning of the next line, serial device needs to send `CR LF` (0x13 0x10) sequence.

### Line editing

The main source file of the example illustrates how to use `linenoise` library, including line completion, hints, and history.

### Commands

Several commands are registered using `esp_console_cmd_register()` function. See the `register_wifi()` and `register_system()` functions in `cmd_wifi.c` and `cmd_system.c` files.

### Command handling

Main loop inside `app_main()` function illustrates how to use `linenoise` and `esp_console_run()` to implement read/eval loop.

### Argument parsing

Several commands implemented in `cmd_wifi.c` and `cmd_system.c` use the Argtable3 library to parse and check the arguments.

### Command history

Each time a new command line is obtained from `linenoise`, it is written into history and the history is saved into a file in flash memory. On reset, history is initialized from that file.
