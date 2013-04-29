/*------------------------------------------------------------------------------
* rtklib unit test driver : atomospheric models
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include "../../src/rtklib.h"

/* ionmodel() */
void utest1(void)
{
    double e1[]={2007,1,16, 1,0,0};
    double e2[]={2007,1,16,13,0,0};
    double e3[]={2007,1,16,22,0,0};
    double ion1[]={
         0.2E-7,-0.8e-8,-0.5e-7, 0.1e-6, 0.2E+6, 0.2e+6,-0.1e+6,-0.1e+7
    };
    double ion2[]={
         0.2E-7,-0.8e-8,-0.5e-7, 0.1e-6, 0.2E+6, 0.2e+6,-0.1e+6,-0.1e+7
    };
    double pos1 []={ 35*D2R, 140*D2R, 100.0};
    double pos2 []={-80*D2R,-170*D2R,1000.0};
    double pos3 []={ 10*D2R,  30*D2R,   0.0};
    double azel1[]={ 60*D2R, 75*D2R};
    double azel2[]={190*D2R,  3*D2R};
    double azel3[]={350*D2R, 60*D2R};
    double azel4[]={  0*D2R, 90*D2R};
    gtime_t t1=epoch2time(e1),t2=epoch2time(e2),t3=epoch2time(e3);
    double dion;
    
    dion=ionmodel(t1,ion1,pos1,azel1);
        assert(fabs(dion-6.73590532099438)<1e-8);
    
    dion=ionmodel(t1,ion1,pos2,azel1);
        assert(fabs(dion-3.56895382197387)<1e-8);
    
    dion=ionmodel(t1,ion1,pos3,azel1);
        assert(fabs(dion-3.80716435655161)<1e-8);
    
    dion=ionmodel(t2,ion1,pos1,azel1);
        assert(fabs(dion-5.21796954585452)<1e-8);
    
    dion=ionmodel(t3,ion1,pos1,azel1);
        assert(fabs(dion-5.90190539264777)<1e-8);
    
    dion=ionmodel(t1,ion1,pos1,azel2);
        assert(fabs(dion-21.6345415123632)<1e-8);
    
    dion=ionmodel(t1,ion1,pos1,azel3);
        assert(fabs(dion-7.33844278822561)<1e-8);
    
    dion=ionmodel(t1,ion1,pos1,azel4);
        assert(fabs(dion-6.58339711400694)<1e-8);
    
    dion=ionmodel(t1,ion2,pos1,azel1);
        assert(fabs(dion-6.73590532099438)<1e-8);
    
    printf("%s utest1 : OK\n",__FILE__);
}
/* ionmapf() */
void utest2(void)
{
    printf("%s utest2 : OK\n",__FILE__);
}
/* tropmodel */
void utest3(void)
{
    gtime_t time={0};
    double pos1 []={ 35*D2R, 140*D2R, 100.0};
    double pos2 []={-80*D2R,-170*D2R,1000.0};
    double pos3 []={-80*D2R,-170*D2R,100000.0};
    double pos4 []={-80*D2R,-170*D2R,-200.0};
    double azel1[]={ 60*D2R, 75*D2R};
    double azel2[]={190*D2R,  3*D2R};
    double azel3[]={190*D2R,-10*D2R};
    double dtrp;
    
    dtrp=tropmodel(time,pos1,azel1,0.5);
        assert(fabs(dtrp-2.44799870144088)<1e-8);
    
    dtrp=tropmodel(time,pos1,azel2,0.5);
        assert(fabs(dtrp-45.1808916506163)<1e-8);
    
    dtrp=tropmodel(time,pos2,azel1,0.5);
        assert(fabs(dtrp-2.17295817298152)<1e-8);
    
    dtrp=tropmodel(time,pos1,azel3,0.0);
        assert(fabs(dtrp-0.00000000000000)<1e-8);
    
    dtrp=tropmodel(time,pos3,azel1,0.0);
        assert(fabs(dtrp-0.00000000000000)<1e-8);
    
    dtrp=tropmodel(time,pos4,azel1,0.0);
        assert(fabs(dtrp-0.00000000000000)<1e-8);
    
    printf("%s utest3 : OK\n",__FILE__);
}
void utest4(void)
{
    double mapfd,mapfw;
    double e1[]={2007,1,16,6,0,0},e2[]={2030,12,31,23,59,59};
    double pos1 []={ 35*D2R, 140*D2R, 100.0};
    double pos2 []={-80*D2R,-170*D2R,1000.0};
    double pos3 []={ 10*D2R,  30*D2R,   0.0};
    double azel1[]={ 60*D2R, 75*D2R};
    double azel2[]={190*D2R,  3*D2R};
    double azel3[]={350*D2R, 60*D2R};
    double azel4[]={  0*D2R, 90*D2R};
    gtime_t t1=epoch2time(e1),t2=epoch2time(e2);

    mapfd=tropmapf(t1,pos1,azel1,&mapfw);
        assert(fabs(mapfd-1.035184526466435)<1e-8);
        assert(fabs(mapfw-1.035233787448654)<1e-8);
    
    mapfd=tropmapf(t1,pos1,azel2,&mapfw);
        assert(fabs(mapfd-14.643271711748200)<1e-8);
        assert(fabs(mapfw-16.455045694559484)<1e-8);
    
    mapfd=tropmapf(t1,pos1,azel3,&mapfw);
        assert(fabs(mapfd-1.154226397147367)<1e-8);
        assert(fabs(mapfw-1.154481126139610)<1e-8);
    
    mapfd=tropmapf(t1,pos1,azel4,&mapfw);
        assert(fabs(mapfd-1.000000000000000)<1e-8);
        assert(fabs(mapfw-1.000000000000000)<1e-8);
    
    mapfd=tropmapf(t2,pos1,azel1,&mapfw);
        assert(fabs(mapfd-1.035184415128022)<1e-8);
        assert(fabs(mapfw-1.035233787448654)<1e-8);
    
    mapfd=tropmapf(t1,pos2,azel1,&mapfw);
        assert(fabs(mapfd-1.035186155749051)<1e-8);
        assert(fabs(mapfw-1.035230548304367)<1e-8);
    
    mapfd=tropmapf(t1,pos3,azel1,&mapfw);
        assert(fabs(mapfd-1.035181919429758)<1e-8);
        assert(fabs(mapfw-1.035233200318210)<1e-8);
    
    mapfd=tropmapf(t1,pos1,azel1,NULL);
        assert(fabs(mapfd-1.035184526466435)<1e-8);
    
    printf("%s utest4 : OK\n",__FILE__);
}
int main(void)
{
    utest1();
    utest2();
    utest3();
    utest4();
    return 0;
}
