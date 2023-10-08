/*------------------------------------------------------------------------------
* rtklib unit test driver : time and string functions
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../../src/rtklib.h"

#define TIME_64BIT 1

/* str2num() */
void utest1(void)
{
    double a;
    char s1[30]="123456789012345678901234567890";
    char s2[30]="....3D45......................";
    char s3[30]="...  3456.789 ................";
    a=str2num(s1,0,0);   assert(fabs(a-0.0     )<1E-15);
    a=str2num(s1,30,10); assert(fabs(a-0.0     )<1E-15);
    a=str2num(s1,10,0);  assert(fabs(a-0.0     )<1E-15);
    a=str2num(s1,-1,10); assert(fabs(a-0.0     )<1E-15);
    a=str2num(s1,0,3);   assert(fabs(a-123.0   )<1E-13);
    a=str2num(s1,10,6);  assert(fabs(a-123456.0)<1E-10);
    a=str2num(s1,28,10); assert(fabs(a-90.0    )<1E-14);
    a=str2num(s2,4,4);   assert(fabs(a-3E45    )<1E+30);
    a=str2num(s3,4,8);   assert(fabs(a-3456.78 )<1E-12);
    
    printf("%s utset1 : OK\n",__FILE__);
}
/* str2time() */
void utest2(void)
{
    char s1[30]="....2004 1 1 0 1 2.345........";
    char s2[30]="....  00 2 3 23 59 59.999.....";
    char s3[30]="....  80 10 30 6 58 9.........";
    char s4[30]="....  37 12 31 1 2 3 .........";
    int s;
    gtime_t t;
    double ep[6];
    s=str2time(s1,0,0,&t);   assert(s<0);
    s=str2time(s1,30,10,&t); assert(s<0);
    s=str2time(s1,10,0,&t);  assert(s<0);
    s=str2time(s1,-1,10,&t); assert(s<0);
    s=str2time(s1,4,17,&t); time2epoch(t,ep);
        assert(!s&&ep[0]==2004&&ep[1]==1&&ep[2]==1&&ep[3]==0&&ep[4]==1&&fabs(ep[5]-2.34)<1E-15);
    s=str2time(s2,4,21,&t); time2epoch(t,ep);
        assert(!s&&ep[0]==2000&&ep[1]==2&&ep[2]==3&&ep[3]==23&&ep[4]==59&&fabs(ep[5]-59.999)<1E-14);
    s=str2time(s3,4,20,&t); time2epoch(t,ep);
        assert(!s&&ep[0]==1980&&ep[1]==10&&ep[2]==30&&ep[3]==6&&ep[4]==58&&ep[5]==9);
    s=str2time(s4,4,20,&t); time2epoch(t,ep);
        assert(!s&&ep[0]==2037&&ep[1]==12&&ep[2]==31&&ep[3]==1&&ep[4]==2&&ep[5]==3);
    
    printf("%s utset2 : OK\n",__FILE__);
}
/* epoch2time(),time2epoch() */
void utest3(void)
{
    double ep0[]={1980, 1, 6, 0, 0, 0.000000};
    double ep1[]={2004, 2,28, 2, 0,59.999999};
    double ep2[]={2004, 2,29, 2, 0,30.000000};
    double ep3[]={2004,12,31,23,59,59.999999};
    double ep4[]={2037,10, 1, 0, 0, 0.000000};
#ifdef TIME_64BIT
    double ep5[]={2049, 2, 3, 4, 5, 6.000000}; /* 64bit time_t */
    double ep6[]={2099,12,31,23,59,59.999999}; /* 64bit time_t */
#endif
    int year,month,day,mday[]={31,28,31,30,31,30,31,31,30,31,30,31};
    gtime_t t;
    double ep[6];
    
    t=epoch2time(ep0); time2epoch(t,ep);
        assert(ep[0]==1980&&ep[1]==1&&ep[2]==6&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=epoch2time(ep1); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==2&&ep[2]==28&&ep[3]==2&&ep[4]==0&&fabs(ep[5]-59.999999)<1E-14);
    t=epoch2time(ep2); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==2&&ep[2]==29&&ep[3]==2&&ep[4]==0&&ep[5]==30.0);
    t=epoch2time(ep3); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==12&&ep[2]==31&&ep[3]==23&&ep[4]==59&&fabs(ep[5]-59.999999)<1E-14);
    t=epoch2time(ep4); time2epoch(t,ep);
        assert(ep[0]==2037&&ep[1]==10&&ep[2]==1&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
#ifdef TIME_64BIT
    t=epoch2time(ep5); time2epoch(t,ep);
        assert(ep[0]==2049&&ep[1]==2&&ep[2]==3&&ep[3]==4&&ep[4]==5&&ep[5]==6.0);
    t=epoch2time(ep6); time2epoch(t,ep);
        assert(ep[0]==2099&&ep[1]==12&&ep[2]==31&&ep[3]==23&&ep[4]==59&&fabs(ep[5]-59.999999)<1E-14);
#endif
    
#ifdef TIME_64BIT
    for (year=1970;year<=2099;year++) {
#else
    for (year=1970;year<=2037;year++) {
#endif
        mday[1]=year%4==0?29:28;
        for (month=1;month<=12;month++) {
            for (day=1;day<=mday[month-1];day++) {
                if (year==1970&&month==1&&day==1) continue;
                ep0[0]=year; ep0[1]=month; ep0[2]=day;
                t=epoch2time(ep0); time2epoch(t,ep);
                /* fprintf(stderr,"ep=%.0f %2.0f %2.0f : %.0f %2.0f %2.0f\n",
                        ep0[0],ep0[1],ep0[2],ep[0],ep[1],ep[2]); */
                assert(ep[0]==ep0[0]&&ep[1]==ep0[1]&&ep[2]==ep0[2]);
                assert(ep[3]==0.0&&ep[4]==0.0&&ep[5]==0.0);
            }
        }
    }
    printf("%s utset3 : OK\n",__FILE__);
}
/* gpst2time(), time2gpst() */
void utest4(void)
{
    gtime_t t;
    double ep[6];
    int w,week;
    double time,tt;
    t=gpst2time(0,0.0); time2epoch(t,ep);
        assert(ep[0]==1980&&ep[1]==1&&ep[2]==6&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=gpst2time(1400,86400.0); time2epoch(t,ep);
        assert(ep[0]==2006&&ep[1]==11&&ep[2]==6&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=gpst2time(1400,86400.0*7-1.0); time2epoch(t,ep);
        assert(ep[0]==2006&&ep[1]==11&&ep[2]==11&&ep[3]==23&&ep[4]==59&&ep[5]==59.0);
    t=gpst2time(1400,86400.0*7); time2epoch(t,ep);
        assert(ep[0]==2006&&ep[1]==11&&ep[2]==12&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=gpst2time(1401,0.0); time2epoch(t,ep);
        assert(ep[0]==2006&&ep[1]==11&&ep[2]==12&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
#ifdef TIME_64BIT
    t=gpst2time(4000,0.0); time2epoch(t,ep);
        assert(ep[0]==2056&&ep[1]==9&&ep[2]==3&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=gpst2time(6260,345600.0); time2epoch(t,ep);
        assert(ep[0]==2099&&ep[1]==12&&ep[2]==31&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
#endif
    
#ifdef TIME_64BIT
    for (w=1000;w<=6260;w++) {
#else
    for (w=1000;w<1100;w++) {
#endif
        for (time=0.0;time<86400.0*7;time+=3600.0) {
            t=gpst2time(w,time); tt=time2gpst(t,&week);
            assert(tt==time&&week==w);
        }
    }
    printf("%s utset4 : OK\n",__FILE__);
}
/* timeadd() */
void utest5(void)
{
    double ep0[]={2003,12,31,23,59,59.000000};
    double ep1[]={2004, 1, 1, 0, 0, 1.000000};
    double ep2[]={2004, 2,28, 0, 0, 0.000000};
    double ep3[]={2004, 2,29, 0, 0, 0.000000};
    gtime_t t;
    double ep[6];
    t=timeadd(epoch2time(ep0),3.0); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==1&&ep[2]==1&&ep[3]==0&&ep[4]==0&&ep[5]==2.0);
    t=timeadd(epoch2time(ep1),-3.0); time2epoch(t,ep);
        assert(ep[0]==2003&&ep[1]==12&&ep[2]==31&&ep[3]==23&&ep[4]==59&&ep[5]==58.0);
    t=timeadd(epoch2time(ep2),86400.0); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==2&&ep[2]==29&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=timeadd(epoch2time(ep2),86400.0*2); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==3&&ep[2]==1&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=timeadd(epoch2time(ep3),86400.0*2); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==3&&ep[2]==2&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    
    printf("%s utset5 : OK\n",__FILE__);
}
/* timediff() */
void utest6(void)
{
    double ep0[]={2003,12,31,23,59,59.000000};
    double ep1[]={2004, 1, 1, 0, 0, 1.000000};
    double ep2[]={2004, 2,28, 0, 0, 0.000000};
    double ep3[]={2004, 2,29, 0, 0, 0.000000};
    double ep4[]={2004, 3, 1, 0, 0, 0.000000};
    double sec;
    sec=timediff(epoch2time(ep1),epoch2time(ep0));
        assert(sec==2.0);
    sec=timediff(epoch2time(ep0),epoch2time(ep1));
        assert(sec==-2.0);
    sec=timediff(epoch2time(ep3),epoch2time(ep2));
        assert(sec==86400.0);
    sec=timediff(epoch2time(ep4),epoch2time(ep2));
        assert(sec==86400.0*2);
    sec=timediff(epoch2time(ep3),epoch2time(ep4));
        assert(sec==-86400.0);
    
    printf("%s utset6 : OK\n",__FILE__);
}
/* gpst2utc() */
void utest7(void)
{
    double ep0[]={1980, 1, 6, 0, 0, 0.000000};
    double ep1[]={1992, 7, 1, 0, 0, 6.999999};
    double ep2[]={1992, 7, 1, 0, 0, 7.000000};
    double ep3[]={1992, 7, 1, 0, 0, 8.000000};
    double ep4[]={2004,12,31,23,59,59.999999};
    double ep5[]={2006, 1, 1, 0, 0, 0.000000};
    double ep6[]={2038, 1, 1, 0, 0, 0.000000};
    gtime_t t;
    double ep[6];
    t=gpst2utc(epoch2time(ep0)); time2epoch(t,ep);
        assert(ep[0]==1980&&ep[1]==1&&ep[2]==6&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=gpst2utc(epoch2time(ep1)); time2epoch(t,ep);
        assert(ep[0]==1992&&ep[1]==6&&ep[2]==30&&ep[3]==23&&ep[4]==59&&fabs(ep[5]-59.999999)<1E-14);
    t=gpst2utc(epoch2time(ep2)); time2epoch(t,ep);
        assert(ep[0]==1992&&ep[1]==7&&ep[2]==1&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=gpst2utc(epoch2time(ep3)); time2epoch(t,ep);
        assert(ep[0]==1992&&ep[1]==7&&ep[2]==1&&ep[3]==0&&ep[4]==0&&ep[5]==0.0);
    t=gpst2utc(epoch2time(ep4)); time2epoch(t,ep);
        assert(ep[0]==2004&&ep[1]==12&&ep[2]==31&&ep[3]==23&&ep[4]==59&&fabs(ep[5]-46.999999)<1E-14);
    t=gpst2utc(epoch2time(ep5)); time2epoch(t,ep);
        assert(ep[0]==2005&&ep[1]==12&&ep[2]==31&&ep[3]==23&&ep[4]==59&&ep[5]==47.0);
    t=gpst2utc(epoch2time(ep6)); time2epoch(t,ep);
        assert(ep[0]==2037&&ep[1]==12&&ep[2]==31&&ep[3]==23&&ep[4]==59&&ep[5]==42.0);
    printf("%s utset7 : OK\n",__FILE__);
}
/* utc2gpst(), gpst2utc() */
void utest8(void)
{
    double ep0[]={1980, 1, 6, 0, 0, 0.000000};
    double ep1[]={2010,12,31,23,59,59.999999};
    gtime_t t0,t1,t2,t3;
    t0=epoch2time(ep0);
    t1=epoch2time(ep1);
    while (t0.time<t1.time) {
        t2=utc2gpst(t0); t3=gpst2utc(t2); assert(t0.time==t3.time&&t0.sec==t3.sec);
        t0.time+=86400.0;
    }
    
    printf("%s utset8 : OK\n",__FILE__);
}
/* time2str() */
void utest9(void)
{
    double ep0[]={1970,12,31,23,59,59.1234567890123456};
    double ep1[]={2004, 1, 1, 0, 0, 0.0000000000000000};
    double ep2[]={2006, 2,28,23,59,59.9999995000000000};
    char s[128];
    int ret;
    time2str(epoch2time(ep0),s,0);
        ret=strcmp(s,"1970/12/31 23:59:59");
        assert(!ret);
    time2str(epoch2time(ep0),s,-1);
        ret=strcmp(s,"1970/12/31 23:59:59");
        assert(!ret);
    time2str(epoch2time(ep0),s,10);
        ret=strcmp(s,"1970/12/31 23:59:59.1234567890");
        assert(!ret);
    time2str(epoch2time(ep1),s,0);
        ret=strcmp(s,"2004/01/01 00:00:00");
        assert(!ret);
    time2str(epoch2time(ep1),s,16);
        ret=strcmp(s,"2004/01/01 00:00:00.000000000000");
        assert(!ret);
    time2str(epoch2time(ep2),s,0);
        ret=strcmp(s,"2006/03/01 00:00:00");
        assert(!ret);
    time2str(epoch2time(ep2),s,6);
        ret=strcmp(s,"2006/03/01 00:00:00.000000");
        assert(!ret);
    time2str(epoch2time(ep2),s,7);
        ret=strcmp(s,"2006/02/28 23:59:59.9999995");
        assert(!ret);
    
    printf("%s utset9 : OK\n",__FILE__);
}
/* timeget() */
void utest10(void)
{
    char s1[64],s2[64];
    gtime_t time1,time2;
    int i,j;
    time1=timeget();
    for (i=0;i<2000000000;i++) j=1;
    time2=timeget();
    time2str(time1,s1,6);
    time2str(time2,s2,6);
    assert(timediff(time1,time2)<=0.0);
    
    printf("%s utset10 : OK\n",__FILE__);
}
/* time2doy() */
void utest11(void)
{
    double ep1[]={2004,1,1,0,0,0};
    double ep2[]={2004,12,31,0,0,0};
    double ep3[]={2005,12,31,12,0,0};
    double doy1,doy2,doy3;
    doy1=time2doy(epoch2time(ep1));
    doy2=time2doy(epoch2time(ep2));
    doy3=time2doy(epoch2time(ep3));
    assert(fabs(doy1-1.0)<1E-6);
    assert(fabs(doy2-366.0)<1E-6);
    assert(fabs(doy3-365.5)<1E-6);
    
    printf("%s utset11 : OK\n",__FILE__);
}
int main(void)
{
    utest1();
    utest2();
    utest3();
    utest4();
    utest5();
    utest6();
    utest7();
    utest8();
    utest9();
    utest10();
    utest11();
    return 0;
}
