@echo off
cd /d "%~dp0"
set "PATH=%PATH%;C:\Users\Fati\.mingit\cmd"
echo ========================================================
echo   Pushing Ps5Downloadly to GitHub (MidoN37/Ps5Downloadly)
echo ========================================================
echo.
git push -u origin main
echo.
pause
