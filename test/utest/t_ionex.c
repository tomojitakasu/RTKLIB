/*------------------------------------------------------------------------------
* rtklib unit test driver : ionex function
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include "../../src/rtklib.h"

static void dumptec(const tec_t *tec, int n, int level)
{
    const tec_t *p;
    char s[64];
    int i,j,k,m;
    
    for (i=0;i<n;i++) {
        p=tec+i;
        time2str(p->time,s,3);
        printf("(%2d) time =%s ndata=%d %d %d\n",i+1,s,p->ndata[0],p->ndata[1],p->ndata[2]);
        if (level<1) continue;
        printf("lats =%6.1f %6.1f %6.1f\n",p->lats[0],p->lats[1],p->lats[2]);
        printf("lons =%6.1f %6.1f %6.1f\n",p->lons[0],p->lons[1],p->lons[2]);
        printf("hgts =%6.1f %6.1f %6.1f\n",p->hgts[0],p->hgts[1],p->hgts[2]);
        printf("data =\n");
        for (j=0;j<p->ndata[2];j++) { /* hgt */
            for (k=0;k<p->ndata[0];k++) { /* lat */
                printf("lat=%.1f lon=%.1f:%.1f hgt=%.1f\n",p->lats[0]+k*p->lats[2],
                       p->lons[0],p->lons[1],p->hgts[0]+j*p->hgts[2]);
                for (m=0;m<p->ndata[1];m++) { /* lon */
                    if (m>0&&m%16==0) printf("\n");
                    printf("%5.1f ",p->data[k+p->ndata[0]*(m+p->ndata[1]*j)]);
                }
                printf("\n");
            }
            printf("\n");
        }
        printf("rms  =\n");
        for (j=0;j<p->ndata[2];j++) { /* hgt */
            for (k=0;k<p->ndata[0];k++) { /* lat */
                printf("lat=%.1f lon=%.1f:%.1f hgt=%.1f\n",p->lats[0]+k*p->lats[2],
                       p->lons[0],p->lons[1],p->hgts[0]+j*p->hgts[2]);
                for (m=0;m<p->ndata[1];m++) { /* lon */
                    if (m>0&&m%16==0) printf("\n");
                    printf("%5.1f ",p->rms[k+p->ndata[0]*(m+p->ndata[1]*j)]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }
}
static void dumpdcb(const nav_t *nav)
{
    int i;
    printf("dcbs=\n");
    for (i=0;i<MAXSAT;i++) {
        printf("%3d: P1-P2=%6.3f\n",i+1,nav->cbias[i][0]/CLIGHT*1E9); /* ns */
    }
}
/* readtec() */
void utest1(void)
{
    char *file1="../data/sp3/igrg3380.10j";
    char *file2="../data/sp3/igrg3380.10i";
    char *file3="../data/sp3/igrg33*0.10i";
    nav_t nav={0};
    
    printf("file=%s\n",file1);
    readtec(file1,&nav,0);
        assert(nav.nt==0);
    
    printf("file=%s\n",file2);
    readtec(file2,&nav,0);
        assert(nav.nt==13);
    dumptec(nav.tec,nav.nt,1);
    
    printf("file=%s\n",file3);
    readtec(file3,&nav,0);
        assert(nav.nt==25);
    dumptec(nav.tec,nav.nt,0);
    
    dumptec(nav.tec   ,1,1);
    dumptec(nav.tec+12,1,1);
    dumpdcb(&nav);
    
    printf("%s utest1 : OK\n",__FILE__);
}
/* iontec() 1 */
void utest2(void)
{
    char *file3="../data/sp3/igrg33*0.10i";
    nav_t nav={0};
    gtime_t time1,time2,time3,time4;
    double ep1  []={2010,12, 4, 0, 0, 0};
    double ep2  []={2010,12, 5,23,59,59};
    double ep3  []={2010,12, 3,23,59,59}; /* error */
    double ep4  []={2010,12, 6, 0, 0, 0}; /* error */
    double pos1 []={ 45.1*D2R, 135.7*D2R,0.0};
    double pos2 []={-45.1*D2R,-170.7*D2R,0.0};
    double pos3 []={-45.1*D2R, 189.3*D2R,0.0};
    double pos4 []={ 87.6*D2R,   0.0*D2R,0.0}; /* out of grid */
    double pos5 []={-87.6*D2R,   0.0*D2R,0.0}; /* out of grid */
    double azel1[]={  0.0,90.0*D2R};
    double azel2[]={120.0,30.0*D2R};
    double azel3[]={  0.0,-0.1*D2R}; /* error */
    double delay1,var1,delay2=0,var2;
    int stat;
    
    time1=epoch2time(ep1);
    time2=epoch2time(ep2);
    time3=epoch2time(ep3);
    time4=epoch2time(ep4);
    
    readtec(file3,&nav,0);
    stat=iontec(time1,&nav,pos1,azel1,1,&delay1,&var1);
        assert(stat==1);
    stat=iontec(time2,&nav,pos1,azel1,1,&delay1,&var1);
        assert(stat==1);
    stat=iontec(time3,&nav,pos1,azel1,1,&delay1,&var1);
        assert(stat==0);
    stat=iontec(time4,&nav,pos1,azel1,1,&delay1,&var1);
        assert(stat==0);
    stat=iontec(time1,&nav,pos2,azel1,1,&delay1,&var1);
        assert(stat==1);
    stat=iontec(time1,&nav,pos3,azel1,1,&delay2,&var2);
        assert(stat==1);
        assert(fabs(delay1-delay2)<1E-4);
        assert(fabs(var1-var2)<1E-8);
    stat=iontec(time1,&nav,pos4,azel1,1,&delay1,&var1);
        assert(stat==1);
    stat=iontec(time1,&nav,pos5,azel1,1,&delay1,&var1);
        assert(stat==1);
    stat=iontec(time1,&nav,pos1,azel2,1,&delay1,&var1);
        assert(stat==1);
    stat=iontec(time1,&nav,pos1,azel3,1,&delay1,&var1);
        assert(stat==1&&delay1==0.0);
    
    printf("%s utest2 : OK\n",__FILE__);
}
/* iontec() 2 */
void utest3(void)
{
    FILE *fp;
    char *file3="../data/sp3/igrg33*0.10i";
    nav_t nav={0};
    gtime_t time1;
    double ep1[]={2010,12, 5, 0, 0, 0};
    double delay,var,pos[3]={0},azel[]={0.0,PI/2};
    int i,j;
    
    time1=epoch2time(ep1);
    readtec(file3,&nav,0);
    
    fp=fopen("testionex3.m","w");
        assert(fp);
    
    fprintf(fp,"tec=[\n");
    for (i=90;i>=-90;i-=2) {
        for (j=0;j<=360;j+=2) {
            pos[0]=i*D2R;
            pos[1]=j*D2R;
            if (iontec(time1,&nav,pos,azel,1,&delay,&var)) {
                fprintf(fp,"%4.2f ",delay);
            }
            else {
                fprintf(fp," nan ");
            }
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"];\n");
    fclose(fp);
    
    fp=fopen("testionex3.m","a");
        assert(fp);
    
    fprintf(fp,"rms=[\n");
    for (i=90;i>=-90;i-=2) {
        for (j=0;j<=360;j+=2) {
            pos[0]=i*D2R;
            pos[1]=j*D2R;
            if (iontec(time1,&nav,pos,azel,1,&delay,&var)) {
                fprintf(fp,"%4.2f ",sqrt(var));
            }
            else {
                fprintf(fp," nan ");
            }
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"];\n");
    fclose(fp);
    
    printf("%s utest3 : OK\n",__FILE__);
}
/* iontec() 3 */
void utest4(void)
{
    FILE *fp;
    char *file3="../data/sp3/igrg33*0.10i";
    nav_t nav={0};
    gtime_t time1;
    double ep1[]={2010,12, 3,12, 0, 0};
    double delay,var,pos[3]={25*D2R,135*D2R,0};
    double azel[]={75*D2R,90*D2R};
    int i;
    
    time1=epoch2time(ep1);
    readtec(file3,&nav,0);
    
    fp=fopen("testionex4.m","w");
        assert(fp);
    
    fprintf(fp,"tec=[\n");
    for (i=0;i<=86400*3;i+=30) {
        if (iontec(timeadd(time1,i),&nav,pos,azel,1,&delay,&var)) {
            fprintf(fp,"%6d %5.3f %5.3f\n",i,delay,sqrt(var));
        }
        else {
            fprintf(fp,"%6d  nan   nan\n",i);
        }
    }
    fprintf(fp,"];\n");
    fclose(fp);
    
    printf("%s utest4 : OK\n",__FILE__);
}
int main(int argc, char **argv)
{
    utest1();
    utest2();
    utest3();
    utest4();
    return 0;
}
