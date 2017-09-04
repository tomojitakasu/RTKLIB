/*------------------------------------------------------------------------------
* qzslex.c : qzss lex functions
*
* references :
*     [1] IS-QZSS v.1.1, Quasi-Zenith Satellite System Navigation Service
*         Interface Specification for QZSS, Japan Aerospace Exploration Agency,
*         July 31, 2009
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:48:06 $
* history : 2011/05/27 1.0  new
*           2011/07/01 1.1  support 24bytes header format for lexconvbin()
*           2013/03/27 1.2  support message type 12
*           2013/05/11 1.3  fix bugs on decoding message type 12
*           2013/09/01 1.4  consolidate mt 12 handling codes provided by T.O.
*           2016/07/29 1.5  crc24q() -> rtk_crc24q()
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define LEXFRMLEN       2000            /* lex frame length (bits) */
#define LEXHDRLEN       49              /* lex header length (bits) */
#define LEXRSLEN        256             /* lex reed solomon length (bits) */
#define LEXFRMPREAMB    0x1ACFFC1Du     /* lex frame preamble */
#define LEXEPHMAXAGE    360.0           /* max age of lex ephemeris (s) */
#define LEXIONMAXAGE    3600.0          /* max age of lex ionos correction (s) */
#define RTCM3PREAMB     0xD3            /* rtcm ver.3 frame preamble */

#define LEXHEADLEN      24              /* lex binary header length (bytes) */

