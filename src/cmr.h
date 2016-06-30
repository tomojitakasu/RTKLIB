/*------------------------------------------------------------------------------
* cmr.h : CMR dependent functions include file
*
* references:
*     [1] https://github.com/astrodanco/RTKLIB/tree/cmr/src/cmr.h
*
* version : $Revision:$ $Date:$
*-----------------------------------------------------------------------------
*/

/*
| Written in June 2016 by Daniel A. Cook, for inclusion into the RTKLIB library.
*/

/*
| CMR+ protocol defined antenna number to name lookup table.
|
| Note that this is not a table like the ones in igs08.atx or ngs_abs.pcv and
| cannot be replaced with nor derrived from the content of those files.
|
| Adapted from "c:\program files (x86)\common files\trimble\config\antenna.ini"
| after installing the latest Trimble Office Configuration File Update Utility
| found at <http://www.trimble.com/infrastructure/trimbleconfiguration_ts.aspx>
| For each antenna definition block in antenna.ini the "Type" keyword provides
| the number and the "IGSAntenna" keyword provides the antenna name. If no
| "IGSAntenna" keyword is present then the "Name" keyword provides the name.
| The comment is taken from the "Manufacturer" keyword followed by the "Name"
| keyword unless the name already contains the manufacturer name in which case
| the manufacturer name is not repeated.
*/
static const char *AntennasTable[] = {
/* 000 */ "UNKNOWN_EXT     NONE",       /* Unknown External */
/* 001 */ "TRM4000ST_INT   NONE",       /* Trimble 4000ST Internal */
/* 002 */ "TRM14156.00-GP  NONE",       /* Trimble 4000ST Kinematic Ext. */
/* 003 */ "TRM16741.00     NONE",       /* Trimble Compact Dome */
/* 004 */ "TRM14177.00     NONE",       /* Trimble 4000ST L1 Geodetic */
/* 005 */ "TRM14532.00     NONE",       /* Trimble 4000SST/SSE L1/L2 Geodetic */
/* 006 */ "TRM12562.00+SGP NONE",       /* Trimble 4000SLD L1/L2 Square */
/* 007 */ "TRM10877.10_H   NONE",       /* Trimble 4000SX Helical */
/* 008 */ "TRM11877.10+SGP NONE",       /* Trimble 4000SX Micro Square */
/* 009 */ "TRM12333.00+RGP NONE",       /* Trimble 4000SL Micro Round */
/* 010 */ "TRM17200.00     NONE",       /* Trimble 4000SE Attachable */
/* 011 */ "TRM14532.10     NONE",       /* Trimble 4000SSE Kin L1/L2 */
/* 012 */ "TRM22020.00+GP  NONE",       /* Trimble Compact L1/L2 w/Ground Plane */
/* 013 */ "TRM22020.00-GP  NONE",       /* Trimble Compact L1/L2 */
/* 014 */ "TRM16741.00_RTK NONE",       /* Trimble Compact Dome w/Init */
/* 015 */ "TRM14532.10_RTK NONE",       /* Trimble L1/L2 Kinematic w/Init */
/* 016 */ "TRM22020.00_RTK NONE",       /* Trimble Compact L1/L2 w/Init */
/* 017 */ "TRM23965.00_RTK NONE",       /* Trimble Compact L1 w/Init */
/* 018 */ "TRM23965.00     NONE",       /* Trimble Compact L1 w/Ground Plane */
/* 019 */ "TRM23965.00-GP  NONE",       /* Trimble Compact L1 */
/* 020 */ "TRM23903.00     NONE",       /* Trimble Permanent L1/L2 */
/* 021 */ "TRM4600LS       NONE",       /* Trimble 4600LS Internal */
/* 022 */ "TRM12562.10+RGP NONE",       /* Trimble 4000SLD L1/L2 Round */
/* 023 */ "AOAD/M_T        NONE",       /* Dorne Margolin Model T */
/* 024 */ "ASH700228A      NONE",       /* Ashtech L1/L2 (A-B) */
/* 025 */ "ASH700936A_M    NONE",       /* Ashtech 700936A_M */
/* 026 */ "LEISRX99+GP     NONE",       /* Leica SR299/SR399 External w/Ground Plane */
/* 027 */ "TRM29659.00     NONE",       /* Trimble Choke Ring */
/* 028 */ "JPLD/M_R        NONE",       /* Dorne Margolin Model R */
/* 029 */ "ASH700829.2     SNOW",       /* Ashtech Geodetic III USCG w/Radome */
/* 030 */ "TRM29653.00     NONE",       /* Trimble Integrated GPS/Beacon */
/* 031 */ "Mobile GPS Antenna",         /* Trimble Mobile GPS Antenna */
/* 032 */ "GeoExplorer Internal",       /* Trimble GeoExplorer Internal */
/* 033 */ "TOP72110        NONE",       /* Topcon Turbo-SII */
/* 034 */ "TRM22020.00+GP  TCWD",       /* Trimble Compact L1/L2 w/Ground Plane+Radome */
/* 035 */ "TRM23903.00     TCWD",       /* Trimble Permanent L1/L2 w/Radome */
/* 036 */ "LEISRX99-GP     NONE",       /* Leica SR299/SR399 External */
/* 037 */ "AOAD/M_B        NONE",       /* Dorne Margolin Model B */
/* 038 */ "TRM4800         NONE",       /* Trimble 4800 Internal */
/* 039 */ "TRM33429.00-GP  NONE",       /* Trimble Micro-centered L1/L2 */
/* 040 */ "TRM33429.00+GP  NONE",       /* Trimble Micro-centered L1/L2 w/Ground Plane */
/* 041 */ "TRM33580.50     NONE",       /* Trimble Integrated GPS/Beacon/Sat DGPS */
/* 042 */ "TRM35143.00     NONE",       /* Trimble AeroAntenna */
/* 043 */ "TOP700779A      NONE",       /* Topcon Geodetic 3 Rev.D */
/* 044 */ "ASH700718A      NONE",       /* Ashtech Geodetic III L1/L2 */
/* 045 */ "ASH700936A_M    SNOW",       /* Ashtech Choke w/Radome */
/* 046 */ "TOP700337       NONE",       /* Topcon Geodetic 1 Rev.B */
/* 047 */ "TRM36569.00+GP  NONE",       /* Trimble Rugged Micro-centered w/13Inch GP */
/* 048 */ "GeoExplorer 3",              /* Trimble GeoExplorer 3 */
/* 049 */ "TRM33429.20+GP  NONE",       /* Trimble Micro-centered L1/L2 Permanent */
/* 050 */ "TRM33429.20+GP  TCWD",       /* Trimble Micro-centered L1/L2 Permanent w/Radome */
/* 051 */ "TRM27947.00-GP  NONE",       /* Trimble Rugged L1/L2 */
/* 052 */ "TRM27947.00+GP  NONE",       /* Trimble Rugged L1/L2 w/Ground Plane */
/* 053 */ "ASH701008.01B   NONE",       /* Ashtech Geodetic IIIA */
/* 054 */ "ASH700228C      NONE",       /* Ashtech L1/L2, no Level (C) */
/* 055 */ "ASH700228E      NONE",       /* Ashtech L1/L2, Rev. B (D-E) */
/* 056 */ "ASH700700.A     NONE",       /* Ashtech Marine L1/L2 (A) */
/* 057 */ "ASH700700.B     NONE",       /* Ashtech Marine L1/L2 (B) */
/* 058 */ "JPSLEGANT_E     NONE",       /* Javad Positioning Systems JPS Legant w/Flat Groundplane */
/* 059 */ "JPSREGANT_SD_E  NONE",       /* Javad Positioning Systems JPS Regant w/Single Depth Choke, Ext */
/* 060 */ "JPSREGANT_DD_E  NONE",       /* Javad Positioning Systems JPS Regant w/Dual Depth Choke, Ext */
/* 061 */ "LEIAT202-GP     NONE",       /* Leica AT202 */
/* 062 */ "LEIAT302-GP     NONE",       /* Leica AT302 */
/* 063 */ "LEIAT303        LEIC",       /* Leica AT303 w/Choke Ring+Radome */
/* 064 */ "LEIAT303        NONE",       /* Leica AT303 w/Choke Ring */
/* 065 */ "LEIAT502        NONE",       /* Leica AT502 */
/* 066 */ "LEIAT504        NONE",       /* Leica AT504 w/Choke Ring */
/* 067 */ "MAC4647942      MMAC",       /* Macrometer Crossed Dipoles */
/* 068 */ "NOV501          NONE",       /* NovAtel GPS-501 L1 */
/* 069 */ "NOV501+CR       NONE",       /* NovAtel GPS-501 L1 w/Choke */
/* 070 */ "NOV502          NONE",       /* NovAtel GPS-502 L1/L2 */
/* 071 */ "NOV502+CR       NONE",       /* NovAtel GPS-502 L1/L2 w/Choke */
/* 072 */ "NOV531          NONE",       /* NovAtel GPS-531 L1 */
/* 073 */ "NOV531+CR       NONE",       /* NovAtel GPS-531 L1 w/Choke */
/* 074 */ "SEN67157514     NONE",       /* Sensor Systems L1/L2, Passive */
/* 075 */ "SEN67157514+CR  NONE",       /* Sensor Systems L1/L2 w/Choke, Passive */
/* 076 */ "SEN67157549     NONE",       /* Sensor Systems L1 */
/* 077 */ "SEN67157549+CR  NONE",       /* Sensor Systems L1 w/Choke Ring */
/* 078 */ "SEN67157596     NONE",       /* Sensor Systems L1/L2, Active */
/* 079 */ "SEN67157596+CR  NONE",       /* Sensor Systems L1/L2 w/Choke, Active */
/* 080 */ "NGSD/M+GP60     NONE",       /* NGS D/M+gp60 */
/* 081 */ "ASH701945.02B   NONE",       /* Ashtech D/M Choke, Rev B */
/* 082 */ "ASH701946.2     NONE",       /* Ashtech D/M Choke, Rev B, GPS-Glonass */
/* 083 */ "SPP571908273    NONE",       /* Spectra Precision Choke */
/* 084 */ "SPP571908273    SPKE",       /* Spectra Precision Choke w/Radome */
/* 085 */ "TRM39105.00     NONE",       /* Trimble Zephyr */
/* 086 */ "TRM41249.00     NONE",       /* Trimble Zephyr Geodetic */
/* 087 */ "PF Power Internal",          /* Trimble PF Power Internal */
/* 088 */ "SPP571212430    NONE",       /* Spectra Precision Compact L2 */
/* 089 */ "SPP571212238+GP NONE",       /* Spectra Precision Geod. w/GP L2 */
/* 090 */ "SPP571908941    NONE",       /* Spectra Precision Mini-Geodetic L1/L2 */
/* 091 */ "SPPGEOTRACER2000NONE",       /* Spectra Precision SPP Geotracer 2000/2100 L1 Internal */
/* 092 */ "SPP571212240    NONE",       /* Spectra Precision Compact L1 */
/* 093 */ "SPP571212236    NONE",       /* Spectra Precision Geod. w/GP L1 */
/* 094 */ "SPP571212774    NONE",       /* Spectra Precision Mini-Geodetic L1 */
/* 095 */ "SPP571212790    NONE",       /* Spectra Precision GG/GPS Pro L1 */
/* 096 */ "ZEIMINI_GEOD    NONE",       /* Zeiss Mini-Geodetic L1/L2 */
/* 097 */ "TRM5800         NONE",       /* Trimble R8/5800/SPS78x Internal */
/* 098 */ "NOV503+CR       NONE",       /* NovAtel GPS-503 L1/L2 w/Choke */
/* 099 */ "NOV503+CR       SPKE",       /* NovAtel GPS-503 L1/L2 w/Choke+Radome */
/* 100 */ "ASH103661       NONE",       /* Ashtech Marine IV L1 */
/* 101 */ "ASH104847       NONE",       /* Ashtech Marine IV GG L1 */
/* 102 */ "ASH700489       NONE",       /* Ashtech Dimension L1 */
/* 103 */ "ASH700699A      NONE",       /* Ashtech Marine III A L1 */
/* 104 */ "ASH700699B      NONE",       /* Ashtech Marine III B L1 */
/* 105 */ "DSNP DGU001     NONE",       /* DSNP DGU001 */
/* 106 */ "DSNP DGU002     NONE",       /* DSNP DGU002 */
/* 107 */ "DSNP NAP001     NONE",       /* DSNP NAP001 */
/* 108 */ "DSNP NAP002     NONE",       /* DSNP NAP002 */
/* 109 */ "TRM53406.00     NONE",       /* Trimble A3 */
/* 110 */ "ASH701933C_M    NONE",       /* Ashtech 701933 */
/* 111 */ "ASH701933C_M    SNOW",       /* Ashtech 701933 w/Radome */
/* 112 */ "ASH701941.1     NONE",       /* Ashtech 701941.021 */
/* 113 */ "ASH701975.01A   NONE",       /* Ashtech Geodetic IV */
/* 114 */ "ASH701975.01AGP NONE",       /* Ashtech Geodetic IV w/Ground Plane */
/* 115 */ "NOV600          NONE",       /* NovAtel GPS-600 */
/* 116 */ "SOKA110         NONE",       /* Sokkia A110 */
/* 117 */ "SOKA120         NONE",       /* Sokkia A120 */
/* 118 */ "LEISR299_INT    NONE",       /* Leica SR299/SR399A Internal */
/* 119 */ "TRM26738.00     NONE",       /* Trimble Permanent L1 */
/* 120 */ "GeoXT Internal",             /* Trimble GeoXT Internal */
/* 121 */ "TRM47950.00     NONE",       /* Trimble MS980 Internal */
/* 122 */ "GeoXM Internal",             /* Trimble GeoXM Internal */
/* 123 */ "TRM41249.00     TZGD",       /* Trimble Zephyr Geodetic w/Radome */
/* 124 */ "External Mini",              /* Trimble External Mini */
/* 125 */ "Hurricane",                  /* Trimble Hurricane */
/* 126 */ "LEISR399_INT    NONE",       /* Leica SR399 Internal */
/* 127 */ "AERAT2775_42    NONE",       /* AeroAntenna AT2775-42 */
/* 128 */ "AERAT2775_43    NONE",       /* AeroAntenna AT2775-43 */
/* 129 */ "AERAT2775_43    SPKE",       /* AeroAntenna AT2775-43+rd */
/* 130 */ "AERAT2775_62    NONE",       /* AeroAntenna AT2775-62 */
/* 131 */ "AERAT2775_160   NONE",       /* AeroAntenna AT2775-160 */
/* 132 */ "ASH110454       NONE",       /* Ashtech ProAntenna L1 */
/* 133 */ "JNSMARANT_GGD   NONE",       /* Javad Positioning Systems Marant GGD */
/* 134 */ "MPL1230         NONE",       /* Micro Pulse MPL1230 */
/* 135 */ "MPL1370W        NONE",       /* Micro Pulse MPL1370W */
/* 136 */ "MPL_WAAS_2224NW NONE",       /* Micro Pulse MPLWAAS */
/* 137 */ "SOK502          NONE",       /* Sokkia 502 */
/* 138 */ "SOK600          NONE",       /* Sokkia 600 */
/* 139 */ "SOK_RADIAN_IS   NONE",       /* Sokkia Radian IS */
/* 140 */ "TPSCR3_GGD      NONE",       /* Topcon CR3 GGD */
/* 141 */ "TPSCR3_GGD      CONE",       /* Topcon CR3 GGD w/Radome */
/* 142 */ "TPSHIPER_GD     NONE",       /* Topcon HiPer GD */
/* 143 */ "TPSLEGANT_G     NONE",       /* Topcon Legant G */
/* 144 */ "TPSLEGANT3_UHF  NONE",       /* Topcon Legant 3 UHF */
/* 145 */ "TRM29659.00     UNAV",       /* Trimble Choke Ring w/Dome */
/* 146 */ "TRMR10          NONE",       /* Trimble R10 Internal */
/* 147 */ "TRMSPS985       NONE",       /* Trimble SPS985 Internal */
/* 148 */ "TRM59900.00     NONE",       /* Trimble GNSS-Ti Choke Ring */
/* 149 */ "NOV_WAAS_600    NONE",       /* NovAtel WAAS-600 */
/* 150 */ "Ag252 Internal Phat",        /* Trimble Ag252 Internal Phat */
/* 151 */ "TPSLEGANT2      NONE",       /* Topcon Legant 2 */
/* 152 */ "TPSCR4          NONE",       /* Topcon CR4 */
/* 153 */ "TPSCR4          CONE",       /* Topcon CR4 w/Radome */
/* 154 */ "TPSODYSSEY_I    NONE",       /* Topcon Odyssey */
/* 155 */ "TPSPG_A1        NONE",       /* Topcon PG-A1 */
/* 156 */ "TPSHIPER_LITE   NONE",       /* Topcon HiPer Lite */
/* 157 */ "TPSHIPER_PLUS   NONE",       /* Topcon HiPer Plus */
/* 158 */ "TRM33429.20+GP  UNAV",       /* Trimble Micro-centered L1/L2 Perm. w/UNAV Dome */
/* 159 */ "ASH701945.02B   SNOW",       /* Ashtech 701945 D/M Choke w/Snow Dome */
/* 160 */ "ASH701945.02B   SCIS",       /* Ashtech 701945 D/M Choke w/SCIS Dome */
/* 161 */ "ASH701945.02B   SCIT",       /* Ashtech 701945 D/M Choke w/SCIT Dome */
/* 162 */ "ASH700936D_M    SCIS",       /* Ashtech 700936 D/M Choke w/SCIS Dome */
/* 163 */ "LEIAT504        LEIS",       /* Leica AT504 w/LEIS Dome */
/* 164 */ "AERAT2775_41    NONE",       /* AeroAntenna AT2775-41 */
/* 165 */ "NOV702          NONE",       /* NovAtel 702 Rev 2.02 */
/* 166 */ "SOKSTRATUS      NONE",       /* Sokkia Stratus L1 */
/* 167 */ "AOARASCAL       NONE",       /* Alan Osborne Associates Rascal */
/* 168 */ "NAVAN2004T      NONE",       /* NavCom AN2004T */
/* 169 */ "NAVAN2008T      NONE",       /* NavCom AN2008T */
/* 170 */ "NAVSF2040G      NONE",       /* NavCom SF2040G */
/* 171 */ "NAVRT3010S      NONE",       /* NavCom RT3010S */
/* 172 */ "LEIAX1202       NONE",       /* Leica AX1202 */
/* 173 */ "THAZMAX         NONE",       /* Thales ZMax */
/* 174 */ "GeoXH Internal",             /* Trimble GeoXH Internal */
/* 175 */ "Pathfinder XB Internal",     /* Trimble Pathfinder XB Internal */
/* 176 */ "ProXT Internal",             /* Trimble ProXT Internal */
/* 177 */ "ProXH Internal",             /* Trimble ProXH Internal */
/* 178 */ "NOV702_3.00     NONE",       /* NovAtel 702 Rev 3.00 */
/* 179 */ "SOK702_3.00     NONE",       /* Sokkia 702 Rev 3.00 */
/* 180 */ "TPSPG_A1+GP     NONE",       /* Topcon PG-A1 w/GP */
/* 181 */ "TPSPG_A5        NONE",       /* Topcon PG-A5 */
/* 182 */ "TRM59900.00     SCIS",       /* Trimble GNSS-Ti Choke w/SCIS Dome */
/* 183 */ "Recon GPS CF Card Internal", /* Trimble Recon GPS CF Card Internal */
/* 184 */ "TRM55970.00     NONE",       /* Trimble Zephyr - Model 2 */
/* 185 */ "TRM55971.00     NONE",       /* Trimble Zephyr Geodetic 2 */
/* 186 */ "TRMR8_GNSS      NONE",       /* Trimble R8 GNSS/SPS88x Internal */
/* 187 */ "TRM67770.00     NONE",       /* Trimble MS99x Internal */
/* 188 */ "LEIAT503        LEIC",       /* Leica AT503 w/Choke Ring+Radome */
/* 189 */ "LEIAT503        NONE",       /* Leica AT503 w/Choke Ring */
/* 190 */ "SPP53406.90     NONE",       /* Spectra Precision EPOCH L1 */
/* 191 */ "TRM55971.00     TZGD",       /* Trimble Zephyr Geodetic 2 w/Dome */
/* 192 */ "SPP39105.90     NONE",       /* Spectra Precision EPOCH L1/L2 */
/* 193 */ "TRM57200.00     NONE",       /* Trimble Z Plus */
/* 194 */ "TRM55550.00     NONE",       /* Trimble GA510 */
/* 195 */ "TRM29659.00     SCIS",       /* Trimble Choke Ring w/SCIS Dome */
/* 196 */ "TRM29659.00     SCIT",       /* Trimble Choke Ring w/SCIT Dome */
/* 197 */ "TRM59800.00     SCIT",       /* Trimble GNSS Choke w/SCIT Dome */
/* 198 */ "TRMR4-2         NONE",       /* Trimble R4-2 Internal */
/* 199 */ "TRMR6           NONE",       /* Trimble R6 Internal */
/* 200 */ "Pathfinder XC Internal",     /* Trimble Pathfinder XC Internal */
/* 201 */ "AOAD/M_T        AUST",       /* Dorne Margolin D/M Model T w/AUST Dome */
/* 202 */ "AOAD/M_T        JPLA",       /* Dorne Margolin D/M Model T w/JPLA Dome */
/* 203 */ "TRM55971.00     SCIT",       /* Trimble Zephyr Geodetic 2 w/SCIT */
/* 204 */ "Juno Internal",              /* Trimble Juno Internal */
/* 205 */ "MPL_WAAS_2225NW NONE",       /* Micro Pulse MPLWAAS+L5 */
/* 206 */ "TRM41249.00     SCIT",       /* Trimble Zephyr Geodetic w/SCIT Dome */
/* 207 */ "TRM41249USCG    SCIT",       /* Trimble Zephyr Geodetic w/USCG SCIT Dome */
/* 208 */ "LEIAX1202A      NONE",       /* Leica AX1202A */
/* 209 */ "TRM59800.00     NONE",       /* Trimble GNSS Choke */
/* 210 */ "TRM59800.00     SCIS",       /* Trimble GNSS Choke w/SCIS Dome */
/* 211 */ "LEIAX1202GG     NONE",       /* Leica AX1202GG */
/* 212 */ "TPSCR.G3        NONE",       /* Topcon TPS CR.G3 */
/* 213 */ "TPSCR.G3        TPSH",       /* Topcon TPS CR.G3 w/TPSH */
/* 214 */ "TPSG3_A1        NONE",       /* Topcon TPS G3_A1 */
/* 215 */ "TPSG3_A1        TPSD",       /* Topcon TPS G3_A1 w/TPSD */
/* 216 */ "TPSGR3          NONE",       /* Topcon TPS GR3 */
/* 217 */ "TPSMAPANT_B     NONE",       /* Topcon TPSMAPANT_B */
/* 218 */ "TPSMG_A2        NONE",       /* Topcon TPSMG_A2 */
/* 219 */ "TPSPG_A1        TPSD",       /* Topcon TPSPG_A1 w/GP+TPSD */
/* 220 */ "TPSPG_A2        NONE",       /* Topcon TPS PG_A2 */
/* 221 */ "TPS_CR.3        SCIS",       /* Topcon TPSCR.3 w/SCIS */
/* 222 */ "TPS_CR4         SCIS",       /* Topcon TPS CR4 w/SCIS */
/* 223 */ "TPS_MC.A5       NONE",       /* Topcon TPS MC.A5 */
/* 224 */ "LEIAT504GG      NONE",       /* Leica LEI AT504GG */
/* 225 */ "LEIAT504GG      LEIS",       /* Leica LEI AT504GG w/LEIS */
/* 226 */ "LEIAT504GG      SCIS",       /* Leica LEI AT504GG w/SCIS */
/* 227 */ "LEIAT504GG      SCIT",       /* Leica LEI AT504GG w/SCIT */
/* 228 */ "LEIATX1230      NONE",       /* Leica LEI ATX1230 */
/* 229 */ "LEIATX1230GG    NONE",       /* Leica LEI ATX1230GG */
/* 230 */ "THA800961+REC   NONE",       /* Thales THA 800961+REC */
/* 231 */ "THA800961+RTK   NONE",       /* Thales THA 800961+RTK */
/* 232 */ "THA800961RECUHF NONE",       /* Thales THA 800961RECUHF */
/* 233 */ "THA800961RTKUHF NONE",       /* Thales THA 800961RTKUHF */
/* 234 */ "THANAP002       NONE",       /* Thales THA NAP002 */
/* 235 */ "NOV533          RADM",       /* NovAtel NOV 533 w/RADM */
/* 236 */ "NOV702L_1.01    NONE",       /* NovAtel NOV 702L_1.01 */
/* 237 */ "SOK702          NONE",       /* Sokkia SOK 702 */
/* 238 */ "SOK_GSR2700IS   NONE",       /* Sokkia SOK GSR2700IS */
/* 239 */ "ASH701975.01BGP NONE",       /* Ashtech ASH 701975.01BGP */
/* 240 */ "AERAT2775_42+CR NONE",       /* AeroAntenna AER AT2775-42 w/Chokering */
/* 241 */ "AERAT2775_150   NONE",       /* AeroAntenna AER AT2775-150 */
/* 242 */ "AERAT2775_159   NONE",       /* AeroAntenna AER AT2775-159 */
/* 243 */ "AERAT2775_159   SPKE",       /* AeroAntenna AER AT2775-159 w/SPKE */
/* 244 */ "AERAT2775_270   NONE",       /* AeroAntenna AER AT2775-270 */
/* 245 */ "Nomad Internal",             /* Trimble Nomad Internal */
/* 246 */ "GeoXH 2008 Internal",        /* Trimble GeoXH 2008 Internal */
/* 247 */ "ASH701933C_M    SCIS",       /* Ashtech 701933 w/SCIS Dome */
/* 248 */ "ASH701933C_M    SCIT",       /* Ashtech 701933 w/SCIT Dome */
/* 249 */ "TRM65212.00     NONE",       /* Trimble Zephyr - Model 2 Rugged */
/* 250 */ "TRM44530.00     NONE",       /* Trimble GA530 */
/* 251 */ "SPP77410.00     NONE",       /* Spectra Precision EPOCH 35 Internal */
/* 252 */ "TRMR4           NONE",       /* Trimble R4 Internal */
/* 253 */ "TRMR6-2         NONE",       /* Trimble R6-2 Internal */
/* 254 */ "RELNULLANTENNA  NONE",       /* Relative Null Antenna */
/* 255 */ "GPPNULLANTENNA  NONE",       /* AdV Null Antenna */
/* 256 */ "TRM29659.00     TCWD",       /* Trimble Choke Ring w/TCWD Dome */
/* 257 */ "LEIAT302+GP     NONE",       /* Leica AT302 w/Ground Plane */
/* 258 */ "JNSCR_C146-22-1 NONE",       /* Javad Navigation Systems JNSCR_C146-22-1 */
/* 259 */ "LEIAR25         NONE",       /* Leica AR25 */
/* 260 */ "LEIAR25         LEIT",       /* Leica AR25 w/LEIT Dome */
/* 261 */ "NOV702GG        NONE",       /* NovAtel 702GG */
/* 262 */ "3S-02-TSADM     NONE",       /* 3S Navigation 3S-02-TSADM */
/* 263 */ "LEIAT504        OLGA",       /* Leica AT504 w/OLGA Dome */
/* 264 */ "Yuma Internal",              /* Trimble Yuma Internal */
/* 265 */ "TRM57971.00     NONE",       /* Trimble Zephyr Geodetic 2 RoHS */
/* 266 */ "TRMAG25         NONE",       /* Trimble AG25 GNSS */
/* 267 */ "TRM57972.00     NONE",       /* Trimble Tornado */
/* 268 */ "TRM59800.80     NONE",       /* Trimble TRM59800-80 */
/* 269 */ "TRM59800.80     SCIS",       /* Trimble TRM59800-80 w/SCIS Dome */
/* 270 */ "MAG990596       NONE",       /* Magellan ProMark 500 */
/* 271 */ "LEIAX1203+GNSS  NONE",       /* Leica AX1203+GNSS */
/* 272 */ "TRMR8-4         NONE",       /* Trimble R8-4 Internal */
/* 273 */ "TRM60600.02     NONE",       /* Trimble AG15 */
/* 274 */ "Tempest",                    /* Trimble Tempest */
/* 275 */ "TRM59800.80     SCIT",       /* Trimble TRM59800-80 w/SCIT Dome */
/* 276 */ "TRM57971.00     SCIT",       /* Trimble Zephyr Geodetic 2 RoHS w/SCIT Dome */
/* 277 */ "TRM57971.00     TZGD",       /* Trimble Zephyr Geodetic 2 RoHS w/TZGD Dome */
/* 278 */ "ASH701945B_M    NONE",       /* Ashtech 701945B_M */
/* 279 */ "ASH701945C_M    NONE",       /* Ashtech 701945C_M */
/* 280 */ "ASH701945D_M    NONE",       /* Ashtech 701945D_M */
/* 281 */ "ASH701945E_M    NONE",       /* Ashtech 701945E_M */
/* 282 */ "ASH701945G_M    NONE",       /* Ashtech 701945G_M */
/* 283 */ "ASH701945B_M    SCIS",       /* Ashtech 701945B_M w/SCIS Dome */
/* 284 */ "ASH701945C_M    SCIS",       /* Ashtech 701945C_M w/SCIS Dome */
/* 285 */ "ASH701945D_M    SCIS",       /* Ashtech 701945D_M w/SCIS Dome */
/* 286 */ "ASH701945E_M    SCIS",       /* Ashtech 701945E_M w/SCIS Dome */
/* 287 */ "ASH701945G_M    SCIS",       /* Ashtech 701945G_M w/SCIS Dome */
/* 288 */ "ASH701945B_M    SCIT",       /* Ashtech 701945B_M w/SCIT Dome */
/* 289 */ "ASH701945C_M    SCIT",       /* Ashtech 701945C_M w/SCIT Dome */
/* 290 */ "ASH701945D_M    SCIT",       /* Ashtech 701945D_M w/SCIT Dome */
/* 291 */ "ASH701945E_M    SCIT",       /* Ashtech 701945E_M w/SCIT Dome */
/* 292 */ "ASH701945G_M    SCIT",       /* Ashtech 701945G_M w/SCIT Dome */
/* 293 */ "ASH701945B_M    SNOW",       /* Ashtech 701945B_M w/Snow Dome */
/* 294 */ "ASH701945C_M    SNOW",       /* Ashtech 701945C_M w/Snow Dome */
/* 295 */ "ASH701945D_M    SNOW",       /* Ashtech 701945D_M w/Snow Dome */
/* 296 */ "ASH701945E_M    SNOW",       /* Ashtech 701945E_M w/Snow Dome */
/* 297 */ "ASH701945G_M    SNOW",       /* Ashtech 701945G_M w/Snow Dome */
/* 298 */ "ASH701945.02B   UNAV",       /* Ashtech 701945.02B w/UNAV Dome */
/* 299 */ "ASH701945C_M    UNAV",       /* Ashtech 701945C_M w/UNAV Dome */
/* 300 */ "ASH701945D_M    UNAV",       /* Ashtech 701945D_M w/UNAV Dome */
/* 301 */ "ASH701945E_M    UNAV",       /* Ashtech 701945E_M w/UNAV Dome */
/* 302 */ "ASH701933B_M    SCIT",       /* Ashtech 701933B_M w/SCIT Dome */
/* 303 */ "ASH700936D_M    SCIT",       /* Ashtech 700936D_M w/SCIT Dome */
/* 304 */ "ASH700936D_M    CAFG",       /* Ashtech 700936D_M w/CAFG Dome */
/* 305 */ "ASH700936E_C    NONE",       /* Ashtech 700936E_C */
/* 306 */ "ASH700936E_C    SCIT",       /* Ashtech 700936E_C w/SCIT Dome */
/* 307 */ "ASH700936B_M    SNOW",       /* Ashtech 700936B_M w/SNOW Dome */
/* 308 */ "ASH700936B_M    SCIT",       /* Ashtech 700936B_M w/SCIT Dome */
/* 309 */ "TRMAV59         NONE",       /* Trimble AV59 */
/* 310 */ "AERAT1675_182   NONE",       /* AeroAntenna AT1675-182 */
/* 311 */ "ASH700936B_M    NONE",       /* Ashtech 700936B_M */
/* 312 */ "ASH700936C_M    NONE",       /* Ashtech 700936C_M */
/* 313 */ "ASH700936D_M    NONE",       /* Ashtech 700936D_M */
/* 314 */ "ASH700936E      NONE",       /* Ashtech 700936E */
/* 315 */ "ASH701073.1     NONE",       /* Ashtech 701073.1 */
/* 316 */ "ASH701073.3     NONE",       /* Ashtech 701073.3 */
/* 317 */ "TRM99810.00     NONE",       /* Trimble GA810 */
/* 318 */ "GeoXT 6000 Internal",        /* Trimble GeoXT 6000 Internal */
/* 319 */ "GeoXH 6000 Internal",        /* Trimble GeoXH 6000 Internal */
/* 320 */ "SOK_GSR2700ISX  NONE",       /* Sokkia SOK GSR2700ISX */
/* 321 */ "JAV_TRIUMPH-1   NONE",       /* Javad GNSS JAV TRIUMPH-1 */
/* 322 */ "TPSCR3_GGD      OLGA",       /* Topcon CR3 GGD w/OLGA */
/* 323 */ "LEIAR25.R3      NONE",       /* Leica AR25.R3 */
/* 324 */ "LEIAR25.R3      LEIT",       /* Leica AR25.R3 w/LEIT Dome */
/* 325 */ "AERAT1675_20W   SPKE",       /* AeroAntenna AT1675-20W w/SPKE Dome */
/* 326 */ "NAV_ANT3001BR   SPKE",       /* NavCom ANT3001BR w/SPKE Dome */
/* 327 */ "LEIAR10         NONE",       /* Leica AR10 */
/* 328 */ "LEIAS10         NONE",       /* Leica AS10 */
/* 329 */ "SPP68410_10     NONE",       /* Spectra Precision Epoch 50 Internal */
/* 330 */ "LEIGS15         NONE",       /* Leica GS15 */
/* 331 */ "LEIAR25.R4      NONE",       /* Leica AR25.R4 */
/* 332 */ "LEIAR25.R4      LEIT",       /* Leica AR25.R4 w/LEIT Dome */
/* 333 */ "LEIAR25.R4      SCIT",       /* Leica AR25.R4 w/SCIT Dome */
/* 334 */ "NOV750.R4       NONE",       /* NovAtel 750.R4 */
/* 335 */ "NOV750.R4       NOVS",       /* NovAtel 750.R4 w/NOVS Dome */
/* 336 */ "JAV_GRANT-G3T   NONE",       /* Javad GNSS JAV GRANT-G3T */
/* 337 */ "JAV_RINGANT_G3T NONE",       /* Javad GNSS JAV RINGANT-G3T */
/* 338 */ "JAVRINGANT_DM   NONE",       /* Javad GNSS JAV RINGANT-DM */
/* 339 */ "JAV_RINGANT_G3T JAVC",       /* Javad GNSS JAV RINGANT-G3T w/JAVC */
/* 340 */ "JAVRINGANT_DM   JVDM",       /* Javad GNSS JAV RINGANT-DM w/JVDM */
/* 341 */ "JAVRINGANT_DM   SCIS",       /* Javad GNSS JAV RINGANT-DM w/SCIS */
/* 342 */ "JAVRINGANT_DM   SCIT",       /* Javad GNSS JAV RINGANT-DM w/SCIT */
/* 343 */ "LEIMNA950GG     NONE",       /* Leica MNA950GG */
/* 344 */ "TPSCR.G3        SCIS",       /* Topcon TPS CR.G3 w/SCIS */
/* 345 */ "ASH701945C_M    PFAN",       /* Ashtech 701945C_M w/PFAN */
/* 346 */ "LEIATX1230+GNSS NONE",       /* Leica ATX1230+GNSS */
/* 347 */ "TRM_MS972       NONE",       /* Trimble MS972 */
/* 348 */ "MAG111406       NONE",       /* Magellan ProFLEX 500 Survey Antenna */
/* 349 */ "TRM_AV33        NONE",       /* Trimble AV33 */
/* 350 */ "AOAD/M_B        OSOD",       /* Allen Osborne Associates AOAD Model B w/OSOD Dome */
/* 351 */ "AOAD/M_T        OSOD",       /* Allen Osborne Associates AOAD Model T w/OSOD Dome */
/* 352 */ "ASH700936A_M    OSOD",       /* Ashtech 700936A_M w/OSOD Dome */
/* 353 */ "ASH700936D_M    OSOD",       /* Ashtech 700936D_M w/OSOD Dome */
/* 354 */ "ASH700936E      OSOD",       /* Ashtech 700936E w/OSOD Dome */
/* 355 */ "ASH700936F_C    OSOD",       /* Ashtech 700936F_C w/OSOD Dome */
/* 356 */ "701073.1        OSOD",       /* Ashtech 701073.1 w/OSOD Dome */
/* 357 */ "ASH701941.B     OSOD",       /* Ashtech 701941.B w/OSOD Dome */
/* 358 */ "701945B_M       OSOD",       /* Ashtech 701945B_M w/OSOD Dome */
/* 359 */ "701945C_M       OSOD",       /* Ashtech 701945C_M w/OSOD Dome */
/* 360 */ "701945E_M       OSOD",       /* Ashtech 701945E_M w/OSOD Dome */
/* 361 */ "701946.3        OSOD",       /* Ashtech 701946.3 w/OSOD Dome */
/* 362 */ "JAVRINGANT_DM   OSOD",       /* Javad GNSS RINGANT-DM w/OSOD Dome */
/* 363 */ "JNSCR_C146-22-1 OSOD",       /* Javad Navigation Systems JNSCR_C146-22-1 w/OSOD DOme */
/* 364 */ "ASH700936F_C    NONE",       /* Ashtech 700936F_C */
/* 365 */ "ASH700936E      SNOW",       /* Ashtech 700936E w/SNOW Dome */
/* 366 */ "ASH701941.B     NONE",       /* Ashtech 701941.B */
/* 367 */ "ASH701946.3     NONE",       /* Ashtech 701946.3 */
/* 368 */ "ASH700936C_M    SNOW",       /* Ashtech 700936C_M w/SNOW Dome */
/* 369 */ "ASH700936D_M    SNOW",       /* Ashtech 700936D_M w/SNOW Dome */
/* 370 */ "ASH700936E_C    SNOW",       /* Ashtech 700936E_C w/SNOW Dome */
/* 371 */ "Controller Internal",        /* Trimble Controller Internal */
/* 372 */ "GeoXR 6000 Internal",        /* Trimble GeoXR 6000 Internal */
/* 373 */ "ASH700936B_M    OSOD",       /* Ashtech 700936B_M w/OSOD Dome */
/* 374 */ "ASH701933B_M    NONE",       /* Ashtech 701933B_M */
/* 375 */ "ASH701933B_M    SNOW",       /* Ashtech 701933B_M w/SNOW Dome */
/* 376 */ "ASH701933B_M    OSOD",       /* Ashtech 701933B_M w/OSOD Dome */
/* 377 */ "JNSCHOKERING_DM NONE",       /* Javad Navigation Systems JNSCHOKERING_DM */
/* 378 */ "JNSCHOKERING_DM OSOD",       /* Javad Navigation Systems JNSCHOKERING_DM w/OSOD DOme */
/* 379 */ "TRMSPS585       NONE",       /* Trimble SPS585 Internal */
/* 380 */ "TRM44530R.00    NONE",       /* Trimble Rugged GA530 */
/* 381 */ "TRMR6-3         NONE",       /* Trimble R6-3 Internal */
/* 382 */ "RNG80971.00     NONE",       /* Rusnavgeoset RNG80971.00 */
/* 383 */ "RNG80971.00     SCIT",       /* Rusnavgeoset RNG80971.00 w/SCIT Dome */
/* 384 */ "RNG80971.00     TZGD",       /* Rusnavgeoset RNG80971.00 w/TZGD Dome */
/* 385 */ "TPSCR.G3        SCIT",       /* Topcon TPS CR.G3 w/SCIT */
/* 386 */ "ASH111660       NONE",       /* Ashtech 111660 */
/* 387 */ "ASH111661       NONE",       /* Ashtech 111661 */
/* 388 */ "ASH802129       NONE",       /* Ashtech ProMark 500 Galileo */
/* 389 */ "LEIAR25.R3      SCIS",       /* Leica AR25.R3 w/SCIS Dome */
/* 390 */ "LEIAR25.R3      SCIT",       /* Leica AR25.R3 w/SCIT Dome */
/* 391 */ "ASH802147_A     NONE",       /* Spectra Precision ProMark 800 */
/* 392 */ "APSAPS-3        NONE",       /* Altus APS-3 */
/* 393 */ "TPSHIPER_II     NONE",       /* Topcon HiPer II */
/* 394 */ "Tempest Rev. B",             /* Trimble Tempest Rev. B */
/* 395 */ "Pro 6H Internal",            /* Trimble Pro 6H Internal */
/* 396 */ "Pro 6T Internal",            /* Trimble Pro 6T Internal */
/* 397 */ "ASH701941.B     SCIS",       /* Ashtech 701941.B w/SCIS Dome */
/* 398 */ "TPSCR.G5        NONE",       /* Topcon CR.G5 */
/* 399 */ "TPSCR.G5        SCIS",       /* Topcon CR.G5 w/SCIS Dome */
/* 400 */ "TPSCR.G5        SCIT",       /* Topcon CR.G5 w/SCIT Dome */
/* 401 */ "TPSCR.G5        TPSH",       /* Topcon CR.G5 w/TPSH Dome */
/* 402 */ "STXS9SA7224V3.0 NONE",       /* STONEX S9II GNSS Internal */
/* 403 */ "TRM_AV34        NONE",       /* Trimble AV34 */
/* 404 */ "TRMAV37         NONE",       /* Trimble AV37 */
/* 405 */ "AERAT1675_80    NONE",       /* AeroAntenna AT1675-80 */
/* 406 */ "TRMLV59         NONE",       /* Trimble LV59 */
/* 407 */ "AERAT1675_382   NONE",       /* AeroAntenna AT1675-382 */
/* 408 */ "ASH802111       NONE",       /* Ashtech MobileMapper 100 Internal */
/* 409 */ "TRMGEO5T        NONE",       /* Trimble Geo 5T Internal */
/* 410 */ "TRM57970.00     NONE",       /* Trimble Zephyr - Model 2 RoHS */
/* 411 */ "TPSGR5          NONE",       /* Topcon GR5 */
/* 412 */ "TRMAG_342       NONE",       /* Trimble AG-342 Internal */
/* 413 */ "HEMS320         NONE",       /* Hemisphere S320 */
/* 414 */ "ACC123CGNSSA_XN NONE",       /* Antcom 123CGNSSA-XN */
/* 415 */ "ACC2G1215A_XT_1 NONE",       /* Antcom 2G1215A-XT-1 */
/* 416 */ "ACC3G1215A_XT_1 NONE",       /* Antcom 3G1215A-XT-1 */
/* 417 */ "ACC42G1215A_XT1 NONE",       /* Antcom 42G1215A-XT1 */
/* 418 */ "ACC4G1215A_XT_1 NONE",       /* Antcom 4G1215A-XT-1 */
/* 419 */ "ACC53G1215A_XT1 NONE",       /* Antcom 53G1215A-XT1 */
/* 420 */ "ACC53GO1215AXT1 NONE",       /* Antcom 53GO1215AXT1 */
/* 421 */ "ACC72CGNSSA     NONE",       /* Antcom 72CGNSSA */
/* 422 */ "ACC72GNSSA_XT_1 NONE",       /* Antcom 72GNSSA-XT-1 */
/* 423 */ "ACCG3ANT_3AT1   NONE",       /* Antcom G3ANT-3AT1 */
/* 424 */ "ACCG3ANT_42AT   NONE",       /* Antcom G3ANT-42AT1 */
/* 425 */ "ACCG3ANT_52AT   NONE",       /* Antcom G3ANT-52AT1 */
/* 426 */ "ACCG5ANT_123CAN NONE",       /* Antcom G5ANT_123CAN */
/* 427 */ "ACCG5ANT_2AT1   NONE",       /* Antcom G5ANT_2AT1 */
/* 428 */ "ACCG5ANT_3AT1   NONE",       /* Antcom G5ANT_3AT1 */
/* 429 */ "ACCG5ANT_42AT1  NONE",       /* Antcom G5ANT_42AT1 */
/* 430 */ "ACCG5ANT_52AT1  NONE",       /* Antcom G5ANT_52AT1 */
/* 431 */ "ACCG5ANT_72AT1  NONE",       /* Antcom G5ANT_72AT1 */
/* 432 */ "MAG105645       NONE",       /* Magellan L1/L2 GPS Antenna */
/* 433 */ "CHCA300GNSS     NONE",       /* CHC A300GNSS */
/* 434 */ "CHCX900B        NONE",       /* CHC X900B */
/* 435 */ "CHCX900R        NONE",       /* CHC X900R */
/* 436 */ "CHCX91B         NONE",       /* CHC X91B */
/* 437 */ "CHCX91R         NONE",       /* CHC X91R */
/* 438 */ "TRMR6-4         NONE",       /* Trimble R6-4 Internal */
/* 439 */ "TRMR4-3         NONE",       /* Trimble R4-3 Internal */
/* 440 */ "SPP89823_10     NONE",       /* Spectra Precision ProMark 700 */
/* 441 */ "NOV750.R4       SCIT",       /* NovAtel 750.R4 w/SCIT Dome */
/* 442 */ "NOV750.R4       SCIS",       /* NovAtel 750.R4 w/SCIS Dome */
/* 443 */ "NOV702L_1.03    NONE",       /* NovAtel 702L 1.03 */
/* 444 */ "NOV702GGL_1.01  NONE",       /* NovAtel 702-GGL 1.01 */
/* 445 */ "NOV702GG_1.02   NONE",       /* NovAtel 702-GG 1.02 */
/* 446 */ "NOV702GG_1.03   NONE",       /* NovAtel 702-GG 1.03 */
/* 447 */ "NOV701GGL       NONE",       /* NovAtel 701-GGL */
/* 448 */ "NOV701GG_1.03   NONE",       /* NovAtel 701-GG 1.03 */
/* 449 */ "SOKGRX1         NONE",       /* Sokkia GRX1 */
/* 450 */ "SOKGRX1+10      NONE",       /* Sokkia GRX1 + 10cm Standoff */
/* 451 */ "SOKGRX2         NONE",       /* Sokkia GRX2 */
/* 452 */ "SOKGRX2+10      NONE",       /* Sokkia GRX2 + 10cm Standoff */
/* 453 */ "SOKLOCUS        NONE",       /* Sokkia Locus */
/* 454 */ "HITV30          NONE",       /* Hi-Target V30 GNSS */
/* 455 */ "MAGNAP100       NONE",       /* Magellan NAP100 */
/* 456 */ "AERAT1675_29    NONE",       /* AeroAntenna AT1675-29 */
/* 457 */ "JAV_TRIUMPH-1R  NONE",       /* Javad GNSS TRIUMPH-1R */
/* 458 */ "SEPCHOKE_MC     NONE",       /* Septentrio Choke MC */
/* 459 */ "SEPCHOKE_MC     SPKE",       /* Septentrio Choke MC w/SPKE Dome */
/* 460 */ "TRM77970.00     NONE",       /* Trimble Zephyr - Model 2 US/CAN */
/* 461 */ "TRM77971.00     NONE",       /* Trimble Zephyr Geodetic 2 US/CAN */
/* 462 */ "LEIGS08         NONE",       /* Leica GS08 */
/* 463 */ "LEIGS08PLUS     NONE",       /* Leica GS08plus */
/* 464 */ "LEIGS09         NONE",       /* Leica GS09 */
/* 465 */ "LEIGS12         NONE",       /* Leica GS12 */
/* 466 */ "LEIGS14         NONE",       /* Leica GS14 */
/* 467 */ "Geo 7X Internal",            /* Trimble Geo 7X Internal */
/* 468 */ "SPP91564_1      NONE",       /* Spectra Precision SP80 */
/* 469 */ "SPP91564_2      NONE",       /* Spectra Precision SP80 UHF */
/* 470 */ "TRM44830.00     NONE",       /* Trimble GA830 */
/* 471 */ NULL,
/* 472 */ NULL,
/* 473 */ "TPSHIPER_SR     NONE",       /* Topcon HiPer SR */
/* 474 */ "JAVTRIUMPH-VS   NONE",       /* Javad GNSS Triumph-VS */
/* 475 */ NULL,
/* 476 */ NULL,
/* 477 */ "AERAT1675_180   NONE",       /* AeroAntenna AT1675-180 */
/* 478 */ "TRMAV39         NONE",       /* Trimble AV39 */
/* 479 */ "LEIAR20         NONE",       /* Leica AR20 */
/* 480 */ "LEIAR20         LEIM",       /* Leica AR20 w/LEIM Dome */
/* 481 */ "TPSHIPER_V      NONE",       /* Topcon HiPer V */
/* 482 */ "AERAT1675_120   NONE",       /* AeroAntenna AT1675-120 */
/* 483 */ "AERAT1675_120   SPKE",       /* AeroAntenna AT1675-120 w/SPKE Dome */
/* 484 */ "R1/PG200 Internal",          /* Trimble R1/PG200 Internal */
/* 485 */ "ITT3750323      NONE",       /* ITT 3750323 */
/* 486 */ "ITT3750323      SCIS",       /* ITT 3750323 w/SCIS Dome */
/* 487 */ "TPSPG_A1_6      NONE",       /* Topcon PG-A1-6 */
/* 488 */ "TPSPG_A1_6+GP   NONE",       /* Topcon PG-A1-6 w/GP */
/* 489 */ "TPSPG_F1        NONE",       /* Topcon PG-F1 */
/* 490 */ "TPSPG_F1+GP     NONE",       /* Topcon PG-F1 w/GP */
/* 491 */ "TPSPG_S1        NONE",       /* Topcon PG-S1 */
/* 492 */ "TPSPG_S1+GP     NONE",       /* Topcon PG-S1 w/GP */
/* 493 */ "TPSPN.A5        NONE",       /* Topcon PN-A5 */
/* 494 */ "TPSPN.A5        SCIS",       /* Topcon PN-A5 w/SCIS */
/* 495 */ "TPSPN.A5        SCIT",       /* Topcon PN-A5 w/SCIT */
/* 496 */ "TPSPN.A5        TPSH",       /* Topcon PN-A5 w/TPSH */
/* 497 */ "TPSHIPER_XT     NONE",       /* Topcon HiPer XT */
/* 498 */ "TPSHIPER_II+10  NONE",       /* Topcon HiPer II + 10cm Standoff */
/* 499 */ "TPSHIPER_SR+10  NONE",       /* Topcon HiPer SR + 10cm Standoff */
/* 500 */ "TPSHIPER_V+10   NONE",       /* Topcon HiPer V + 10cm Standoff */
/* 501 */ "TPSCR3_GGD      PFAN",       /* Topcon CR3 GGD w/PFAN */
/* 502 */ NULL,
/* 503 */ NULL,
/* 504 */ NULL,
/* 505 */ NULL,
/* 506 */ NULL,
/* 507 */ "HXCGG486A       NONE",       /* Harxon HX-GG486A */
/* 508 */ "HXCGG486A       HXCS",       /* Harxon HX-GG486A w/HXCS */
/* 509 */ "HXCGS488A       NONE",       /* Harxon HX-GS488A */
/* 510 */ "SPP98147_10     NONE",       /* Spectra Precision MobileMapper 300 */
/* 511 */ "AERAT1675_219   NONE",       /* AeroAntenna AT1675-219 */
/* 512 */ NULL,
/* 513 */ NULL,
/* 514 */ "STXS9PX001A     NONE",       /* STONEX S9III+ GNSS Internal */
/* 515 */ "STXG5ANT_72AT1  NONE",       /* STONEX G5Ant_72AT1 */
/* 516 */ "TRMR2           NONE",       /* Trimble R2 Internal */
/* 517 */ "SEPPOLANT_X_MF  NONE",       /* Septentrio PolaNt-x MF */
/* 518 */ "SEPPOLANT_X_SF  NONE",       /* Septentrio PolaNt-x SF */
/* 519 */ "SEP_POLANT+     NONE",       /* Septentrio PolaNt* */
/* 520 */ "SEP_POLANT+_GG  NONE",       /* Septentrio PolaNt* GG */
/* 521 */ "TRMAT1675_540TS NONE",       /* Trimble AT1675-540TS */
/* 522 */ NULL,
/* 523 */ "UX5_HP          NONE",       /* Trimble UX5 HP Internal GNSS */
/* 524 */ "TRM77971.00     SCIT",       /* Trimble Zephyr Geodetic 2 US/CAN w/SCIT */
/* 525 */ "TPSLEGANT       NONE",       /* Topcon Legant */
/* 526 */ "TPSHIPER_GGD    NONE",       /* Topcon HiPer GGD */
/* 527 */ "TRMR8S          NONE",       /* Trimble R8s Internal */
/* 528 */ "STH82VHX-BS601A NONE",       /* SOUTH S82V2 GNSS */
/* 529 */ "STHS82HX-BS601A NONE",       /* SOUTH S82-2013 GNSS */
/* 530 */ "STHS82_7224V3.0 NONE",       /* SOUTH S82T/S82V GNSS */
/* 531 */ "STHS86HX-BS611A NONE",       /* SOUTH S86-2013 GNSS */
/* 532 */ "STHS86_7224V3.1 NONE",       /* SOUTH S86 GNSS */
/* 533 */ "TRMAG_372       NONE",       /* Trimble AG-372 Internal */
/* 534 */ "HXCCG7601A      NONE",       /* Harxon HX-CG7601A */
/* 535 */ "HXCCG7601A      HXCG",       /* Harxon HX-CG7601A w/HXCG */
/* 536 */ "HXCCGX601A      NONE",       /* Harxon HX-CGX601A */
/* 537 */ "HXCCGX601A      HXCS",       /* Harxon HX-CGX601A w/HXCS */
/* 538 */ "HXCCSX601A      NONE",       /* Harxon HX-CSX601A */
/* 539 */ "HXCCA7607A      NONE",       /* Harxon HX-CA7607A */
/* 540 */ "HXCCG7602A      NONE",       /* Harxon HX-CG7602A */
/* 541 */ "HXCCG7602A      HXCG",       /* Harxon HX-CG7602A w/HXCG */
/* 542 */ "LEIICG60        NONE",       /* Leica ICG60 */
/* 543 */ "CHCA220GR       NONE",       /* CHC A220GR */
/* 544 */ "CHCC220GR       NONE",       /* CHC C220GR */
/* 545 */ "CHCC220GR       CHCD",       /* C220GR w/CHCD */
/* 546 */ "CHCX90D-OPUS    NONE",       /* CHC X90D-OPUS */
/* 547 */ "CHCX91+S        NONE",       /* CHC X91+S */
/* 548 */ NULL,
/* 549 */ NULL,
/* 550 */ "JAVTRIUMPH_2A   NONE",       /* Javad GNSS TRIUMPH-2 */
/* 551 */ "JAVTRIUMPH_LS   NONE",       /* Javad GNSS TRIUMPH-LS */
/* 552 */ "ACCG8ANT-CHOKES NONE",       /* Antcom G8ANT-CHOKES */
/* 553 */ "ACCG8ANT_3A4TB1 NONE",       /* Antcom G8ANT_3A4TB1 */
/* 554 */ "ACCG8ANT_3A4_M1 NONE",       /* Antcom G8ANT_3A4_M1 */
/* 555 */ "ACCG8ANT_52A4T1 NONE",       /* Antcom G8ANT_52A4T1 */
/* 556 */ "ACCG8ANT_52A4TC NONE",       /* Antcom G8ANT_52A4TC */
/* 557 */ "SPP101861       NONE",       /* Spectra Precision SP60 */
/* 558 */ NULL,
/* 559 */ "GMXZENITH20     NONE",       /* GeoMax ZENITH 20 */
/* 560 */ "NAX3G+C         NONE",       /* NavXperience 3G+C */
/* 561 */ "CHANV3          NONE",       /* Champion Instruments NV3 */
/* 562 */ "CHATKO          NONE",       /* Champion Instruments TKO GNSS */
/* 563 */ "TRMBEACONIM3_NF NONE",       /* Trimble Beacon IM3-NF */
/* 564 */ "RAVMBA2_NF      NONE",       /* Raven MBA-2 NF */
/* 565 */ "RAVMBA2_FF      NONE",       /* Raven MBA-2 FF */
/* 566 */ NULL,
/* 567 */ NULL,
/* 568 */ "SLGSL600_V1     NONE",       /* Satlab SL600 V1 */
/* 569 */ NULL,
/* 570 */ NULL,
/* 571 */ "STXS10SX017A    NONE",       /* STONEX S10 Internal */
};

