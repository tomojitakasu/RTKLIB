       SUBROUTINE DEHANTTIDEINEL(XSTA,YR,MONTH,DAY,FHR,XSUN,XMON,DXTIDE)
*+
*  - - - - - - - - - - - - - - - 
*   D E H A N T T I D E I N E L
*  - - - - - - - - - - - - - - - 
*
*  This routine is part of the International Earth Rotation and
*  Reference Systems Service (IERS) Conventions software collection.
*
*  This subroutine computes the tidal corrections of station displacements
*  caused by lunar and solar gravitational attraction (see References). 
*  The computations are calculated by the following steps:
*
*  Step 1): General degree 2 and degree 3 corrections + CALL ST1IDIU 
*  + CALL ST1ISEM + CALL ST1L1.
*  
*  Step 2): CALL STEP2DIU + CALL STEP2LON
*
*  It has been decided that the Step 3 non-correction for permanent tide
*  would not be applied in order to avoid a jump in the reference frame.
*  This Step 3 must be added in order to get the non-tidal station position
*  and to conform with the IAG Resolution.
*
*  In general, Class 1, 2, and 3 models represent physical effects that
*  act on geodetic parameters while canonical models provide lower-level
*  representations or basic computations that are used by Class 1, 2, or
*  3 models.
* 
*  Status: Class 1
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
*     XSTA          d(3)   Geocentric position of the IGS station (Note 1)
*     XSUN          d(3)   Geocentric position of the Sun (Note 2)
*     XMON          d(3)   Geocentric position of the Moon (Note 2)
*     YR            i      Year (Note 3)
*     MONTH         i      Month (Note 3)
*     DAY           i      Day of Month (Note 3)
*     FHR           d      Hour in the day (Note 4)
*
*  Returned:
*     DXTIDE        d(3)   Displacement vector (Note 5)
*
*  Notes:
*
*  1) The IGS station is in ITRF co-rotating frame.  All coordinates,
*     X, Y, and Z, are expressed in meters. 
*  
*  2) The position is in Earth Centered Earth Fixed (ECEF) frame.  All
*     coordinates are expressed in meters.
*
*  3) The values are expressed in Coordinated Universal Time (UTC).
*
*  4) The fractional hours in the day is computed as the hour + minutes/60.0
*     + sec/3600.0.  The unit is expressed in Universal Time (UT).
*
*  5) The displacement vector is in the geocentric ITRF.  All components are
*     expressed in meters.
*
*  Called:
*     SPROD             Finds the scalar product and unit vector of two vectors 
*     ZERO_VEC8         Returns the zero vector
*     ST1IDIU           Corrects for the out-of-phase part of Love numbers
*                       for the diurnal band
*     ST1ISEM           Same as above for the semi-diurnal band
*     ST1L1             Corrects for the latitude dependence of Love numbers
*     CAL2JD            Computes Julian Date from Gregorian calendar date
*     DAT               Computes the difference TAI-UTC
*     STEP2DIU          Computes in-phase and out-of-phase corrections in
*                       the diurnal band
*     STEP2LON          Same as above for the long period band
*
*  Test case:
*     given input: XSTA(1) = 4075578.385D0 meters
*                  XSTA(2) =  931852.890D0 meters
*                  XSTA(3) = 4801570.154D0 meters   
*                  XSUN(1) = 137859926952.015D0 meters
*                  XSUN(2) = 54228127881.4350D0 meters
*                  XSUN(3) = 23509422341.6960D0 meters
*                  XMON(1) = -179996231.920342D0 meters
*                  XMON(2) = -312468450.131567D0 meters
*                  XMON(3) = -169288918.592160D0 meters
*                  YR      = 2009
*                  MONTH   = 4
*                  DAY     = 13
*                  FHR     = 0.00D0 seconds 
*                  
*     expected output:  DXTIDE(1) = 0.7700420357108125891D-01 meters
*                       DXTIDE(2) = 0.6304056321824967613D-01 meters
*                       DXTIDE(3) = 0.5516568152597246810D-01 meters
*
*  References:
*
*     Groten, E., 2000, Geodesists Handbook 2000, Part 4,
*     http://www.gfy.ku.dk/~iag/HB2000/part4/groten.htm. See also
*     ''Parameters of Common Relevance of Astronomy, Geodesy, and
*     Geodynamics," J. Geod., 74, pp. 134-140
*
*     Mathews, P. M., Dehant, V., and Gipson, J. M., 1997, ''Tidal station
*     displacements," J. Geophys. Res., 102(B9), pp. 20,469-20,477
*
*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010),
*     IERS Technical Note No. 36, BKG (2010)
*     
*     Pitjeva, E. and Standish, E. M., 2009, ''Proposals for the masses
*     of the three largest asteroids, the Moon-Earth mass ratio and the
*     Astronomical Unit," Celest. Mech. Dyn. Astr., 103, pp. 365-372
*
*     Ries, J. C., Eanes, R. J., Shum, C. K. and Watkins, M. M., 1992,
*     ''Progress in the Determination of the Gravitational Coefficient
*     of the Earth," Geophys. Res. Lett., 19(6), pp. 529-531
*
*  Revisions:
*  1996 March    23 V. Dehant      Original code
*                   P. M. Mathews
*                   J. Gipson
*  2000 May      17 V. Dehant      Last modifications
*                   P. M. Mathews
*  2006 February 06 J. Ray         Header comments modified to clarify
*                                  input/output units and systems
*  2006 February 06 J. Ray         Subroutine DUTC modified for leap
*                                  second on 2006.0 and to correct 
*                                  do 5 i=1,87 from 84 to 87
*  2006 August   31 G. Petit       Correct DUTC for dates after 2007
*  2007 June     20 H. Manche      Modified DUTC to correct past mistake
*                                  and corrected DE line in subroutine
*                                  STEP2DIU
*  2007 October  23 H. Manche      Replace subroutines DUTC and FJLDY with
*                   G. Petit       SOFA subroutines iau_CAL2JD and iau_DAT
*                                  and correct time arguments of subroutine
*                                  STEP2DIU
*  2009 February 19 G. Petit       Update routine iau_DAT for 2009.0 leap
*                                  second
*  2009 August   06 B.E. Stetzler  Initial standardization of code 
*  2009 August   07 B.E. Stetzler  Updated MASS_RATIO_SUN, 
*                                  MASS_RATIO_MOON and RE to CBEs and
*                                  provided a test case
*  2009 August  07  B.E. Stetzler  Capitalized all variables for Fortran
*                                  77 compatibility
*  2009 September 01 B.E. Stetzler Removed 'iau_' from redistributed SOFA
*                                  subroutines
*-----------------------------------------------------------------------

      IMPLICIT NONE
      
      INTEGER STATUT,YR,MONTH,DAY,I,J
      DOUBLE PRECISION XSTA(3),XSUN(3),XMON(3),DXTIDE(3),XCORSTA(3),
     .                 FHR,H20,L20,H3,L3,H2,L2,SCS,RSTA,SCM,RSUN,RMON,
     .                 SCSUN,SCMON,COSPHI,P2SUN,P2MON,P3SUN,P3MON,
     .                 X2SUN,X2MON,X3SUN,X3MON,MASS_RATIO_SUN,
     .                 MASS_RATIO_MOON,RE,FAC2SUN,FAC2MON,FAC3SUN,
     .                 FAC3MON,JJM0,JJM1,DTT,T,PI,SINPHI,COSLA,SINLA,
     .                 DR,DN
      PARAMETER ( PI = 3.1415926535897932384626433D0 ) 
