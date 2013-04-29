      SUBROUTINE DAT ( IY, IM, ID, FD, DELTAT, J )
*+
*  - - - - - - - -
*   D A T
*  - - - - - - - -
*
*  For a given UTC date, calculate delta(AT) = TAI-UTC.
*
*     :------------------------------------------:
*     :                                          :
*     :                 IMPORTANT                :
*     :                                          :
*     :  A new version of this routine must be   :
*     :  produced whenever a new leap second is  :
*     :  announced.  There are five items to     :
*     :  change on each such occasion:           :
*     :                                          :
*     :  1) The parameter NDAT must be           :
*     :     increased by 1.                      :
*     :                                          :
*     :  2) A new line must be added to the set  :
*     :     of DATA statements that initialize   :
*     :     the arrays IDATE and DATS.           :
*     :                                          :
*     :  3) The parameter IYV must be set to     :
*     :     the current year.                    :
*     :                                          :
*     :  4) The "Latest leap second" comment     :
*     :     below must be set to the new leap    :
*     :     second date.                         :
*     :                                          :
*     :  5) The "This revision" comment, later,  :
*     :     must be set to the current date.     :
*     :                                          :
*     :  Change (3) must also be carried out     :
*     :  whenever the routine is re-issued,      :
*     :  even if no leap seconds have been       :
*     :  added.                                  :
*     :                                          :
*     :  Latest leap second:  2008 December 31   :
*     :                                          :
*     :__________________________________________:
*
*  This routine is part of the International Astronomical Union's
*  SOFA (Standards of Fundamental Astronomy) software collection.
*
*  Status:  support routine.
*
*  Given:
*     IY       i     UTC:  year (Notes 1 and 2)
*     IM       i           month (Note 2)
*     ID       i           day (Notes 2 and 3)
*     FD       d           fraction of day (Note 4)
*
*  Returned:
*     DELTAT   d     TAI minus UTC, seconds
*     J        i     status (Note 5):
*                       1 = dubious year (Note 1)
*                       0 = OK
*                      -1 = bad year
*                      -2 = bad month
*                      -3 = bad day (Note 3)
*                      -4 = bad fraction (Note 4)
*
*  Notes:
*
*  1) UTC began at 1960 January 1.0 (JD 2436934.5) and it is improper
*     to call the routine with an earlier date.  If this is attempted,
*     zero is returned together with a warning status.
*
*     Because leap seconds cannot, in principle, be predicted in
*     advance, a reliable check for dates beyond the valid range is
*     impossible.  To guard against gross errors, a year five or more
*     after the release year of the present routine (see parameter IYV)
*     is considered dubious.  In this case a warning status is returned
*     but the result is computed in the normal way.
*
*     For both too-early and too-late years, the warning status is J=+1.
*     This is distinct from the error status J=-1, which signifies a
*     year so early that JD could not be computed.
*
*  2) If the specified date is for a day which ends with a leap second,
*     the UTC-TAI value returned is for the period leading up to the
*     leap second.  If the date is for a day which begins as a leap
*     second ends, the UTC-TAI returned is for the period following the
*     leap second.
*
*  3) The day number must be in the normal calendar range, for example
*     1 through 30 for April.  The "almanac" convention of allowing
*     such dates as January 0 and December 32 is not supported in this
*     routine, in order to avoid confusion near leap seconds.
*
*  4) The fraction of day is used only for dates before the introduction
*     of leap seconds, the first of which occurred at the end of 1971.
*     It is tested for validity (zero to less than 1 is the valid range)
*     even if not used;  if invalid, zero is used and status J=-4 is
*     returned.  For many applications, setting FD to zero is
*     acceptable;  the resulting error is always less than 3 ms (and
*     occurs only pre-1972).
*
*  5) The status value returned in the case where there are multiple
*     errors refers to the first error detected.  For example, if the
*     month and day are 13 and 32 respectively, J=-2 (bad month) will be
*     returned.
*
*  6) In cases where a valid result is not available, zero is returned.
*
*  References:
*
*  1) For dates from 1961 January 1 onwards, the expressions from the
*     file ftp://maia.usno.navy.mil/ser7/tai-utc.dat are used.
*
*  2) The 5ms timestep at 1961 January 1 is taken from 2.58.1 (p87) of
*     the 1992 Explanatory Supplement.
*
*  Called:
*     CAL2JD       Gregorian calendar to Julian Day number
*
*  This revision:  2008 July 5
*
*  Copyright (C) 2008 IAU SOFA Review Board.  See notes at end.
*
*-----------------------------------------------------------------------

      IMPLICIT NONE

      INTEGER IY, IM, ID
      DOUBLE PRECISION FD, DELTAT
      INTEGER J