/* ura value -----------------------------------------------------------------*/
static double vareph(int ura)
{
    const double uraval[]={
        0.08,0.11,0.15,0.21,0.30,0.43,0.60,0.85,1.20,1.70,2.40,3.40,4.85,6.85,
        9.65,9.65
    };
    if (ura<0||15<ura) ura=15;
    return uraval[ura];
}
/* get signed 33bit field ----------------------------------------------------*/
static double getbits_33(const unsigned char *buff, int pos)
{
    return (double)getbits(buff,pos,32)*2.0+getbitu(buff,pos+32,1);
}
/* decode tof and toe field (ref [1] 5.7.2.2.1.1) ----------------------------*/
static int decode_lextof(const unsigned char *buff, int i, gtime_t *tof,
                         gtime_t *toe)
{
    double tt,toes;
    int tow,week;
    char s1[64],s2[64];
    
    trace(3,"decode_lextof:\n");
    
    tow =getbitu(buff,i,20);      i+=20;
    week=getbitu(buff,i,13);      i+=13;
    toes=getbitu(buff,i,16)*15.0; i+=16;
    *tof=gpst2time(week,tow);
    *toe=gpst2time(week,toes);
    
    trace(3,"decode_lextof: tow=%d week=%d toe=%d\n",tow,week,toes);
    
    tt=timediff(*toe,*tof);
    if      (tt<-302400.0) *toe=timeadd(*toe, 604800.0);
    else if (tt> 302400.0) *toe=timeadd(*toe,-604800.0);
    
    time2str(*tof,s1,3);
    time2str(*toe,s2,3);
    trace(4,"decode_lextof: tof=%s toe=%s\n",s1,s2);
    return i;
}
/* decode signal health field (ref [1] 5.7.2.2.1.1) --------------------------*/
static int decode_lexhealth(const unsigned char *buff, int i, gtime_t tof,
                            nav_t *nav)
{
    int j,sat;
    unsigned char health;
    
    trace(3,"decode_lexhealth: tof=%s\n",time_str(tof,0));
    
    for (j=0;j<35;j++) {
        health=getbitu(buff,i,5); i+= 5;
        
        if (j<3) sat=satno(SYS_QZS,j+193);
        else     sat=satno(SYS_GPS,j-2);
        if (!sat) continue;
        
        nav->lexeph[sat-1].tof=tof;
        nav->lexeph[sat-1].health=health;
        
        trace(4,"sat=%2d health=%d\n",sat,health);
    }
    return i;
}
/* decode ephemeris and sv clock field (ref [1] 5.7.2.2.1.2) -----------------*/
static int decode_lexeph(const unsigned char *buff, int i, gtime_t toe,
                         nav_t *nav)
{
    lexeph_t eph={{0}};
    gtime_t tof;
    unsigned char health;
    int j,prn,sat;
    
    trace(3,"decode_lexeph: toe=%s\n",time_str(toe,0));
    
    prn        =getbitu(buff,i, 8);       i+= 8;
    eph.ura    =getbitu(buff,i, 4);       i+= 4;
    eph.pos [0]=getbits_33(buff,i)*P2_6;  i+=33;
    eph.pos [1]=getbits_33(buff,i)*P2_6;  i+=33;
    eph.pos [2]=getbits_33(buff,i)*P2_6;  i+=33;
    eph.vel [0]=getbits(buff,i,28)*P2_15; i+=28;
    eph.vel [1]=getbits(buff,i,28)*P2_15; i+=28;
    eph.vel [2]=getbits(buff,i,28)*P2_15; i+=28;
    eph.acc [0]=getbits(buff,i,24)*P2_24; i+=24;
    eph.acc [1]=getbits(buff,i,24)*P2_24; i+=24;
    eph.acc [2]=getbits(buff,i,24)*P2_24; i+=24;
    eph.jerk[0]=getbits(buff,i,20)*P2_32; i+=20;
    eph.jerk[1]=getbits(buff,i,20)*P2_32; i+=20;
    eph.jerk[2]=getbits(buff,i,20)*P2_32; i+=20;
    eph.af0    =getbits(buff,i,26)*P2_35; i+=26;
    eph.af1    =getbits(buff,i,20)*P2_48; i+=20;
    eph.tgd    =getbits(buff,i,13)*P2_35; i+=13;
    for (j=0;j<7;j++) {
        eph.isc[j]=getbits(buff,i,13)*P2_35; i+=13;
    }
    if (prn==255) return i; /* no satellite */
    
    if      (  1<=prn&&prn<= 32) sat=satno(SYS_GPS,prn);
    else if (193<=prn&&prn<=195) sat=satno(SYS_QZS,prn);
    else {
        trace(2,"lex ephemeris prn error prn=%d\n",prn);
        return i;
    }
    eph.toe=toe;
    eph.sat=sat;
    tof   =nav->lexeph[sat-1].tof;
    health=nav->lexeph[sat-1].health;
    nav->lexeph[sat-1]=eph;
    nav->lexeph[sat-1].tof   =tof;
    nav->lexeph[sat-1].health=health;
    
    trace(4,"sat=%2d toe=%s pos=%.3f %.3f %.3f vel=%.5f %.5f %.5f\n",
          sat,time_str(toe,0),eph.pos[0],eph.pos[1],eph.pos[2],
          eph.vel[0],eph.vel[1],eph.vel[2]);
    trace(4,"clk=%11.3f %8.5f tgd=%7.3f\n",eph.af0*1E9,eph.af1*1E9,
          eph.tgd*1E9);
    trace(4,"isc=%6.3f %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f\n",
          eph.isc[0]*1E9,eph.isc[1]*1E9,eph.isc[2]*1E9,eph.isc[3]*1E9,
          eph.isc[4]*1E9,eph.isc[5]*1E9,eph.isc[6]*1E9);
    return i;
}
/* decode ionosphere correction field (ref [1] 5.7.2.2.1.3) ------------------*/
static int decode_lexion(const unsigned char *buff, int i, gtime_t tof,
                         nav_t *nav)
{
    lexion_t ion={{0}};
    int tow,week;
    
    trace(3,"decode_lexion: tof=%s\n",time_str(tof,0));
    
    tow=getbitu(buff,i,20); i+=20;
    
    if (tow==0xFFFFF) { /* correction not available */
        return i+192;
    }
    week=getbitu(buff,i,13); i+=13;
    ion.t0=gpst2time(week,tow);
    ion.tspan     =getbitu(buff,i, 8)*60.0; i+= 8; /* time span (s) */
    ion.pos0[0]   =getbits(buff,i,19)*1E-5; i+=19; /* latitude  (rad) */
    ion.pos0[1]   =getbits(buff,i,20)*1E-5; i+=20; /* longitude (rad) */
    ion.coef[0][0]=getbits(buff,i,22)*1E-3; i+=22;
    ion.coef[1][0]=getbits(buff,i,22)*1E-2; i+=22;
    ion.coef[2][0]=getbits(buff,i,22)*1E-2; i+=22;
    ion.coef[0][1]=getbits(buff,i,22)*1E-2; i+=22;
    ion.coef[1][1]=getbits(buff,i,22)*1E-2; i+=22;
    ion.coef[2][1]=getbits(buff,i,22)*1E-1; i+=22;
    nav->lexion=ion;
    
    trace(4,"t0=%s tspan=%.0f pos0=%.1f %.1f coef=%.3f %.3f %.3f %.3f %.3f %.3f\n",
          time_str(ion.t0,0),ion.tspan,ion.pos0[0]*R2D,ion.pos0[1]*R2D,
          ion.coef[0][0],ion.coef[1][0],ion.coef[2][0],ion.coef[0][1],
          ion.coef[1][1],ion.coef[2][1]);
    return i;
}
/* decode type 10: ephemeris data and clock (ref [1] 5.7.2.2.1,1) ------------*/
static int decode_lextype10(const lexmsg_t *msg, nav_t *nav, gtime_t *tof)
{
    gtime_t toe;
    int i=0,j;
    
    trace(3,"decode_lextype10:\n");
    
    /* decode tof and toe field */
    i=decode_lextof(msg->msg,i,tof,&toe);
    
    /* decode signal health field */
    i=decode_lexhealth(msg->msg,i,*tof,nav);
    
    /* decode ephemeris and sv clock field */
    for (j=0;j<3;j++) {
        i=decode_lexeph(msg->msg,i,toe,nav);
    }
    return 1;
}
/* decode type 11: ephemeris data and clock (ref [1] 5.7.2.2.1,1) ------------*/
static int decode_lextype11(const lexmsg_t *msg, nav_t *nav, gtime_t *tof)
{
    gtime_t toe;
    int i=0,j;
    
    trace(3,"decode_lextype11:\n");
    
    /* decode tof and toe field */
    i=decode_lextof(msg->msg,i,tof,&toe);
    
    /* decode signal health field */
    i=decode_lexhealth(msg->msg,i,*tof,nav);
    
    /* decode ephemeris and sv clock field */
    for (j=0;j<2;j++) {
        i=decode_lexeph(msg->msg,i,toe,nav);
    }
    /* decode ionosphere correction field */
    decode_lexion(msg->msg,i,*tof,nav);
    
    return 1;
}
/* convert lex type 12 to rtcm ssr message -----------------------------------*/
static int lex2rtcm(const unsigned char *msg, int i, unsigned char *buff)
{
    unsigned int crc;
    int j,ns,type,n=0;
    
    if (i+12>=LEXFRMLEN-LEXHDRLEN-LEXRSLEN) return 0;
    
    switch ((type=getbitu(msg,i,12))) {
        
        case 1057: ns=getbitu(msg,i+62,6); n=68+ns*135; break; /* gps */
        case 1058: ns=getbitu(msg,i+61,6); n=67+ns* 76; break;
        case 1059: ns=getbitu(msg,i+61,6); n=67;
                   for (j=0;j<ns;j++) n+=11+getbitu(msg,i+n+6,5)*19;
                   break;
        case 1060: ns=getbitu(msg,i+62,6); n=68+ns*205; break;
        case 1061: ns=getbitu(msg,i+61,6); n=67+ns* 12; break;
        case 1062: ns=getbitu(msg,i+61,6); n=67+ns* 28; break;
        case 1063: ns=getbitu(msg,i+59,6); n=65+ns*134; break; /* glonass */
        case 1064: ns=getbitu(msg,i+58,6); n=64+ns* 75; break;
        case 1065: ns=getbitu(msg,i+58,6); n=64;
                   for (j=0;j<ns;j++) n+=10+getbitu(msg,i+n+5,5)*19;
                   break;
        case 1066: ns=getbitu(msg,i+59,6); n=65+ns*204; break;
        case 1067: ns=getbitu(msg,i+58,6); n=64+ns* 11; break;
        case 1068: ns=getbitu(msg,i+58,6); n=64+ns* 27; break;
        case 1240: ns=getbitu(msg,i+62,6); n=68+ns*135; break; /* galileo */
        case 1241: ns=getbitu(msg,i+61,6); n=67+ns* 76; break;
        case 1242: ns=getbitu(msg,i+61,6); n=67;
                   for (j=0;j<ns;j++) n+=11+getbitu(msg,i+n+6,5)*19;
                   break;
        case 1243: ns=getbitu(msg,i+62,6); n=68+ns*205; break;
        case 1244: ns=getbitu(msg,i+61,6); n=67+ns* 12; break;
        case 1245: ns=getbitu(msg,i+61,6); n=67+ns* 28; break;
        case 1246: ns=getbitu(msg,i+62,4); n=66+ns*133; break; /* qzss */
        case 1247: ns=getbitu(msg,i+61,4); n=65+ns* 74; break;
        case 1248: ns=getbitu(msg,i+61,4); n=65;
                   for (j=0;j<ns;j++) n+=9+getbitu(msg,i+n+4,5)*19;
                   break;
        case 1249: ns=getbitu(msg,i+62,4); n=66+ns*203; break;
        case 1250: ns=getbitu(msg,i+61,4); n=65+ns* 10; break;
        case 1251: ns=getbitu(msg,i+61,4); n=65+ns* 26; break;
        default:
            if (type) trace(2,"lex 12: unsupported type=%4d\n",type);
            return 0;
    }
    n=(n+7)/8; /* message length (bytes) */
    
    if (i+n*8>LEXFRMLEN-LEXRSLEN) {
        trace(2,"lex 12: invalid ssr size: len=%4d\n",n);
        return 0;
    }
    /* save rtcm message to buffer */
    setbitu(buff, 0, 8,RTCM3PREAMB);
    setbitu(buff, 8, 6,0);
    setbitu(buff,14,10,n);
    for (j=0;j<n;j++) {
        buff[j+3]=getbitu(msg,i+j*8,8);
    }
    crc=rtk_crc24q(buff,3+n);
    setbitu(buff,24+n*8,24,crc);
    return n;
}
/* decode type 12: madoca orbit and clock correction -------------------------*/
static int decode_lextype12(const lexmsg_t *msg, nav_t *nav, gtime_t *tof)
{
    static rtcm_t stock_rtcm={0};
    rtcm_t rtcm={0};
    double tow;
    unsigned char buff[1200];
    int i=0,j,k,l,n,week;
    
    trace(3,"decode_lextype12:\n");
    
    tow =getbitu(msg->msg,i,20); i+=20;
    week=getbitu(msg->msg,i,13); i+=13;
    *tof=gpst2time(week,tow);
    
    /* copy rtcm ssr corrections */
    for (k=0;k<MAXSAT;k++) {
        rtcm.ssr[k]=nav->ssr[k];
        rtcm.ssr[k].update=0;
    }
    /* convert lex type 12 to rtcm ssr message */
    while ((n=lex2rtcm(msg->msg,i,buff))) {
        
        rtcm.time=*tof;
        
        for (j=0;j<n+6;j++) {
            
            /* input rtcm ssr message */
            if (input_rtcm3(&rtcm,buff[j])==-1) continue;
            
            /* update ssr corrections in nav data */
            for (k=0;k<MAXSAT;k++) {
                if (!rtcm.ssr[k].update) continue;
                
                rtcm.ssr[k].update=0;
                
                if (rtcm.ssr[k].t0[3].time){      /* ura */
                    stock_rtcm.ssr[k].t0[3]=rtcm.ssr[k].t0[3];
                    stock_rtcm.ssr[k].udi[3]=rtcm.ssr[k].udi[3];
                    stock_rtcm.ssr[k].iod[3]=rtcm.ssr[k].iod[3];
                    stock_rtcm.ssr[k].ura=rtcm.ssr[k].ura;
                }
                if (rtcm.ssr[k].t0[2].time){      /* hr-clock correction*/
                    
                    /* convert hr-clock correction to clock correction*/
                    stock_rtcm.ssr[k].t0[1]=rtcm.ssr[k].t0[2];
                    stock_rtcm.ssr[k].udi[1]=rtcm.ssr[k].udi[2];
                    stock_rtcm.ssr[k].iod[1]=rtcm.ssr[k].iod[2];
                    stock_rtcm.ssr[k].dclk[0]=rtcm.ssr[k].hrclk;
                    stock_rtcm.ssr[k].dclk[1]=stock_rtcm.ssr[k].dclk[2]=0.0;
                    
                    /* activate orbit correction(60.0s is tentative) */
                    if((stock_rtcm.ssr[k].iod[0]==rtcm.ssr[k].iod[2]) &&
                       (timediff(stock_rtcm.ssr[k].t0[0],rtcm.ssr[k].t0[2]) < 60.0)){
                        rtcm.ssr[k] = stock_rtcm.ssr[k];
                    }
                    else continue; /* not apply */
                }
                else if (rtcm.ssr[k].t0[0].time){ /* orbit correction*/
                    stock_rtcm.ssr[k].t0[0]=rtcm.ssr[k].t0[0];
                    stock_rtcm.ssr[k].udi[0]=rtcm.ssr[k].udi[0];
                    stock_rtcm.ssr[k].iod[0]=rtcm.ssr[k].iod[0];
                    for (l=0;l<3;l++) {
                        stock_rtcm.ssr[k].deph [l]=rtcm.ssr[k].deph [l];
                        stock_rtcm.ssr[k].ddeph[l]=rtcm.ssr[k].ddeph[l];
                    }
                    stock_rtcm.ssr[k].iode=rtcm.ssr[k].iode;
                    stock_rtcm.ssr[k].refd=rtcm.ssr[k].refd;
                    
                    /* activate clock correction(60.0s is tentative) */
                    if((stock_rtcm.ssr[k].iod[1]==rtcm.ssr[k].iod[0]) &&
                      (timediff(stock_rtcm.ssr[k].t0[1],rtcm.ssr[k].t0[0]) < 60.0)){
                        rtcm.ssr[k] = stock_rtcm.ssr[k];
                    }
                    else continue; /* not apply */
                }
                /* apply */
                nav->ssr[k]=rtcm.ssr[k];
            }
        }
        i+=n*8;
    }
    return 1;
}
/* decode type 20: gsi experiment message (ref [1] 5.7.2.2.2) ----------------*/
static int decode_lextype20(const lexmsg_t *msg, nav_t *nav, gtime_t *tof)
{
    trace(3,"decode_lextype20:\n");
    
    return 0; /* not supported */
}
/* update lex corrections ------------------------------------------------------
* update lex correction parameters in navigation data with a lex message
* args   : lexmsg_t *msg    I   lex message
*          nav_t    *nav    IO  navigation data
*          gtime_t  *tof    O   time of frame
* return : status (1:ok,0:error or not supported type)
*-----------------------------------------------------------------------------*/
extern int lexupdatecorr(const lexmsg_t *msg, nav_t *nav, gtime_t *tof)
{
    trace(3,"lexupdatecorr: type=%d\n",msg->type);
    
    switch (msg->type) {
        case 10: return decode_lextype10(msg,nav,tof); /* jaxa */
        case 11: return decode_lextype11(msg,nav,tof); /* jaxa */
        case 12: return decode_lextype12(msg,nav,tof); /* jaxa */
        case 20: return decode_lextype20(msg,nav,tof); /* gsi */
    }
    trace(2,"unsupported lex message: type=%2d\n",msg->type);
    return 0;
}
/* read qzss lex message log file ----------------------------------------------
* read sbas message file
* args   : char     *file   I   qzss lex message file
*          int      sel     I   qzss lex satellite prn number selection (0:all)
*          qzslex_t *lex    IO  qzss lex messages
* return : status (1:ok,0:error)
* notes  : only input file with extension .lex or .LEX.
*-----------------------------------------------------------------------------*/
extern int lexreadmsg(const char *file, int sel, lex_t *lex)
{
    lexmsg_t *lex_msgs;
    int i,prn,type,alert;
    unsigned int b;
    char buff[1024],*p;
    FILE *fp;
    
    trace(3,"readmsgs: file=%s sel=%d\n",file,sel);
    
    if (!(p=strrchr(file,'.'))||(strcmp(p,".lex")&&strcmp(p,".LEX"))) return 0;
    
    if (!(fp=fopen(file,"r"))) {
        trace(2,"lex message log open error: %s\n",file);
        return 0;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        if (sscanf(buff,"%d %d %d",&prn,&type,&alert)==3&&(p=strstr(buff,": "))) {
            p+=2;
        }
        else {
            trace(2,"invalid lex log: %s\n",buff);
            continue;
        }
        if (sel!=0&&sel!=prn) continue;
        
        if (lex->n>=lex->nmax) {
            lex->nmax=lex->nmax==0?1024:lex->nmax*2;
            if (!(lex_msgs=(lexmsg_t *)realloc(lex->msgs,lex->nmax*sizeof(lexmsg_t)))) {
                trace(1,"lexreadmsg malloc error: nmax=%d\n",lex->nmax);
                free(lex->msgs); lex->msgs=NULL; lex->n=lex->nmax=0;
                return 0;
            }
            lex->msgs=lex_msgs;
        }
        lex->msgs[lex->n].prn  =prn;
        lex->msgs[lex->n].type =type;
        lex->msgs[lex->n].alert=alert;
        for (i=0;i<212;i++) lex->msgs[lex->n].msg[i]=0;
        for (i=0;*(p-1)&&*p&&i<212;p+=2,i++) {
            if (sscanf(p,"%2X",&b)==1) lex->msgs[lex->n].msg[i]=(unsigned char)b;
        }
        lex->n++;
    }
    fclose(fp);
    
    return 1;
}
/* output lex messages ---------------------------------------------------------
* output lex message record to output file in rtklib lex log format
* args   : FILE   *fp       I   output file pointer
*          lexmsg_t *lexmsg I   lex messages
* return : none
* notes  : see ref [1] 5.7.2.1
*-----------------------------------------------------------------------------*/
extern void lexoutmsg(FILE *fp, const lexmsg_t *msg)
{
    int i;
    
    trace(4,"lexoutmsg:\n");
    
    fprintf(fp,"%3d %2d %1d : ",msg->prn,msg->type,msg->alert);
    for (i=0;i<212;i++) fprintf(fp,"%02X",msg->msg[i]);
    fprintf(fp,"\n");
}
/* convert lex binary file to lex message log ----------------------------------
* convert lex binary file to lex message log
* args   : int    type      I   output type (0:all)
*          int    format    I   lex binary format (0:no-headr,1:with-header)
*          char   *infile   I   input file
*          char   *outfile  I   output file
* return : status (1:ok,0:no correction)
* notes  : see ref [1] 5.7.2.1
*-----------------------------------------------------------------------------*/
extern int lexconvbin(int type, int format, const char *infile,
                      const char *outfile)
{
    FILE *ifp,*ofp;
    lexmsg_t msg;
    unsigned int preamb;
    unsigned char buff[LEXHEADLEN+LEXFRMLEN/8];
    int i,j,n=0;
    size_t len=(format?LEXHEADLEN:0)+LEXFRMLEN/8;
    
    trace(3,"lexconvbin:type=%d infile=%s outfile=%s\n",type,infile,outfile);
    
    if (!(ifp=fopen(infile,"rb"))) {
        trace(1,"lexconvbin infile open error: %s\n",infile);
        return 0;
    }
    if (!(ofp=fopen(outfile,"w"))) {
        trace(1,"lexconvbin outfile open error: %s\n",outfile);
        fclose(ifp);
        return 0;
    }
    while (fread(buff,1,len,ifp)==len) {
        i=format?LEXHEADLEN*8:0;
        preamb   =getbitu(buff,i,32); i+=32;
        msg.prn  =getbitu(buff,i, 8); i+= 8;
        msg.type =getbitu(buff,i, 8); i+= 8;
        msg.alert=getbitu(buff,i, 1); i+= 1;
        if (preamb!=LEXFRMPREAMB) {
            trace(1,"lex frame preamble error: preamb=%08X\n",preamb);
            continue;
        }
        for (j=0;j<212;j++) {
            msg.msg[j]=(unsigned char)getbitu(buff,i,8); i+=8;
        }
        msg.msg[211]&=0xFE;
        
        fprintf(stderr,"frame=%5d prn=%d type=%d alert=%d\r",++n,msg.prn,
                msg.type,msg.alert);
        
        if (type==0||type==msg.type) {
            lexoutmsg(ofp,&msg);
        }
    }
    fclose(ifp);
    fclose(ofp);
    fprintf(stderr,"\n");
    return 1;
}
/* lex satellite ephemeris and clock correction -------------------------------
* satellite position by lex ephemeris
* args   : gtime_t time     I   time (gpst)
*          int    sat       I   satellite
*          nav_t  *nav      I   navigation data
*          double *rs       O   satellite position and velocity
*                               {x,y,z,vx,vy,vz} (ecef) (m|m/s)
*          double *dts      O   satellite clock {bias,drift} (s|s/s)
*          double *var      O   satellite position and clock variance (m^2)
* return : status (1:ok,0:no correction)
* notes  : see ref [1] 5.7.2.2.1.2
*          before calling the function, call lexupdatecorr() to set lex 
*          corrections to navigation data
*          dts includes relativistic effect correction
*          dts does not include code bias correction
*-----------------------------------------------------------------------------*/
extern int lexeph2pos(gtime_t time, int sat, const nav_t *nav, double *rs,
                      double *dts, double *var)
{
    const lexeph_t *eph;
    double t,t2,t3;
    int i;
    
    trace(3,"lexsatpos: time=%s sat=%2d\n",time_str(time,3),sat);
    
    if (!sat) return 0;
    
    eph=nav->lexeph+sat-1;
    
    if (eph->sat!=sat||eph->toe.time==0) {
         trace(2,"no lex ephemeris: time=%s sat=%2d\n",time_str(time,0),sat);
         return 0;
    }
    if (fabs(t=timediff(time,eph->toe))>LEXEPHMAXAGE) {
         trace(2,"lex ephemeris age error: time=%s sat=%2d t=%.3f\n",
               time_str(time,0),sat,t);
         return 0;
    }
#if 0
    if (eph->health&0x18) {
         trace(2,"lex ephemeris unhealthy: sat=%2d health=0x%02X\n",sat,eph->health);
         return 0;
    }
#endif
    t2=t*t/2.0; t3=t2*t/3.0;
    for (i=0;i<3;i++) {
        rs[  i]=eph->pos[i]+eph->vel[i]*t+eph->acc [i]*t2+eph->jerk[i]*t3;
        rs[i+3]=eph->vel[i]+eph->acc[i]*t+eph->jerk[i]*t2;
    }
    dts[0]=eph->af0+eph->af1*t;
    dts[1]=eph->af1;
    
    /* relativistic effect correction */
    dts[0]-=2.0*dot(rs,rs+3,3)/CLIGHT/CLIGHT;
    
    *var=vareph(eph->ura);
    return 1;
}
/* lex ionosphere correction --------------------------------------------------
* ionosphere correction by lex correction
* args   : gtime_t  time    I   time
*          nav_t    *nav    I   navigation data
*          double   *pos    I   receiver position {lat,lon,height} (rad/m)
*          double   *azel   I   satellite azimuth/elavation angle (rad)
*          double   *delay  O   slant ionospheric delay (L1) (m)
*          double   *var    O   variance of ionospheric delay (m^2)
* return : status (1:ok, 0:no correction)
* notes  : see ref [1] 5.7.2.2.1.3
*          before calling the function, call lexupdatecorr() to set lex 
*          corrections to navigation data
*-----------------------------------------------------------------------------*/
extern int lexioncorr(gtime_t time, const nav_t *nav, const double *pos,
                      const double *azel, double *delay, double *var)
{
    const double re=6378.137,hion=350.0;
#if 0
    const double dl1=(141.0-129.0)/(45.5-34.7);
    const double dl2=(129.0-126.7)/(34.7-26.0);
#endif
    double tt,sinlat,coslat,sinaz,cosaz,cosel,rp,ap,sinap,cosap,latpp,lonpp;
    double dlat,dlon,Enm,F;
    int n,m;
    
    trace(4,"lexioncorr: time=%s pos=%.3f %.3f azel=%.3f %.3f\n",time_str(time,3),
          pos[0]*R2D,pos[1]*R2D,azel[0]*R2D,azel[1]*R2D);
    
    *delay=*var=0.0;
    
    if (pos[2]<-100.0||azel[1]<=0.0) return 1;
    
    tt=timediff(time,nav->lexion.t0);
    
    /* check time span */
    if (fabs(tt)>nav->lexion.tspan) {
        trace(2,"lex iono age error: tt=%.0f tspan=%.0f\n",tt,nav->lexion.tspan);
        return 0;
    }
    /* check user position range (ref [1] 4.1.5) */
#if 0
    if (pos[0]> 45.5*D2R||pos[0]< 26.0*D2R||
        pos[1]>146.0*D2R||
        pos[1]<129.0*D2R+dl1*(pos[0]-34.7*D2R)||
        pos[1]<126.7*D2R+dl2*(pos[0]-26.0*D2R)) {
        trace(2,"lex iono out of coverage pos=%.3f %.3f\n",pos[0]*R2D,pos[1]*R2D);
        return 0;
    }
#endif
    /* ionospheric pierce point position */
    sinlat=sin(pos[0]);
    coslat=cos(pos[0]);
    sinaz=sin(azel[0]);
    cosaz=cos(azel[0]);
    cosel=cos(azel[1]);
    rp=re/(re+hion)*cosel;
    ap=PI/2.0-azel[1]-asin(rp);
    sinap=sin(ap);
    cosap=cos(ap);
    latpp=asin(sinlat*cosap+coslat*sinap*cosaz);
    lonpp=pos[1]+atan(sinap*sinaz/(cosap*coslat-sinap*cosaz*sinlat));
    
    trace(4,"lexioncorr: pppos=%.3f %.3f\n",latpp*R2D,lonpp*R2D);
    
    /* inclination factor */
    F=1.0/sqrt(1.0-rp*rp);
    
    /* delta latitude/longitude (rad) */
    dlat=latpp-nav->lexion.pos0[0];
    dlon=lonpp-nav->lexion.pos0[1];
    trace(4,"lexioncorr: pos0=%.1f %.1f dlat=%.1f dlon=%.1f\n",
          nav->lexion.pos0[0]*R2D,nav->lexion.pos0[1]*R2D,dlat*R2D,dlon*R2D);
    
    /* slant ionosphere delay (L1) */
    for (n=0;n<=2;n++) for (m=0;m<=1;m++) {
        Enm=nav->lexion.coef[n][m];
        *delay+=F*Enm*pow(dlat,n)*pow(dlon,m);
        
        trace(5,"lexioncorr: F=%8.3f Enm[%d][%d]=%8.3f delay=%8.3f\n",F,n,m,Enm,
              F*Enm*pow(dlat,n)*pow(dlon,m));
    }
    trace(4,"lexioncorr: time=%s delay=%.3f\n",time_str(time,0),*delay);
    
    return 1;
}
