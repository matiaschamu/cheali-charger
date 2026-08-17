# CMS32L051 flash programmer script for OpenOCD
# FMC base: 0x40020000
#   FLSTS   +0x00  (bit0=OVF, bit2=EVF)
#   FLOPMD1 +0x04
#   FLOPMD2 +0x08
#   FLERMD  +0x0C  (0x08=chip erase, 0x10=sector erase, 0x00=program)
#   FLPROT  +0x20  (0xF1=unlock, 0xF0=lock)

set FMC_BASE    0x40020000
set FMC_FLSTS   [expr {$FMC_BASE + 0x00}]
set FMC_FLOPMD1 [expr {$FMC_BASE + 0x04}]
set FMC_FLOPMD2 [expr {$FMC_BASE + 0x08}]
set FMC_FLERMD  [expr {$FMC_BASE + 0x0C}]
set FMC_FLPROT  [expr {$FMC_BASE + 0x20}]

set CMS32_EEPROM_ADDR 0xFC00
set CMS32_EEPROM_SIZE 0x0400

proc fmc_wait_ovf {} {
    global FMC_FLSTS
    set tries 0
    while 1 {
        set s [mrw $FMC_FLSTS]
        if {($s & 0x1) != 0} {
            mww $FMC_FLSTS 0x1
            return 0
        }
        incr tries
        if {$tries > 100000} {
            echo "FMC timeout!"
            return 1
        }
    }
}

proc cms32_chip_erase {} {
    global FMC_FLERMD FMC_FLPROT FMC_FLOPMD1 FMC_FLOPMD2
    echo "Chip erase..."
    mww $FMC_FLERMD  0x08
    mww $FMC_FLPROT  0xF1
    mww $FMC_FLOPMD1 0x55
    mww $FMC_FLOPMD2 0xAA
    mww 0x00000000   0xFFFFFFFF
    fmc_wait_ovf
    mww $FMC_FLERMD 0x00
    mww $FMC_FLPROT 0xF0
    echo "Chip erase done"
}

# CMS32L051 flash sector size, in bytes. Empirically 256 (0x100); using a
# larger step leaves gaps unerased and subsequent byte-program cannot turn
# bits back on (flash bits can only go 1→0 between erases).
set CMS32_SECTOR_SIZE 0x100

# Sector-erase a single sector starting at the given address.
proc cms32_sector_erase {addr} {
    global FMC_FLERMD FMC_FLPROT FMC_FLOPMD1 FMC_FLOPMD2
    mww $FMC_FLERMD  0x10
    mww $FMC_FLPROT  0xF1
    mww $FMC_FLOPMD1 0x55
    mww $FMC_FLOPMD2 0xAA
    mww $addr        0xFFFFFFFF
    fmc_wait_ovf
    mww $FMC_FLERMD 0x00
    mww $FMC_FLPROT 0xF0
}

# Erase firmware sectors 0x0000..0xFBFF, preserving the EEPROM-emulation
# shadow that lives in the last 1 KB (0xFC00..0xFFFF).
proc cms32_firmware_erase {} {
    global CMS32_SECTOR_SIZE
    echo "Firmware erase (preserving EEPROM at 0xFC00)..."
    for {set addr 0} {$addr < 0xFC00} {incr addr $CMS32_SECTOR_SIZE} {
        cms32_sector_erase $addr
    }
    echo "Firmware erase done"
}

proc cms32_eeprom_erase {} {
    global CMS32_SECTOR_SIZE CMS32_EEPROM_ADDR CMS32_EEPROM_SIZE
    set end [expr {$CMS32_EEPROM_ADDR + $CMS32_EEPROM_SIZE}]
    echo "EEPROM shadow erase..."
    for {set addr $CMS32_EEPROM_ADDR} {$addr < $end} {incr addr $CMS32_SECTOR_SIZE} {
        cms32_sector_erase $addr
    }
    echo "EEPROM shadow erase done"
}

proc cms32_write_byte {addr val} {
    global FMC_FLPROT FMC_FLOPMD1 FMC_FLOPMD2
    mww $FMC_FLPROT  0xF1
    mww $FMC_FLOPMD1 0xAA
    mww $FMC_FLOPMD2 0x55
    mwb $addr $val
    fmc_wait_ovf
    mww $FMC_FLPROT 0xF0
}

