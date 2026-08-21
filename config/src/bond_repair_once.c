/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <stdio.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(chary_bond_repair, LOG_LEVEL_INF);

#define REPAIR_SETTINGS_ROOT "chary_repair"
#define REPAIR_SETTINGS_KEY REPAIR_SETTINGS_ROOT "/bond_v1"
#define REPAIR_MARKER 0xA5

static bool repair_already_complete;
static struct k_work_delayable repair_work;

static int repair_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                               void *cb_arg) {
    if (!settings_name_steq(name, "bond_v1", NULL) || len != sizeof(uint8_t)) {
        return -ENOENT;
    }

    uint8_t marker = 0;
    int err = read_cb(cb_arg, &marker, sizeof(marker));
    if (err < 0) {
        return err;
    }

    repair_already_complete = marker == REPAIR_MARKER;
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(chary_bond_repair, REPAIR_SETTINGS_ROOT, NULL,
                               repair_settings_set, NULL, NULL);

static void delete_setting(const char *name) {
    int err = settings_delete(name);
    if (err && err != -ENOENT) {
        LOG_WRN("Failed to delete %s: %d", name, err);
    }
}

static void repair_work_handler(struct k_work *work) {
    if (repair_already_complete) {
        LOG_INF("One-time Bluetooth bond repair already complete");
        return;
    }

    LOG_WRN("Removing every legacy Bluetooth and split bond");

    int err = bt_unpair(BT_ID_DEFAULT, BT_ADDR_LE_ANY);
    if (err && err != -ENOENT) {
        LOG_WRN("bt_unpair returned %d", err);
    }

    err = bt_disable();
    if (err && err != -EALREADY) {
        LOG_WRN("bt_disable returned %d", err);
    }

    delete_setting("ble/active_profile");

    for (int i = 0; i < 8; i++) {
        char name[40];

        snprintf(name, sizeof(name), "ble/profiles/%d", i);
        delete_setting(name);

        snprintf(name, sizeof(name), "ble/peripheral_addresses/%d", i);
        delete_setting(name);
    }

    const uint8_t marker = REPAIR_MARKER;
    err = settings_save_one(REPAIR_SETTINGS_KEY, &marker, sizeof(marker));
    if (err) {
        LOG_ERR("Failed to save repair marker: %d", err);
        k_work_reschedule(&repair_work, K_SECONDS(2));
        return;
    }

    LOG_INF("Legacy bonds removed; rebooting into clean pairing state");
    k_sleep(K_MSEC(100));
    sys_reboot(SYS_REBOOT_COLD);
}

static int repair_init(void) {
    k_work_init_delayable(&repair_work, repair_work_handler);
    return k_work_schedule(&repair_work, K_SECONDS(2));
}

SYS_INIT(repair_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
