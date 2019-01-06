/*------------------------------------------------------------------------------
* convrnx.c : rinex translator for rtcm and receiver raw data log
*
*          Copyright (C) 2009-2018 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.2 $ $Date: 2008/07/17 21:48:06 $
* history : 2009/04/10 1.0  new
*           2009/06/02 1.1  support glonass
*           2009/12/18 1.2  add check return of init_rtcm()/init_raw()
*           2010/07/15 1.3  support wildcard expansion of input file
*                           support rinex 3.00
*                           support rinex as input format
*                           support output of geo navigation message
*                           support rtcm antenna and receiver info
*                           changed api:
*                               convrnx()
*           2011/05/27 1.4  support GW10, Javad, LEX receiver
*                           support lex message conversion
*                           change api convrnx()
*           2012/10/18 1.5  support multiple codes in a frequency
*           2012/10/29 1.6  fix bug on scanning obs types
*                           support output of compass navigation data
*                           add supported obs types for rinex input
*           2013/03/11 1.7  support binex and rinex 3.02
*                           add approx position in rinex obs header if blank
*           2014/05/24 1.8  support beidou B1
*           2014/08/26 1.9  support input format rt17
*           2015/05/24 1.10 fix bug on setting antenna delta in rtcm2opt()
*           2016/07/04 1.11 support IRNSS
*           2016/10/10 1.12 support event output by staid change in rtcm
*                           support separted navigation files for ver.3
*           2017/06/06 1.13 fix bug on array overflow in set_obstype() and
*                           scan_obstype()
*           2018/10/10 1.14 add trace of half-cycle ambiguity status
*                           fix bug on missing navigation data
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define NOUTFILE        9       /* number of output files */
#define NSATSYS         7       /* number of satellite systems */
#define TSTARTMARGIN    60.0    /* time margin for file name replacement */

#define TL_HALFC        4       /* trace level for half-cyc ambiguity status */

/* type definition -----------------------------------------------------------*/

typedef struct stas_tag {       /* station list type */
    int    staid;               /* station id */
    gtime_t time;               /* first epoch time */
    sta_t  sta;                 /* station parameters */
    struct stas_tag *next;      /* next list */
} stas_t;

typedef struct halfd_tag {      /* half-cycle ambiguity data type */
    gtime_t ts,te;              /* start/end time (gpst) */
    unsigned char stat;         /* status (0:unresolved,1:half-cycle added, */
                                /*         2:half-cycle not added) */
    struct halfd_tag *next;     /* next list */
} halfd_t;

typedef struct {                /* half-cycle ambiguity status type */
    int stat[MAXSAT][NFREQ+NEXOBS];
    halfd_t *data[MAXSAT][NFREQ+NEXOBS];
} halfc_t;

typedef struct {                /* stream file type */
    int    format;              /* stream format (STRFMT_???) */
    int    sat;                 /* input satellite */
    obs_t  *obs;                /* input observation data */
    nav_t  *nav;                /* input navigation data */
    gtime_t time;               /* current time */
    rtcm_t rtcm;                /* rtcm data */
    raw_t  raw;                 /* receiver raw data */
    rnxctr_t rnx;               /* rinex data */
    FILE   *fp;                 /* file pointer */
} strfile_t;

