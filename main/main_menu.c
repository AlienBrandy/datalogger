/**
 * main_menu.c
 * 
 * SPDX-FileCopyrightText: Copyright © 2024 Honulanding Software <dev@honulanding.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>
#include "main_menu.h"
#include "wifi_menu.h"
#include "console_windows.h"

static menu_item_t* show_wifi_menu(int argc, char* argv[])
{
    // switch menus
    wifi_menu_set_parent(main_menu);
    return wifi_menu(0, NULL);
}

static menu_item_t menu_item_main = {
    .func = main_menu,
    .cmd  = "",
    .desc = ""
};

static menu_item_t menu_item_wifi = {
    .func = show_wifi_menu,
    .cmd  = "wifi",
    .desc = "wifi submenu"
};

static menu_item_t* menu_item_list[] = 
{
    &menu_item_wifi,
};

static void show_help(void)
{
    // first item by convention is the name of the menu and gets unique formatting
    console_windows_printf(CONSOLE_WINDOW_2, "\nmain menu\n");

    static const int list_length = sizeof(menu_item_list) / sizeof(menu_item_list[0]);
    for (int i = 0; i < list_length; i++)
    {
        console_windows_printf(CONSOLE_WINDOW_2, "%-20s: %s\n", menu_item_list[i]->cmd, menu_item_list[i]->desc);
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
    console_windows_printf(CONSOLE_WINDOW_2, "unknown command [%s]\n", argv[0]);
    return NULL;
}
