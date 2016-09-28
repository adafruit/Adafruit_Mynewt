# LPC43xx Mynewt MCU Support

This repository contains the core support files for the ARM Cortex M4F core on
the LPC43xx MCU from NXP Semiconductors.

## LPC43xx ARM Cortex Cores

The LPC43xx series contains either two or three ARM Cortex M cores, depending
on the model used.

The master core that runs at startup is the ARM Cortex M4F, and there are up to
two additional ARM Cortex M0 cores available. Resources must be allocated to a
specific core, meaning some planning is required during the design phase if
multiple cores will be used.

### ARM Cortex M4F Core (M4F)

Execution always starts in this core on the LPC43xx.

- Flash Memory Base Location: 0x1A000000
- Total Flash Memory Size: **512KB**
- Reserved SRAM Memory: RamLoc40 (0x10080000), RamAHB32 (0x20000000)
- Total SRAM Memory: **72KB** (40KB + 32KB)

### ARM Cortex M0 Core (M0A)

The main ARM Cortex M0 core on the LPC43xx. This additional core must be
started by the primary M4F core at startup, assuming that valid code is found
on the M0A core.  

- Flash Memory Base Location: 0x1B000000
- Total Flash Memory Size: **512KB**
- Reserved SRAM Memory: RamLoc32 (0x10000000), RamAHB16 (0x20008000)
- Total SRAM Memory: **48KB** (32KB + 16KB)

### Secondary ARM Cortex M0 Core (M0B)

Certain LPC43xx models contain a second (limited) M0 core.

The M0B core has no associated flash memory and code on this core must be
run entirely from SRAM.

For the moment this optional third core will be ignored, but should be kept in
mind during development.

# Resource Allocation

### SRAM

The SRAM blocks available on the LPC43xx are allocated via the following scheme:

| SRAM Block | M4F | M0A | M0B | Other |
|------------|-----|-----|-----|-------|
| RamLoc32   |     |  x  |     |       |
| RamLoc40   |  x  |     |     |       |
| RamAHB32   |  x  |     |     |       |
| RamAHB16   |     |  x  |     |       |
| RamETB16   |     |     |     | TRACE |

**Note**: The 16KB ETM memory block (RamAHB_ETB16) should be reserved for trace
output for now, but may eventually be freed up for use in either the M4F or M0
cores in the future.

### Peripherals

| Peripheral | M4F | M0A | M0B | Other |
|------------|-----|-----|-----|-------|
| TBD        |     |     |     |       |

# Related Links

- USB DFU: https://github.com/mossmann/hackrf/wiki/LPC43xx-USB-DFU-Notes
- General LPC43xx code to review: https://github.com/mossmann/hackrf/wiki/Firmware-Development-Setup ... **consider seriously using this branch of libopencm3 as a starting point since they have done a lot of work and it's open source**
- Previous 43xx code including dual-core: https://github.com/microbuilder/LPC43xx_CodeBase/tree/master/m4
