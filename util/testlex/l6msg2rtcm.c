/*------------------------------------------------------------------------------
* l6msg2rtcm.c : convert CSSR in L6 messages into RTCM 3 messages
*
* 2021/1/3 new
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "rtklib.h"

/* convert L6 messages into RTCM3 messages ---------------------------------------------------------*/
static void convl6msg(FILE *fp, FILE *fpout)
{
    static rtcm_t _rtcm;
    rtcm_t *rtcm=&_rtcm;
    int i=0,stat,n,nbit;
    static uint8_t buff[1024];
    
    init_rtcm(rtcm);
    
    while ((stat=input_l6msgsf(rtcm,1,fp))>=0) {
        if (stat!=10) continue; /* ssr message */
        i=0;
        while ((nbit=l6msg2rtcm(rtcm,i,buff))) { /* convert to RTCM3 */
        	n=(nbit+7)/8;
        	fwrite(buff,1,n+6,fpout); /* rtcm-header(3)+message(n)+cs(3) */
        	i+=nbit;
        }
    }
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    const char *usage="l6msg2rtcm [-o fileout][-x tr] file";
    
    FILE *fp=NULL,*fpout=NULL;
    char *file="",*fileout="l6msg.rtcm3";
    int i,trl=0;
    
    for (i=0;i<argc;i++) {
        if (!strcmp(argv[i],"-o")) fileout=argv[++i];
        else if (!strcmp(argv[i],"-x")&&i+1<argc) trl=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-h")) {
            fprintf(stderr,"usage: %s\n",usage);
            return 0;
        }
        else file=argv[i];
    }
    
    if (!(fp=fopen(file,"rb"))) {
        fprintf(stderr,"file open error: %s\n",file);
        return -1;
    }
    if (!(fpout=fopen(fileout,"wb"))) {
        fprintf(stderr,"file open error: %s\n",fileout);
        return -1;
    }
    if (trl>0) {
        traceopen("l6msg2rtcm.trace");
        tracelevel(trl);
    }
    
    convl6msg(fp,fpout);
    
    fclose(fpout);
    fclose(fp);
    traceclose();
    
    return 0;
}
