//---------------------------------------------------------------------------
// plotcmn.c: rtkplot common functions
//---------------------------------------------------------------------------
#include "rtklib.h"
#include "plotmain.h"

#define SQR(x)  ((x)*(x))

//---------------------------------------------------------------------------
extern "C" {
int showmsg(char *format,...) {return 0;}
}
//---------------------------------------------------------------------------
const char *PTypes[]={
    "Gnd Trk","Position","Velocity","Accel","NSat","Residuals",
    "Sat Vis","Skyplot","DOP/NSat","SNR/MP/EL","SNR/MP-EL","MP-Skyplot",""
};
// show message in status-bar -----------------------------------------------
void __fastcall TPlot::ShowMsg(AnsiString msg)
{
    Message1->Caption=msg;
    Panel21->Repaint();
}
// execute command ----------------------------------------------------------
int __fastcall TPlot::ExecCmd(AnsiString cmd)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    
    if (!CreateProcess(NULL,p,NULL,NULL,false,CREATE_NO_WINDOW,NULL,NULL,&si,
                       &info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
// get time span and time interval ------------------------------------------
void __fastcall TPlot::TimeSpan(gtime_t *ts, gtime_t *te, double *tint)
{
    gtime_t t0={0};
    
    trace(3,"TimeSpan\n");
    
    *ts=*te=t0; *tint=0.0;
    if (TimeEna[0]) *ts=TimeStart;
    if (TimeEna[1]) *te=TimeEnd;
    if (TimeEna[2]) *tint=TimeInt;
}
// get position regarding time ----------------------------------------------
double __fastcall TPlot::TimePos(gtime_t time)
{
    double tow;
    int week;
    
    if (TimeLabel<=1) { // www/ssss or gpst
        tow=time2gpst(time,&week);
    }
    else if (TimeLabel==2) { // utc
        tow=time2gpst(gpst2utc(time),&week);
    }
    else { // jst
        tow=time2gpst(timeadd(gpst2utc(time),9*3600.0),&week);
    }
    return tow+(week-Week)*86400.0*7;
}
// show legand in status-bar ------------------------------------------------
void __fastcall TPlot::ShowLegend(AnsiString *msgs)
{
    TLabel *ql[]={QL1,QL2,QL3,QL4,QL5,QL6,QL7};
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"ShowLegend\n");
    
    for (i=0;i<7;i++) {
        if (!msgs||msgs[i]=="") {
            ql[i]->Caption=" "; ql[i]->Width=1;
        }
        else {
            ql[i]->Caption=A2U(msgs[i]);
            ql[i]->Font->Color=MColor[sel][i+1];
        }
    }
}
// get current cursor position ----------------------------------------------
int __fastcall TPlot::GetCurrentPos(double *rr)
{
    sol_t *data;
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"GetCurrentPos\n");
    
    if (PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP) return 0;
    if (!(data=getsol(SolData+sel,SolIndex[sel]))) return 0;
    if (data->type) return 0;
    for (i=0;i<3;i++) rr[i]=data->rr[i];
    return 1;
}
// get center position of plot ----------------------------------------------
int __fastcall TPlot::GetCenterPos(double *rr)
{
    double xc,yc,opos[3],pos[3],enu[3]={0},dr[3];
    int i,j;
    
    trace(3,"GetCenterPos\n");
    
    if (PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP&&PlotType!=PLOT_TRK) return 0;
    if (norm(OPos,3)<=0.0) return 0;
    GraphT->GetCent(xc,yc);
    ecef2pos(OPos,opos);
    enu[0]=xc;
    enu[1]=yc;
    for (i=0;i<6;i++) {
        enu2ecef(opos,enu,dr);
        for (j=0;j<3;j++) rr[j]=OPos[j]+dr[j];
        ecef2pos(rr,pos);
        enu[2]-=pos[2];
    }
    return 1;
}
// get position, velocity or accel from solutions ---------------------------
TIMEPOS * __fastcall TPlot::SolToPos(solbuf_t *sol, int index, int qflag, int type)
{
    TIMEPOS *pos,*vel,*acc;
    gtime_t ts={0};
    sol_t *data;
    double tint,xyz[3],xyzs[4];
    int i;
    
    trace(3,"SolToPos: n=%d\n",sol->n);
    
    pos=new TIMEPOS(index<0?sol->n:3,1);
    
    if (index>=0) {
        if (type==1&&index>sol->n-2) index=sol->n-2;
        if (type==2&&index>sol->n-3) index=sol->n-3;
    }
    for (i=index<0?0:index;data=getsol(sol,i);i++) {
        
        tint=TimeEna[2]?TimeInt:0.0;
        
        if (index<0&&!screent(data->time,ts,ts,tint)) continue;
        if (qflag&&data->stat!=qflag) continue;
        
        PosToXyz(data->time,data->rr,data->type,xyz);
        CovToXyz(data->rr,data->qr,data->type,xyzs);
        if (xyz[2]<-RE_WGS84) continue;
        
        pos->t  [pos->n]=data->time;
        pos->x  [pos->n]=xyz [0];
        pos->y  [pos->n]=xyz [1];
        pos->z  [pos->n]=xyz [2];
        pos->xs [pos->n]=xyzs[0]; // var x^2
        pos->ys [pos->n]=xyzs[1]; // var y^2
        pos->zs [pos->n]=xyzs[2]; // var z^2
        pos->xys[pos->n]=xyzs[3]; // cov xy
        pos->q  [pos->n]=data->stat;
        pos->n++;
        
        if (index>=0&&pos->n>=3) break;
    }
    if (type!=1&&type!=2) return pos; // position
    
    vel=pos->tdiff();
    delete pos;
    if (type==1) return vel; // velocity
    
    acc=vel->tdiff();
    delete vel;
    return acc; // acceleration
}
// get number of satellites, age and ratio from solutions -------------------
TIMEPOS * __fastcall TPlot::SolToNsat(solbuf_t *sol, int index, int qflag)
{
    TIMEPOS *ns;
    sol_t *data;
    int i;
    
    trace(3,"SolToNsat: n=%d\n",sol->n);
    
    ns=new TIMEPOS(index<0?sol->n:3,1);
    
    for (i=index<0?0:index;data=getsol(sol,i);i++) {
        
        if (qflag&&data->stat!=qflag) continue;
        
        ns->t[ns->n]=data->time;
        ns->x[ns->n]=data->ns;
        ns->y[ns->n]=data->age;
        ns->z[ns->n]=data->ratio;
        ns->q[ns->n]=data->stat;
        ns->n++;
        
        if (index>=0&&i>=2) break;
    }
    return ns;
}
// transform solution to xyz-terms ------------------------------------------
void __fastcall TPlot::PosToXyz(gtime_t time, const double *rr, int type,
                                double *xyz)
{
    double opos[3],pos[3],r[3],enu[3];
    int i;
    
    trace(4,"PosToXyz:\n");
    
    if (type==0) { // xyz
        for (i=0;i<3;i++) {
            opos[i]=OPos[i];
            if (time.time==0.0||OEpoch.time==0.0) continue;
            opos[i]+=OVel[i]*timediff(time,OEpoch);
        }
        for (i=0;i<3;i++) r[i]=rr[i]-opos[i];
        ecef2pos(opos,pos);
        ecef2enu(pos,r,enu);
        xyz[0]=enu[0];
        xyz[1]=enu[1];
        xyz[2]=enu[2];
    }
    else { // enu
        xyz[0]=rr[0];
        xyz[1]=rr[1];
        xyz[2]=rr[2];
    }
}
// transform covariance to xyz-terms ----------------------------------------
void __fastcall TPlot::CovToXyz(const double *rr, const float *qr, int type,
                                double *xyzs)
{
    double pos[3],P[9],Q[9];
    
    trace(4,"CovToXyz:\n");
    
    if (type==0) { // xyz
        ecef2pos(rr,pos);
        P[0]=qr[0];
        P[4]=qr[1];
        P[8]=qr[2];
        P[1]=P[3]=qr[3];
        P[5]=P[7]=qr[4];
        P[2]=P[6]=qr[5];
        covenu(pos,P,Q);
        xyzs[0]=Q[0];
        xyzs[1]=Q[4];
        xyzs[2]=Q[8];
        xyzs[3]=Q[1];
    }
    else { // enu
        xyzs[0]=qr[0];
        xyzs[1]=qr[1];
        xyzs[2]=qr[2];
        xyzs[3]=qr[3];
    }
}
// computes solution statistics ---------------------------------------------
void __fastcall TPlot::CalcStats(const double *x, int n,
    double ref, double &ave, double &std, double &rms)
{
    double sum=0.0,sumsq=0.0;
    int i;
    
    trace(3,"CalcStats: n=%d\n",n);
    
    if (n<=0) {
        ave=std=rms=0.0;
        return;
    }
    ave=std=rms=0.0;
    
    for (i=0;i<n;i++) {
        sum  +=x[i];
        sumsq+=x[i]*x[i];
    }
    ave=sum/n;
    std=n>1?SQRT((sumsq-2.0*sum*ave+ave*ave*n)/(n-1)):0.0;
    rms=SQRT((sumsq-2.0*sum*ref+ref*ref*n)/n);
}
// get system color ---------------------------------------------------------
TColor __fastcall TPlot::SysColor(int sat)
{
    switch (satsys(sat,NULL)) {
        case SYS_GPS: return MColor[0][1];
        case SYS_GLO: return MColor[0][2];
        case SYS_GAL: return MColor[0][3];
        case SYS_QZS: return MColor[0][4];
        case SYS_CMP: return MColor[0][5];
        case SYS_SBS: return MColor[0][6];
    }
    return MColor[0][0];
}
// get observation data color -----------------------------------------------
TColor __fastcall TPlot::ObsColor(const obsd_t *obs, double az, double el)
{
    TColor color=clBlack;
    AnsiString ObsType_Text;
    char *code="";
    int i;
    
    trace(4,"ObsColor\n");
    
    if (!SatSel[obs->sat-1]) return clBlack;
    
    if (PlotType==PLOT_SNR||PlotType==PLOT_SNRE) {
        ObsType_Text=ObsType2->Text;
        code=ObsType_Text.c_str()+1;
    }
    else if (ObsType->ItemIndex) {
        ObsType_Text=ObsType->Text;
        code=ObsType_Text.c_str()+1;
    }
    if (SimObs) {
        color=SysColor(obs->sat);
    }
    else if (*code) {
        for (i=0;i<NFREQ+NEXOBS;i++) {
            if (!strstr(code2obs(obs->code[i],NULL),code)) continue;
            color=SnrColor(obs->SNR[i]*0.25);
            break;
        }
        if (i>=NFREQ+NEXOBS) return clBlack;
    }
    else {
        if      (obs->L[0]!=0.0&&obs->L[1]!=0.0&&obs->L[2]) color=MColor[0][4];
        else if (obs->L[0]!=0.0&&obs->L[1]!=0.0) color=MColor[0][1];
        else if (obs->L[0]!=0.0&&obs->L[2]!=0.0) color=MColor[0][5];
        else if (obs->L[0]!=0.0) color=MColor[0][2];
        else if (obs->P[1]!=0.0) color=MColor[0][3];
        else if (obs->P[2]!=0.0) color=MColor[0][6];
        else return clBlack;
    }
    if (el<ElMask*D2R||(ElMaskP&&el<ElMaskData[(int)(az*R2D+0.5)])) {
        return HideLowSat?clBlack:MColor[0][0];
    }
    return color;
}
// get observation data color -----------------------------------------------
TColor __fastcall TPlot::SnrColor(double snr)
{
    unsigned int c1,c2,r1,r2,g1,g2,b1,b2;
    double a;
    int i;
    
    if (snr<25.0) return MColor[0][7];
    if (snr<27.5) return MColor[0][5];
    if (snr>47.5) return MColor[0][1];
    a=(snr-27.5)/5.0;
    i=(int)a; a-=i;
    c1=(unsigned int)MColor[0][4-i];
    c2=(unsigned int)MColor[0][5-i];
    r1=c1&0xFF; g1=(c1>>8)&0xFF; b1=(c1>>16)&0xFF;
    r2=c2&0xFF; g2=(c2>>8)&0xFF; b2=(c2>>16)&0xFF;
    r1=(unsigned int)(a*r1+(1.0-a)*r2)&0xFF;
    g1=(unsigned int)(a*g1+(1.0-a)*g2)&0xFF;
    b1=(unsigned int)(a*b1+(1.0-a)*b2)&0xFF;
    
    return (TColor)((b1<<16)+(g1<<8)+r1);
}
// get mp color -------------------------------------------------------------
TColor __fastcall TPlot::MpColor(double mp)
{
    TColor colors[5];
    unsigned int c1,c2,r1,r2,g1,g2,b1,b2;
    double a;
    int i;
    
    colors[4]=MColor[0][5]; /*      mp> 0.6 */
    colors[3]=MColor[0][4]; /*  0.6>mp> 0.2 */
    colors[2]=MColor[0][3]; /*  0.2>mp>-0.2 */
    colors[1]=MColor[0][2]; /* -0.2>mp>-0.6 */
    colors[0]=MColor[0][1]; /* -0.6>mp      */
     
    if (mp>= 0.6) return colors[4];
    if (mp<=-0.6) return colors[0];
    a=mp/0.4+0.6;
    i=(int)a; a-=i;
    c1=(unsigned int)colors[i  ];
    c2=(unsigned int)colors[i+1];
    r1=c1&0xFF; g1=(c1>>8)&0xFF; b1=(c1>>16)&0xFF;
    r2=c2&0xFF; g2=(c2>>8)&0xFF; b2=(c2>>16)&0xFF;
    r1=(unsigned int)(a*r1+(1.0-a)*r2)&0xFF;
    g1=(unsigned int)(a*g1+(1.0-a)*g2)&0xFF;
    b1=(unsigned int)(a*b1+(1.0-a)*b2)&0xFF;
    
    return (TColor)((b1<<16)+(g1<<8)+r1);
}
// search solution by xy-position in plot -----------------------------------
int __fastcall TPlot::SearchPos(int x, int y)
{
    sol_t *data;
    TPoint p(x,y);
    double xp,yp,xs,ys,r,xyz[3];
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"SearchPos: x=%d y=%d\n",x,y);
    
    if (!BtnShowTrack->Down||(!BtnSol1->Down&&!BtnSol2->Down)) return -1;
    
    GraphT->ToPos(p,xp,yp);
    GraphT->GetScale(xs,ys);
    r=(MarkSize/2+2)*xs;
    
    for (i=0;data=getsol(SolData+sel,i);i++) {
        if (QFlag->ItemIndex&&data->stat!=QFlag->ItemIndex) continue;
        
        PosToXyz(data->time,data->rr,data->type,xyz);
        if (xyz[2]<-RE_WGS84) continue;
        
        if (SQR(xp-xyz[0])+SQR(yp-xyz[1])<=SQR(r)) return i;
    }
    return -1;
}
// generate time-string -----------------------------------------------------
void __fastcall TPlot::TimeStr(gtime_t time, int n, int tsys, char *str)
{
    struct tm *t;
    char tstr[64],*label="";
    double tow;
    int week;
    
    if (TimeLabel==0) { // www/ssss
        tow=time2gpst(time,&week);
        sprintf(tstr,"%4d/%*.*fs",week,(n>0?6:5)+n,n,tow);
    }
    else if (TimeLabel==1) { // gpst
        time2str(time,tstr,n);
        label=" GPST";
    }
    else if (TimeLabel==2) { // utc
        time2str(gpst2utc(time),tstr,n);
        label=" UTC";
    }
    else { // lt
        time=gpst2utc(time);
        if (!(t=localtime(&time.time))) strcpy(tstr,"2000/01/01 00:00:00.0");
        else sprintf(tstr,"%04d/%02d/%02d %02d:%02d:%02d.%0*d",t->tm_year+1900,
                     t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,
                     n,(int)(time.sec*pow(10.0,n)));
        label=" LT";
    }
    sprintf(str,"%s%s",tstr,label);
}
// latitude/longitude/height string -----------------------------------------
AnsiString __fastcall TPlot::LatLonStr(const double *pos, int ndec)
{
    AnsiString s;
    double dms1[3],dms2[3];
    
    if (LatLonFmt==0) {
        s.sprintf("%*.*f" CHARDEG "%s %*.*f" CHARDEG "%s",
                  ndec+4,ndec,fabs(pos[0]*R2D),pos[0]<0.0?"S":"N",
                  ndec+5,ndec,fabs(pos[1]*R2D),pos[1]<0.0?"W":"E");
    }
    else {
        deg2dms(pos[0]*R2D,dms1,ndec-5);
        deg2dms(pos[1]*R2D,dms2,ndec-5);
        s.sprintf("%3.0f" CHARDEG "%02.0f'%0*.*f\"%s %4.0f" CHARDEG "%02.0f'%0*.*f\"%s",
                  fabs(dms1[0]),dms1[1],ndec-2,ndec-5,dms1[2],pos[0]<0.0?"S":"N",
                  fabs(dms2[0]),dms2[1],ndec-2,ndec-5,dms2[2],pos[1]<0.0?"W":"E");
    }
    return s;
}
// convert unicode to UTF-8 ansistring --------------------------------------
AnsiString __fastcall TPlot::U2A(UnicodeString str)
{
    AnsiString a_str(str);
    return a_str;
}
// convert UTF-8 ansi to unicodestring --------------------------------------
UnicodeString __fastcall TPlot::A2U(AnsiString str)
{
    wchar_t buff[256]={0};
    ::MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,buff,512);
    UnicodeString u_str(buff);
    return u_str;
}
//---------------------------------------------------------------------------
// time-taged xyz-position class implementation
//---------------------------------------------------------------------------