*  Release year for this version of DAT
      INTEGER IYV
      PARAMETER ( IYV = 2009 )

*  Number of Delta(AT) changes (increase by 1 for each new leap second)
      INTEGER NDAT
      PARAMETER ( NDAT = 39 )

*  Number of Delta(AT) expressions before leap seconds were introduced
      INTEGER NERA1
      PARAMETER ( NERA1 = 14 )

*  Dates (year, month) on which new Delta(AT) came into force
      INTEGER IDATE(2,NDAT)

*  New Delta(AT) which came into force on the given dates
      DOUBLE PRECISION DATS(NDAT)

*  Reference dates (MJD) and drift rates (s/day), pre leap seconds
      DOUBLE PRECISION DRIFT(2,NERA1)

*  Miscellaneous local variables
      LOGICAL MORE
      INTEGER JS, I, M, IS
      DOUBLE PRECISION DA, DJM0, DJM

*  Dates and Delta(AT)s
      DATA (IDATE(I, 1),I=1,2),DATS(1)  / 1960,  1,  1.4178180D0 /
      DATA (IDATE(I, 2),I=1,2),DATS(2)  / 1961,  1,  1.4228180D0 /
      DATA (IDATE(I, 3),I=1,2),DATS(3)  / 1961,  8,  1.3728180D0 /
      DATA (IDATE(I, 4),I=1,2),DATS(4)  / 1962,  1,  1.8458580D0 /
      DATA (IDATE(I, 5),I=1,2),DATS(5)  / 1963, 11,  1.9458580D0 /
      DATA (IDATE(I, 6),I=1,2),DATS(6)  / 1964,  1,  3.2401300D0 /
      DATA (IDATE(I, 7),I=1,2),DATS(7)  / 1964,  4,  3.3401300D0 /
      DATA (IDATE(I, 8),I=1,2),DATS(8)  / 1964,  9,  3.4401300D0 /
      DATA (IDATE(I, 9),I=1,2),DATS(9)  / 1965,  1,  3.5401300D0 /
      DATA (IDATE(I,10),I=1,2),DATS(10) / 1965,  3,  3.6401300D0 /
      DATA (IDATE(I,11),I=1,2),DATS(11) / 1965,  7,  3.7401300D0 /
      DATA (IDATE(I,12),I=1,2),DATS(12) / 1965,  9,  3.8401300D0 /
      DATA (IDATE(I,13),I=1,2),DATS(13) / 1966,  1,  4.3131700D0 /
      DATA (IDATE(I,14),I=1,2),DATS(14) / 1968,  2,  4.2131700D0 /
      DATA (IDATE(I,15),I=1,2),DATS(15) / 1972,  1, 10D0 /
      DATA (IDATE(I,16),I=1,2),DATS(16) / 1972,  7, 11D0 /
      DATA (IDATE(I,17),I=1,2),DATS(17) / 1973,  1, 12D0 /
      DATA (IDATE(I,18),I=1,2),DATS(18) / 1974,  1, 13D0 /
      DATA (IDATE(I,19),I=1,2),DATS(19) / 1975,  1, 14D0 /
      DATA (IDATE(I,20),I=1,2),DATS(20) / 1976,  1, 15D0 /
      DATA (IDATE(I,21),I=1,2),DATS(21) / 1977,  1, 16D0 /
      DATA (IDATE(I,22),I=1,2),DATS(22) / 1978,  1, 17D0 /
      DATA (IDATE(I,23),I=1,2),DATS(23) / 1979,  1, 18D0 /
      DATA (IDATE(I,24),I=1,2),DATS(24) / 1980,  1, 19D0 /
      DATA (IDATE(I,25),I=1,2),DATS(25) / 1981,  7, 20D0 /
      DATA (IDATE(I,26),I=1,2),DATS(26) / 1982,  7, 21D0 /
      DATA (IDATE(I,27),I=1,2),DATS(27) / 1983,  7, 22D0 /
      DATA (IDATE(I,28),I=1,2),DATS(28) / 1985,  7, 23D0 /
      DATA (IDATE(I,29),I=1,2),DATS(29) / 1988,  1, 24D0 /
      DATA (IDATE(I,30),I=1,2),DATS(30) / 1990,  1, 25D0 /
      DATA (IDATE(I,31),I=1,2),DATS(31) / 1991,  1, 26D0 /
      DATA (IDATE(I,32),I=1,2),DATS(32) / 1992,  7, 27D0 /
      DATA (IDATE(I,33),I=1,2),DATS(33) / 1993,  7, 28D0 /
      DATA (IDATE(I,34),I=1,2),DATS(34) / 1994,  7, 29D0 /
      DATA (IDATE(I,35),I=1,2),DATS(35) / 1996,  1, 30D0 /
      DATA (IDATE(I,36),I=1,2),DATS(36) / 1997,  7, 31D0 /
      DATA (IDATE(I,37),I=1,2),DATS(37) / 1999,  1, 32D0 /
      DATA (IDATE(I,38),I=1,2),DATS(38) / 2006,  1, 33D0 /
      DATA (IDATE(I,39),I=1,2),DATS(39) / 2009,  1, 34D0 /

