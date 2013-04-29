/*------------------------------------------------------------------------------
* dumpssr.c : dump ssr messages in rtcm log
*
* 2010/06/10 new
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "rtklib.h"

/* print ssr messages --------------------------------------------------------*/
static void printhead(int topt, int mopt)
{
    int i;
    
    printf("%% %s  SAT ",topt?"  DAY      TIME  ":"   GPST  ");
    
    if (mopt&1) {
        printf(" UDI IOD URA REF ");
    }
    if (mopt&2) {
        printf("%8s %8s %8s %8s %8s %8s ","DR","DA","DC","DDR","DDA","DDC");
    }
    if (mopt&4) {
        printf("%8s %8s %8s %8s ","DCLK","DDCLK","DDDCLK","HRCLK");
    }
    if (mopt&8) {
        for (i=0;i<12;i++) printf("   B%02d ",i+1);
    }
    printf("\n");
}
/* print ssr messages --------------------------------------------------------*/
static void printssrmsg(int sat, const ssr_t *ssr, int topt, int mopt)
{
    double tow;
    int week;
    char tstr[32],id[16];
    
    if (topt) {
        time2str(ssr->t0,tstr,0);
        printf("%s ",tstr);
    }
    else {
        tow=time2gpst(ssr->t0,&week);
        printf("%4d %6.0f ",week,tow);
    }
    satno2id(sat,id);
    printf("%4s ",id);
    
    if (mopt&1) {
        printf("%4.0f %3d %3d %3d ",ssr->udint,ssr->iode,ssr->ura,ssr->refd);
    }
    if (mopt&2) {
        printf("%8.3f %8.3f %8.3f %8.3f %8.3f %8.3f ",ssr->deph[0],ssr->deph[1],
               ssr->deph[2],ssr->ddeph[0],ssr->ddeph[1],ssr->ddeph[2]);
    }
    if (mopt&4) {
        printf("%8.3f %8.3f %8.3f %8.3f ",ssr->dclk[0],ssr->dclk[1],ssr->dclk[2],
               ssr->hrclk);
    }
    if (mopt&8) {
        printf("%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f ",
               ssr->cbias[0],ssr->cbias[1],ssr->cbias[2],ssr->cbias[3],
               ssr->cbias[4],ssr->cbias[5],ssr->cbias[6],ssr->cbias[7],
               ssr->cbias[8],ssr->cbias[9],ssr->cbias[10],ssr->cbias[11]);
    }
    printf("\n");
}
/* dump ssr messages ---------------------------------------------------------*/
static void dumpssrmsg(FILE *fp, int sat, int topt, int mopt)
{
    static rtcm_t rtcm;
    static gtime_t t0[MAXSAT]={{0}};
    int i,stat;
    
    init_rtcm(&rtcm);
    
    while ((stat=input_rtcm3f(&rtcm,fp))>=0) {
        
        if (stat!=10) continue; /* ssr message */
        
        for (i=0;i<MAXSAT;i++) {
            if (timediff(rtcm.ssr[i].t0,t0[i])==0.0) continue;
            t0[i]=rtcm.ssr[i].t0;
            
            if (!sat||i+1==sat) {
                printssrmsg(i+1,rtcm.ssr+i,topt,mopt);
            }
        }
    }
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    const char *usage="dumpssr [-t][-s sat][-i][-o][-c][-b][-h][-x tr] file";
    
    FILE *fp;
    char *file="";
    int i,sat=0,topt=0,mopt=0,trl=0;
    
    for (i=0;i<argc;i++) {
        if      (!strcmp(argv[i],"-t")) topt =1;
        else if (!strcmp(argv[i],"-i")) mopt|=1;
        else if (!strcmp(argv[i],"-o")) mopt|=2;
        else if (!strcmp(argv[i],"-c")) mopt|=4;
        else if (!strcmp(argv[i],"-b")) mopt|=8;
        else if (!strcmp(argv[i],"-s")&&i+1<argc) sat=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-x")&&i+1<argc) trl=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-h")) {
            fprintf(stderr,"usage: %s\n",usage);
            return 0;
        }
        else file=argv[i];
    }
    if (!mopt) mopt=0xFF;
    
    if (!(fp=fopen(file,"rb"))) {
        fprintf(stderr,"file open error: %s\n",file);
        return -1;
    }
    if (trl>0) {
        traceopen("dumpssr.trace");
        tracelevel(trl);
    }
    printhead(topt,mopt);
    
    dumpssrmsg(fp,sat,topt,mopt);
    
    fclose(fp);
    traceclose();
    
    return 0;
}
