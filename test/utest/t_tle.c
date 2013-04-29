/*------------------------------------------------------------------------------
* rtklib unit test driver : norad two line element function
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include "../../src/rtklib.h"

#define OUT stdout

/* dump tle ------------------------------------------------------------------*/
static void dumptle(FILE *fp, const tle_t *tle)
{
    int i;
    
    for (i=0;i<tle->n;i++) {
        fprintf(fp,"(%2d) name = %s\n",     i+1,tle->data[i].name );
        fprintf(fp,"(%2d) satno= %s\n",     i+1,tle->data[i].satno);
        fprintf(fp,"(%2d) class= %c\n",     i+1,tle->data[i].satclass);
        fprintf(fp,"(%2d) desig= %s\n",     i+1,tle->data[i].desig);
        fprintf(fp,"(%2d) epoch= %s\n",     i+1,time_str(tle->data[i].epoch,0));
        fprintf(fp,"(%2d) etype= %d\n",     i+1,tle->data[i].etype);
        fprintf(fp,"(%2d) eleno= %d\n",     i+1,tle->data[i].eleno);
        fprintf(fp,"(%2d) ndot = %19.12e\n",i+1,tle->data[i].ndot );
        fprintf(fp,"(%2d) nddot= %19.12e\n",i+1,tle->data[i].nddot);
        fprintf(fp,"(%2d) bstar= %19.12e\n",i+1,tle->data[i].bstar);
        fprintf(fp,"(%2d) inc  = %19.12e\n",i+1,tle->data[i].inc  );
        fprintf(fp,"(%2d) OMG  = %19.12e\n",i+1,tle->data[i].OMG  );
        fprintf(fp,"(%2d) ecc  = %19.12e\n",i+1,tle->data[i].ecc  );
        fprintf(fp,"(%2d) omg  = %19.12e\n",i+1,tle->data[i].omg  );
        fprintf(fp,"(%2d) M    = %19.12e\n",i+1,tle->data[i].M    );
        fprintf(fp,"(%2d) n    = %19.12e\n",i+1,tle->data[i].n    );
        fprintf(fp,"(%2d) rev  = %d\n",     i+1,tle->data[i].rev  );
    }
}
/* tle_read() ----------------------------------------------------------------*/
static void utest1(void)
{
    const char *file1="../data/tle/tle_sgp4.err";
    const char *file2="../data/tle/tle_sgp4.txt";
    const char *file3="../data/tle/tle_nav.txt";
    tle_t tle={0};
    int stat;
    
    stat=tle_read(file1,&tle);
        assert(!stat);
    
    stat=tle_read(file2,&tle);
        assert(stat);
        assert(tle.n==1);
    
    stat=tle_read(file3,&tle);
        assert(stat);
        assert(tle.n==114);
#if 0
    dumptle(OUT,&tle);
#endif
    
    fprintf(OUT,"%s utest1 : OK\n",__FILE__);
}
/* tle_pos() -----------------------------------------------------------------*/
static void utest2(void)
{
    const char *file2="../data/tle/tle_sgp4.txt";
    const double ep0[6]={1980,1,1};
    tle_t tle={0};
    gtime_t epoch;
    double min,rs[6];
    int i,stat;
    
    epoch=utc2gpst(timeadd(epoch2time(ep0),274.98708465*86400.0));
    
    stat=tle_read(file2,&tle);
        assert(stat);
    
    stat=tle_pos(epoch,"TEST_ERR","","",&tle,NULL,rs);
        assert(!stat);
    
    for (i=0;i<5;i++) {
        min=360.0*i;
        
        stat=tle_pos(timeadd(epoch,min*60.0),"TEST_SAT","","",&tle,NULL,rs);
            assert(stat);
        
        fprintf(OUT,"%4.0f: %14.8f %14.8f %14.8f  %11.8f %11.8f %11.8f\n",min,
                rs[0]/1e3,rs[1]/1e3,rs[2]/1e3,rs[3]/1e3,rs[4]/1e3,rs[5]/1e3);
    }
    fprintf(OUT,"%s utest2 : OK\n",__FILE__);
}
/* tle_pos() accuracy --------------------------------------------------------*/
static void utest3(void)
{
    const char *file1="../data/tle/brdc3050.12*";
    const char *file2="../data/tle/TLE_GNSS_20121101.txt";
    const char *file3="../data/tle/igs17127.erp";
    const double ep[6]={2012,10,31,0,0,0};
    nav_t nav={0};
    erp_t erp={0};
    tle_t tle={0};
    gtime_t time;
    char sat[32];
    double rs1[6],rs2[6],ds[6],dts[2],var;
    int i,j,k,stat,svh;
    
    readrnx(file1,0,"",NULL,&nav,NULL);
        assert(nav.n>0);
    
    stat=readerp(file3,&erp);
        assert(stat);
    
    stat=tle_read(file2,&tle);
        assert(stat);
    
    for (i=0;i<MAXSAT;i++) {
        satno2id(i+1,sat);
        
        fprintf(OUT,"SAT=%s\n",sat);
        
        for (j=0;j<96;j++) {
            time=timeadd(epoch2time(ep),900.0*j);
            
            if (!satpos(time,time,i+1,EPHOPT_BRDC,&nav,rs1,dts,&var,&svh)) continue;
            
            if (satsys(i+1,NULL)==SYS_QZS) svh&=0xFE;
            
            if (svh) continue;
            
            stat=tle_pos(time,sat,"","",&tle,&erp,rs2);
                assert(stat);
            
            for (k=0;k<3;k++) ds[k]=rs2[k]-rs1[k];
            
            fprintf(OUT,"%6.0f %11.3f %11.3f %11.3f %11.3f\n",900.0*j,
                    ds[0]/1e3,ds[1]/1e3,ds[2]/1e3,norm(ds,3)/1e3);
                
                assert(norm(ds,3)/1e3<300.0);
        }
        fprintf(OUT,"\n");
    }
    fprintf(OUT,"%s utest3 : OK\n",__FILE__);
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    utest1();
    utest2();
    utest3();
    return 0;
}
