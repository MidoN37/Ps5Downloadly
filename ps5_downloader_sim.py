#!/usr/bin/env python3
"""
PS5 Downloader - Interactive Desktop Simulator & Companion App
Recreates the native PS5 Homebrew UI, DualSense on-screen keyboard,
TinyURL unshortening, and real-time resumable download engine.
"""

import os
import sys
import time
import threading
import urllib.request
import urllib.parse
import urllib.error
import tkinter as tk
from tkinter import messagebox, filedialog

# Default storage locations
PS5_LOCATIONS = [
    "/data/homebrew",
    "/data",
    "/user/home",
    "/mnt/usb0",
    "/mnt/usb1"
]

class DownloaderState:
    IDLE = "IDLE"
    RESOLVING = "RESOLVING"
    DOWNLOADING = "DOWNLOADING"
    PAUSED = "PAUSED"
    COMPLETED = "COMPLETED"
    ERROR = "ERROR"

class PS5DownloaderApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Ps5Downloadly - PS5 Homebrew TV App")
        self.root.geometry("1060x780")
        self.root.configure(bg="#0B0E14")
        self.root.resizable(False, False)

        try:
            icon_p = os.path.abspath("icon.png")
            if os.path.exists(icon_p):
                self._icon_img = tk.PhotoImage(file=icon_p)
                self.root.iconphoto(False, self._icon_img)
        except Exception:
            pass

        # State
        self.state = DownloaderState.IDLE
        self.save_dir = PS5_LOCATIONS[0]
        self.local_save_dir = os.path.abspath("./data/homebrew")
        os.makedirs(self.local_save_dir, exist_ok=True)

        self.short_code = ""
        self.is_shift = False
        self.download_thread = None
        self.pause_event = threading.Event()
        self.cancel_event = threading.Event()

        # Download metrics
        self.target_url = ""
        self.filename = ""
        self.total_size = 0
        self.downloaded_size = 0
        self.speed = 0.0
        self.eta_seconds = 0
        self.resumed_from = 0

        self.setup_ui()

    def setup_ui(self):
        # 1. Header Bar
        header_frame = tk.Frame(self.root, bg="#141C2B", bd=1, relief="ridge", height=65)
        header_frame.pack(fill="x", padx=20, pady=(15, 10))

        title_lbl = tk.Label(header_frame, text="Ps5Downloadly", font=("Segoe UI", 16, "bold"), fg="#00F0FF", bg="#141C2B")
        title_lbl.pack(side="left", padx=15, pady=12)

        sub_lbl = tk.Label(header_frame, text="|  High-Speed Direct Downloader (TinyURL • Real-Debrid)", font=("Segoe UI", 10), fg="#8E9DAF", bg="#141C2B")
        sub_lbl.pack(side="left", pady=12)

        # 2. Main Input & Destination Cards
        cards_frame = tk.Frame(self.root, bg="#0B0E14")
        cards_frame.pack(fill="x", padx=20, pady=5)

        # Input Card (Left)
        input_card = tk.LabelFrame(cards_frame, text="  1. DIRECT DOWNLOAD LINK (TinyURL)  ", 
                                   font=("Segoe UI", 9, "bold"), fg="#8E9DAF", bg="#141C2B", bd=1, relief="solid")
        input_card.pack(side="left", fill="both", expand=True, padx=(0, 10), pady=5)

        url_box_frame = tk.Frame(input_card, bg="#0D141F", bd=1, relief="solid")
        url_box_frame.pack(fill="x", padx=15, pady=(12, 5))

        prefix_lbl = tk.Label(url_box_frame, text="https://tinyurl.com/", font=("Consolas", 13, "bold"), fg="#8E9DAF", bg="#0D141F")
        prefix_lbl.pack(side="left", padx=(10, 0), pady=8)

        self.code_entry = tk.Entry(url_box_frame, font=("Consolas", 14, "bold"), fg="#00F0FF", bg="#0D141F", 
                                   insertbackground="#00F0FF", bd=0)
        self.code_entry.pack(side="left", fill="x", expand=True, padx=5, pady=8)
        self.code_entry.focus_set()
        self.code_entry.bind("<KeyRelease>", self.on_entry_key)

        hint_lbl = tk.Label(input_card, text="Enter only the shortcode (e.g. XsDrt12). DualSense / PC keyboard active.", 
                            font=("Segoe UI", 8), fg="#64748B", bg="#141C2B")
        hint_lbl.pack(anchor="w", padx=15, pady=(0, 10))

        # Destination Card (Right)
        dest_card = tk.LabelFrame(cards_frame, text="  2. SAVE DESTINATION  ", 
                                  font=("Segoe UI", 9, "bold"), fg="#8E9DAF", bg="#141C2B", bd=1, relief="solid", width=340)
        dest_card.pack(side="right", fill="both", pady=5)

        self.path_var = tk.StringVar(value=self.save_dir)
        path_menu = tk.OptionMenu(dest_card, self.path_var, *PS5_LOCATIONS, command=self.on_path_change)
        path_menu.config(font=("Consolas", 10, "bold"), fg="#FFFFFF", bg="#0D141F", activebackground="#2D5FFF", 
                         activeforeground="#FFFFFF", bd=1, relief="solid", width=22)
        path_menu.pack(padx=15, pady=(15, 5))

        change_btn = tk.Button(dest_card, text="Choose Custom PC Folder...", font=("Segoe UI", 8), 
                               fg="#00F0FF", bg="#141C2B", bd=0, cursor="hand2", command=self.choose_custom_folder)
        change_btn.pack(padx=15, pady=(0, 10))

        # 3. Virtual On-Screen Controller Keyboard
        kb_card = tk.LabelFrame(self.root, text="  DUALSENSE ON-SCREEN VIRTUAL KEYBOARD  ", 
                                font=("Segoe UI", 9, "bold"), fg="#8E9DAF", bg="#141C2B", bd=1, relief="solid")
        kb_card.pack(fill="x", padx=20, pady=5)

        self.kb_frame = tk.Frame(kb_card, bg="#141C2B")
        self.kb_frame.pack(padx=10, pady=8)
        self.render_keyboard()

        # 4. Status & Download Monitor Card
        monitor_card = tk.LabelFrame(self.root, text="  3. DOWNLOAD MONITOR & RESUME ENGINE  ", 
                                     font=("Segoe UI", 9, "bold"), fg="#8E9DAF", bg="#141C2B", bd=1, relief="solid")
        monitor_card.pack(fill="both", expand=True, padx=20, pady=5)

        # Status text
        self.status_lbl = tk.Label(monitor_card, text="Ready. Enter TinyURL code and click 'Start Download'.", 
                                   font=("Segoe UI", 11, "bold"), fg="#FFFFFF", bg="#141C2B")
        self.status_lbl.pack(anchor="w", padx=15, pady=(10, 5))

        # Progress bar canvas
        self.pb_canvas = tk.Canvas(monitor_card, height=26, bg="#0A0F1A", bd=0, highlightthickness=1, highlightbackground="#223048")
        self.pb_canvas.pack(fill="x", padx=15, pady=5)

        # Metrics Labels
        metrics_frame = tk.Frame(monitor_card, bg="#141C2B")
        metrics_frame.pack(fill="x", padx=15, pady=(5, 10))

        self.progress_pct_lbl = tk.Label(metrics_frame, text="0.0%", font=("Segoe UI", 10, "bold"), fg="#00F0FF", bg="#141C2B")
        self.progress_pct_lbl.pack(side="left")

        self.metrics_lbl = tk.Label(metrics_frame, text="Transferred: 0 B / 0 B  |  Speed: 0.0 MB/s  |  ETA: --:--", 
                                    font=("Segoe UI", 9), fg="#8E9DAF", bg="#141C2B")
        self.metrics_lbl.pack(side="left", padx=20)

        # Buttons (Pause / Resume / Cancel / Start)
        btn_box = tk.Frame(monitor_card, bg="#141C2B")
        btn_box.pack(anchor="e", padx=15, pady=(0, 10))

        self.start_btn = tk.Button(btn_box, text="START DOWNLOAD  ➔", font=("Segoe UI", 10, "bold"), 
                                   fg="#FFFFFF", bg="#0070D1", activebackground="#2D5FFF", activeforeground="#FFFFFF", 
                                   bd=0, padx=16, pady=5, cursor="hand2", command=self.start_download_action)
        self.start_btn.pack(side="left", padx=5)

        self.pause_btn = tk.Button(btn_box, text="PAUSE", font=("Segoe UI", 9, "bold"), 
                                   fg="#FFFFFF", bg="#334155", activebackground="#475569", activeforeground="#FFFFFF", 
                                   bd=0, padx=12, pady=5, cursor="hand2", state="disabled", command=self.toggle_pause)
        self.pause_btn.pack(side="left", padx=5)

        self.cancel_btn = tk.Button(btn_box, text="CANCEL", font=("Segoe UI", 9, "bold"), 
                                    fg="#FFFFFF", bg="#4B1E22", activebackground="#7F1D1D", activeforeground="#FFFFFF", 
                                    bd=0, padx=12, pady=5, cursor="hand2", state="disabled", command=self.cancel_download)
        self.cancel_btn.pack(side="left", padx=5)

        # 5. Footer Legend Bar
        footer = tk.Frame(self.root, bg="#080B10", height=40)
        footer.pack(fill="x", side="bottom")

        legend = " [X] Select Key    [Square] Backspace / Pause    [Triangle] Change Path    [Options] Start Download "
        tk.Label(footer, text=legend, font=("Segoe UI", 9), fg="#64748B", bg="#080B10").pack(pady=10)

        self.update_progress_bar(0)

    def render_keyboard(self):
        for widget in self.kb_frame.winfo_children():
            widget.destroy()

        rows = [
            ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "_"],
            ["Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"],
            ["A", "S", "D", "F", "G", "H", "J", "K", "L"],
            ["Z", "X", "C", "V", "B", "N", "M", "."],
            ["SHIFT", "⌫ BACKSPACE", "CLEAR ALL", "START DOWNLOAD ➔"]
        ] if self.is_shift else [
            ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "_"],
            ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p"],
            ["a", "s", "d", "f", "g", "h", "j", "k", "l"],
            ["z", "x", "c", "v", "b", "n", "m", "."],
            ["shift", "⌫ BACKSPACE", "CLEAR ALL", "START DOWNLOAD ➔"]
        ]

        for r_idx, row in enumerate(rows):
            r_frame = tk.Frame(self.kb_frame, bg="#141C2B")
            r_frame.pack(pady=2)
            for key in row:
                w = 16 if len(key) > 2 else 4
                bg_col = "#2D5FFF" if "START" in key else ("#1E293B" if len(key) > 2 else "#0F172A")
                fg_col = "#FFFFFF" if ("START" in key or len(key) > 2) else "#94A3B8"
                btn = tk.Button(r_frame, text=key, font=("Consolas", 10, "bold"), fg=fg_col, bg=bg_col,
                                activebackground="#00F0FF", activeforeground="#000000", bd=1, relief="solid",
                                width=w, height=1, cursor="hand2",
                                command=lambda k=key: self.on_key_press(k))
                btn.pack(side="left", padx=2)

    def on_key_press(self, key):
        if key in ("SHIFT", "shift"):
            self.is_shift = not self.is_shift
            self.render_keyboard()
        elif "BACKSPACE" in key:
            cur = self.code_entry.get()
            if cur:
                self.code_entry.delete(len(cur) - 1, tk.END)
        elif "CLEAR" in key:
            self.code_entry.delete(0, tk.END)
        elif "START" in key:
            self.start_download_action()
        else:
            self.code_entry.insert(tk.END, key)

    def on_entry_key(self, event):
        val = self.code_entry.get()
        # Clean if full tinyurl pasted
        clean = self.sanitize_code(val)
        if clean != val:
            self.code_entry.delete(0, tk.END)
            self.code_entry.insert(0, clean)

    def sanitize_code(self, val):
        val = val.strip()
        for prefix in ("https://tinyurl.com/", "http://tinyurl.com/", "tinyurl.com/"):
            if val.startswith(prefix):
                val = val[len(prefix):]
        return val.lstrip("/")

    def on_path_change(self, val):
        self.save_dir = val
        self.status_lbl.config(text=f"Save path updated to PS5 location: {self.save_dir}")

    def choose_custom_folder(self):
        folder = filedialog.askdirectory(initialdir=self.local_save_dir, title="Select Local Save Directory")
        if folder:
            self.local_save_dir = folder
            self.status_lbl.config(text=f"Local save directory: {self.local_save_dir}")

    def update_progress_bar(self, percentage):
        self.pb_canvas.delete("all")
        w = self.pb_canvas.winfo_width() or 1000
        h = self.pb_canvas.winfo_height() or 26
        fill_w = int(w * (percentage / 100.0))
        if fill_w > 0:
            self.pb_canvas.create_rectangle(0, 0, fill_w, h, fill="#00F0FF", outline="")
            self.pb_canvas.create_rectangle(0, 0, fill_w, 2, fill="#FFFFFF", outline="")

    def format_bytes(self, num):
        for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
            if abs(num) < 1024.0:
                return f"{num:3.2f} {unit}"
            num /= 1024.0
        return f"{num:.2f} PB"

    def format_eta(self, seconds):
        if seconds <= 0:
            return "--:--"
        m, s = divmod(seconds, 60)
        h, m = divmod(m, 60)
        return f"{h:02d}h {m:02d}m {s:02d}s" if h else f"{m:02d}m {s:02d}s"

    def start_download_action(self):
        raw_code = self.code_entry.get().strip()
        code = self.sanitize_code(raw_code)
        if not code:
            messagebox.showwarning("Input Required", "Please enter the TinyURL code (e.g. XsDrt12).")
            return

        self.short_code = code
        self.cancel_event.clear()
        self.pause_event.clear()

        self.start_btn.config(state="disabled")
        self.pause_btn.config(state="normal", text="PAUSE")
        self.cancel_btn.config(state="normal")

        self.download_thread = threading.Thread(target=self.run_downloader_thread, daemon=True)
        self.download_thread.start()

    def run_downloader_thread(self):
        try:
            # 1. Resolving TinyURL
            self.state = DownloaderState.RESOLVING
            full_url = f"https://tinyurl.com/{self.short_code}"
            self.status_lbl.config(text=f"Resolving {full_url} ...", fg="#00F0FF")

            req = urllib.request.Request(full_url, headers={"User-Agent": "Mozilla/5.0 (PlayStation 5; PS5 Downloader/1.0)"})
            response = urllib.request.urlopen(req, timeout=15)
            self.target_url = response.geturl()

            # Filename detection
            cd = response.headers.get("Content-Disposition")
            if cd and "filename=" in cd:
                fname = cd.split("filename=")[1].strip('"\' ')
            else:
                path = urllib.parse.urlparse(self.target_url).path
                fname = os.path.basename(path) or f"download_{self.short_code}.bin"

            self.filename = fname
            local_filepath = os.path.join(self.local_save_dir, self.filename)

            # Get Content-Length
            length_hdr = response.headers.get("Content-Length")
            self.total_size = int(length_hdr) if length_hdr else 0

            # 2. Check for Resume Support
            existing_bytes = 0
            if os.path.exists(local_filepath):
                existing_bytes = os.path.getsize(local_filepath)

            file_mode = "ab" if existing_bytes > 0 else "wb"
            self.downloaded_size = existing_bytes
            self.resumed_from = existing_bytes

            resume_header = {}
            if existing_bytes > 0:
                resume_header = {"Range": f"bytes={existing_bytes}-"}
                self.status_lbl.config(text=f"Resuming {self.filename} from {self.format_bytes(existing_bytes)} ...", fg="#00E676")
            else:
                self.status_lbl.config(text=f"Downloading {self.filename} ({self.format_bytes(self.total_size)}) ...", fg="#FFFFFF")

            # Stream download
            stream_req = urllib.request.Request(self.target_url, headers={
                "User-Agent": "Mozilla/5.0 (PlayStation 5; PS5 Downloader/1.0)",
                **resume_header
            })
            stream_res = urllib.request.urlopen(stream_req, timeout=20)

            # If resuming and response is 206, total size is adjusted
            if existing_bytes > 0 and stream_res.getcode() == 206:
                content_range = stream_res.headers.get("Content-Range")
                if content_range and "/" in content_range:
                    self.total_size = int(content_range.split("/")[1])
            elif existing_bytes > 0 and stream_res.getcode() == 200:
                # Server does not support resume, start over
                file_mode = "wb"
                self.downloaded_size = 0
                self.resumed_from = 0

            self.state = DownloaderState.DOWNLOADING

            last_calc_time = time.time()
            bytes_since_calc = 0
            start_time = time.time()

            with open(local_filepath, file_mode) as f:
                while True:
                    if self.cancel_event.is_set():
                        self.state = DownloaderState.IDLE
                        self.status_lbl.config(text="Download cancelled.", fg="#FF5252")
                        break

                    # Pause check
                    if self.pause_event.is_set():
                        self.state = DownloaderState.PAUSED
                        self.status_lbl.config(text=f"Download paused. Resume anytime.", fg="#FFB300")
                        time.sleep(0.2)
                        continue

                    chunk = stream_res.read(64 * 1024)
                    if not chunk:
                        # Completed!
                        self.state = DownloaderState.COMPLETED
                        self.status_lbl.config(text=f"SUCCESS: {self.filename} saved to {self.save_dir}", fg="#00E676")
                        self.update_progress_bar(100)
                        self.progress_pct_lbl.config(text="100.0%")
                        break

                    f.write(chunk)
                    self.downloaded_size += len(chunk)
                    bytes_since_calc += len(chunk)

                    # Update speed & ETA every 0.5 sec
                    now = time.time()
                    elapsed = now - last_calc_time
                    if elapsed >= 0.5:
                        self.speed = bytes_since_calc / elapsed
                        bytes_since_calc = 0
                        last_calc_time = now

                        if self.total_size > 0:
                            pct = min(100.0, (self.downloaded_size / self.total_size) * 100.0)
                            rem_bytes = max(0, self.total_size - self.downloaded_size)
                            self.eta_seconds = int(rem_bytes / self.speed) if self.speed > 0 else 0
                        else:
                            pct = 0.0
                            self.eta_seconds = 0

                        self.progress_pct_lbl.config(text=f"{pct:4.1f}%")
                        self.metrics_lbl.config(
                            text=f"Transferred: {self.format_bytes(self.downloaded_size)} / {self.format_bytes(self.total_size)}  |  Speed: {self.format_bytes(self.speed)}/s  |  ETA: {self.format_eta(self.eta_seconds)}"
                        )
                        self.update_progress_bar(pct)

        except urllib.error.HTTPError as e:
            self.state = DownloaderState.ERROR
            self.status_lbl.config(text=f"HTTP Error {e.code}: {e.reason}", fg="#FF5252")
        except Exception as e:
            self.state = DownloaderState.ERROR
            self.status_lbl.config(text=f"Error: {str(e)}", fg="#FF5252")
        finally:
            self.start_btn.config(state="normal")
            self.pause_btn.config(state="disabled")
            self.cancel_btn.config(state="disabled")

    def toggle_pause(self):
        if self.state == DownloaderState.DOWNLOADING:
            self.pause_event.set()
            self.pause_btn.config(text="RESUME", bg="#0070D1")
        elif self.state == DownloaderState.PAUSED:
            self.pause_event.clear()
            self.state = DownloaderState.DOWNLOADING
            self.pause_btn.config(text="PAUSE", bg="#334155")
            self.status_lbl.config(text=f"Resuming {self.filename} ...", fg="#FFFFFF")

    def cancel_download(self):
        if self.state in (DownloaderState.DOWNLOADING, DownloaderState.PAUSED):
            self.cancel_event.set()
            self.pause_event.clear()

if __name__ == "__main__":
    root = tk.Tk()
    app = PS5DownloaderApp(root)
    root.mainloop()
