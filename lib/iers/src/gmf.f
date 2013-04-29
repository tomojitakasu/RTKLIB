      SUBROUTINE GMF (DMJD,DLAT,DLON,DHGT,ZD,GMFH,GMFW)
*+
*  - - - - - - - - -
*   G M F 
*  - - - - - - - - -
*
*  This routine is part of the International Earth Rotation and
*  Reference Systems Service (IERS) Conventions software collection.
*
*  This subroutine determines the Global Mapping Functions GMF (Boehm et al. 2006).
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
*     DHGT           d      Height in meters (mean sea level)
*     ZD             d      Zenith distance in radians
*
*  Returned:
*     GMFH           d      Hydrostatic mapping function (Note 1)
*     GMFW           d      Wet mapping function (Note 1)
*
*  Notes:
*
*  1) The mapping functions are dimensionless scale factors.
*
*  2) This is from a 9x9 Earth Gravitational Model (EGM).
*
*  Test case:
*     given input: DMJD = 55055D0
*                  DLAT = 0.6708665767D0 radians (NRAO, Green Bank, WV)
*                  DLON = -1.393397187D0 radians
*                  DHGT = 844.715D0 meters
*                  ZD   = 1.278564131D0 radians
*
*     expected output: GMFH = 3.425245519339138678D0
*                      GMFW = 3.449589116182419257D0
*                     
*  References:
*
*     Boehm, J., Niell, A., Tregoning, P. and Schuh, H., (2006), 
*     "Global Mapping Functions (GMF): A new empirical mapping
*     function based on numerical weather model data",
*     Geophy. Res. Lett., Vol. 33, L07304, doi:10.1029/2005GL025545.
*
*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010),
*     IERS Technical Note No. 36, BKG (2010)
*
*  Revisions:
*  2005 August 30 J. Boehm    Original code
*  2009 August 11 B.E. Stetzler Added header and copyright
*  2009 August 12 B.E. Stetzler More modifications and defined twopi
*  2009 August 12 B.E. Stetzler Provided test case
*  2009 August 12 B.E. Stetzler Capitalized all variables for FORTRAN 77
*                              compatibility and corrected test case
*                              latitude and longitude coordinates
*-----------------------------------------------------------------------

      IMPLICIT NONE

      DOUBLE PRECISION DMJD, DLAT, DLON, DHGT, ZD, GMFH, GMFW

      DOUBLE PRECISION DFAC(20),P(10,10),AP(55),BP(55),
     .          AH_MEAN(55),BH_MEAN(55),AH_AMP(55),BH_AMP(55),
     .          AW_MEAN(55),BW_MEAN(55),AW_AMP(55),BW_AMP(55)

      INTEGER I, J, K, M, N, IR   

      DOUBLE PRECISION TWOPI, DOY, T, SUM1, BH, C0H, PHH, C11H, C10H,
     .                 CH, AHM, AHA, AH, SINE, BETA, GAMMA, TOPCON,
     .                 A_HT, B_HT, C_HT, HS_KM, HT_CORR_COEF, HT_CORR,
     .                 BW, CW, AWM, AWA, AW, PI

      PARAMETER ( PI = 3.1415926535897932384626433D0 )
      PARAMETER (TWOPI = 6.283185307179586476925287D0)

