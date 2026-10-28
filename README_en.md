**Read this in other languages: [English](README_en.md) | [中文](README.md)**

---

# OpenThermal System (v3.0.0) - User Manual

> **Current Version**: v3.0.0  
> OpenThermal is a high-performance infrared thermal imaging and temperature measurement system. It supports **wireless transmission** as well as **Type-C wired transmission**, offering a versatile multi-platform client ecosystem: **cross-platform LAN browser-based online viewing (zero installation)**, a dedicated **Windows desktop client**, and an **installable Android mobile APK** for diverse thermal diagnostics across PCs, smartphones, and tablets.

---

## Software Packages & Application Links

This release package comes bundled with client applications and the production flashing tool. Click the links below to open or install directly:

| Platform / Tool | File Path & Direct Link | Description |
| :--- | :--- | :--- |
| 💻 **Windows Desktop** | [`windows/OpenThermal.exe`](windows/OpenThermal.exe) | Standalone desktop application, ready to use (includes full [`windows/`](windows/) runtimes) |
| 📱 **Android Mobile** | [`android/OpenThermal.apk`](android/OpenThermal.apk) | Universal Android APK (architecture-specific packages in [`android/`](android/)) |
| 🛠️ **Firmware Flashing Tool** | [`烧录工具.exe`](烧录工具.exe) | Single-file production tool for flashing, key backup, and system recovery |
| 🌐 **LAN Online Web Console** | `http://openthermal.local` or `http://192.168.4.1` | Zero installation; accessible from any modern browser on the same local network |

---

## Transmission Modes & Platform Support

- **Connection & Transmission**:
  - **Wireless Transmission (Wi-Fi)**: Direct wireless connection via the camera's built-in AP hotspot, or integration into a local Wi-Fi router for local area network (LAN) streaming.
  - **Wired Transmission (Type-C USB)**: High-speed, low-latency wired connection to a PC via Type-C data cable, or direct plug-and-play connection to an Android phone via Type-C OTG cable.
- **Cross-Platform Client Ecosystem**:
  - **LAN Web Online Viewing (Cross-Platform Zero-Installation)**: Open a modern web browser on any device (Windows, macOS, Linux, iOS, Android) to stream live thermal video without installing any third-party app.
  - **Windows Desktop Client**: Standalone desktop application with support for both high-speed USB wired streaming and wireless Wi-Fi connections.
  - **Android Mobile App (APK)**: Installable APK supporting plug-and-play USB-OTG direct tethering and Wi-Fi streaming on Android phones and tablets.

---

## Key Features

1. **High-Definition Thermal Smoothing**:
   - Incorporates multiple interpolation algorithms (with `B-Spline` recommended by default, alongside 12 selectable filter kernels including `Lanczos3`, `Gaussian`, etc.) to smoothly resample the sensor array into a high-resolution display with natural gradient transitions.
2. **Multi-Modal Color Palettes**:
   - Offers 14 industrial and medical colormaps (including `Ironbow`, `Rainbow`, `Classic`, `White-Hot`, `Black-Hot`, `Medical`, etc.) with palette inversion support to accommodate diverse ambient environments.
3. **Automated Spot Tracking & Crosshairs**:
   - Automatically tracks and marks the **Maximum Temperature** (red crosshair), **Minimum Temperature** (blue crosshair), and **Center Fixed Point** (green crosshair), alongside real-time full-frame average temperature readouts.
4. **Custom Fixed-Point Spot Metering & Thermal Trend Curves**:
   - Click or drag anywhere on screen to place a temperature spot metering pin, measuring the precise real-time temperature of any coordinate.
   - Built-in **continuous temperature graphing** plots real-time thermal trends over time, with support for clearing data and exporting historical temperature logs in CSV format.
5. **Adaptive Full Dynamic Range Mapping (Auto Range)**:
   - Dynamically maps the full false-color gradient across the real-time maximum and minimum temperatures in the frame, ensuring subtle thermal differences are highlighted with rich contrast.
