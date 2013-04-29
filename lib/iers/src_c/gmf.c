/* ../src/gmf.f -- translated by f2c (version 20090411).
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

static integer c_n1 = -1;
static integer c__2 = 2;

/* Subroutine */ int gmf_(doublereal *dmjd, doublereal *dlat, doublereal *
	dlon, doublereal *dhgt, doublereal *zd, doublereal *gmfh, doublereal *
	gmfw)
{
    /* Initialized data */

    static doublereal ah_mean__[55] = { 125.17,.8503,.06936,-6.76,.1771,.0113,
	    .5963,.01808,.002801,-.001414,-1.212,.093,.003683,.001095,
	    4.671e-5,.3959,-.03867,.005413,-5.289e-4,3.229e-4,2.067e-5,.3,
	    .02031,.0059,4.573e-4,-7.619e-5,2.327e-6,3.845e-6,.1182,.01158,
	    .005445,6.219e-5,4.204e-6,-2.093e-6,1.54e-7,-4.28e-8,-.4751,
	    -.0349,.001758,4.019e-4,-2.799e-6,-1.287e-6,5.468e-7,7.58e-8,
	    -6.3e-9,-.116,.008301,8.771e-4,9.955e-5,-1.718e-6,-2.012e-6,
	    1.17e-8,1.79e-8,-1.3e-9,1e-10 };
    static doublereal bh_mean__[55] = { 0.,0.,.03249,0.,.03324,.0185,0.,
	    -.1115,.02519,.004923,0.,.02737,.01595,-7.332e-4,1.933e-4,0.,
	    -.04796,.006381,-1.599e-4,-3.685e-4,1.815e-5,0.,.07033,.002426,
	    -.001111,-1.357e-4,-7.828e-6,2.547e-6,0.,.005779,.003133,
	    -5.312e-4,-2.028e-5,2.323e-7,-9.1e-8,-1.65e-8,0.,.03688,-8.638e-4,
	    -8.514e-5,-2.828e-5,5.403e-7,4.39e-7,1.35e-8,1.8e-9,0.,-.02736,
	    -2.977e-4,8.113e-5,2.329e-7,8.451e-7,4.49e-8,-8.1e-9,-1.5e-9,
	    2e-10 };
    static doublereal ah_amp__[55] = { -.2738,-2.837,.01298,-.3588,.02413,
	    .03427,-.7624,.07272,.0216,-.003385,.4424,.03722,.02195,-.001503,
	    2.426e-4,.3013,.05762,.01019,-4.476e-4,6.79e-5,3.227e-5,.3123,
	    -.03535,.00484,3.025e-6,-4.363e-5,2.854e-7,-1.286e-6,-.6725,
	    -.0373,8.964e-4,1.399e-4,-3.99e-6,7.431e-6,-2.796e-7,-1.601e-7,
	    .04068,-.01352,7.282e-4,9.594e-5,2.07e-6,-9.62e-8,-2.742e-7,
	    -6.37e-8,-6.3e-9,.08625,-.005971,4.705e-4,2.335e-5,4.226e-6,
	    2.475e-7,-8.85e-8,-3.6e-8,-2.9e-9,0. };
    static doublereal bh_amp__[55] = { 0.,0.,-.1136,0.,-.1868,-.01399,0.,
	    -.1043,.01175,-.00224,0.,-.03222,.01333,-.002647,-2.316e-5,0.,
	    .05339,.01107,-.003116,-1.079e-4,-1.299e-5,0.,.004861,.008891,
	    -6.448e-4,-1.279e-5,6.358e-6,-1.417e-7,0.,.03041,.00115,-8.743e-4,
	    -2.781e-5,6.367e-7,-1.14e-8,-4.2e-8,0.,-.02982,-.003,1.394e-5,
	    -3.29e-5,-1.705e-7,7.44e-8,2.72e-8,-6.6e-9,0.,.01236,-9.981e-4,
	    -3.792e-5,-1.355e-5,1.162e-6,-1.789e-7,1.47e-8,-2.4e-9,-4e-10 };
    static doublereal aw_mean__[55] = { 56.4,1.555,-1.011,-3.975,.03171,.1065,
	    .6175,.1376,.04229,.003028,1.688,-.1692,.05478,.02473,6.059e-4,
	    2.278,.006614,-3.505e-4,-.006697,8.402e-4,7.033e-4,-3.236,.2184,
	    -.04611,-.01613,-.001604,5.42e-5,7.922e-5,-.2711,-.4406,-.03376,
	    -.002801,-4.09e-4,-2.056e-5,6.894e-6,2.317e-6,1.941,-.2562,.01598,
	    .005449,3.544e-4,1.148e-5,7.503e-6,-5.667e-7,-3.66e-8,.8683,
	    -.05931,-.001864,-1.277e-4,2.029e-4,1.269e-5,1.629e-6,9.66e-8,
	    -1.015e-7,-5e-10 };
    static doublereal bw_mean__[55] = { 0.,0.,.2592,0.,.02974,-.5471,0.,
	    -.5926,-.103,-.01567,0.,.171,.09025,.02689,.002243,0.,.3439,
	    .02402,.00541,.001601,9.669e-5,0.,.09502,-.03063,-.001055,
	    -1.067e-4,-1.13e-4,2.124e-5,0.,-.3129,.008463,2.253e-4,7.413e-5,
	    -9.376e-5,-1.606e-6,2.06e-6,0.,.2739,.001167,-2.246e-5,-1.287e-4,
	    -2.438e-5,-7.561e-7,1.158e-6,4.95e-8,0.,-.1344,.005342,3.775e-4,
	    -6.756e-5,-1.686e-6,-1.184e-6,2.768e-7,2.73e-8,5.7e-9 };
    static doublereal aw_amp__[55] = { .1023,-2.695,.3417,-.1405,.3175,.2116,
	    3.536,-.1505,-.0166,.02967,.3819,-.1695,-.07444,.007409,-.006262,
	    -1.836,-.01759,-.06256,-.002371,7.947e-4,1.501e-4,-.8603,-.136,
	    -.03629,-.003706,-2.976e-4,1.857e-5,3.021e-5,2.248,-.1178,.01255,
	    .001134,-2.161e-4,-5.817e-6,8.836e-7,-1.769e-7,.7313,-.1188,
	    .01145,.001011,1.083e-4,2.57e-6,-2.14e-6,-5.71e-8,2e-8,-1.632,
	    -.006948,-.003893,8.592e-4,7.577e-5,4.539e-6,-3.852e-7,-2.213e-7,
	    -1.37e-8,5.8e-9 };
    static doublereal bw_amp__[55] = { 0.,0.,-.08865,0.,-.4309,.0634,0.,.1162,
	    .06176,-.004234,0.,.253,.04017,-.006204,.004977,0.,-.1737,
	    -.005638,1.488e-4,4.857e-4,-1.809e-4,0.,-.1514,-.01685,.005333,
	    -7.611e-5,2.394e-5,8.195e-6,0.,.09326,-.01275,-3.071e-4,5.374e-5,
	    -3.391e-5,-7.436e-6,6.747e-7,0.,-.08637,-.003807,-6.833e-4,
	    -3.861e-5,-2.268e-5,1.454e-6,3.86e-7,-1.068e-7,0.,-.02658,
	    -.001947,7.131e-4,-3.506e-5,1.885e-7,5.792e-7,3.99e-8,2e-8,
	    -5.7e-9 };

    /* System generated locals */
    integer i__1, i__2, i__3, i__4;
    doublereal d__1, d__2;

    /* Builtin functions */
    double sin(doublereal);
    integer pow_ii(integer *, integer *);
    double pow_di(doublereal *, integer *), sqrt(doublereal), cos(doublereal);

    /* Local variables */
    static integer i__, j, k, m, n;
    static doublereal p[100]	/* was [10][10] */, t, ah, bh, ch, ap[55], bp[
	    55], aw, bw, cw;
    static integer ir;
    static doublereal c0h, aha, c10h, c11h, ahm, awa, phh, awm, doy, sum1, 
	    dfac[20], beta, a_ht__, b_ht__, c_ht__, sine, ht_corr_coef__, 
	    gamma, hs_km__, topcon, ht_corr__;

/* + */
/*  - - - - - - - - - */
/*   G M F */
/*  - - - - - - - - - */

/*  This routine is part of the International Earth Rotation and */
/*  Reference Systems Service (IERS) Conventions software collection. */

/*  This subroutine determines the Global Mapping Functions GMF (Boehm et al. 2006). */

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
/*     DMJD           d      Modified Julian Date */
/*     DLAT           d      Latitude given in radians (North Latitude) */
/*     DLON           d      Longitude given in radians (East Longitude) */
/*     DHGT           d      Height in meters (mean sea level) */
/*     ZD             d      Zenith distance in radians */

/*  Returned: */
/*     GMFH           d      Hydrostatic mapping function (Note 1) */
/*     GMFW           d      Wet mapping function (Note 1) */

/*  Notes: */

/*  1) The mapping functions are dimensionless scale factors. */

/*  2) This is from a 9x9 Earth Gravitational Model (EGM). */

/*  Test case: */
/*     given input: DMJD = 55055D0 */
/*                  DLAT = 0.6708665767D0 radians (NRAO, Green Bank, WV) */
/*                  DLON = -1.393397187D0 radians */
/*                  DHGT = 844.715D0 meters */
/*                  ZD   = 1.278564131D0 radians */

/*     expected output: GMFH = 3.425245519339138678D0 */
/*                      GMFW = 3.449589116182419257D0 */

/*  References: */

/*     Boehm, J., Niell, A., Tregoning, P. and Schuh, H., (2006), */
/*     "Global Mapping Functions (GMF): A new empirical mapping */
/*     function based on numerical weather model data", */
/*     Geophy. Res. Lett., Vol. 33, L07304, doi:10.1029/2005GL025545. */

/*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010), */
/*     IERS Technical Note No. 36, BKG (2010) */

/*  Revisions: */
/*  2005 August 30 J. Boehm    Original code */
/*  2009 August 11 B.E. Stetzler Added header and copyright */
/*  2009 August 12 B.E. Stetzler More modifications and defined twopi */
/*  2009 August 12 B.E. Stetzler Provided test case */
/*  2009 August 12 B.E. Stetzler Capitalized all variables for FORTRAN 77 */
/*                              compatibility and corrected test case */
/*                              latitude and longitude coordinates */
/* ----------------------------------------------------------------------- */
/* +--------------------------------------------------------------------- */
/*     Reference day is 28 January 1980 */
/*     This is taken from Niell (1996) to be consistent */
/* ---------------------------------------------------------------------- */
    doy = *dmjd - 44239. - 27;
/*     Define a parameter t */
    t = sin(*dlat);
/*     Define degree n and order m EGM */
    n = 9;
    m = 9;
/*     Determine n!  (factorial)  moved by 1 */
    dfac[0] = 1.;
    i__1 = (n << 1) + 1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	dfac[i__] = dfac[i__ - 1] * i__;
    }