// constructor --------------------------------------------------------------
TIMEPOS::TIMEPOS(int nmax, int sflg)
{
    nmax_=nmax;
    n=0;
    t=new gtime_t[nmax];
    x=new double [nmax];
    y=new double [nmax];
    z=new double [nmax];
    if (sflg) {
        xs =new double [nmax];
        ys =new double [nmax];
        zs =new double [nmax];
        xys=new double [nmax];
    }
    else xs=ys=zs=xys=NULL;
    q=new int [nmax];
}
// destructor ---------------------------------------------------------------
TIMEPOS::~TIMEPOS()
{
    delete [] t;
    delete [] x;
    delete [] y;
    delete [] z;
    if (xs) {
        delete [] xs;
        delete [] ys;
        delete [] zs;
        delete [] xys;
    }
    delete [] q;
}
// xyz-position difference from previous ------------------------------------
TIMEPOS * TIMEPOS::tdiff(void)
{
    TIMEPOS *pos=new TIMEPOS(n,1);
    double tt;
    int i;
    
    for (i=0;i<n-1;i++) {
        
        tt=timediff(t[i+1],t[i]);
        
        if (tt==0.0||fabs(tt)>MAXTDIFF) continue;
        
        pos->t[pos->n]=timeadd(t[i],tt/2.0);
        pos->x[pos->n]=(x[i+1]-x[i])/tt;
        pos->y[pos->n]=(y[i+1]-y[i])/tt;
        pos->z[pos->n]=(z[i+1]-z[i])/tt;
        if (xs) {
            pos->xs [pos->n]=SQR(xs [i+1])+SQR(xs [i]);
            pos->ys [pos->n]=SQR(ys [i+1])+SQR(ys [i]);
            pos->zs [pos->n]=SQR(zs [i+1])+SQR(zs [i]);
            pos->xys[pos->n]=SQR(xys[i+1])+SQR(xys[i]);
        }
        pos->q[pos->n]=MAX(q[i],q[i+1]);
        pos->n++;
    }
    return pos;
}
// xyz-position difference between TIMEPOS ----------------------------------
TIMEPOS *TIMEPOS::diff(const TIMEPOS *pos2, int qflag)
{
    TIMEPOS *pos1=this,*pos=new TIMEPOS(MIN(n,pos2->n),1);
    double tt;
    int i,j,q;
    
    for (i=0,j=0;i<pos1->n&&j<pos2->n;i++,j++) {
        
        tt=timediff(pos1->t[i],pos2->t[j]);
        
        if      (tt<-TTOL) {j--; continue;}
        else if (tt> TTOL) {i--; continue;}
        
        pos->t[pos->n]=pos1->t[i];
        pos->x[pos->n]=pos1->x[i]-pos2->x[j];
        pos->y[pos->n]=pos1->y[i]-pos2->y[j];
        pos->z[pos->n]=pos1->z[i]-pos2->z[j];
        if (pos->xs) {
            pos->xs [pos->n]=SQR(pos1->xs [i])+SQR(pos2->xs [j]);
            pos->ys [pos->n]=SQR(pos1->ys [i])+SQR(pos2->ys [j]);
            pos->zs [pos->n]=SQR(pos1->zs [i])+SQR(pos2->zs [j]);
            pos->xys[pos->n]=SQR(pos1->xys[i])+SQR(pos2->xys[j]);
        }
        q=MAX(pos1->q[i],pos2->q[j]);
        
        if (!qflag||qflag==q) {
            pos->q[pos->n++]=q;
        }
    }
    return pos;
}
//---------------------------------------------------------------------------
