      SUBROUTINE GPT (DMJD,DLAT,DLON,DHGT,PRES,TEMP,UNDU)
*+
*  - - - - - - - - -
*   G P T 
*  - - - - - - - - -
*
*  This routine is part of the International Earth Rotation and
*  Reference Systems Service (IERS) Conventions software collection.
*
*  This subroutine determines Global Pressure and Temperature (Boehm et al. 2007)
*  based on Spherical Harmonics up to degree and order 9.
*
*  In general, Class 1, 2, and 3 models represent physical effects that
*  act on geodetic parameters while canonical models provide lower-level
*  representations or basic computations that are used by Class 1, 2, or
*  3 models.
* 
*  Status: Class 1 model	
* 
*     Class 1 models are those recommended to be used a priori in the
*     reduction of raw space geodetic data in order to determine
*     geodetic parameter estimates.
*     Class 2 models are those that eliminate an observational
*     singularity and are purely conventional in nature.
*     Class 3 models are those that are not required as either Class
*     1 or 2.
*     Canonical models are accepted as is and cannot be classified as a
*     Class 1, 2, or 3 model.
*
*  Given:
*     DMJD           d      Modified Julian Date
*     DLAT           d      Latitude given in radians (North Latitude)
*     DLON           d      Longitude given in radians (East Longitude)
*     DHGT           d      Ellipsoidal height in meters
*
*  Returned:
*     PRES           d      Pressure given in hPa
*     TEMP           d      Temperature in degrees Celsius
*     UNDU           d      Geoid undulation in meters (Note 1)
*
*  Notes:
*
*  1) This is from a 9x9 Earth Gravitational Model (EGM).
*
*  Test case:
*     given input: DMJD = 55055D0
*                  DLAT = 0.6708665767D0 radians (NRAO, Green Bank, WV)
*                  DLON = -1.393397187D0 radians
*                  DHGT = 812.546 meters
*     expected output: PRES = 919.1930225603726967D0 hPa 
*                      TEMP = 28.94460920276309679D0 degrees Celsius
*                      UNDU = -42.78796423912972813D0 meters
*
*  References:
*
*     Boehm, J., Heinkelmann, R. and Schuh, H., 2007, "Short Note: A 
*     Global model of pressure and temperature for geodetic applications",
*     Journal of Geodesy, 81(10), pp. 679-683.
*
*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010),
*     IERS Technical Note No. 36, BKG (2010)
*
*  Revisions:
*  2006 June 12 J. Boehm    Original code
*  2006 June 16 J. Boehm    Accounted for geoid undulation
*  2006 August 14 O. Montenbruck Recursions for Legendre polynomials
*  2009 February 13 B.E. Stetzler Added header and copyright
*  2009 March 30 B.E. Stetzler More modifications and defined twopi
*  2009 March 31 B.E. Stetzler Provided test case
*  2009 July  29 B.E. Stetzler Capitalized all variables for FORTRAN 77
*                              compatibility and corrected test case
*                              latitude and longitude coordinates
*-----------------------------------------------------------------------

      IMPLICIT NONE

      DOUBLE PRECISION DMJD,DLAT,DLON,DHGT,PRES,TEMP,UNDU

      DOUBLE PRECISION V(10,10),W(10,10),
     . AP_MEAN(55),BP_MEAN(55),AP_AMP(55),BP_AMP(55),
     . AT_MEAN(55),BT_MEAN(55),AT_AMP(55),BT_AMP(55),
     . A_GEOID(55),B_GEOID(55)

      INTEGER*4 I,N,M,NMAX,MMAX
      DOUBLE PRECISION TWOPI,DOY,TEMP0,PRES0,
     . APM,APA,ATM,ATA,HORT,X,Y,Z

      PARAMETER (TWOPI = 6.283185307179586476925287D0)

      DATA (A_GEOID(I),I=1,55)/ 
     .-5.6195D-001,-6.0794D-002,-2.0125D-001,-6.4180D-002,-3.6997D-002,
     .+1.0098D+001,+1.6436D+001,+1.4065D+001,+1.9881D+000,+6.4414D-001,
     .-4.7482D+000,-3.2290D+000,+5.0652D-001,+3.8279D-001,-2.6646D-002,
     .+1.7224D+000,-2.7970D-001,+6.8177D-001,-9.6658D-002,-1.5113D-002,
     .+2.9206D-003,-3.4621D+000,-3.8198D-001,+3.2306D-002,+6.9915D-003,
     .-2.3068D-003,-1.3548D-003,+4.7324D-006,+2.3527D+000,+1.2985D+000,
     .+2.1232D-001,+2.2571D-002,-3.7855D-003,+2.9449D-005,-1.6265D-004,
     .+1.1711D-007,+1.6732D+000,+1.9858D-001,+2.3975D-002,-9.0013D-004,
     .-2.2475D-003,-3.3095D-005,-1.2040D-005,+2.2010D-006,-1.0083D-006,
     .+8.6297D-001,+5.8231D-001,+2.0545D-002,-7.8110D-003,-1.4085D-004,
     .-8.8459D-006,+5.7256D-006,-1.5068D-006,+4.0095D-007,-2.4185D-008/ 

      DATA (B_GEOID(I),I=1,55)/ 
     .+0.0000D+000,+0.0000D+000,-6.5993D-002,+0.0000D+000,+6.5364D-002,
     .-5.8320D+000,+0.0000D+000,+1.6961D+000,-1.3557D+000,+1.2694D+000,
     .+0.0000D+000,-2.9310D+000,+9.4805D-001,-7.6243D-002,+4.1076D-002,
     .+0.0000D+000,-5.1808D-001,-3.4583D-001,-4.3632D-002,+2.2101D-003,
     .-1.0663D-002,+0.0000D+000,+1.0927D-001,-2.9463D-001,+1.4371D-003,
     .-1.1452D-002,-2.8156D-003,-3.5330D-004,+0.0000D+000,+4.4049D-001,
     .+5.5653D-002,-2.0396D-002,-1.7312D-003,+3.5805D-005,+7.2682D-005,
     .+2.2535D-006,+0.0000D+000,+1.9502D-002,+2.7919D-002,-8.1812D-003,
     .+4.4540D-004,+8.8663D-005,+5.5596D-005,+2.4826D-006,+1.0279D-006,
     .+0.0000D+000,+6.0529D-002,-3.5824D-002,-5.1367D-003,+3.0119D-005,
     .-2.9911D-005,+1.9844D-005,-1.2349D-006,-7.6756D-009,+5.0100D-008/ 

      DATA (AP_MEAN(I),I=1,55)/ 
     .+1.0108D+003,+8.4886D+000,+1.4799D+000,-1.3897D+001,+3.7516D-003,
     .-1.4936D-001,+1.2232D+001,-7.6615D-001,-6.7699D-002,+8.1002D-003,
     .-1.5874D+001,+3.6614D-001,-6.7807D-002,-3.6309D-003,+5.9966D-004,
     .+4.8163D+000,-3.7363D-001,-7.2071D-002,+1.9998D-003,-6.2385D-004,
     .-3.7916D-004,+4.7609D+000,-3.9534D-001,+8.6667D-003,+1.1569D-002,
     .+1.1441D-003,-1.4193D-004,-8.5723D-005,+6.5008D-001,-5.0889D-001,
     .-1.5754D-002,-2.8305D-003,+5.7458D-004,+3.2577D-005,-9.6052D-006,
     .-2.7974D-006,+1.3530D+000,-2.7271D-001,-3.0276D-004,+3.6286D-003,
     .-2.0398D-004,+1.5846D-005,-7.7787D-006,+1.1210D-006,+9.9020D-008,
     .+5.5046D-001,-2.7312D-001,+3.2532D-003,-2.4277D-003,+1.1596D-004,
     .+2.6421D-007,-1.3263D-006,+2.7322D-007,+1.4058D-007,+4.9414D-009/ 

      DATA (BP_MEAN(I),I=1,55)/ 
     .+0.0000D+000,+0.0000D+000,-1.2878D+000,+0.0000D+000,+7.0444D-001,
     .+3.3222D-001,+0.0000D+000,-2.9636D-001,+7.2248D-003,+7.9655D-003,
     .+0.0000D+000,+1.0854D+000,+1.1145D-002,-3.6513D-002,+3.1527D-003,
     .+0.0000D+000,-4.8434D-001,+5.2023D-002,-1.3091D-002,+1.8515D-003,
     .+1.5422D-004,+0.0000D+000,+6.8298D-001,+2.5261D-003,-9.9703D-004,
     .-1.0829D-003,+1.7688D-004,-3.1418D-005,+0.0000D+000,-3.7018D-001,
     .+4.3234D-002,+7.2559D-003,+3.1516D-004,+2.0024D-005,-8.0581D-006,
     .-2.3653D-006,+0.0000D+000,+1.0298D-001,-1.5086D-002,+5.6186D-003,
     .+3.2613D-005,+4.0567D-005,-1.3925D-006,-3.6219D-007,-2.0176D-008,
     .+0.0000D+000,-1.8364D-001,+1.8508D-002,+7.5016D-004,-9.6139D-005,
     .-3.1995D-006,+1.3868D-007,-1.9486D-007,+3.0165D-010,-6.4376D-010/ 

      DATA (AP_AMP(I),I=1,55)/ 
     .-1.0444D-001,+1.6618D-001,-6.3974D-002,+1.0922D+000,+5.7472D-001,
     .-3.0277D-001,-3.5087D+000,+7.1264D-003,-1.4030D-001,+3.7050D-002,
     .+4.0208D-001,-3.0431D-001,-1.3292D-001,+4.6746D-003,-1.5902D-004,
     .+2.8624D+000,-3.9315D-001,-6.4371D-002,+1.6444D-002,-2.3403D-003,
     .+4.2127D-005,+1.9945D+000,-6.0907D-001,-3.5386D-002,-1.0910D-003,
     .-1.2799D-004,+4.0970D-005,+2.2131D-005,-5.3292D-001,-2.9765D-001,
     .-3.2877D-002,+1.7691D-003,+5.9692D-005,+3.1725D-005,+2.0741D-005,
     .-3.7622D-007,+2.6372D+000,-3.1165D-001,+1.6439D-002,+2.1633D-004,
     .+1.7485D-004,+2.1587D-005,+6.1064D-006,-1.3755D-008,-7.8748D-008,
     .-5.9152D-001,-1.7676D-001,+8.1807D-003,+1.0445D-003,+2.3432D-004,
     .+9.3421D-006,+2.8104D-006,-1.5788D-007,-3.0648D-008,+2.6421D-010/ 

      DATA (BP_AMP(I),I=1,55)/ 
     .+0.0000D+000,+0.0000D+000,+9.3340D-001,+0.0000D+000,+8.2346D-001,
     .+2.2082D-001,+0.0000D+000,+9.6177D-001,-1.5650D-002,+1.2708D-003,
     .+0.0000D+000,-3.9913D-001,+2.8020D-002,+2.8334D-002,+8.5980D-004,
     .+0.0000D+000,+3.0545D-001,-2.1691D-002,+6.4067D-004,-3.6528D-005,
     .-1.1166D-004,+0.0000D+000,-7.6974D-002,-1.8986D-002,+5.6896D-003,
     .-2.4159D-004,-2.3033D-004,-9.6783D-006,+0.0000D+000,-1.0218D-001,
     .-1.3916D-002,-4.1025D-003,-5.1340D-005,-7.0114D-005,-3.3152D-007,
     .+1.6901D-006,+0.0000D+000,-1.2422D-002,+2.5072D-003,+1.1205D-003,
     .-1.3034D-004,-2.3971D-005,-2.6622D-006,+5.7852D-007,+4.5847D-008,
     .+0.0000D+000,+4.4777D-002,-3.0421D-003,+2.6062D-005,-7.2421D-005,
     .+1.9119D-006,+3.9236D-007,+2.2390D-007,+2.9765D-009,-4.6452D-009/ 

      DATA (AT_MEAN(I),I=1,55)/ 
     .+1.6257D+001,+2.1224D+000,+9.2569D-001,-2.5974D+001,+1.4510D+000,
     .+9.2468D-002,-5.3192D-001,+2.1094D-001,-6.9210D-002,-3.4060D-002,
     .-4.6569D+000,+2.6385D-001,-3.6093D-002,+1.0198D-002,-1.8783D-003,
     .+7.4983D-001,+1.1741D-001,+3.9940D-002,+5.1348D-003,+5.9111D-003,
     .+8.6133D-006,+6.3057D-001,+1.5203D-001,+3.9702D-002,+4.6334D-003,
     .+2.4406D-004,+1.5189D-004,+1.9581D-007,+5.4414D-001,+3.5722D-001,
     .+5.2763D-002,+4.1147D-003,-2.7239D-004,-5.9957D-005,+1.6394D-006,
     .-7.3045D-007,-2.9394D+000,+5.5579D-002,+1.8852D-002,+3.4272D-003,
     .-2.3193D-005,-2.9349D-005,+3.6397D-007,+2.0490D-006,-6.4719D-008,
     .-5.2225D-001,+2.0799D-001,+1.3477D-003,+3.1613D-004,-2.2285D-004,
     .-1.8137D-005,-1.5177D-007,+6.1343D-007,+7.8566D-008,+1.0749D-009/ 

      DATA (BT_MEAN(I),I=1,55)/ 
     .+0.0000D+000,+0.0000D+000,+1.0210D+000,+0.0000D+000,+6.0194D-001,
     .+1.2292D-001,+0.0000D+000,-4.2184D-001,+1.8230D-001,+4.2329D-002,
     .+0.0000D+000,+9.3312D-002,+9.5346D-002,-1.9724D-003,+5.8776D-003,
     .+0.0000D+000,-2.0940D-001,+3.4199D-002,-5.7672D-003,-2.1590D-003,
     .+5.6815D-004,+0.0000D+000,+2.2858D-001,+1.2283D-002,-9.3679D-003,
     .-1.4233D-003,-1.5962D-004,+4.0160D-005,+0.0000D+000,+3.6353D-002,
     .-9.4263D-004,-3.6762D-003,+5.8608D-005,-2.6391D-005,+3.2095D-006,
     .-1.1605D-006,+0.0000D+000,+1.6306D-001,+1.3293D-002,-1.1395D-003,
     .+5.1097D-005,+3.3977D-005,+7.6449D-006,-1.7602D-007,-7.6558D-008,
     .+0.0000D+000,-4.5415D-002,-1.8027D-002,+3.6561D-004,-1.1274D-004,
     .+1.3047D-005,+2.0001D-006,-1.5152D-007,-2.7807D-008,+7.7491D-009/ 

      DATA (AT_AMP(I),I=1,55)/ 
     .-1.8654D+000,-9.0041D+000,-1.2974D-001,-3.6053D+000,+2.0284D-002,
     .+2.1872D-001,-1.3015D+000,+4.0355D-001,+2.2216D-001,-4.0605D-003,
     .+1.9623D+000,+4.2887D-001,+2.1437D-001,-1.0061D-002,-1.1368D-003,
     .-6.9235D-002,+5.6758D-001,+1.1917D-001,-7.0765D-003,+3.0017D-004,
     .+3.0601D-004,+1.6559D+000,+2.0722D-001,+6.0013D-002,+1.7023D-004,
     .-9.2424D-004,+1.1269D-005,-6.9911D-006,-2.0886D+000,-6.7879D-002,
     .-8.5922D-004,-1.6087D-003,-4.5549D-005,+3.3178D-005,-6.1715D-006,
     .-1.4446D-006,-3.7210D-001,+1.5775D-001,-1.7827D-003,-4.4396D-004,
     .+2.2844D-004,-1.1215D-005,-2.1120D-006,-9.6421D-007,-1.4170D-008,
     .+7.8720D-001,-4.4238D-002,-1.5120D-003,-9.4119D-004,+4.0645D-006,
     .-4.9253D-006,-1.8656D-006,-4.0736D-007,-4.9594D-008,+1.6134D-009/ 

      DATA (BT_AMP(I),I=1,55)/ 
     .+0.0000D+000,+0.0000D+000,-8.9895D-001,+0.0000D+000,-1.0790D+000,
     .-1.2699D-001,+0.0000D+000,-5.9033D-001,+3.4865D-002,-3.2614D-002,
     .+0.0000D+000,-2.4310D-002,+1.5607D-002,-2.9833D-002,-5.9048D-003,
     .+0.0000D+000,+2.8383D-001,+4.0509D-002,-1.8834D-002,-1.2654D-003,
     .-1.3794D-004,+0.0000D+000,+1.3306D-001,+3.4960D-002,-3.6799D-003,
     .-3.5626D-004,+1.4814D-004,+3.7932D-006,+0.0000D+000,+2.0801D-001,
     .+6.5640D-003,-3.4893D-003,-2.7395D-004,+7.4296D-005,-7.9927D-006,
     .-1.0277D-006,+0.0000D+000,+3.6515D-002,-7.4319D-003,-6.2873D-004,
     .-8.2461D-005,+3.1095D-005,-5.3860D-007,-1.2055D-007,-1.1517D-007,
     .+0.0000D+000,+3.1404D-002,+1.5580D-002,-1.1428D-003,+3.3529D-005,
     .+1.0387D-005,-1.9378D-006,-2.7327D-007,+7.5833D-009,-9.2323D-009/ 