/*     Determine Legendre functions (Heiskanen and Moritz, */
/*     Physical Geodesy, 1967, eq. 1-62) */
    i__1 = n;
    for (i__ = 0; i__ <= i__1; ++i__) {
	i__2 = min(i__,m);
	for (j = 0; j <= i__2; ++j) {
	    ir = (i__ - j) / 2;
	    sum1 = 0.;
	    i__3 = ir;
	    for (k = 0; k <= i__3; ++k) {
		i__4 = i__ - j - (k << 1);
		sum1 += pow_ii(&c_n1, &k) * dfac[(i__ << 1) - (k << 1)] / 
			dfac[k] / dfac[i__ - k] / dfac[i__ - j - (k << 1)] * 
			pow_di(&t, &i__4);
	    }
/*         Legendre functions moved by 1 */
/* Computing 2nd power */
	    d__2 = t;
	    d__1 = 1 - d__2 * d__2;
	    p[i__ + 1 + (j + 1) * 10 - 11] = 1. / pow_ii(&c__2, &i__) * sqrt(
		    pow_di(&d__1, &j)) * sum1;
	}
    }
/*     Calculate spherical harmonics */
    i__ = 0;
    for (n = 0; n <= 9; ++n) {
	i__1 = n;
	for (m = 0; m <= i__1; ++m) {
	    ++i__;
	    ap[i__ - 1] = p[n + 1 + (m + 1) * 10 - 11] * cos(m * *dlon);
	    bp[i__ - 1] = p[n + 1 + (m + 1) * 10 - 11] * sin(m * *dlon);
	}
    }
