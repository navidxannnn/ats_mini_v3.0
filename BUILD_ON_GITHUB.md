# ATS Mini – New V3.0 — Build on GitHub (Simple Guide)

## 1. Create a repository
1. Go to https://github.com/new
2. Name it anything, e.g. `ats-mini-new-v3`.
3. Choose **Public** or **Private** (both work fine with Actions).
4. Click **Create repository**. Leave it empty (no README/license needed).

## 2. Upload the project files
Easiest way (no git command line needed):
1. Extract the ZIP you received.
2. On your new repo's GitHub page, click **"uploading an existing file"**
   (or **Add file -> Upload files**).
3. Drag the WHOLE extracted folder contents in (including the hidden
   `.github` folder — make sure your file manager shows hidden files,
   otherwise the workflow won't be uploaded).
4. Scroll down, click **Commit changes**.

   > Tip: if your browser/OS hides the `.github` folder from drag-and-drop,
   > use `git` instead:
   > ```
   > git init
   > git add -A
   > git commit -m "ATS Mini New V3.0"
   > git branch -M main
   > git remote add origin https://github.com/<you>/<repo>.git
   > git push -u origin main
   > ```

## 3. Run the build
1. On your repo page, click the **Actions** tab.
2. In the left sidebar, click **"Build Release Artifact (ATS-MINI New V3.0 / OSPI)"**.
3. Click the **"Run workflow"** button (top right of the list) -> **Run workflow**.
4. Wait a few minutes. A green checkmark means it succeeded; a red X means
   it failed — click into the run and open the failing step to see the
   error (send it to me and I'll fix the source/workflow).

## 4. Download the artifact
1. Click into the finished run (the one with the green checkmark).
2. Scroll down to the **Artifacts** section.
3. Download **`ATS-MINI-New-V3.0-Firmware`** (a zip file).
4. Unzip it — you'll find:
   - `firmware.bin`, `bootloader.bin`, `partitions.bin`, `boot_app0.bin`, `merged.bin`
   - `firmware.elf`
   - `partitions.csv`
   - `FLASH_ADDRESS.txt` (exact addresses for every file)
   - `BUILD_INFO.txt` (version, board, core, library versions used)

## 5. Flash it with esptool
Install esptool if you don't have it:
```
pip install esptool
```

Connect the ATS Mini via USB, find its serial port (e.g. `COM5` on
Windows, `/dev/ttyUSB0` or `/dev/ttyACM0` on Linux, `/dev/cu.usbserial-*`
on macOS), then either:

**Option A — one merged file (simplest, always resets saved settings):**
```
esptool.py --chip esp32s3 --port PORT --baud 921600 write_flash -z 0x0 merged.bin
```

**Option B — the three separate files (may preserve settings):**
```
esptool.py --chip esp32s3 --port PORT --baud 921600 write_flash -z \
  0x0     bootloader.bin \
  0x8000  partitions.bin \
  0x10000 firmware.bin
```
(Replace `PORT` with your actual serial port. Exact addresses are also
written out in `FLASH_ADDRESS.txt` inside the artifact, generated from the
real `partitions.csv` at build time — not guessed.)

After flashing, it's a good idea to hold the encoder button while powering
the device on once, to reset preferences to defaults for the new firmware.
