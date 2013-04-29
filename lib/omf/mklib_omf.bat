@echo off
rem generate omf library from dll for rtklib

set BDSDIR=C:\Program Files (x86)\Embarcadero\RAD Studio\9.0\bin
set PATH=%BDSDIR%\bin;%PATH%

impdef -a mkl_p4p_omf.def ..\..\bin\mkl_p4p.dll
impdef -a mkl_lapack_omf.def ..\..\bin\mkl_lapack.dll
implib -a mkl_p4p_omf.lib mkl_p4p_omf.def
implib -a mkl_lapack_omf.lib mkl_lapack_omf.def
del mkl_p4p_omf.def
del mkl_lapack_omf.def
