@echo off

cd /d %~dp0

python convert_sprite.py

rem compile fxdata
python fxdata-build.py fxdata.txt

if %errorlevel%==0 goto postbuildfx

pause
exit /b 1

:postbuildfx
