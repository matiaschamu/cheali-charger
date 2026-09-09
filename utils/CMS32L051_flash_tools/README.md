# CMS32L051_flash_tools

Flash tools for Cmsemicon CMS32L051 (Cortex-M0+) CPU for use with OpenOCD and an
ST-Link adapter.

The vendor has no native OpenOCD flash driver, so the flash controller (FMC at
`0x40020000`) is driven directly from a TCL script via SWD memory writes:

* `tcl/target_CMS32L051_win.cfg` — OpenOCD config: ST-Link interface + Cortex-M0+
  target (overrides the STM32F0 DAP IDCODE check, the only difference between
  the two cores' DAP IDs).
* `tcl/CMS32L051_flash.tcl` — chip erase, page program and verify procedures.

## Driver setup (Windows)

The first time you connect the ST-Link, install the **WinUSB** driver with
[Zadig](https://zadig.akeo.ie/) — without it OpenOCD reports
`LIBUSB_ERROR_NOT_SUPPORTED`.

## Flashing

### Recommended during development: preserve and verify EEPROM

The CMS32L051 port stores calibration, settings and battery profiles in the
last 1 KiB of flash (`0xFC00..0xFFFF`). The normal firmware image is limited to
`0x0000..0xFBFF`; `cms32_flash_safe` additionally saves that EEPROM region to a
file and verifies that it did not change during the firmware update.

Use a new backup filename for every important development step:

```
openocd.exe -f tcl/target_CMS32L051_win.cfg \
            -c "script tcl/CMS32L051_flash.tcl" \
            -c "init" \
            -c "cms32_flash_safe <path/to/firmware.bin> <path/to/eeprom-backup.bin>" \
            -c "shutdown"
```

The command refuses firmware images larger than `0xFC00` bytes and EEPROM
backups whose size is not exactly 1024 bytes. If EEPROM verification fails,
OpenOCD stops before resetting the target and leaves the backup on disk.

An EEPROM backup is only compatible while the firmware keeps the same EEPROM
layout/version. Do not restore an old backup after changing the EEPROM data
structures or version constants without reviewing the migration.

### Backup, verify or restore EEPROM separately

The available TCL procedures are:

```
cms32_eeprom_backup <path/to/eeprom-backup.bin>
cms32_eeprom_verify <path/to/eeprom-backup.bin>
cms32_eeprom_restore <path/to/eeprom-backup.bin>
```

`cms32_eeprom_restore` erases and rewrites `0xFC00..0xFFFF`; use it only with a
known-good, compatible 1024-byte backup. It does not touch the firmware area.

### Firmware update without creating a backup

`cms32_flash` still preserves `0xFC00..0xFFFF`, but the safe command above is
preferred while developing:

```
openocd.exe -f tcl/target_CMS32L051_win.cfg \
            -c "script tcl/CMS32L051_flash.tcl" \
            -c "init" \
            -c "cms32_flash <path/to/firmware.bin>" \
            -c "shutdown"
```

The legacy procedures program byte-by-byte. `cms32_flash_safe_word` uses the
documented 32-bit word-program mode while retaining the same EEPROM backup and
verification steps. On the tested ST-Link V2 setup at 4 MHz, a 34,664-byte
image completed in about 56 seconds instead of about 210 seconds.
