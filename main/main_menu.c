/**
 * main_menu.c
 * 
 * SPDX-FileCopyrightText: Copyright © 2024 Honulanding Software <dev@honulanding.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>
#include "main_menu.h"
#include "network_manager_menu.h"
#include "datastream_menu.h"
#include "rgb_led_menu.h"
#include "config_menu.h"
#include "console_windows.h"
#include "esp_log.h"

static menu_item_t* show_network_manager_menu(int argc, char* argv[])
{
    // switch menus
    network_manager_menu_set_parent(main_menu);
    return network_manager_menu(0, NULL);
}

static menu_item_t* show_datastream_menu(int argc, char* argv[])
{
    // switch menus
    datastream_menu_set_parent(main_menu);
    return datastream_menu(0, NULL);
}

static menu_item_t* show_rgb_led_menu(int argc, char* argv[])
{
    // switch menus
    rgb_led_menu_set_parent(main_menu);
    return rgb_led_menu(0, NULL);
}

static menu_item_t* show_config_menu(int argc, char* argv[])
{
    // switch menus
    config_menu_set_parent(main_menu);
    return config_menu(0, NULL);
}

static menu_item_t* set_log_level(int argc, char* argv[])
{
    if (argc < 2)
    {
        console_windows_printf(MENU_WINDOW, "set_log_level: missing param(s)\n");
        return NULL;
    }
    int level = atoi(argv[1]);
    esp_log_level_set(PROJECT_NAME, level);
    return NULL;
}

static menu_item_t menu_item_main = {
    .func = main_menu,
    .cmd  = "",
    .desc = ""
};

static menu_item_t menu_item_set_log_level = {
    .func = set_log_level,
    .cmd  = "log",
    .desc = "set log level to <0:none thru 5:verbose>"
};

static menu_item_t menu_item_network_manager = {
    .func = show_network_manager_menu,
    .cmd  = "network",
    .desc = "network manager submenu"
};

static menu_item_t menu_item_datastream = {
    .func = show_datastream_menu,
    .cmd  = "datastream",
    .desc = "datastream submenu"
};

static menu_item_t menu_item_rgb_led = {
    .func = show_rgb_led_menu,
    .cmd  = "rgb",
    .desc = "rgb_led submenu"
};

static menu_item_t menu_item_config = {
    .func = show_config_menu,
    .cmd  = "config",
    .desc = "config submenu"
};

static menu_item_t* menu_item_list[] = 
{
    &menu_item_set_log_level,
    &menu_item_network_manager,
    &menu_item_datastream,
    &menu_item_rgb_led,
    &menu_item_config,
};

static void show_help(void)
{
    PRINT_MENU_TITLE("Main Menu");
    static const int list_length = sizeof(menu_item_list) / sizeof(menu_item_list[0]);
    for (int i = 0; i < list_length; i++)
    {
        console_windows_printf(MENU_WINDOW, "%-20s: %s\n", menu_item_list[i]->cmd, menu_item_list[i]->desc);
    }
}

menu_item_t* main_menu(int argc, char* argv[])
{
    // check for blank line which is an indication to display the help menu
    if (argc == 0 || argv == NULL)
    {
        show_help();
        return &menu_item_main;
    }

    // search for matching command in list of registered menu items
    static const int list_length = sizeof(menu_item_list) / sizeof(menu_item_list[0]);
    for (int i = 0; i < list_length; i++)
    {
        if (strcmp(argv[0], menu_item_list[i]->cmd) == 0)
        {
            // match found, execute menu item function.
            return (*menu_item_list[i]->func)(argc, argv);
        }
    }
    console_windows_printf(MENU_WINDOW, "unknown command [%s]\n", argv[0]);
    return NULL;
}
