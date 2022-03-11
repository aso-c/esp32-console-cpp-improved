/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Register all system functions
void register_system(void);

// Register common system functions: "version", "restart", "free", "heap", "tasks"
void register_system_common(void);

// Register deep and light sleep functions
void register_system_sleep(void);

/*
 * @brief Get string with version information of project current state
 * @return string containing the current version of project.
 */
const char* version_str(void);

#ifdef __cplusplus
}
#endif

// Enclose an argument in quotas
//#define str(a) quote(a)
//#define quote(a) #a

#if 0
#define VERSION_STRING	"Version " str(VER_prj-VER_sfx) \
    " of " str(DATE_prj) "," \
    " modified by " str(MODIFIER_prj) "."
#endif

#define VERSION_STRING	"Version " VER_prj-VER_sfx \
    " of " DATE_prj "," \
    " modified by " MODIFIER_prj "."

