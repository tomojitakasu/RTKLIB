/*------------------------------------------------------------------------------
* ppp_ar.c : ppp ambiguity resolution
*
* reference :
*    [1] H.Okumura, C-gengo niyoru saishin algorithm jiten (in Japanese),
*        Software Technology, 1991
*
*          Copyright (C) 2012-2015 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2013/03/11  1.0  new
*           2016/05/10  1.1  delete codes
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

/* ambiguity resolution in ppp -----------------------------------------------*/
extern int ppp_ar(rtk_t *rtk, const obsd_t *obs, int n, int *exc,
                  const nav_t *nav, const double *azel, double *x, double *P)
{
    return 0;
}