*+---------------------------------------------------------------------
*     Reference day is 28 January 1980
*     This is taken from Niell (1996) to be consistent
*----------------------------------------------------------------------
      DOY = DMJD  - 44239D0 + 1 - 28

      DATA (AH_MEAN(I),I=1,55)/
     .+1.2517D+02, +8.503D-01, +6.936D-02, -6.760D+00, +1.771D-01,
     . +1.130D-02, +5.963D-01, +1.808D-02, +2.801D-03, -1.414D-03,
     . -1.212D+00, +9.300D-02, +3.683D-03, +1.095D-03, +4.671D-05,
     . +3.959D-01, -3.867D-02, +5.413D-03, -5.289D-04, +3.229D-04,
     . +2.067D-05, +3.000D-01, +2.031D-02, +5.900D-03, +4.573D-04,
     . -7.619D-05, +2.327D-06, +3.845D-06, +1.182D-01, +1.158D-02,
     . +5.445D-03, +6.219D-05, +4.204D-06, -2.093D-06, +1.540D-07,
     . -4.280D-08, -4.751D-01, -3.490D-02, +1.758D-03, +4.019D-04,
     . -2.799D-06, -1.287D-06, +5.468D-07, +7.580D-08, -6.300D-09,
     . -1.160D-01, +8.301D-03, +8.771D-04, +9.955D-05, -1.718D-06,
     . -2.012D-06, +1.170D-08, +1.790D-08, -1.300D-09, +1.000D-10/

      DATA (BH_MEAN(I),I=1,55)/
     . +0.000D+00, +0.000D+00, +3.249D-02, +0.000D+00, +3.324D-02,
     . +1.850D-02, +0.000D+00, -1.115D-01, +2.519D-02, +4.923D-03,
     . +0.000D+00, +2.737D-02, +1.595D-02, -7.332D-04, +1.933D-04,
     . +0.000D+00, -4.796D-02, +6.381D-03, -1.599D-04, -3.685D-04,
     . +1.815D-05, +0.000D+00, +7.033D-02, +2.426D-03, -1.111D-03,
     . -1.357D-04, -7.828D-06, +2.547D-06, +0.000D+00, +5.779D-03,
     . +3.133D-03, -5.312D-04, -2.028D-05, +2.323D-07, -9.100D-08,
     . -1.650D-08, +0.000D+00, +3.688D-02, -8.638D-04, -8.514D-05,
     . -2.828D-05, +5.403D-07, +4.390D-07, +1.350D-08, +1.800D-09,
     . +0.000D+00, -2.736D-02, -2.977D-04, +8.113D-05, +2.329D-07,
     . +8.451D-07, +4.490D-08, -8.100D-09, -1.500D-09, +2.000D-10/
       
      DATA (AH_AMP(I),I=1,55)/
     . -2.738D-01, -2.837D+00, +1.298D-02, -3.588D-01, +2.413D-02,
     . +3.427D-02, -7.624D-01, +7.272D-02, +2.160D-02, -3.385D-03,
     . +4.424D-01, +3.722D-02, +2.195D-02, -1.503D-03, +2.426D-04,
     . +3.013D-01, +5.762D-02, +1.019D-02, -4.476D-04, +6.790D-05,
     . +3.227D-05, +3.123D-01, -3.535D-02, +4.840D-03, +3.025D-06,
     . -4.363D-05, +2.854D-07, -1.286D-06, -6.725D-01, -3.730D-02,
     . +8.964D-04, +1.399D-04, -3.990D-06, +7.431D-06, -2.796D-07,
     . -1.601D-07, +4.068D-02, -1.352D-02, +7.282D-04, +9.594D-05,
     . +2.070D-06, -9.620D-08, -2.742D-07, -6.370D-08, -6.300D-09,
     . +8.625D-02, -5.971D-03, +4.705D-04, +2.335D-05, +4.226D-06,
     . +2.475D-07, -8.850D-08, -3.600D-08, -2.900D-09, +0.000D+00/
       
      DATA (BH_AMP(I),I=1,55)/
     . +0.000D+00, +0.000D+00, -1.136D-01, +0.000D+00, -1.868D-01,
     . -1.399D-02, +0.000D+00, -1.043D-01, +1.175D-02, -2.240D-03,
     . +0.000D+00, -3.222D-02, +1.333D-02, -2.647D-03, -2.316D-05,
     . +0.000D+00, +5.339D-02, +1.107D-02, -3.116D-03, -1.079D-04,
     . -1.299D-05, +0.000D+00, +4.861D-03, +8.891D-03, -6.448D-04,
     . -1.279D-05, +6.358D-06, -1.417D-07, +0.000D+00, +3.041D-02,
     . +1.150D-03, -8.743D-04, -2.781D-05, +6.367D-07, -1.140D-08,
     . -4.200D-08, +0.000D+00, -2.982D-02, -3.000D-03, +1.394D-05,
     . -3.290D-05, -1.705D-07, +7.440D-08, +2.720D-08, -6.600D-09,
     . +0.000D+00, +1.236D-02, -9.981D-04, -3.792D-05, -1.355D-05,
     . +1.162D-06, -1.789D-07, +1.470D-08, -2.400D-09, -4.000D-10/
       
      DATA (AW_MEAN(I),I=1,55)/
     . +5.640D+01, +1.555D+00, -1.011D+00, -3.975D+00, +3.171D-02,
     . +1.065D-01, +6.175D-01, +1.376D-01, +4.229D-02, +3.028D-03,
     . +1.688D+00, -1.692D-01, +5.478D-02, +2.473D-02, +6.059D-04,
     . +2.278D+00, +6.614D-03, -3.505D-04, -6.697D-03, +8.402D-04,
     . +7.033D-04, -3.236D+00, +2.184D-01, -4.611D-02, -1.613D-02,
     . -1.604D-03, +5.420D-05, +7.922D-05, -2.711D-01, -4.406D-01,
     . -3.376D-02, -2.801D-03, -4.090D-04, -2.056D-05, +6.894D-06,
     . +2.317D-06, +1.941D+00, -2.562D-01, +1.598D-02, +5.449D-03,
     . +3.544D-04, +1.148D-05, +7.503D-06, -5.667D-07, -3.660D-08,
     . +8.683D-01, -5.931D-02, -1.864D-03, -1.277D-04, +2.029D-04,
     . +1.269D-05, +1.629D-06, +9.660D-08, -1.015D-07, -5.000D-10/
       
      DATA (BW_MEAN(I),I=1,55)/
     . +0.000D+00, +0.000D+00, +2.592D-01, +0.000D+00, +2.974D-02,
     . -5.471D-01, +0.000D+00, -5.926D-01, -1.030D-01, -1.567D-02,
     . +0.000D+00, +1.710D-01, +9.025D-02, +2.689D-02, +2.243D-03,
     . +0.000D+00, +3.439D-01, +2.402D-02, +5.410D-03, +1.601D-03,
     . +9.669D-05, +0.000D+00, +9.502D-02, -3.063D-02, -1.055D-03,
     . -1.067D-04, -1.130D-04, +2.124D-05, +0.000D+00, -3.129D-01,
     . +8.463D-03, +2.253D-04, +7.413D-05, -9.376D-05, -1.606D-06,
     . +2.060D-06, +0.000D+00, +2.739D-01, +1.167D-03, -2.246D-05,
     . -1.287D-04, -2.438D-05, -7.561D-07, +1.158D-06, +4.950D-08,
     . +0.000D+00, -1.344D-01, +5.342D-03, +3.775D-04, -6.756D-05,
     . -1.686D-06, -1.184D-06, +2.768D-07, +2.730D-08, +5.700D-09/
       
      DATA (AW_AMP(I),I=1,55)/
     . +1.023D-01, -2.695D+00, +3.417D-01, -1.405D-01, +3.175D-01,
     . +2.116D-01, +3.536D+00, -1.505D-01, -1.660D-02, +2.967D-02,
     . +3.819D-01, -1.695D-01, -7.444D-02, +7.409D-03, -6.262D-03,
     . -1.836D+00, -1.759D-02, -6.256D-02, -2.371D-03, +7.947D-04,
     . +1.501D-04, -8.603D-01, -1.360D-01, -3.629D-02, -3.706D-03,
     . -2.976D-04, +1.857D-05, +3.021D-05, +2.248D+00, -1.178D-01,
     . +1.255D-02, +1.134D-03, -2.161D-04, -5.817D-06, +8.836D-07,
     . -1.769D-07, +7.313D-01, -1.188D-01, +1.145D-02, +1.011D-03,
     . +1.083D-04, +2.570D-06, -2.140D-06, -5.710D-08, +2.000D-08,
     . -1.632D+00, -6.948D-03, -3.893D-03, +8.592D-04, +7.577D-05,
     . +4.539D-06, -3.852D-07, -2.213D-07, -1.370D-08, +5.800D-09/
       
      DATA (BW_AMP(I),I=1,55)/
     . +0.000D+00, +0.000D+00, -8.865D-02, +0.000D+00, -4.309D-01,
     . +6.340D-02, +0.000D+00, +1.162D-01, +6.176D-02, -4.234D-03,
     . +0.000D+00, +2.530D-01, +4.017D-02, -6.204D-03, +4.977D-03,
     . +0.000D+00, -1.737D-01, -5.638D-03, +1.488D-04, +4.857D-04,
     . -1.809D-04, +0.000D+00, -1.514D-01, -1.685D-02, +5.333D-03,
     . -7.611D-05, +2.394D-05, +8.195D-06, +0.000D+00, +9.326D-02,
     . -1.275D-02, -3.071D-04, +5.374D-05, -3.391D-05, -7.436D-06,
     . +6.747D-07, +0.000D+00, -8.637D-02, -3.807D-03, -6.833D-04,
     . -3.861D-05, -2.268D-05, +1.454D-06, +3.860D-07, -1.068D-07,
     . +0.000D+00, -2.658D-02, -1.947D-03, +7.131D-04, -3.506D-05,
     . +1.885D-07, +5.792D-07, +3.990D-08, +2.000D-08, -5.700D-09/

