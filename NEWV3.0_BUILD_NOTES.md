# ATS Mini – New V3.0 — Build Notes

This archive contains the MODIFIED SOURCE CODE only (no compiled .bin files —
see "Why no .bin files" below).

## What changed for New V3.0
- ats-mini/Common.h      : version bumped to 3.00, added FIRMWARE_EDITION "New V3.0"
- ats-mini/Utils.cpp     : getVersion() now shows "ATS-Mini New V3.0 F/W: v3.00 <date>"
- ats-mini/SplashImage.h : NEW FILE — 320x170 RGB565 splash logo (auto-generated)
- ats-mini/ats-mini.ino  : shows the splash logo at boot (no added blocking delay)
- ats-mini/Menu.h        : added CMD_RESTART
- ats-mini/Menu.cpp      : added "Restart" item in Settings menu, with a
                           Yes/No confirmation screen (defaults to "No")

Nothing else was removed or altered. All existing features (Battery, Charging
indicator, BLE, Wi-Fi, EIBI, Themes, About/System Info, etc.) are untouched.

## Why no .bin / bootloader / partition files
Building this firmware requires the Arduino ESP32 core + several third-party
libraries (TFT_eSPI, PU2CLR SI4735, Async TCP, ESP Async WebServer, NTPClient),
all downloaded from the internet. The environment used to make these source
changes has no internet access and no ESP32 build toolchain installed, so a
real .bin/bootloader/partition set could not be produced there — and just as
important, YOU need to pick the correct board profile below before compiling.

## Your board: OSPI PSRAM (confirmed)
Use the **esp32s3-ospi** profile / "OPI PSRAM" setting everywhere below.

## How to get the actual flashable files (pick ONE)

### Option A — Arduino IDE (easiest, no command line)
1. Install Arduino IDE 2.x, then add this board index URL in
   Preferences -> Additional boards manager URLs:
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
2. Boards Manager -> install "esp32" by Espressif (version 3.3.8).
3. Library Manager -> install: TFT_eSPI (2.5.43), PU2CLR SI4735 (2.1.8),
   Async TCP (3.4.10), ESP Async WebServer (3.11.0), NTPClient (3.2.1).
4. Open ats-mini/ats-mini.ino.
5. Tools -> Board -> ESP32S3 Dev Module, then set:
   - Flash Size: 8MB
   - Partition Scheme: Custom (uses ats-mini/partitions.csv)
   - PSRAM: "OPI PSRAM"   <-- your board (OSPI), confirmed
   - CPU Frequency: 80MHz, USB Mode: Hardware CDC and JTAG
6. Sketch -> Export Compiled Binary. Arduino IDE will produce, next to
   the .ino file (in a "build" subfolder):
   - ats-mini.ino.bin          (the application)
   - ats-mini.ino.bootloader.bin
   - ats-mini.ino.partitions.bin
   - ats-mini.ino.merged.bin   (all-in-one, flash this at offset 0x0)

### Option B — Use the project's own GitHub Actions build (most reliable)
This repository already ships a working CI pipeline
(.github/workflows/build.yml) that compiles BOTH the OSPI and QSPI PSRAM
variants automatically and uploads ready-to-flash binaries as artifacts.
1. Push this modified `ats-mini/` folder to your own GitHub fork/branch.
2. Go to Actions -> "Build Firmware" -> "Run workflow" (workflow_dispatch).
3. Download the artifact named "...-ospi" (matches your confirmed board) —
   it contains the exported .bin, .bootloader.bin and .partitions.bin files.
   (Ignore the "-qspi" and "-lilygo-t-embed" artifacts unless you also have
   one of those boards.)

### Option C — arduino-cli (command line)
Same idea as Option A, using the exact profile already defined in
ats-mini/sketch.yaml for your board:
    arduino-cli compile --profile esp32s3-ospi --export-binaries ats-mini

## Flashing
Once you have the four files (bootloader, partitions, boot_app0 [from the
ESP32 core], and the app .bin — or just the single merged .bin), flash with
esptool.py or the ESP Web Flasher, per docs/source/flash.md in this repo.
Preferences reset is recommended after a firmware update: hold the encoder
button while powering on.
