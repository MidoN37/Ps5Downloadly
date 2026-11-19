#!/usr/bin/env python3
"""
PS5 ELF Payload Sender
Sends compiled ELF payloads (like ps5-downloader.elf) to a jailbroken PS5
running an ELF loader (e.g. elfldr / etaHEN on port 9020 or 9021).
"""

import sys
import socket
import os

DEFAULT_PORT = 9021

def send_elf(ps5_ip, elf_path, port=DEFAULT_PORT):
    if not os.path.exists(elf_path):
        print(f"[-] Error: File not found: {elf_path}")
        return False

    file_size = os.path.getsize(elf_path)
    print(f"[*] Payload : {elf_path} ({file_size} bytes)")
    print(f"[*] Target  : {ps5_ip}:{port}")

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(10)
            print("[*] Connecting to PS5 ELF loader...")
            s.connect((ps5_ip, port))
            print("[+] Connected! Sending payload...")
            
            with open(elf_path, "rb") as f:
                while chunk := f.read(64 * 1024):
                    s.sendall(chunk)
                    
            print("[+] Payload successfully transmitted to PS5!")
            return True
    except ConnectionRefusedError:
        print(f"[-] Connection refused on {ps5_ip}:{port}. Make sure your ELF loader / jailbreak is active.")
    except socket.timeout:
        print(f"[-] Connection timed out connecting to {ps5_ip}:{port}.")
    except Exception as e:
        print(f"[-] Error: {e}")
    return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python send_payload.py <PS5_IP> [ELF_PATH] [PORT]")
        print("Example: python send_payload.py 192.168.1.50 dist/ps5-downloader.elf 9021")
        sys.exit(1)

    ip = sys.argv[1]
    path = sys.argv[2] if len(sys.argv) > 2 else "dist/ps5-downloader.elf"
    p = int(sys.argv[3]) if len(sys.argv) > 3 else DEFAULT_PORT

    send_elf(ip, path, p)
