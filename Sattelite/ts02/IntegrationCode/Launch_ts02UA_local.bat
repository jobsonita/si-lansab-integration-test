@SET PATH=C:/cygwin/bin;%PATH%
@echo off
cd .
call ts02UA 127.0.0.1 1232 224.255.255.255 127.0.0.1 12340
pause