6. **Surface Emissivity Calibration**:
   - Adjustable emissivity from 0.01 to 1.00 provides accurate temperature measurement across reflective metals, human skin, matte coatings, and coarse surfaces.
7. **Multi-Unit Temperature Switching**:
   - One-click switching between four international temperature units: **Celsius (°C)**, **Fahrenheit (°F)**, **Kelvin (K)**, and **Rankine (°Ra)**.
8. **Image Rotation & Mirroring**:
   - Supports horizontal mirror, vertical mirror, and 90°/180°/270° orientation rotation to adapt to any physical mounting or hand-held orientation.
9. **Denoising & Sensor Calibration**:
   - **Kalman Filtering**: Dampens temporal sensor noise to eliminate fluctuating temperature digits.
   - **Gamma Correction**: Adjusts the dynamic contrast range to reveal subtle details in cold or overheated zones.
   - **Sensor Uniformity Calibration**: Used to eliminate uneven background patterns. **Calibration Method**: Cover the front of the lens with a flat object of uniform temperature (such as cardboard, a tabletop, or a wall), then click "传感器一致性校准" (Sensor Calibration) in [System Settings] to calibrate the image with one click.
   - **Slope & Intercept Fine-Tuning**: Supports micro-adjustments to the temperature slope and intercept offset for high-precision linear calibration.
10. **Touchscreen & Gesture Shortcuts**:
    - **[Mobile App & Web Browser Client]**:
      - **Vertical Swipe (Up / Down)**: Quickly cycle through false-color palettes (swipe down for the next palette, swipe up for the previous one), with instant on-screen HUD pill notifications.
      - **Horizontal Swipe (Left / Right)**: Seamlessly cycle through spatial resampling filters (swipe right for the next filter, swipe left for the previous one) to compare visual clarity among 12 algorithms (`B-Spline`, `Lanczos3`, `Gaussian`, etc.) in real time.
      - **Single Tap / Click Anywhere**: Instantly reposition the fixed-point spot metering crosshair to the touched location, reading and displaying its exact coordinate temperature in the bottom metric card.
    - **[Handheld On-Screen Hardware Models]**:
      - **Swipe Up / Down**: Quick cycle through false-color palettes.
      - **Swipe Left / Right**: Toggle the sidebar docked position between the left and right sides of the screen for convenient one-handed operation.
      - **Single Tap**: Tap anywhere on the thermal image to reposition the spot metering pin.
      - **Long Press**: Press and hold anywhere on screen to directly open the [System Settings] menu.
      - **Short Press Power Button**: Freeze / unfreeze the live thermal stream (pause and resume).
11. **Screen Backlight Adjustment (On-Screen Models)**:
    - Stepless backlight brightness control (1% ~ 100%) to adapt to varying indoor and outdoor ambient light conditions.
12. **Multi-Language & Modern UI Themes**:
    - Provides multi-language interface switching (Simplified Chinese, English, etc.).
    - Supports modern Light and Dark theme switching.
13. **High-Definition Image Capture & MP4 Video Recording**:
    - **One-Click Snapshot**: Capture crystal-clear thermal frames with temperature overlays and crosshairs, automatically saved as high-resolution PNG images to your device album or downloads folder.
    - **Real-Time MP4 Video Recording**: Start dynamic video recording with one click to log live thermal variations over time. Upon stopping, it automatically encodes and saves a standard MP4 video file, perfect for diagnostic reports, research logging, and field verification.
14. **Hardware-Free Offline Demo Mode**:
    - Both the mobile app and PC client feature built-in dynamic mathematical thermal field models. Even without physical camera hardware connected, clicking "Demo Mode" allows instant evaluation of real-time thermal streams, gesture palette switching, and resampling filter algorithms.

---

## Operating Instructions & Client Setup

The system supports three primary client interfaces:

### 1. LAN Web Online Viewing

Ideal for PCs, Macs, tablets, iPhones, iPads, and Android devices running a modern web browser (zero installation required).

1. **Power On & Connect**:
   - Power on the thermal device. It automatically creates a Wi-Fi access point, typically named `INFRARED-xxxxxx` (open network, no password required).
   - Connect your smartphone, tablet, or PC to this Wi-Fi network.
