/* ../src/st1isem.f -- translated by f2c (version 20090411).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include "f2c.h"

/* Subroutine */ int st1isem_(doublereal *xsta, doublereal *xsun, doublereal *
	xmon, doublereal *fac2sun, doublereal *fac2mon, doublereal *xcorsta)
{
    /* Initialized data */

    static doublereal dhi = -.0022;
    static doublereal dli = -7e-4;

    /* System generated locals */
    doublereal d__1, d__2, d__3, d__4;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static doublereal costwola, sintwola, de, dn, dr, rsta, rmon, rsun;
    extern doublereal norm8_(doublereal *);
    static doublereal cosla, demon, sinla, dnmon, desun, drmon, dnsun, drsun, 
	    cosphi, sinphi;

/* + */
/*  - - - - - - - - - - - */
/*   S T 1 I S E M */
/*  - - - - - - - - - - - */

/*  This routine is part of the International Earth Rotation and */
/*  Reference Systems Service (IERS) Conventions software collection. */

/*  This subroutine gives the out-of-phase corrections induced by */
/*  mantle anelasticity in the semi-diurnal band. */

/*  In general, Class 1, 2, and 3 models represent physical effects that */
/*  act on geodetic parameters while canonical models provide lower-level */
/*  representations or basic computations that are used by Class 1, 2, or */
/*  3 models. */

/*  Status: Class 1 */

/*     Class 1 models are those recommended to be used a priori in the */
/*     reduction of raw space geodetic data in order to determine */
/*     geodetic parameter estimates. */
/*     Class 2 models are those that eliminate an observational */
/*     singularity and are purely conventional in nature. */
/*     Class 3 models are those that are not required as either Class */
/*     1 or 2. */
/*     Canonical models are accepted as is and cannot be classified as a */
/*     Class 1, 2, or 3 model. */

/*  Given: */
/*     XSTA          d(3)   Geocentric position of the IGS station (Note 1) */
/*     XSUN          d(3)   Geocentric position of the Sun (Note 2) */
/*     XMON          d(3)   Geocentric position of the Moon (Note 2) */
/*     FAC2SUN       d      Degree 2 TGP factor for the Sun (Note 3) */
/*     FAC2MON       d      Degree 2 TGP factor for the Moon (Note 3) */

/*  Returned: */
/*     XCORSTA       d(3)   Out of phase station corrections for */
/*                          semi-diurnal band */

/*  Notes: */

/*  1) The IGS station is in ITRF co-rotating frame.  All coordinates are */
/*     expressed in meters. */

/*  2) The position is in Earth Centered Earth Fixed (ECEF) frame.  All */
/*     coordinates are expressed in meters. */

/*  3) The expressions are computed in the main program.  TGP is the tide */
/*     generated potential.  The units are inverse meters. */

/*  Test case: */
/*     given input: XSTA(1) = 4075578.385D0 meters */
/*                  XSTA(2) =  931852.890D0 meters */
/*                  XSTA(3) = 4801570.154D0 meters */
/*                  XSUN(1) = 137859926952.015D0 meters */
/*                  XSUN(2) = 54228127881.4350D0 meters */
/*                  XSUN(3) = 23509422341.6960D0 meters */
/*                  XMON(1) = -179996231.920342D0 meters */
/*                  XMON(2) = -312468450.131567D0 meters */
/*                  XMON(3) = -169288918.592160D0 meters */
/*                  FAC2SUN =  0.163271964478954D0 1/meters */
/*                  FAC2MON =  0.321989090026845D0 1/meters */

/*     expected output:  XCORSTA(1) = -0.2801334805106874015D-03 meters */
/*                       XCORSTA(2) =  0.2939522229284325029D-04 meters */
/*                       XCORSTA(3) = -0.6051677912316721561D-04 meters */

/*  References: */

/*     Mathews, P. M., Dehant, V., and Gipson, J. M., 1997, ''Tidal station */
/*     displacements," J. Geophys. Res., 102(B9), pp. 20,469-20,477 */

/*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010), */
/*     IERS Technical Note No. 36, BKG (2010) */

/*  Revisions: */
/*  1996 March    23 V. Dehant      Original code */
/*  2009 July     31 B.E. Stetzler  Initial standardization of code */
/*  2009 July     31 B.E. Stetzler  Provided a test case */
/* ----------------------------------------------------------------------- */
    /* Parameter adjustments */
    --xcorsta;
    --xmon;
    --xsun;
    --xsta;

    /* Function Body */
/* Compute the normalized position vector of the IGS station. */
    rsta = norm8_(&xsta[1]);
    sinphi = xsta[3] / rsta;
    cosphi = sqrt(xsta[1] * xsta[1] + xsta[2] * xsta[2]) / rsta;
    sinla = xsta[2] / cosphi / rsta;
    cosla = xsta[1] / cosphi / rsta;
    costwola = cosla * cosla - sinla * sinla;
    sintwola = cosla * 2. * sinla;
/* Compute the normalized position vector of the Moon. */
    rmon = norm8_(&xmon[1]);
/* Compute the normalized position vector of the Sun. */
    rsun = norm8_(&xsun[1]);
/* Computing 2nd power */
    d__1 = cosphi;
/* Computing 2nd power */
    d__2 = xsun[1];
/* Computing 2nd power */
    d__3 = xsun[2];
/* Computing 2nd power */
    d__4 = rsun;
    drsun = dhi * -.75 * (d__1 * d__1) * *fac2sun * ((d__2 * d__2 - d__3 * 
	    d__3) * sintwola - xsun[1] * 2. * xsun[2] * costwola) / (d__4 * 
	    d__4);
/* Computing 2nd power */
    d__1 = cosphi;
/* Computing 2nd power */
    d__2 = xmon[1];
/* Computing 2nd power */
    d__3 = xmon[2];
/* Computing 2nd power */
    d__4 = rmon;
    drmon = dhi * -.75 * (d__1 * d__1) * *fac2mon * ((d__2 * d__2 - d__3 * 
	    d__3) * sintwola - xmon[1] * 2. * xmon[2] * costwola) / (d__4 * 
	    d__4);
/* Computing 2nd power */
    d__1 = xsun[1];
/* Computing 2nd power */
    d__2 = xsun[2];
/* Computing 2nd power */
    d__3 = rsun;
    dnsun = dli * 1.5 * sinphi * cosphi * *fac2sun * ((d__1 * d__1 - d__2 * 
	    d__2) * sintwola - xsun[1] * 2. * xsun[2] * costwola) / (d__3 * 
	    d__3);
/* Computing 2nd power */
    d__1 = xmon[1];
/* Computing 2nd power */
    d__2 = xmon[2];
/* Computing 2nd power */
    d__3 = rmon;
    dnmon = dli * 1.5 * sinphi * cosphi * *fac2mon * ((d__1 * d__1 - d__2 * 
	    d__2) * sintwola - xmon[1] * 2. * xmon[2] * costwola) / (d__3 * 
	    d__3);
/* Computing 2nd power */
    d__1 = xsun[1];
/* Computing 2nd power */
    d__2 = xsun[2];
/* Computing 2nd power */
    d__3 = rsun;
    desun = dli * -1.5 * cosphi * *fac2sun * ((d__1 * d__1 - d__2 * d__2) * 
	    costwola + xsun[1] * 2. * xsun[2] * sintwola) / (d__3 * d__3);
/* Computing 2nd power */
    d__1 = xmon[1];
/* Computing 2nd power */
    d__2 = xmon[2];
/* Computing 2nd power */
    d__3 = rmon;
    demon = dli * -1.5 * cosphi * *fac2mon * ((d__1 * d__1 - d__2 * d__2) * 
	    costwola + xmon[1] * 2. * xmon[2] * sintwola) / (d__3 * d__3);
    dr = drsun + drmon;
    dn = dnsun + dnmon;
    de = desun + demon;
    xcorsta[1] = dr * cosla * cosphi - de * sinla - dn * sinphi * cosla;
    xcorsta[2] = dr * sinla * cosphi + de * cosla - dn * sinphi * sinla;
    xcorsta[3] = dr * sinphi + dn * cosphi;
    return 0;
/*  Finished. */
/* +---------------------------------------------------------------------- */

/*  Copyright (C) 2008 */
/*  IERS Conventions Center */

/*  ================================== */
/*  IERS Conventions Software License */
/*  ================================== */

/*  NOTICE TO USER: */

/*  BY USING THIS SOFTWARE YOU ACCEPT THE FOLLOWING TERMS AND CONDITIONS */
/*  WHICH APPLY TO ITS USE. */

/*  1. The Software is provided by the IERS Conventions Center ("the */
/*     Center"). */

/*  2. Permission is granted to anyone to use the Software for any */
/*     purpose, including commercial applications, free of charge, */
/*     subject to the conditions and restrictions listed below. */

/*  3. You (the user) may adapt the Software and its algorithms for your */
/*     own purposes and you may distribute the resulting "derived work" */
/*     to others, provided that the derived work complies with the */
/*     following requirements: */

/*     a) Your work shall be clearly identified so that it cannot be */
/*        mistaken for IERS Conventions software and that it has been */
/*        neither distributed by nor endorsed by the Center. */

/*     b) Your work (including source code) must contain descriptions of */
/*        how the derived work is based upon and/or differs from the */
/*        original Software. */

/*     c) The name(s) of all modified routine(s) that you distribute */
/*        shall be changed. */

/*     d) The origin of the IERS Conventions components of your derived */
/*        work must not be misrepresented; you must not claim that you */
/*        wrote the original Software. */

/*     e) The source code must be included for all routine(s) that you */
/*        distribute.  This notice must be reproduced intact in any */
/*        source distribution. */

/*  4. In any published work produced by the user and which includes */
/*     results achieved by using the Software, you shall acknowledge */
/*     that the Software was used in obtaining those results. */

/*  5. The Software is provided to the user "as is" and the Center makes */
/*     no warranty as to its use or performance.   The Center does not */
/*     and cannot warrant the performance or results which the user may */
/*     obtain by using the Software.  The Center makes no warranties, */
/*     express or implied, as to non-infringement of third party rights, */
/*     merchantability, or fitness for any particular purpose.  In no */
/*     event will the Center be liable to the user for any consequential, */
/*     incidental, or special damages, including any lost profits or lost */
/*     savings, even if a Center representative has been advised of such */
/*     damages, or for any claim by any third party. */

/*  Correspondence concerning IERS Conventions software should be */
/*  addressed as follows: */

/*                     Gerard Petit */
/*     Internet email: gpetit[at]bipm.org */
/*     Postal address: IERS Conventions Center */
/*                     Time, frequency and gravimetry section, BIPM */
/*                     Pavillon de Breteuil */
/*                     92312 Sevres  FRANCE */

/*     or */

/*                     Brian Luzum */
/*     Internet email: brian.luzum[at]usno.navy.mil */
/*     Postal address: IERS Conventions Center */
/*                     Earth Orientation Department */
/*                     3450 Massachusetts Ave, NW */
/*                     Washington, DC 20392 */


/* ----------------------------------------------------------------------- */
} /* st1isem_ */

