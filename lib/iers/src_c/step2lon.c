/* ../src/step2lon.f -- translated by f2c (version 20090411).
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

/* Table of constant values */

static doublereal c_b2 = 360.;

/* Subroutine */ int step2lon_(doublereal *xsta, doublereal *t, doublereal *
	xcorsta)
{
    /* Initialized data */

    static doublereal datdi[45]	/* was [9][5] */ = { 0.,0.,0.,1.,0.,.47,.23,
	    .16,.07,0.,2.,0.,0.,0.,-.2,-.12,-.11,-.05,1.,0.,-1.,0.,0.,-.11,
	    -.08,-.09,-.04,2.,0.,0.,0.,0.,-.13,-.11,-.15,-.07,2.,0.,0.,1.,0.,
	    -.05,-.05,-.06,-.03 };

    /* System generated locals */
    doublereal d__1, d__2, d__3;

    /* Builtin functions */
    double sqrt(doublereal), d_mod(doublereal *, doublereal *), cos(
	    doublereal), sin(doublereal);

    /* Local variables */
    static doublereal h__;
    static integer i__, j;
    static doublereal p, s, de, dn, dr, pr, ps, zns, rsta, cosla, sinla, 
	    thetaf, cosphi, dn_tot__, sinphi, dr_tot__, deg2rad;

/* + */
/*  - - - - - - - - - - - */
/*   S T E P 2 L O N */
/*  - - - - - - - - - - - */

/*  This routine is part of the International Earth Rotation and */
/*  Reference Systems Service (IERS) Conventions software collection. */

/*  This subroutine gives the in-phase and out-of-phase corrections */
/*  induced by mantle anelasticity in the long period band. */

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
/*     T             d      Centuries since J2000 */

/*  Returned: */
/*     XCORSTA       d(3)   In phase and out of phase station corrections */
/*                          for diurnal band (Note 2) */

/*  Notes: */

/*  1) The IGS station is in ITRF co-rotating frame.  All coordinates are */
/*     expressed in meters. */

/*  2) All coordinates are expressed in meters. */

/*  Test case: */
/*     given input: XSTA(1) = 4075578.385D0 meters */
/*                  XSTA(2) =  931852.890D0 meters */
/*                  XSTA(3) = 4801570.154D0 meters */
/*                  T       = 0.1059411362080767D0 Julian centuries */

/*     expected output:  XCORSTA(1) = -0.9780962849562107762D-04 meters */
/*                       XCORSTA(2) = -0.2236349699932734273D-04 meters */
/*                       XCORSTA(3) =  0.3561945821351565926D-03 meters */

/*  References: */

/*     Mathews, P. M., Dehant, V., and Gipson, J. M., 1997, ''Tidal station */
/*     displacements," J. Geophys. Res., 102(B9), pp. 20,469-20,477 */

/*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010), */
/*     IERS Technical Note No. 36, BKG (2010) */

/*  Revisions: */
/*  1996 March    23 V. Dehant      Original code */
/*  2009 August   07 B.E. Stetzler  Initial standardization of code */
/*                                  and found unnecessary variables tau */
/*                                  and fhr */
/*  2009 August   07 B.E. Stetzler  Provided a test case */
/*  2009 August   07 B.E. Stetzler  Capitalized all variables for */
/*                                  Fortran 77 compatibility */
/*  2010 October  20 B.E. Stetzler  Input T corrected to be number of */
/*                                  centuries since J2000 */
/* ----------------------------------------------------------------------- */
    /* Parameter adjustments */
    --xcorsta;
    --xsta;

    /* Function Body */
    deg2rad = .017453292519943295;
/*  Compute the phase angles in degrees. */
    s = ((*t * 1.85139e-6 - .0014663889) * *t + 481267.88194) * *t + 
	    218.31664563;
    pr = (((*t * 7e-9 + 2.1e-8) * *t + 3.08889e-4) * *t + 1.396971278) * *t;
    s += pr;
    h__ = (((*t * -6.54e-9 + 2e-8) * *t + 3.0322222e-4) * *t + 36000.7697489) 
	    * *t + 280.46645;
    p = (((*t * 5.263e-8 - 1.24991e-5) * *t - .01032172222) * *t + 
	    4069.01363525) * *t + 83.35324312;
    zns = (((*t * 1.65e-8 - 2.13944e-6) * *t - .00207561111) * *t + 
	    1934.13626197) * *t + 234.95544499;
    ps = (((*t * -3.34e-9 - 1.778e-8) * *t + 4.5688889e-4) * *t + 
	    1.71945766667) * *t + 282.93734098;
/* Computing 2nd power */
    d__1 = xsta[1];
/* Computing 2nd power */
    d__2 = xsta[2];
/* Computing 2nd power */
    d__3 = xsta[3];
    rsta = sqrt(d__1 * d__1 + d__2 * d__2 + d__3 * d__3);
    sinphi = xsta[3] / rsta;
/* Computing 2nd power */
    d__1 = xsta[1];
/* Computing 2nd power */
    d__2 = xsta[2];
    cosphi = sqrt(d__1 * d__1 + d__2 * d__2) / rsta;
    cosla = xsta[1] / cosphi / rsta;
    sinla = xsta[2] / cosphi / rsta;
/* Reduce angles to between the range 0 and 360. */
    s = d_mod(&s, &c_b2);
/*      TAU = DMOD(TAU,360D0) */
    h__ = d_mod(&h__, &c_b2);
    p = d_mod(&p, &c_b2);
    zns = d_mod(&zns, &c_b2);
    ps = d_mod(&ps, &c_b2);
    dr_tot__ = 0.;
    dn_tot__ = 0.;
    for (i__ = 1; i__ <= 3; ++i__) {
	xcorsta[i__] = 0.;
/* L99: */
    }
    for (j = 1; j <= 5; ++j) {
	thetaf = (datdi[j * 9 - 9] * s + datdi[j * 9 - 8] * h__ + datdi[j * 9 
		- 7] * p + datdi[j * 9 - 6] * zns + datdi[j * 9 - 5] * ps) * 
		deg2rad;
/* Computing 2nd power */
	d__1 = sinphi;
/* Computing 2nd power */
	d__2 = sinphi;
	dr = datdi[j * 9 - 4] * (d__1 * d__1 * 3. - 1.) / 2. * cos(thetaf) + 
		datdi[j * 9 - 2] * (d__2 * d__2 * 3. - 1.) / 2. * sin(thetaf);
	dn = datdi[j * 9 - 3] * (cosphi * sinphi * 2.) * cos(thetaf) + datdi[
		j * 9 - 1] * (cosphi * sinphi * 2.) * sin(thetaf);
	de = 0.;
	dr_tot__ += dr;
	dn_tot__ += dn;
	xcorsta[1] = xcorsta[1] + dr * cosla * cosphi - de * sinla - dn * 
		sinphi * cosla;
	xcorsta[2] = xcorsta[2] + dr * sinla * cosphi + de * cosla - dn * 
		sinphi * sinla;
	xcorsta[3] = xcorsta[3] + dr * sinphi + dn * cosphi;
/* L98: */
    }
    for (i__ = 1; i__ <= 3; ++i__) {
	xcorsta[i__] /= 1e3;
/* L97: */
    }
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
} /* step2lon_ */

