/*------------------------------------------------------------------------------
* preceph.c : precise ephemeris and clock functions
*
*          Copyright (C) 2007-2020 by T.TAKASU, All rights reserved.
*
* references :
*     [1] S.Hilla, The Extended Standard Product 3 Orbit Format (SP3-c),
*         12 February, 2007
*     [2] J.Ray, W.Gurtner, RINEX Extensions to Handle Clock Information,
*         27 August, 1998
*     [3] D.D.McCarthy, IERS Technical Note 21, IERS Conventions 1996, July 1996
*     [4] D.A.Vallado, Fundamentals of Astrodynamics and Applications 2nd ed,
*         Space Technology Library, 2004
*     [5] S.Hilla, The Extended Standard Product 3 Orbit Format (SP3-d),
*         February 21, 2016
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:48:06 $
* history : 2009/01/18 1.0  new
*           2009/01/31 1.1  fix bug on numerical error to read sp3a ephemeris
*           2009/05/15 1.2  support glonass,galileo,qzs
*           2009/12/11 1.3  support wild-card expansion of file path
*           2010/07/21 1.4  added api:
*                               eci2ecef(),sunmoonpos(),peph2pos(),satantoff(),
*                               readdcb()
*                           changed api:
*                               readsp3()
*                           deleted api:
*                               eph2posp()
*           2010/09/09 1.5  fix problem when precise clock outage
*           2011/01/23 1.6  support qzss satellite code
*           2011/09/12 1.7  fix problem on precise clock outage
*                           move sunmmonpos() to rtkcmn.c
*           2011/12/01 1.8  modify api readsp3()
*                           precede later ephemeris if ephemeris is NULL
*                           move eci2ecef() to rtkcmn.c
*           2013/05/08 1.9  fix bug on computing std-dev of precise clocks
*           2013/11/20 1.10 modify option for api readsp3()
*           2014/04/03 1.11 accept extenstion including sp3,eph,SP3,EPH
*           2014/05/23 1.12 add function to read sp3 velocity records
*                           change api: satantoff()
*           2014/08/31 1.13 add member cov and vco in peph_t sturct
*           2014/10/13 1.14 fix bug on clock error variance in peph2pos()
*           2015/05/10 1.15 add api readfcb()
*                           modify api readdcb()
*           2017/04/11 1.16 fix bug on antenna offset correction in peph2pos()
*           2020/11/30 1.17 support SP3-d [5] to accept more than 85 satellites
*                           support NavIC/IRNSS in API peph2pos()
*                           LC defined GPS/QZS L1-L2, GLO G1-G2, GAL E1-E5b,
*                            BDS B1I-B2I and IRN L5-S for API satantoff()
*                           fix bug on reading SP3 file extension
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define SQR(x)      ((x)*(x))

#define NMAX        10              /* order of polynomial interpolation */
#define MAXDTE      900.0           /* max time difference to ephem time (s) */
#define EXTERR_CLK  1E-3            /* extrapolation error for clock (m/s) */
#define EXTERR_EPH  5E-7            /* extrapolation error for ephem (m/s^2) */

