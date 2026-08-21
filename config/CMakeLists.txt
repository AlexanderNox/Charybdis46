zephyr_library()
zephyr_library_sources_ifdef(CONFIG_CHARY_BOND_REPAIR_ONCE
                             src/bond_repair_once.c)

if(CONFIG_CHARY_FORCE_LEGACY_SPLIT_PAIRING)
  # The generated Kconfig still enables Secure Connections through ZMK's
  # `select`.  Apply this header after autoconf.h so the Bluetooth host does
  # not advertise or enter the ECDH path on affected controller boards.
  zephyr_compile_options(
    -include ${CMAKE_CURRENT_LIST_DIR}/include/chary_legacy_split_pairing.h)
endif()

if(CONFIG_CHARY_SKIP_SPLIT_LAYOUT_SYNC)
  # ZMK 0.3 unconditionally writes the selected physical-layout index after
  # discovering every peripheral. This keyboard has only one layout, and the
  # write wedges the affected legacy-paired controller. Keep the characteristic
  # discovery intact but leave its handle invalid so all later sync attempts
  # safely return -EAGAIN.
  set(chary_split_central_source
      "${APPLICATION_SOURCE_DIR}/src/split/bluetooth/central.c")
  file(READ "${chary_split_central_source}" chary_split_central_contents)
  set(chary_layout_handle_original
      "slot->selected_physical_layout_handle = bt_gatt_attr_value_handle(attr);")
  set(chary_layout_handle_disabled
      "slot->selected_physical_layout_handle = 0; /* Chary46 has one fixed layout. */")
  string(FIND "${chary_split_central_contents}"
              "${chary_layout_handle_original}" chary_layout_handle_pos)
  if(NOT chary_layout_handle_pos EQUAL -1)
    string(REPLACE "${chary_layout_handle_original}"
                   "${chary_layout_handle_disabled}"
                   chary_split_central_contents
                   "${chary_split_central_contents}")
    file(WRITE "${chary_split_central_source}"
               "${chary_split_central_contents}")
  else()
    string(FIND "${chary_split_central_contents}"
                "${chary_layout_handle_disabled}" chary_layout_disabled_pos)
    if(chary_layout_disabled_pos EQUAL -1)
      message(FATAL_ERROR
              "Unable to disable ZMK split physical-layout synchronization")
    endif()
  endif()
endif()
