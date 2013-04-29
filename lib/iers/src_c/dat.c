/* ../src/dat.f -- translated by f2c (version 20090411).
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

/* Subroutine */ int dat_(integer *iy, integer *im, integer *id, doublereal *
	fd, doublereal *deltat, integer *j)
{
    /* Initialized data */

    static integer idate[78]	/* was [2][39] */ = { 1960,1,1961,1,1961,8,
	    1962,1,1963,11,1964,1,1964,4,1964,9,1965,1,1965,3,1965,7,1965,9,
	    1966,1,1968,2,1972,1,1972,7,1973,1,1974,1,1975,1,1976,1,1977,1,
	    1978,1,1979,1,1980,1,1981,7,1982,7,1983,7,1985,7,1988,1,1990,1,
	    1991,1,1992,7,1993,7,1994,7,1996,1,1997,7,1999,1,2006,1,2009,1 };
    static doublereal dats[39] = { 1.417818,1.422818,1.372818,1.845858,
	    1.945858,3.24013,3.34013,3.44013,3.54013,3.64013,3.74013,3.84013,
	    4.31317,4.21317,10.,11.,12.,13.,14.,15.,16.,17.,18.,19.,20.,21.,
	    22.,23.,24.,25.,26.,27.,28.,29.,30.,31.,32.,33.,34. };
    static doublereal drift[28]	/* was [2][14] */ = { 37300.,.001296,37300.,
	    .001296,37300.,.001296,37665.,.0011232,37665.,.0011232,38761.,
	    .001296,38761.,.001296,38761.,.001296,38761.,.001296,38761.,
	    .001296,38761.,.001296,38761.,.001296,39126.,.002592,39126.,
	    .002592 };

    static integer i__, m;
    static doublereal da;
    static integer is, js;
    static doublereal djm, djm0;
    static logical more;
    extern /* Subroutine */ int cal2jd_(integer *, integer *, integer *, 
	    doublereal *, doublereal *, integer *);

/* + */
/*  - - - - - - - - */
/*   D A T */
/*  - - - - - - - - */

/*  For a given UTC date, calculate delta(AT) = TAI-UTC. */

/*     :------------------------------------------: */
/*     :                                          : */
/*     :                 IMPORTANT                : */
/*     :                                          : */
/*     :  A new version of this routine must be   : */
/*     :  produced whenever a new leap second is  : */
/*     :  announced.  There are five items to     : */
/*     :  change on each such occasion:           : */
/*     :                                          : */
/*     :  1) The parameter NDAT must be           : */
/*     :     increased by 1.                      : */
/*     :                                          : */
/*     :  2) A new line must be added to the set  : */
/*     :     of DATA statements that initialize   : */
/*     :     the arrays IDATE and DATS.           : */
/*     :                                          : */
/*     :  3) The parameter IYV must be set to     : */
/*     :     the current year.                    : */
/*     :                                          : */
/*     :  4) The "Latest leap second" comment     : */
/*     :     below must be set to the new leap    : */
/*     :     second date.                         : */
/*     :                                          : */
/*     :  5) The "This revision" comment, later,  : */
/*     :     must be set to the current date.     : */
/*     :                                          : */
/*     :  Change (3) must also be carried out     : */
/*     :  whenever the routine is re-issued,      : */
/*     :  even if no leap seconds have been       : */
/*     :  added.                                  : */
/*     :                                          : */
/*     :  Latest leap second:  2008 December 31   : */
/*     :                                          : */
/*     :__________________________________________: */

/*  This routine is part of the International Astronomical Union's */
/*  SOFA (Standards of Fundamental Astronomy) software collection. */

/*  Status:  support routine. */

/*  Given: */
/*     IY       i     UTC:  year (Notes 1 and 2) */
/*     IM       i           month (Note 2) */
/*     ID       i           day (Notes 2 and 3) */
/*     FD       d           fraction of day (Note 4) */

/*  Returned: */
/*     DELTAT   d     TAI minus UTC, seconds */
/*     J        i     status (Note 5): */
/*                       1 = dubious year (Note 1) */
/*                       0 = OK */
/*                      -1 = bad year */
/*                      -2 = bad month */
/*                      -3 = bad day (Note 3) */
/*                      -4 = bad fraction (Note 4) */

/*  Notes: */

/*  1) UTC began at 1960 January 1.0 (JD 2436934.5) and it is improper */
/*     to call the routine with an earlier date.  If this is attempted, */
/*     zero is returned together with a warning status. */

/*     Because leap seconds cannot, in principle, be predicted in */
/*     advance, a reliable check for dates beyond the valid range is */
/*     impossible.  To guard against gross errors, a year five or more */
/*     after the release year of the present routine (see parameter IYV) */
/*     is considered dubious.  In this case a warning status is returned */
/*     but the result is computed in the normal way. */

/*     For both too-early and too-late years, the warning status is J=+1. */
/*     This is distinct from the error status J=-1, which signifies a */
/*     year so early that JD could not be computed. */

/*  2) If the specified date is for a day which ends with a leap second, */
/*     the UTC-TAI value returned is for the period leading up to the */
/*     leap second.  If the date is for a day which begins as a leap */
/*     second ends, the UTC-TAI returned is for the period following the */
/*     leap second. */

/*  3) The day number must be in the normal calendar range, for example */
/*     1 through 30 for April.  The "almanac" convention of allowing */
/*     such dates as January 0 and December 32 is not supported in this */
/*     routine, in order to avoid confusion near leap seconds. */

/*  4) The fraction of day is used only for dates before the introduction */
/*     of leap seconds, the first of which occurred at the end of 1971. */
/*     It is tested for validity (zero to less than 1 is the valid range) */
/*     even if not used;  if invalid, zero is used and status J=-4 is */
/*     returned.  For many applications, setting FD to zero is */
/*     acceptable;  the resulting error is always less than 3 ms (and */
/*     occurs only pre-1972). */

/*  5) The status value returned in the case where there are multiple */
/*     errors refers to the first error detected.  For example, if the */
/*     month and day are 13 and 32 respectively, J=-2 (bad month) will be */
/*     returned. */

/*  6) In cases where a valid result is not available, zero is returned. */

/*  References: */

/*  1) For dates from 1961 January 1 onwards, the expressions from the */
/*     file ftp://maia.usno.navy.mil/ser7/tai-utc.dat are used. */

/*  2) The 5ms timestep at 1961 January 1 is taken from 2.58.1 (p87) of */
/*     the 1992 Explanatory Supplement. */

/*  Called: */
/*     CAL2JD       Gregorian calendar to Julian Day number */

/*  This revision:  2008 July 5 */

/*  Copyright (C) 2008 IAU SOFA Review Board.  See notes at end. */

/* ----------------------------------------------------------------------- */
/*  Release year for this version of DAT */
/*  Number of Delta(AT) changes (increase by 1 for each new leap second) */
/*  Number of Delta(AT) expressions before leap seconds were introduced */
/*  Dates (year, month) on which new Delta(AT) came into force */
/*  New Delta(AT) which came into force on the given dates */
/*  Reference dates (MJD) and drift rates (s/day), pre leap seconds */
/*  Miscellaneous local variables */
/*  Dates and Delta(AT)s */
/*  Reference dates and drift rates */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*  Initialize the result to zero and the status to OK. */
    da = 0.;
    js = 0;
/*  If invalid fraction of a day, set error status and give up. */
    if (*fd < 0. || *fd >= 1.) {
	js = -4;
	goto L9000;
    }
/*  Convert the date into an MJD. */
    cal2jd_(iy, im, id, &djm0, &djm, &js);
/*  If invalid year, month, or day, give up. */
    if (js < 0) {
	goto L9000;
    }
/*  If pre-UTC year, set warning status and give up. */
    if (*iy < idate[0]) {
	js = 1;
	goto L9000;
    }
/*  If suspiciously late year, set warning status but proceed. */
    if (*iy > 2014) {
	js = 1;
    }
/*  Combine year and month. */
    m = *iy * 12 + *im;
/*  Prepare to search the tables. */
    more = TRUE_;
/*  Find the most recent table entry. */
    for (i__ = 39; i__ >= 1; --i__) {
	if (more) {
	    is = i__;
	    more = m < idate[(i__ << 1) - 2] * 12 + idate[(i__ << 1) - 1];
	}
/* L1: */
    }
/*  Get the Delta(AT). */
    da = dats[is - 1];
/*  If pre-1972, adjust for drift. */
    if (is <= 14) {
	da += (djm + *fd - drift[(is << 1) - 2]) * drift[(is << 1) - 1];
    }
/*  Return the Delta(AT) value and the status. */
L9000:
    *deltat = da;
    *j = js;
/*  Finished. */
/* +----------------------------------------------------------------------- */

/*  Copyright (C) 2008 */
/*  Standards Of Fundamental Astronomy Review Board */
/*  of the International Astronomical Union. */

/*  ===================== */
/*  SOFA Software License */
/*  ===================== */

/*  NOTICE TO USER: */

/*  BY USING THIS SOFTWARE YOU ACCEPT THE FOLLOWING TERMS AND CONDITIONS */
/*  WHICH APPLY TO ITS USE. */

/*  1. The Software is owned by the IAU SOFA Review Board ("the Board"). */

/*  2. Permission is granted to anyone to use the SOFA software for any */
/*     purpose, including commercial applications, free of charge and */
/*     without payment of royalties, subject to the conditions and */
/*     restrictions listed below. */

/*  3. You (the user) may copy and adapt the SOFA software and its */
/*     algorithms for your own purposes and you may copy and distribute */
/*     a resulting "derived work" to others on a world-wide, royalty-free */
/*     basis, provided that the derived work complies with the following */
/*     requirements: */

/*     a) Your work shall be marked or carry a statement that it (i) uses */
/*        routines and computations derived by you from software provided */
/*        by SOFA under license to you; and (ii) does not contain */
/*        software provided by SOFA or software that has been distributed */
/*        by or endorsed by SOFA. */

/*     b) The source code of your derived work must contain descriptions */
/*        of how the derived work is based upon and/or differs from the */
/*        original SOFA software. */

/*     c) The name(s) of all routine(s) that you distribute shall differ */
/*        from the SOFA names, even when the SOFA content has not been */
/*        otherwise changed. */

/*     d) The routine-naming prefix "iau" shall not be used. */

/*     e) The origin of the SOFA components of your derived work must not */
/*        be misrepresented;  you must not claim that you wrote the */
/*        original software, nor file a patent application for SOFA */
/*        software or algorithms embedded in the SOFA software. */

/*     f) These requirements must be reproduced intact in any source */
/*        distribution and shall apply to anyone to whom you have granted */
/*        a further right to modify the source code of your derived work. */

/*  4. In any published work or commercial products which includes */
/*     results achieved by using the SOFA software, you shall acknowledge */
/*     that the SOFA software was used in obtaining those results. */

/*  5. You shall not cause the SOFA software to be brought into */
/*     disrepute, either by misuse, or use for inappropriate tasks, or by */
/*     inappropriate modification. */

/*  6. The SOFA software is provided "as is" and the Board makes no */
/*     warranty as to its use or performance.   The Board does not and */
/*     cannot warrant the performance or results which the user may obtain */
/*     by using the SOFA software.  The Board makes no warranties, express */
/*     or implied, as to non-infringement of third party rights, */
/*     merchantability, or fitness for any particular purpose.  In no */
/*     event will the Board be liable to the user for any consequential, */
/*     incidental, or special damages, including any lost profits or lost */
/*     savings, even if a Board representative has been advised of such */
/*     damages, or for any claim by any third party. */

/*  7. The provision of any version of the SOFA software under the terms */
/*     and conditions specified herein does not imply that future */
/*     versions will also be made available under the same terms and */
/*     conditions. */

/*  Correspondence concerning SOFA software should be addressed as */
/*  follows: */

/*     Internet email: sofa@rl.ac.uk */
/*     Postal address: IAU SOFA Center */
/*                     Rutherford Appleton Laboratory */
/*                     Chilton, Didcot, Oxon OX11 0QX */
/*                     United Kingdom */

/* ----------------------------------------------------------------------- */
    return 0;
} /* dat_ */