/*     Compute hydrostatic mapping function */
    bh = .0029;
    c0h = .062;
    if (*dlat < 0.) {
/* SOUTHERN HEMISPHERE */
	phh = 3.1415926535897932384626433;
	c11h = .007;
	c10h = .002;
    } else {
/* NORTHERN HEMISPHERE */
	phh = 0.;
	c11h = .005;
	c10h = .001;
    }
    ch = c0h + ((cos(doy / 365.25 * 6.283185307179586476925287 + phh) + 1.) * 
	    c11h / 2. + c10h) * (1. - cos(*dlat));
    ahm = 0.;
    aha = 0.;
    for (i__ = 1; i__ <= 55; ++i__) {
	ahm += (ah_mean__[i__ - 1] * ap[i__ - 1] + bh_mean__[i__ - 1] * bp[
		i__ - 1]) * 1e-5;
	aha += (ah_amp__[i__ - 1] * ap[i__ - 1] + bh_amp__[i__ - 1] * bp[i__ 
		- 1]) * 1e-5;
    }
    ah = ahm + aha * cos(doy / 365.25 * 6.283185307179586476925287);
    sine = sin(1.5707963267948966 - *zd);
    beta = bh / (sine + ch);
    gamma = ah / (sine + beta);
    topcon = ah / (bh / (ch + 1.) + 1.) + 1.;
    *gmfh = topcon / (sine + gamma);
