/*------------------------------------------------------------------------------
* crescent.c : hemisphere crescent/eclipse receiver dependent functions
*
*          Copyright (C) 2007-2018 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] Hemisphere GPS, Grescent Integrator's Manual, December, 2005
*     [2] Hemisphere GPS, GPS Technical Reference, Part No. 875-0175-000,
*         Rev.D1, 2008
*     [3] Hemisphere GPS, Hemisphere GPS Technical Reference, 2014
*
* version : $Revision: 1.2 $ $Date: 2008/07/14 00:05:05 $
* history : 2008/05/21 1.0 new
*           2009/04/01 1.1 support sbas, set 0 to L2 observables
*                          fix bug on getting doppler observables
*           2009/10/19 1.2 support eclipse (message bin 76)
*           2009/10/24 1.3 ignore vaild phase flag
*           2011/05/27 1.4 add -EPHALL option
*                          fix problem with ARM compiler
*           2011/07/01 1.5 suppress warning
*           2013/02/23 1.6 fix memory access violation problem on arm
*           2014/05/13 1.7 support bin65 and bin66
*                          add receiver option -TTCORR
*           2014/06/21 1.8 move decode_glostr() to rcvraw.c
*           2017/04/11 1.9 (char *) -> (signed char *)
*           2018/01/29 1.10 fix bug on unrecognized glonass obs with bin 96
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define CRESSYNC    "$BIN"      /* hemis bin sync code */

#define ID_CRESPOS   1          /* hemis msg id: bin 1 position/velocity */
#define ID_CRESGLOEPH 65        /* hemis msg id: bin 65 glonass ephemeris */
#define ID_CRESGLORAW 66        /* hemis msg id: bin 66 glonass L1/L2 phase and code */
#define ID_CRESRAW2 76          /* hemis msg id: bin 76 dual-freq raw */
#define ID_CRESWAAS 80          /* hemis msg id: bin 80 waas messages */
#define ID_CRESIONUTC 94        /* hemis msg id: bin 94 ion/utc parameters */
#define ID_CRESEPH  95          /* hemis msg id: bin 95 raw ephemeris */
#define ID_CRESRAW  96          /* hemis msg id: bin 96 raw phase and code */

