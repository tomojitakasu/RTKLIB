@echo off
rem generate import library for libopenblas.dll

set BDSDIR=C:\Program Files (x86)\Embarcadero\Studio\2.0\9.0\bin
set PATH=%BDSDIR%\bin;%PATH%

mkexp libopenblas.a ..\..\..\RTKLIB_bin\bin\libopenblas.dll
