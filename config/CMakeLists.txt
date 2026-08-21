zephyr_library()
zephyr_library_sources_ifdef(CONFIG_CHARY_PMW3610_PERIPHERAL_COMPAT
                             src/keymap_peripheral_compat.c)
zephyr_library_sources_ifdef(CONFIG_CHARY_BOND_REPAIR_ONCE
                             src/bond_repair_once.c)

if(CONFIG_CHARY_FORCE_LEGACY_SPLIT_PAIRING)
  # The generated Kconfig still enables Secure Connections through ZMK's
  # `select`.  Apply this header after autoconf.h so the Bluetooth host does
  # not advertise or enter the ECDH path on affected controller boards.
  zephyr_compile_options(
    -include ${CMAKE_CURRENT_LIST_DIR}/include/chary_legacy_split_pairing.h)
endif()
