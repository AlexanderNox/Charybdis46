Bluetooth split keyboard

Split peripherals automatically cold-reboot if a connection that was working
does not recover within 30 seconds. Recovery is armed only after the first
successful connection following boot, so powering a half while the dongle is
off does not create a reboot loop or erase its Bluetooth bond.

When upgrading from firmware using the `bond_v1` migration, flash the dongle
and both halves from the same CI artifact. The `bond_v2` migration clears stale
split bonds once on every part so they can establish matching bonds again.

The normal CI artifact intentionally does not contain `settings_reset`.
Flashing that recovery image erases Bluetooth bonds and must not be part of a
regular firmware update.

