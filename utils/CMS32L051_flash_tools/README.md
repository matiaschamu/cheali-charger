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

```
openocd.exe -f tcl/target_CMS32L051_win.cfg \
            -c "script tcl/CMS32L051_flash.tcl" \
            -c "init" \
            -c "cms32_flash <path/to/firmware.bin>" \
            -c "shutdown"
```

Programming is byte-by-byte over SWD so a full 32 KB image takes ~2–3 minutes.