/*     Height correction for hydrostatic mapping function from Niell (1996) */
    a_ht__ = 2.53e-5;
    b_ht__ = .00549;
    c_ht__ = .00114;
    hs_km__ = *dhgt / 1e3;
    beta = b_ht__ / (sine + c_ht__);
    gamma = a_ht__ / (sine + beta);
    topcon = a_ht__ / (b_ht__ / (c_ht__ + 1.) + 1.) + 1.;
    ht_corr_coef__ = 1. / sine - topcon / (sine + gamma);
    ht_corr__ = ht_corr_coef__ * hs_km__;
    *gmfh += ht_corr__;
/*     Compute wet mapping function */
    bw = .00146;
    cw = .04391;
    awm = 0.;
    awa = 0.;
    for (i__ = 1; i__ <= 55; ++i__) {
	awm += (aw_mean__[i__ - 1] * ap[i__ - 1] + bw_mean__[i__ - 1] * bp[
		i__ - 1]) * 1e-5;
	awa += (aw_amp__[i__ - 1] * ap[i__ - 1] + bw_amp__[i__ - 1] * bp[i__ 
		- 1]) * 1e-5;
    }
    aw = awm + awa * cos(doy / 365.25 * 6.283185307179586476925287);
    beta = bw / (sine + cw);
    gamma = aw / (sine + beta);
    topcon = aw / (bw / (cw + 1.) + 1.) + 1.;
    *gmfw = topcon / (sine + gamma);
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
} /* gmf_ */