*----------------------------------------------------------------------  
* NOMINAL SECOND DEGREE AND THIRD DEGREE LOVE NUMBERS AND SHIDA NUMBERS  
*----------------------------------------------------------------------  
      DATA H20/0.6078D0/,L20/0.0847D0/,H3/0.292D0/,L3/0.015D0/
*----------------------------------------------------------------------  
* SCALAR PRODUCT OF STATION VECTOR WITH SUN/MOON VECTOR  
*----------------------------------------------------------------------  
      CALL SPROD(XSTA,XSUN,SCS,RSTA,RSUN)  
      CALL SPROD(XSTA,XMON,SCM,RSTA,RMON)  
      SCSUN=SCS/RSTA/RSUN  
      SCMON=SCM/RSTA/RMON
*----------------------------------------------------------------------   
* COMPUTATION OF NEW H2 AND L2  
*----------------------------------------------------------------------  
      COSPHI=DSQRT(XSTA(1)**2+XSTA(2)**2)/RSTA
      H2=H20-0.0006D0*(1D0-3D0/2D0*COSPHI**2)
      L2=L20+0.0002D0*(1D0-3D0/2D0*COSPHI**2)  

* P2 term  
      P2SUN=3D0*(H2/2D0-L2)*SCSUN**2-H2/2D0  
      P2MON=3D0*(H2/2D0-L2)*SCMON**2-H2/2D0  

* P3 term  
      P3SUN=5D0/2D0*(H3-3D0*L3)*SCSUN**3+3D0/2D0*(L3-H3)*SCSUN
      P3MON=5D0/2D0*(H3-3D0*L3)*SCMON**3+3D0/2D0*(L3-H3)*SCMON

*----------------------------------------------------------------------  
* TERM IN DIRECTION OF SUN/MOON VECTOR  
*----------------------------------------------------------------------  
      X2SUN=3D0*L2*SCSUN  
      X2MON=3D0*L2*SCMON  
      X3SUN=3D0*L3/2D0*(5D0*SCSUN**2-1D0)  
      X3MON=3D0*L3/2D0*(5D0*SCMON**2-1D0)
*----------------------------------------------------------------------  
* FACTORS FOR SUN/MOON USING IAU CURRENT BEST ESTIMATES (SEE REFERENCES) 
*----------------------------------------------------------------------  
      MASS_RATIO_SUN=332946.0482D0
      MASS_RATIO_MOON=0.0123000371D0
      RE=6378136.6D0
      FAC2SUN=MASS_RATIO_SUN*RE*(RE/RSUN)**3
      FAC2MON=MASS_RATIO_MOON*RE*(RE/RMON)**3
      FAC3SUN=FAC2SUN*(RE/RSUN)
      FAC3MON=FAC2MON*(RE/RMON)
  
