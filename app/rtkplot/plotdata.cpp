//---------------------------------------------------------------------------
// plotdata : rtkplot data functions
//---------------------------------------------------------------------------
#include "rtklib.h"
#include "plotmain.h"
#include "mapdlg.h"
#include "pntdlg.h"
#include "geview.h"

#define HEADXML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
#define HEADGPX "<gpx version=\"1.1\" creator=\"%s\" xmlns=\"%s\">\n"
#define TAILGPX "</gpx>"

#define MAX_SIMOBS	16384			// max genrated obs epochs
#define MAX_SKYIMG_R 2048			// max size of resampled sky image

#define THRES_SLIP  2.0             // threshold of cycle-slip

static char path_str[MAXNFILE][1024];
static const char *XMLNS="http://www.topografix.com/GPX/1/1";

// read solutions -----------------------------------------------------------
void __fastcall TPlot::ReadSol(TStrings *files, int sel)
{
	FILETIME tc,ta,tw;
	SYSTEMTIME st;
	HANDLE h;
    solbuf_t sol={0};
    AnsiString s;
    gtime_t ts,te;
    double tint,ep[6];
    int i,n=0;
    char *paths[MAXNFILE];
    
    trace(3,"ReadSol: sel=%d\n",sel);
    
    for (i=0;i<MAXNFILE;i++) paths[i]=path_str[i];
    
    if (files->Count<=0) return;
    
    ReadWaitStart();
	
	s=files->Strings[0];
    
	if ((h=CreateFile(s.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,
					  FILE_ATTRIBUTE_NORMAL,0))==INVALID_HANDLE_VALUE) {
		return;
	}
	GetFileTime(h,&tc,&ta,&tw);
	CloseHandle(h);
	FileTimeToSystemTime(&tc,&st); // file create time
	ep[0]=st.wYear;
	ep[1]=st.wMonth;
	ep[2]=st.wDay;
	ep[3]=st.wHour;
	ep[4]=st.wMinute;
	ep[5]=st.wSecond;
	sol.time=utc2gpst(epoch2time(ep));
    
    for (i=0;i<files->Count&&n<MAXNFILE;i++) {
        strcpy(paths[n++],U2A(files->Strings[i]).c_str());
    }
    TimeSpan(&ts,&te,&tint);
    
    ShowMsg(s.sprintf("reading %s...",paths[0]));
    ShowLegend(NULL);
    
    if (!readsolt(paths,n,ts,te,tint,0,&sol)) {
        ShowMsg(s.sprintf("no solution data : %s...",paths[0]));
        ShowLegend(NULL);
        ReadWaitEnd();
        return;
    }
    freesolbuf(SolData+sel);
    SolData[sel]=sol;
    
    if (SolFiles[sel]!=files) {
        SolFiles[sel]->Assign(files);
    }
    Caption="";
    
    ReadSolStat(files,sel);
    
    for (i=0;i<2;i++) {
        if (SolFiles[i]->Count==0) continue;
        Caption=Caption+SolFiles[i]->Strings[0]+(SolFiles[i]->Count>1?"... ":" ");
    }
    BtnSol12->Down=False;
    if (sel==0) BtnSol1->Down=true;
    else        BtnSol2->Down=true;
    
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
void __fastcall TPlot::ReadSolStat(TStrings *files, int sel)
{
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i,n=0;
    char *paths[MAXNFILE],id[32];
    
    trace(3,"ReadSolStat\n");
    
    freesolstatbuf(SolStat+sel);
    
    for (i=0;i<MAXNFILE;i++) paths[i]=path_str[i];
    
    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<files->Count&&n<MAXNFILE;i++) {
        strcpy(paths[n++],U2A(files->Strings[i]).c_str());
    }
    ShowMsg(s.sprintf("reading %s...",paths[0]));
    ShowLegend(NULL);
    
    readsolstatt(paths,n,ts,te,tint,SolStat+sel);
    
    UpdateSatList();
}
// read observation data ----------------------------------------------------
void __fastcall TPlot::ReadObs(TStrings *files)
{
    obs_t obs={0};
    nav_t nav={0};
    sta_t sta={0};
    AnsiString s;
    char file[1024];
    int i,nobs;
    
    trace(3,"ReadObs\n");
    
    if (files->Count<=0) return;
    
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
        ObsFiles->Assign(files);
    }
    NavFiles->Clear();
    
    strcpy(file,U2A(files->Strings[0]).c_str());
    
    Caption=s.sprintf("%s%s",file,files->Count>1?"...":"");
    
    BtnSol1->Down=true;
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
int __fastcall TPlot::ReadObsRnx(TStrings *files, obs_t *obs, nav_t *nav,
                                 sta_t *sta)
{
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i,n;
    char obsfile[1024],navfile[1024]="",*p,*q,*opt=RnxOpts.c_str();
    
    trace(3,"ReadObsRnx\n");
    
    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<files->Count;i++) {
        strcpy(obsfile,U2A(files->Strings[i]).c_str());
        
        ShowMsg(s.sprintf("reading obs data... %s",obsfile));
        Application->ProcessMessages();
        
        if (readrnxt(obsfile,1,ts,te,tint,opt,obs,nav,sta)<0) {
            ShowMsg("error: insufficient memory");
            return -1;
        }
    }
    ShowMsg("reading nav data...");
    Application->ProcessMessages();
    
    for (i=0;i<files->Count;i++) {
        strcpy(navfile,U2A(files->Strings[i]).c_str());
        
        if (!(p=strrchr(navfile,'.'))) continue;
        
        if (!strcmp(p,".obs")||!strcmp(p,".OBS")) {
            strcpy(p,".nav" ); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".gnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".hnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".qnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".lnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".cnav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p,".inav"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
        }
        else if (!strcmp(p+3,"o" )||!strcmp(p+3,"d" )||
                 !strcmp(p+3,"O" )||!strcmp(p+3,"D" )) {
            n=nav->n;
            
            strcpy(p+3,"N"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"G"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"H"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"Q"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"L"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"C"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"I"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            strcpy(p+3,"P"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
            
            if (nav->n>n||!(q=strrchr(navfile,'\\'))) continue;
            
            // read brdc navigation data
            memcpy(q+1,"BRDC",4);
            strcpy(p+3,"N"); readrnxt(navfile,1,ts,te,tint,opt,NULL,nav,NULL);
        }
    }
    if (obs->n<=0) {
        ShowMsg(s.sprintf("no observation data: %s...",files->Strings[0].c_str()));
        freenav(nav,0xFF);
        return 0;
    }
    uniqnav(nav);
    return sortobs(obs);
}
// read navigation data -----------------------------------------------------
void __fastcall TPlot::ReadNav(TStrings *files)
{
    AnsiString s;
    gtime_t ts,te;
    double tint;
    char navfile[1024],*opt=RnxOpts.c_str();
    int i;
    
    trace(3,"ReadNav\n");
    
    if (files->Count<=0) return;
    
    ReadWaitStart();
    ShowLegend(NULL);
    
    TimeSpan(&ts,&te,&tint);
    
    freenav(&Nav,0xFF);
    
    ShowMsg("reading nav data...");
    Application->ProcessMessages();
    
    for (i=0;i<files->Count;i++) {
        strcpy(navfile,U2A(files->Strings[i]).c_str());
        readrnxt(navfile,1,ts,te,tint,opt,NULL,&Nav,NULL);
    }
    uniqnav(&Nav);
    
    if (Nav.n<=0&&Nav.ng<=0&&Nav.ns<=0) {
        ShowMsg(s.sprintf("no nav message: %s...",files->Strings[0].c_str()));
        ReadWaitEnd();
        return;
    }
    if (NavFiles!=files) {
        NavFiles->Assign(files);
    }
    UpdateObs(NObs);
    UpdateMp();
    ReadWaitEnd();
    
    UpdatePlot();
    UpdateEnable();
}
// read elevation mask data -------------------------------------------------
void __fastcall TPlot::ReadElMaskData(AnsiString file)
{
    AnsiString s;
    FILE *fp;
    double az0=0.0,el0=0.0,az1,el1;
    int i,j;
    char buff[256];
    
    trace(3,"ReadElMaskData\n");
    
    for (i=0;i<=360;i++) ElMaskData[i]=0.0;
    
    if (!(fp=fopen(file.c_str(),"r"))) {
        ShowMsg(s.sprintf("no el mask data: %s...",file.c_str()));
        ShowLegend(NULL);
        return;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (buff[0]=='%'||sscanf(buff,"%lf %lf",&az1,&el1)<2) continue;
        
        if (az0<az1&&az1<=360.0&&0.0<=el1&&el1<=90.0) {
            
            for (j=(int)az0;j<(int)az1;j++) ElMaskData[j]=el0*D2R;
            ElMaskData[j]=el1*D2R;
        }
        az0=az1; el0=el1;
    }
    fclose(fp);
    UpdatePlot();
    UpdateEnable();
}
// generate visibility data ---------------------------------------------------
void __fastcall TPlot::GenVisData(void)
{
    gtime_t time,ts,te;
    obsd_t data={{0}};
    sta_t sta={0};
    double tint,r,pos[3],rr[3],rs[6],e[3],azel[2];
    int i,j,nobs=0;
    char name[16];
    
    trace(3,"GenVisData\n");
    
    ClearObs();
    SimObs=1;
    
    ts=TimeStart;
    te=TimeEnd;
    tint=TimeInt;
    matcpy(pos,OOPos,3,1);
    pos2ecef(pos,rr);
    
    ReadWaitStart();
    ShowLegend(NULL);
    ShowMsg("generating satellite visibility...");
    Application->ProcessMessages();
    
    for (time=ts;timediff(time,te)<=0.0;time=timeadd(time,tint)) {
        for (i=0;i<MAXSAT;i++) {
            satno2id(i+1,name);
            if (!tle_pos(time,name,"","",&TLEData,NULL,rs)) continue;
            if ((r=geodist(rs,rr,e))<=0.0) continue;
            if (satazel(pos,e,azel)<=0.0) continue;
            if (Obs.n>=Obs.nmax) {
                Obs.nmax=Obs.nmax<=0?4096:Obs.nmax*2;
                Obs.data=(obsd_t *)realloc(Obs.data,sizeof(obsd_t)*Obs.nmax);
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
        ShowMsg("no satellite visibility");
        return;
    }
    UpdateObs(nobs);
    
    Caption="Satellite Visibility (Predicted)";
    BtnSol1->Down=true;
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
void __fastcall TPlot::ReadMapData(AnsiString file)
{
    TJPEGImage *image=new TJPEGImage;
    AnsiString s;
    double pos[3];
    
    trace(3,"ReadMapData\n");
    
    ShowMsg(s.sprintf("reading map image... %s",file.c_str()));
    
    try {
        image->LoadFromFile(file);
    }
    catch (Exception &exception) {
        ShowMsg(s.sprintf("map file read error: %s",file));
        ShowLegend(NULL);
        return;
    }
    MapImage->Assign(image);
    MapImageFile=file;
    MapSize[0]=MapImage->Width;
    MapSize[1]=MapImage->Height;
    delete image;
    
    ReadMapTag(file);
    if (norm(OPos,3)<=0.0&&(MapLat!=0.0||MapLon!=0.0)) {
        pos[0]=MapLat*D2R;
        pos[1]=MapLon*D2R;
        pos[2]=0.0;
        pos2ecef(pos,OPos);
    }
    BtnShowImg->Down=true;
    
    MapAreaDialog->UpdateField();
    UpdateOrigin();
    UpdatePlot();
    UpdateEnable();
    ShowMsg("");
}
// resample image pixel -----------------------------------------------------
#define ResPixelNN(img1,x,y,b1,pix) {\
    int ix=(int)((x)+0.5),iy=(int)((y)+0.5);\
    BYTE *p=(img1)+ix*3+iy*(b1);\
    pix[0]=p[0]; pix[1]=p[1]; pix[2]=p[2];\
}
#define ResPixelBL(img1,x,y,b1,pix) {\
    int ix=(int)(x),iy=(int)(y);\
    double dx1=(x)-ix,dy1=(y)-iy,dx2=1.0-dx1,dy2=1.0-dy1;\
    double a1=dx2*dy2,a2=dx2*dy1,a3=dx1*dy2,a4=dx1*dy1;\
    BYTE *p1=(img1)+ix*3+iy*(b1),*p2=p1+(b1);\
    pix[0]=(BYTE)(a1*p1[0]+a2*p2[0]+a3*p1[3]+a4*p2[3]);\
    pix[1]=(BYTE)(a1*p1[1]+a2*p2[1]+a3*p1[4]+a4*p2[4]);\
    pix[2]=(BYTE)(a1*p1[2]+a2*p2[2]+a3*p1[5]+a4*p2[5]);\
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
static void YCrCb(const BYTE *pix, double *Y)
{
    //         R(0-255)     G(0-255)     B(0-255)
    Y[0]=( 0.299*pix[0]+0.587*pix[1]+0.114*pix[2])/255; // Y  (0-1)
    Y[1]=( 0.500*pix[0]-0.419*pix[1]+0.081*pix[2])/255; // Cr (-.5-.5)
    Y[2]=(-0.169*pix[0]-0.331*pix[1]+0.500*pix[2])/255; // Cb (-.5-.5)
}
// update sky image ---------------------------------------------------------
void __fastcall TPlot::UpdateSky(void)
{
    BITMAP bm1,bm2;
    BYTE *pix;
    double x,y,xp,yp,r,a,b,p[3],q[3],R[9]={0},dr,dist,Yz[3]={0},Y[3];
    int i,j,k,w1,h1,b1,w2,h2,b2,wz,nz=0;
    
    if (!GetObject(SkyImageI->Handle,sizeof(bm1),&bm1)||
        !GetObject(SkyImageR->Handle,sizeof(bm2),&bm2)||
        bm1.bmBitsPixel!=24) return;
    w1=bm1.bmWidth; h1=bm1.bmHeight; b1=bm1.bmWidthBytes;
    w2=bm2.bmWidth; h2=bm2.bmHeight; b2=bm2.bmWidthBytes;
    
    if (w1<=0||h1<=0||b1<w1*3||w2<=0||h2<=0||b2<w2*3) return;
    
    memset(bm2.bmBits,224,b2*h2); // fill bitmap by silver
    
    if (norm(SkyFov,3)>1e-12) {
        RPY(SkyFov,R);
    }
    if (SkyBinarize) { // average of zenith image
        wz=h1/16; // sky area size
        for (i=w1/2-wz;i<=w1/2+wz;i++) for (j=h1/2-wz;j<=h1/2+wz;j++) {
            pix=(BYTE *)bm1.bmBits+i*3+j*b1;
            YCrCb(pix,Y);
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
                a=SQRT(SQR(q[0])+SQR(q[1]));
                xp=r*q[0]/a;
                yp=r*q[1]/a;
            }
        }
        // correct lense distortion
        if (SkyDestCorr) {
            if (r<=0.0||r>=1.0) continue;
            k=(int)(r*9.0); dr=r*9.0-k;
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
        pix=(BYTE *)bm2.bmBits+i*3+j*b2;
        if (!SkyRes) {
            ResPixelNN((BYTE *)bm1.bmBits,x,y,b1,pix)
        }
        else {
            ResPixelBL((BYTE *)bm1.bmBits,x,y,b1,pix)
        }
        if (SkyBinarize) {
            YCrCb(pix,Y);
            for (k=1;k<3;k++) Y[k]-=Yz[k];
            
            // threshold by brightness and color-distance
            if (Y[0]>SkyBinThres1&&norm(Y+1,2)<SkyBinThres2) {
                pix[0]=pix[1]=pix[2]=255; // sky
            }
            else {
                pix[0]=pix[1]=pix[2]=96; // others
            }
        }
    }
    UpdatePlot();
}
// read sky tag data --------------------------------------------------------
void __fastcall TPlot::ReadSkyTag(AnsiString file)
{
    FILE *fp;
    char buff[1024],*p;
    
    trace(3,"ReadSkyTag\n");
    
    if (!(fp=fopen(file.c_str(),"r"))) return;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (buff[0]=='\0'||buff[0]=='%'||buff[0]=='#') continue;
        if (!(p=strchr(buff,'='))) continue;
        *p='\0';
        if      (strstr(buff,"centx"   )==buff) sscanf(p+1,"%lf",SkyCent    );
        else if (strstr(buff,"centy"   )==buff) sscanf(p+1,"%lf",SkyCent+1  );
        else if (strstr(buff,"scale"   )==buff) sscanf(p+1,"%lf",&SkyScale  );
        else if (strstr(buff,"roll"    )==buff) sscanf(p+1,"%lf",SkyFov     );
        else if (strstr(buff,"pitch"   )==buff) sscanf(p+1,"%lf",SkyFov+1   );
        else if (strstr(buff,"yaw"     )==buff) sscanf(p+1,"%lf",SkyFov+2   );
        else if (strstr(buff,"destcorr")==buff) sscanf(p+1,"%d",&SkyDestCorr);
        else if (strstr(buff,"elmask"  )==buff) sscanf(p+1,"%d",&SkyElMask  );
        else if (strstr(buff,"resample")==buff) sscanf(p+1,"%d",&SkyRes     );
        else if (strstr(buff,"flip"    )==buff) sscanf(p+1,"%d",&SkyFlip    );
        else if (strstr(buff,"dest"    )==buff) {
            sscanf(p+1,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",SkyDest+1,
                SkyDest+2,SkyDest+3,SkyDest+4,SkyDest+5,SkyDest+6,SkyDest+7,
                SkyDest+8,SkyDest+9);
        }
        else if (strstr(buff,"binarize")==buff) sscanf(p+1,"%d",&SkyBinarize);
        else if (strstr(buff,"binthr1")==buff) sscanf(p+1,"%lf",&SkyBinThres1);
        else if (strstr(buff,"binthr2")==buff) sscanf(p+1,"%lf",&SkyBinThres2);
    }
    fclose(fp);
}
// read sky image data ------------------------------------------------------
void __fastcall TPlot::ReadSkyData(AnsiString file)
{
    TJPEGImage *image=new TJPEGImage;
    AnsiString s;
    int i,w,h,wr;
    
    trace(3,"ReadSkyData\n");
    
    ShowMsg(s.sprintf("reading sky image... %s",file.c_str()));
    
    try {
        image->LoadFromFile(file);
    }
    catch (Exception &exception) {
        ShowMsg(s.sprintf("sky image file read error: %s",file));
        ShowLegend(NULL);
        return;
    }
    SkyImageI->Assign(image);
    SkyImageR->Assign(image);
    w=MAX(SkyImageI->Width,SkyImageI->Height);
    h=MIN(SkyImageI->Width,SkyImageI->Height);
    wr=MIN(w,MAX_SKYIMG_R);
    SkyImageR->SetSize(wr,wr);
    SkyImageFile=file;
    SkySize[0]=SkyImageI->Width;
    SkySize[1]=SkyImageI->Height;
    SkyCent[0]=SkySize[0]/2.0;
    SkyCent[1]=SkySize[1]/2.0;
    SkyFov[0]=SkyFov[1]=SkyFov[2]=0.0;
    SkyScale=h/2.0;
    SkyScaleR=SkyScale*wr/w;
    SkyDestCorr=SkyRes=SkyFlip=0;
    SkyElMask=1;
    for (i=0;i<10;i++) SkyDest[i]=0.0;
    delete image;
    
    ReadSkyTag(file+".tag");
    
    ShowMsg("");
    BtnShowImg->Down=true;
    
    UpdateSky();
}
// read map tag data --------------------------------------------------------
void __fastcall TPlot::ReadMapTag(AnsiString file)
{
    FILE *fp;
    char buff[1024],*p;
    
    trace(3,"ReadMapTag\n");
    
    file=file+".tag";
    
    if (!(fp=fopen(file.c_str(),"r"))) return;
    
    MapScaleX=MapScaleY=1.0;
    MapScaleEq=0;
    MapLat=MapLon=0.0;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (buff[0]=='\0'||buff[0]=='%'||buff[0]=='#') continue;
        if (!(p=strchr(buff,'='))) continue;
        *p='\0';
        if      (strstr(buff,"scalex" )==buff) sscanf(p+1,"%lf",&MapScaleX );
        else if (strstr(buff,"scaley" )==buff) sscanf(p+1,"%lf",&MapScaleY );
        else if (strstr(buff,"scaleeq")==buff) sscanf(p+1,"%d" ,&MapScaleEq);
        else if (strstr(buff,"lat"    )==buff) sscanf(p+1,"%lf",&MapLat    );
        else if (strstr(buff,"lon"    )==buff) sscanf(p+1,"%lf",&MapLon    );
    }
    fclose(fp);
}
// read shapefile -----------------------------------------------------------
void __fastcall TPlot::ReadShapeFile(TStrings *files)
{
    UnicodeString name;
    AnsiString s;
    double pos[3];
    char path[1024];
    int i,j;
    
    ReadWaitStart();
    
    gis_free(&Gis);
    
    for (i=0;i<files->Count&&i<MAXMAPLAYER;i++) {
        strcpy(path,U2A(files->Strings[i]).c_str());
        ShowMsg(s.sprintf("reading shapefile... %s",path));
        gis_read(path,&Gis,i);
        
        name=files->Strings[i];
        while ((j=name.Pos(L"\\"))) {
            name=name.SubString(j+1,name.Length()-j);
        }
        if ((j=name.Pos(L"."))) {
            name=name.SubString(1,j-1);
        }
        strcpy(Gis.name[i],U2A(name).c_str());
    }
    ReadWaitEnd();
    ShowMsg("");
    
    BtnShowMap->Down=true;
    
    if (norm(OPos,3)<=0.0) {
        pos[0]=(Gis.bound[0]+Gis.bound[1])/2.0;
        pos[1]=(Gis.bound[2]+Gis.bound[3])/2.0;
        pos[2]=0.0;
        pos2ecef(pos,OPos);
    }
    UpdateOrigin();
    UpdatePlot();
    UpdateEnable();
}
// read waypoint ------------------------------------------------------------
void __fastcall TPlot::ReadWaypoint(AnsiString file)
{
    UTF8String label1(L"<ogr:–¼Ì>"),label2(L"<ogr:“_–¼Ì>");
    AnsiString s;
    FILE *fp;
    char buff[1024],name[256]="",*p;
    double pos[3]={0};
    
    if (!(fp=fopen(file.c_str(),"r"))) return;
    
    ReadWaitStart();
    ShowMsg(s.sprintf("reading waypoint... %s",file.c_str()));
    
    NWayPnt=0;
    
    while (fgets(buff,sizeof(buff),fp)&&NWayPnt<MAXWAYPNT) {
        if ((p=strstr(buff,"<wpt "))) {
            if (sscanf(p+strlen("<wpt "),"lat=\"%lf\" lon=\"%lf\"",pos,
                       pos+1)<2) continue;
        }
        else if ((p=strstr(buff,"<ele>"))&&norm(pos,2)>0.0) {
            sscanf(p+strlen("<ele>"),"%lf",pos+2);
        }
        else if ((p=strstr(buff,"<name>"))&&norm(pos,3)>0.0) {
            sscanf(p+strlen("<name>"),"%[^<]",name);
        }
        else if ((p=strstr(buff,label1.c_str()))&&norm(pos,3)>0.0&&!name[0]) {
            sscanf(p+strlen(label1.c_str()),"%[^<]",name);
        }
        else if ((p=strstr(buff,label2.c_str()))&&norm(pos,3)>0.0&&!name[0]) {
            sscanf(p+strlen(label2.c_str()),"%[^<]",name);
        }
        else if (strstr(buff,"</wpt>")&&norm(pos,3)>0.0) {
            PntPos[NWayPnt][0]=pos[0];
            PntPos[NWayPnt][1]=pos[1];
            PntPos[NWayPnt][2]=pos[2];
            PntName[NWayPnt++]=name; // UTF-8
            pos[0]=pos[1]=pos[2]=0.0;
            name[0]='\0';
        }
    }
    fclose(fp);
    
    ReadWaitEnd();
    ShowMsg("");
    
    BtnShowMap->Down=true;
    
    UpdatePlot();
    UpdateEnable();
    PntDialog->SetPoint();
}
// save waypoint ------------------------------------------------------------
void __fastcall TPlot::SaveWaypoint(AnsiString file)
{
    FILE *fp;
    int i;
    
    if (!(fp=fopen(file.c_str(),"w"))) return;
    
    fprintf(fp,HEADXML);
    fprintf(fp,HEADGPX,"RTKLIB " VER_RTKLIB,XMLNS);
     
    for (i=0;i<NWayPnt;i++) {
        fprintf(fp,"<wpt lat=\"%.9f\" lon=\"%.9f\">\n",PntPos[i][0],
                PntPos[i][1]);
        if (PntPos[i][2]!=0.0) {
            fprintf(fp," <ele>%.4f</ele>\n",PntPos[i][2]);
        }
        UTF8String str(PntName[i]);
        fprintf(fp," <name>%s</name>\n",str); // UTF-8
        fprintf(fp,"</wpt>\n");
    }
    fprintf(fp,"%s\n",TAILGPX);
    fclose(fp);
}
// read station position data -----------------------------------------------
void __fastcall TPlot::ReadStaPos(const char *file, const char *sta,
                                  double *rr)
{
    FILE *fp;
    char buff[256],code[256],name[256];
    double pos[3];
    int sinex=0;
    
    if (!(fp=fopen(file,"r"))) return;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (strstr(buff,"%=SNX")==buff) sinex=1;
        if (buff[0]=='%'||buff[1]=='#') continue;
        if (sinex) {
            if (strlen(buff)<68||strncmp(buff+14,sta,4)) continue;
            if (!strncmp(buff+7,"STAX",4)) rr[0]=str2num(buff,47,21);
            if (!strncmp(buff+7,"STAY",4)) rr[1]=str2num(buff,47,21);
            if (!strncmp(buff+7,"STAZ",4)) {
                rr[2]=str2num(buff,47,21);
                break;
            }
        }
        else {
            if (sscanf(buff,"%lf %lf %lf %s",pos,pos+1,pos+2,code)<4) continue;
            if (strcmp(code,sta)) continue;
            pos[0]*=D2R;
            pos[1]*=D2R;
            pos2ecef(pos,rr);
            break;
        }
    }
	fclose(fp);
}
// save dop -----------------------------------------------------------------
void __fastcall TPlot::SaveDop(AnsiString file)
{
    FILE *fp;
    gtime_t time;
    double azel[MAXOBS*2],dop[4],tow;
    int i,j,ns,week;
    char tstr[64],*tlabel;
    
    trace(3,"SaveDop: file=%s\n",file.c_str());
    
    if (!(fp=fopen(file.c_str(),"w"))) return;
    
    tlabel=TimeLabel<=1?"TIME (GPST)":(TimeLabel<=2?"TIME (UTC)":"TIME (JST)");
    
    fprintf(fp,"%% %-*s %6s %8s %8s %8s %8s (EL>=%.0fdeg)\n",TimeLabel==0?13:19,
            tlabel,"NSAT","GDOP","PDOP","HDOP","VDOP",ElMask);
    
    for (i=0;i<NObs;i++) {
        ns=0;
        for (j=IndexObs[i];j<Obs.n&&j<IndexObs[i+1];j++) {
            if (SatMask[Obs.data[j].sat-1]) continue;
            if (El[j]<ElMask*D2R) continue;
            if (ElMaskP&&El[j]<ElMaskData[(int)(Az[j]*R2D+0.5)]) continue;
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
        fprintf(fp,"%s %6d %8.1f %8.1f %8.1f %8.1f\n",tstr,ns,dop[0],dop[1],
                dop[2], dop[3]);
    }
    fclose(fp);
}
// save snr and mp -------------------------------------------------------------
void __fastcall TPlot::SaveSnrMp(AnsiString file)
{
    FILE *fp;
    AnsiString ObsTypeText=ObsType2->Text;
    gtime_t time;
    double tow;
    char sat[32],mp[32],tstr[64],*tlabel,*code=ObsTypeText.c_str()+1;
    int i,j,k,week;
    
    trace(3,"SaveSnrMp: file=%s\n",file.c_str());
    
    if (!(fp=fopen(file.c_str(),"w"))) return;
    
    tlabel=TimeLabel<=1?"TIME (GPST)":(TimeLabel<=2?"TIME (UTC)":"TIME (JST)");
    
    sprintf(mp,"%s MP(m)",ObsTypeText.c_str());
    fprintf(fp,"%% %-*s %6s %8s %8s %9s %10s\n",TimeLabel==0?13:19,tlabel,"SAT",
            "AZ(deg)","EL(deg)","SNR(dBHz)",mp);
    
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
            fprintf(fp,"%s %6s %8.1f %8.1f %9.2f %10.4f\n",tstr,sat,Az[j]*R2D,
                    El[j]*R2D,Obs.data[j].SNR[k]*0.25,!Mp[k]?0.0:Mp[k][j]);
        }
    }
    fclose(fp);
}
// save elev mask --------------------------------------------------------------
void __fastcall TPlot::SaveElMask(AnsiString file)
{
    FILE *fp;
    double el,el0=0.0;
    int az;
    
    trace(3,"SaveElMask: file=%s\n",file.c_str());
    
    if (!(fp=fopen(file.c_str(),"w"))) return;
    
    fprintf(fp,"%% Elevation Mask\n");
    fprintf(fp,"%% AZ(deg) EL(deg)\n");
    
    for (az=0;az<=360;az++) {
        el=floor(ElMaskData[az]*R2D/0.1+0.5)*0.1;
        if (el==el0) continue;
        fprintf(fp,"%9.1f %6.1f\n",(double)az,el);
        el0=el;
    }
    fclose(fp);
}
// connect to external sources ----------------------------------------------
void __fastcall TPlot::Connect(void)
{
    AnsiString s;
    char *cmd,*path,buff[MAXSTRPATH],*name[2]={"",""},*p;
    int i,mode=STR_MODE_R;
    
    trace(3,"Connect\n");
    
    if (ConnectState) return;
    
    for (i=0;i<2;i++) {
        if      (RtStream[i]==STR_NONE    ) continue;
        else if (RtStream[i]==STR_SERIAL  ) path=StrPaths[i][0].c_str();
        else if (RtStream[i]==STR_FILE    ) path=StrPaths[i][2].c_str();
        else if (RtStream[i]<=STR_NTRIPCLI) path=StrPaths[i][1].c_str();
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
            ShowMsg(s.sprintf("connect error: %s",name));
            ShowLegend(NULL);
            trace(1,"stream open error: ch=%d type=%d path=%s\n",i+1,RtStream[i],path);
            continue;
        }
        strsettimeout(Stream+i,RtTimeOutTime,RtReConnTime);
        
        if (StrCmdEna[i][0]) {
            cmd=StrCmds[i][0].c_str();
            strwrite(Stream+i,(unsigned char *)cmd,strlen(cmd));
        }
        ConnectState=1;
    }
    if (!ConnectState) return;
    
    if (Title!="") Caption=Title;
    else Caption=s.sprintf("CONNECT %s %s",name[0],name[1]);
    
    BtnConnect->Down=true;
    BtnSol1   ->Down=*name[0];
    BtnSol2   ->Down=*name[1];
    BtnSol12  ->Down=false;
    BtnShowTrack->Down=true;
    BtnFixHoriz->Down=true;
    BtnReload->Visible=false;
    StrStatus->Left=Panel11->Width-BtnOptions->Width;
    StrStatus->Visible=true;
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// disconnect from external sources -----------------------------------------
void __fastcall TPlot::Disconnect(void)
{
    AnsiString s;
    char *cmd,caption[1024];
    int i;
    
    trace(3,"Disconnect\n");
    
    if (!ConnectState) return;
    
    ConnectState=0;
    
    for (i=0;i<2;i++) {
        if (StrCmdEna[i][1]) {
            cmd=StrCmds[i][1].c_str();
            strwrite(Stream+i,(unsigned char *)cmd,strlen(cmd));
        }
        strclose(Stream+i);
    }
    strcpy(caption,U2A(Caption).c_str());
    
    if (strstr(caption,"CONNECT")) {
        Caption=s.sprintf("DISCONNECT%s",caption+7);
    }
    StrStatus->Visible=false;
    BtnReload->Left=Panel11->Width-BtnOptions->Width;
    BtnReload->Visible=true;
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// check observation data types ---------------------------------------------
int __fastcall TPlot::CheckObs(AnsiString file)
{
    char *p;
    
    trace(3,"CheckObs\n");
    
    if (!(p=strrchr(file.c_str(),'.'))) return 0;
    if (!strcmp(p,".z")||!strcmp(p,".gz")||!strcmp(p,".zip")||
        !strcmp(p,".Z")||!strcmp(p,".GZ")||!strcmp(p,".ZIP")) {
        return *(p-1)=='o'||*(p-1)=='O'||*(p-1)=='d'||*(p-1)=='D';
    }
    return !strcmp(p,".obs")||!strcmp(p,".OBS")||
           !strcmp(p+3,"o" )||!strcmp(p+3,"O" )||
           !strcmp(p+3,"d" )||!strcmp(p+3,"D" );
}
// update observation data index, azimuth/elevation, satellite list ---------
void __fastcall TPlot::UpdateObs(int nobs)
{
    AnsiString s;
    prcopt_t opt=prcopt_default;
    gtime_t time;
    sol_t sol={0};
    double pos[3],rr[3],e[3],azel[MAXOBS*2]={0},rs[6],dts[2],var;
    int i,j,k,svh,per,per_=-1;
    char msg[128],name[16];
    
    trace(3,"UpdateObs\n");
    
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
            ShowMsg(s.sprintf("updating azimuth/elevation... (%d%%)",(per_=per)));
            Application->ProcessMessages();
        }
    }
    IndexObs[NObs]=Obs.n;
    
    UpdateSatList();
    
    ReadWaitEnd();
}
// update Multipath ------------------------------------------------------------
void __fastcall TPlot::UpdateMp(void)
{
    AnsiString s;
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
            
            if (data->P[j]!=0.0&&data->L[j]!=0.0&&data->L[f2-1]) {
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
            ShowMsg(s.sprintf("updating multipath... (%d%%)",(per_=per)));
            Application->ProcessMessages();
        }
    }
    ReadWaitEnd();
}
// set connect path ---------------------------------------------------------
void __fastcall TPlot::ConnectPath(const char *path, int ch)
{
    const char *p;
    
    trace(3,"ConnectPath: path=%s ch=%d\n",path,ch);
    
    RtStream[ch]=STR_NONE;
    
    if (!(p=strstr(path,"://"))) return;
    if      (!strncmp(path,"serial",6)) RtStream[ch]=STR_SERIAL;
    else if (!strncmp(path,"tcpsvr",6)) RtStream[ch]=STR_TCPSVR;
    else if (!strncmp(path,"tcpcli",6)) RtStream[ch]=STR_TCPCLI;
    else if (!strncmp(path,"ntrip", 5)) RtStream[ch]=STR_NTRIPCLI;
    else if (!strncmp(path,"file",  4)) RtStream[ch]=STR_FILE;
    else return;
    
    StrPaths[ch][1]=p+3;
    RtFormat[ch]=SOLF_LLH;
    RtTimeForm=0;
    RtDegForm =0;
    RtFieldSep=" ";
    RtTimeOutTime=0;
    RtReConnTime =10000;
    
    BtnShowTrack->Down=true;
    BtnFixHoriz ->Down=true;
    BtnFixVert  ->Down=true;
}
// clear obs data --------------------------------------------------------------
void __fastcall TPlot::ClearObs(void)
{
    sta_t sta0={0};
    int i;
    
    freeobs(&Obs);
    freenav(&Nav,0xFF);
    delete [] IndexObs; IndexObs=NULL;
    delete [] Az; Az=NULL;
    delete [] El; El=NULL;
    for (i=0;i<NFREQ+NEXOBS;i++) {
        delete [] Mp[i]; Mp[i]=NULL;
    }
    ObsFiles->Clear();
    NavFiles->Clear();
    NObs=0;
    Sta=sta0;
    ObsIndex=0;
    SimObs=0;
}
// clear solution --------------------------------------------------------------
void __fastcall TPlot::ClearSol(void)
{
    int i;
    
    for (i=0;i<2;i++) {
        freesolbuf(SolData+i);
        free(SolStat[i].data);
        SolStat[i].n=0;
        SolStat[i].data=NULL;
    }
    SolFiles[0]->Clear();
    SolFiles[1]->Clear();
    SolIndex[0]=SolIndex[1]=0;
}
// clear data ------------------------------------------------------------------
void __fastcall TPlot::Clear(void)
{
    AnsiString s;
    double ep[]={2010,1,1,0,0,0};
    int i;
    
    trace(3,"Clear\n");
    
    Week=0;
    
    ClearObs();
    ClearSol();
    gis_free(&Gis);
    
    MapImage->Height=0;
    MapImage->Width=0;
    MapImageFile="";
    MapSize[0]=MapSize[1]=0;
    
    for (i=0;i<3;i++) {
        TimeEna[i]=0;
    }
    TimeStart=TimeEnd=epoch2time(ep);
    BtnAnimate->Down=false;
    
    if (PlotType>PLOT_NSAT) {
        UpdateType(PLOT_TRK);
    }
    if (!ConnectState) {
        initsolbuf(SolData  ,0,0);
        initsolbuf(SolData+1,0,0);
        Caption=Title!=""?Title:s.sprintf("%s ver.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    }
    else {
        initsolbuf(SolData  ,1,RtBuffSize+1);
        initsolbuf(SolData+1,1,RtBuffSize+1);
    }
    GoogleEarthView->Clear();
    
    for (i=0;i<=360;i++) ElMaskData[i]=0.0;
    
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// reload data --------------------------------------------------------------
void __fastcall TPlot::Reload(void)
{
    TStrings *obsfiles,*navfiles;
    
    trace(3,"Reload\n");
    
    if (SimObs) {
        GenVisData();
        return;
    }
    obsfiles=new TStringList;
    navfiles=new TStringList;
    obsfiles->Assign(ObsFiles);
    navfiles->Assign(NavFiles);
    
    ReadObs(obsfiles);
    ReadNav(navfiles);
    ReadSol(SolFiles[0],0);
    ReadSol(SolFiles[1],1);
    
    delete obsfiles;
    delete navfiles;
}
// read wait start ----------------------------------------------------------
void __fastcall TPlot::ReadWaitStart(void)
{
    MenuFile->Enabled=false;
    MenuEdit->Enabled=false;
    MenuView->Enabled=false;
    MenuHelp->Enabled=false;
    Panel1->Enabled=false;
    Disp->Enabled=false;
    Screen->Cursor=crHourGlass;
}
// read wait end ------------------------------------------------------------
void __fastcall TPlot::ReadWaitEnd(void)
{
    MenuFile->Enabled=true;
    MenuEdit->Enabled=true;
    MenuView->Enabled=true;
    MenuHelp->Enabled=true;
    Panel1->Enabled=true;
    Disp->Enabled=true;
    Screen->Cursor=crDefault;
}
// --------------------------------------------------------------------------
