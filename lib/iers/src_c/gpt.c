/* ../src/gpt.f -- translated by f2c (version 20090411).
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

static doublereal c_b2 = 5.225;

/* Subroutine */ int gpt_(doublereal *dmjd, doublereal *dlat, doublereal *
	dlon, doublereal *dhgt, doublereal *pres, doublereal *temp, 
	doublereal *undu)
{
    /* Initialized data */

    static doublereal a_geoid__[55] = { -.56195,-.060794,-.20125,-.06418,
	    -.036997,10.098,16.436,14.065,1.9881,.64414,-4.7482,-3.229,.50652,
	    .38279,-.026646,1.7224,-.2797,.68177,-.096658,-.015113,.0029206,
	    -3.4621,-.38198,.032306,.0069915,-.0023068,-.0013548,4.7324e-6,
	    2.3527,1.2985,.21232,.022571,-.0037855,2.9449e-5,-1.6265e-4,
	    1.1711e-7,1.6732,.19858,.023975,-9.0013e-4,-.0022475,-3.3095e-5,
	    -1.204e-5,2.201e-6,-1.0083e-6,.86297,.58231,.020545,-.007811,
	    -1.4085e-4,-8.8459e-6,5.7256e-6,-1.5068e-6,4.0095e-7,-2.4185e-8 };
    static doublereal bt_amp__[55] = { 0.,0.,-.89895,0.,-1.079,-.12699,0.,
	    -.59033,.034865,-.032614,0.,-.02431,.015607,-.029833,-.0059048,0.,
	    .28383,.040509,-.018834,-.0012654,-1.3794e-4,0.,.13306,.03496,
	    -.0036799,-3.5626e-4,1.4814e-4,3.7932e-6,0.,.20801,.006564,
	    -.0034893,-2.7395e-4,7.4296e-5,-7.9927e-6,-1.0277e-6,0.,.036515,
	    -.0074319,-6.2873e-4,-8.2461e-5,3.1095e-5,-5.386e-7,-1.2055e-7,
	    -1.1517e-7,0.,.031404,.01558,-.0011428,3.3529e-5,1.0387e-5,
	    -1.9378e-6,-2.7327e-7,7.5833e-9,-9.2323e-9 };
    static doublereal b_geoid__[55] = { 0.,0.,-.065993,0.,.065364,-5.832,0.,
	    1.6961,-1.3557,1.2694,0.,-2.931,.94805,-.076243,.041076,0.,
	    -.51808,-.34583,-.043632,.0022101,-.010663,0.,.10927,-.29463,
	    .0014371,-.011452,-.0028156,-3.533e-4,0.,.44049,.055653,-.020396,
	    -.0017312,3.5805e-5,7.2682e-5,2.2535e-6,0.,.019502,.027919,
	    -.0081812,4.454e-4,8.8663e-5,5.5596e-5,2.4826e-6,1.0279e-6,0.,
	    .060529,-.035824,-.0051367,3.0119e-5,-2.9911e-5,1.9844e-5,
	    -1.2349e-6,-7.6756e-9,5.01e-8 };
    static doublereal ap_mean__[55] = { 1010.8,8.4886,1.4799,-13.897,.0037516,
	    -.14936,12.232,-.76615,-.067699,.0081002,-15.874,.36614,-.067807,
	    -.0036309,5.9966e-4,4.8163,-.37363,-.072071,.0019998,-6.2385e-4,
	    -3.7916e-4,4.7609,-.39534,.0086667,.011569,.0011441,-1.4193e-4,
	    -8.5723e-5,.65008,-.50889,-.015754,-.0028305,5.7458e-4,3.2577e-5,
	    -9.6052e-6,-2.7974e-6,1.353,-.27271,-3.0276e-4,.0036286,
	    -2.0398e-4,1.5846e-5,-7.7787e-6,1.121e-6,9.902e-8,.55046,-.27312,
	    .0032532,-.0024277,1.1596e-4,2.6421e-7,-1.3263e-6,2.7322e-7,
	    1.4058e-7,4.9414e-9 };
    static doublereal bp_mean__[55] = { 0.,0.,-1.2878,0.,.70444,.33222,0.,
	    -.29636,.0072248,.0079655,0.,1.0854,.011145,-.036513,.0031527,0.,
	    -.48434,.052023,-.013091,.0018515,1.5422e-4,0.,.68298,.0025261,
	    -9.9703e-4,-.0010829,1.7688e-4,-3.1418e-5,0.,-.37018,.043234,
	    .0072559,3.1516e-4,2.0024e-5,-8.0581e-6,-2.3653e-6,0.,.10298,
	    -.015086,.0056186,3.2613e-5,4.0567e-5,-1.3925e-6,-3.6219e-7,
	    -2.0176e-8,0.,-.18364,.018508,7.5016e-4,-9.6139e-5,-3.1995e-6,
	    1.3868e-7,-1.9486e-7,3.0165e-10,-6.4376e-10 };
    static doublereal ap_amp__[55] = { -.10444,.16618,-.063974,1.0922,.57472,
	    -.30277,-3.5087,.0071264,-.1403,.03705,.40208,-.30431,-.13292,
	    .0046746,-1.5902e-4,2.8624,-.39315,-.064371,.016444,-.0023403,
	    4.2127e-5,1.9945,-.60907,-.035386,-.001091,-1.2799e-4,4.097e-5,
	    2.2131e-5,-.53292,-.29765,-.032877,.0017691,5.9692e-5,3.1725e-5,
	    2.0741e-5,-3.7622e-7,2.6372,-.31165,.016439,2.1633e-4,1.7485e-4,
	    2.1587e-5,6.1064e-6,-1.3755e-8,-7.8748e-8,-.59152,-.17676,
	    .0081807,.0010445,2.3432e-4,9.3421e-6,2.8104e-6,-1.5788e-7,
	    -3.0648e-8,2.6421e-10 };
    static doublereal bp_amp__[55] = { 0.,0.,.9334,0.,.82346,.22082,0.,.96177,
	    -.01565,.0012708,0.,-.39913,.02802,.028334,8.598e-4,0.,.30545,
	    -.021691,6.4067e-4,-3.6528e-5,-1.1166e-4,0.,-.076974,-.018986,
	    .0056896,-2.4159e-4,-2.3033e-4,-9.6783e-6,0.,-.10218,-.013916,
	    -.0041025,-5.134e-5,-7.0114e-5,-3.3152e-7,1.6901e-6,0.,-.012422,
	    .0025072,.0011205,-1.3034e-4,-2.3971e-5,-2.6622e-6,5.7852e-7,
	    4.5847e-8,0.,.044777,-.0030421,2.6062e-5,-7.2421e-5,1.9119e-6,
	    3.9236e-7,2.239e-7,2.9765e-9,-4.6452e-9 };
    static doublereal at_mean__[55] = { 16.257,2.1224,.92569,-25.974,1.451,
	    .092468,-.53192,.21094,-.06921,-.03406,-4.6569,.26385,-.036093,
	    .010198,-.0018783,.74983,.11741,.03994,.0051348,.0059111,
	    8.6133e-6,.63057,.15203,.039702,.0046334,2.4406e-4,1.5189e-4,
	    1.9581e-7,.54414,.35722,.052763,.0041147,-2.7239e-4,-5.9957e-5,
	    1.6394e-6,-7.3045e-7,-2.9394,.055579,.018852,.0034272,-2.3193e-5,
	    -2.9349e-5,3.6397e-7,2.049e-6,-6.4719e-8,-.52225,.20799,.0013477,
	    3.1613e-4,-2.2285e-4,-1.8137e-5,-1.5177e-7,6.1343e-7,7.8566e-8,
	    1.0749e-9 };
    static doublereal bt_mean__[55] = { 0.,0.,1.021,0.,.60194,.12292,0.,
	    -.42184,.1823,.042329,0.,.093312,.095346,-.0019724,.0058776,0.,
	    -.2094,.034199,-.0057672,-.002159,5.6815e-4,0.,.22858,.012283,
	    -.0093679,-.0014233,-1.5962e-4,4.016e-5,0.,.036353,-9.4263e-4,
	    -.0036762,5.8608e-5,-2.6391e-5,3.2095e-6,-1.1605e-6,0.,.16306,
	    .013293,-.0011395,5.1097e-5,3.3977e-5,7.6449e-6,-1.7602e-7,
	    -7.6558e-8,0.,-.045415,-.018027,3.6561e-4,-1.1274e-4,1.3047e-5,
	    2.0001e-6,-1.5152e-7,-2.7807e-8,7.7491e-9 };
    static doublereal at_amp__[55] = { -1.8654,-9.0041,-.12974,-3.6053,
	    .020284,.21872,-1.3015,.40355,.22216,-.0040605,1.9623,.42887,
	    .21437,-.010061,-.0011368,-.069235,.56758,.11917,-.0070765,
	    3.0017e-4,3.0601e-4,1.6559,.20722,.060013,1.7023e-4,-9.2424e-4,
	    1.1269e-5,-6.9911e-6,-2.0886,-.067879,-8.5922e-4,-.0016087,
	    -4.5549e-5,3.3178e-5,-6.1715e-6,-1.4446e-6,-.3721,.15775,
	    -.0017827,-4.4396e-4,2.2844e-4,-1.1215e-5,-2.112e-6,-9.6421e-7,
	    -1.417e-8,.7872,-.044238,-.001512,-9.4119e-4,4.0645e-6,-4.9253e-6,
	    -1.8656e-6,-4.0736e-7,-4.9594e-8,1.6134e-9 };

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1;

    /* Builtin functions */
    double cos(doublereal), sin(doublereal), pow_dd(doublereal *, doublereal *
	    );

    /* Local variables */
    static integer i__, m, n;
    static doublereal v[100]	/* was [10][10] */, w[100]	/* was [10][
	    10] */, x, y, z__, apa, ata, apm, atm, doy;
    static integer mmax, nmax;
    static doublereal hort, temp0, pres0;

/* + */
/*  - - - - - - - - - */
/*   G P T */
/*  - - - - - - - - - */

/*  This routine is part of the International Earth Rotation and */
/*  Reference Systems Service (IERS) Conventions software collection. */

/*  This subroutine determines Global Pressure and Temperature (Boehm et al. 2007) */
/*  based on Spherical Harmonics up to degree and order 9. */

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
/*     DHGT           d      Ellipsoidal height in meters */

/*  Returned: */
/*     PRES           d      Pressure given in hPa */
/*     TEMP           d      Temperature in degrees Celsius */
/*     UNDU           d      Geoid undulation in meters (Note 1) */

/*  Notes: */

/*  1) This is from a 9x9 Earth Gravitational Model (EGM). */

/*  Test case: */
/*     given input: DMJD = 55055D0 */
/*                  DLAT = 0.6708665767D0 radians (NRAO, Green Bank, WV) */
/*                  DLON = -1.393397187D0 radians */
/*                  DHGT = 812.546 meters */
/*     expected output: PRES = 919.1930225603726967D0 hPa */
/*                      TEMP = 28.94460920276309679D0 degrees Celsius */
/*                      UNDU = -42.78796423912972813D0 meters */

/*  References: */

/*     Boehm, J., Heinkelmann, R. and Schuh, H., 2007, "Short Note: A */
/*     Global model of pressure and temperature for geodetic applications", */
/*     Journal of Geodesy, 81(10), pp. 679-683. */

/*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010), */
/*     IERS Technical Note No. 36, BKG (2010) */

/*  Revisions: */
/*  2006 June 12 J. Boehm    Original code */
/*  2006 June 16 J. Boehm    Accounted for geoid undulation */
/*  2006 August 14 O. Montenbruck Recursions for Legendre polynomials */
/*  2009 February 13 B.E. Stetzler Added header and copyright */
/*  2009 March 30 B.E. Stetzler More modifications and defined twopi */
/*  2009 March 31 B.E. Stetzler Provided test case */
/*  2009 July  29 B.E. Stetzler Capitalized all variables for FORTRAN 77 */
/*                              compatibility and corrected test case */
/*                              latitude and longitude coordinates */
/* ----------------------------------------------------------------------- */
/*     Reference day is 28 January 1980 */
/*     This is taken from Niell (1996) to be consistent (See References) */
/*     For constant values use: doy = 91.3125 */
    doy = *dmjd - 44239. + 1 - 28;
/*     Define degree n and order m EGM */
    nmax = 9;
    mmax = 9;
/*     Define unit vector */
    x = cos(*dlat) * cos(*dlon);
    y = cos(*dlat) * sin(*dlon);
    z__ = sin(*dlat);
/*     Legendre polynomials */
    v[0] = 1.;
    w[0] = 0.;
    v[1] = z__ * v[0];
    w[1] = 0.f;
    i__1 = nmax;
    for (n = 2; n <= i__1; ++n) {
	v[n] = (((n << 1) - 1) * z__ * v[n - 1] - (n - 1) * v[n - 2]) / n;
	w[n] = 0.;
    }
    i__1 = nmax;
    for (m = 1; m <= i__1; ++m) {
	v[m + 1 + (m + 1) * 10 - 11] = ((m << 1) - 1) * (x * v[m + m * 10 - 
		11] - y * w[m + m * 10 - 11]);
	w[m + 1 + (m + 1) * 10 - 11] = ((m << 1) - 1) * (x * w[m + m * 10 - 
		11] + y * v[m + m * 10 - 11]);
	if (m < nmax) {
	    v[m + 2 + (m + 1) * 10 - 11] = ((m << 1) + 1) * z__ * v[m + 1 + (
		    m + 1) * 10 - 11];
	    w[m + 2 + (m + 1) * 10 - 11] = ((m << 1) + 1) * z__ * w[m + 1 + (
		    m + 1) * 10 - 11];
	}
	i__2 = nmax;
	for (n = m + 2; n <= i__2; ++n) {
	    v[n + 1 + (m + 1) * 10 - 11] = (((n << 1) - 1) * z__ * v[n + (m + 
		    1) * 10 - 11] - (n + m - 1) * v[n - 1 + (m + 1) * 10 - 11]
		    ) / (n - m);
	    w[n + 1 + (m + 1) * 10 - 11] = (((n << 1) - 1) * z__ * w[n + (m + 
		    1) * 10 - 11] - (n + m - 1) * w[n - 1 + (m + 1) * 10 - 11]
		    ) / (n - m);
	}
    }
/*     Geoidal height */
    *undu = 0.;
    i__ = 0;
    i__1 = nmax;
    for (n = 0; n <= i__1; ++n) {
	i__2 = n;
	for (m = 0; m <= i__2; ++m) {
	    ++i__;
	    *undu += a_geoid__[i__ - 1] * v[n + 1 + (m + 1) * 10 - 11] + 
		    b_geoid__[i__ - 1] * w[n + 1 + (m + 1) * 10 - 11];
	}
    }
/*     orthometric height */
    hort = *dhgt - *undu;
/*     Surface pressure on the geoid */
    apm = 0.;
    apa = 0.;
    i__ = 0;
    i__1 = nmax;
    for (n = 0; n <= i__1; ++n) {
	i__2 = n;
	for (m = 0; m <= i__2; ++m) {
	    ++i__;
	    apm += ap_mean__[i__ - 1] * v[n + 1 + (m + 1) * 10 - 11] + 
		    bp_mean__[i__ - 1] * w[n + 1 + (m + 1) * 10 - 11];
	    apa += ap_amp__[i__ - 1] * v[n + 1 + (m + 1) * 10 - 11] + 
		    bp_amp__[i__ - 1] * w[n + 1 + (m + 1) * 10 - 11];
	}
    }
    pres0 = apm + apa * cos(doy / 365.25 * 6.283185307179586476925287);
/*     height correction for pressure */
    d__1 = 1. - hort * 2.26e-5;
    *pres = pres0 * pow_dd(&d__1, &c_b2);
/*     Surface temperature on the geoid */
    atm = 0.;
    ata = 0.;
    i__ = 0;
    i__1 = nmax;
    for (n = 0; n <= i__1; ++n) {
	i__2 = n;
	for (m = 0; m <= i__2; ++m) {
	    ++i__;
	    atm += at_mean__[i__ - 1] * v[n + 1 + (m + 1) * 10 - 11] + 
		    bt_mean__[i__ - 1] * w[n + 1 + (m + 1) * 10 - 11];
	    ata += at_amp__[i__ - 1] * v[n + 1 + (m + 1) * 10 - 11] + 
		    bt_amp__[i__ - 1] * w[n + 1 + (m + 1) * 10 - 11];
	}
    }
    temp0 = atm + ata * cos(doy / 365.25 * 6.283185307179586476925287);
/*     height correction for temperature */
    *temp = temp0 - hort * .0065;
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
} /* gpt_ */

