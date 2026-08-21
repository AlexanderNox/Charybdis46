zephyr_library()
zephyr_library_sources_ifdef(CONFIG_CHARY_PMW3610_PERIPHERAL_COMPAT
                             src/keymap_peripheral_compat.c)
zephyr_library_sources_ifdef(CONFIG_CHARY_BOND_REPAIR_ONCE
                             src/bond_repair_once.c)
