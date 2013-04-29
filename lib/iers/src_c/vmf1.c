/* ../src/vmf1.f -- translated by f2c (version 20090411).
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

/* Subroutine */ int vmf1_(doublereal *ah, doublereal *aw, doublereal *dmjd, 
	doublereal *dlat, doublereal *zd, doublereal *vmf1h, doublereal *
	vmf1w)
{
    /* Builtin functions */
    double cos(doublereal), sin(doublereal);

    /* Local variables */
    static doublereal bh, ch, bw, cw, c0h, c10h, c11h, phh, doy, beta, sine, 
	    gamma, topcon;

/* + */
/*  - - - - - - - - - */
/*   V M F 1 */
/*  - - - - - - - - - */

/*  This routine is part of the International Earth Rotation and */
/*  Reference Systems Service (IERS) Conventions software collection. */

/*  This subroutine determines the Vienna Mapping Function 1 (VMF1) (Boehm et al, 2006). */
/*  This is the site dependent version. */

/*  In general, Class 1, 2, and 3 models represent physical effects that */
/*  act on geodetic parameters while canonical models provide lower-level */
/*  representations or basic computations that are used by Class 1, 2, or */
/*  3 models. */

/*  Status: Class 1 model */

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
/*     AH             d      Hydrostatic coefficient a (Note 1) */
/*     AW             d      Wet coefficient a (Note 1) */
/*     DMJD           d      Modified Julian Date */
/*     DLAT           d      Latitude given in radians (North Latitude) */
/*     ZD             d      Zenith distance in radians */

/*  Returned: */
/*     VMF1H           d      Hydrostatic mapping function (Note 2) */
/*     VMF1W           d      Wet mapping function (Note 2) */

/*  Notes: */

/*  1) The coefficients can be obtained from the primary website */
/*     http://ggosatm.hg.tuwien.ac.at/DELAY/ or the back-up website */
/*     http://www.hg.tuwien.ac.at/~ecmwf1/. */

/*  2) The mapping functions are dimensionless scale factors. */

/*  Test case: */
/*     given input: AH   = 0.00125711D0 */
/*                  AW   = 0.00058801D0 */
/*                  DMJD = 55055D0 */
/*                  DLAT = 0.6708665767D0 radians (NRAO, Green Bank, WV) */
/*                  ZD   = 1.278564131D0 radians */

/*     expected output: VMF1H = 3.425054275537719128D0 */
/*                      VMF1W = 3.449100942061193553D0 */

/*  References: */

/*     Boehm, J., Werl, B., and Schuh, H., (2006), */
/*     "Troposhere mapping functions for GPS and very long baseline */
/*     interferometry from European Centre for Medium-Range Weather */
/*     Forecasts operational analysis data," J. Geophy. Res., Vol. 111, */
/*     B02406, doi:10.1029/2005JB003629 */

/*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010), */
/*     IERS Technical Note No. 36, BKG (2010) */

/*  Revisions: */
/*  2005 October 02 J. Boehm          Original code */
/*  2009 August 17 B.E. Stetzler      Added header and copyright */
/*  2009 August 17 B.E. Stetzler      More modifications and defined twopi */
/*  2009 August 17 B.E. Stetzler      Provided test case */
/*  2009 August 17 B.E. Stetzler      Capitalized all variables for FORTRAN 77 */
/*                                    compatibility */
/*  2010 September 08 B.E. Stetzler   Provided new primary website to obtain */
/*                                    VMF coefficients */
/* ----------------------------------------------------------------------- */
/* +--------------------------------------------------------------------- */
/*     Reference day is 28 January 1980 */
/*     This is taken from Niell (1996) to be consistent */
/* ---------------------------------------------------------------------- */
    doy = *dmjd - 44239. - 27;
    bh = .0029;
    c0h = .062;
    if (*dlat < 0.) {
/* southern hemisphere */
	phh = 3.1415926535897932384626433;
	c11h = .007;
	c10h = .002;
    } else {
/* northern hemisphere */
	phh = 0.;
	c11h = .005;
	c10h = .001;
    }
    ch = c0h + ((cos(doy / 365.25 * 6.283185307179586476925287 + phh) + 1.) * 
	    c11h / 2. + c10h) * (1. - cos(*dlat));
    sine = sin(1.5707963267948966 - *zd);
    beta = bh / (sine + ch);
    gamma = *ah / (sine + beta);
    topcon = *ah / (bh / (ch + 1.) + 1.) + 1.;
    *vmf1h = topcon / (sine + gamma);
    bw = .00146;
    cw = .04391;
    beta = bw / (sine + cw);
    gamma = *aw / (sine + beta);
    topcon = *aw / (bw / (cw + 1.) + 1.) + 1.;
    *vmf1w = topcon / (sine + gamma);
/* Finished. */
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
    return 0;
} /* vmf1_ */

