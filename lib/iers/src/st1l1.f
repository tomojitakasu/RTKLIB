      SUBROUTINE ST1L1 (XSTA,XSUN,XMON,FAC2SUN,FAC2MON,XCORSTA)
*+
*  - - - - - - - - - - -
*   S T 1 L 1
*  - - - - - - - - - - -
*
*  This routine is part of the International Earth Rotation and
*  Reference Systems Service (IERS) Conventions software collection.
*
*  This subroutine gives the corrections induced by the latitude 
*  dependence given by L^1 in Mathews et al. 1991 (See References).
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
*     FAC2SUN       d      Degree 2 TGP factor for the Sun (Note 3)      
*     FAC2MON       d      Degree 2 TGP factor for the Moon (Note 3) 
*
*  Returned:
*     XCORSTA       d(3)   Out of phase station corrections for
*                          semi-diurnal band
*
*  Notes:
*
*  1) The IGS station is in ITRF co-rotating frame.  All coordinates are
*     expressed in meters. 
*  
*  2) The position is in Earth Centered Earth Fixed (ECEF) frame.  All
*     coordinates are expressed in meters.
*
*  3) The expressions are computed in the main program. TGP is the tide
*     generated potential.  The units are inverse meters. 
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
*                  FAC2SUN =  0.163271964478954D0 1/meters     
*                  FAC2MON =  0.321989090026845D0 1/meters    
*                  
*     expected output:  XCORSTA(1) = 0.2367189532359759044D-03 meters
*                       XCORSTA(2) = 0.5181609907284959182D-03 meters
*                       XCORSTA(3) = -0.3014881422940427977D-03 meters
*
*  References:
*
*     Mathews, P. M., Buffett, B. A., Herring, T. A., Shapiro, I. I.,
*     1991b, Forced nutations of the Earth: Influence of inner core
*     Dynamics 2. Numerical results and comparisons, J. Geophys. Res.,
*     96, 8243-8257
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
*  2009 July     31 B.E. Stetzler  Provided a test case and Mathews
*                                  reference
*-----------------------------------------------------------------------

      IMPLICIT NONE
      DOUBLE PRECISION NORM8, RSTA, SINPHI, COSPHI, COSTWOLA, SINLA,
     .                 COSLA, RMON, RSUN, DRSUN, DRMON, DNSUN, DNMON,
     .                 DESUN, DEMON, DR, DN, DE, XSTA, XSUN, XMON,
     .                 XCORSTA, DHI, DLI, FAC2SUN, FAC2MON, SINTWOLA,
     .                 L1, L1D, L1SD

      DIMENSION XSTA(3),XSUN(3),XMON(3),XCORSTA(3)  
      DATA L1D/0.0012D0/,L1SD/0.0024D0/

* Compute the normalized position vector of the IGS station.
      RSTA = NORM8(XSTA)
      SINPHI = XSTA(3)/RSTA  
      COSPHI = DSQRT(XSTA(1)**2+XSTA(2)**2)/RSTA  
      SINLA = XSTA(2)/COSPHI/RSTA  
      COSLA = XSTA(1)/COSPHI/RSTA  

* Compute the normalized position vector of the Moon.
      RMON = NORM8(XMON)

* Compute the normalized position vector of the Sun.
      RSUN = NORM8(XSUN)

* Compute the station corrections for the diurnal band.

      L1=L1D  
      DNSUN=-L1*SINPHI**2*FAC2SUN*XSUN(3)*(XSUN(1)*COSLA+XSUN(2)*SINLA)
     .            /RSUN**2
      DNMON=-L1*SINPHI**2*FAC2MON*XMON(3)*(XMON(1)*COSLA+XMON(2)*SINLA)
     .            /RMON**2
      DESUN=L1*SINPHI*(COSPHI**2-SINPHI**2)*FAC2SUN*XSUN(3)*
     . (XSUN(1)*SINLA-XSUN(2)*COSLA)/RSUN**2
      DEMON=L1*SINPHI*(COSPHI**2-SINPHI**2)*FAC2MON*XMON(3)*
     . (XMON(1)*SINLA-XMON(2)*COSLA)/RMON**2

      DE = 3D0*(DESUN+DEMON)  
      DN = 3D0*(DNSUN+DNMON)  

      XCORSTA(1) = -DE*SINLA-DN*SINPHI*COSLA  
      XCORSTA(2) = DE*COSLA-DN*SINPHI*SINLA  
      XCORSTA(3) = DN*COSPHI  
   
* Compute the station corrections for the semi-diurnal band.
  
      L1=L1SD  
      COSTWOLA=COSLA**2-SINLA**2  
      SINTWOLA=2.*COSLA*SINLA  

      DNSUN=-L1/2D0*SINPHI*COSPHI*FAC2SUN*((XSUN(1)**2-XSUN(2)**2)*
     . COSTWOLA+2D0*XSUN(1)*XSUN(2)*SINTWOLA)/RSUN**2

      DNMON=-L1/2D0*SINPHI*COSPHI*FAC2MON*((XMON(1)**2-XMON(2)**2)*
     . COSTWOLA+2D0*XMON(1)*XMON(2)*SINTWOLA)/RMON**2

      DESUN=-L1/2D0*SINPHI**2*COSPHI*FAC2SUN*((XSUN(1)**2-XSUN(2)**2)*
     . SINTWOLA-2D0*XSUN(1)*XSUN(2)*COSTWOLA)/RSUN**2

      DEMON=-L1/2D0*SINPHI**2*COSPHI*FAC2MON*((XMON(1)**2-XMON(2)**2)*
     . SINTWOLA-2D0*XMON(1)*XMON(2)*COSTWOLA)/RMON**2

      DE = 3D0*(DESUN+DEMON)  
      DN = 3D0*(DNSUN+DNMON)  

      XCORSTA(1)=XCORSTA(1)-DE*SINLA-DN*SINPHI*COSLA  
      XCORSTA(2)=XCORSTA(2)+DE*COSLA-DN*SINPHI*SINLA  
      XCORSTA(3)=XCORSTA(3)+DN*COSPHI  

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