*     Define a parameter t
      T = DSIN(DLAT)

*     Define degree n and order m EGM
      N = 9
      M = 9

*     Determine n!  (factorial)  moved by 1
      DFAC(1) = 1
      DO I = 1,(2*N + 1)
        DFAC(I+1) = DFAC(I)*I
      END DO

*     Determine Legendre functions (Heiskanen and Moritz,
*     Physical Geodesy, 1967, eq. 1-62)
      DO I = 0,N
        DO J = 0,MIN(I,M)
          IR = INT((I - J)/2)
          SUM1 = 0
          DO K = 0,IR
            SUM1 = SUM1 + (-1)**K*DFAC(2*I - 2*K + 1)/DFAC(K + 1)/
     .       DFAC(I - K + 1)/DFAC(I - J - 2*K + 1)*T**(I - J - 2*K)
          END DO
*         Legendre functions moved by 1
          P(I + 1,J + 1) = 1.D0/2**I*DSQRT((1 - T**2)**(J))*SUM1
        END DO
      END DO

*     Calculate spherical harmonics
      I = 0
      DO N = 0,9
        DO M = 0,N
          I = I + 1
          AP(I) = P(N+1,M+1)*DCOS(M*DLON)
          BP(I) = P(N+1,M+1)*DSIN(M*DLON)
        END DO
      END DO

