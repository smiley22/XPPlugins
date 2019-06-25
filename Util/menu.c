/**
 * Utility library for X-Plane 11 Plugins.
 *
 * Static library containing common functionality for stuff like logging and
 * dealing with configuration files. Linked against by most plugins in the
 * solution.
 *
 * Copyright 2019 Torben Könke.
 */
#include "util.h"

static XPLMMenuID menu_id;
static menu_item_t menu_items[MAX_MENU_ITEMS];

static void menu_cb(void *menu_ref, void *item_ref) {
    int index = (int)item_ref;
    menu_item_t *item = &menu_items[index];
    XPLMMenuCheck check;
    XPLMCheckMenuItemState(menu_id, index, &check);
    int new_val = check == xplm_Menu_Checked ? 0 : 1;

    XPLMCheckMenuItem(menu_id, index, new_val ? xplm_Menu_Checked :
        xplm_Menu_Unchecked);
    if (item->ini_name)
        ini_seti(item->ini_name, new_val);
    if (item->var)
        *(item->var) = new_val;
}

int menu_init(const char *name, menu_item_t *items, int num) {
    if (!(menu_id = XPLMCreateMenu(name, NULL, 0, menu_cb, NULL)))
        return 0;
    memcpy(menu_items, items, num * sizeof(menu_item_t));
    for (int i = 0; i < num; i++) {
        int index = XPLMAppendMenuItem(menu_id, menu_items[i].name, i, 0);
        if (index < 0)
            return 0;
        int val = menu_items[i].value;
        if (menu_items[i].ini_name)
            val = ini_geti(menu_items[i].ini_name, menu_items[i].value);
        if (menu_items[i].var)
            *(menu_items[i].var) = val;
        XPLMCheckMenuItem(menu_id, index, val ? xplm_Menu_Checked :
            xplm_Menu_Unchecked);
    }
    return 1;
}

int menu_deinit() {
    if (menu_id)
        XPLMDestroyMenu(menu_id);
    menu_id = NULL;
    return 1;
}