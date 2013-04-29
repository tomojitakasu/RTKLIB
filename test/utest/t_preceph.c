/*------------------------------------------------------------------------------
* rtklib unit test driver : precise ephemeris function
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include "../../src/rtklib.h"

static void dumpeph(peph_t *peph, int n)
{
    char s[64];
    int i,j;
    for (i=0;i<n;i++) {
        time2str(peph[i].time,s,3);
        printf("time=%s\n",s);
        for (j=0;j<MAXSAT;j++) {
            printf("%03d: %14.3f %14.3f %14.3f : %5.3f %5.3f %5.3f\n",
                   j+1,peph[i].pos[j][0],peph[i].pos[j][1],peph[i].pos[j][2],
                   peph[i].std[j][0],peph[i].std[j][1],peph[i].std[j][2]);
        }
    }
}
static void dumpclk(pclk_t *pclk, int n)
{
    char s[64];
    int i,j;
    for (i=0;i<n;i++) {
        time2str(pclk[i].time,s,3);
        printf("time=%s\n",s);
        for (j=0;j<MAXSAT;j++) {
            printf("%03d: %14.3f : %5.3f\n",
                   j+1,pclk[i].clk[j][0]*1E9,pclk[i].std[j][0]*1E9);
        }
    }
}
/* readsp3() */
void utest1(void)
{
    char *file1="../data/sp3/igs15904.sp4";
    char *file2="../data/sp3/igs15904.sp3";
    char *file3="../data/sp3/igs1590*.sp3";
    nav_t nav={0};
    
    printf("file=%s\n",file1);
    readsp3(file1,&nav,0);
        assert(nav.ne==0);
    
    printf("file=%s\n",file2);
    readsp3(file2,&nav,0);
        assert(nav.ne==96);
    dumpeph(nav.peph,nav.ne);
    
    printf("file=%s\n",file3);
    readsp3(file3,&nav,0);
        assert(nav.ne==192);
    dumpeph(nav.peph,nav.ne);
    
    printf("%s utest1 : OK\n",__FILE__);
}
/* readsap() */
void utest2(void)
{
    double ep1[]={2008,3,1,0,0,0};
    double ep2[]={2006,11,4,23,59,59};
    char *file1="../data/sp3/igs06.atx";
    char *file2="../../data/igs05.atx";
    pcvs_t pcvs={0};
    pcv_t *pcv;
    gtime_t time;
    int i,stat;
    
    printf("file=%s\n",file1);
    stat=readpcv(file1,&pcvs);
        assert(!stat);
    stat=readpcv(file2,&pcvs);
        assert(stat);
    
    time=epoch2time(ep1);
    for (i=0;i<MAXSAT;i++) {
        if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
        printf("PRN%02d : %7.4f %7.4f %7.4f\n",i+1,pcv->off[0][0],pcv->off[0][1],pcv->off[0][2]);
    }
    time=epoch2time(ep2);
    for (i=0;i<MAXSAT;i++) {
        if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
        printf("PRN%02d : %7.4f %7.4f %7.4f\n",i+1,pcv->off[0][0],pcv->off[0][1],pcv->off[0][2]);
    }
    
    printf("%s utest2 : OK\n",__FILE__);
}
/* readrnxc() */
void utest3(void)
{
    char *file1="../data/sp3/igs15904.cls";
    char *file2="../data/sp3/igs15904.clk";
    char *file3="../data/sp3/igs1590*.clk";
    nav_t nav={0};
    
    printf("file=%s\n",file1);
    readrnxc(file1,&nav);
        assert(nav.nc==0);
    
    printf("file=%s\n",file2);
    readrnxc(file2,&nav);
        assert(nav.nc>0);
    dumpclk(nav.pclk,nav.nc);
    free(nav.pclk); nav.pclk=NULL; nav.nc=nav.ncmax=0;
    
    printf("file=%s\n",file3);
    readrnxc(file3,&nav);
        assert(nav.nc>0);
    dumpclk(nav.pclk,nav.nc);
    free(nav.pclk); nav.pclk=NULL; nav.nc=nav.ncmax=0;
    
    printf("%s utest3 : OK\n",__FILE__);
}
/* peph2pos() */
void utest4(void)
{
    FILE *fp;
    char *file1="../data/sp3/igs1590*.sp3"; /* 2010/7/1 */
    char *file2="../data/sp3/igs1590*.clk"; /* 2010/7/1 */
    nav_t nav={0};
    int i,j,stat,sat;
    double ep[]={2010,7,1,0,0,0};
    double rs[6]={0},dts[2]={0};
    double var;
    gtime_t t,time;
    
    time=epoch2time(ep);
    
    readsp3(file1,&nav,0);
        assert(nav.ne>0);
    readrnxc(file2,&nav);
        assert(nav.nc>0);
    stat=peph2pos(time,0,&nav,0,rs,dts,&var);
        assert(!stat);
    stat=peph2pos(time,160,&nav,0,rs,dts,&var);
        assert(!stat);
    
    fp=fopen("testpeph1.out","w");
    
    sat=4;
    
    for (i=0;i<86400*2;i+=30) {
        t=timeadd(time,(double)i);
        for (j=0;j<6;j++) rs [j]=0.0;
        for (j=0;j<2;j++) dts[j]=0.0;
        peph2pos(t,sat,&nav,0,rs,dts,&var);
        fprintf(fp,"%02d %6d %14.3f %14.3f %14.3f %14.3f %10.3f %10.3f %10.3f %10.3f\n",
                sat,i,rs[0],rs[1],rs[2],dts[0]*1E9,rs[3],rs[4],rs[5],dts[1]*1E9);
    }
    fclose(fp);
    printf("%s utest4 : OK\n",__FILE__);
}
/* satpos() */
void utest5(void)
{
    FILE *fp;
    char *file1="../data/sp3/igs1590*.sp3"; /* 2010/7/1 */
    char *file2="../data/sp3/igs1590*.clk"; /* 2010/7/1 */
    char *file3="../../data/igs05.atx";
    char *file4="../data/rinex/brdc*.10n";
    pcvs_t pcvs={0};
    pcv_t *pcv;
    nav_t nav={0};
    int i,stat,sat,svh;
    double ep[]={2010,7,1,0,0,0};
    double rs1[6]={0},dts1[2]={0},rs2[6]={0},dts2[2]={0};
    double var;
    gtime_t t,time;
    
    time=epoch2time(ep);
    
    readsp3(file1,&nav,0);
        assert(nav.ne>0);
    readrnxc(file2,&nav);
        assert(nav.nc>0);
    stat=readpcv(file3,&pcvs);
        assert(stat);
    readrnx(file4,1,"",NULL,&nav,NULL);
        assert(nav.n>0);
    for (i=0;i<MAXSAT;i++) {
        if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
        nav.pcvs[i]=*pcv;
    }
    fp=fopen("testpeph2.out","w");
    
    sat=3;
    
    for (i=0;i<86400*2;i+=30) {
        t=timeadd(time,(double)i);
        satpos(t,t,sat,EPHOPT_BRDC,&nav,rs1,dts1,&var,&svh);
        satpos(t,t,sat,EPHOPT_PREC,&nav,rs2,dts2,&var,&svh);
        fprintf(fp,"%02d %6d %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f %14.3f\n",
                sat,i,
                rs1[0],rs1[1],rs1[2],dts1[0]*1E9,rs2[0],rs2[1],rs2[2],dts2[0]*1E9);
    }
    fclose(fp);
    printf("%s utest4 : OK\n",__FILE__);
}
int main(int argc, char **argv)
{
    utest1();
    utest2();
    utest3();
    utest4();
    utest5();
    return 0;
}
