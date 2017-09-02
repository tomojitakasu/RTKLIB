/*------------------------------------------------------------------------------
* ppp_corr.c : ppp corrections functions
*
*          Copyright (C) 2015 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2015/05/20 1.0 new
*           2016/05/10 1.1 delete codes
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

/* read ppp corrections --------------------------------------------------------
* read ppp correction data from external file
* args   : pppcorr_t *corr  IO  ppp correction data
*          char   *file     I   file
* return : status (1:ok,0:error)
* notes  : file types are recognized by file extenstions as follows.
*            .stat,.STAT : solution status file by rtklib
*            .stec,.STEC : stec parameters file by mgest
*            others      : sinex troposphere file
*          read data are added to ppp correction data.
*          To clear data, call pppcorr_free()
*-----------------------------------------------------------------------------*/
extern int pppcorr_read(pppcorr_t *corr, const char *file)
{
    return 0;
}
/* free ppp corrections --------------------------------------------------------
* free and clear ppp correction data
* args   : pppcorr_t *corr  IO  ppp correction data
* return : none
*-----------------------------------------------------------------------------*/
extern void pppcorr_free(pppcorr_t *corr)
{
}
/* get tropospheric correction -------------------------------------------------
* get tropospheric correction from ppp correcion data
* args   : pppcorr_t *corr  I   ppp correction data
*          gtime_t time     I   time (GPST)
*          double *pos      I   receiver position {lat,lon,heght} (rad,m)
*          double *trp      O   tropos parameters {ztd,grade,gradn} (m)
*          double *std      O   standard deviation (m)
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int pppcorr_trop(const pppcorr_t *corr, gtime_t time, const double *pos,
                        double *trp, double *std)
{
    return 0;
}
/* get ionospherec correction --------------------------------------------------
* get ionospheric correction from ppp correction data
* args   : pppcorr_t *corr  I   ppp correction data
*          gtime_t time     I   time (GPST)
*          double *pos      I   receiver ecef position {x,y,z} (m)
*          double *ion      O   L1 slant ionos delay for each sat (MAXSAT x 1)
*                               (ion[i]==0: no correction data)
*          double *std      O   standard deviation (m)
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int pppcorr_stec(const pppcorr_t *corr, gtime_t time, const double *pos,
                        double *ion, double *std)
{
    return 0;
}