#define SNR2CN0_L1  30.0        /* hemis snr to c/n0 offset (db) L1 */
#define SNR2CN0_L2  30.0        /* hemis snr to c/n0 offset (db) L2 */

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((signed char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static short          I2(unsigned char *p) {short          i; memcpy(&i,p,2); return i;}
static int            I4(unsigned char *p) {int            i; memcpy(&i,p,4); return i;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}

/* checksum ------------------------------------------------------------------*/
static int chksum(const unsigned char *buff, int len)
{
    unsigned short sum=0;
    int i;
    
    for (i=8;i<len-4;i++) sum+=buff[i];
    trace(4,"checksum=%02X%02X %02X%02X:%02X%02X\n",
          sum>>8,sum&0xFF,buff[len-3],buff[len-4],buff[len-2],buff[len-1]);
    return (sum>>8)==buff[len-3]&&(sum&0xFF)==buff[len-4]&&
           buff[len-2]==0x0D&&buff[len-1]==0x0A;
}
/* decode bin 1 postion/velocity ---------------------------------------------*/
static int decode_crespos(raw_t *raw)
{
    int ns,week,mode;
    double tow,pos[3],vel[3],std;
    char tstr[64];
    unsigned char *p=raw->buff+8;
    
    trace(4,"decode_crespos: len=%d\n",raw->len);
    
    if (raw->len!=64) {
        trace(2,"crescent bin 1 message length error: len=%d\n",raw->len);
        return -1;
    }
    ns  =U1(p+1);
    week=U2(p+2);
    tow =R8(p+4);
    pos[0]=R8(p+12);
    pos[1]=R8(p+20);
    pos[2]=R4(p+28);
    vel[0]=R4(p+32);
    vel[1]=R4(p+36);
    vel[2]=R4(p+40);
    std =R4(p+44);
    mode=U2(p+48);
    time2str(gpst2time(week,tow),tstr,3);
    trace(3,"$BIN1 %s %13.9f %14.9f %10.4f %4d %3d %.3f\n",tstr,pos[0],pos[1],
          pos[2],mode==6?1:(mode>4?2:(mode>1?5:0)),ns,std);
    return 0;
}
/* decode bin 96 raw phase and code ------------------------------------------*/
static int decode_cresraw(raw_t *raw)
{
    gtime_t time;
    double tow,tows,toff=0.0,cp,pr,dop,snr;
    int i,j,n,prn,sat,week,word2,lli=0;
    unsigned int word1,sn,sc;
    unsigned char *p=raw->buff+8;
    
    trace(4,"decode_cresraw: len=%d\n",raw->len);
    
    if (raw->len!=312) {
        trace(2,"crescent bin 96 message length error: len=%d\n",raw->len);
        return -1;
    }
    week=U2(p+2);
    tow =R8(p+4);
    tows=floor(tow*1000.0+0.5)/1000.0; /* round by 1ms */
    time=gpst2time(week,tows);
    
    /* time tag offset correction */
    if (strstr(raw->opt,"-TTCORR")) {
        toff=CLIGHT*(tows-tow);
    }
    for (i=n=0,p+=12;i<12&&n<MAXOBS;i++,p+=24) {
        word1=U4(p  );
        word2=I4(p+4);
        if ((prn=word1&0xFF)==0) continue; /* if 0, no data */
        if (!(sat=satno(prn<=MAXPRNGPS?SYS_GPS:SYS_SBS,prn))) {
            trace(2,"creasent bin 96 satellite number error: prn=%d\n",prn);
            continue;
        }
        pr=R8(p+ 8)-toff;
        cp=R8(p+16)-toff;
        if (!(word2&1)) cp=0.0; /* invalid phase */
        sn =(word1>>8)&0xFF;
        snr=sn==0?0.0:10.0*log10(0.8192*sn)+SNR2CN0_L1;
        sc =(unsigned int)(word1>>24);
        if (raw->time.time!=0) {
            lli=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][0])>0;
        }
        raw->lockt[sat-1][0]=(unsigned char)sc;
        dop=word2/16/4096.0;
        
        raw->obs.data[n].time=time;
        raw->obs.data[n].sat =sat;
        raw->obs.data[n].P[0]=pr;
        raw->obs.data[n].L[0]=cp/lam_carr[0];
        raw->obs.data[n].D[0]=-(float)(dop/lam_carr[0]);
        raw->obs.data[n].SNR[0]=(unsigned char)(snr*4.0+0.5);
        raw->obs.data[n].LLI[0]=(unsigned char)lli;
        raw->obs.data[n].code[0]=CODE_L1C;
        
        for (j=1;j<NFREQ;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
    if (strstr(raw->opt,"-ENAGLO")) return 0; /* glonass follows */
    return 1;
}
/* decode bin 76 dual-freq raw phase and code --------------------------------*/
static int decode_cresraw2(raw_t *raw)
{
    gtime_t time;
    double tow,tows,toff=0.0,cp[2]={0},pr1,pr[2]={0},dop[2]={0},snr[2]={0};
    int i,j,n=0,prn,sat,week,lli[2]={0};
    unsigned int word1,word2,word3,sc,sn;
    unsigned char *p=raw->buff+8;
    
    trace(4,"decode_cresraw2: len=%d\n",raw->len);
    
    if (raw->len!=460) {
        trace(2,"crescent bin 76 message length error: len=%d\n",raw->len);
        return -1;
    }
    tow =R8(p);
    week=U2(p+8);
    tows=floor(tow*1000.0+0.5)/1000.0; /* round by 1ms */
    time=gpst2time(week,tows);
    
    /* time tag offset correction */
    if (strstr(raw->opt,"-TTCORR")) {
        toff=CLIGHT*(tows-tow);
    }
    if (fabs(timediff(time,raw->time))<1e-9) {
        n=raw->obs.n;
    }
    for (i=0,p+=16;i<15&&n<MAXOBS;i++) {
        word1=U4(p+324+4*i); /* L1CACodeMSBsPRN */
        if ((prn=word1&0xFF)==0) continue; /* if 0, no data */
        if (!(sat=satno(prn<=MAXPRNGPS?SYS_GPS:SYS_SBS,prn))) {
            trace(2,"creasent bin 76 satellite number error: prn=%d\n",prn);
            continue;
        }
        pr1=(word1>>13)*256.0; /* upper 19bit of L1CA pseudorange */
        
        word1=U4(p+144+12*i); /* L1CASatObs */
        word2=U4(p+148+12*i);
        word3=U4(p+152+12*i);
        sn=word1&0xFFF;
        snr[0]=sn==0?0.0:10.0*log10(0.1024*sn)+SNR2CN0_L1;
        sc=(unsigned int)(word1>>24);
        if (raw->time.time!=0) {
            lli[0]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][0])>0;
        }
        else {
            lli[0]=0;
        }
        lli[0]|=((word1>>12)&7)?2:0;
        raw->lockt[sat-1][0]=(unsigned char)sc;
        dop[0]=((word2>>1)&0x7FFFFF)/512.0;
        if ((word2>>24)&1) dop[0]=-dop[0];
        pr[0]=pr1+(word3&0xFFFF)/256.0;
        cp[0]=floor(pr[0]/lam_carr[0]/8192.0)*8192.0;
        cp[0]+=((word2&0xFE000000)+((word3&0xFFFF0000)>>7))/524288.0;
        if      (cp[0]-pr[0]/lam_carr[0]<-4096.0) cp[0]+=8192.0;
        else if (cp[0]-pr[0]/lam_carr[0]> 4096.0) cp[0]-=8192.0;
        
        if (i<12) {
            word1=U4(p  +12*i); /* L2PSatObs */
            word2=U4(p+4+12*i);
            word3=U4(p+8+12*i);
            sn=word1&0xFFF;
            snr[1]=sn==0?0.0:10.0*log10(0.1164*sn)+SNR2CN0_L2;
            sc=(unsigned int)(word1>>24);
            if (raw->time.time==0) {
                lli[1]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][1])>0;
            }
            else {
                lli[1]=0;
            }
            lli[1]|=((word1>>12)&7)?2:0;
            raw->lockt[sat-1][1]=(unsigned char)sc;
            dop[1]=((word2>>1)&0x7FFFFF)/512.0;
            if ((word2>>24)&1) dop[1]=-dop[1];
            pr[1]=(word3&0xFFFF)/256.0;
            if (pr[1]!=0.0) {
                pr[1]+=pr1;
                if      (pr[1]-pr[0]<-128.0) pr[1]+=256.0;
                else if (pr[1]-pr[0]> 128.0) pr[1]-=256.0;
                cp[1]=floor(pr[1]/lam_carr[1]/8192.0)*8192.0;
                cp[1]+=((word2&0xFE000000)+((word3&0xFFFF0000)>>7))/524288.0;
                if      (cp[1]-pr[1]/lam_carr[1]<-4096.0) cp[1]+=8192.0;
                else if (cp[1]-pr[1]/lam_carr[1]> 4096.0) cp[1]-=8192.0;
            }
            else cp[1]=0.0;
        }
        raw->obs.data[n].time=time;
        raw->obs.data[n].sat =sat;
        for (j=0;j<NFREQ;j++) {
            if (j==0||(j==1&&i<12)) {
                raw->obs.data[n].P[j]=pr[j]==0.0?0.0:pr[j]-toff;
                raw->obs.data[n].L[j]=cp[j]==0.0?0.0:cp[j]-toff/lam_carr[j];
                raw->obs.data[n].D[j]=-(float)dop[j];
                raw->obs.data[n].SNR[j]=(unsigned char)(snr[j]*4.0+0.5);
                raw->obs.data[n].LLI[j]=(unsigned char)lli[j];
                raw->obs.data[n].code[j]=j==0?CODE_L1C:CODE_L2P;
            }
            else {
                raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
                raw->obs.data[n].D[j]=0.0;
                raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
                raw->obs.data[n].code[j]=CODE_NONE;
            }
        }
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
    if (strstr(raw->opt,"-ENAGLO")) return 0; /* glonass follows */
    return 1;
}
/* decode bin 95 ephemeris ---------------------------------------------------*/
static int decode_creseph(raw_t *raw)
{
    eph_t eph={0};
    unsigned int word;
    int i,j,k,prn,sat;
    unsigned char *p=raw->buff+8,buff[90];
    
    trace(4,"decode_creseph: len=%d\n",raw->len);
    
    if (raw->len!=140) {
        trace(2,"crescent bin 95 message length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U2(p);
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"crescent bin 95 satellite number error: prn=%d\n",prn);
        return -1;
    }
    for (i=0;i<3;i++) for (j=0;j<10;j++) {
        word=U4(p+8+i*40+j*4)>>6;
        for (k=0;k<3;k++) buff[i*30+j*3+k]=(unsigned char)((word>>(8*(2-k)))&0xFF);
    }
    if (decode_frame(buff   ,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(buff+30,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(buff+60,&eph,NULL,NULL,NULL,NULL)!=3) {
        trace(2,"crescent bin 95 navigation frame error: prn=%d\n",prn);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode bin 94 ion/utc parameters ------------------------------------------*/
static int decode_cresionutc(raw_t *raw)
{
    int i;
    unsigned char *p=raw->buff+8;
    
    trace(4,"decode_cresionutc: len=%d\n",raw->len);
    
    if (raw->len!=108) {
        trace(2,"crescent bin 94 message length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=0;i<8;i++) raw->nav.ion_gps[i]=R8(p+i*8);
    raw->nav.utc_gps[0]=R8(p+64);
    raw->nav.utc_gps[1]=R8(p+72);
    raw->nav.utc_gps[2]=(double)U4(p+80);
    raw->nav.utc_gps[3]=(double)U2(p+84);
    raw->nav.leaps=I2(p+90);
    return 9;
}
/* decode bin 80 waas messages -----------------------------------------------*/
static int decode_creswaas(raw_t *raw)
{
    double tow;
    unsigned int word;
    int i,j,k,prn;
    unsigned char *p=raw->buff+8;
    
    trace(4,"decode_creswaas: len=%d\n",raw->len);
    
    if (raw->len!=52) {
        trace(2,"creasent bin 80 message length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U2(p);
    if (prn<MINPRNSBS||MAXPRNSBS<prn) {
        trace(2,"creasent bin 80 satellite number error: prn=%d\n",prn);
        return -1;
    }
    raw->sbsmsg.prn=prn;
    raw->sbsmsg.tow=U4(p+4);
    tow=time2gpst(raw->time,&raw->sbsmsg.week);
    if      (raw->sbsmsg.tow<tow-302400.0) raw->sbsmsg.week++;
    else if (raw->sbsmsg.tow>tow+302400.0) raw->sbsmsg.week--;
    
    for (i=k=0;i<8&&k<29;i++) {
        word=U4(p+8+i*4);
        for (j=0;j<4&&k<29;j++) raw->sbsmsg.msg[k++]=(unsigned char)(word>>(3-j)*8);
    }
    raw->sbsmsg.msg[28]&=0xC0;
    return 3;
}
/* decode bin 66 glonass L1/L2 code and carrier phase ------------------------*/
static int decode_cresgloraw(raw_t *raw)
{
    gtime_t time;
    double tow,tows,toff=0.0,cp[2]={0},pr1,pr[2]={0},dop[2]={0},snr[2]={0};
    int i,j,n=0,prn,sat,week,lli[2]={0};
    unsigned int word1,word2,word3,sc,sn;
    unsigned char *p=raw->buff+8;
    
    trace(4,"decode_cregloraw: len=%d\n",raw->len);
    
    if (!strstr(raw->opt,"-ENAGLO")) return 0;
    
    if (raw->len!=364) {
        trace(2,"crescent bin 66 message length error: len=%d\n",raw->len);
        return -1;
    }
    tow =R8(p);
    week=U2(p+8);
    tows=floor(tow*1000.0+0.5)/1000.0; /* round by 1ms */
    time=gpst2time(week,tows);
    
    /* time tag offset correction */
    if (strstr(raw->opt,"-TTCORR")) {
        toff=CLIGHT*(tows-tow);
    }
    if (fabs(timediff(time,raw->time))<1e-9) {
        n=raw->obs.n;
    }
    for (i=0,p+=16;i<12&&n<MAXOBS;i++) {
        word1=U4(p+288+4*i); /* L1CACodeMSBsSlot */
        if ((prn=word1&0xFF)==0) continue; /* if 0, no data */
        if (!(sat=satno(SYS_GLO,prn))) {
            trace(2,"creasent bin 66 satellite number error: prn=%d\n",prn);
            continue;
        }
        pr1=(word1>>13)*256.0; /* upper 19bit of L1CA pseudorange */
        
        /* L1Obs */
        word1=U4(p  +12*i);
        word2=U4(p+4+12*i);
        word3=U4(p+8+12*i);
        sn=word1&0xFFF;
        snr[0]=sn==0?0.0:10.0*log10(0.1024*sn)+SNR2CN0_L1;
        sc=(unsigned int)(word1>>24);
        if (raw->time.time!=0) {
            lli[0]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][0])>0;
        }
        else {
            lli[0]=0;
        }
        lli[0]|=((word1>>12)&7)?2:0;
        raw->lockt[sat-1][0]=(unsigned char)sc;
        dop[0]=((word2>>1)&0x7FFFFF)/512.0;
        if ((word2>>24)&1) dop[0]=-dop[0];
        pr[0]=pr1+(word3&0xFFFF)/256.0;
        cp[0]=floor(pr[0]/lam_carr[0]/8192.0)*8192.0;
        cp[0]+=((word2&0xFE000000)+((word3&0xFFFF0000)>>7))/524288.0;
        if      (cp[0]-pr[0]/lam_carr[0]<-4096.0) cp[0]+=8192.0;
        else if (cp[0]-pr[0]/lam_carr[0]> 4096.0) cp[0]-=8192.0;
        
        /* L2Obs */
        word1=U4(p+144+12*i);
        word2=U4(p+148+12*i);
        word3=U4(p+152+12*i);
        sn=word1&0xFFF;
        snr[1]=sn==0?0.0:10.0*log10(0.1164*sn)+SNR2CN0_L2;
        sc=(unsigned int)(word1>>24);
        if (raw->time.time==0) {
            lli[1]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][1])>0;
        }
        else {
            lli[1]=0;
        }
        lli[1]|=((word1>>12)&7)?2:0;
        raw->lockt[sat-1][1]=(unsigned char)sc;
        dop[1]=((word2>>1)&0x7FFFFF)/512.0;
        if ((word2>>24)&1) dop[1]=-dop[1];
        pr[1]=(word3&0xFFFF)/256.0;
        if (pr[1]!=0.0) {
            pr[1]+=pr1;
            if      (pr[1]-pr[0]<-128.0) pr[1]+=256.0;
            else if (pr[1]-pr[0]> 128.0) pr[1]-=256.0;
            cp[1]=floor(pr[1]/lam_carr[1]/8192.0)*8192.0;
            cp[1]+=((word2&0xFE000000)+((word3&0xFFFF0000)>>7))/524288.0;
            if      (cp[1]-pr[1]/lam_carr[1]<-4096.0) cp[1]+=8192.0;
            else if (cp[1]-pr[1]/lam_carr[1]> 4096.0) cp[1]-=8192.0;
        }
        raw->obs.data[n].time=time;
        raw->obs.data[n].sat =sat;
        for (j=0;j<NFREQ;j++) {
            if (j==0||(j==1&&i<12)) {
                raw->obs.data[n].P[j]=pr[j]==0.0?0.0:pr[j]-toff;
                raw->obs.data[n].L[j]=cp[j]==0.0?0.0:cp[j]-toff/lam_carr[j];
                raw->obs.data[n].D[j]=-(float)dop[j];
                raw->obs.data[n].SNR[j]=(unsigned char)(snr[j]*4.0+0.5);
                raw->obs.data[n].LLI[j]=(unsigned char)lli[j];
                raw->obs.data[n].code[j]=j==0?CODE_L1C:CODE_L2P;
            }
            else {
                raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
                raw->obs.data[n].D[j]=0.0;
                raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
                raw->obs.data[n].code[j]=CODE_NONE;
            }
        }
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
    return 1;
}
/* decode bin 65 glonass ephemeris -------------------------------------------*/
static int decode_cresgloeph(raw_t *raw)
{
    geph_t geph={0};
    unsigned char *p=raw->buff+8,str[12];
    int i,j,k,sat,prn,frq,time,no;
    
    trace(4,"decode_cregloeph: len=%d\n",raw->len);
    
    if (!strstr(raw->opt,"-ENAGLO")) return 0;
    
    prn =U1(p);   p+=1;
    frq =U1(p)-8; p+=1+2;
    time=U4(p);   p+=4;
    
    if (!(sat=satno(SYS_GLO,prn))) {
        trace(2,"creasent bin 65 satellite number error: prn=%d\n",prn);
        return -1;
    }
    for (i=0;i<5;i++) {
        for (j=0;j<3;j++) for (k=3;k>=0;k--) {
            str[k+j*4]=U1(p++);
        }
        if ((no=getbitu(str,1,4))!=i+1) {
            trace(2,"creasent bin 65 string no error: sat=%2d no=%d %d\n",sat,
                  i+1,no);
            return -1;
        }
        memcpy(raw->subfrm[sat-1]+10*i,str,10);
    }
    /* decode glonass ephemeris strings */
    geph.tof=raw->time;
    if (!decode_glostr(raw->subfrm[sat-1],&geph)||geph.sat!=sat) return -1;
    geph.frq=frq;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (geph.iode==raw->nav.geph[prn-1].iode) return 0; /* unchanged */
    }
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;
    return 2;
}
/* decode crescent raw message -----------------------------------------------*/
static int decode_cres(raw_t *raw)
{
    int type=U2(raw->buff+4);
    
    trace(3,"decode_cres: type=%2d len=%d\n",type,raw->len);
    
    if (!chksum(raw->buff,raw->len)) {
        trace(2,"crescent checksum error: type=%2d len=%d\n",type,raw->len);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype,"HEMIS %2d (%4d):",type,raw->len);
    }
    switch (type) {
        case ID_CRESPOS   : return decode_crespos(raw);
        case ID_CRESRAW   : return decode_cresraw(raw);
        case ID_CRESRAW2  : return decode_cresraw2(raw);
        case ID_CRESEPH   : return decode_creseph(raw);
        case ID_CRESWAAS  : return decode_creswaas(raw);
        case ID_CRESIONUTC: return decode_cresionutc(raw);
        case ID_CRESGLORAW: return decode_cresgloraw(raw);
        case ID_CRESGLOEPH: return decode_cresgloeph(raw);
    }
    return 0;
}
/* sync code -----------------------------------------------------------------*/
static int sync_cres(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=buff[3]; buff[3]=data;
    return buff[0]==CRESSYNC[0]&&buff[1]==CRESSYNC[1]&&
           buff[2]==CRESSYNC[2]&&buff[3]==CRESSYNC[3];
}
/* input cresent raw message ---------------------------------------------------
* input next crescent raw message from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options, set raw->opt to the following option
*          strings separated by spaces.
*
*          -EPHALL      : input all ephemerides
*          -TTCORR      : time-tag offset correction
*          -ENAGLO      : enable glonass messages
*
*-----------------------------------------------------------------------------*/
extern int input_cres(raw_t *raw, unsigned char data)
{
    trace(5,"input_cres: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (!sync_cres(raw->buff,data)) return 0;
        raw->nbyte=4;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==8) {
        if ((raw->len=U2(raw->buff+6)+12)>MAXRAWLEN) {
            trace(2,"cresent length error: len=%d\n",raw->len);
            raw->nbyte=0;
            return -1;
        }
    }
    if (raw->nbyte<8||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode crescent raw message */
    return decode_cres(raw);
}
/* input crescent raw message from file ----------------------------------------
* input next crescent raw message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_cresf(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_cresf:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_cres(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+4,1,4,fp)<4) return -2;
    raw->nbyte=8;
    
    if ((raw->len=U2(raw->buff+6)+12)>MAXRAWLEN) {
        trace(2,"crescent length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+8,1,raw->len-8,fp)<(size_t)(raw->len-8)) return -2;
    raw->nbyte=0;
    
    /* decode crescent raw message */
    return decode_cres(raw);
}
