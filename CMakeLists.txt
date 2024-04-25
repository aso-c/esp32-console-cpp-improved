# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

#set(EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/examples/cxx/experimental/experimental_cpp_component")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
get_filename_component(ProjectId ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

message(WARNING "${ProjectId} (ESP32 Improved Console) version v.${CONFIG_APP_PROJECT_VER}-${CONFIG_APP_PROJECT_FLAVOUR} of ${CONFIG_APP_PROJECT_DATE} by ${CONFIG_APP_PROJECT_AUTHOR}(${CONFIG_APP_PROJECT_AUTHOR_NICK})")
message(STATUS "${ProjectId} Status message (ESP32 Improved Console) version v.${CONFIG_APP_PROJECT_VER}-${CONFIG_APP_PROJECT_FLAVOUR} of ${CONFIG_APP_PROJECT_DATE} from ${CONFIG_APP_PROJECT_AUTHOR}(${CONFIG_APP_PROJECT_AUTHOR_NICK})")
message(VERBOSE "${ProjectId} Verbose message (ESP32 Improved Console) version v.${CONFIG_APP_PROJECT_VER}-${CONFIG_APP_PROJECT_FLAVOUR} of ${CONFIG_APP_PROJECT_DATE} by ${CONFIG_APP_PROJECT_AUTHOR}(${CONFIG_APP_PROJECT_AUTHOR_NICK})")
