Bluetooth split keyboard

Split peripherals automatically cold-reboot if a connection that was working
does not recover within 30 seconds. Recovery is armed only after the first
successful connection following boot, so powering a half while the dongle is
off does not create a reboot loop or erase its Bluetooth bond.