*     Reference day is 28 January 1980
*     This is taken from Niell (1996) to be consistent (See References)
*     For constant values use: doy = 91.3125
      DOY = DMJD  - 44239D0 + 1 - 28

*     Define degree n and order m EGM
      NMAX = 9
      MMAX = 9

*     Define unit vector
      X = DCOS(DLAT)*DCOS(DLON)
      Y = DCOS(DLAT)*DSIN(DLON)
      Z = DSIN(DLAT)
  
*     Legendre polynomials
      V(1,1) = 1.0D0
      W(1,1) = 0.0D0
      V(2,1) = Z * V(1,1)
      W(2,1) = 0.0

      DO N=2,NMAX
        V(N+1,1) = ((2*N-1) * Z * V(N,1) - (N-1) * V(N-1,1)) / N
        W(N+1,1) = 0.0D0
      ENDDO

      DO M=1,NMAX
        V(M+1,M+1) = (2*M-1) * (X*V(M,M) - Y*W(M,M))
        W(M+1,M+1) = (2*M-1) * (X*W(M,M) + Y*V(M,M))
        IF (M < NMAX) THEN
          V(M+2,M+1) = (2*M+1) * Z * V(M+1,M+1)
          W(M+2,M+1) = (2*M+1) * Z * W(M+1,M+1)
        ENDIF
        DO N=M+2,NMAX
          V(N+1,M+1) = ((2*N-1)*Z*V(N,M+1) - (N+M-1)*V(N-1,M+1)) / (N-M)
          W(N+1,M+1) = ((2*N-1)*Z*W(N,M+1) - (N+M-1)*W(N-1,M+1)) / (N-M)
        ENDDO
      ENDDO

