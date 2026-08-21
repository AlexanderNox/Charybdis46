if(CONFIG_CHARY_PMW3610_PERIPHERAL_COMPAT)
  zephyr_library()
  zephyr_library_sources(src/keymap_peripheral_compat.c)
endif()
