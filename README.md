# Ps5Downloadly

**Ps5Downloadly** is a high-speed direct download manager built for jailbroken PlayStation 5 consoles running **kstuff** and **ShadowMountPlus** (Firmware 3.xx - 4.xx - 5.xx+).

Designed for unthrottled downloads from direct CDNs and debrid services (such as Real-Debrid) using shortened **TinyURL** links. The application requires only the shortcode (e.g., `XsDrt12`), automatically resolves redirects, detects target filenames, and streams files directly to console storage with resume support.

---

## Features

- **Minimalist Shortcode Input:** Prompts only for the code after the slash (e.g., `XsDrt12` for `https://tinyurl.com/XsDrt12`).
- **Direct Line-Speed Downloads:** Built for direct and unrestricted links without third-party scrapers, captchas, or queue limits.
- **Resumable Transfers:** Supports HTTP `Range: bytes=` requests. Interrupted or paused downloads automatically resume from where they stopped.
- **Real-Time Monitoring:** Displays a live progress bar, active download speed (MB/s), transferred vs. total size, and estimated time remaining (ETA).
- **Flexible Storage Paths:** Defaults to `/data/homebrew/`, with quick selection for `/data/`, `/user/home/`, or external USB drives (`/mnt/usb0/`, `/mnt/usb1/`).
- **Native PS5 Controller UI:** Built-in on-screen alphanumeric virtual keyboard and DualSense button mappings.

---

## Installation

Ps5Downloadly is packaged as a standard ShadowMountPlus package: **`PPSA99188.ffpkg`**.

1. Copy **`PPSA99188.ffpkg`** to:
   ```text
   /data/homebrew/
   ```
   *(Transfers can be done via FTP or a USB drive using file management tools on the console).*
2. ShadowMountPlus will automatically register the application.
3. Launch **Ps5Downloadly** directly from the **Games** row on your PS5 home screen.

---

## Usage Workflow

1. **Generate Direct Link:** Unrestrict your desired file or torrent link on your debrid service (e.g., Real-Debrid).
2. **Shorten the URL:** Shorten the direct download link at [tinyurl.com](https://tinyurl.com) (e.g., `https://tinyurl.com/XsDrt12`).
3. **Download on Console:**
   - Open **Ps5Downloadly** on your PS5.
   - Enter only the shortcode (e.g., `XsDrt12`) using the on-screen keyboard.
   - Press **Options** (or select **START DOWNLOAD**).
   - The file will download directly to your selected destination.

---

## Controller Layout

| Button | Action |
| :---: | :--- |
| **D-Pad / Left Stick** | Navigate virtual keyboard and menus |
| **✕ (Cross)** | Select character / Confirm action |
| **□ (Square)** | **Backspace** when typing / **Pause & Resume** during download |
| **△ (Triangle)** | Change destination directory |
| **Options** | Start download |
| **○ (Circle)** | Cancel download / Exit |
