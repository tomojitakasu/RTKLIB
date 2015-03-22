--------------------------------------------------------------------------------

           RTKLIB: An Open Source Program Package for GNSS Positioning

--------------------------------------------------------------------------------

OVERVIEW

RTKLIB is an open source program package for standard and precise positioning
with GNSS (global navigation satellite system). RTKLIB consists of a portable
program library and several APs (application programs) utilizing the library.
The features of RTKLIB are:

(1) It supports standard and precise positioning algorithms with:
    
    GPS, GLONASS, Galileo, QZSS, BeiDou and SBAS
    
(2) It supports various positioning modes with GNSS for both real-time- and
    post-processing:
    
    Single, DGPS/DGNSS, Kinematic, Static, Moving-Baseline, Fixed,
    PPP-Kinematic, PPP-Static and PPP-Fixed.
    
(3) It supports many standard formats and protocols for GNSS:
    
    RINEX 2.10,2.11,2.12 OBS/NAV/GNAV/HNAV/LNAV/QNAV, RINEX 3.00,3.01,3.02
    OBS/NAV,RINEX 3.02 CLK,RTCM ver.2.3,RTCM ver.3.1 (with amendment 1-5),
    RTCM ver.3.2, BINEX, NTRIP 1.0, NMEA 0183, SP3-c, ANTEX 1.4, IONEX 1.0,
    NGS PCV and EMS 2.0.
    
(4) It supports several GNSS receivers' proprietary messages:
    
    NovAtel: OEM4/V/6,OEM3,OEMStar,Superstar II, Hemisphere: Eclipse,Crescent,
    u-blox: LEA-4T/5T/6T, SkyTraq: S1315F, JAVAD GRIL/GREIS, Furuno
    GW-10-II/III and NVS NV08C BINR.
    
(5) It supports external communication via:
    
    Serial, TCP/IP, NTRIP, local log file (record and playback) and FTP/HTTP
    (automatic download).
    
(6) It provides many library functions and APIs (application program
    interfaces):
    
    Satellite and navigation system functions, matrix and vector functions,
    time and string functions, coordinates transformation, input and output
    functions, debug trace functions, platform dependent functions,
    positioning models, atmosphere models, antenna models, earth tides models,
    geoid models, datum transformation, RINEX functions, ephemeris and clock
    functions, precise ephemeris and clock functions, receiver raw data
    functions, RTCM functions, solution functions, Google Earth KML converter,
    SBAS functions, options functions, stream data input and output functions,
    integer ambiguity resolution, standard positioning, precise positioning,
    post-processing positioning, stream server functions, RTK server
    functions, downloader functions.
    
(7) It includes the following GUI (graphical user interface) and CUI
    (command-line user interface) APs.
    
    --------------------------------------------------------------------------
        Function                     GUI AP          CUI AP
    --------------------------------------------------------------------------
    (a) AP Launcher                  RTKLAUNCH       -
    (b) Real-Time Positioning        RTKNAVI         RTKRCV
    (c) Communication Server         STRSVR          STR2STR
    (d) Post-Processing Analysis     RTKPOST         RNX2RTKP
    (e) RINEX Converter              RTKCONV         CONVBIN
    (f) Plot Solutions and Obs Data  RTKPLOT         -
    (g) Downloder of GNSS Data       RTKGET          -
    (h) NTRIP Browser                NTRIPSRCBROWS   -
    --------------------------------------------------------------------------

(8) All of the executable binary APs for Windows are included in the package as
    well as whole source programs of the library and the APs.

--------------------------------------------------------------------------------

SYSTEM REQUIEREMENTS

The executable binary GUI and CUI APs included in the package require Microsoft
Windows environment. On the other OS or environment, you have to compile and
build CUI APs by yourself.
All of the library functions and APIs were written in ANSI C (C89). The library
internally uses winsock and WIN32 thread for Windows with the compiler option
-DWIN32 and the standard socket and pthread (POSIX thread) for Linux/UNIX
without any option. By setting the compiler option -DLAPACK or -DMKL, the
library uses LAPACK/BLAS [36] or Intel MKL for fast matrix computation. The CUI
APs were written in ANSI C. The library and CUI APs can be built on many
environments like gcc on Linux. The GUI APs were written in C++ and utilize
Embarcadero/Borland VCL (visual component library) for GUI toolkits. All of the
executable binary APs in the package were built by Embarcadero C++ builder XE2
Starter Edition on Windows 7.
The executable GUI APs were tested on Windows 7 (64bit). The CUI APs were also
built and tested on Ubuntu 11.04 Linux and x86 CPU.