/* global variables ----------------------------------------------------------*/
static const int navsys[]={     /* system codes */
    SYS_GPS,SYS_GLO,SYS_GAL,SYS_QZS,SYS_SBS,SYS_CMP,SYS_IRN,0
};
/* convert rinex obs type ver.3 -> ver.2 -------------------------------------*/
static void convcode(double ver, int sys, char *type)
{
    if (ver>=2.12&&(sys==SYS_GPS||sys==SYS_QZS||sys==SYS_SBS)&&
        !strcmp(type+1,"1C")) { /* L1C/A */
        strcpy(type+1,"A");
    }
    else if (ver>=2.12&&(sys==SYS_GPS||sys==SYS_QZS)&&
             (!strcmp(type+1,"1S")||!strcmp(type+1,"1L")||
              !strcmp(type+1,"1X"))) { /* L1C */
        strcpy(type+1,"B");
    }
    else if (ver>=2.12&&(sys==SYS_GPS||sys==SYS_QZS)&&
             (!strcmp(type+1,"2S")||!strcmp(type+1,"2L")||
              !strcmp(type+1,"2X"))) { /* L2C */
        strcpy(type+1,"C");
    }
    else if (ver>=2.12&&sys==SYS_GLO&&!strcmp(type+1,"1C")) { /* L1C/A */
        strcpy(type+1,"A");
    }
    else if (ver>=2.12&&sys==SYS_GLO&&!strcmp(type+1,"2C")) { /* L2C/A */
        strcpy(type+1,"D");
    }
    else if (sys==SYS_CMP&&(!strcmp(type+1,"1I")||!strcmp(type+1,"1Q")||
             !strcmp(type+1,"1X"))) { /* B1 */
        strcpy(type+1,"2");
    }
    else if (!strcmp(type,"C1P")||!strcmp(type,"C1W")||!strcmp(type,"C1Y")||
             !strcmp(type,"C1N")) { /* L1P,P(Y) */
        strcpy(type,"P1");
    }
    else if (!strcmp(type,"C2P")||!strcmp(type,"C2W")||!strcmp(type,"C2Y")||
             !strcmp(type,"C2N")||!strcmp(type,"C2D")) { /* L2P,P(Y) */
        strcpy(type,"P2");
    }
    else {
        type[2]='\0';
    }
}
/* set rinex station and receiver info to options ----------------------------*/
static void rnx2opt(const rnxctr_t *rnx, rnxopt_t *opt)
{
    double pos[3],enu[3];
    int i;
    
    trace(3,"rnx2opt:\n");
    
    /* receiver and antenna info */
    if (!*opt->marker&&!*opt->markerno) {
        strcpy(opt->marker,rnx->sta.name);
        strcpy(opt->markerno,rnx->sta.marker);
    }
    if (!*opt->rec[0]&&!*opt->rec[1]&&!*opt->rec[2]) {
        strcpy(opt->rec[0],rnx->sta.recsno);
        strcpy(opt->rec[1],rnx->sta.rectype);
        strcpy(opt->rec[2],rnx->sta.recver);
    }
    if (!*opt->ant[0]&&!*opt->ant[1]) {
        strcpy(opt->ant[0],rnx->sta.antsno);
        strcpy(opt->ant[1],rnx->sta.antdes);
    }
    /* antenna approx position */
    if (!opt->autopos&&norm(rnx->sta.pos,3)>0.0) {
        for (i=0;i<3;i++) opt->apppos[i]=rnx->sta.pos[i];
    }
    /* antenna delta */
    if (norm(opt->antdel,3)>0.0) {
        ;
    }
    else if (norm(rnx->sta.del,3)>0.0) {
        if (!rnx->sta.deltype) { /* enu */
            opt->antdel[0]=rnx->sta.del[2]; /* h */
            opt->antdel[1]=rnx->sta.del[0]; /* e */
            opt->antdel[2]=rnx->sta.del[1]; /* n */
        }
        else if (norm(opt->apppos,3)>0.0) { /* xyz */
            ecef2pos(opt->apppos,pos);
            ecef2enu(pos,rnx->sta.del,enu);
            opt->antdel[0]=enu[2];
            opt->antdel[1]=enu[0];
            opt->antdel[2]=enu[1];
        }
    }
}
/* set rtcm antenna and receiver info to options -----------------------------*/
static void rtcm2opt(const rtcm_t *rtcm, const stas_t *stas, rnxopt_t *opt)
{
    const stas_t *p;
    const sta_t *sta=NULL;
    double pos[3],enu[3];
    int i,staid=-1;
    
    trace(3,"rtcm2opt:\n");
    
    /* search first epoch station info */
    for (p=stas;p;p=p->next) {
        sta=&p->sta;
        staid=p->staid;
        if (timediff(p->time,opt->tstart)<DTTOL) break;
    }
    if (!sta) {
        sta=&rtcm->sta;
    }
    /* comment */
    if (staid>=0) {
        if (!*opt->marker) sprintf(opt->marker,"%04d",staid);
        sprintf(opt->comment[1]+strlen(opt->comment[1]),", station ID: %d",
                staid);
    }
    /* receiver and antenna info */
    if (!*opt->rec[0]&&!*opt->rec[1]&&!*opt->rec[2]) {
        strcpy(opt->rec[0],sta->recsno);
        strcpy(opt->rec[1],sta->rectype);
        strcpy(opt->rec[2],sta->recver);
    }
    if (!*opt->ant[0]&&!*opt->ant[1]&&!*opt->ant[2]) {
        strcpy(opt->ant[0],sta->antsno);
        strcpy(opt->ant[1],sta->antdes);
        if (sta->antsetup) {
            sprintf(opt->ant[2],"%d",sta->antsetup);
        }
        else *opt->ant[2]='\0';
    }
    /* antenna approx position */
    if (!opt->autopos&&norm(sta->pos,3)>0.0) {
        for (i=0;i<3;i++) opt->apppos[i]=sta->pos[i];
    }
    /* antenna delta */
    if (norm(opt->antdel,3)>0.0) {
        ;
    }
    else if (norm(sta->del,3)>0.0) {
        if (!sta->deltype&&norm(sta->del,3)>0.0) { /* enu */
            opt->antdel[0]=sta->del[2]; /* h */
            opt->antdel[1]=sta->del[0]; /* e */
            opt->antdel[2]=sta->del[1]; /* n */
        }
        else if (norm(sta->pos,3)>0.0) { /* xyz */
            ecef2pos(sta->pos,pos);
            ecef2enu(pos,sta->del,enu);
            opt->antdel[0]=enu[2]; /* h */
            opt->antdel[1]=enu[0]; /* e */
            opt->antdel[2]=enu[1]; /* n */
        }
    }
    else {
        opt->antdel[0]=sta->hgt;
        opt->antdel[1]=0.0;
        opt->antdel[2]=0.0;
    }
}
/* set raw antenna and receiver info to options ------------------------------*/
static void raw2opt(const raw_t *raw, rnxopt_t *opt)
{
    double pos[3],enu[3];
    int i;
    
    trace(3,"raw2opt:\n");
    
    /* receiver and antenna info */
    if (!*opt->rec[0]&&!*opt->rec[1]&&!*opt->rec[2]) {
        strcpy(opt->rec[0],raw->sta.recsno);
        strcpy(opt->rec[1],raw->sta.rectype);
        strcpy(opt->rec[2],raw->sta.recver);
    }
    if (!*opt->ant[0]&&!*opt->ant[1]&&!*opt->ant[2]) {
        strcpy(opt->ant[0],raw->sta.antsno);
        strcpy(opt->ant[1],raw->sta.antdes);
        if (raw->sta.antsetup) {
            sprintf(opt->ant[2],"%d",raw->sta.antsetup);
        }
        else *opt->ant[2]='\0';
    }
    /* antenna approx position */
    if (!opt->autopos&&norm(raw->sta.pos,3)>0.0) {
        for (i=0;i<3;i++) opt->apppos[i]=raw->sta.pos[i];
    }
    /* antenna delta */
    if (norm(raw->sta.del,3)>0.0) {
        if (!raw->sta.deltype&&norm(raw->sta.del,3)>0.0) { /* enu */
            opt->antdel[0]=raw->sta.del[2]; /* h */
            opt->antdel[1]=raw->sta.del[0]; /* e */
            opt->antdel[2]=raw->sta.del[1]; /* n */
        }
        else if (norm(raw->sta.pos,3)>0.0) { /* xyz */
            ecef2pos(raw->sta.pos,pos);
            ecef2enu(pos,raw->sta.del,enu);
            opt->antdel[0]=enu[2]; /* h */
            opt->antdel[1]=enu[0]; /* e */
            opt->antdel[2]=enu[1]; /* n */
        }
    }
    else {
        opt->antdel[0]=raw->sta.hgt;
        opt->antdel[1]=0.0;
        opt->antdel[2]=0.0;
    }
}
/* generate stream file ------------------------------------------------------*/
static strfile_t *gen_strfile(int format, const char *opt, gtime_t time)
{
    strfile_t *str;
    
    trace(3,"init_strfile:\n");
    
    if (!(str=(strfile_t *)calloc(sizeof(strfile_t),1))) return NULL;
    
    if (format==STRFMT_RTCM2||format==STRFMT_RTCM3) {
        if (!init_rtcm(&str->rtcm)) {
            showmsg("init rtcm error");
            return 0;
        }
        str->rtcm.time=time;
        str->obs=&str->rtcm.obs;
        str->nav=&str->rtcm.nav; 
        strcpy(str->rtcm.opt,opt);
    }
    else if (format<=MAXRCVFMT) {
        if (!init_raw(&str->raw,format)) {
            showmsg("init raw error");
            return 0;
        }
        str->raw.time=time;
        str->obs=&str->raw.obs;
        str->nav=&str->raw.nav;
        strcpy(str->raw.opt,opt);
    }
    else if (format==STRFMT_RINEX) {
        if (!init_rnxctr(&str->rnx)) {
            showmsg("init rnx error");
            return 0;
        }
        str->obs=&str->rnx.obs;
        str->nav=&str->rnx.nav;
        strcpy(str->rnx.opt,opt);
    }
    str->format=format;
    str->sat=0;
    str->fp=NULL;
    return str;
}
/* free stream file ----------------------------------------------------------*/
static void free_strfile(strfile_t *str)
{
    trace(3,"free_strfile:\n");
    
    if (str->format==STRFMT_RTCM2||str->format==STRFMT_RTCM3) {
        free_rtcm(&str->rtcm);
    }
    else if (str->format<=MAXRCVFMT) {
        free_raw(&str->raw);
    }
    else if (str->format==STRFMT_RINEX) {
        free_rnxctr(&str->rnx);
    }
    free(str);
}
/* input stream file ---------------------------------------------------------*/
static int input_strfile(strfile_t *str)
{
    int type=0;
    
    trace(4,"input_strfile:\n");
    
    if (str->format==STRFMT_RTCM2) {
        if ((type=input_rtcm2f(&str->rtcm,str->fp))>=1) {
            str->time=str->rtcm.time;
            str->sat=str->rtcm.ephsat;
        }
    }
    else if (str->format==STRFMT_RTCM3) {
        if ((type=input_rtcm3f(&str->rtcm,str->fp))>=1) {
            str->time=str->rtcm.time;
            str->sat=str->rtcm.ephsat;
        }
    }
    else if (str->format<=MAXRCVFMT) {
        if ((type=input_rawf(&str->raw,str->format,str->fp))>=1) {
            str->time=str->raw.time;
            str->sat=str->raw.ephsat;
        }
    }
    else if (str->format==STRFMT_RINEX) {
        if ((type=input_rnxctr(&str->rnx,str->fp))>=1) {
            str->time=str->rnx.time;
            str->sat=str->rnx.ephsat;
        }
    }
    trace(4,"input_strfile: time=%s type=%d sat=%2d\n",time_str(str->time,3),
          type,str->sat);
    return type;
}
/* open stream file ----------------------------------------------------------*/
static int open_strfile(strfile_t *str, const char *file)
{
    trace(3,"open_strfile: file=%s\n",file);
    
    if (str->format==STRFMT_RTCM2||str->format==STRFMT_RTCM3) {
        if (!(str->fp=fopen(file,"rb"))) {
            showmsg("rtcm open error: %s",file);
            return 0;
        }
    }
    else if (str->format<=MAXRCVFMT) {
        if (!(str->fp=fopen(file,"rb"))) {
            showmsg("log open error: %s",file);
            return 0;
        }
        /* read head to resolve time ambiguity */
        if (str->time.time==0) {
            str->raw.flag=1;
            while (input_strfile(str)>=-1&&str->time.time==0) ;
            str->raw.flag=1;
            rewind(str->fp);
        }
    }
    else if (str->format==STRFMT_RINEX) {
        if (!(str->fp=fopen(file,"r"))) {
            showmsg("rinex open error: %s",file);
            return 0;
        }
        /* open rinex control */
        if (!open_rnxctr(&str->rnx,str->fp)) {
            showmsg("no rinex file: %s",file);
            fclose(str->fp);
            return 0;
        }
    }
    return 1;
}
/* close stream file ---------------------------------------------------------*/
static void close_strfile(strfile_t *str)
{
    trace(3,"close_strfile:\n");
    
    if (str->format==STRFMT_RTCM2||str->format==STRFMT_RTCM3) {
        if (str->fp) fclose(str->fp);
    }
    else if (str->format<=MAXRCVFMT) {
        if (str->fp) fclose(str->fp);
    }
    else if (str->format==STRFMT_RINEX) {
        if (str->fp) fclose(str->fp);
    }
}
/* sort codes ----------------------------------------------------------------*/
static void sort_codes(unsigned char *codes, unsigned char *types, int n)
{
    unsigned char tmp;
    char *obs1,*obs2;
    int i,j;
    
    for (i=0;i<n-1;i++) for (j=i+1;j<n;j++) {
       obs1=code2obs(codes[i],NULL);
       obs2=code2obs(codes[j],NULL);
       if (strcmp(obs1,obs2)<=0) continue;
       tmp=codes[i]; codes[i]=codes[j]; codes[j]=tmp;
       tmp=types[i]; types[i]=types[j]; types[j]=tmp;
    }
}
/* set observation types in rinex option -------------------------------------*/
static void setopt_obstype(const unsigned char *codes,
                           const unsigned char *types, int sys, rnxopt_t *opt)
{
    const char type_str[]="CLDS";
    char type[16],*id;
    int i,j,k,freq;
    
    trace(3,"setopt_obstype: sys=%d\n",sys);
    
    opt->nobs[sys]=0;
    
    if (!(navsys[sys]&opt->navsys)) return;
    
    for (i=0;codes[i];i++) {
        
        if (!(id=code2obs(codes[i],&freq))) continue;
        
        if (!(opt->freqtype&(1<<(freq-1)))||opt->mask[sys][codes[i]-1]=='0') {
            continue;
        }
        for (j=0;j<4;j++) {
            if (!(opt->obstype&(1<<j))) continue;
            if (types&&!(types[i]&(1<<j))) continue;
            
            /* observation type in ver.3 */
            sprintf(type,"%c%s",type_str[j],id);
            if (type[0]=='C'&&type[2]=='N') continue; /* codeless */
            
            if (opt->rnxver<=2.99) { /* ver.2 */
                
                /* ver.3 -> ver.2 */
                convcode(opt->rnxver,navsys[sys],type);
                
                /* check duplicated observation type */
                for (k=0;k<opt->nobs[0];k++) {
                    if (!strcmp(opt->tobs[0][k],type)) break;
                }
                if (k>=opt->nobs[0]&&opt->nobs[0]<MAXOBSTYPE) {
                    strcpy(opt->tobs[0][opt->nobs[0]++],type);
                }
            }
            else if (opt->nobs[sys]<MAXOBSTYPE) { /* ver.3 */
                strcpy(opt->tobs[sys][opt->nobs[sys]++],type);
            }
        }
    }
}
/* update station list -------------------------------------------------------*/
static void update_stas(stas_t **stas, strfile_t *str)
{
    stas_t *p;
    
    if (str->format!=STRFMT_RTCM2&&str->format!=STRFMT_RTCM3) return;
    
    for (p=*stas;p;p=p->next) {
        if (p->staid==str->rtcm.staid) {
            p->sta=str->rtcm.sta;
            return;
        }
    }
    if (!(p=(stas_t *)calloc(sizeof(stas_t),1))) return;
    p->staid=str->rtcm.staid;
    p->time=str->rtcm.time;
    p->sta=str->rtcm.sta;
    p->next=*stas;
    *stas=p;
    trace(2,"update_stas: staid=%d time=%s\n",str->rtcm.staid,
          time_str(str->rtcm.time,0));
}
/* update half-cycle ambiguity status ----------------------------------------*/
static void update_halfc(halfc_t *halfc, obsd_t *obs)
{
    halfd_t *p;
    int i;
    
    for (i=0;i<NFREQ+NEXOBS;i++) {
        if (obs->LLI[i]&LLI_SLIP) {
            halfc->stat[obs->sat-1][i]=0;
        }
        if (obs->LLI[i]&LLI_HALFC) {
            if (!halfc->stat[obs->sat-1][i]) {
                if (!(p=(halfd_t *)calloc(sizeof(halfd_t),1))) return;
                p->ts=p->te=obs->time;
                p->next=halfc->data[obs->sat-1][i];
                halfc->data[obs->sat-1][i]=p;
                halfc->stat[obs->sat-1][i]=1;
            }
            else if ((p=halfc->data[obs->sat-1][i])) {
                p->te=obs->time;
            }
        }
        else if (halfc->stat[obs->sat-1][i]&&obs->L[i]!=0.0&&
                 (p=halfc->data[obs->sat-1][i])) {
            
            if (obs->LLI[i]&LLI_HALFA) {
                p->stat=2; /* resolved with half-cycle added */
            }
            else if (obs->LLI[i]&LLI_HALFS) {
                p->stat=3; /* resolved with half-cycle subtracted */
            }
            else {
                p->stat=1; /* resolved wihout half-cycle added */
            }
        }
    }
}
/* dump half-cycle ambiguity status ------------------------------------------*/
static void dump_halfc(halfc_t *halfc)
{
    halfd_t *p;
    char s0[32],s1[32],s2[32];
    int i,j;
    
    trace(TL_HALFC,"HALF-CYC AMBIGUITY STATUS\n");
    
    for (i=0;i<MAXSAT;i++) for (j=0;j<NFREQ+NEXOBS;j++) {
        for (p=halfc->data[i][j];p;p=p->next) {
            satno2id(i+1,s0);
            time2str(p->ts,s1,2);
            time2str(p->te,s2,2);
            trace(TL_HALFC,"%s L%d : %s - %s : %d\n",s0,j+1,s1,s2,p->stat);
        }
    }
}
/* resolve half-cycle ambiguity ----------------------------------------------*/
static void resolve_halfc(halfc_t *halfc, obsd_t *obs)
{
    halfd_t *p;
    int i;
    
    for (i=0;i<NFREQ+NEXOBS;i++) {
        for (p=halfc->data[obs->sat-1][i];p;p=p->next) {
            if (!p->stat) continue;
            if (timediff(obs->time,p->ts)<-DTTOL||
                timediff(obs->time,p->te)> DTTOL) continue;
            
            if (p->stat==2) {
                obs->L[i]-=0.5;
            }
            else if (p->stat==3) {
                obs->L[i]+=0.5;
            }
            obs->LLI[i]&=~LLI_HALFC;
        }
    }
}
/* scan observation types and station parameters -----------------------------*/
static int scan_obstype(int format, char **files, int nf, rnxopt_t *opt,
                        stas_t **stas, halfc_t *halfc, gtime_t *time)
{
    strfile_t *str;
    unsigned char codes[NSATSYS][33]={{0}};
    unsigned char types[NSATSYS][33]={{0}};
    char msg[128];
    int i,j,k,l,m,c=0,type,sys,abort=0,n[NSATSYS]={0};
    
    trace(3,"scan_obstype: nf=%d, opt=%s\n",nf,opt);
    
    if (!(str=gen_strfile(format,opt->rcvopt,*time))) return 0;
    
    for (m=0;m<nf&&!abort;m++) {
        
        if (!open_strfile(str,files[m])) {
            continue;
        }
        /* scan codes and station info in input file */
        while ((type=input_strfile(str))>=-1) {
            
            if (type==5) { /* update station list */
                update_stas(stas,str);
                continue;
            }
            if (type!=1||str->obs->n<=0) continue;
            
            if (!opt->ts.time||timediff(str->obs->data[0].time,opt->ts)>=0.001) {
                 
                for (i=0;i<str->obs->n;i++) {
                    sys=satsys(str->obs->data[i].sat,NULL);
                    for (l=0;navsys[l];l++) if (navsys[l]==sys) break;
                    if (!navsys[l]) continue;
                    
                    for (j=0;j<NFREQ+NEXOBS;j++) {
                        if (!str->obs->data[i].code[j]) continue;
                        
                        for (k=0;k<n[l];k++) {
                            if (codes[l][k]==str->obs->data[i].code[j]) break;
                        }
                        if (k>=n[l]&&n[l]<32) {
                            codes[l][n[l]++]=str->obs->data[i].code[j];
                        }
                        if (k<n[l]) {
                            if (str->obs->data[i].P[j]!=0.0) types[l][k]|=1;
                            if (str->obs->data[i].L[j]!=0.0) types[l][k]|=2;
                            if (str->obs->data[i].D[j]!=0.0) types[l][k]|=4;
                            if (str->obs->data[i].SNR[j]!=0) types[l][k]|=8;
                        }
                    }
                    /* update half-cycle ambiguity status */
                    update_halfc(halfc,str->obs->data+i);
                }
                if (!time->time) *time=str->obs->data[0].time;
            }
            if (opt->te.time&&timediff(str->obs->data[0].time,opt->te)>10.0) break;
            
            if (++c%11) continue;
            
            sprintf(msg,"scanning: %s %s%s%s%s%s%s%s",time_str(str->time,0),
                    n[0]?"G":"",n[1]?"R":"",n[2]?"E":"",n[3]?"J":"",
                    n[4]?"S":"",n[5]?"C":"",n[6]?"I":"");
            if ((abort=showmsg(msg))) break;
        }
        close_strfile(str);
    }
    free_strfile(str);
    
    showmsg("");
    
    if (abort) {
        trace(2,"aborted in scan\n");
        return 0;
    }
    for (i=0;i<NSATSYS;i++) for (j=0;j<n[i];j++) {
        trace(2,"scan_obstype: sys=%d code=%s type=%d\n",i,code2obs(codes[i][j],NULL),types[i][j]);
    }
    for (i=0;i<NSATSYS;i++) {
        
        /* sort codes */
        sort_codes(codes[i],types[i],n[i]);
        
        /* set observation types in rinex option */
        setopt_obstype(codes[i],types[i],i,opt);
        
        for (j=0;j<n[i];j++) {
            trace(3,"scan_obstype: sys=%d code=%s\n",i,code2obs(codes[i][j],NULL));
        }
    }
    return 1;
}
/* set observation types -----------------------------------------------------*/
static void set_obstype(int format, rnxopt_t *opt)
{
    /* default supported codes for {GPS,GLO,GAL,QZS,SBS,CMP,IRN} */
    static const unsigned char codes_rtcm2[NSATSYS][8]={ /* rtcm2 */
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P},
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P},
        {0},
        {0},
        {0},
        {0},
        {0}
    };
    static const unsigned char codes_rtcm3[NSATSYS][8]={ /* rtcm3 */
        {CODE_L1C,CODE_L1W,CODE_L2W,CODE_L2X,CODE_L5X},
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P},
        {CODE_L1X,CODE_L5X,CODE_L7X,CODE_L8X},
        {CODE_L1C,CODE_L2X,CODE_L5X},
        {CODE_L1C,CODE_L5X},
        {CODE_L1I,CODE_L7I},
        {0}
    };
    static const unsigned char codes_oem3[NSATSYS][8]={ /* novatel oem3 */
        {CODE_L1C,CODE_L2P},
        {0},
        {0},
        {0},
        {CODE_L1C},
        {0},
        {0}
    };
    static const unsigned char codes_oem4[NSATSYS][8]={ /* novatel oem6 */
        {CODE_L1C,CODE_L1P,CODE_L2D,CODE_L2X,CODE_L5Q},
        {CODE_L1C,CODE_L2C,CODE_L2P},
        {CODE_L1B,CODE_L1C,CODE_L5Q,CODE_L7Q,CODE_L8Q},
        {CODE_L1C,CODE_L2X,CODE_L5Q},
        {CODE_L1C,CODE_L5I},
        {CODE_L1I,CODE_L7I},
        {0}
    };
    static const unsigned char codes_cres[NSATSYS][8]={ /* hemisphere */
        {CODE_L1C,CODE_L2P},
        {CODE_L1C,CODE_L2P},
        {0},
        {0},
        {CODE_L1C},
        {0},
        {0}
    };
    static const unsigned char codes_javad[NSATSYS][8]={ /* javad */
        {CODE_L1C,CODE_L1W,CODE_L1X,CODE_L2X,CODE_L2W,CODE_L5X},
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P},
        {CODE_L1X,CODE_L5X,CODE_L7X,CODE_L8X,CODE_L6X},
        {CODE_L1C,CODE_L1X,CODE_L1Z,CODE_L2X,CODE_L5X,CODE_L6X},
        {CODE_L1C,CODE_L5X},
        {CODE_L1I,CODE_L7I},
        {0}
    };
    static const unsigned char codes_rinex[NSATSYS][32]={ /* rinex and binex */
        {CODE_L1C,CODE_L1P,CODE_L1W,CODE_L1Y,CODE_L1M,CODE_L1N,CODE_L1S,CODE_L1L,
         CODE_L2C,CODE_L2D,CODE_L2S,CODE_L2L,CODE_L2X,CODE_L2P,CODE_L2W,CODE_L2Y,
         CODE_L2M,CODE_L2N,CODE_L5I,CODE_L5Q,CODE_L5X},
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P,CODE_L3I,CODE_L3Q,CODE_L3X},
        {CODE_L1C,CODE_L1A,CODE_L1B,CODE_L1X,CODE_L1Z,CODE_L5I,CODE_L5Q,CODE_L5X,
         CODE_L6A,CODE_L6B,CODE_L6C,CODE_L6X,CODE_L6Z,CODE_L7I,CODE_L7Q,CODE_L7X,
         CODE_L8I,CODE_L8Q,CODE_L8X},
        {CODE_L1C,CODE_L1S,CODE_L1L,CODE_L1X,CODE_L1Z,CODE_L2S,CODE_L2L,CODE_L2X,
         CODE_L5I,CODE_L5Q,CODE_L5X,CODE_L6S,CODE_L6L,CODE_L6X},
        {CODE_L1C,CODE_L5I,CODE_L5Q,CODE_L5X},
        {CODE_L1I,CODE_L1Q,CODE_L1X,CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L6I,CODE_L6Q,
         CODE_L6X},
        {CODE_L5A,CODE_L5B,CODE_L5C,CODE_L5X,CODE_L9A,CODE_L9B,CODE_L9C,CODE_L9X}
    };
    static const unsigned char codes_rt17[NSATSYS][8]={ /* rt17 */
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P,CODE_L2W},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
    };
    static const unsigned char codes_cmr[NSATSYS][8]={ /* cmr */
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P,CODE_L2W},
        {CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P},
        {0},
        {0},
        {0},
        {0},
        {0},
    };
    static const unsigned char codes_other[NSATSYS][8]={ /* others */
        {CODE_L1C},
        {CODE_L1C},
        {CODE_L1C},
        {CODE_L1C},
        {CODE_L1C},
        {CODE_L1I},
        {0}
    };
    const unsigned char *codes;
    int i;
    
    trace(3,"set_obstype: format=%d\n",format);
    
    for (i=0;i<NSATSYS;i++) {
        switch (format) {
            case STRFMT_RTCM2: codes=codes_rtcm2[i]; break;
            case STRFMT_RTCM3: codes=codes_rtcm3[i]; break;
            case STRFMT_OEM4 : codes=codes_oem4 [i]; break;
            case STRFMT_OEM3 : codes=codes_oem3 [i]; break;
            case STRFMT_CRES : codes=codes_cres [i]; break;
            case STRFMT_JAVAD: codes=codes_javad[i]; break;
            case STRFMT_BINEX: codes=codes_rinex[i]; break;
            case STRFMT_RT17 : codes=codes_rt17 [i]; break;
            case STRFMT_CMR  : codes=codes_cmr  [i]; break;
            case STRFMT_RINEX: codes=codes_rinex[i]; break;
            default:           codes=codes_other[i]; break;
        }
        /* set observation types in rinex option */
        setopt_obstype(codes,NULL,i,opt);
    }
}
/* save slip conditions ------------------------------------------------------*/
static void saveslips(unsigned char slips[][NFREQ+NEXOBS], obsd_t *data, int n)
{
    int i,j;
    
    for (i=0;i<n;i++) for (j=0;j<NFREQ+NEXOBS;j++) {
        if (data[i].LLI[j]&1) slips[data[i].sat-1][j]|=1;
    }
}
/* restore slip conditions ---------------------------------------------------*/
static void restslips(unsigned char slips[][NFREQ+NEXOBS], obsd_t *data, int n)
{
    int i,j;
    
    for (i=0;i<n;i++) for (j=0;j<NFREQ+NEXOBS;j++) {
        if (slips[data[i].sat-1][j]&1) data[i].LLI[j]|=1;
        slips[data[i].sat-1][j]=0;
    }
}
/* open output files ---------------------------------------------------------*/
static int openfile(FILE **ofp, char *files[], const char *file,
                    const rnxopt_t *opt, nav_t *nav)
{
    char path[1024];
    int i;
    
    trace(3,"openfile:\n");
    
    for (i=0;i<NOUTFILE;i++) {
        
        if (!*files[i]) continue;
        
        strcpy(path,files[i]);
        
        /* check overwrite input file and modify output file */
        if (!strcmp(path,file)) strcat(path,"_");
        
        /* create directory if not exist */
        createdir(path);
        
        if (!(ofp[i]=fopen(path,"w"))) {
            showmsg("file open error: %s",path);
            for (i--;i>=0;i--) if (ofp[i]) fclose(ofp[i]);
            return 0;
        }
        /* write header to file */
        switch (i) {
            case 0: outrnxobsh (ofp[0],opt,nav); break;
            case 1: outrnxnavh (ofp[1],opt,nav); break;
            case 2: outrnxgnavh(ofp[2],opt,nav); break;
            case 3: outrnxhnavh(ofp[3],opt,nav); break;
            case 4: outrnxqnavh(ofp[4],opt,nav); break;
            case 5: outrnxlnavh(ofp[5],opt,nav); break;
            case 6: outrnxcnavh(ofp[6],opt,nav); break;
            case 7: outrnxinavh(ofp[7],opt,nav); break;
        }
    }
    return 1;
}
/* close output files --------------------------------------------------------*/
static void closefile(FILE **ofp, const rnxopt_t *opt, nav_t *nav)
{
    int i;
    
    trace(3,"closefile:\n");
    
    for (i=0;i<NOUTFILE;i++) {
        
        if (!ofp[i]) continue;
        
        /* rewrite header to file */
        rewind(ofp[i]);
        switch (i) {
            case 0: outrnxobsh (ofp[0],opt,nav); break;
            case 1: outrnxnavh (ofp[1],opt,nav); break;
            case 2: outrnxgnavh(ofp[2],opt,nav); break;
            case 3: outrnxhnavh(ofp[3],opt,nav); break;
            case 4: outrnxqnavh(ofp[4],opt,nav); break;
            case 5: outrnxlnavh(ofp[5],opt,nav); break;
            case 6: outrnxlnavh(ofp[6],opt,nav); break;
            case 7: outrnxlnavh(ofp[7],opt,nav); break;
        }
        fclose(ofp[i]);
    }
}
/* output rinex event --------------------------------------------------------*/
static void outrnxevent(FILE *fp, rnxopt_t *opt, int staid, stas_t *stas)
{
    stas_t *p;
    double pos[3],enu[3],del[3];
    
    trace(2,"outrnxevent: staid=%d\n",staid);
    
    for (p=stas;p;p=p->next) {
        if (p->staid==staid) break;
    }
    fprintf(fp,"%31s%d%3d\n","",3,p?6:2); /* new site occupation event */
    fprintf(fp,"station ID: %4d%44s%-20s\n",staid,"","COMMENT");
    fprintf(fp,"%04d%-56s%-20s\n",staid,"","MARKER NAME");
    if (!p) {
        return;
    }
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",p->sta.recsno,
            p->sta.rectype,p->sta.recver,"REC # / TYPE / VERS");
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",p->sta.antsno,
            p->sta.antdes,"","ANT # / TYPE");
    fprintf(fp,"%14.4f%14.4f%14.4f%-18s%-20s\n",p->sta.pos[0],
            p->sta.pos[1],p->sta.pos[2],"","APPROX POSITION XYZ");
    
    /* antenna delta */
    if (norm(p->sta.del,3)>0.0) {
        if (!p->sta.deltype&&norm(p->sta.del,3)>0.0) { /* enu */
            del[0]=p->sta.del[2]; /* h */
            del[1]=p->sta.del[0]; /* e */
            del[2]=p->sta.del[1]; /* n */
        }
        else if (norm(p->sta.pos,3)>0.0) { /* xyz */
            ecef2pos(p->sta.pos,pos);
            ecef2enu(pos,p->sta.del,enu);
            del[0]=enu[2]; /* h */
            del[1]=enu[0]; /* e */
            del[2]=enu[1]; /* n */
        }
    }
    else {
        del[0]=p->sta.hgt;
        del[1]=0.0;
        del[2]=0.0;
    }
    fprintf(fp,"%14.4f%14.4f%14.4f%-18s%-20s\n",del[0],del[1],del[2],"",
            "ANTENNA: DELTA H/E/N");
}
/* screen time with time tolerance -------------------------------------------*/
static int screent_ttol(gtime_t time, gtime_t ts, gtime_t te, double tint,
                        double ttol)
{
    if (ttol==0.0) ttol=DTTOL;
    
    return (tint<=0.0||fmod(time2gpst(time,NULL)+DTTOL,tint)<=ttol*2.0)&&
           (ts.time==0||timediff(time,ts)>=-ttol)&&
           (te.time==0||timediff(time,te)<  ttol);
}
/* convert obs message -------------------------------------------------------*/
static void convobs(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *staid,
                    stas_t *stas, halfc_t *halfc, int *n,
                    unsigned char slips[][NFREQ+NEXOBS])
{
    gtime_t time;
    int i;
    
    trace(3,"convobs :\n");
    
    if (!ofp[0]||str->obs->n<=0) return;
    
    time=str->obs->data[0].time;
    
    if (opt->ts.time!=0&&timediff(time,opt->ts)<-opt->ttol) return;
    
    /* save slips */
    saveslips(slips,str->obs->data,str->obs->n);
    
    if (!screent_ttol(time,opt->ts,opt->te,opt->tint,opt->ttol)) return;
    
    /* restore slips */
    restslips(slips,str->obs->data,str->obs->n);
    
    /* output event if station changed */
    if ((str->format==STRFMT_RTCM2||str->format==STRFMT_RTCM3)&&
        str->rtcm.staid!=*staid) {
        if (*staid>=0) {
            outrnxevent(ofp[0],opt,str->rtcm.staid,stas);
        }
        *staid=str->rtcm.staid;
    }
    /* half-cycle ambiguity correction */
    if (opt->halfcyc) {
        for (i=0;i<str->obs->n;i++) {
            resolve_halfc(halfc,str->obs->data+i);
        }
    }
    /* output rinex obs */
    outrnxobsb(ofp[0],opt,str->obs->data,str->obs->n,0);
    
    if (opt->tstart.time==0) opt->tstart=time;
    opt->tend=time;
    
    n[0]++;
}
/* convert nav message -------------------------------------------------------*/
static void convnav(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *n)
{
    gtime_t ts1,te1,ts2,te2;
    int sys,prn,sep_nav=opt->rnxver<=2.99||opt->sep_nav;
    
    trace(3,"convnav :\n");
    
    ts1=opt->ts; if (ts1.time!=0) ts1=timeadd(ts1,-MAXDTOE);
    te1=opt->te; if (te1.time!=0) te1=timeadd(te1, MAXDTOE);
    ts2=opt->ts; if (ts2.time!=0) ts2=timeadd(ts2,-MAXDTOE_GLO);
    te2=opt->te; if (te2.time!=0) te2=timeadd(te2, MAXDTOE_GLO);
    
    sys=satsys(str->sat,&prn)&opt->navsys;
    
    if (sys==SYS_GPS) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
    }
    else if (sys==SYS_GLO) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts2,te2,0.0)) return;
        
        if (ofp[1]&&!sep_nav) {
            
            /* output rinex nav */
            outrnxgnavb(ofp[1],opt,str->nav->geph+prn-1);
            n[1]++;
        }
        if (ofp[2]&&sep_nav) {
            
            /* output rinex gnav */
            outrnxgnavb(ofp[2],opt,str->nav->geph+prn-1);
            n[2]++;
        }
    }
    else if (sys==SYS_SBS) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&!sep_nav) {
            
            /* output rinex nav */
            outrnxhnavb(ofp[1],opt,str->nav->seph+prn-MINPRNSBS);
            n[1]++;
        }
        if (ofp[3]&&sep_nav) {
            
            /* output rinex hnav */
            outrnxhnavb(ofp[3],opt,str->nav->seph+prn-MINPRNSBS);
            n[3]++;
        }
    }
    else if (sys==SYS_QZS) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&!sep_nav) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
        if (ofp[4]&&sep_nav) {
            
            /* output rinex qnav */
            outrnxnavb(ofp[4],opt,str->nav->eph+str->sat-1);
            n[4]++;
        }
    }
    else if (sys==SYS_GAL) {
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&!sep_nav) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
        if (ofp[5]&&sep_nav) {
            
            /* output rinex lnav */
            outrnxnavb(ofp[5],opt,str->nav->eph+str->sat-1);
            n[5]++;
        }
    }
    else if (sys==SYS_CMP) {
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&!sep_nav) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
        if (ofp[6]&&sep_nav) {
            
            /* output rinex cnav */
            outrnxnavb(ofp[6],opt,str->nav->eph+str->sat-1);
            n[6]++;
        }
    }
    else if (sys==SYS_IRN) {
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&!sep_nav) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
        if (ofp[7]&&sep_nav) {
            
            /* output rinex inav */
            outrnxnavb(ofp[7],opt,str->nav->eph+str->sat-1);
            n[7]++;
        }
    }
}
/* convert sbas message ------------------------------------------------------*/
static void convsbs(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *n)
{
    gtime_t ts1,te1;
    int msg,prn,sat,sys,sep_nav=opt->rnxver<=2.99||opt->sep_nav;
    
    trace(3,"convsbs :\n");
    
    ts1=opt->ts; if (ts1.time!=0) ts1=timeadd(ts1,-MAXDTOE);
    te1=opt->te; if (te1.time!=0) te1=timeadd(te1, MAXDTOE);
    
    msg=sbsupdatecorr(&str->raw.sbsmsg,str->nav);
    
    prn=str->raw.sbsmsg.prn;
    if      (MINPRNSBS<=prn&&prn<=MAXPRNSBS) sys=SYS_SBS;
    else if (MINPRNQZS<=prn&&prn<=MAXPRNQZS) sys=SYS_QZS;
    else {
        trace(2,"sbas message satellite error: prn=%d\n",prn);
        return;
    }
    if (!(sat=satno(sys,prn))||opt->exsats[sat-1]==1) return;
    
    if (ofp[NOUTFILE-1]) { /* output sbas log */
        if (screent(gpst2time(str->raw.sbsmsg.week,str->raw.sbsmsg.tow),opt->ts,
                    opt->te,0.0)) {
            sbsoutmsg(ofp[NOUTFILE-1],&str->raw.sbsmsg); n[NOUTFILE-1]++;
        }
    }
    if (!(opt->navsys&SYS_SBS)||msg!=9||
        !screent(str->time,ts1,te1,0.0)) return;
    
    if (ofp[1]&&!sep_nav) {
        
        /* output rinex nav */
        outrnxhnavb(ofp[1],opt,str->nav->seph+prn-MINPRNSBS);
        n[1]++;
    }
    if (ofp[3]&&sep_nav) {
        
        /* output rinex hnav */
        outrnxhnavb(ofp[3],opt,str->nav->seph+prn-MINPRNSBS);
        n[3]++;
    }
}
/* convert lex message -------------------------------------------------------*/
static void convlex(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *n)
{
    gtime_t ts1,te1;
    
    trace(3,"convlex :\n");
    
    ts1=opt->ts; if (ts1.time!=0) ts1=timeadd(ts1,-MAXDTOE);
    te1=opt->te; if (te1.time!=0) te1=timeadd(te1, MAXDTOE);
    
    if (ofp[NOUTFILE-1]&&screent(str->time,opt->ts,opt->te,0.0)) {
        lexoutmsg(ofp[NOUTFILE-1],&str->raw.lexmsg); n[NOUTFILE-1]++;
    }
}
/* set approx position -------------------------------------------------------*/
static void setapppos(strfile_t *str, rnxopt_t *opt)
{
    prcopt_t prcopt=prcopt_default;
    sol_t sol={{0}};
    char msg[128];
    
    prcopt.navsys=opt->navsys;
    
    /* point positioning with last obs data */
    if (!pntpos(str->obs->data,str->obs->n,str->nav,&prcopt,&sol,NULL,NULL,
                msg)) {
        trace(2,"point position error (%s)\n",msg);
        return;
    }
    matcpy(opt->apppos,sol.rr,3,1);
}
/* show status message -------------------------------------------------------*/
static int showstat(int sess, gtime_t ts, gtime_t te, int *n)
{
    const char type[]="ONGHQLCISE";
    char msg[1024]="",*p=msg,s[64];
    int i;
    
    if (sess>0) {
        p+=sprintf(p,"(%d) ",sess);
    }
    if (ts.time!=0) {
        time2str(ts,s,0);
        p+=sprintf(p,"%s",s);
    }
    if (te.time!=0&&timediff(te,ts)>0.9) {
        time2str(te,s,0);
        p+=sprintf(p,"-%s",s+5);
    }
    p+=sprintf(p,": ");
    
    for (i=0;i<NOUTFILE+1;i++) {
        if (n[i]==0) continue;
        p+=sprintf(p,"%c=%d%s",type[i],n[i],i<NOUTFILE?" ":"");
    }
    return showmsg(msg);
}
/* rinex converter for single-session ----------------------------------------*/
static int convrnx_s(int sess, int format, rnxopt_t *opt, const char *file,
                     char **ofile)
{
    FILE *ofp[NOUTFILE]={NULL};
    strfile_t *str;
    stas_t *stas=NULL,*p,*next;
    halfc_t halfc={{{0}}};
    gtime_t ts={0},te={0},tend={0},time={0};
    unsigned char slips[MAXSAT][NFREQ+NEXOBS]={{0}};
    int i,j,nf,type,n[NOUTFILE+1]={0},staid=-1,abort=0;
    char path[1024],*paths[NOUTFILE],s[NOUTFILE][1024];
    char *epath[MAXEXFILE]={0},*staname=*opt->staid?opt->staid:"0000";
    
    trace(3,"convrnx_s: sess=%d format=%d file=%s ofile=%s %s %s %s %s %s %s %s %s\n",
          sess,format,file,ofile[0],ofile[1],ofile[2],ofile[3],ofile[4],
          ofile[5],ofile[6],ofile[7],ofile[8]);
    
    /* replace keywords in input file */
    if (reppath(file,path,opt->ts,staname,"")<0) {
        showmsg("no time for input file: %s",file);
        return 0;
    }
    /* expand wild-cards in input file */
    for (i=0;i<MAXEXFILE;i++) {
        if (!(epath[i]=(char *)malloc(1024))) {
            for (i=0;i<MAXEXFILE;i++) free(epath[i]);
            return 0;
        }
    }
    nf=expath(path,epath,MAXEXFILE);
    
    if (format==STRFMT_RTCM2||format==STRFMT_RTCM3) {
        time=opt->trtcm;
    }
    if (opt->scanobs) {
        
        /* scan observation types and station parameters */
        if (!scan_obstype(format,epath,nf,opt,&stas,&halfc,&time)) return 0;
    }
    else {
        /* set observation types by format */
        set_obstype(format,opt);
    }
    dump_halfc(&halfc);
    
    if (!(str=gen_strfile(format,opt->rcvopt,time))) {
        for (i=0;i<MAXEXFILE;i++) free(epath[i]);
        return 0;
    }
    time=opt->ts.time?opt->ts:(time.time?timeadd(time,TSTARTMARGIN):time);
    
    /* replace keywords in output file */
    for (i=0;i<NOUTFILE;i++) {
        paths[i]=s[i];
        if (reppath(ofile[i],paths[i],time,staname,"")<0) {
            showmsg("no time for output path: %s",ofile[i]);
            for (i=0;i<MAXEXFILE;i++) free(epath[i]);
            free_strfile(str);
            return 0;
        }
    }
    /* open output files */
    if (!openfile(ofp,paths,path,opt,str->nav)) {
        for (i=0;i<MAXEXFILE;i++) free(epath[i]);
        free_strfile(str);
        return 0;
    }
    for (i=0;i<nf&&!abort;i++) {
        
        /* open stream file */
        if (!open_strfile(str,epath[i])) continue;
        
        /* input message */
        for (j=0;(type=input_strfile(str))>=-1;j++) {
            
            if (j%11==1&&(abort=showstat(sess,te,te,n))) break;
            
            /* avioid duplicated if overlapped data */
            if (type==1&&tend.time&&timediff(str->time,tend)<=0.0) continue;
            
            /* convert message */
            switch (type) {
                case  1: convobs(ofp,opt,str,&staid,stas,&halfc,n,slips); break;
                case  2: convnav(ofp,opt,str,n); break;
                case  3: convsbs(ofp,opt,str,n); break;
                case 31: convlex(ofp,opt,str,n); break;
                case -1: n[NOUTFILE]++; break; /* error */
            }
            te=str->time; if (ts.time==0) ts=te;
            
            /* set approx position */
            if (type==1&&!opt->autopos&&norm(opt->apppos,3)<=0.0) {
                setapppos(str,opt);
            }
            if (opt->te.time&&timediff(te,opt->te)>=-opt->ttol) break;
        }
        /* close stream file */
        close_strfile(str);
        
        tend=te; /* end time of a file */
    }
    /* set receiver and antenna information to option */
    if (format==STRFMT_RTCM2||format==STRFMT_RTCM3) {
        rtcm2opt(&str->rtcm,stas,opt);
    }
    else if (format==STRFMT_RINEX) {
        rnx2opt(&str->rnx,opt);
    }
    else if (format==STRFMT_CMR) {
        raw2opt(&str->raw,opt);
    }
    /* close output files */
    closefile(ofp,opt,str->nav);
    
    /* remove empty output files */
    for (i=0;i<NOUTFILE;i++) {
        if (ofp[i]&&n[i]<=0) remove(ofile[i]);
    }
    if (ts.time>0) showstat(sess,ts,te,n);
    
    for (p=stas;p;p=next) {
        next=p->next;
        free(p);
    }
    free_strfile(str);
    
    for (i=0;i<MAXEXFILE;i++) free(epath[i]);
    
    if (opt->tstart.time==0) opt->tstart=opt->ts;
    if (opt->tend  .time==0) opt->tend  =opt->te;
    
    return abort?-1:1;
}
/* rinex converter -------------------------------------------------------------
* convert receiver log file to rinex obs/nav, sbas log files
* args   : int    format I      receiver raw format (STRFMT_???)
*          rnxopt_t *opt IO     rinex options (see below)
*          char   *file  I      rtcm, receiver raw or rinex file
*                               (wild-cards (*) are expanded)
*          char   **ofile IO    output files
*                               ofile[0] rinex obs file   ("": no output)
*                               ofile[1] rinex nav file   ("": no output)
*                               ofile[2] rinex gnav file  ("": no output)
*                               ofile[3] rinex hnav file  ("": no output)
*                               ofile[4] rinex qnav file  ("": no output)
*                               ofile[5] rinex lnav file  ("": no output)
*                               ofile[6] rinex cnav file  ("": no output)
*                               ofile[7] rinex inav file  ("": no output)
*                               ofile[8] sbas/lex log file("": no output)
* return : status (1:ok,0:error,-1:abort)
* notes  : the following members of opt are replaced by information in last
*          converted rinex: opt->tstart, opt->tend, opt->obstype, opt->nobs
*          keywords in ofile[] are replaced by first obs date/time and station
*          id (%r)
*          the order of wild-card expanded files must be in-order by time
*-----------------------------------------------------------------------------*/
extern int convrnx(int format, rnxopt_t *opt, const char *file, char **ofile)
{
    gtime_t t0={0};
    rnxopt_t opt_=*opt;
    double tu,ts;
    int i,week,stat=1;
    
    trace(3,"convrnx: format=%d file=%s ofile=%s %s %s %s %s %s %s %s %s\n",
          format,file,ofile[0],ofile[1],ofile[2],ofile[3],ofile[4],ofile[5],
          ofile[6],ofile[7],ofile[8]);
    
    showmsg("");
    
    if (opt->ts.time==0||opt->te.time==0||opt->tunit<=0.0) {
        
        /* single-session */
        opt_.tstart=opt_.tend=t0;
        stat=convrnx_s(0,format,&opt_,file,ofile);
    }
    else if (timediff(opt->ts,opt->te)<=0.0) {
        
        /* multiple-session */
        tu=opt->tunit<86400.0?opt->tunit:86400.0;
        ts=tu*(int)floor(time2gpst(opt->ts,&week)/tu);
        
        for (i=0;;i++) { /* for each session */
            opt_.ts=gpst2time(week,ts+i*tu);
            opt_.te=timeadd(opt_.ts,tu-DTTOL-0.001);
            opt_.trtcm=timeadd(opt->trtcm,timediff(opt_.ts,opt->ts));
            if (timediff(opt_.ts,opt->te)>0.0) break;
            
            if (timediff(opt_.ts,opt->ts)<0.0) opt_.ts=opt->ts;
            if (timediff(opt_.te,opt->te)>0.0) opt_.te=opt->te;
            opt_.tstart=opt_.tend=t0;
            if ((stat=convrnx_s(i+1,format,&opt_,file,ofile))<0) break;
        }
    }
    else {
        showmsg("no period");
        return 0;
    }
    /* output start/end time */
    opt->tstart=opt_.tstart; opt->tend=opt_.tend;
    
    return stat;
}