2. **Open Online Viewer**:
   - Open a browser and navigate to:
     - `http://192.168.4.1`
     - Or the mDNS domain: `http://openthermal.local`
   - With the built-in captive DNS portal, accessing any URL will automatically redirect to the console. The live thermal image will stream immediately, allowing palette changes, filter tweaks, and emissivity adjustments.
3. **Connect to a Local Router (Optional LAN Access)**:
   - To integrate the device into your home/office Wi-Fi network, navigate to [System Settings] -> [Network & Connections / Wi-Fi] within the web console.
   - Select your target network from the scan list, enter the Wi-Fi password, and click Save.
   - Once connected to your local router, any device on the same local network can access the thermal camera via its assigned IP address or `http://openthermal.local`.
4. **Extended Management Features**:
   - Expand the sidebar navigation drawer in the top-left corner to access **File Manager** (browse and download onboard LittleFS storage data), **Temperature Analytics**, and custom **Boot Splash Screen** generation for on-screen models.

---

### 2. Windows Desktop Client

Provides a responsive desktop viewing and diagnostic interface.

1. **Launch**: Double-click to run [`windows/OpenThermal.exe`](windows/OpenThermal.exe) located under the [`windows/`](windows/) directory.
2. **USB Wired Mode (Recommended)**:
   - Connect the camera to your computer using a Type-C USB cable.
   - In the connection panel, select the detected COM port, keep the default baud rate (921600), and click **Connect**.
3. **Wi-Fi Wireless Mode**:
   - Ensure the PC is connected to the camera's Wi-Fi hotspot or is on the same local network.
   - Select Network mode, enter the device IP address (default is `192.168.4.1` when connected directly to its hotspot), and click **Connect**.

---

### 3. Android Mobile Client

Portable inspection directly on Android phones and tablets.

1. **Install APK**:
   - Install [`android/OpenThermal.apk`](android/OpenThermal.apk) on your Android device (architecture-specific APKs can be found in the [`android/`](android/) folder).
2. **Type-C OTG Wired Connection (Recommended)**:
   - Connect the camera to your phone's charging port using a Type-C to Type-C OTG cable.
   - When the system prompts for USB permission, check the authorization box and accept; open the app to immediately view the live thermal stream.
   - *Note: If your phone does not recognize the connected camera, ensure that the [OTG] feature is enabled in your phone's system settings.*
3. **Wi-Fi Wireless Mode**:
   - Connect your phone to the camera's Wi-Fi hotspot, then launch the app to stream wirelessly.
4. **Hardware-Free Offline Demo Mode**:
   - If you do not have the thermal camera connected yet, tap "Demo Mode" directly on the welcome screen to launch the built-in mathematical thermal model and experience the live stream and algorithm controls.

---

## Recommended Parameter Settings

| Parameter | Recommended Value | Description & Use Cases |
| :--- | :--- | :--- |
| **Palette** | `Ironbow` / `Rainbow` | Most intuitive for general thermal diagnostics. Use `Rainbow` for detecting subtle temperature variations, and `White-Hot` for dark environments. |
| **Filter** | `B-Spline` | Provides the best balance of smoothness and edge clarity. Use `Lanczos3` when sharper thermal hotspot boundaries are needed. |
| **Emissivity** | By material (see right) | Adjust according to the target surface material:<br>• Human skin / Water / Ice: `0.98`<br>• Matte paint / Rubber / Plastics: `0.90 ~ 0.95`<br>• Oxidized / Rough metals: `0.50 ~ 0.70`<br>• Polished metals (copper, aluminum): `0.10 ~ 0.20` |
| **Kalman Filter** | Enabled (50) | Increase strength when ambient temperature differences are subtle or when pixels flicker due to thermal noise. |
| **Gamma Contrast** | `1.0` (Default) | Lower to `0.7` to discern background details around hot targets; raise to `1.4` to accentuate faint warm anomalies. |

---

## Safety & Handling Precautions

