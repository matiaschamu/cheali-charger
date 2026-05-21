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

# Sector-erase a single 1 KB sector at the given address.
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

# Erase firmware sectors 0x0000..0xFBFF (63 sectors of 1 KB), preserving the
# EEPROM-emulation shadow at 0xFC00..0xFFFF.
proc cms32_firmware_erase {} {
    echo "Firmware erase (preserving EEPROM at 0xFC00)..."
    for {set addr 0} {$addr < 0xFC00} {incr addr 0x400} {
        cms32_sector_erase $addr
    }
    echo "Firmware erase done"
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

proc cms32_program_bin {filename} {
    set fp [open $filename rb]
    set data [read $fp]
    close $fp
    set len [string length $data]
    echo "Programming $len bytes..."
    set addr 0
    set pct_prev -1
    for {set i 0} {$i < $len} {incr i} {
        set byte [scan [string index $data $i] %c]
        cms32_write_byte $addr $byte
        incr addr
        set pct [expr {$i * 100 / $len}]
        if {$pct != $pct_prev} {
            echo "$pct%"
            set pct_prev $pct
        }
    }
    echo "Programming done"
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
