/* ../src/sprod.f -- translated by f2c (version 20090411).
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

/* Subroutine */ int sprod_(doublereal *x, doublereal *y, doublereal *scal, 
	doublereal *r1, doublereal *r2)
{
    /* Builtin functions */
    double sqrt(doublereal);

/* + */
/*  - - - - - - - - - - - */
/*   S P R O D */
/*  - - - - - - - - - - - */

/*  This routine is part of the International Earth Rotation and */
/*  Reference Systems Service (IERS) Conventions software collection. */

/*  This subroutine computes the scalar product of two vectors and */
/*  their norms. */

/*  In general, Class 1, 2, and 3 models represent physical effects that */
/*  act on geodetic parameters while canonical models provide lower-level */
/*  representations or basic computations that are used by Class 1, 2, or */
/*  3 models. */

/*  Status: Canonical model */

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
/*     X            d(3)      components of vector x */
/*     Y            d(3)      components of vector y */

/*  Returned: */
/*     SCAL         d      scalar product of vector x and vector y */
/*     R1           d      length of vector x */
/*     R2           d      length of vector y */

/*  Called: */
/*     None */

/*  Test case: */
/*     given input: X(1) = 2D0	Y(1) = 1D0 */
/*                  X(2) = 2D0	Y(2) = 3D0 */
/*                  X(3) = 3D0	Y(3) = 4D0 */

/*     expected output: SCAL = 20D0 */
/*                      R1 = 4.123105625617660586D0 */
/*                      R2 = 5.099019513592784492D0 */

/*  References: */

/*     Mathews, P. M., Dehant, V., and Gipson, J. M., 1997, ''Tidal station */
/*     displacements," J. Geophys. Res., 102(B9), pp. 20,469-20,477 */

/*     Petit, G. and Luzum, B. (eds.), IERS Conventions (2010), */
/*     IERS Technical Note No. 36, BKG (2010) */

/*  Revisions: */
/*  2009 July 10 B.E.Stetzler Initial standardization of function, */
/*                            explicit exponential notation and */
/*                            provided a test case */
/* ----------------------------------------------------------------------- */
    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
    *r1 = sqrt(x[1] * x[1] + x[2] * x[2] + x[3] * x[3]);
    *r2 = sqrt(y[1] * y[1] + y[2] * y[2] + y[3] * y[3]);
    *scal = x[1] * y[1] + x[2] * y[2] + x[3] * y[3];
    return 0;
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
} /* sprod_ */