/*
| CMR+ protocol defined receiver number to name lookup table.
|
| Adapted from "c:\program files (x86)\common files\trimble\config\receiver.ini"
| after installing the latest Trimble Office Configuration File Update Utility
| found at <http://www.trimble.com/infrastructure/trimbleconfiguration_ts.aspx>
| For each receiver definition block in receiver.ini the "RxId" keyword provides
| the receiver number and the "IGSReceiver" keyword provides the receiver name.
| A handful of receivers also have an "OldRxId" entry which provides an additional
| number with the same name.
*/
static const char *ReceiversTable[] = {
/* 000 */ NULL,
/* 001 */ "TRIMBLE 4000A",
/* 002 */ "TRIMBLE 4000ST",
/* 003 */ "TRIMBLE 4000SE",
/* 004 */ "TRIMBLE 4000SE",
/* 005 */ "TRIMBLE 4000SI",
/* 006 */ "TRIMBLE 4400",
/* 007 */ "TRIMBLE 4600",
/* 008 */ "TRIMBLE MSGR",
/* 009 */ "TRIMBLE 4800",
/* 010 */ "TRIMBLE 4000A",
/* 011 */ "TRIMBLE 4000SX",
/* 012 */ "TRIMBLE 4000SLD",
/* 013 */ "TRIMBLE 4000ST",
/* 014 */ "TRIMBLE 4000SST",
/* 015 */ "TRIMBLE 4000SE",
/* 016 */ "TRIMBLE 4000SE",
/* 017 */ "TRIMBLE 4000SSE",
/* 018 */ "TRIMBLE 4000SI",
/* 019 */ "TRIMBLE 4000SSI",
/* 020 */ "TRIMBLE 4400",
/* 021 */ "TRIMBLE 4600",
/* 022 */ "TRIMBLE MSGR",
/* 023 */ "TRIMBLE 4800",
/* 024 */ "TRIMBLE 4700",
/* 025 */ "TRIMBLE MS750",
/* 026 */ "TRIMBLE 5700",
/* 027 */ "TRIMBLE 5800",
/* 028 */ "TRIMBLE MS980",
/* 029 */ "TRIMBLE NETRS",
/* 030 */ "TRIMBLE BD950",
/* 031 */ "TRIMBLE R7",
/* 032 */ "TRIMBLE R8",
/* 033 */ "TRIMBLE R3",
/* 034 */ "TRIMBLE SPS750",
/* 035 */ "TRIMBLE NETR9 GEO",
/* 036 */ "TRIMBLE SPS780",
/* 037 */ "TRIMBLE SPS770",
/* 038 */ "TRIMBLE SPS850",
/* 039 */ "TRIMBLE SPS880",
/* 040 */ "TRIMBLE EPOCH10",
/* 041 */ "TRIMBLE SPS651",
/* 042 */ "TRIMBLE SPS550",
/* 043 */ "TRIMBLE MS990",
/* 044 */ "TRIMBLE R8 GNSS",
/* 045 */ "TRIMBLE NETR5",
/* 046 */ "TRIMBLE SPS550H",
/* 047 */ "TRIMBLE EPOCH25",
/* 048 */ "TRIMBLE R4-2",
/* 049 */ "TRIMBLE AGRTKBASE",
/* 050 */ "TRIMBLE R7 GNSS",
/* 051 */ "TRIMBLE R6",
/* 052 */ "TRIMBLE R8-4",
/* 053 */ "TRIMBLE R6-2",
/* 054 */ "TRIMBLE SPS781",
/* 055 */ "TRIMBLE SPS881",
/* 056 */ "TRIMBLE SPS551",
/* 057 */ "TRIMBLE SPS551H",
/* 058 */ "TRIMBLE SPS751",
/* 059 */ "TRIMBLE SPS851",
/* 060 */ "TRIMBLE AGGPS432",
/* 061 */ "TRIMBLE AGGPS442",
/* 062 */ "TRIMBLE BD960",
/* 063 */ "TRIMBLE NETR8",
/* 064 */ "TRIMBLE 5800II",
/* 065 */ "TRIMBLE SPS351",
/* 066 */ "TRIMBLE SPS361",
/* 067 */ "TRIMBLE PROXRT",
/* 068 */ "TRIMBLE NETR3",
/* 069 */ "TRIMBLE 5700II",
/* 070 */ "TRIMBLE SPS461",
/* 071 */ "TRIMBLE R8 GNSS3",
/* 072 */ "TRIMBLE SPS882",
/* 073 */ "TRIMBLE 9200-G2",
/* 074 */ "TRIMBLE MS992",
/* 075 */ "TRIMBLE BD970",
/* 076 */ "TRIMBLE NETR9",
/* 077 */ "TRIMBLE BD982",
/* 078 */ "TRIMBLE 9205 GNSS",
/* 079 */ "TRIMBLE GEOXR 6000",
/* 080 */ "TRIMBLE R6-3",
/* 081 */ "TRIMBLE GEOXH2008",
/* 082 */ "TRIMBLE JUNOST",
/* 083 */ "TRIMBLE PFXC",
/* 084 */ "TRIMBLE RECONGPSCF",
/* 085 */ "TRIMBLE PROXH",
/* 086 */ "TRIMBLE PROXT",
/* 087 */ "TRIMBLE PFXB",
/* 088 */ "TRIMBLE GEOXH",
/* 089 */ "TRIMBLE GEOXM",
/* 090 */ "TRIMBLE GEOXT",
/* 091 */ "TRIMBLE PROXRT-3",
/* 092 */ "TRIMBLE PFPOWER",
/* 093 */ "TRIMBLE AGL2",
/* 094 */ "TRIMBLE GEOEXPLORER3",
/* 095 */ "TRIMBLE PROXRS",
/* 096 */ "TRIMBLE PROXR",
/* 097 */ "TRIMBLE PROXL",
/* 098 */ "TRIMBLE GEOEXPLORER",
/* 099 */ "TRIMBLE PFBASIC",
/* 100 */ "TRIMBLE R10",
/* 101 */ "TRIMBLE SPS985",
/* 102 */ "TRIMBLE MS952",
/* 103 */ "RNG FASA+",
/* 104 */ NULL,
/* 105 */ "TRIMBLE SPS552H",
/* 106 */ "TRIMBLE MS972",
/* 107 */ "TRIMBLE SPS852",
/* 108 */ "TRIMBLE PROXRT-2",
/* 109 */ "TRIMBLE BD910",
/* 110 */ "TRIMBLE BD920",
/* 111 */ "TRIMBLE BD930",
/* 112 */ "TRIMBLE AGGPS542",
/* 113 */ "TRIMBLE EPOCH35GNSS",
/* 114 */ "TRIMBLE R4",
/* 115 */ "TRIMBLE R5",
/* 116 */ "TRIMBLE EPOCH50",
/* 117 */ "TRIMBLE MS352",
/* 118 */ "TRIMBLE SPS855",
/* 119 */ "TRIMBLE SPS555H",
/* 120 */ "LEICA L1",
/* 121 */ "LEICA L2",
/* 122 */ "WILD WM101",
/* 123 */ "WILD WM102",
/* 124 */ "LEICA SR261",
/* 125 */ "LEICA SR299",
/* 126 */ "LEICA SR399",
/* 127 */ "LEICA SR9400",
/* 128 */ "LEICA SR9500",
/* 129 */ "LEICA CRS1000",
/* 130 */ "LEICA SR510",
/* 131 */ "LEICA SR520",
/* 132 */ "LEICA SR530",
/* 133 */ "LEICA MC500",
/* 134 */ "LEICA GPS1200",
/* 135 */ "LEICA RS500",
/* 136 */ "TRIMBLE R8S",
/* 137 */ "TRIMBLE AG-342",
/* 138 */ "TRIMBLE SPS356",
/* 139 */ "TRIMBLE DELTA7",
/* 140 */ "ASHTECH L1",
/* 141 */ "ASHTECH L2",
/* 142 */ "ASHTECH DIMENSION",
/* 143 */ "ASHTECH L-XII",
/* 144 */ "ASHTECH M-XII",
/* 145 */ "ASHTECH P-XII3",
/* 146 */ "ASHTECH Z-XII3",
/* 147 */ "ASHTECH GG24C",
/* 148 */ "ASHTECH UZ-12",
/* 149 */ "ASHTECH ICGRS",
/* 150 */ "TRIMBLE GEO 5T",
/* 151 */ "TRIMBLE R6-4",
/* 152 */ "TRIMBLE R4-3",
/* 153 */ "SPP PROMARK700",
/* 154 */ "TRIMBLE NETR9 TI-M",
/* 155 */ "TRIMBLE M7-PPS",
/* 156 */ "TRIMBLE APX-15",
/* 157 */ NULL,
/* 158 */ NULL,
/* 159 */ "TRIMBLE MX100",
/* 160 */ "TOPCON GP-R1",
/* 161 */ "TOPCON GP-R1D",
/* 162 */ NULL,
/* 163 */ NULL,
/* 164 */ NULL,
/* 165 */ NULL,
/* 166 */ NULL,
/* 167 */ NULL,
/* 168 */ "TRIMBLE R9S",
/* 169 */ NULL,
/* 170 */ "DELNORTE 3009",
/* 171 */ "SOK RADIAN",
/* 172 */ "SOK GSR2600",
/* 173 */ NULL,
/* 174 */ NULL,
/* 175 */ NULL,
/* 176 */ NULL,
/* 177 */ NULL,
/* 178 */ NULL,
/* 179 */ NULL,
/* 180 */ "GEOD GEOTRACER2000",
/* 181 */ "SPP GPSMODULEL1",
/* 182 */ "SPP GEOTRACERL1L2",
/* 183 */ NULL,
/* 184 */ NULL,
/* 185 */ "NOV MILLEN-STD",
/* 186 */ "NIKON LOGPAK",
/* 187 */ "NIKON LOGPAKII",
/* 188 */ NULL,
/* 189 */ NULL,
/* 190 */ NULL,
/* 191 */ NULL,
/* 192 */ NULL,
/* 193 */ NULL,
/* 194 */ NULL,
/* 195 */ "ZEISS EXPERIENCE",
/* 196 */ "LEICA GRX1200GGPRO",
/* 197 */ "TPS NETG3",
/* 198 */ "LEICA GRX1200+GNSS",
/* 199 */ "TPS NET-G3A",
/* 200 */ "LITTON MINIMAC",
/* 201 */ NULL,
/* 202 */ NULL,
/* 203 */ NULL,
/* 204 */ NULL,
/* 205 */ NULL,
/* 206 */ NULL,
/* 207 */ NULL,
/* 208 */ NULL,
/* 209 */ NULL,
/* 210 */ NULL,
/* 211 */ NULL,
/* 212 */ NULL,
/* 213 */ NULL,
/* 214 */ NULL,
/* 215 */ NULL,
/* 216 */ NULL,
/* 217 */ NULL,
/* 218 */ NULL,
/* 219 */ NULL,
/* 220 */ "TOPCON GP-SX1",
/* 221 */ "TOPCON GP-DX1",
/* 222 */ "NGS NETSURV1000",
/* 223 */ "NGS NETSURV1000L",
/* 224 */ "NGS NETSURV2000",
/* 225 */ NULL,
/* 226 */ NULL,
/* 227 */ NULL,
/* 228 */ NULL,
/* 229 */ NULL,
/* 230 */ "JPS LEGACY",
/* 231 */ "JPS REGENCY",
/* 232 */ "JPS ODYSSEY",
/* 233 */ "TPS HIPER_GD",
/* 234 */ "JPS PREGO",
/* 235 */ "JPS E_GGD",
/* 236 */ "JAVAD TRE_G3TH DELTA",
/* 237 */ NULL,
/* 238 */ NULL,
/* 239 */ "SPP MOBILEMAPPER300",
/* 240 */ "TRIMBLE BD935",
/* 241 */ NULL,
/* 242 */ "TRIMBLE SPS985L",
/* 243 */ "TRIMBLE CPS205",
/* 244 */ "TRIMBLE M7-SPS",
/* 245 */ NULL,
/* 246 */ NULL,
/* 247 */ NULL,
/* 248 */ NULL,
/* 249 */ NULL,
/* 250 */ "TRIMBLE SPS585",
/* 251 */ "TRIMBLE BRAVO7",
/* 252 */ "TRIMBLE GEOXT6000",
/* 253 */ "TRIMBLE GEOXH6000",
/* 254 */ "VRS",
/* 255 */ "UNKNOWN",
/* 256 */ NULL,
/* 257 */ "LEICA 10",
/* 258 */ "TPS GB-1000",
/* 259 */ "TRIMBLE YUMA",
/* 260 */ "TRIMBLE TSC3",
/* 261 */ "ASHTECH PROMARK120",
/* 262 */ "ASHTECH PROMARK220",
/* 263 */ "ASHTECH PROMARK500",
/* 264 */ "ASHTECH PF500",
/* 265 */ "SPP PROMARK800",
/* 266 */ "ASHTECH PF800",
/* 267 */ "SOK GSR2700 RS",
/* 268 */ "SOK GSR2700 RSX",
/* 269 */ "SOK RADIAN_IS",
/* 270 */ "TRIMBLE PRO_6H",
/* 271 */ "TRIMBLE PRO_6T",
/* 272 */ "STONEX S9II GNSS",
/* 273 */ "ASHTECH MM100",
/* 274 */ "TPS GR5",
/* 275 */ "HEM S320",
/* 276 */ "TRIMBLE JUNO5",
/* 277 */ "SOK_GSR2700IS",
/* 278 */ "SOK_GSR2700ISX",
/* 279 */ "SOK_GRX1",
/* 280 */ "SOK_GRX2",
/* 281 */ "SOK_LOCUS",
/* 282 */ "TRIMBLE GEOXH6000_CM",
/* 283 */ "TRIMBLE GEO7T",
/* 284 */ "TRIMBLE GEO7H",
/* 285 */ "TRIMBLE GEO7HCM",
/* 286 */ "HITARGET V30",
/* 287 */ "TRIMBLE GEO7X",
/* 288 */ "LEICA GS08",
/* 289 */ "LEICA GS08PLUS",
/* 290 */ "LEICA GS09",
/* 291 */ "LEICA GS10",
/* 292 */ "LEICA GS12",
/* 293 */ "LEICA GS14",
/* 294 */ "LEICA GS15",
/* 295 */ "LEICA GS25",
/* 296 */ "JPS EGGDT",
/* 297 */ "BKG EGGDT",
/* 298 */ "LEICA GR25",
/* 299 */ "JAVAD TRE_G3TH SIGMA",
/* 300 */ "SPECTRA SP80",
/* 301 */ "TPSHIPER_SR",
/* 302 */ "JAVAD TR_VS",
/* 303 */ "TPSHIPER_V",
/* 304 */ "JAVAD TRE_G3T SIGMA",
/* 305 */ "TRIMBLE R1/PG200",
/* 306 */ "TRIMBLE SG160_0X",
/* 307 */ "TRIMBLE BD930_SG",
/* 308 */ "TRIMBLE BD982_SG",
/* 309 */ "STONEX S9III+ GNSS",
/* 310 */ "TRIMBLE R2"
};
