/* SPDX-License-Identifier: MIT */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(chary_split_bond_recovery, LOG_LEVEL_INF);

struct stale_bond {
    uint8_t id;
    bt_addr_le_t peer;
};

K_MSGQ_DEFINE(stale_bond_queue, sizeof(struct stale_bond), CONFIG_BT_MAX_CONN, 4);

static void stale_bond_work_handler(struct k_work *work) {
    ARG_UNUSED(work);

    struct stale_bond stale;
    while (k_msgq_get(&stale_bond_queue, &stale, K_NO_WAIT) == 0) {
        char peer[BT_ADDR_LE_STR_LEN];
        bt_addr_le_to_str(&stale.peer, peer, sizeof(peer));

        LOG_WRN("Removing mismatched split bond for %s", peer);
        int err = bt_unpair(stale.id, &stale.peer);
        if (err) {
            LOG_ERR("Failed to remove mismatched split bond for %s: %d", peer, err);
        }
    }
}

K_WORK_DEFINE(stale_bond_work, stale_bond_work_handler);

static void split_security_changed(struct bt_conn *conn, bt_security_t level,
                                   enum bt_security_err err) {
    ARG_UNUSED(level);

    if (err != BT_SECURITY_ERR_PIN_OR_KEY_MISSING &&
        err != BT_SECURITY_ERR_KEY_REJECTED) {
        return;
    }

    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) != 0 || info.type != BT_CONN_TYPE_LE) {
        return;
    }

    const bool is_split_link = IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
                                   ? info.role == BT_CONN_ROLE_CENTRAL
                                   : info.role == BT_CONN_ROLE_PERIPHERAL;
    if (!is_split_link) {
        return;
    }

    struct stale_bond stale = {.id = info.id};
    bt_addr_le_copy(&stale.peer, info.le.dst);

    int queue_err = k_msgq_put(&stale_bond_queue, &stale, K_NO_WAIT);
    if (queue_err) {
        LOG_ERR("Unable to queue mismatched split bond recovery: %d", queue_err);
        return;
    }

    (void)k_work_submit(&stale_bond_work);
}

BT_CONN_CB_DEFINE(chary_split_bond_callbacks) = {
    .security_changed = split_security_changed,
};