*     Geoidal height
      UNDU = 0.D0
      I = 0
      DO N=0,NMAX
        DO M=0,N
          I = I+1
          UNDU = UNDU + (A_GEOID(I)*V(N+1,M+1) + B_GEOID(I)*W(N+1,M+1))
        ENDDO
      ENDDO

*     orthometric height
      HORT = DHGT - UNDU

*     Surface pressure on the geoid
      APM = 0.D0
      APA = 0.D0
      I = 0
      DO N=0,NMAX
        DO M=0,N
          I = I+1
          APM = APM + (AP_MEAN(I)*V(N+1,M+1) + BP_MEAN(I)*W(N+1,M+1))
          APA = APA + (AP_AMP(I) *V(N+1,M+1) + BP_AMP(I) *W(N+1,M+1))
        ENDDO
      ENDDO
      PRES0  = APM + APA*DCOS(DOY/365.25D0*TWOPI)

*     height correction for pressure
      PRES = PRES0*(1D0-0.0000226D0*HORT)**5.225D0

*     Surface temperature on the geoid
      ATM = 0D0
      ATA = 0D0
      I = 0
      DO N=0,NMAX
        DO M=0,N
          I = I+1
          ATM = ATM + (AT_MEAN(I)*V(N+1,M+1) + BT_MEAN(I)*W(N+1,M+1))
          ATA = ATA + (AT_AMP(I) *V(N+1,M+1) + BT_AMP(I) *W(N+1,M+1))
        ENDDO
      ENDDO
      TEMP0 =  ATM + ATA*DCOS(DOY/365.25D0*TWOPI)

