/*------------------------------------------------------------------------------
* rtklib unit test driver : glonass ephemeris function
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include "../../src/rtklib.h"

static void dumpgeph(geph_t *geph, int n)
{
    char s1[64],s2[64];
    int i;
    for (i=0;i<n;i++) {
        time2str(geph[i].toe,s1,0);
        time2str(geph[i].tof,s2,0);
        printf("(%3d) sat=%2d frq=%2d svh=%d age=%d toe=%s %s "
               "pos=%13.3f %13.3f %13.3f vel=%9.3f %9.3f %9.3f "
               "acc=%10.3E %10.3E %10.3E taun=%12.5E gamn=%12.5E\n",
            i+1,geph[i].sat,geph[i].frq,geph[i].svh,geph[i].age,s1,s2,
            geph[i].pos[0],geph[i].pos[1],geph[i].pos[2],
            geph[i].vel[0],geph[i].vel[1],geph[i].vel[2],
            geph[i].acc[0],geph[i].acc[1],geph[i].acc[2],
            geph[i].taun,geph[i].gamn);
    }
}
/* readrnx() */
void utest1(void)
{
    char file1[]="../data/rinex/brdd0910.09g";
    char file2[]="../data/rinex/brdc0910.09g";
    nav_t nav={0};
    
    readrnx(file1,1,"",NULL,&nav,NULL);
        assert(nav.ng==0);
    readrnx(file2,1,"",NULL,&nav,NULL);
        assert(nav.ng>0);
    dumpgeph(nav.geph,nav.ng);
    
    printf("%s utest1 : OK\n",__FILE__);
}
/* readsp3() */
void utest2(void)
{
    char *file1="../data/sp3/igl15253.sp4";
    char *file2="../data/sp3/igl15253.sp3";
    nav_t nav={0};
    double tow,*pos;
    int i,week,sat;
    
    sat=satno(SYS_GLO,13);
    
    readsp3(file1,&nav,0);
        assert(nav.ne<=0);
    readsp3(file2,&nav,0);
        assert(nav.ne>0);
    
    for (i=0;i<nav.ne;i++) {
        tow=time2gpst(nav.peph[i].time,&week);
        pos=nav.peph[i].pos[sat-1];
        printf("%4d %6.0f %2d %13.3f %13.3f %13.3f %10.3f\n",
               week,tow,sat,pos[0],pos[1],pos[2],pos[3]*1E9);
        assert(norm(pos,4)>0.0);
    }
    printf("\n");
    
    printf("%s utest2 : OK\n",__FILE__);
}
/* broadcast ephemeris */
void utest3(void)
{
    gtime_t time;
    char file[]="../data/rinex/brdc0910.09g";
    nav_t nav={0};
    double ep[]={2009,4,1,0,0,0};
    double tspan=86400.0,tint=30.0,tow;
    double rs[6],dts[2];
    double var;
    int i,sat,week,svh;
    
    sat=satno(SYS_GLO,7);
    
    readrnx(file,1,"",NULL,&nav,NULL);
    
    for (i=0;i<tspan/tint;i++) {
        time=timeadd(epoch2time(ep),tint*i);
        satpos(time,time,sat,EPHOPT_BRDC,&nav,rs,dts,&var,&svh);
        tow=time2gpst(time,&week);
        printf("%4d %6.0f %2d %13.3f %13.3f %13.3f %10.3f\n",
               week,tow,sat,rs[0],rs[1],rs[2],dts[0]*1E9);
        assert(norm(rs,3)>0.0);
        assert(norm(rs+3,3)>0.0);
        assert(dts[0]!=0.0);
        assert(dts[1]!=0.0);
    }
    printf("\n");
    
    printf("%s utest3 : OK\n",__FILE__);
}
/* precise ephemeris */
void utest4(void)
{
    gtime_t time;
    char *file="../data/sp3/igl15253.sp3";
    nav_t nav={0};
    double ep[]={2009,4,1,0,0,0};
    double tspan=86400.0,tint=30.0,tow;
    double rs[6],dts[2];
    double var;
    int i,sat,week,svh;
    
    sat=satno(SYS_GLO,7);
    
    readsp3(file,&nav,0);
    
    for (i=0;i<tspan/tint;i++) {
        time=timeadd(epoch2time(ep),tint*i);
        satpos(time,time,sat,EPHOPT_PREC,&nav,rs,dts,&var,&svh);
        tow=time2gpst(time,&week);
        printf("%4d %6.0f %2d %13.3f %13.3f %13.3f %10.3f\n",
               week,tow,sat,rs[0],rs[1],rs[2],dts[0]*1E9);
        assert(norm(rs,3)>0.0);
        assert(norm(rs+3,3)>0.0);
        assert(dts[0]!=0.0);
    }
    printf("\n");
    
    printf("%s utest4 : OK\n",__FILE__);
}
/* readsap() */
void utest5(void)
{
    char *file="../../data/igs05.atx",id[32];
    double ep[]={2009,4,1,0,0,0};
    gtime_t time=epoch2time(ep);
    nav_t nav={0};
    int i,stat;
    
    stat=readsap(file,time,&nav);
    
    for (i=0;i<MAXSAT;i++) {
        satno2id(i+1,id);
        printf("%2d %-4s %8.3f %8.3f %8.3f\n",i+1,id,
               nav.pcvs[i].off[0][0],nav.pcvs[i].off[0][1],nav.pcvs[i].off[0][2]);
    }
    printf("\n");
    
    printf("%s utest5 : OK\n",__FILE__);
}
/* satpos() */
void utest6(void)
{
    FILE *fp;
    char *file1="../data/rinex/brdc0910.09g";
    char *file2="../data/sp3/igl15253.sp3";
    char *file3="../../data/igs05.atx";
/*
    char *file4="../data/esa15253.sp3";
    char *file5="../data/esa15253.clk";
*/
    char *outfile="testgloeph.out";
    double ep[]={2009,4,1,0,0,0};
    double tspan=86400.0,tint=30.0,tow;
    double rs1[6],dts1[2],rs2[6],dts2[2],dr[3],ddts;
    double var1,var2;
    gtime_t time=epoch2time(ep);
    nav_t nav={0};
    int i,j,sat,week,svh1,svh2;
    
    readrnx(file1,1,"",NULL,&nav,NULL);
    readsp3(file2,&nav,0);
/*
    readsp3(file4,&nav,0);
    readrnxc(file5,&nav);
*/
    readsap(file3,time,&nav);
    
/*
    sat=satno(SYS_GLO,21);
*/
    sat=satno(SYS_GLO,22);
    
    fp=fopen(outfile,"w");
    
    for (i=0;i<tspan/tint;i++) {
        time=timeadd(epoch2time(ep),tint*i);
        tow=time2gpst(time,&week);
        satpos(time,time,sat,EPHOPT_BRDC,&nav,rs1,dts1,&var1,&svh1);
        satpos(time,time,sat,EPHOPT_PREC,&nav,rs2,dts2,&var2,&svh2);
        
        if (norm(rs1,3)<=0.0||norm(rs2,3)<=0.0) continue;
        
        for (j=0;j<3;j++) dr[j]=rs1[j]-rs2[j];
        ddts=dts1[0]-dts2[0];
        fprintf(fp,"%4d %6.0f %2d %8.3f %8.3f %8.3f %10.3f\n",week,tow,sat,
                dr[0],dr[1],dr[2],ddts*1E9);
        
        assert(norm(dr,3)<10.0);
    }
    fclose(fp);
    
    printf("output to: %s\n",outfile);
    
    printf("%s utest6 : OK\n",__FILE__);
}
/* unit test main */
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