# Same as cms32_write_byte but assumes FLPROT is already unlocked (0xF1).
# Used in tight programming loops to halve the per-byte SWD overhead.
proc cms32_write_byte_fast {addr val} {
    global FMC_FLOPMD1 FMC_FLOPMD2
    mww $FMC_FLOPMD1 0xAA
    mww $FMC_FLOPMD2 0x55
    mwb $addr $val
    fmc_wait_ovf
}

proc cms32_program_bin_at {filename base_addr max_len} {
    global FMC_FLPROT
    if {![file exists $filename]} {
        error "File not found: $filename"
    }
    set fp [open $filename rb]
    set data [read $fp]
    close $fp
    set len [string length $data]
    if {$len > $max_len} {
        error [format "Image is too large: %d bytes (limit %d)" $len $max_len]
    }
    echo [format "Programming %d bytes at 0x%08X..." $len $base_addr]
    set addr $base_addr
    set pct_prev -1
    set skipped 0
    mww $FMC_FLPROT 0xF1    ;# unlock once for the whole batch
    for {set i 0} {$i < $len} {incr i} {
        set byte [scan [string index $data $i] %c]
        if {$byte == 0xFF} {
            ;# Just-erased flash already reads 0xFF; skip the program cycle.
            incr skipped
        } else {
            cms32_write_byte_fast $addr $byte
        }
        incr addr
        set pct [expr {$i * 100 / $len}]
        if {$pct != $pct_prev} {
            echo "$pct%"
            set pct_prev $pct
        }
    }
    mww $FMC_FLPROT 0xF0
    echo "Programming done ($skipped of $len bytes were 0xFF, skipped)"
}

proc cms32_program_bin {filename} {
    global CMS32_EEPROM_ADDR
    cms32_program_bin_at $filename 0x00000000 $CMS32_EEPROM_ADDR
}

proc cms32_check_eeprom_file {filename} {
    global CMS32_EEPROM_SIZE
    if {![file exists $filename]} {
        error "EEPROM backup not found: $filename"
    }
    set len [file size $filename]
    if {$len != $CMS32_EEPROM_SIZE} {
        error [format "Invalid EEPROM backup size: %d bytes (expected %d)" $len $CMS32_EEPROM_SIZE]
    }
}

proc cms32_eeprom_backup {filename} {
    global CMS32_EEPROM_ADDR CMS32_EEPROM_SIZE
    halt
    dump_image $filename $CMS32_EEPROM_ADDR $CMS32_EEPROM_SIZE
    cms32_check_eeprom_file $filename
    echo "EEPROM backup saved to $filename"
}

proc cms32_eeprom_verify {filename} {
    global CMS32_EEPROM_ADDR
    cms32_check_eeprom_file $filename
    halt
    verify_image $filename $CMS32_EEPROM_ADDR bin
    echo "EEPROM backup matches flash"
}

proc cms32_eeprom_restore {filename} {
    global CMS32_EEPROM_ADDR CMS32_EEPROM_SIZE
    cms32_check_eeprom_file $filename
    halt
    cms32_eeprom_erase
    cms32_program_bin_at $filename $CMS32_EEPROM_ADDR $CMS32_EEPROM_SIZE
    verify_image $filename $CMS32_EEPROM_ADDR bin
    echo "EEPROM restore done - resetting"
    reset run
}

proc cms32_flash {binfile} {
    halt
    cms32_firmware_erase
    cms32_program_bin $binfile
    echo "Verifying..."
    verify_image $binfile 0x0 bin
    echo "Done - resetting"
    reset run
}

# Recommended development workflow: snapshot the EEPROM shadow, update only
# the firmware area, verify both images, then reset. If EEPROM verification
# fails OpenOCD aborts before reset and the backup remains available for an
# explicit cms32_eeprom_restore.
proc cms32_flash_safe {binfile eepromfile} {
    halt
    cms32_eeprom_backup $eepromfile
    cms32_firmware_erase
    cms32_program_bin $binfile
    echo "Verifying firmware..."
    verify_image $binfile 0x0 bin
    echo "Verifying preserved EEPROM..."
    cms32_eeprom_verify $eepromfile
    echo "Done - resetting"
    reset run
}
