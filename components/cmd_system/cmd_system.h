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

#ifdef __cplusplus
}
#endif

// Enclose an argument in quotas
#define str(a) quote(a)
#define quote(a) #a

#define VERSION_STRING	"Version " str(VER_prj-VER_sfx) " of " str(DATE_prj) ", modified by " str(MODIFIER_prj) ".\n"