*     Compute hydrostatic mapping function
      BH = 0.0029D0
      C0H = 0.062D0
      IF (DLAT.LT.0D0) THEN ! SOUTHERN HEMISPHERE
        PHH  = PI
        C11H = 0.007D0
        C10H = 0.002D0
      ELSE                  ! NORTHERN HEMISPHERE
        PHH  = 0D0
        C11H = 0.005D0
        C10H = 0.001D0
      END IF
      CH = C0H + ((DCOS(DOY/365.25D0*TWOPI + PHH)+1D0)*C11H/2D0 + C10H)*
     .           (1D0-DCOS(DLAT))

      AHM = 0D0
      AHA = 0D0
      DO I = 1,55
        AHM = AHM + (AH_MEAN(I)*AP(I) + BH_MEAN(I)*BP(I))*1D-5
        AHA = AHA + (AH_AMP(I) *AP(I) + BH_AMP(I) *BP(I))*1D-5
      END DO
      AH  = AHM + AHA*DCOS(DOY/365.25D0*TWOPI)

      SINE   = DSIN(PI/2D0 - ZD)
      BETA   = BH/( SINE + CH  )
      GAMMA  = AH/( SINE + BETA)
      TOPCON = (1D0 + AH/(1D0 + BH/(1D0 + CH)))
      GMFH   = TOPCON/(SINE+GAMMA)

*     Height correction for hydrostatic mapping function from Niell (1996)
      A_HT = 2.53D-5
      B_HT = 5.49D-3
      C_HT = 1.14D-3
      HS_KM  = DHGT/1000D0

      BETA   = B_HT/( SINE + C_HT )
      GAMMA  = A_HT/( SINE + BETA)
      TOPCON = (1D0 + A_HT/(1D0 + B_HT/(1D0 + C_HT)))
      HT_CORR_COEF = 1D0/SINE - TOPCON/(SINE + GAMMA)
      HT_CORR      = HT_CORR_COEF * HS_KM
      GMFH         = GMFH + HT_CORR

*     Compute wet mapping function
      BW = 0.00146D0
      CW = 0.04391D0

      AWM = 0D0
      AWA = 0D0
      DO I = 1,55
        AWM = AWM + (AW_MEAN(I)*AP(I) + BW_MEAN(I)*BP(I))*1D-5
        AWA = AWA + (AW_AMP(I) *AP(I) + BW_AMP(I) *BP(I))*1D-5
      END DO
      AW =  AWM + AWA*DCOS(DOY/365.25D0*TWOPI)

      BETA   = BW/( SINE + CW )
      GAMMA  = AW/( SINE + BETA)
      TOPCON = (1D0 + AW/(1D0 + BW/(1D0 + CW)))
      GMFW   = TOPCON/(SINE+GAMMA)
      
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
