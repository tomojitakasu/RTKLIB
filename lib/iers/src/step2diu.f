      SUBROUTINE STEP2DIU (XSTA,FHR,T,XCORSTA)  
*+
*  - - - - - - - - - - -
*   S T E P 2 D I U
*  - - - - - - - - - - -
*
*  This routine is part of the International Earth Rotation and
*  Reference Systems Service (IERS) Conventions software collection.
*
*  This subroutine gives the in-phase and out-of-phase corrections
*  induced by mantle anelasticity in the diurnal band. 
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
*     FHR           d      Fractional hours in the day (Note 2)
*     T             d      Centuries since J2000
*
*  Returned:
*     XCORSTA       d(3)   In phase and out of phase station corrections
*                          for diurnal band (Note 4)
*
*  Notes:
*
*  1) The IGS station is in ITRF co-rotating frame.  All coordinates are
*     expressed in meters. 
*  
*  2) The fractional hours in the day is computed as the hour + minutes/60.0
*     + sec/3600.0.  The unit is expressed in Universal Time (UT).
*
*  4) All coordinates are expressed in meters.
*
*  Test case:
*     given input: XSTA(1) = 4075578.385D0 meters
*                  XSTA(2) =  931852.890D0 meters
*                  XSTA(3) = 4801570.154D0 meters 
*                  FHR     = 0.00D0 hours
*                  T       = 0.1059411362080767D0 Julian centuries
*                  
*     expected output:  XCORSTA(1) = 0.4193085327321284701D-02 meters
*                       XCORSTA(2) = 0.1456681241014607395D-02 meters
*                       XCORSTA(3) = 0.5123366597450316508D-02 meters
*
*  References:
*
*     Mathews, P. M., Dehant, V., and Gipson, J. M., 1997, ''Tidal station
*     displacements," J. Geophys. Res., 102(B9), pp. 20,469-20,477
*
*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010),
*     IERS Technical Note No. 36, BKG (2010)
*
*  Revisions:
*  1996 March    23 V. Dehant      Original code
*  2009 July     31 B.E. Stetzler  Initial standardization of code 
*  2009 August   06 B.E. Stetzler  Provided a test case
*  2009 August   06 B.E. Stetzler  Capitalized all variables for 
*                                  Fortran 77 compatibility
*  2010 October  20 B.E. Stetzler  Input T corrected to be number of
*                                  centuries since J2000
*-----------------------------------------------------------------------

      IMPLICIT NONE
      INTEGER I, J
      DOUBLE PRECISION XSTA(3), XCORSTA(3), DATDI(9,31), DEG2RAD, FHR,
     .                 T, S, TAU, PR, H, P, ZNS, PS, RSTA, SINPHI,
     .                 COSPHI, COSLA, SINLA, ZLA, THETAF, DR, DN, DE
      DOUBLE PRECISION D2PI
      PARAMETER ( D2PI = 6.283185307179586476925287D0 )

      DATA ((DATDI(I,J),I=1,9),J=1,31)/  

     . -3D0, 0D0, 2D0, 0D0, 0D0,-0.01D0, 0D0, 0D0, 0D0,   
     . -3D0, 2D0, 0D0, 0D0, 0D0,-0.01D0, 0D0, 0D0, 0D0,   
     . -2D0, 0D0, 1D0,-1D0, 0D0,-0.02D0, 0D0, 0D0, 0D0,   
     . -2D0, 0D0, 1D0, 0D0, 0D0,-0.08D0, 0D0,-0.01D0, 0.01D0,
     . -2D0, 2D0,-1D0, 0D0, 0D0,-0.02D0, 0D0, 0D0, 0D0,
     . -1D0, 0D0, 0D0,-1D0, 0D0,-0.10D0, 0D0, 0D0, 0D0,
     . -1D0, 0D0, 0D0, 0D0, 0D0,-0.51D0, 0D0,-0.02D0, 0.03D0,
     . -1D0, 2D0, 0D0, 0D0, 0D0, 0.01D0, 0D0, 0D0, 0D0,
     .  0D0,-2D0, 1D0, 0D0, 0D0, 0.01D0, 0D0, 0D0, 0D0,
     .  0D0, 0D0,-1D0, 0D0, 0D0, 0.02D0, 0D0, 0D0, 0D0,
     .  0D0, 0D0, 1D0, 0D0, 0D0, 0.06D0, 0D0, 0D0, 0D0,
     .  0D0, 0D0, 1D0, 1D0, 0D0, 0.01D0, 0D0, 0D0, 0D0,
     .  0D0, 2D0,-1D0, 0D0, 0D0, 0.01D0, 0D0, 0D0, 0D0,
     .  1D0,-3D0, 0D0, 0D0, 1D0,-0.06D0, 0D0, 0D0, 0D0,
     .  1D0,-2D0, 0D0,-1D0, 0D0, 0.01D0, 0D0, 0D0, 0D0,
     .  1D0,-2D0, 0D0, 0D0, 0D0,-1.23D0,-0.07D0, 0.06D0, 0.01D0,
     .  1D0,-1D0, 0D0, 0D0,-1D0, 0.02D0, 0D0, 0D0, 0D0,
     .  1D0,-1D0, 0D0, 0D0, 1D0, 0.04D0, 0D0, 0D0, 0D0,
     .  1D0, 0D0, 0D0,-1D0, 0D0,-0.22D0, 0.01D0, 0.01D0, 0D0,
     .  1D0, 0D0, 0D0, 0D0, 0D0,12.00D0,-0.80D0,-0.67D0,-0.03D0,
     .  1D0, 0D0, 0D0, 1D0, 0D0, 1.73D0,-0.12D0,-0.10D0, 0D0,
     .  1D0, 0D0, 0D0, 2D0, 0D0,-0.04D0, 0D0, 0D0, 0D0, 
     .  1D0, 1D0, 0D0, 0D0,-1D0,-0.50D0,-0.01D0, 0.03D0, 0D0,
     .  1D0, 1D0, 0D0, 0D0, 1D0, 0.01D0, 0D0, 0D0, 0D0,
     .  0D0, 1D0, 0D0, 1D0,-1D0,-0.01D0, 0D0, 0D0, 0D0,
     .  1D0, 2D0,-2D0, 0D0, 0D0,-0.01D0, 0D0, 0D0, 0D0,
     .  1D0, 2D0, 0D0, 0D0, 0D0,-0.11D0, 0.01D0, 0.01D0, 0D0,
     .  2D0,-2D0, 1D0, 0D0, 0D0,-0.01D0, 0D0, 0D0, 0D0,
     .  2D0, 0D0,-1D0, 0D0, 0D0,-0.02D0, 0D0, 0D0, 0D0,
     .  3D0, 0D0, 0D0, 0D0, 0D0, 0D0, 0D0, 0D0, 0D0,
     .  3D0, 0D0, 0D0, 1D0, 0D0, 0D0, 0D0, 0D0, 0D0/

      DEG2RAD = D2PI/360D0

