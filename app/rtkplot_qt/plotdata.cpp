//---------------------------------------------------------------------------
// plotdata : rtkplot data functions
//---------------------------------------------------------------------------
#include <QString>
#include <QStringList>
#include <QFile>
#include <QImage>
#include <QColor>
#include <QDir>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "rtklib.h"
#include "plotmain.h"
#include "mapdlg.h"
#include "pntdlg.h"
#include "geview.h"

#define MAX_SIMOBS	16384			// max genrated obs epochs
#define MAX_SKYIMG_R 2048			// max size of resampled sky image

#define THRES_SLIP  2.0             // threshold of cycle-slip


static char path_str[MAXNFILE][1024];
static const char *XMLNS="http://www.topografix.com/GPX/1/1";

// read solutions -----------------------------------------------------------
void Plot::ReadSol(const QStringList &files, int sel)
{
    solbuf_t sol;
    gtime_t ts,te;
    double tint;
    int i,n=0;
    char *paths[MAXNFILE];
    
    trace(3,"ReadSol: sel=%d\n",sel);

    setlocale(LC_NUMERIC,"C"); // use point as decimal separator in formated output

    memset(&sol,0,sizeof(solbuf_t));
    
    for (i=0;i<MAXNFILE;i++) paths[i]=path_str[i];
    
    if (files.count()<=0) return;
    
    ReadWaitStart();
    
    for (i=0;i<files.count()&&n<MAXNFILE;i++) {
        strcpy(paths[n++],qPrintable(files.at(i)));
    }
    TimeSpan(&ts,&te,&tint);
    
    ShowMsg(QString("reading %1...").arg(paths[0]));
    ShowLegend(NULL);
    
    if (!readsolt(paths,n,ts,te,tint,0,&sol)) {
        ShowMsg(QString("no solution data : %1...").arg(paths[0]));
        ShowLegend(NULL);
        ReadWaitEnd();
        return;
    }
    freesolbuf(SolData+sel);
    SolData[sel]=sol;
    
    if (SolFiles[sel]!=files) {
        SolFiles[sel]=files;
    }
    setWindowTitle("");
    
    ReadSolStat(files,sel);
    
    for (i=0;i<2;i++) {
        if (SolFiles[i].length()==0) continue;
        setWindowTitle(windowTitle()+SolFiles[i].at(0)+(SolFiles[i].count()>1?"... ":" "));
    }
    BtnSol12->setChecked(false);
    if (sel==0) BtnSol1->setChecked(true);
    else        BtnSol2->setChecked(true);
    
    if (sel==0||SolData[0].n<=0) {
        time2gpst(SolData[sel].data[0].time,&Week);
        UpdateOrigin();
    }
    SolIndex[0]=SolIndex[1]=ObsIndex=0;
    
    GEDataState[sel]=0;
    
    if (PlotType>PLOT_NSAT) {
        UpdateType(PLOT_TRK);
    }
    else {
        UpdatePlotType();
    }
    FitTime();
    if (AutoScale&&PlotType<=PLOT_SOLA) {
        FitRange(1);
    }
    else {
        SetRange(1,YRange);
    }
    ReadWaitEnd();
    
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// read solution status -----------------------------------------------------
void Plot::ReadSolStat(const QStringList &files, int sel)
{
    gtime_t ts,te;
    double tint;
    int i,n=0;
    char *paths[MAXNFILE];
    
    trace(3,"ReadSolStat\n");
    
    setlocale(LC_NUMERIC,"C"); // use point as decimal separator in formated output

    freesolstatbuf(SolStat+sel);
    
    for (i=0;i<MAXNFILE;i++) paths[i]=path_str[i];
    
    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<files.count()&&n<MAXNFILE;i++) {
        strcpy(paths[n++],qPrintable(files.at(i)));
    }
    ShowMsg(QString("reading %1...").arg(paths[0]));
    ShowLegend(NULL);
    
    readsolstatt(paths,n,ts,te,tint,SolStat+sel);
    
    UpdateSatList();
}
// read observation data ----------------------------------------------------
void Plot::ReadObs(const QStringList &files)
{
    obs_t obs={0,0,NULL};
    nav_t nav;
    sta_t sta;
    int nobs;
    
    trace(3,"ReadObs\n");
    
    setlocale(LC_NUMERIC,"C"); // use point as decimal separator in formated output

    memset(&nav,0,sizeof(nav_t));
    memset(&sta,0,sizeof(sta_t));

    if (files.size()==0) return;
    
    ReadWaitStart();
    ShowLegend(NULL);
    
    if ((nobs=ReadObsRnx(files,&obs,&nav,&sta))<=0) {
        ReadWaitEnd();
        return;
    }
    ClearObs();

    Obs=obs;
    Nav=nav;
    Sta=sta;
    SimObs=0;

    UpdateObs(nobs);
    UpdateMp();
    
    if (ObsFiles!=files) {
        ObsFiles=files;
    }
    NavFiles.clear();
    
    setWindowTitle(files.at(0) + (files.size()>1?"...":""));
    
    BtnSol1->setChecked(true);

    time2gpst(Obs.data[0].time,&Week);
    SolIndex[0]=SolIndex[1]=ObsIndex=0;
    
    if (PlotType<PLOT_OBS||PLOT_DOP<PlotType) {
        UpdateType(PLOT_OBS);
    }
    else {
        UpdatePlotType();
    }
    FitTime();
    
    ReadWaitEnd();
    UpdateObsType();
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// read observation data rinex ----------------------------------------------
int Plot::ReadObsRnx(const QStringList &files, obs_t *obs, nav_t *nav,
                                 sta_t *sta)
{
    gtime_t ts,te;
    double tint;
    int i,n;
    char obsfile[1024],navfile[1024]="",*p,*q,opt[2048];
    strcpy(opt,qPrintable(RnxOpts));
    
    trace(3,"ReadObsRnx\n");
    
    setlocale(LC_NUMERIC,"C"); // use point as decimal separator in formated output

    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<files.count();i++) {
        strcpy(obsfile,qPrintable(QDir::toNativeSeparators(files.at(i))));
        
        ShowMsg(QString(tr("reading obs data... %1")).arg(obsfile));
        qApp->processEvents();

        if (readrnxt(obsfile,1,ts,te,tint,opt,obs,nav,sta)<0) {
            ShowMsg(tr("error: insufficient memory"));
            return -1;
        }
    }
    ShowMsg(tr("reading nav data..."));
    qApp->processEvents();

    for (i=0;i<files.count();i++) {
        strcpy(navfile,qPrintable(QDir::toNativeSeparators(files.at(i))));
        
        if (!(p=strrchr(navfile,'.'))) continue;
        
        if (!strcmp(p,".obs")||!strcmp(p,".OBS")) {
            strcpy(p,".nav" ); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".gnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".hnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".qnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".lnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
        }
        else if (!strcmp(p+3,"o" )||!strcmp(p+3,"d" )||
                 !strcmp(p+3,"O" )||!strcmp(p+3,"D" )) {
            n=nav->n;
            
            strcpy(p+3,"N"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"G"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"H"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"Q"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"L"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"P"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            
            if (nav->n>n||!(q=strrchr(navfile,'\\'))) continue;
            
            // read brdc navigation data
            memcpy(q+1,"BRDC",4);
            strcpy(p+3,"N"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
        }
    }
    if (obs->n<=0) {
        ShowMsg(QString(tr("no observation data: %1...")).arg(files.at(0)));
        qApp->processEvents();
        freenav(nav,0xFF);
        return 0;
    }
    uniqnav(nav);
    return sortobs(obs);
}
// read navigation data -----------------------------------------------------
void Plot::ReadNav(const QStringList &files)
{
    gtime_t ts,te;
    double tint;
    char navfile[1024],opt[2048];
    int i;
    strcpy(opt,qPrintable(RnxOpts));

    trace(3,"ReadNav\n");
    
    if (files.size()<=0) return;
    
    setlocale(LC_NUMERIC,"C"); // use point as decimal separator in formated output

    ReadWaitStart();
    ShowLegend(NULL);
    
    TimeSpan(&ts,&te,&tint);
    
    freenav(&Nav,0xFF);
    
    ShowMsg(tr("reading nav data..."));

    qApp->processEvents();
    
    for (i=0;i<files.size();i++) {
        strcpy(navfile,qPrintable(QDir::toNativeSeparators(files.at(i))));
        readrnxt(navfile,1,ts,te,tint,opt,NULL,&Nav,NULL);
    }
    uniqnav(&Nav);
    
    if (Nav.n<=0&&Nav.ng<=0&&Nav.ns<=0) {
        ShowMsg(QString(tr("no nav message: %1...")).arg(QDir::toNativeSeparators(files.at(i))));
        ReadWaitEnd();
        return;
    }
    if (NavFiles!=files) {
        NavFiles=files;
    }
    for (i=0;i<NavFiles.size();i++) NavFiles[i]=QDir::toNativeSeparators(NavFiles.at(i));

    UpdateObs(NObs);
    UpdateMp();
    ReadWaitEnd();
    
    UpdatePlot();
    UpdateEnable();
}
// read elevation mask data -------------------------------------------------
void Plot::ReadElMaskData(const QString &file)
{
    QFile fp(file);
    double az0=0.0,el0=0.0,az1,el1;
    int i,j;
    QByteArray buff;
    
    trace(3,"ReadElMaskData\n");
    
    for (i=0;i<=360;i++) ElMaskData[i]=0.0;
    
    if (!fp.open(QIODevice::ReadOnly)) {
        ShowMsg(QString(tr("no el mask data: %1...")).arg(file));
        ShowLegend(NULL);
        return;
    }
    while (!fp.atEnd()) {
        buff=fp.readLine();

        if (buff.at(0)=='%') continue;
        QList<QByteArray> tokens=buff.split(' ');
        if (tokens.size()!=2) continue;
        bool okay;
        az1=tokens.at(0).toDouble(&okay); if (!okay) continue;
        el1=tokens.at(1).toDouble(&okay); if (!okay) continue;
        
        if (az0<az1&&az1<=360.0&&0.0<=el1&&el1<=90.0) {
            
            for (j=static_cast<int>(az0);j<static_cast<int>(az1);j++) ElMaskData[j]=el0*D2R;
            ElMaskData[j]=el1*D2R;
        }
        az0=az1; el0=el1;
    }
    UpdatePlot();
    UpdateEnable();
}
// generate visibility data ----------------------------------------------------
void Plot::GenVisData(void)
{
    gtime_t time,ts,te;
    obsd_t data;
    double tint,pos[3],rr[3],rs[6],e[3],azel[2];
    unsigned char i,j;
    int nobs=0;
    char name[16];
    
    trace(3,"GenVisData\n");
    
    memset(&data,0,sizeof(obsd_t));

    ClearObs();
    SimObs=1;
    
    ts=TimeStart;
    te=TimeEnd;
    tint=TimeInt;
    matcpy(pos,OOPos,3,1);
    pos2ecef(pos,rr);
    
    ReadWaitStart();
    ShowLegend(NULL);
    ShowMsg(tr("generating satellite visibility..."));
    qApp->processEvents();
    
    for (time=ts;timediff(time,te)<=0.0;time=timeadd(time,tint)) {
        for (i=0;i<MAXSAT;i++) {
            satno2id(i+1,name);
            if (!tle_pos(time,name,"","",&TLEData,NULL,rs)) continue;
            if ((geodist(rs,rr,e))<=0.0) continue;
            if (satazel(pos,e,azel)<=0.0) continue;
            if (Obs.n>=Obs.nmax) {
                Obs.nmax=Obs.nmax<=0?4096:Obs.nmax*2;
                Obs.data=static_cast<obsd_t *>(realloc(Obs.data,sizeof(obsd_t)*Obs.nmax));
                if (!Obs.data) {
                    Obs.n=Obs.nmax=0;
                    break;
                }
            }
            data.time=time;
            data.sat=i+1;
            
            for (j=0;j<NFREQ;j++) {
                data.P[j]=data.L[j]=0.0;
                data.code[j]=CODE_NONE;
            }
            data.code[0]=CODE_L1C;
            Obs.data[Obs.n++]=data;
        }
        if (++nobs>=MAX_SIMOBS) break;
    }
    if (Obs.n<=0) {
        ReadWaitEnd();
        ShowMsg(tr("no satellite visibility"));
        return;
    }
    UpdateObs(nobs);
    
    setWindowTitle(tr("Satellite Visibility (Predicted)"));
    BtnSol1->setChecked(true);
    time2gpst(Obs.data[0].time,&Week);
    SolIndex[0]=SolIndex[1]=ObsIndex=0;
    if (PlotType<PLOT_OBS||PLOT_DOP<PlotType) {
        UpdateType(PLOT_OBS);
    }
    else {
        UpdatePlotType();
    }
    FitTime();
    ReadWaitEnd();
    UpdateObsType();
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// read map image data ------------------------------------------------------
void Plot::ReadMapData(const QString &file)
{
    QImage image;
    
    trace(3,"ReadMapData\n");

    ShowMsg(QString("reading map image... %1").arg(file));
    
    if (!image.load(file)) {
        ShowMsg(QString(tr("map file read error: %1")).arg(file));
        ShowLegend(NULL);
        return;
    }
    MapImage=image;
    MapImageFile=file;
    MapSize[0]=MapImage.width();
    MapSize[1]=MapImage.height();
    
    ReadMapTag(file);
    
    BtnShowImg->setChecked(true);
    
    mapAreaDialog->UpdateField();
    UpdatePlot();
    UpdateOrigin();
    UpdateEnable();
    ShowMsg("");
}
// resample image pixel -----------------------------------------------------
#define ResPixelNN(img1,x,y,b1,pix) {\
    int ix=static_cast<int>((x)+0.5),iy=static_cast<int>((y)+0.5);\
    pix=img1.pixel(ix,iy);\
}
#define ResPixelBL(img1,x,y,b1,pix) {\
    int ix=static_cast<int>(x),iy=static_cast<int>(y);\
    double dx1=(x)-ix,dy1=(y)-iy,dx2=1.0-dx1,dy2=1.0-dy1;\
    double a1=dx2*dy2,a2=dx2*dy1,a3=dx1*dy2,a4=dx1*dy1;\
    QRgb p1=img1.pixel(ix,iy); \
    QRgb p2=img1.pixel(ix,iy+1); \
    pix=qRgb(static_cast<quint8>(a1*qRed(p1)+a2*qRed(p2)  +a3*qRed(p1)  +a4*qRed(p2)),\
             static_cast<quint8>(a1*qRed(p1)+a2*qGreen(p2)+a3*qGreen(p1)+a4*qGreen(p2)),\
             static_cast<quint8>(a1*qRed(p1)+a2*qBlue(p2) +a3*qBlue(p1) +a4*qBlue(p2)));\
}
// rotate coordintates roll-pitch-yaw ---------------------------------------
static void RPY(const double *rpy, double *R)
{
    double sr=sin(-rpy[0]*D2R),cr=cos(-rpy[0]*D2R);
    double sp=sin(-rpy[1]*D2R),cp=cos(-rpy[1]*D2R);
    double sy=sin(-rpy[2]*D2R),cy=cos(-rpy[2]*D2R);
    R[0]=cy*cr-sy*sp*sr; R[1]=-sy*cp; R[2]=cy*sr+sy*sp*cr;
    R[3]=sy*cr+cy*sp*sr; R[4]=cy*cp;  R[5]=sy*sr-cy*sp*cr;
    R[6]=-cp*sr;         R[7]=sp;     R[8]=cp*cr;
}
// RGB -> YCrCb (ITU-R BT.601) ----------------------------------------------
static void YCrCb(const QRgb *pix, double *Y)
{
    //         R(0-255)     G(0-255)     B(0-255)
    Y[0]=( 0.299*qRed(*pix)+0.587*qGreen(*pix)+0.114*qBlue(*pix))/255; // Y  (0-1)
    Y[1]=( 0.500*qRed(*pix)-0.419*qGreen(*pix)+0.081*qBlue(*pix))/255; // Cr (-.5-.5)
    Y[2]=(-0.169*qRed(*pix)-0.331*qGreen(*pix)+0.500*qBlue(*pix))/255; // Cb (-.5-.5)
}
// update sky image ---------------------------------------------------------
void Plot::UpdateSky(void)
{
    QImage &bm1=SkyImageI,&bm2=SkyImageR;
    QRgb pix;
    double x,y,xp,yp,r,a,p[3],q[3],R[9]={0},dr,dist,Yz[3]={0},Y[3];
    int i,j,k,w1,h1,w2,h2,wz,nz=0;
    

    w1=bm1.width(); h1=bm1.height();
    w2=bm2.width(); h2=bm2.height();
    
    if (w1<=0||h1<=0||w2<=0||h2<=0) return;
    
    bm2.fill(QColor("silver")); // fill bitmap by silver
    
    if (norm(SkyFov,3)>1e-12) {
        RPY(SkyFov,R);
    }
    if (SkyBinarize) { // average of zenith image
        wz=h1/16; // sky area size
        for (i=w1/2-wz;i<=w1/2+wz;i++) for (j=h1/2-wz;j<=h1/2+wz;j++) {
            pix=bm1.pixel(i,j);
            YCrCb(&pix,Y);
            for (k=0;k<3;k++) Yz[k]+=Y[k];
            nz++;
        }
        if (nz>0) {
            for (k=0;k<3;k++) Yz[k]/=nz;
        }
    }
    for (j=0;j<h2;j++) for (i=0;i<w2;i++) {
        xp=(w2/2.0-i)/SkyScaleR;
        yp=(j-h2/2.0)/SkyScaleR;
        r=sqrt(SQR(xp)+SQR(yp));
        if (SkyElMask&&r>1.0) continue;
        
        // rotate coordinates roll-pitch-yaw
        if (norm(SkyFov,3)>1e-12) {
            if (r<1e-12) {
                p[0]=p[1]=0.0;
                p[2]=1.0;
            }
            else {
                a=sin(r*PI/2.0);
                p[0]=a*xp/r;
                p[1]=a*yp/r;
                p[2]=cos(r*PI/2.0);
            }
            q[0]=R[0]*p[0]+R[3]*p[1]+R[6]*p[2];
            q[1]=R[1]*p[0]+R[4]*p[1]+R[7]*p[2];
            q[2]=R[2]*p[0]+R[5]*p[1]+R[8]*p[2];
            if (q[2]>=1.0) {
                xp=yp=r=0.0;
            }
            else {
                r=acos(q[2])/(PI/2.0);
                a=sqrt(SQR(q[0])+SQR(q[1]));
                xp=r*q[0]/a;
                yp=r*q[1]/a;
            }
        }
        // correct lense distortion
        if (SkyDestCorr) {
            if (r<=0.0||r>=1.0) continue;
            k=static_cast<int>(r*9.0);
            dr=r*9.0-k;
            dist=k>8?SkyDest[9]:(1.0-dr)*SkyDest[k]+dr*SkyDest[k+1];
            xp*=dist/r;
            yp*=dist/r;
        }
        else {
            xp*=SkyScale;
            yp*=SkyScale;
        }
        if (SkyFlip) xp=-xp;
        x=SkyCent[0]+xp;
        y=SkyCent[1]+yp;
        if (x<0.0||x>=w1-1||y<0.0||y>=h1-1) continue;
        if (!SkyRes) {
            ResPixelNN(bm1,x,y,b1,pix)
        }
        else {
            ResPixelBL(bm1,x,y,b1,pix)
        }
        bm2.setPixel(i,j,pix);
        if (SkyBinarize) {
            YCrCb(&pix,Y);
            for (k=1;k<3;k++) Y[k]-=Yz[k];
            
            // threshold by brightness and color-distance
            if (Y[0]>SkyBinThres1&&norm(Y+1,2)<SkyBinThres2) {
                bm2.setPixel(i,j,qRgb(255,255,255)); // sky
            }
            else {
                bm2.setPixel(i,j,qRgb(96,96,96)); // others
            }
        }
    }
    UpdatePlot();
}
// read sky tag data --------------------------------------------------------
void Plot::ReadSkyTag(const QString &file)
{
    QFile fp(file);
    QByteArray buff;
    
    trace(3,"ReadSkyTag\n");
    
    if (!fp.open(QIODevice::ReadOnly)) return;
    
    while (!fp.atEnd()) {
        buff=fp.readLine();
        if (buff.at(0)=='\0'||buff.at(0)=='%'||buff.at(0)=='#') continue;
        QList<QByteArray> tokens=buff.split('=');
        if (tokens.size()<2) continue;

        if      (tokens.at(0)=="centx")  SkyCent[0]=tokens.at(1).toDouble();
        else if (tokens.at(0)=="centy")  SkyCent[1]=tokens.at(1).toDouble();
        else if (tokens.at(0)=="scale")  SkyScale=tokens.at(1).toDouble();
        else if (tokens.at(0)=="roll")   SkyFov[0]=tokens.at(1).toDouble();
        else if (tokens.at(0)=="pitch")  SkyFov[1]=tokens.at(1).toDouble();
        else if (tokens.at(0)=="yaw")    SkyFov[2]=tokens.at(1).toDouble();
        else if (tokens.at(0)=="destcorr")SkyDestCorr=tokens.at(1).toInt();
        else if (tokens.at(0)=="elmask")   SkyElMask=tokens.at(1).toInt();
        else if (tokens.at(0)=="resample")   SkyRes=tokens.at(1).toInt();
        else if (tokens.at(0)=="flip")   SkyFlip=tokens.at(1).toInt();
        else if (tokens.at(0)=="dest") {
            QList<QByteArray> t=tokens.at(1).split(' ');
            if (t.size()==9)
                for (int i=0;i<9;i++) SkyDest[i]=t.at(i).toDouble();
        }
        else if (tokens.at(0)=="binarize")   SkyBinarize=tokens.at(1).toInt();
        else if (tokens.at(0)=="binthr1")   SkyBinThres1=tokens.at(1).toInt();
        else if (tokens.at(0)=="binthr2")   SkyBinThres2=tokens.at(1).toInt();
    }
}
// read sky image data ------------------------------------------------------
void Plot::ReadSkyData(const QString &file)
{
    QImage image;
    int i,w,h,wr;
    
    trace(3,"ReadSkyData\n");
    
    ShowMsg(QString("reading sky data... %1").arg(file));

    if (!image.load(file)) {
        ShowMsg(QString(tr("sky image file read error: %1")).arg(file));
        ShowLegend(NULL);
        return;
    }
    SkyImageI=image;
    SkyImageR=image;
    w=MAX(SkyImageI.width(),SkyImageI.height());
    h=MIN(SkyImageI.width(),SkyImageI.height());
    wr=MIN(w,MAX_SKYIMG_R);
    SkyImageR=QImage(wr,wr,QImage::Format_RGB32);
    SkyImageFile=file;
    SkySize[0]=SkyImageI.width();
    SkySize[1]=SkyImageI.height();
    SkyCent[0]=SkySize[0]/2.0;
    SkyCent[1]=SkySize[1]/2.0;
    SkyFov[0]=SkyFov[1]=SkyFov[2]=0.0;
    SkyScale=h/2.0;
    SkyScaleR=SkyScale*wr/w;
    SkyDestCorr=SkyRes=SkyFlip=0;
    SkyElMask=1;
    for (i=0;i<10;i++) SkyDest[i]=0.0;
    
    ReadSkyTag(file+".tag");
    
    ShowMsg("");
    BtnShowImg->setChecked(true);
    
    UpdateSky();
}
// read map tag data --------------------------------------------------------
void Plot::ReadMapTag(const QString &file)
{
    QFile fp(file+".tag");
    QByteArray buff;
    
    trace(3,"ReadMapTag\n");
    
    if (!(fp.open(QIODevice::ReadOnly))) return;
    
    MapScaleX=MapScaleY=1.0;
    MapScaleEq=0;
    MapLat=MapLon=0.0;
    
    while (!fp.atEnd()) {
        buff=fp.readLine();
        if (buff.at(0)=='\0'||buff.at(0)=='%'||buff.at(0)=='#') continue;
        QList<QByteArray> tokens=buff.split('=');
        if (tokens.size()<2) continue;

        if      (tokens.at(0)=="scalex")  MapScaleX=tokens.at(1).toDouble();
        else if (tokens.at(0)=="scaley")  MapScaleY=tokens.at(1).toDouble();
        else if (tokens.at(0)=="scaleeq") MapScaleEq=tokens.at(1).toInt();
        else if (tokens.at(0)=="lat")     MapLat=tokens.at(1).toDouble();
        else if (tokens.at(0)=="lon")     MapLon=tokens.at(1).toDouble();
    }
}
// read shapefile -----------------------------------------------------------
void Plot::ReadShapeFile(const QStringList &files)
{
    int i;
    char path[1024];
    
    ReadWaitStart();
    
    gis_free(&Gis);

    for (i=0;i<files.count()&&i<MAXMAPLAYER;i++) {
        strcpy(path,qPrintable(files.at(i)));
        ShowMsg(QString("reading shapefile... %1").arg(path));
        gis_read(path,&Gis,i);

        QFileInfo fi(files.at(i));

        strcpy(Gis.name[i],qPrintable(fi.baseName()));
    }
    
    ReadWaitEnd();
    ShowMsg("");
    BtnShowMap->setChecked(true);
    
    UpdateOrigin();
    UpdatePlot();
    UpdateEnable();

}
// read waypoint ------------------------------------------------------------
void Plot::ReadWaypoint(const QString &file)
{
    QFile fp(file);
    QByteArray buff;
    QString name;
    double pos[3]={0};

    if (!fp.open(QIODevice::ReadOnly|QIODevice::Text)) return;

    ReadWaitStart();
    ShowMsg(QString("reading waypoint... %1").arg(file));

    NWayPnt=0;

    QXmlStreamReader inputStream(&fp);
    while (!inputStream.atEnd() && !inputStream.hasError()&&NWayPnt<MAXWAYPNT)
    {
        inputStream.readNext();
        if (inputStream.isStartElement()) {
            QString tag = inputStream.name().toString();
            if (tag.toLower() == "wpt")
            {
                pos[0]=inputStream.attributes().value("lat").toFloat();
                pos[1]=inputStream.attributes().value("lon").toFloat();
            } else if ((tag.toLower() == "ele")&&norm(pos,2)>0.0) {
                pos[2]=inputStream.text().toFloat();
            } else if ((tag.toLower() == "name")&&norm(pos,3)>0.0) {
                inputStream.readNext();
                name=inputStream.text().toString();
            }
        } else if (inputStream.isEndElement()) {
            QString tag = inputStream.name().toString();
            if (tag.toLower() == "wpt")
            {
                PntPos[NWayPnt][0]=pos[0];
                PntPos[NWayPnt][1]=pos[1];
                PntPos[NWayPnt][2]=pos[2];
                PntName[NWayPnt++]=name;
                pos[0]=pos[1]=pos[2]=0.0;
                name.clear();
            }
        }
    }

    ReadWaitEnd();
    ShowMsg("");

    BtnShowMap->setChecked(true);

    UpdatePlot();
    UpdateEnable();

    pntDialog->SetPoint();
}
// save waypoint ------------------------------------------------------------
void Plot::SaveWaypoint(const QString &file)
{
    QFile fp(file);
    int i;

    if (!fp.open(QIODevice::WriteOnly|QIODevice::Text)) return;
    QXmlStreamWriter stream(&fp);

    stream.setAutoFormatting(true);
    stream.writeStartDocument("1.0");

    stream.writeStartElement("gpx");
    stream.writeAttribute("version","1.1");
    stream.writeAttribute("creator","RTKLIB");
    stream.writeAttribute("xmlns",XMLNS);

    for (i=0;i<NWayPnt;i++) {
        stream.writeStartElement("wpt");
        stream.writeAttribute("lat",QString::number(PntPos[i][0],'f',9));
        stream.writeAttribute("lon",QString::number(PntPos[i][1],'f',9));
        if (PntPos[i][2]!=0.0) {
            stream.writeTextElement("ele",QString::number(PntPos[i][2],'f',4));
        }
        stream.writeTextElement("name",PntName[i]);
        stream.writeEndElement();
    }
    stream.writeEndElement();

    stream.writeEndDocument();
}
// read station position data -----------------------------------------------
void Plot::ReadStaPos(const QString &file, const QString &sta,
                                  double *rr)
{
    QFile fp(file);
    QByteArray buff;
    QString code;
    double pos[3];
    int sinex=0;
    
    if (!(fp.open(QIODevice::ReadOnly))) return;

    while (!fp.atEnd()) {
        buff=fp.readLine();
        if (buff.indexOf("%=SNX")) sinex=1;
        if (buff.at(0)=='%'||buff.at(1)=='#') continue;
        if (sinex) {

            if ((buff.length()<68)||(buff.mid(14,4)!=sta)) continue;
            if (buff.mid(7,4)=="STAX") rr[0]=buff.mid(47,21).toDouble();
            if (buff.mid(7,4)=="STAY") rr[1]=buff.mid(47,21).toDouble();
            if (buff.mid(7,4)=="STAZ") {rr[2]=buff.mid(47,21).toDouble();break;};
        }
        else {
            QList<QByteArray> tokens=buff.split(' ');
            if (tokens.size()<4) continue;
            for (int i=0;i<3;i++) pos[i]=tokens.at(i).toDouble();
            for (int i=3;i<tokens.size();i++) code=tokens.at(i)+' ';
            code.simplified();

            if (code!=sta) continue;

            pos[0]*=D2R;
            pos[1]*=D2R;
            pos2ecef(pos,rr);
            break;
        }
    }
}
// save dop -----------------------------------------------------------------
void Plot::SaveDop(const QString &file)
{
    QFile fp(file);
    gtime_t time;
    QString data;
    double azel[MAXOBS*2],dop[4],tow;
    int i,j,ns,week;
    char tstr[64];
    QString tlabel;
    
    trace(3,"SaveDop: file=%s\n",qPrintable(file));
    
    if (!(fp.open(QIODevice::WriteOnly))) return;
    
    tlabel=TimeLabel<=1?tr("TIME (GPST)"):(TimeLabel<=2?tr("TIME (UTC)"):tr("TIME (JST))"));
    
    data=QString(tr("%% %1 %2 %3 %4 %5 %6 (EL>=%7deg)\n"))
            .arg(tlabel,TimeLabel==0?13:19).arg("NSAT",6).arg("GDOP",8).arg("PDOP",8).arg("HDOP",8).arg("VDOP",8).arg(ElMask,0,'f',0);
    fp.write(data.toLatin1());

    for (i=0;i<NObs;i++) {
        ns=0;
        for (j=IndexObs[i];j<Obs.n&&j<IndexObs[i+1];j++) {
            if (SatMask[Obs.data[j].sat-1]) continue;
            if (El[j]<ElMask*D2R) continue;
            if (ElMaskP&&El[j]<ElMaskData[static_cast<int>(Az[j]*R2D+0.5)]) continue;
            azel[  ns*2]=Az[j];
            azel[1+ns*2]=El[j];
            ns++;
        }
        if (ns<=0) continue;
        
        dops(ns,azel,ElMask*D2R,dop);
        
        time=Obs.data[IndexObs[i]].time;
        if (TimeLabel==0) {
            tow=time2gpst(time,&week);
            sprintf(tstr,"%4d %8.1f ",week,tow);
        }
        else if (TimeLabel==1) {
            time2str(time,tstr,1);
        }
        else if (TimeLabel==2) {
            time2str(gpst2utc(time),tstr,1);
        }
        else {
            time2str(timeadd(gpst2utc(time),9*3600.0),tstr,1);
        }
        data=QString("%1 %2 %3 %4 %5 %6\n").arg(tstr).arg(ns,6).arg(dop[0],8,'f',1).arg(dop[1],8,'f',1)
                .arg(dop[2],8,'f',1).arg(dop[3],8,'f',1);
        fp.write(data.toLatin1());
    }
}
// save snr and mp -------------------------------------------------------------
void Plot::SaveSnrMp(const QString &file)
{
    QFile fp(file);
    QString ObsTypeText=ObsType2->currentText();
    gtime_t time;
    double tow;
    char sat[32],tstr[64],code[64];
    QString mp,data,tlabel;
    int i,j,k,week;
    strcpy(code,qPrintable(ObsTypeText.mid(1)));
    
    trace(3,"SaveSnrMp: file=%s\n",qPrintable(file));
    
    if (!(fp.open(QIODevice::WriteOnly))) return;

    tlabel=TimeLabel<=1?tr("TIME (GPST)"):(TimeLabel<=2?tr("TIME (UTC)"):tr("TIME (JST)"));
    
    mp=ObsTypeText +" MP(m)";
    data=QString("%% %1 %2 %3 %4 %5 %6\n").arg(tlabel,TimeLabel==0?13:19).arg("SAT",6)
            .arg("AZ(deg)",8).arg("EL(deg)",8).arg("SNR(dBHz)",9).arg(mp,10);
    fp.write(data.toLatin1());

    for (i=0;i<MAXSAT;i++) {
        if (SatMask[i]||!SatSel[i]) continue;
        satno2id(i+1,sat);
        
        for (j=0;j<Obs.n;j++) {
            if (Obs.data[j].sat!=i+1) continue;
            
            for (k=0;k<NFREQ+NEXOBS;k++) {
                if (strstr(code2obs(Obs.data[j].code[k],NULL),code)) break;
            }
            if (k>=NFREQ+NEXOBS) continue;
            
            time=Obs.data[j].time;
            
            if (TimeLabel==0) {
                tow=time2gpst(time,&week);
                sprintf(tstr,"%4d %9.1f ",week,tow);
            }
            else if (TimeLabel==1) {
                time2str(time,tstr,1);
            }
            else if (TimeLabel==2) {
                time2str(gpst2utc(time),tstr,1);
            }
            else {
                time2str(timeadd(gpst2utc(time),9*3600.0),tstr,1);
            }
            data=QString("%1 %2 %3 %4 %5 %6f\n").arg(tstr).arg(sat,6).arg(Az[j]*R2D,8,'f',1)
                    .arg(El[j]*R2D,8,'f',1).arg(Obs.data[j].SNR[k]*0.25,9,'f',2).arg(!Mp[k]?0.0:Mp[k][j],10,'f',4);
            fp.write(data.toLatin1());
        }
    }
}
// save elev mask --------------------------------------------------------------
void Plot::SaveElMask(const QString &file)
{
    QFile fp(file);
    QString data;
    double el,el0=0.0;
    int az;
    
    trace(3,"SaveElMask: file=%s\n",qPrintable(file));
    
    if (!(fp.open(QIODevice::WriteOnly))) return;
    
    fp.write("%% Elevation Mask\n");
    fp.write("%% AZ(deg) EL(deg)\n");
    
    for (az=0;az<=360;az++) {
        el=floor(ElMaskData[az]*R2D/0.1+0.5)*0.1;
        if (qFuzzyCompare(el,el0)) continue;
        data=QString("%1 %2\n").arg(static_cast<double>(az),9,'f',1).arg(el,6,'f',1);
        fp.write(data.toLatin1());
        el0=el;
    }
}
// connect to external sources ----------------------------------------------
void Plot::Connect(void)
{
    char cmd[1024],path[1024],buff[MAXSTRPATH],*name[2]={"",""},*p;
    int i,mode=STR_MODE_R;
    
    trace(3,"Connect\n");
    
    if (ConnectState) return;
    
    for (i=0;i<2;i++) {
        if      (RtStream[i]==STR_NONE    ) continue;
        else if (RtStream[i]==STR_SERIAL  ) strcpy(path,qPrintable(StrPaths[i][0]));
        else if (RtStream[i]==STR_FILE    ) strcpy(path,qPrintable(StrPaths[i][2]));
        else if (RtStream[i]<=STR_NTRIPCLI) strcpy(path,qPrintable(StrPaths[i][1]));
        else continue;
        
        if (RtStream[i]==STR_FILE||!SolData[i].cyclic||SolData[i].nmax!=RtBuffSize+1) {
            Clear();
            initsolbuf(SolData+i,1,RtBuffSize+1);
        }
        if (RtStream[i]==STR_SERIAL) mode|=STR_MODE_W;
        
        strcpy(buff,path);
        if ((p=strstr(buff,"::"))) *p='\0';
        if ((p=strstr(buff,"/:"))) *p='\0';
        if ((p=strstr(buff,"@"))) name[i]=p+1; else name[i]=buff;
        
        if (!stropen(Stream+i,RtStream[i],mode,path)) {
            ShowMsg(QString(tr("connect error: %1")).arg(name[0]));
            ShowLegend(NULL);
            trace(1,"stream open error: ch=%d type=%d path=%s\n",i+1,RtStream[i],path);
            continue;
        }
        strsettimeout(Stream+i,RtTimeOutTime,RtReConnTime);
        
        if (StrCmdEna[i][0]) {
            strcpy(cmd,qPrintable(StrCmds[i][0]));
            strwrite(Stream+i,(unsigned char *)cmd,strlen(cmd));
        }
        ConnectState=1;
    }
    if (!ConnectState) return;
    
    if (Title!="") setWindowTitle(Title);
    else setWindowTitle(QString(tr("CONNECT %1 %2")).arg(name[0]).arg(name[1]));
    
    BtnConnect->setChecked(true);
    BtnSol1   ->setChecked(*name[0]);
    BtnSol2   ->setChecked(*name[1]);
    BtnSol12  ->setChecked(false);
    BtnShowTrack->setChecked(true);
    BtnFixHoriz->setChecked(true);
    UpdateEnable();
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// disconnect from external sources -----------------------------------------
void Plot::Disconnect(void)
{
    char cmd[1024];
    int i;
    
    trace(3,"Disconnect\n");
    
    if (!ConnectState) return;
    
    ConnectState=0;
    
    for (i=0;i<2;i++) {
        if (StrCmdEna[i][1]) {
            strcpy(cmd,qPrintable(StrCmds[i][1]));
            strwrite(Stream+i,(unsigned char *)cmd,strlen(cmd));
        }
        strclose(Stream+i);
    }
    
    if (windowTitle().indexOf(tr("CONNECT"))) {
        setWindowTitle(QString(tr("DISCONNECT%1")).arg(windowTitle().mid(7)));
    }

    StrStatus1->setStyleSheet(QStringLiteral("QLabel {color: gray;}"));
    StrStatus2->setStyleSheet(QStringLiteral("QLabel {color: gray;}"));
    ConnectMsg->setText("");

    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// check observation data types ---------------------------------------------
int Plot::CheckObs(const QString &file)
{
    
    if (!file.indexOf('.')) return 0;
    int p=file.lastIndexOf('.');
    if (file.indexOf(".z")||file.indexOf(".gz")||file.indexOf(".zip")||
        file.indexOf(".Z")||file.indexOf(".GZ")||file.indexOf(".ZIP")) {
        return file.at(p-1)=='o'||file.at(p-1)=='O'||file.at(p-1)=='d'||file.at(p-1)=='D';
    }
    return file.indexOf(".obs")||file.indexOf(".OBS")||
           file.mid(p+3)=="o" ||file.mid(p+3)=="O"||
           file.mid(p+3)=="d" ||file.mid(p+3)=="D" ;
}
// update observation data index, azimuth/elevation, satellite list ---------
void Plot::UpdateObs(int nobs)
{
    prcopt_t opt=prcopt_default;
    gtime_t time;
    sol_t sol;
    double pos[3],rr[3],e[3],azel[MAXOBS*2]={0},rs[6],dts[2],var;
    int i,j,k,svh,per,per_=-1;
    char msg[128],name[16];
    
    trace(3,"UpdateObs\n");
    
    memset(&sol,0,sizeof(sol_t));

    delete [] IndexObs; IndexObs=NULL;
    delete [] Az; Az=NULL;
    delete [] El; El=NULL;
    NObs=0;
    if (nobs<=0) return;
    
    IndexObs=new int[nobs+1];
    Az=new double[Obs.n];
    El=new double[Obs.n];
    
    opt.err[0]=900.0;
    
    ReadWaitStart();
    ShowLegend(NULL);
    
    for (i=0;i<Obs.n;i=j) {
        time=Obs.data[i].time;
        for (j=i;j<Obs.n;j++) {
            if (timediff(Obs.data[j].time,time)>TTOL) break;
        }
        IndexObs[NObs++]=i;
        
        for (k=0;k<j-i;k++) {
            azel[k*2]=azel[1+k*2]=0.0;
        }
        if (RcvPos==0) {
            pntpos(Obs.data+i,j-i,&Nav,&opt,&sol,azel,NULL,msg);
            matcpy(rr,sol.rr,3,1);
            ecef2pos(rr,pos);
        }
        else {
            if (RcvPos==1) { // lat/lon/height
                for (k=0;k<3;k++) pos[k]=OOPos[k];
                pos2ecef(pos,rr);
            }
            else { // rinex header position
                for (k=0;k<3;k++) rr[k]=Sta.pos[k];
                ecef2pos(rr,pos);
            }
            for (k=0;k<j-i;k++) {
                azel[k*2]=azel[1+k*2]=0.0;
                if (!satpos(time,time,Obs.data[i+k].sat,EPHOPT_BRDC,&Nav,rs,dts,
                            &var,&svh)) continue;
                if (geodist(rs,rr,e)>0.0) satazel(pos,e,azel+k*2);
            }
        }
        // satellite azel by tle data
        for (k=0;k<j-i;k++) {
            if (azel[k*2]!=0.0||azel[1+k*2]!=0.0) continue;
            satno2id(Obs.data[i+k].sat,name);
            if (!tle_pos(time,name,"","",&TLEData,NULL,rs)) continue;
            if (geodist(rs,rr,e)>0.0) satazel(pos,e,azel+k*2);
        }
        for (k=0;k<j-i;k++) {
            Az[i+k]=azel[  k*2];
            El[i+k]=azel[1+k*2];
            if (Az[i+k]<0.0) Az[i+k]+=2.0*PI;
        }
        per=(i+1)*100/Obs.n;
        if (per!=per_) {
            ShowMsg(QString(tr("updating azimuth/elevation... (%1%)")).arg(per_=per));
            qApp->processEvents();
        }
    }
    IndexObs[NObs]=Obs.n;
    
    UpdateSatList();
    
    ReadWaitEnd();
}
// update Multipath ------------------------------------------------------------
void Plot::UpdateMp(void)
{
    obsd_t *data;
    double lam1,lam2,I,C,B;
    int i,j,k,f1,f2,sat,sys,per,per_=-1,n;
    
    trace(3,"UpdateMp\n");
    
    for (i=0;i<NFREQ+NEXOBS;i++) {
        delete [] Mp[i]; Mp[i]=NULL;
    }
    if (Obs.n<=0) return;
    
    for (i=0;i<NFREQ+NEXOBS;i++) {
        Mp[i]=new double[Obs.n];
    }
    ReadWaitStart();
    ShowLegend(NULL);
    
    for (i=0;i<Obs.n;i++) {
        data=Obs.data+i;
        sys=satsys(data->sat,NULL);
        
        for (j=0;j<NFREQ+NEXOBS;j++) {
            Mp[j][i]=0.0;
            
            code2obs(data->code[j],&f1);
            
            if (sys==SYS_CMP) {
                if      (f1==5) f1=2; /* B2 */
                else if (f1==4) f1=3; /* B3 */
            }
            if      (sys==SYS_GAL) f2=f1==1?3:1; /* E1/E5a */
            else if (sys==SYS_SBS) f2=f1==1?3:1; /* L1/L5 */
            else if (sys==SYS_CMP) f2=f1==1?2:1; /* B1/B2 */
            else                   f2=f1==1?2:1; /* L1/L2 */
            
            lam1=satwavelen(data->sat,f1-1,&Nav);
            lam2=satwavelen(data->sat,f2-1,&Nav);
            if (lam1==0.0||lam2==0.0) continue;
            
            if (data->P[j]!=0.0&&data->L[j]!=0.0&&data->L[f2-1]!=0.0) {
                C=SQR(lam1)/(SQR(lam1)-SQR(lam2));
                I=lam1*data->L[j]-lam2*data->L[f2-1];
                Mp[j][i]=data->P[j]-lam1*data->L[j]+2.0*C*I;
            }
        }
    }
    for (sat=1;sat<=MAXSAT;sat++) for (i=0;i<NFREQ+NEXOBS;i++) {
        sys=satsys(sat,NULL);
        
        for (j=k=n=0,B=0.0;j<Obs.n;j++) {
            if (Obs.data[j].sat!=sat) continue;
            
            code2obs(Obs.data[j].code[i],&f1);
            
            if (sys==SYS_CMP) {
                if      (f1==5) f1=2; /* B2 */
                else if (f1==4) f1=3; /* B3 */
            }
            if      (sys==SYS_GAL) f2=f1==1?3:1;
            else if (sys==SYS_CMP) f2=f1==1?2:1;
            else                   f2=f1==1?2:1;
            
            if ((Obs.data[j].LLI[i]&1)||(Obs.data[j].LLI[f2-1]&1)||
                fabs(Mp[i][j]-B)>THRES_SLIP) {
                
                for (;k<j;k++) if (Obs.data[k].sat==sat) Mp[i][k]-=B;
                B=Mp[i][j]; n=1; k=j;
            }
            else {
                if (n==0) k=j;
                B+=(Mp[i][j]-B)/++n;
            }
        }
        if (n>0) {
            for (;k<j;k++) if (Obs.data[k].sat==sat) Mp[i][k]-=B;
        }
        per=sat*100/MAXSAT;
        if (per!=per_) {
            ShowMsg(QString(tr("updating multipath... (%1%)")).arg(per));
            per_=per;
            qApp->processEvents();
        }
    }
    ReadWaitEnd();
}
// set connect path ---------------------------------------------------------
void Plot::ConnectPath(const QString &path, int ch)
{
    trace(3,"ConnectPath: path=%s ch=%d\n",qPrintable(path),ch);
    
    RtStream[ch]=STR_NONE;
    
    if (!path.indexOf("://")) return;
    if      (path.indexOf("serial")!=-1) RtStream[ch]=STR_SERIAL;
    else if (path.indexOf("tcpsvr")!=-1) RtStream[ch]=STR_TCPSVR;
    else if (path.indexOf("tcpcli")!=-1) RtStream[ch]=STR_TCPCLI;
    else if (path.indexOf("ntrip" )!=-1) RtStream[ch]=STR_NTRIPCLI;
    else if (path.indexOf("file"  )!=-1) RtStream[ch]=STR_FILE;
    else return;
    
    StrPaths[ch][1]=path.mid(path.indexOf("://")+3);
    RtFormat[ch]=SOLF_LLH;
    RtTimeForm=0;
    RtDegForm =0;
    RtFieldSep=" ";
    RtTimeOutTime=0;
    RtReConnTime =10000;
    
    BtnShowTrack->setChecked(true);
    BtnFixHoriz ->setChecked(true);
    BtnFixVert  ->setChecked(true);
}
// clear obs data --------------------------------------------------------------
void Plot::ClearObs(void)
{
    sta_t sta0;
    int i;
    
    memset(&sta0,0,sizeof(sta_t));

    freeobs(&Obs);
    freenav(&Nav,0xFF);
    delete [] IndexObs; IndexObs=NULL;
    delete [] Az; Az=NULL;
    delete [] El; El=NULL;
    for (i=0;i<NFREQ+NEXOBS;i++) {
        delete [] Mp[i]; Mp[i]=NULL;
    }
    ObsFiles.clear();
    NavFiles.clear();
    NObs=0;
    Sta=sta0;
    ObsIndex=0;
    SimObs=0;
}
// clear solution --------------------------------------------------------------
void Plot::ClearSol(void)
{
    int i;
    
    for (i=0;i<2;i++) {
        freesolbuf(SolData+i);
        free(SolStat[i].data);
        SolStat[i].n=0;
        SolStat[i].data=NULL;
    }
    SolFiles[0].clear();
    SolFiles[1].clear();
    SolIndex[0]=SolIndex[1]=0;
}
// clear data ------------------------------------------------------------------
void Plot::Clear(void)
{
    double ep[]={2010,1,1,0,0,0};
    int i;
    
    trace(3,"Clear\n");
    
    Week=0;
    
    ClearObs();
    ClearSol();
    gis_free(&Gis);
    
    MapImage=QImage();
    MapImageFile="";
    MapSize[0]=MapSize[1]=0;

    for (i=0;i<3;i++) {
        TimeEna[i]=0;
    }
    TimeStart=TimeEnd=epoch2time(ep);
    BtnAnimate->setChecked(false);
    
    if (PlotType>PLOT_NSAT) {
        UpdateType(PLOT_TRK);
    }
    if (!ConnectState) {
        initsolbuf(SolData  ,0,0);
        initsolbuf(SolData+1,0,0);
        setWindowTitle(Title!=""?Title:QString(tr("%1 ver.%2 %3")).arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    }
    else {
        initsolbuf(SolData  ,1,RtBuffSize+1);
        initsolbuf(SolData+1,1,RtBuffSize+1);
    }
    googleEarthView->Clear();
    
    for (i=0;i<=360;i++) ElMaskData[i]=0.0;
    
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// reload data --------------------------------------------------------------
void Plot::Reload(void)
{
    QStringList obsfiles,navfiles;
    
    trace(3,"Reload\n");
    
    if (SimObs) {
        GenVisData();
        return;
    }
    obsfiles=ObsFiles;
    navfiles=NavFiles;
    
    ReadObs(obsfiles);
    ReadNav(navfiles);
    ReadSol(SolFiles[0],0);
    ReadSol(SolFiles[1],1);
    
}
// read wait start ----------------------------------------------------------
void Plot::ReadWaitStart(void)
{
    MenuFile->setEnabled(false);
    MenuEdit->setEnabled(false);
    MenuView->setEnabled(false);
    MenuHelp->setEnabled(false);
    Panel1->setEnabled(false);
    Disp->setEnabled(false);
    setCursor(Qt::WaitCursor);
}
// read wait end ------------------------------------------------------------
void Plot::ReadWaitEnd(void)
{
    MenuFile->setEnabled(true);
    MenuEdit->setEnabled(true);
    MenuView->setEnabled(true);
    MenuHelp->setEnabled(true);
    Panel1->setEnabled(true);
    Disp->setEnabled(true);
    setCursor(Qt::ArrowCursor);
}
// --------------------------------------------------------------------------