/* satellite code to satellite system ----------------------------------------*/
static int code2sys(char code)
{
    if (code=='G'||code==' ') return SYS_GPS;
    if (code=='R') return SYS_GLO;
    if (code=='E') return SYS_GAL; /* SP3-d */
    if (code=='J') return SYS_QZS; /* SP3-d */
    if (code=='C') return SYS_CMP; /* SP3-d */
    if (code=='I') return SYS_IRN; /* SP3-d */
    if (code=='L') return SYS_LEO; /* SP3-d */
    return SYS_NONE;
}
/* read SP3 header -----------------------------------------------------------*/
static int readsp3h(FILE *fp, gtime_t *time, char *type, int *sats,
                    double *bfact, char *tsys)
{
    int i,j,k=0,ns=0,sys,prn;
    char buff[1024];
    
    trace(3,"readsp3h:\n");
    
    for (i=0;;i++) {
        if (!fgets(buff,sizeof(buff),fp)) break;
        
        if (i==0) {
            *type=buff[2];
            if (str2time(buff,3,28,time)) return 0;
        }
        else if (!strncmp(buff,"+ ",2)) { /* satellite id */
            if (ns==0) {
                ns=(int)str2num(buff,4,2);
            }
            for (j=0;j<17&&k<ns;j++) {
                sys=code2sys(buff[9+3*j]);
                prn=(int)str2num(buff,10+3*j,2);
                if (k<MAXSAT) sats[k++]=satno(sys,prn);
            }
        }
        else if (!strncmp(buff,"++",2)) { /* orbit accuracy */
            continue;
        }
        else if (!strncmp(buff,"%c",2)) { /* time system */
            strncpy(tsys,buff+9,3); tsys[3]='\0';
        }
        else if (!strncmp(buff,"%f",2)&&bfact[0]==0.0) { /* fp base number */
            bfact[0]=str2num(buff, 3,10);
            bfact[1]=str2num(buff,14,12);
        }
        else if (!strncmp(buff,"%i",2)) {
            continue;
        }
        else if (!strncmp(buff,"/*",2)) { /* comment */
            continue;
        }
        else if (!strncmp(buff,"* ",2)) { /* first record */
            /* roll back file pointer */
            fseek(fp,-(long)strlen(buff),SEEK_CUR);
            break;
        }
    }
    return ns;
}
/* add precise ephemeris -----------------------------------------------------*/
static int addpeph(nav_t *nav, peph_t *peph)
{
    peph_t *nav_peph;
    
    if (nav->ne>=nav->nemax) {
        nav->nemax+=256;
        if (!(nav_peph=(peph_t *)realloc(nav->peph,sizeof(peph_t)*nav->nemax))) {
            trace(1,"readsp3b malloc error n=%d\n",nav->nemax);
            free(nav->peph); nav->peph=NULL; nav->ne=nav->nemax=0;
            return 0;
        }
        nav->peph=nav_peph;
    }
    nav->peph[nav->ne++]=*peph;
    return 1;
}
/* read SP3 body -------------------------------------------------------------*/
static void readsp3b(FILE *fp, char type, int *sats, int ns, double *bfact,
                     char *tsys, int index, int opt, nav_t *nav)
{
    peph_t peph;
    gtime_t time;
    double val,std,base;
    int i,j,sat,sys,prn,n=ns*(type=='P'?1:2),pred_o,pred_c,v;
    char buff[1024];
    
    trace(3,"readsp3b: type=%c ns=%d index=%d opt=%d\n",type,ns,index,opt);
    
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (!strncmp(buff,"EOF",3)) break;
        
        if (buff[0]!='*'||str2time(buff,3,28,&time)) {
            trace(2,"sp3 invalid epoch %31.31s\n",buff);
            continue;
        }
        if (!strcmp(tsys,"UTC")) time=utc2gpst(time); /* utc->gpst */
        peph.time =time;
        peph.index=index;
        
        for (i=0;i<MAXSAT;i++) {
            for (j=0;j<4;j++) {
                peph.pos[i][j]=0.0;
                peph.std[i][j]=0.0f;
                peph.vel[i][j]=0.0;
                peph.vst[i][j]=0.0f;
            }
            for (j=0;j<3;j++) {
                peph.cov[i][j]=0.0f;
                peph.vco[i][j]=0.0f;
            }
        }
        for (i=pred_o=pred_c=v=0;i<n&&fgets(buff,sizeof(buff),fp);i++) {
            
            if (strlen(buff)<4||(buff[0]!='P'&&buff[0]!='V')) continue;
            
            sys=buff[1]==' '?SYS_GPS:code2sys(buff[1]);
            prn=(int)str2num(buff,2,2);
            if      (sys==SYS_SBS) prn+=100;
            else if (sys==SYS_QZS) prn+=192; /* extension to sp3-c */
            
            if (!(sat=satno(sys,prn))) continue;
            
            if (buff[0]=='P') {
                pred_c=strlen(buff)>=76&&buff[75]=='P';
                pred_o=strlen(buff)>=80&&buff[79]=='P';
            }
            for (j=0;j<4;j++) {
                
                /* read option for predicted value */
                if (j< 3&&(opt&1)&& pred_o) continue;
                if (j< 3&&(opt&2)&&!pred_o) continue;
                if (j==3&&(opt&1)&& pred_c) continue;
                if (j==3&&(opt&2)&&!pred_c) continue;
                
                val=str2num(buff, 4+j*14,14);
                std=str2num(buff,61+j* 3,j<3?2:3);
                
                if (buff[0]=='P') { /* position */
                    if (val!=0.0&&fabs(val-999999.999999)>=1E-6) {
                        peph.pos[sat-1][j]=val*(j<3?1000.0:1E-6);
                        v=1; /* valid epoch */
                    }
                    if ((base=bfact[j<3?0:1])>0.0&&std>0.0) {
                        peph.std[sat-1][j]=(float)(pow(base,std)*(j<3?1E-3:1E-12));
                    }
                }
                else if (v) { /* velocity */
                    if (val!=0.0&&fabs(val-999999.999999)>=1E-6) {
                        peph.vel[sat-1][j]=val*(j<3?0.1:1E-10);
                    }
                    if ((base=bfact[j<3?0:1])>0.0&&std>0.0) {
                        peph.vst[sat-1][j]=(float)(pow(base,std)*(j<3?1E-7:1E-16));
                    }
                }
            }
        }
        if (v) {
            if (!addpeph(nav,&peph)) return;
        }
    }
}
/* compare precise ephemeris -------------------------------------------------*/
static int cmppeph(const void *p1, const void *p2)
{
    peph_t *q1=(peph_t *)p1,*q2=(peph_t *)p2;
    double tt=timediff(q1->time,q2->time);
    return tt<-1E-9?-1:(tt>1E-9?1:q1->index-q2->index);
}
/* combine precise ephemeris -------------------------------------------------*/
static void combpeph(nav_t *nav, int opt)
{
    int i,j,k,m;
    
    trace(3,"combpeph: ne=%d\n",nav->ne);
    
    qsort(nav->peph,nav->ne,sizeof(peph_t),cmppeph);
    
    if (opt&4) return;
    
    for (i=0,j=1;j<nav->ne;j++) {
        
        if (fabs(timediff(nav->peph[i].time,nav->peph[j].time))<1E-9) {
            
            for (k=0;k<MAXSAT;k++) {
                if (norm(nav->peph[j].pos[k],4)<=0.0) continue;
                for (m=0;m<4;m++) nav->peph[i].pos[k][m]=nav->peph[j].pos[k][m];
                for (m=0;m<4;m++) nav->peph[i].std[k][m]=nav->peph[j].std[k][m];
                for (m=0;m<4;m++) nav->peph[i].vel[k][m]=nav->peph[j].vel[k][m];
                for (m=0;m<4;m++) nav->peph[i].vst[k][m]=nav->peph[j].vst[k][m];
            }
        }
        else if (++i<j) nav->peph[i]=nav->peph[j];
    }
    nav->ne=i+1;
    
    trace(4,"combpeph: ne=%d\n",nav->ne);
}
/* read sp3 precise ephemeris file ---------------------------------------------
* read sp3 precise ephemeris/clock files and set them to navigation data
* args   : char   *file       I   sp3-c precise ephemeris file
*                                 (wind-card * is expanded)
*          nav_t  *nav        IO  navigation data
*          int    opt         I   options (1: only observed + 2: only predicted +
*                                 4: not combined)
* return : none
* notes  : see ref [1]
*          precise ephemeris is appended and combined
*          nav->peph and nav->ne must by properly initialized before calling the
*          function
*          only files with extensions of .sp3, .SP3, .eph* and .EPH* are read
*-----------------------------------------------------------------------------*/
extern void readsp3(const char *file, nav_t *nav, int opt)
{
    FILE *fp;
    gtime_t time={0};
    double bfact[2]={0};
    int i,j,n,ns,sats[MAXSAT]={0};
    char *efiles[MAXEXFILE],*ext,type=' ',tsys[4]="";
    
    trace(3,"readpephs: file=%s\n",file);
    
    for (i=0;i<MAXEXFILE;i++) {
        if (!(efiles[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(efiles[i]);
            return;
        }
    }
    /* expand wild card in file path */
    n=expath(file,efiles,MAXEXFILE);
    
    for (i=j=0;i<n;i++) {
        if (!(ext=strrchr(efiles[i],'.'))) continue;
        
        if (!strstr(ext,".sp3")&&!strstr(ext,".SP3")&&
            !strstr(ext,".eph")&&!strstr(ext,".EPH")) continue;
        
        if (!(fp=fopen(efiles[i],"r"))) {
            trace(2,"sp3 file open error %s\n",efiles[i]);
            continue;
        }
        /* read sp3 header */
        ns=readsp3h(fp,&time,&type,sats,bfact,tsys);
        
        /* read sp3 body */
        readsp3b(fp,type,sats,ns,bfact,tsys,j++,opt,nav);
        
        fclose(fp);
    }
    for (i=0;i<MAXEXFILE;i++) free(efiles[i]);
    
    /* combine precise ephemeris */
    if (nav->ne>0) combpeph(nav,opt);
}
/* read satellite antenna parameters -------------------------------------------
* read satellite antenna parameters
* args   : char   *file       I   antenna parameter file
*          gtime_t time       I   time
*          nav_t  *nav        IO  navigation data
* return : status (1:ok,0:error)
* notes  : only support antex format for the antenna parameter file
*-----------------------------------------------------------------------------*/
extern int readsap(const char *file, gtime_t time, nav_t *nav)
{
    pcvs_t pcvs={0};
    pcv_t pcv0={0},*pcv;
    int i;
    
    trace(3,"readsap : file=%s time=%s\n",file,time_str(time,0));
    
    if (!readpcv(file,&pcvs)) return 0;
    
    for (i=0;i<MAXSAT;i++) {
        pcv=searchpcv(i+1,"",time,&pcvs);
        nav->pcvs[i]=pcv?*pcv:pcv0;
    }
    free(pcvs.pcv);
    return 1;
}
/* read DCB parameters file --------------------------------------------------*/
static int readdcbf(const char *file, nav_t *nav, const sta_t *sta)
{
    FILE *fp,*fpd;
    double cbias;
     /*
     str1 -> BIAS, str2 -> SVN, str3 -> PRN, str4 -> OBS1, 
     str5 -> OBS2, str6 -> BIAS_START, str7 -> BIAS_END, 
     str8 -> UNIT, str9 -> VALUE, str10 -> STD
     */
    char buff[2048],str1[32]="",str2[32]="",str3[32]="";
    char str4[32],str5[32]="",str6[32]="",str7[32],str8[32]="";
    char str9[32]="",str10[32]="",str11[32]="",target_code1[4]="",target_code2[4]="",target_sat[4]="",target_rcv[32]="";    
    int sat,start=0;

    

    trace(3,"readdcbf: file=%s\n",file);
    
    if (!(fp=fopen(file,"r"))) {
        trace(0,"dcb parameters file open error: %s\n",file);
        return 0;
    }


    
    while (fgets(buff,sizeof(buff),fp)) {
        if (strstr(buff, "*BIAS SVN_ PRN STATION__ OBS1 OBS2 BIAS_START____ BIAS_END______ UNIT __ESTIMATED_VALUE____ _STD_DEV___")) start=1;
        if (strstr(buff,"POINTS")) start=0;
        if (!start||sscanf(buff,"%s %s %s %3s %s %s %s %s %s %s",str1,str2,str3,str4,str5,str6,str7,str8,str9,str10)<0) continue;
        /*trace(3,"%s %s %s %3s %s %s %s %s %s %s \n\r",str1,str2,str3,str4,str5,str6,str7,str8,str9,str10);*/
        if (start == 2)
        {
            strcpy(target_code1, str4);
            strcpy(target_code2, str5);
            strcpy(target_sat, str3);
            if(!strcmp(target_code1, str4) && !strcmp(target_code2, str5))
            {
                cbias = atof(str9); /*DCB*/
                sat=satid2no(target_sat);
                nav->cbias[sat-1][codeconv(target_code1)][codeconv(target_code2)]=cbias*1E-9*CLIGHT;
            }   
        }
        start=2;
    }
    
    fclose(fp);

    if (!(fpd=fopen(file,"r"))) {
        trace(0,"dcb parameters file open error: %s\n",file);
        return 0;
    }

    while (fgets(buff,sizeof(buff),fpd)) {
        if (strstr(buff, "POINTS")) start=1;
        if (strstr(buff,"-BIAS/SOLUTION ")) start=0;
        if (!start||sscanf(buff,"%s %s %s %s %s %s %s %s %s %s %s",str1,str2,str3,str4,str5,str6,str7,str8,str9,str10,str11)<0)
        /*trace(3,"%s %s %s %s %s %s %s %s %s %s %s\n\r",str1,str2,str3,str4,str5,str6,str7,str8,str9,str10,str11);*/
        if (start == 2){
            if (!strcmp(station,str4))
            {
                strcpy(target_code1, str5);
                strcpy(target_code2, str6);
                strcpy(target_rcv, str4);
                if(!strcmp(target_code1, str5) && !strcmp(target_code2, str6))
                {
                    cbias = atof(str10); /*DCB*/
                    nav->rbias[codeconv(target_code1)][codeconv(target_code2)] = cbias*1E-9*CLIGHT;
                }
            }
        }
        if (start == 1) start = 2;
        
    }
    fclose(fpd);
    
    return 1;
}

/*convert observation code CHAR to NUM*/
int codeconv(char *obscode)
{
    int i;
    
    int codeL[] = {CODE_L1C,CODE_L1P,CODE_L1W,CODE_L1Y,CODE_L1M,CODE_L1N,CODE_L1S,CODE_L1L,CODE_L1E,CODE_L1A
    ,CODE_L1B,CODE_L1X,CODE_L1Z,CODE_L2C,CODE_L2D,CODE_L2S,CODE_L2L,CODE_L2X,CODE_L2P,CODE_L2W,CODE_L2Y,CODE_L2M
    ,CODE_L2N,CODE_L5I,CODE_L5Q,CODE_L5X,CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L6A,CODE_L6B,CODE_L6C,CODE_L6X,CODE_L6Z
    ,CODE_L6S,CODE_L6L,CODE_L8I,CODE_L8Q,CODE_L8X,CODE_L2I,CODE_L2Q,CODE_L6I,CODE_L6Q,CODE_L3I,CODE_L3Q,CODE_L3X
    ,CODE_L1I,CODE_L1Q,CODE_L5A,CODE_L5B,CODE_L5C,CODE_L9A,CODE_L9B,CODE_L9C,CODE_L9X,CODE_L1D,CODE_L5D,CODE_L5P
    ,CODE_L5Z,CODE_L6E,CODE_L7D,CODE_L7P,CODE_L7Z,CODE_L8D,CODE_L8P,CODE_L4A,CODE_L4B,CODE_L4X};
    char *codeC[] = {"C1C","C1P","C1W","C1Y","C1M","C1N","C1S","C1L","C1E","C1A","C1B","C1X","C1Z","C2C","C2D"
    ,"C2S","C2L","C2X","C2P","C2W","C2Y","C2M","C2N","C5I","C5Q","C5X","C7I","C7Q","C7X","C6A","C6B","C6C","C6X","C6Z","C6S"
    ,"C6L","C8I","C8Q","C8X","C2I","C2Q","C6I","C6Q","C3I","C3Q","C3X","C1I","C1Q","C5A","C5B","C5C","C9A","C9B"
    ,"C9C","C9X","C1D","C5D","C5P","C5Z","C6E","C7D","C7P","C7Z","C8D","C8P","C4A","C4B","C4X"};
      
    for (i = 0; i < 68; i++) if(!strcmp(codeC[i], obscode)) return codeL[i];

    return 0;
}

/* read DCB parameters ---------------------------------------------------------
* read differential code bias (DCB) parameters
* args   : char   *file       I   DCB parameters file (wild-card * expanded)
*          nav_t  *nav        IO  navigation data
*          sta_t  *sta        I   station info data to inport receiver DCB
*                                 (NULL: no use)
* return : status (1:ok,0:error)
* notes  : currently only support P1-P2, P1-C1, P2-C2, bias in DCB file
*-----------------------------------------------------------------------------*/
extern int readdcb(const char *file, nav_t *nav, const sta_t *sta)
{
    int i,j,k,n;
    char *efiles[MAXEXFILE]={0};
    
    trace(3,"readdcb : file=%s\n",file);
    
    for (i=0;i<MAXSAT;i++) for (j=0;j<MAXCODE;j++) for (k=0;k<MAXCODE;k++) {
        nav->cbias[i][j][k]=0.0;
    }
    for (i=0;i<MAXEXFILE;i++) {
        if (!(efiles[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(efiles[i]);
            return 0;
        }
    }
    n=expath(file,efiles,MAXEXFILE);
    
    for (i=0;i<n;i++) {
        readdcbf(efiles[i],nav,sta);
    }
    for (i=0;i<MAXEXFILE;i++) free(efiles[i]);
    dcbf = 1;
    return 1;
}
/* polynomial interpolation by Neville's algorithm ---------------------------*/
static double interppol(const double *x, double *y, int n)
{
    int i,j;
    
    for (j=1;j<n;j++) {
        for (i=0;i<n-j;i++) {
            y[i]=(x[i+j]*y[i]-x[i]*y[i+1])/(x[i+j]-x[i]);
        }
    }
    return y[0];
}
/* satellite position by precise ephemeris -----------------------------------*/
static int pephpos(gtime_t time, int sat, const nav_t *nav, double *rs,
                   double *dts, double *vare, double *varc)
{
    double t[NMAX+1],p[3][NMAX+1],c[2],*pos,std=0.0,s[3],sinl,cosl;
    int i,j,k,index;
    
    trace(4,"pephpos : time=%s sat=%2d\n",time_str(time,3),sat);
    
    rs[0]=rs[1]=rs[2]=dts[0]=0.0;
    
    if (nav->ne<NMAX+1||
        timediff(time,nav->peph[0].time)<-MAXDTE||
        timediff(time,nav->peph[nav->ne-1].time)>MAXDTE) {
        trace(3,"no prec ephem %s sat=%2d\n",time_str(time,0),sat);
        return 0;
    }
    /* binary search */
    for (i=0,j=nav->ne-1;i<j;) {
        k=(i+j)/2;
        if (timediff(nav->peph[k].time,time)<0.0) i=k+1; else j=k;
    }
    index=i<=0?0:i-1;
    
    /* polynomial interpolation for orbit */
    i=index-(NMAX+1)/2;
    if (i<0) i=0; else if (i+NMAX>=nav->ne) i=nav->ne-NMAX-1;
    
    for (j=0;j<=NMAX;j++) {
        t[j]=timediff(nav->peph[i+j].time,time);
        if (norm(nav->peph[i+j].pos[sat-1],3)<=0.0) {
            trace(3,"prec ephem outage %s sat=%2d\n",time_str(time,0),sat);
            return 0;
        }
    }
    for (j=0;j<=NMAX;j++) {
        pos=nav->peph[i+j].pos[sat-1];
        /* correciton for earh rotation ver.2.4.0 */
        sinl=sin(OMGE*t[j]);
        cosl=cos(OMGE*t[j]);
        p[0][j]=cosl*pos[0]-sinl*pos[1];
        p[1][j]=sinl*pos[0]+cosl*pos[1];
        p[2][j]=pos[2];
    }
    for (i=0;i<3;i++) {
        rs[i]=interppol(t,p[i],NMAX+1);
    }
    if (vare) {
        for (i=0;i<3;i++) s[i]=nav->peph[index].std[sat-1][i];
        std=norm(s,3);
        
        /* extrapolation error for orbit */
        if      (t[0   ]>0.0) std+=EXTERR_EPH*SQR(t[0   ])/2.0;
        else if (t[NMAX]<0.0) std+=EXTERR_EPH*SQR(t[NMAX])/2.0;
        *vare=SQR(std);
    }
    /* linear interpolation for clock */
    t[0]=timediff(time,nav->peph[index  ].time);
    t[1]=timediff(time,nav->peph[index+1].time);
    c[0]=nav->peph[index  ].pos[sat-1][3];
    c[1]=nav->peph[index+1].pos[sat-1][3];
    
    if (t[0]<=0.0) {
        if ((dts[0]=c[0])!=0.0) {
            std=nav->peph[index].std[sat-1][3]*CLIGHT-EXTERR_CLK*t[0];
        }
    }
    else if (t[1]>=0.0) {
        if ((dts[0]=c[1])!=0.0) {
            std=nav->peph[index+1].std[sat-1][3]*CLIGHT+EXTERR_CLK*t[1];
        }
    }
    else if (c[0]!=0.0&&c[1]!=0.0) {
        dts[0]=(c[1]*t[0]-c[0]*t[1])/(t[0]-t[1]);
        i=t[0]<-t[1]?0:1;
        std=nav->peph[index+i].std[sat-1][3]+EXTERR_CLK*fabs(t[i]);
    }
    else {
        dts[0]=0.0;
    }
    if (varc) *varc=SQR(std);
    return 1;
}
/* satellite clock by precise clock ------------------------------------------*/
static int pephclk(gtime_t time, int sat, const nav_t *nav, double *dts,
                   double *varc)
{
    double t[2],c[2],std;
    int i,j,k,index;
    
    trace(4,"pephclk : time=%s sat=%2d\n",time_str(time,3),sat);
    
    if (nav->nc<2||
        timediff(time,nav->pclk[0].time)<-MAXDTE||
        timediff(time,nav->pclk[nav->nc-1].time)>MAXDTE) {
        trace(3,"no prec clock %s sat=%2d\n",time_str(time,0),sat);
        return 1;
    }
    /* binary search */
    for (i=0,j=nav->nc-1;i<j;) {
        k=(i+j)/2;
        if (timediff(nav->pclk[k].time,time)<0.0) i=k+1; else j=k;
    }
    index=i<=0?0:i-1;
    
    /* linear interpolation for clock */
    t[0]=timediff(time,nav->pclk[index  ].time);
    t[1]=timediff(time,nav->pclk[index+1].time);
    c[0]=nav->pclk[index  ].clk[sat-1][0];
    c[1]=nav->pclk[index+1].clk[sat-1][0];
    
    if (t[0]<=0.0) {
        if ((dts[0]=c[0])==0.0) return 0;
        std=nav->pclk[index].std[sat-1][0]*CLIGHT-EXTERR_CLK*t[0];
    }
    else if (t[1]>=0.0) {
        if ((dts[0]=c[1])==0.0) return 0;
        std=nav->pclk[index+1].std[sat-1][0]*CLIGHT+EXTERR_CLK*t[1];
    }
    else if (c[0]!=0.0&&c[1]!=0.0) {
        dts[0]=(c[1]*t[0]-c[0]*t[1])/(t[0]-t[1]);
        i=t[0]<-t[1]?0:1;
        std=nav->pclk[index+i].std[sat-1][0]*CLIGHT+EXTERR_CLK*fabs(t[i]);
    }
    else {
        trace(3,"prec clock outage %s sat=%2d\n",time_str(time,0),sat);
        return 0;
    }
    if (varc) *varc=SQR(std);
    return 1;
}
/* satellite antenna phase center offset ---------------------------------------
* compute satellite antenna phase center offset in ecef
* args   : gtime_t time       I   time (gpst)
*          double *rs         I   satellite position and velocity (ecef)
*                                 {x,y,z,vx,vy,vz} (m|m/s)
*          int    sat         I   satellite number
*          nav_t  *nav        I   navigation data
*          double *dant       I   satellite antenna phase center offset (ecef)
*                                 {dx,dy,dz} (m) (iono-free LC value)
* return : none
* notes  : iono-free LC frequencies defined as follows:
*            GPS/QZSS : L1-L2
*            GLONASS  : G1-G2
*            Galileo  : E1-E5b
*            BDS      : B1I-B2I
*            NavIC    : L5-S
*-----------------------------------------------------------------------------*/
extern void satantoff(gtime_t time, const double *rs, int sat, const nav_t *nav,
                      double *dant)
{
    const pcv_t *pcv=nav->pcvs+sat-1;
    double ex[3],ey[3],ez[3],es[3],r[3],rsun[3],gmst,erpv[5]={0},freq[2];
    double C1,C2,dant1,dant2;
    int i,sys;
    
    trace(4,"satantoff: time=%s sat=%2d\n",time_str(time,3),sat);
    
    dant[0]=dant[1]=dant[2]=0.0;
    
    /* sun position in ecef */
    sunmoonpos(gpst2utc(time),erpv,rsun,NULL,&gmst);
    
    /* unit vectors of satellite fixed coordinates */
    for (i=0;i<3;i++) r[i]=-rs[i];
    if (!normv3(r,ez)) return;
    for (i=0;i<3;i++) r[i]=rsun[i]-rs[i];
    if (!normv3(r,es)) return;
    cross3(ez,es,r);
    if (!normv3(r,ey)) return;
    cross3(ey,ez,ex);
    
    /* iono-free LC coefficients */
    sys=satsys(sat,NULL);
    if (sys==SYS_GPS||sys==SYS_QZS) { /* L1-L2 */
        freq[0]=FREQ1;
        freq[1]=FREQ2;
    }
    else if (sys==SYS_GLO) { /* G1-G2 */
        freq[0]=sat2freq(sat,CODE_L1C,nav);
        freq[1]=sat2freq(sat,CODE_L2C,nav);
    }
    else if (sys==SYS_GAL) { /* E1-E5b */
        freq[0]=FREQ1;
        freq[1]=FREQ7;
    }
    else if (sys==SYS_CMP) { /* B1I-B2I */
        freq[0]=FREQ1_CMP;
        freq[1]=FREQ2_CMP;
    }
    else if (sys==SYS_IRN) { /* B1I-B2I */
        freq[0]=FREQ5;
        freq[1]=FREQ9;
    }
    else return;
    
    C1= SQR(freq[0])/(SQR(freq[0])-SQR(freq[1]));
    C2=-SQR(freq[1])/(SQR(freq[0])-SQR(freq[1]));
    
    /* iono-free LC */
    for (i=0;i<3;i++) {
        dant1=pcv->off[0][0]*ex[i]+pcv->off[0][1]*ey[i]+pcv->off[0][2]*ez[i];
        dant2=pcv->off[1][0]*ex[i]+pcv->off[1][1]*ey[i]+pcv->off[1][2]*ez[i];
        dant[i]=C1*dant1+C2*dant2;
    }
}
/* satellite position/clock by precise ephemeris/clock -------------------------
* compute satellite position/clock with precise ephemeris/clock
* args   : gtime_t time       I   time (gpst)
*          int    sat         I   satellite number
*          nav_t  *nav        I   navigation data
*          int    opt         I   sat postion option
*                                 (0: center of mass, 1: antenna phase center)
*          double *rs         O   sat position and velocity (ecef)
*                                 {x,y,z,vx,vy,vz} (m|m/s)
*          double *dts        O   sat clock {bias,drift} (s|s/s)
*          double *var        IO  sat position and clock error variance (m)
*                                 (NULL: no output)
* return : status (1:ok,0:error or data outage)
* notes  : clock includes relativistic correction but does not contain code bias
*          before calling the function, nav->peph, nav->ne, nav->pclk and
*          nav->nc must be set by calling readsp3(), readrnx() or readrnxt()
*          if precise clocks are not set, clocks in sp3 are used instead
*-----------------------------------------------------------------------------*/
extern int peph2pos(gtime_t time, int sat, const nav_t *nav, int opt,
                    double *rs, double *dts, double *var)
{
    gtime_t time_tt;
    double rss[3],rst[3],dtss[1],dtst[1],dant[3]={0},vare=0.0,varc=0.0,tt=1E-3;
    int i;
    
    trace(4,"peph2pos: time=%s sat=%2d opt=%d\n",time_str(time,3),sat,opt);
    
    if (sat<=0||MAXSAT<sat) return 0;
    
    /* satellite position and clock bias */
    if (!pephpos(time,sat,nav,rss,dtss,&vare,&varc)||
        !pephclk(time,sat,nav,dtss,&varc)) return 0;
    
    time_tt=timeadd(time,tt);
    if (!pephpos(time_tt,sat,nav,rst,dtst,NULL,NULL)||
        !pephclk(time_tt,sat,nav,dtst,NULL)) return 0;
    
    /* satellite antenna offset correction */
    if (opt) {
        satantoff(time,rss,sat,nav,dant);
    }
    for (i=0;i<3;i++) {
        rs[i  ]=rss[i]+dant[i];
        rs[i+3]=(rst[i]-rss[i])/tt;
    }
    /* relativistic effect correction */
    if (dtss[0]!=0.0) {
        dts[0]=dtss[0]-2.0*dot(rs,rs+3,3)/CLIGHT/CLIGHT;
        dts[1]=(dtst[0]-dtss[0])/tt;
    }
    else { /* no precise clock */
        dts[0]=dts[1]=0.0;
    }
    if (var) *var=vare+varc;
    
    return 1;
}