*  Reference dates and drift rates
      DATA (DRIFT(I, 1),I=1,2) / 37300D0, 0.001296D0 /
      DATA (DRIFT(I, 2),I=1,2) / 37300D0, 0.001296D0 /
      DATA (DRIFT(I, 3),I=1,2) / 37300D0, 0.001296D0 /
      DATA (DRIFT(I, 4),I=1,2) / 37665D0, 0.0011232D0 /
      DATA (DRIFT(I, 5),I=1,2) / 37665D0, 0.0011232D0 /
      DATA (DRIFT(I, 6),I=1,2) / 38761D0, 0.001296D0 /
      DATA (DRIFT(I, 7),I=1,2) / 38761D0, 0.001296D0 /
      DATA (DRIFT(I, 8),I=1,2) / 38761D0, 0.001296D0 /
      DATA (DRIFT(I, 9),I=1,2) / 38761D0, 0.001296D0 /
      DATA (DRIFT(I,10),I=1,2) / 38761D0, 0.001296D0 /
      DATA (DRIFT(I,11),I=1,2) / 38761D0, 0.001296D0 /
      DATA (DRIFT(I,12),I=1,2) / 38761D0, 0.001296D0 /
      DATA (DRIFT(I,13),I=1,2) / 39126D0, 0.002592D0 /
      DATA (DRIFT(I,14),I=1,2) / 39126D0, 0.002592D0 /

* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

*  Initialize the result to zero and the status to OK.
      DA = 0D0
      JS = 0

*  If invalid fraction of a day, set error status and give up.
      IF ( FD.LT.0D0 .OR. FD.GE.1D0 ) THEN
         JS = -4
         GO TO 9000
      END IF

*  Convert the date into an MJD.
      CALL CAL2JD ( IY, IM, ID, DJM0, DJM, JS )

*  If invalid year, month, or day, give up.
      IF ( JS .LT. 0 ) GO TO 9000

*  If pre-UTC year, set warning status and give up.
      IF ( IY .LT. IDATE(1,1) ) THEN
         JS = 1
         GO TO 9000
      END IF

*  If suspiciously late year, set warning status but proceed.
      IF ( IY .GT. IYV+5 ) JS = 1

*  Combine year and month.
      M = 12*IY+IM

