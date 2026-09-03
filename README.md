Bluetooth split keyboard

Split peripherals automatically cold-reboot if a connection that was working
does not recover within 8 seconds. Recovery is armed only after the first
successful connection following boot, so powering a half while the dongle is
off does not create a reboot loop or erase its Bluetooth bond.

Release dongle artifacts are built without USB debug logging. The right-hand
PMW3610 input rate is capped at a 16 ms interval so its separate X/Y split
notifications cannot flood the shared BLE transport.

If the two sides of a split bond contain different encryption keys, firmware
detects `PIN_OR_KEY_MISSING`, removes only that failed peer bond, and reconnects
for a clean pairing. There is no delayed startup bond wipe or manual reset step.

The normal CI artifact intentionally does not contain `settings_reset`.
Flashing that recovery image erases Bluetooth bonds and must not be part of a
regular firmware update.