*  Compute the phase angles in degrees.
      S = 218.31664563D0
     .  + (481267.88194D0
     .  + (-0.0014663889D0 
     .  + (0.00000185139D0)*T)*T)*T 

      TAU = FHR*15D0
     .    + 280.4606184D0
     .    + (36000.7700536D0
     .    + (0.00038793D0
     .    + (-0.0000000258D0)*T)*T)*T
     .    + (-S)

      PR = (1.396971278D0
     .   + (0.000308889D0
     .   + (0.000000021D0
     .   + (0.000000007D0)*T)*T)*T)*T 

      S = S + PR 

      H = 280.46645D0
     .  + (36000.7697489D0
     .  + (0.00030322222D0 
     .  + (0.000000020D0
     .  + (-0.00000000654D0)*T)*T)*T)*T  

      P = 83.35324312D0
     .  + (4069.01363525D0
     .  + (-0.01032172222D0
     .  + (-0.0000124991D0
     .  + (0.00000005263D0)*T)*T)*T)*T  

      ZNS = 234.95544499D0
     .    + (1934.13626197D0
     .    + (-0.00207561111D0
     .    + (-0.00000213944D0
     .    + (0.00000001650D0)*T)*T)*T)*T 

      PS = 282.93734098D0
     .   + (1.71945766667D0
     .   + (0.00045688889D0
     .   + (-0.00000001778D0
     .   + (-0.00000000334D0)*T)*T)*T)*T

* Reduce angles to between the range 0 and 360.
      S =  DMOD(S,360D0)
      TAU = DMOD(TAU,360D0)
      H =  DMOD(H,360D0)
      P =  DMOD(P,360D0)
      ZNS = DMOD(ZNS,360D0)
      PS = DMOD(PS,360D0)

      RSTA = DSQRT(XSTA(1)**2+XSTA(2)**2+XSTA(3)**2)  
      SINPHI = XSTA(3)/RSTA  
      COSPHI = DSQRT(XSTA(1)**2+XSTA(2)**2)/RSTA  

      COSLA = XSTA(1)/COSPHI/RSTA
      SINLA = XSTA(2)/COSPHI/RSTA
      ZLA = DATAN2(XSTA(2),XSTA(1))

      DO 99 I=1,3  
* Initialize.
      XCORSTA(I) = 0D0
99    CONTINUE
      DO 98 J=1,31
* Convert from degrees to radians.
      THETAF=(TAU+DATDI(1,J)*S+DATDI(2,J)*H+DATDI(3,J)*P+
     . DATDI(4,J)*ZNS+DATDI(5,J)*PS)*DEG2RAD

      DR=DATDI(6,J)*2D0*SINPHI*COSPHI*SIN(THETAF+ZLA)+
     . DATDI(7,J)*2D0*SINPHI*COSPHI*COS(THETAF+ZLA)

      DN=DATDI(8,J)*(COSPHI**2-SINPHI**2)*SIN(THETAF+ZLA)+
     . DATDI(9,J)*(COSPHI**2-SINPHI**2)*COS(THETAF+ZLA)
*      DE=DATDI(8,J)*SINPHI*COS(THETAF+ZLA)+
*     Modified 20 June 2007

      DE=DATDI(8,J)*SINPHI*COS(THETAF+ZLA)-
     . DATDI(9,J)*SINPHI*SIN(THETAF+ZLA)


      XCORSTA(1)=XCORSTA(1)+DR*COSLA*COSPHI-DE*SINLA  
     . -DN*SINPHI*COSLA  
      XCORSTA(2)=XCORSTA(2)+DR*SINLA*COSPHI+DE*COSLA  
     . -DN*SINPHI*SINLA  
      XCORSTA(3)=XCORSTA(3)+DR*SINPHI+DN*COSPHI  
98    CONTINUE   

      DO 97 I=1,3
      XCORSTA(I)=XCORSTA(I)/1000D0
97    CONTINUE  
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

