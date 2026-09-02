/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/reboot.h>

#include <zmk/event_manager.h>
#include <zmk/events/split_peripheral_status_changed.h>

LOG_MODULE_REGISTER(chary_split_recovery, LOG_LEVEL_INF);

static atomic_t connected;
static atomic_t connected_once;

static void recovery_work_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (atomic_get(&connected)) {
        return;
    }

    LOG_ERR("Split link did not recover; rebooting peripheral");
    sys_reboot(SYS_REBOOT_COLD);
}

K_WORK_DELAYABLE_DEFINE(recovery_work, recovery_work_handler);

static int split_status_listener(const zmk_event_t *eh) {
    const struct zmk_split_peripheral_status_changed *event =
        as_zmk_split_peripheral_status_changed(eh);

    if (event == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (event->connected) {
        atomic_set(&connected, 1);
        atomic_set(&connected_once, 1);
        (void)k_work_cancel_delayable(&recovery_work);
        return ZMK_EV_EVENT_BUBBLE;
    }

    atomic_set(&connected, 0);
    if (atomic_get(&connected_once)) {
        LOG_WRN("Split link lost; scheduling recovery reboot in %d seconds",
                CONFIG_CHARY_SPLIT_PERIPHERAL_RECOVERY_TIMEOUT);
        (void)k_work_reschedule(
            &recovery_work,
            K_SECONDS(CONFIG_CHARY_SPLIT_PERIPHERAL_RECOVERY_TIMEOUT));
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(chary_split_recovery, split_status_listener);
ZMK_SUBSCRIPTION(chary_split_recovery, zmk_split_peripheral_status_changed);