Notes:
Previous versions of RTKLIB until ver. 2.4.1 were built by a free edition of
Borland C++ (Turbo C++ 2006). Turbo C++, however, is no longer supported in
ver. 2.4.2 because of type incompatibility problem of GUI strings between
ver.2.4.2 and the previous ones.

--------------------------------------------------------------------------------

LICENSE

The RTKLIB software package is distributed under the following BSD 2-clause
license (http://opensource.org/licenses/BSD-2-Clause) and additional two
exclusive clauses. Users are permitted to develop, produce or sell their own
non-commercial or commercial products utilizing, linking or including RTKLIB as
long as they comply with the license.

          Copyright (c) 2007-2013, T. Takasu, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

- The software package includes some companion executive binaries or shared
  libraries necessary to execute APs on Windows. These licenses succeed to the
  original ones of these software. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Notes:
Previous versions of RTKLIB until ver. 2.4.1 had been distributed under GPLv3
(http://www.gnu.org/licenses/gpl-3.0.en.html) license.

--------------------------------------------------------------------------------

DIRECTORY STRUCTURE OF PACKAGE

  rtklib_<ver>
  ./src             source programs of RTKLIB library *
    ./rcv           source programs depending on GPS/GNSS receivers *
  ./bin             executable binary APs and DLLs for Windows
  ./data            sample data for APs
  ./app             build environment of APs *
    ./rtknavi       RTKNAVI       (GUI) *
    ./rtknavi_mkl   RTKNAVI_MKL   (GUI) *
    ./strsvr        STRSVR        (GUI) *
    ./rtkpost       RTKPOST       (GUI) *
    ./rtkpost_mkl   RTKPOST_MKL   (GUI) *
    ./rtkplot       RTKPLOT       (GUI) *
    ./rtkconv       RTKCONV       (GUI) *
    ./srctblbrows   NTRIP Browser (GUI) *
    ./rtkget        RTKGET        (GUI) *
    ./rtklaunch     RTKLAUNCH     (GUI) *
    ./rtkrcv        RTKRCV        (CUI) *
    ./rnx2rtkp      RNX2RTKP      (CUI) *
    ./pos2kml       POS2KML       (CUI) *
    ./convbin       CONVBIN       (CUI) *
    ./str2str       STR2STR       (CUI) *
    ./appcmn        common routines for GUI APs *
    ./icon          icon data for GUI APs *
  ./lib             library genration environment *
  ./test            test programs and data *
  ./util            utilities *
  ./doc             document files
  
  * not included in the binary package rtklib_<ver>_bin.zip

--------------------------------------------------------------------------------

MANUAL

Refer rtklib_<ver>/doc/manual_<ver>.pdf.

--------------------------------------------------------------------------------

SUPPORT INFORMATION

Refer http://www.rtklib.com/rtklib_support.htm.

--------------------------------------------------------------------------------

HISTORY

  ver.1.0      2007/01/25 new release
  ver.1.1      2007/03/20 add rnx2rtkp_gui, fix bugs, improve performance
  ver.2.1.0    2008/07/15 refactored, add applications
  ver.2.1.1    2008/10/19 fix bugs
  ver.2.2.0    2009/01/20 add stream.c,rtksvr.c,preceph.c in src
                          add rtknavi,rtkpost_mkl,srctblbrows,strsvr,str2str in app
  ver.2.2.1    2009/05/17 see relnotes_2.2.1.txt
  ver.2.2.2    2009/09/07 see relnotes_2.2.2.txt
  ver.2.3.0    2009/12/17 see relnotes_2.3.0.txt
  ver.2.4.0    2010/08/08 see relnotes_2.4.0.pdf
  ver.2.4.1    2011/06/01 see relnotes_2.4.1.htm
  ver.2.4.2    2013/04/19 see relnotes_2.4.2.htm

--------------------------------------------------------------------------------