1. **Avoid Extreme Radiation Sources**:
   - **Never aim the lens directly at high-intensity radiation sources such as the sun, welding arcs, or high-power industrial lasers.** Direct exposure will cause permanent and irreversible damage to the thermopile sensor array!
2. **Lens Care**:
   - The infrared optical lens features precision optical coatings. Avoid touching the lens with fingers or abrasive objects. Clean using an optical blower or dedicated lens cleaning paper only.
3. **Distance & Field of View**:
   - Measurement accuracy depends on target size relative to distance. Ensure the target object covers a sufficient number of pixels on screen for precise temperature readings.

---

## DIY

This chapter is intended for makers and geeks interested in using the standalone factory flashing tool [`烧录工具.exe`](烧录工具.exe), backing up device keys, reflashing official firmware after trying custom DIY builds, and obtaining hardware development schematics.

### 1. Flashing Tool Guide

Used for flashing DIY firmware, restoring factory official firmware, or performing device maintenance:

1. **Device Connection**:
   - Connect the thermal imager to your PC using a Type-C USB cable.
   - If the computer does not recognize the serial port automatically, verify that the USB serial driver is installed, or open Windows [Device Manager] to check the port (COM & LPT) status.
2. **Launch Flashing Tool**:
   - Double-click to run [`烧录工具.exe`](烧录工具.exe) in the release root directory.
3. **One-Click Flashing**:
   - Select the target firmware binary you wish to flash.
   - Click the **"一键烧录"** (One-Click Flash) button at the bottom to automatically program the full firmware and filesystem.
4. **Full Flash Erase (Optional)**:
   - The tool provides a red **"擦除芯片"** (Erase Chip) button to completely erase and clear the entire flash chip.

---

### 2. Device Key Backup & Storage Location

Each device comes from the factory with unique hardware authentication and calibration data stored in the chip's internal NVS flash partition (one unique key per unit). Before flashing custom DIY firmware, backing up this data is strongly recommended:

1. **How to Back Up the Key**:
   - Connect the device to your PC and click the green **"备份密钥"** (Backup Key) button in [`烧录工具.exe`](烧录工具.exe).
   - The tool automatically reads the unique hardware key and calibration parameters from the chip and completes the backup.
2. **Where the Backup Key is Stored**:
   - The generated backup file is named **`nvs_backup.bin`**.
   - This file is saved **in the exact same directory as [`烧录工具.exe`](烧录工具.exe)** (the tool's current folder). Please keep this file in a safe place.

---

### 3. How to Reflash & Restore Factory Firmware After DIY

If you have flashed custom or third-party DIY firmware and want to restore the official factory system:

1. **Place the Backup Key**:
   - Ensure your previously backed-up **`nvs_backup.bin` file is located in the same directory as [`烧录工具.exe`](烧录工具.exe)**.
2. **Automatic Detection & Autofill**:
   - Launch [`烧录工具.exe`](烧录工具.exe) and connect the device.
   - The flashing tool automatically detects `nvs_backup.bin` in its directory, verifies the cryptographic fingerprint within seconds, and autofills the original factory license and calibration parameters—no manual input required.
3. **Reflash**:
   - Select your hardware model and click **"一键烧录"** to seamlessly restore the device to the official factory firmware.

---

### 4. Hardware Schematics

If you need in-depth details on hardware peripheral pinouts, circuit designs, or wish to develop custom hardware expansions, **hardware schematics and related development resources are available upon request by contacting customer service**.

---

## Historical Archive

Legacy firmware images and documentation are preserved in the `archive/v2.x.x/` directory for reference.

---

## Where to Buy

Please scan the QR code below or click the link below to go directly to the official store:

<div align="center">
  <img src="./assets/shop.png" width="35%" alt="Official Taobao Store QR Code" />
  <p><a href="https://shop237247974.taobao.com/" target="_blank">Click here to go directly to the link 🚀</a></p>
</div>

---

## Open Source & Licensing Statement

This project is actively under development. The full source code will be officially open-sourced once firmware stability milestones are achieved (scheduled for release within 2027, or immediately open-sourced upon reaching 300 Stars 😏).