*     height correction for temperature
      TEMP = TEMP0 - 0.0065D0*HORT

* Finished.

*+----------------------------------------------------------------------
*
*  Copyright (C) 2008
*  IERS Conventions Center
*
*  ==================================
*  IERS Conventions Software License
*  ==================================
*
*  NOTICE TO USER:
*
*  BY USING THIS SOFTWARE YOU ACCEPT THE FOLLOWING TERMS AND CONDITIONS
*  WHICH APPLY TO ITS USE.
*
*  1. The Software is provided by the IERS Conventions Center ("the
*     Center").
*
*  2. Permission is granted to anyone to use the Software for any
*     purpose, including commercial applications, free of charge,
*     subject to the conditions and restrictions listed below.
*
*  3. You (the user) may adapt the Software and its algorithms for your
*     own purposes and you may distribute the resulting "derived work"
*     to others, provided that the derived work complies with the
*     following requirements:
*
*     a) Your work shall be clearly identified so that it cannot be
*        mistaken for IERS Conventions software and that it has been
*        neither distributed by nor endorsed by the Center.
*
*     b) Your work (including source code) must contain descriptions of
*        how the derived work is based upon and/or differs from the
*        original Software.
*
*     c) The name(s) of all modified routine(s) that you distribute
*        shall be changed.
* 
*     d) The origin of the IERS Conventions components of your derived
*        work must not be misrepresented; you must not claim that you
*        wrote the original Software.
*
*     e) The source code must be included for all routine(s) that you
*        distribute.  This notice must be reproduced intact in any
*        source distribution. 
*
*  4. In any published work produced by the user and which includes
*     results achieved by using the Software, you shall acknowledge
*     that the Software was used in obtaining those results.
*
*  5. The Software is provided to the user "as is" and the Center makes
*     no warranty as to its use or performance.   The Center does not
*     and cannot warrant the performance or results which the user may
*     obtain by using the Software.  The Center makes no warranties,
*     express or implied, as to non-infringement of third party rights,
*     merchantability, or fitness for any particular purpose.  In no
*     event will the Center be liable to the user for any consequential,
*     incidental, or special damages, including any lost profits or lost
*     savings, even if a Center representative has been advised of such
*     damages, or for any claim by any third party.
*
*  Correspondence concerning IERS Conventions software should be
*  addressed as follows:
*
*                     Gerard Petit
*     Internet email: gpetit[at]bipm.org
*     Postal address: IERS Conventions Center
*                     Time, frequency and gravimetry section, BIPM
*                     Pavillon de Breteuil
*                     92312 Sevres  FRANCE
*
*     or
*
*                     Brian Luzum
*     Internet email: brian.luzum[at]usno.navy.mil
*     Postal address: IERS Conventions Center
*                     Earth Orientation Department
*                     3450 Massachusetts Ave, NW
*                     Washington, DC 20392
*
*
*-----------------------------------------------------------------------
      END      
