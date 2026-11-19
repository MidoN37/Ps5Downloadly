@echo off
title PS5 Downloader Simulator
cd /d "%~dp0"
python ps5_downloader_sim.py
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo An error occurred running the simulator.
    pause
)