*  Prepare to search the tables.
      MORE = .TRUE.

*  Find the most recent table entry.
      DO 1 I=NDAT,1,-1
         IF ( MORE ) THEN
            IS = I
            MORE = M .LT. ( 12*IDATE(1,I) + IDATE(2,I) )
         END IF
 1    CONTINUE

*  Get the Delta(AT).
      DA = DATS(IS)

*  If pre-1972, adjust for drift.
      IF ( IS .LE. NERA1 ) DA = DA +
     :                          ( DJM + FD - DRIFT(1,IS) ) * DRIFT(2,IS)

*  Return the Delta(AT) value and the status.
 9000 CONTINUE
      DELTAT = DA
      J = JS

*  Finished.

*+-----------------------------------------------------------------------
*
*  Copyright (C) 2008
*  Standards Of Fundamental Astronomy Review Board
*  of the International Astronomical Union.
*
*  =====================
*  SOFA Software License
*  =====================
*
*  NOTICE TO USER:
*
*  BY USING THIS SOFTWARE YOU ACCEPT THE FOLLOWING TERMS AND CONDITIONS
*  WHICH APPLY TO ITS USE.
*
*  1. The Software is owned by the IAU SOFA Review Board ("the Board").
*
*  2. Permission is granted to anyone to use the SOFA software for any
*     purpose, including commercial applications, free of charge and
*     without payment of royalties, subject to the conditions and 
*     restrictions listed below.
*
*  3. You (the user) may copy and adapt the SOFA software and its 
*     algorithms for your own purposes and you may copy and distribute
*     a resulting "derived work" to others on a world-wide, royalty-free 
*     basis, provided that the derived work complies with the following
*     requirements: 
*
*     a) Your work shall be marked or carry a statement that it (i) uses
*        routines and computations derived by you from software provided 
*        by SOFA under license to you; and (ii) does not contain
*        software provided by SOFA or software that has been distributed
*        by or endorsed by SOFA.
*
*     b) The source code of your derived work must contain descriptions
*        of how the derived work is based upon and/or differs from the
*        original SOFA software.
*
*     c) The name(s) of all routine(s) that you distribute shall differ
*        from the SOFA names, even when the SOFA content has not been
*        otherwise changed.
*
*     d) The routine-naming prefix "iau" shall not be used.
*
*     e) The origin of the SOFA components of your derived work must not
*        be misrepresented;  you must not claim that you wrote the
*        original software, nor file a patent application for SOFA
*        software or algorithms embedded in the SOFA software.
*
*     f) These requirements must be reproduced intact in any source
*        distribution and shall apply to anyone to whom you have granted 
*        a further right to modify the source code of your derived work.
*
*  4. In any published work or commercial products which includes
*     results achieved by using the SOFA software, you shall acknowledge
*     that the SOFA software was used in obtaining those results.
*
*  5. You shall not cause the SOFA software to be brought into
*     disrepute, either by misuse, or use for inappropriate tasks, or by
*     inappropriate modification.
*
*  6. The SOFA software is provided "as is" and the Board makes no 
*     warranty as to its use or performance.   The Board does not and 
*     cannot warrant the performance or results which the user may obtain 
*     by using the SOFA software.  The Board makes no warranties, express 
*     or implied, as to non-infringement of third party rights,
*     merchantability, or fitness for any particular purpose.  In no
*     event will the Board be liable to the user for any consequential,
*     incidental, or special damages, including any lost profits or lost
*     savings, even if a Board representative has been advised of such
*     damages, or for any claim by any third party.
*
*  7. The provision of any version of the SOFA software under the terms 
*     and conditions specified herein does not imply that future
*     versions will also be made available under the same terms and
*     conditions.
*
*  Correspondence concerning SOFA software should be addressed as
*  follows:
*
*     Internet email: sofa@rl.ac.uk
*     Postal address: IAU SOFA Center
*                     Rutherford Appleton Laboratory
*                     Chilton, Didcot, Oxon OX11 0QX
*                     United Kingdom
*
*-----------------------------------------------------------------------

      END
