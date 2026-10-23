# RoboArm Writer

RoboArm Writer is an ESP8266-controlled, six-joint robot arm that draws short text on paper with a marker. Its dashboard is served directly by the ESP8266: connect to the arm's Wi-Fi network, open `http://192.168.4.1/`, calibrate the arm, and send text to draw.

The dashboard is embedded in the firmware; it does not need a separate web server, cloud service, or internet connection. Calibration and drawing settings are saved in the ESP8266's LittleFS storage.

## Before you start

- **This project targets an ESP8266 NodeMCU (ESP-12E), not an ESP32.** The firmware includes ESP8266-specific libraries.
- Assemble the arm and mount a marker before calibration. Keep the pen above the paper while checking motion.
- Use an adequately rated, regulated **5–6 V servo supply** (the firmware comments recommend 5 A or more). Servo current depends on the servos and load.
- Do not power the servos from the NodeMCU's 3.3 V pin or USB. Connect the servo supply ground, PCA9685 ground, and NodeMCU ground together.
- Keep hands, cables, and loose items clear of the arm. The dashboard's **STOP** button stops commanded motion; it is not a physical emergency stop. **Relax** cuts servo holding pulses and the arm may drop, so support it before using Relax or disconnecting servo power.
- Once all required servo references are saved, the arm enables the servos and moves to its saved park pose at power-on. Keep the movement area clear during startup.

## Hardware and wiring

The firmware uses a PCA9685 16-channel PWM driver. The default servo channels are:

| PCA9685 channel | Joint |
| --- | --- |
| CH0 | Base |
| CH1 | Shoulder |
| CH2 | Elbow |
| CH3 | Wrist rotate |
| CH4 | Wrist pitch |
| CH5 | Claw (optional; off by default) |

Connect the PCA9685 logic and I2C signals as follows:

| NodeMCU | PCA9685 |
| --- | --- |
| D2 / GPIO4 | SDA |
| D1 / GPIO5 | SCL |
| 3V3 | VCC (logic power) |
| GND | GND |

Power the servos through the PCA9685's servo-power input from the separate 5–6 V supply. **Join all grounds. Do not connect the external servo voltage to NodeMCU 3V3.** Check your PCA9685 board's own input labels before connecting power.

## Arduino IDE setup and upload

1. Install the **ESP8266 boards platform** from the Boards Manager in Arduino IDE.
2. Install these libraries from Library Manager:
   - **Adafruit PWM Servo Driver Library**
   - **ArduinoJson** version 7
3. In this project folder, make your private settings file from the example only if you do not already have one:

   ```powershell
   if (-not (Test-Path config.h)) { Copy-Item config.example.h config.h }
   ```

4. Edit `config.h`:
   - Leave `WIFI_USE_HOME` at `0` to start the arm's own Wi-Fi hotspot (recommended for first setup).
   - Set `AP_SSID` and replace `AP_PASS` with a unique Wi-Fi password of at least 8 characters.
   - Only if you want the arm to join your router, set `WIFI_USE_HOME` to `1` and enter your Wi-Fi name and password in `WIFI_SSID` and `WIFI_PASS`. If the connection fails, the firmware starts its own hotspot after 15 seconds.
