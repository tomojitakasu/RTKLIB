/*------------------------------------------------------------------------------
* qzsl6.c : qzss L6 functions
*
* references :
*     [1] IS-QZSS-L6-003, Quasi-Zenith Satellite System
*         Interface Specification Centimeter Level Augmentation Service,
*         Cabinet Office, August 20, 2020

* history : 2021/01/05 1.0  new (based on qzslex.c)
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

/* convert QZSS L6 message to rtcm ssr message -----------------------------------*/
static int l6msg2rtcm(const unsigned char *msg, int i, unsigned char *buff)
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
    while ((n=l6msg2rtcm(msg->msg,i,buff))) {
        
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

/* update L6 corrections ------------------------------------------------------
* update L6 correction parameters in navigation data with a lex message
* args   : lexmsg_t *msg    I   lex message
*          nav_t    *nav    IO  navigation data
*          gtime_t  *tof    O   time of frame
* return : status (1:ok,0:error or not supported type)
*-----------------------------------------------------------------------------*/
extern int l6updatecorr(const lexmsg_t *msg, nav_t *nav, gtime_t *tof)
{
    trace(3,"l6updatecorr: type=%d\n",msg->type);
    
    switch (msg->type) {
        case 33:                                       /* L6E/madoca */
                 return decode_lextype12(msg,nav,tof);
    }
    trace(2,"unsupported L6 message: type=%2d\n",msg->type);
    return 0;
}
/* read qzss L6 message log file ----------------------------------------------
* read sbas message file
* args   : char     *file   I   qzss lex message file
*          int      sel     I   qzss lex satellite prn number selection (0:all)
*          qzslex_t *lex    IO  qzss lex messages
* return : status (1:ok,0:error)
* notes  : only input file with extension .lex or .LEX.
*-----------------------------------------------------------------------------*/
extern int l6readmsg(const char *file, int sel, lex_t *lex)
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
/* output L6 messages ---------------------------------------------------------
* output L6 message record to output file in rtklib lex log format
* args   : FILE   *fp       I   output file pointer
*          lexmsg_t *lexmsg I   lex messages
* return : none
* notes  : see ref [1] 5.7.2.1
*-----------------------------------------------------------------------------*/
extern void l6outmsg(FILE *fp, const lexmsg_t *msg)
{
    int i;
    
    trace(4,"lexoutmsg:\n");
    
    fprintf(fp,"%3d %2d %1d : ",msg->prn,msg->type,msg->alert);
    for (i=0;i<212;i++) fprintf(fp,"%02X",msg->msg[i]);
    fprintf(fp,"\n");
}
/* convert L6 binary file to lex message log ----------------------------------
* convert L6 binary file to lex message log
* args   : int    type      I   output type (0:all)
*          int    format    I   lex binary format (0:no-headr,1:with-header)
*          char   *infile   I   input file
*          char   *outfile  I   output file
* return : status (1:ok,0:no correction)
* notes  : see ref [1] 5.7.2.1
*-----------------------------------------------------------------------------*/
extern int l6convbin(int type, int format, const char *infile,
                      const char *outfile)
{
    FILE *ifp,*ofp;
    lexmsg_t msg;
    unsigned int preamb;
    unsigned char buff[LEXHEADLEN+LEXFRMLEN/8];
    int i,j,n=0;
    size_t len=(format?LEXHEADLEN:0)+LEXFRMLEN/8;
    
    trace(3,"l6convbin:type=%d infile=%s outfile=%s\n",type,infile,outfile);
    
    if (!(ifp=fopen(infile,"rb"))) {
        trace(1,"l6convbin infile open error: %s\n",infile);
        return 0;
    }
    if (!(ofp=fopen(outfile,"w"))) {
        trace(1,"l6convbin outfile open error: %s\n",outfile);
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
            trace(1,"L6 frame preamble error: preamb=%08X\n",preamb);
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

#if 0

/* decode QZS L6 CLAS stream */
extern int decode_qzs_msg(rtcm_t *rtcm, int head, uint8_t *frame)
{
    static int startdecode = 0;
    static cssr_t _cssr = {0,};
    static int savefacility = -1;
    static int savedelivery = -1;

    cssr_t *cssr = &_cssr;
    int startbit,week,type,subtype;
    int i, ret = 0;
    double tow;

    if (*frame==0x00 || rtcm->nbit==-1) {
        return 0;
    }

    i = startbit = cssr->nbit;
    if (i+16>rtcm->nbit) return 0;
    type = getbitu(rtcm->buff,i,12); i+= 12;
    if (type != 4073) {
        trace(4, "cssr: decode terminate: frame=%02x, nbit=%d, nbit=%d\n",
        		*frame,rtcm->nbit,cssr->nbit);
        cssr->nbit = 0;
        *frame = 0;
        return 0;
    }
    subtype=getbitu(rtcm->buff,i,4);
    if (subtype!=CSSR_TYPE_MASK && !startdecode) {
        return 0;
    }

    cssr_check_bitlen(rtcm,i); i+=4;

    tow = time2gpst(timeget(), &week);
    trace(4, "cssr: frame=%02x, type=%d, subtype=%d, week=%d, tow=%.2f\n",
    		*frame,type,subtype, week, tow);

    switch (subtype) {
        case CSSR_TYPE_MASK:
            ret=decode_cssr_mask(rtcm, cssr, i, head);
            if (!startdecode) {
                trace(2, "start CSSR decode: week=%d, tow=%.2f\n", week, tow);
            }
            startdecode = 1;
            break;
        case CSSR_TYPE_OC:
            ret=decode_cssr_oc(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_CC:
            ret=decode_cssr_cc(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_CB:
            ret=decode_cssr_cb(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_PB:
            ret=decode_cssr_pb(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_BIAS:
            ret=decode_cssr_bias(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_URA:
            ret=decode_cssr_ura(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_STEC:
            ret=decode_cssr_stec(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_GRID:
            ret=decode_cssr_grid(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_OCC:
            ret=decode_cssr_combo(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_ATMOS:
            ret=decode_cssr_atmos(rtcm, cssr, i, head);
            break;
        case CSSR_TYPE_SI:
        	ret=decode_cssr_si(rtcm, cssr, i, head);
            break;
        default: break;
    }
    if (ret==-1) {
    	cssr->nbit=0;
    	cssr->iod=-1;
    	*frame=0;
    	return 0;
    }

    return ret;
}


#define L6FRMPREAMB 0x1ACFFC1Du /* L6 message frame preamble */
#define BLEN_MSG 218

/* read and decode QZS L6 stream */
int read_qzs_msg(rtcm_t *rtcm, unsigned char *pbuff, int nframe)
{
    int i=0, j, k, jn, jn0 = -1, prn, msgid, alert;
    unsigned char *buff;

    for (j=0;j<5;j++) {
        buff = pbuff+j*BLEN_MSG;

        prn = buff[4];
        msgid = buff[5];
        alert = (buff[6]>>7) & 0x1;

        if (msgid & 0x1) { /* head */
            jn0 = j;
            break;
        }
    }

    if (jn0 < 0) {
        memset(rtcm->buff, 0x00, sizeof(rtcm->buff));
        return 0;
    }

    trace(4,"read_qzs_msg prn=%d msgid=%d alert=%d\n",prn,msgid,alert);

    for (j=0,jn=jn0;j<5;j++,jn++) {
        if (jn >= 5) {
            jn = 0;
        }
        buff = pbuff+jn*BLEN_MSG;

        setbitu(rtcm->buff,i,7, buff[6] & 0x7f); i+=7;
        for (k=0;k<211;k++) {
            setbitu(rtcm->buff,i,8, buff[7+k]); i+=8;
        }
    }

    return 0;
}

/* read list of grid position from ascii file */
extern int read_grid_def(const char *gridfile)
{
	int gridsel=0 ;
    int no, lath, latm, lonh, lonm;
    double lat, lon, alt;
    char buff[1024], *temp, *p;
    int inet, grid[CSSR_MAX_NETWORK] = {0,}, isqzss=0, ret;
    FILE *fp=NULL;

    for (inet = 0; inet < CSSR_MAX_NETWORK; ++inet) {
        clas_grid[inet][0][0] = -1.0;
        clas_grid[inet][0][1] = -1.0;
        clas_grid[inet][0][2] = -1.0;
    }

    trace(2, "read_grid_def(): gridfile=%s\n", gridfile);
    fp = fopen(gridfile, "r");
    if (fp == NULL) {
        return -1;
    }

    while (fgets(buff, sizeof(buff), fp)) {
        if (strstr(buff, "<CSSR Grid Definition>")) {
            while (fgets(buff, sizeof(buff), fp)) {
                if ((temp = strstr(buff, "<Version>"))) {
                    p = temp + 9;
                    if ((temp = strstr(buff, "</Version>"))) {
                        *temp = '\0';
                    }
                    gridsel = atoi(p);
                    trace(2, "grid definition: version=%d\n", gridsel);
                    break;
                }
            }
            break;
		} else if (strstr(buff, "Compact Network ID    GRID No.  Latitude     Longitude   Ellipsoidal height")) {
            gridsel = 3;
			isqzss = 1;
            trace(2, "grid definition: IS attached file version%d\n", gridsel);
            break;
		} else {
            trace(1, "grid definition: invalid format%d\n", gridsel);
            return -1;
		}
    }
    fclose(fp);

    fp = fopen(gridfile, "r");
    if (fp == NULL) {
        return -1;
    }

	if (isqzss == 0) {
        while (fgets(buff, sizeof(buff), fp)) {
            if (sscanf(buff, "<Network%d>", &inet)) {
                while (fscanf(fp, "%d\t%d\t%d\t%lf\t%d\t%d\t%lf\t%lf",
                              &no, &lath, &latm, &lat, &lonh, &lonm, &lon, &alt) > 0) {
                    if (inet >= 0 && inet < CSSR_MAX_NETWORK) {
                        clas_grid[inet][grid[inet]][0] = (double)lath + ((double)latm/60.0) + (lat/3600.0);
                        clas_grid[inet][grid[inet]][1] = (double)lonh + ((double)lonm/60.0) + (lon/3600.0);
                        clas_grid[inet][grid[inet]][2] = alt;
                        grid[inet]++;
                        clas_grid[inet][grid[inet]][0] = -1.0;
                        clas_grid[inet][grid[inet]][1] = -1.0;
                        clas_grid[inet][grid[inet]][2] = -1.0;
                    }
                }
            }
        }
	} else {
        fgets(buff, sizeof(buff), fp);
        while ((ret=fscanf(fp, "%d %d %lf %lf %lf", &inet, &no, &lat, &lon, &alt)) != EOF ) {
            if (inet>=0 && inet<CSSR_MAX_NETWORK && ret==5) {
                clas_grid[inet][grid[inet]][0] = lat;
                clas_grid[inet][grid[inet]][1] = lon;
                clas_grid[inet][grid[inet]][2] = alt;
                grid[inet]++;
                clas_grid[inet][grid[inet]][0] = -1.0;
                clas_grid[inet][grid[inet]][1] = -1.0;
                clas_grid[inet][grid[inet]][2] = -1.0;
            }
            trace(3, "grid_info(fscanf:%d), %d, %d, %lf, %lf, %lf\n", ret, inet, no, lat, lon, alt);
        }
	}
    fclose(fp);
    return 0;
}

/* decode cssr messages in the QZS L6 subframe */
extern int input_l6msg(rtcm_t *rtcm, unsigned char data, uint8_t *frame, lex_t *lex)
{
	int i,j;
    static uint32_t preamble = 0;
    static uint64_t data_p = 0;
    uint8_t prn, msgid, alert, facility, sidx;
    static int nframe = 0;
    static unsigned char buff[BLEN_MSG];
    static int decode_start = 0;

    trace(5,"input_l6msg: data=%02x\n",data);

    /* synchronize frame */
    if (rtcm->nbyte==0) {
        preamble = (preamble << 8) | data;
        data_p = (data_p << 8) | data;
        if (preamble != L6FRMPREAMB) {
            return 0;
        }
        preamble = 0;
        buff[rtcm->nbyte++]=(L6FRMPREAMB>>24) & 0xff;
        buff[rtcm->nbyte++]=(L6FRMPREAMB>>16) & 0xff;
        buff[rtcm->nbyte++]=(L6FRMPREAMB>>8) & 0xff;
        buff[rtcm->nbyte++]=data;
        return 0;
    }
    buff[rtcm->nbyte++]=data;
    rtcm->len = BLEN_MSG;

    if (rtcm->nbyte<rtcm->len) return 0;
    rtcm->nbyte=0;

    prn = buff[4];
    msgid = buff[5];
    alert = (buff[6]>>7) & 0x1;

    facility = (msgid>>3)&0x3;
    sidx = msgid & 0x1;

    lex->n = facility;
    lex->msgs[lex->n].prn = prn;
    lex->msgs[lex->n].type = msgid;
    lex->msgs[lex->n].alert = alert;

    if (sidx) {
    	rtcm->nbit=0;
    	decode_start = 1;
    	*frame = 0;
    	nframe = 0;
    }

    if (decode_start) {
    	i = 1695*nframe;
    	setbitu(rtcm->buff,i,7,buff[6]&0x7f); i+=7;
    	for (j=0;j<211;j++) {
    		setbitu(rtcm->buff,i,8,buff[7+j]); i+=8;
    	}
    	rtcm->nbit += 1695;
    	*frame |= (1<<nframe);
    	nframe++;
    }

    return 0;
}


/* decode cssr messages from file stream ---------------------------------------------*/
extern int input_l6msgf(rtcm_t *rtcm, FILE *fp)
{
    int i,data=0,ret;
    static uint8_t frame = 0;
    static lex_t _lex;
    lex_t *lex=&_lex;

    trace(4,"input_cssrf: data=%02x\n",data);

    if (!lex->msgs) {
    	lex->nmax=4;
    	lex->msgs=(lexmsg_t *)realloc(lex->msgs,lex->nmax*sizeof(lexmsg_t));
    }

    for (i=0;i<4096;i++) {
        if ((ret=decode_qzs_msg(rtcm,0,&frame))) return ret;
        if ((data=fgetc(fp))==EOF) return -2;
        input_l6msg(rtcm, (unsigned char)data, &frame,lex);
    }
    return 0; /* return at every 4k bytes */
}

#endif
