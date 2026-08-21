#pragma once

/*
 * ZMK 0.3 selects Secure Connections-only pairing unconditionally. The
 * affected dongle stops servicing the BLE link while its emulated ECDH
 * operation is active, so every fresh split bond ends in supervision timeout
 * 0x08. Keep SMP and bonding enabled, but do not advertise the emulated ECC
 * HCI commands and allow the peers to negotiate legacy Just Works pairing.
 *
 * This file is force-included after Zephyr's generated autoconf.h.
 */
#ifdef CONFIG_BT_SMP_SC_PAIR_ONLY
#undef CONFIG_BT_SMP_SC_PAIR_ONLY
#endif

#ifdef CONFIG_BT_TINYCRYPT_ECC
#undef CONFIG_BT_TINYCRYPT_ECC
#endif