5. Open `RoboArm_Writer.ino` in Arduino IDE. Select **NodeMCU 1.0 (ESP-12E Module)** in **Tools > Board** and a 4 MB flash-size option with a LittleFS partition (the sketch's original setting is `4MB (FS:2MB OTA:~1019KB)`). Select the correct serial port and upload.
6. Open **Tools > Serial Monitor** at **115200 baud**. In hotspot mode, the monitor reports the hotspot name and dashboard address, but not its password; use the password you set in `config.h`.

The project folder and `.ino` file have matching names, as required by Arduino IDE. No separate LittleFS data upload is needed for a clean first setup.

### Keeping credentials out of GitHub

`config.h` is in `.gitignore`. Keep actual router credentials and your chosen access-point password in that local file only. Commit `config.example.h`, which contains blank router credentials and a placeholder access-point password. Do not replace the example's placeholder with your real password.

The dashboard has no separate login. Anyone who can join the arm's hotspot can issue motion commands. If you enable home-Wi-Fi mode, devices on that network may also be able to reach the dashboard; use a trusted network.

## Connect to the dashboard

With the default hotspot mode:

1. Power on the NodeMCU and wait for it to finish starting.
2. On your phone or computer, join the Wi-Fi network named by `AP_SSID` (default: `RoboArm-Writer`) using the password in your local `config.h`.
3. If your device says the network has no internet, choose to stay connected.
4. Open **`http://192.168.4.1/`** in a browser. Include `http://`; the dashboard is served locally by the arm.

In home-Wi-Fi mode, use the local IP printed to Serial Monitor instead. The `roboarm.local` name may work if your device supports mDNS.

## First calibration

Make sure the arm can move freely, keep a hand near the servo power switch, and calibrate slowly. The firmware leaves servos relaxed at startup until required servo references are saved. A servo can move suddenly when first held; support the arm and keep clear of pinch points.

### 1. Servo calibration

Open **2. Servo calibration**. Calibrate one servo at a time:

1. Use **Hold** to energize the selected servo. It may move to its current pulse position.
2. Use the small nudge buttons to bring the joint to the **Ref A** pose shown on the card, then select **Save as Ref A**.
3. Move to the **Ref B** pose shown on the card and select **Save as Ref B**.
4. Open the limits section. Move toward each mechanical end gradually, leaving clearance before a hard stop or cable tension, and save the minimum and maximum limits.
5. Repeat for the base, shoulder, elbow, wrist rotate, and wrist pitch. The claw is not required unless you enable it in the arm settings.

Do not force a joint against its frame or let a servo buzz/stall. If a reference pose is not possible on your arm, adjust that reference angle on the card to match a reachable pose before saving it.

### 2. Arm size and drawing settings

Open **3. Arm size**. Measure the physical arm in millimetres, from pivot centre to pivot centre:

- `H`: table surface to shoulder pivot.
- `L1`: shoulder pivot to elbow pivot.
- `L2`: elbow pivot to wrist-pitch pivot.
- `L3`: wrist-pitch pivot to the marker tip.
- `d0`: forward/back offset from the base axis to the shoulder pivot.

Set whether the claw is fitted, then select **Save arm and drawing settings**. Start with a small writing area and conservative motion settings. Geometry values must match your actual arm; the defaults are only starting values.

### 3. Writing point

Open **4. Writing point** with paper fixed flat under the marker:

1. Select **Go to center**. The pen moves to the writing-area centre, above the paper.
2. If necessary, adjust the pen's X/Y position with the jog controls.
3. Choose a small step size. Use **Arm down** until the marker just touches the paper; save this as the **writing point**.
4. Select **Go to bottom point**. The arm moves 25 mm closer to the base. Lower the pen until it just touches, then save the **bottom point**. These two points let the firmware compensate for the paper plane along the writing direction.
5. Use **Test square** and **Corner check** from the Draw tab to check reach and pen contact. Stop if the arm binds or the marker presses too hard.

The app saves calibration and settings in LittleFS. Avoid erasing or formatting the ESP8266 filesystem if you want to keep them.

## Write text

1. Open **1. Draw** and enter text (up to 40 characters).
2. Set letter height and gap, then select **Preview**. Unreachable points are highlighted before motion starts.
3. With a pen and paper in place, select **Draw it**. Use **STOP** to stop a drawing if needed.

Supported characters are `A-Z`, `0-9`, spaces, `-`, `.`, `!`, and `?`. Lowercase letters are drawn as uppercase. Long text wraps by words when possible; the writing-area dimensions and reach limit what fits. Use **Park** to return to the saved park pose.

## Configuration and project files

- `RoboArm_Writer.ino` — ESP8266 Wi-Fi, HTTP API, setup, and main loop.
- `config.example.h` — safe template for local Wi-Fi and hardware configuration.
- `config.h` — your local settings; intentionally ignored by Git.
- `settings.h` — calibration, geometry, and LittleFS persistence.
- `kinematics.h` — inverse and forward kinematics.
- `motion.h` — servo output and motion queue.
- `paths.h` — text paths and drawing jobs.
- `font.h` — single-line drawing font.
- `web_page.h` — embedded dashboard.

## Upload this project to GitHub

1. Create a new GitHub repository. Do not add a separate README or `.gitignore` there if you already have these files locally.
2. If this folder is not already a Git repository, initialize it. Then, from PowerShell in this folder, stage and review the project:

   ```powershell
   git init -b main
   git add .
   git status
   ```

3. Check the status before committing. `config.h` must **not** appear in the staged files. If it does, stop and remove it from the index before committing.
4. Commit and connect the GitHub remote:

   ```powershell
   git commit -m "Prepare RoboArm Writer project"
   git remote add origin https://github.com/YOUR-USERNAME/YOUR-REPOSITORY.git
   git push -u origin main
   ```

If you already initialized Git or created a remote, do not repeat the corresponding setup command. Never add your real `config.h` or credentials to a commit.
