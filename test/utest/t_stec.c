/*------------------------------------------------------------------------------
* rtklib unit test driver : stec function
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include "../../src/rtklib.h"

/* dump stec data ------------------------------------------------------------*/
void dump_stec(const nav_t *nav)
{
    int i;
    
    printf("nav.nn=%d\n",nav->nn);
    
    for (i=0;i<nav->nn;i++) {
        printf("(%6d) %s %2d %8.3f %8.3f\n",i+1,time_str(nav->stec->data[i].time,0),
               nav->stec->data[i].sat,nav->stec->data[i].iono,nav->stec->data[i].rms);
    }
}
/* stec_read() ---------------------------------------------------------------*/
void utest1(void)
{
    char *file1="../../util/geniono/iono.txt";
    char *file2="../../util/geniono/iono.stec";
    nav_t nav={0};
    
    printf("file=%s\n",file1);
    stec_read(file1,&nav);
        assert(nav.nn==0);
    
    printf("file=%s\n",file2);
    stec_read(file2,&nav);
        assert(nav.nn>0);
    
#if 0
    dump_stec(&nav);
#endif
    
    printf("%s utest1 : OK\n",__FILE__);
}
/* stec_ion() ----------------------------------------------------------------*/
void utest2(void)
{
    char *file1="../../util/geniono/iono.stec";
    nav_t nav={0};
    gtime_t time1,time2,time3,time4,time5;
    double ep1  []={2012, 1,29, 0, 0, 0}; /* error */
    double ep2  []={2012, 1,29, 0,24,29};
    double ep3  []={2012, 1,29, 3,24,45};
    double ep4  []={2012, 1,29,15, 0, 0};
    double ep5  []={2012, 1,29,23,59,59};
    double pos1 []={ 35.6*D2R, 139.6*D2R,0.0};
    double azel1[]={0.0,0.0};
    double delay,rate,var;
    int sat1=32;
    int stat,brk;
    
    time1=epoch2time(ep1);
    time2=epoch2time(ep2);
    time3=epoch2time(ep3);
    time4=epoch2time(ep4);
    time5=epoch2time(ep5);
    
    stec_read(file1,&nav);
    
    stat=stec_ion(time1,&nav,sat1,pos1,azel1,&delay,&rate,&var,&brk);
        assert(stat==1);
    stat=stec_ion(time2,&nav,sat1,pos1,azel1,&delay,&rate,&var,&brk);
        assert(stat==1);
    stat=stec_ion(time3,&nav,sat1,pos1,azel1,&delay,&rate,&var,&brk);
        assert(stat==1);
    stat=stec_ion(time4,&nav,sat1,pos1,azel1,&delay,&rate,&var,&brk);
        assert(stat==1);
    stat=stec_ion(time5,&nav,sat1,pos1,azel1,&delay,&rate,&var,&brk);
        assert(stat==1);
    
    printf("%s utest2 : OK\n",__FILE__);
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    utest1();
    utest2();
    return 0;
}
