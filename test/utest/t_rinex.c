/*------------------------------------------------------------------------------
* rtklib unit test driver : rinex function
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include "../../src/rtklib.h"

static void dumpobs(obs_t *obs)
{
    gtime_t time={0};
    int i;
    char str[64];
    printf("obs : n=%d\n",obs->n);
    for (i=0;i<obs->n;i++) {
        time2str(obs->data[i].time,str,3);
        printf("%s : %2d %2d %13.3f %13.3f %13.3f %13.3f  %d %d\n",str,obs->data[i].sat,
               obs->data[i].rcv,obs->data[i].L[0],obs->data[i].L[1],
               obs->data[i].P[0],obs->data[i].P[1],obs->data[i].LLI[0],obs->data[i].LLI[1]);
        
        assert(1<=obs->data[i].sat&&obs->data[i].sat<=32);
        assert(timediff(obs->data[i].time,time)>=-DTTOL);
        
        time=obs->data[i].time;
    }
}
static void dumpnav(nav_t *nav)
{
    int i;
    char str[64],s1[64],s2[64];
    printf("nav : n=%d\n",nav->n);
    for (i=0;i<nav->n;i++) {
        time2str(nav->eph[i].toe,str,3);
        time2str(nav->eph[i].toc,s1,0);
        time2str(nav->eph[i].ttr,s2,0);
        printf("%s : %2d    %s %s %3d %3d %2d\n",str,nav->eph[i].sat,s1,s2,
               nav->eph[i].iode,nav->eph[i].iodc,nav->eph[i].svh);
        
        assert(nav->eph[i].iode==(nav->eph[i].iodc&0xFF));
    }
}
static void dumpsta(sta_t *sta)
{
    printf("name    = %s\n",sta->name);
    printf("marker  = %s\n",sta->marker);
    printf("antdes  = %s\n",sta->antdes);
    printf("antsno  = %s\n",sta->antsno);
    printf("rectype = %s\n",sta->rectype);
    printf("recver  = %s\n",sta->recver);
    printf("recsno  = %s\n",sta->recsno);
    printf("antsetup= %d\n",sta->antsetup);
    printf("itrf    = %d\n",sta->itrf);
    printf("deltype = %d\n",sta->deltype);
    printf("pos     = %.3f %.3f %.3f\n",sta->pos[0],sta->pos[1],sta->pos[2]);
    printf("del     = %.3f %.3f %.3f\n",sta->del[0],sta->del[1],sta->del[2]);
    printf("hgt     = %.3f\n",sta->hgt);
}
/* readrnx(), sortobs(), uniqnav()  */
void utest1(void)
{
    char file1[]="abc.00o";
    char file2[]="bcd.00n";
    char file3[]="../data/rinex/07590920.05o";
    char file4[]="../data/rinex/07590920.05n";
    char file5[]="../data/rinex/30400920.05o";
    char file6[]="../data/rinex/30400920.05n";
    obs_t obs={0};
    nav_t nav={0};
    sta_t sta={""};
    int n,stat;
    
    stat=readrnx(file1,1,"",&obs,&nav,&sta);
        assert(stat==0&&obs.n==0&&nav.n==0&&nav.ng==0&&nav.ns==0);
    stat=readrnx(file2,1,"",&obs,&nav,&sta);
        assert(stat==0&&obs.n==0&&nav.n==0&&nav.ng==0&&nav.ns==0);
    stat=readrnx(file3,1,"",&obs,&nav,&sta);
        assert(stat==1);
    stat=readrnx(file4,1,"",&obs,&nav,&sta);
        assert(stat==1);
    stat=readrnx(file5,2,"",&obs,&nav,&sta);
        assert(stat==1);
    stat=readrnx(file6,2,"",&obs,&nav,&sta);
        assert(stat==1);
    n=sortobs(&obs);
        assert(n==171);
    uniqnav(&nav);
        assert(nav.n==167);
    dumpobs(&obs); dumpnav(&nav); dumpsta(&sta);
        assert(obs.data&&obs.n>0&&nav.eph&&nav.n>0);
    free(obs.data);
    free(nav.eph);
    free(nav.geph);
    free(nav.seph);
    
    printf("%s utest1 : OK\n",__FILE__);
}
/* readrnxt() */
void utest2(void)
{
    gtime_t t0={0},ts,te;
    double ep1[]={2005,4,2,1,0,0},ep2[]={2005,4,2,2,0,0};
    char file1[]="../data/rinex/07590920.05o";
    char file2[]="../data/rinex/07590920.05n";
    int n;
    obs_t obs={0};
    nav_t nav={0};
    sta_t sta={""};
    
    ts=epoch2time(ep1);
    te=epoch2time(ep2);
    n=readrnxt(file1,1,ts,te,0.0,"",&obs,&nav,&sta);
    printf("\n\nn=%d\n",n);
    n=readrnxt(file2,1,ts,te,0.0,"",&obs,&nav,&sta);
    dumpobs(&obs);
    free(obs.data); obs.data=NULL; obs.n=obs.nmax=0;
    n=readrnxt(file1,1,t0,t0,240.0,"",&obs,&nav,&sta);
    printf("\n\nn=%d\n",n);
    dumpobs(&obs);
    free(obs.data);
    
    printf("%s utset2 : OK\n",__FILE__);
}
static rnxopt_t opt1={{0}};
static rnxopt_t opt2= {
    {0},{0},0.0,0.0,2.10,SYS_ALL,OBSTYPE_ALL,FREQTYPE_ALL,{{0}},
    "STAID",
    "RROG567890123456789012345678901",
    "RUNBY67890123456789012345678901",
    "MARKER789012345678901234567890123456789012345678901234567890123",
    "MARKNO7890123456789012345678901",
    "MARKTY7890123456789012345678901",
    {"OBSERVER90123456789012345678901",
     "AGENCY7890123456789012345678901"},
    {"RCV1567890123456789012345678901",
     "RCV2567890123456789012345678901",
     "RCV3567890123456789012345678901"},
    {"ANT1567890123456789012345678901",
     "ANT2567890123456789012345678901",
     "ANT3567890123456789012345678901"},
    {12345678.123,99999999.999,100000000.000},
    {123.0345,890123.9012,34567.0001},
    {"COMMENT1 012345678901234567890123456789012345678901234567890123",
     "COMMENT2 012345678901234567890123456789012345678901234567890123",
     "COMMENT3 012345678901234567890123456789012345678901234567890123",
     "COMMENT4 012345678901234567890123456789012345678901234567890123",
     "COMMENT5 012345678901234567890123456789012345678901234567890123"},
     "",{0},1,1,1,1,1,
    {0},{0},{0}
};
/* outrneobsh() */
void utest3(void)
{
    nav_t nav={0};
    
    outrnxobsh(stdout,&opt1,&nav);
    outrnxobsh(stdout,&opt2,&nav);
    
    printf("%s utest3 : OK\n",__FILE__);
}
/* outrneobsb() */
void utest4(void)
{
    char file[]="../data/rinex/07590920.05o";
    obs_t obs={0};
    int i,j;
    
    readrnx(file,1,"",&obs,NULL,NULL);
    outrnxobsb(stdout,&opt2,obs.data,8,9);
    outrnxobsb(stdout,&opt2,obs.data,8,0);
    
    for (i=j=0;i<obs.n;i=j) {
        while (j<obs.n&&timediff(obs.data[j].time,obs.data[i].time)<=0.0) j++;
        outrnxobsb(stdout,&opt2,obs.data+i,j-i,0);
    }
    printf("%s utest4 : OK\n",__FILE__);
}
/* outrnxnavh() */
void utest5(void)
{
    char file1[]="../data/rinex/07590920.05n";
    double ion[]={1E9,2E-4,3E8,4E3,-4E-3,-5E99,-6E-33,-9E-123};
    double utc[]={1E9,2E4,3E2,-9999};
    nav_t nav={0};
    int i;
    for (i=0;i<8;i++) nav.ion_gps[i]=ion[i];
    for (i=0;i<4;i++) nav.utc_gps[i]=utc[i];
    nav.leaps=14;

    readrnx(file1,1,"",NULL,&nav,NULL);

    outrnxnavh(stdout,&opt1,&nav);
    outrnxnavh(stdout,&opt2,&nav);
    
    printf("%s utest5 : OK\n",__FILE__);
}
/* outrnxnavb() */
void utest6(void)
{
    char file[]="../data/rinex/07590920.05n";
    nav_t nav={0};
    int i;
    readrnx(file,1,"",NULL,&nav,NULL);
    for (i=0;i<nav.n;i++) {
        outrnxnavb(stdout,&opt2,nav.eph+i);
    }
    printf("%s utest6 : OK\n",__FILE__);
}
int main(int argc, char **argv)
{
    utest1();
    utest2();
    utest3();
    utest4();
    utest5();
    utest6();
    return 0;
}