* TOTAL DISPLACEMENT  
      DO 10 I=1,3  
      DXTIDE(I)=FAC2SUN*( X2SUN*XSUN(I)/RSUN + P2SUN*XSTA(I)/RSTA ) +
     .          FAC2MON*( X2MON*XMON(I)/RMON + P2MON*XSTA(I)/RSTA ) +  
     .          FAC3SUN*( X3SUN*XSUN(I)/RSUN + P3SUN*XSTA(I)/RSTA ) +   
     .          FAC3MON*( X3MON*XMON(I)/RMON + P3MON*XSTA(I)/RSTA )  
10    CONTINUE  

      CALL ZERO_VEC8(XCORSTA)
*+---------------------------------------------------------------------  
* CORRECTIONS FOR THE OUT-OF-PHASE PART OF LOVE NUMBERS (PART H_2^(0)I  
* AND L_2^(0)I )  
*----------------------------------------------------------------------

* FIRST, FOR THE DIURNAL BAND       

      CALL ST1IDIU(XSTA,XSUN,XMON,FAC2SUN,FAC2MON,XCORSTA)
      DO 11 I=1,3
      DXTIDE(I)=DXTIDE(I)+XCORSTA(I)  
11    CONTINUE
  
* SECOND, FOR THE SEMI-DIURNAL BAND       
 
      CALL ST1ISEM(XSTA,XSUN,XMON,FAC2SUN,FAC2MON,XCORSTA)
      DO 12 I=1,3
      DXTIDE(I)=DXTIDE(I)+XCORSTA(I)
12    CONTINUE  
  
*+---------------------------------------------------------------------
* CORRECTIONS FOR THE LATITUDE DEPENDENCE OF LOVE NUMBERS (PART L^(1) )  
*----------------------------------------------------------------------   
      CALL ST1L1(XSTA,XSUN,XMON,FAC2SUN,FAC2MON,XCORSTA)
      DO 13 I=1,3  
      DXTIDE(I)=DXTIDE(I)+XCORSTA(I)
13    CONTINUE    
  
* CONSIDER CORRECTIONS FOR STEP 2  

*+---------------------------------------------------------------------  
* CORRECTIONS FOR THE DIURNAL BAND:  
* 
*  FIRST, WE NEED TO KNOW THE DATE CONVERTED IN JULIAN CENTURIES 
*        
*   1) CALL THE SUBROUTINE COMPUTING THE JULIAN DATE 
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
      CALL CAL2JD ( YR, MONTH, DAY, JJM0, JJM1, STATUT )
      T=((JJM0-2451545.0D0)+JJM1+FHR/24.0D0)/36525D0
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
*   2) CALL THE SUBROUTINE COMPUTING THE CORRECTION OF UTC TIME  
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
      CALL DAT ( YR, MONTH, DAY, FHR, DTT, STATUT )
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      DTT = DTT + 32.184D0
*     CONVERSION OF T IN TT TIME
      T=T+DTT/(3600.0D0*24.0D0*36525D0)
  
*  SECOND, WE CAN CALL THE SUBROUTINE STEP2DIU, FOR THE DIURNAL BAND
*  CORRECTIONS, (in-phase and out-of-phase frequency dependence):
  
      CALL STEP2DIU(XSTA,FHR,T,XCORSTA)  
      DO 14 I=1,3  
      DXTIDE(I)=DXTIDE(I)+XCORSTA(I)
14    CONTINUE  
  
*  CORRECTIONS FOR THE LONG-PERIOD BAND,
*  (in-phase and out-of-phase frequency dependence):  

      CALL STEP2LON(XSTA,T,XCORSTA)
      DO 15 I=1,3  
      DXTIDE(I)=DXTIDE(I)+XCORSTA(I)
15    CONTINUE  
    
* CONSIDER CORRECTIONS FOR STEP 3  
  
*----------------------------------------------------------------------
* UNCORRECT FOR THE PERMANENT TIDE  
*  
*      SINPHI=XSTA(3)/RSTA  
*      COSPHI=DSQRT(XSTA(1)**2+XSTA(2)**2)/RSTA
*      COSLA=XSTA(1)/COSPHI/RSTA  
*      SINLA=XSTA(2)/COSPHI/RSTA  
*      DR=-DSQRT(5D0/4D0/PI)*H2*0.31460D0*(3D0/2D0*SINPHI**2-0.5D0)
*      DN=-DSQRT(5D0/4D0/PI)*L2*0.31460D0*3D0*COSPHI*SINPHI
*      DXTIDE(1)=DXTIDE(1)-DR*COSLA*COSPHI+DN*COSLA*SINPHI
*      DXTIDE(2)=DXTIDE(2)-DR*SINLA*COSPHI+DN*SINLA*SINPHI  
*      DXTIDE(3)=DXTIDE(3)-DR*SINPHI      -DN*COSPHI
*-----------------------------------------------------------------------
      RETURN  

*  Finished.

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
