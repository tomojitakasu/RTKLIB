//---------------------------------------------------------------------------
// plotcmn.c: rtkplot common functions
//---------------------------------------------------------------------------
#include <QString>
#include <QProcess>
#include <QColor>
#include <QStringList>
#include <QDebug>

#include "plotmain.h"

#include "rtklib.h"

#define SQR(x)  ((x)*(x))

QString color2String(const QColor &c){
    return QString("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}

//---------------------------------------------------------------------------
extern "C" {
int showmsg(char *format,...) {Q_UNUSED(format); return 0;}
}
//---------------------------------------------------------------------------
const QString PTypes[]={
    QT_TR_NOOP("Gnd Trk"),QT_TR_NOOP("Position"),QT_TR_NOOP("Velocity"),QT_TR_NOOP("Accel"),QT_TR_NOOP("NSat"),QT_TR_NOOP("Residuals"),
    QT_TR_NOOP("Sat Vis"),QT_TR_NOOP("Skyplot"),QT_TR_NOOP("DOP/NSat"),QT_TR_NOOP("SNR/MP/EL"),QT_TR_NOOP("SNR/MP-EL"),QT_TR_NOOP("MP-Skyplot"),""
};
// show message in status-bar -----------------------------------------------
void Plot::ShowMsg(const QString &msg)
{
    Message1->setText(msg);
    Message1->adjustSize();
    Panel21->updateGeometry();
}
// execute command ----------------------------------------------------------
int Plot::ExecCmd(const QString &cmd)
{
    return QProcess::startDetached(cmd);
}
// get time span and time interval ------------------------------------------
void Plot::TimeSpan(gtime_t *ts, gtime_t *te, double *tint)
{
    gtime_t t0={0,0};
    
    trace(3,"TimeSpan\n");
    
    *ts=*te=t0; *tint=0.0;
    if (TimeEna[0]) *ts=TimeStart;
    if (TimeEna[1]) *te=TimeEnd;
    if (TimeEna[2]) *tint=TimeInt;
}
// get position regarding time ----------------------------------------------
double Plot::TimePos(gtime_t time)
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
void Plot::ShowLegend(QString msgs[])
{
    QLabel *ql[]={QL1,QL2,QL3,QL4,QL5,QL6,QL7};
    int i,sel=!BtnSol1->isChecked()&&BtnSol2->isChecked()?1:0;
    
    trace(3,"ShowLegend\n");
    
    for (i=0;i<7;i++) {
        if (!msgs||msgs[i]==NULL) {
            ql[i]->setText("");
            ql[i]->adjustSize();
        }
        else {
            ql[i]->setText(msgs[i]);
            ql[i]->adjustSize();
            ql[i]->setStyleSheet(QString("QLabel {color: %1;}").arg(color2String(MColor[sel][i+1])));
        }
    }
    Panel21->updateGeometry();
}
// get current cursor position ----------------------------------------------
int Plot::GetCurrentPos(double *rr)
{
    sol_t *data;
    int i,sel=!BtnSol1->isChecked()&&BtnSol2->isChecked()?1:0;
    
    trace(3,"GetCurrentPos\n");
    
    if (PLOT_OBS<=PlotType&&PlotType<=PLOT_DOP) return 0;
    if (!(data=getsol(SolData+sel,SolIndex[sel]))) return 0;
    if (data->type) return 0;

    for (i=0;i<3;i++) rr[i]=data->rr[i];

    return 1;
}
// get center position of plot ----------------------------------------------
int Plot::GetCenterPos(double *rr)
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
TIMEPOS * Plot::SolToPos(solbuf_t *sol, int index, int qflag, int type)
{
    TIMEPOS *pos,*vel,*acc;
    gtime_t ts={0,0};
    sol_t *data;
    double tint,xyz[3],xyzs[4];
    int i;
    
    trace(3,"SolToPos: n=%d\n",sol->n);
    
    pos=new TIMEPOS(index<0?sol->n:3,1);
    
    if (index>=0) {
        if (type==1&&index>sol->n-2) index=sol->n-2;
        if (type==2&&index>sol->n-3) index=sol->n-3;
    }

    tint=TimeEna[2]?TimeInt:0.0;

    for (i=index<0?0:index;(data=getsol(sol,i))!=NULL;i++) {
        
        if (index<0&&!screent(data->time,ts,ts,tint)) continue;
        if (qflag&&data->stat!=qflag) continue;
        
        PosToXyz(data->time,data->rr,data->type,xyz);
        CovToXyz(data->rr,data->qr,data->type,xyzs);
        
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
TIMEPOS * Plot::SolToNsat(solbuf_t *sol, int index, int qflag)
{
    TIMEPOS *ns;
    sol_t *data;
    int i;
    
    trace(3,"SolToNsat: n=%d\n",sol->n);
    
    ns=new TIMEPOS(index<0?sol->n:3,1);
    
    for (i=index<0?0:index;(data=getsol(sol,i))!=NULL;i++) {
        
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
void Plot::PosToXyz(gtime_t time, const double *rr, int type,
                                double *xyz)
{
    double opos[3],pos[3],r[3],enu[3];
    int i;
    
    trace(4,"SolToXyz:\n");
    
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
void Plot::CovToXyz(const double *rr, const float *qr, int type,
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
void Plot::CalcStats(const double *x, int n,
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
QColor Plot::SysColor(int sat)
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
QColor Plot::ObsColor(const obsd_t *obs, double az, double el)
{
    QColor color=Qt::black;
    QString ObsType_Text;
    char code[16];
    int i;
    
    code[0]='\0';

    trace(4,"ObsColor\n");
    
    if (!SatSel[obs->sat-1]) return Qt::black;
    
    if (PlotType==PLOT_SNR||PlotType==PLOT_SNRE) {
        ObsType_Text=ObsType2->currentText();
        strcpy(code,qPrintable(ObsType_Text.mid(1)));
    }
    else if (ObsType->currentIndex()!=0) {
        ObsType_Text=ObsType->currentText();
        strcpy(code,qPrintable(ObsType_Text.mid(1)));
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
        if (i>=NFREQ+NEXOBS) return Qt::black;
    }
    else {
        if      (obs->L[0]!=0.0&&obs->L[1]!=0.0&&obs->L[2]!=0.0) color=MColor[0][4];
        else if (obs->L[0]!=0.0&&obs->L[1]!=0.0) color=MColor[0][1];
        else if (obs->L[0]!=0.0&&obs->L[2]!=0.0) color=MColor[0][5];
        else if (obs->L[0]!=0.0) color=MColor[0][2];
        else if (obs->P[1]!=0.0) color=MColor[0][3];
        else if (obs->P[2]!=0.0) color=MColor[0][6];
        else return Qt::black;
    }
    if (el<ElMask*D2R||(ElMaskP&&el<ElMaskData[static_cast<int>(az*R2D+0.5)])) {
        return HideLowSat?Qt::black:MColor[0][0];
    }
    return color;
}
// get observation data color -----------------------------------------------
QColor Plot::SnrColor(double snr)
{
    QColor c1,c2;
    unsigned int r1,b1,g1;
    double a;
    int i;
    
    if (snr<25.0) return MColor[0][7];
    if (snr<27.5) return MColor[0][5];
    if (snr>47.5) return MColor[0][1];
    a=(snr-27.5)/5.0;
    i=static_cast<int>(a);
    a-=i;
    c1=MColor[0][4-i];
    c2=MColor[0][5-i];
    r1=static_cast<unsigned int>(a*c1.red()+(1.0-a)*c2.red())&0xFF;
    g1=static_cast<unsigned int>(a*c1.green()+(1.0-a)*c2.green())&0xFF;
    b1=static_cast<unsigned int>(a*c1.blue()+(1.0-a)*c2.blue())&0xFF;
    
    return QColor(r1,g1,b1);
}
// get mp color -------------------------------------------------------------
QColor Plot::MpColor(double mp)
{
    QColor colors[5];
    QColor c1,c2;
    unsigned int r1,b1,g1;
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
    i=static_cast<int>(a);
    a-=i;
    c1=colors[i  ];
    c2=colors[i+1];
    r1=static_cast<unsigned int>(a*c1.red()+(1.0-a)*c2.red())&0xFF;
    g1=static_cast<unsigned int>(a*c1.green()+(1.0-a)*c2.green())&0xFF;
    b1=static_cast<unsigned int>(a*c1.blue()+(1.0-a)*c2.blue())&0xFF;

    return QColor(r1,g1,b1);
}
// search solution by xy-position in plot -----------------------------------
int Plot::SearchPos(int x, int y)
{
    sol_t *data;
    QPoint p(x,y);
    double xp,yp,xs,ys,r,xyz[3];
    int i,sel=!BtnSol1->isChecked()&&BtnSol2->isChecked()?1:0;
    
    trace(3,"SearchPos: x=%d y=%d\n",x,y);
    
    if (!BtnShowTrack->isChecked()||(!BtnSol1->isChecked()&&!BtnSol2->isChecked())) return -1;
    
    GraphT->ToPos(Disp->mapFromGlobal(p),xp,yp);
    GraphT->GetScale(xs,ys);
    r=(MarkSize/2+2)*xs;
    
    for (i=0;(data=getsol(SolData+sel,i))!=NULL;i++) {
        if (QFlag->currentIndex()&&data->stat!=QFlag->currentIndex()) continue;
        
        PosToXyz(data->time,data->rr,data->type,xyz);
        
        if (SQR(xp-xyz[0])+SQR(yp-xyz[1])<=SQR(r)) return i;
    }
    return -1;
}
// generate time-string -----------------------------------------------------
void Plot::TimeStr(gtime_t time, int n, int tsys, QString &str)
{
    struct tm *t;
    char tstr[64];
    QString label="";
    double tow;
    int week;
    Q_UNUSED(tsys);
    
    if (TimeLabel==0) { // www/ssss
        tow=time2gpst(time,&week);
        strcpy(tstr,qPrintable(QString("%1/%2").arg(week,4).arg(tow,(n>0?6:5)+n,'f',n)));
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
        else strcpy(tstr,qPrintable(QString("%1/%2/%3 %4:%5:%6.%7").arg(t->tm_year+1900,4,10,QChar('0'))
                     .arg(t->tm_mon+1,2,10,QChar('0')).arg(t->tm_mday,2,10,QChar('0')).arg(t->tm_hour,2,10,QChar('0')).arg(t->tm_min,2,10,QChar('0'))
                     .arg(t->tm_sec,2,10,QChar('0')).arg(static_cast<int>(time.sec*pow(10.0,n)),n,10)));
        label=" LT";
    }
    str=tstr+label;
}
// latitude/longitude/height string -----------------------------------------
QString Plot::LatLonStr(const double *pos, int ndec)
{
    QString s;
    double dms1[3],dms2[3];
    
    if (LatLonFmt==0) {
        s=QStringLiteral("%1%2%3 %4%5%6").arg(fabs(pos[0]*R2D),ndec+4,'f',ndec).arg(degreeChar).arg(pos[0]<0.0?"S":"N")
                  .arg(fabs(pos[1]*R2D),ndec+5,'f',ndec).arg(degreeChar).arg(pos[1]<0.0?"W":"E");

    }
    else {
        deg2dms(pos[0]*R2D,dms1);
        deg2dms(pos[1]*R2D,dms2);
        s=QStringLiteral("%1%2 %3' %4\" %5 %6%7 %8' %9\" %10")
                  .arg(fabs(dms1[0]),3,'f',0).arg(degreeChar).arg(dms1[1],2,'f',0,QChar('0')).arg(dms1[2],ndec-2,'f',ndec-5,QChar('0')).arg(pos[0]<0.0?"S":"N")
                  .arg(fabs(dms2[0]),4,'f',0).arg(degreeChar).arg(dms2[1],2,'f',0,QChar('0')).arg(dms2[2],ndec-2,'f',ndec-5,QChar('0')).arg(pos[1]<0.0?"W":"E");
    }
    return s;
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
