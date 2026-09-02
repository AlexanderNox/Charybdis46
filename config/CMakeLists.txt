zephyr_library()
zephyr_library_sources_ifdef(CONFIG_CHARY_SPLIT_BOND_RECOVERY
                             src/split_bond_recovery.c)
zephyr_library_sources_ifdef(CONFIG_CHARY_SPLIT_PERIPHERAL_RECOVERY
                             src/split_peripheral_recovery.c)

if(CONFIG_CHARY_FORCE_LEGACY_SPLIT_PAIRING)
  # The generated Kconfig still enables Secure Connections through ZMK's
  # `select`.  Apply this header after autoconf.h so the Bluetooth host does
  # not advertise or enter the ECDH path on affected controller boards.
  zephyr_compile_options(
    -include ${CMAKE_CURRENT_LIST_DIR}/include/chary_legacy_split_pairing.h)
endif()

if(CONFIG_CHARY_DONGLE_USB_PRIORITY)
  # ZMK normally uses the persisted preferred transport when USB and a host
  # BLE profile are both connected. A dongle must be plug-and-play: as long as
  # its USB HID interface is ready, reports must go to that USB host without a
  # keymap command or a settings reset.
  set(chary_endpoints_source
      "${APPLICATION_SOURCE_DIR}/src/endpoints.c")
  file(READ "${chary_endpoints_source}" chary_endpoints_contents)
  set(chary_endpoint_selection_original
      "static enum zmk_transport get_selected_transport(void) {\n    if (is_ble_ready()) {")
  set(chary_endpoint_selection_usb_priority
      "static enum zmk_transport get_selected_transport(void) {\n    if (IS_ENABLED(CONFIG_CHARY_DONGLE_USB_PRIORITY) && is_usb_ready()) {\n        LOG_DBG(\"Dongle USB is ready; forcing USB transport\");\n        return ZMK_TRANSPORT_USB;\n    }\n\n    if (is_ble_ready()) {")
  string(FIND "${chary_endpoints_contents}"
              "${chary_endpoint_selection_original}" chary_endpoint_selection_pos)
  if(NOT chary_endpoint_selection_pos EQUAL -1)
    string(REPLACE "${chary_endpoint_selection_original}"
                   "${chary_endpoint_selection_usb_priority}"
                   chary_endpoints_contents
                   "${chary_endpoints_contents}")
    file(WRITE "${chary_endpoints_source}"
               "${chary_endpoints_contents}")
  else()
    string(FIND "${chary_endpoints_contents}"
                "${chary_endpoint_selection_usb_priority}"
                chary_endpoint_selection_usb_priority_pos)
    if(chary_endpoint_selection_usb_priority_pos EQUAL -1)
      message(FATAL_ERROR
              "Unable to enforce USB transport priority for the Chary46 dongle")
    endif()
  endif()
endif()

if(CONFIG_CHARY_SKIP_SPLIT_LAYOUT_SYNC)
  # ZMK 0.3 unconditionally writes the selected physical-layout index after
  # discovering every peripheral. This keyboard has only one layout, and the
  # write wedges the affected legacy-paired controller. Keep the discovered
  # handle valid because ZMK also uses it to decide that GATT discovery is
  # complete, but make the unused update itself a successful no-op.
  set(chary_split_central_source
      "${APPLICATION_SOURCE_DIR}/src/split/bluetooth/central.c")
  file(READ "${chary_split_central_source}" chary_split_central_contents)
  set(chary_layout_update_original
      "static int update_peripheral_selected_layout(struct peripheral_slot *slot, uint8_t layout_idx) {\n    if (slot->state != PERIPHERAL_SLOT_STATE_CONNECTED) {")
  set(chary_layout_update_disabled
      "static int update_peripheral_selected_layout(struct peripheral_slot *slot, uint8_t layout_idx) {\n    if (IS_ENABLED(CONFIG_CHARY_SKIP_SPLIT_LAYOUT_SYNC)) {\n        return 0; /* Chary46 has one fixed layout. */\n    }\n\n    if (slot->state != PERIPHERAL_SLOT_STATE_CONNECTED) {")
  string(FIND "${chary_split_central_contents}"
              "${chary_layout_update_original}" chary_layout_update_pos)
  if(NOT chary_layout_update_pos EQUAL -1)
    string(REPLACE "${chary_layout_update_original}"
                   "${chary_layout_update_disabled}"
                   chary_split_central_contents
                   "${chary_split_central_contents}")
    file(WRITE "${chary_split_central_source}"
               "${chary_split_central_contents}")
  else()
    string(FIND "${chary_split_central_contents}"
                "${chary_layout_update_disabled}" chary_layout_disabled_pos)
    if(chary_layout_disabled_pos EQUAL -1)
      message(FATAL_ERROR
              "Unable to disable ZMK split physical-layout synchronization")
    endif()
  endif()
endif()
