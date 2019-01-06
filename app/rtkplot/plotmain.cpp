//---------------------------------------------------------------------------
// rtkplot : visualization of solution and obs data ap
//
//          Copyright (C) 2007-2012 by T.TAKASU, All rights reserved.
//
// options : rtkplot [-t title][-i file][-r][-p path][-x level][file ...]
//
//           -t title  window title
//           -i file   ini file path
//           -r        open file as obs and nav file
//           -p path   connect to path
//                       serial://port[:brate[:bsize[:parity[:stopb[:fctr]]]]]
//                       tcpsvr://:port
//                       tcpcli://addr[:port]
//                       ntrip://[user[:passwd]@]addr[:port][/mntpnt]
//                       file://path
//           -p1 path  connect port 1 to path 
//           -p2 path  connect port 2 to path 
//           -x level  debug trace level (0:off)
//           file      solution files or rinex obs/nav file
//
// version : $Revision: 1.1 $ $Date: 2008/07/17 22:15:27 $
// history : 2008/07/14  1.0 new
//           2009/11/27  1.1 rtklib 2.3.0
//           2010/07/18  1.2 rtklib 2.4.0
//           2010/06/10  1.3 rtklib 2.4.1
//           2010/06/19  1.4 rtklib 2.4.1 p1
//           2012/11/21  1.5 rtklib 2.4.2
//           2016/06/11  1.6 rtklib 2.4.3
//---------------------------------------------------------------------------
#include <vcl.h>
#include <inifiles.hpp>
#include <Clipbrd.hpp>
#pragma hdrstop
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "rtklib.h"
#include "plotmain.h"
#include "plotopt.h"
#include "refdlg.h"
#include "tspandlg.h"
#include "satdlg.h"
#include "aboutdlg.h"
#include "fileseldlg.h"
#include "conndlg.h"
#include "confdlg.h"
#include "console.h"
#include "pntdlg.h"
#include "mapdlg.h"
#include "skydlg.h"
#include "geview.h"
#include "gmview.h"
#include "viewer.h"
#include "vmapdlg.h"
#pragma link "SHDocVw_OCX"

#define YLIM_AGE    10.0            // ylimit of age of differential
#define YLIM_RATIO  20.0            // ylimit of raito factor

// instance of TPLOT --------------------------------------------------------
TPlot *Plot;

// constructor --------------------------------------------------------------
__fastcall TPlot::TPlot(TComponent* Owner) : TForm(Owner)
{
    gtime_t t0={0};
    nav_t nav0={0};
    obs_t obs0={0};
    sta_t sta0={0};
    gis_t gis0={0};
    solstatbuf_t solstat0={0};
    AnsiString s;
    double ep[]={2000,1,1,0,0,0},xl[2],yl[2];
    double xs[]={-DEFTSPAN/2,DEFTSPAN/2};
    int i,nfreq=NFREQ;
    char file[1024]="rtkplot.exe",*p;
    
    ::GetModuleFileName(NULL,file,sizeof(file));
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".ini");
    IniFile=file;
    
    FormWidth=FormHeight=0;
    Drag=0; Xn=Yn=-1; NObs=0;
    IndexObs=NULL;
    Week=Flush=PlotType=0;
    AnimCycle=1;
    for (i=0;i<2;i++) {
        initsolbuf(SolData+i,0,0);
        SolStat[i]=solstat0;
        SolIndex[i]=0;
    }
    ObsIndex=0;
    Obs=obs0;
    Nav=nav0;
    Sta=sta0;
    Gis=gis0;
    SimObs=0;
    
    X0=Y0=Xc=Yc=Xs=Ys=Xcent=0.0;
    MouseDownTick=0;
    GEState=GEDataState[0]=GEDataState[1]=0;
    GEHeading=0.0;
    OEpoch=t0;
    for (i=0;i<3;i++) OPos[i]=OVel[i]=0.0;
    Az=El=NULL;
    for (i=0;i<NFREQ+NEXOBS;i++) Mp[i]=NULL;
    OpenFiles  =new TStringList;
    SolFiles[0]=new TStringList;
    SolFiles[1]=new TStringList;
    ObsFiles   =new TStringList;
    NavFiles   =new TStringList;
    Buff    =new Graphics::TBitmap;
    MapImage=new Graphics::TBitmap;
    SkyImageI=new Graphics::TBitmap;
    SkyImageR=new Graphics::TBitmap;
    GraphT =new TGraph(Disp);
    GraphT->Fit=0;
    
    for (i=0;i<3;i++) {
        GraphG[i]=new TGraph(Disp);
        GraphG[i]->XLPos=0;
        GraphG[i]->GetLim(xl,yl);
        GraphG[i]->SetLim(xs,yl);
    }
    GraphR=new TGraph(Disp);
    for (i=0;i<2;i++) {
        GraphE[i]=new TGraph(Disp);
    }
    GraphS=new TGraph(Disp);
    GraphR->GetLim(xl,yl);
    GraphR->SetLim(xs,yl);
    
    MapSize[0]=MapSize[1]=0;
    MapScaleX=MapScaleY=0.1;
    MapScaleEq=0;
    MapLat=MapLon=0.0;
    PointType=0;
    
    NWayPnt=0;
    SelWayPnt=-1;
    
    SkySize[0]=SkySize[1]=SkyCent[0]=SkyCent[1]=0;
    SkyScale=SkyScaleR=240.0;
    SkyFov[0]=SkyFov[1]=SkyFov[2]=0.0;
    SkyElMask=1;
    SkyDestCorr=SkyRes=SkyFlip=0;
    for (i=0;i<10;i++) SkyDest[i]=0.0;
    SkyBinarize=0;
    SkyBinThres1=0.3;
    SkyBinThres2=0.1;
    
    for (i=0;i<3;i++) TimeEna[i]=0;
    TimeLabel=AutoScale=ShowStats=0;
    ShowLabel=ShowGLabel=1;
    ShowArrow=ShowSlip=ShowHalfC=ShowErr=ShowEph=0;
    PlotStyle=MarkSize=Origin=RcvPos=0;
    TimeInt=ElMask=YRange=0.0;
    MaxDop=30.0;
    MaxMP=10.0;
    TimeStart=TimeEnd=epoch2time(ep);
    DoubleBuffered=true;
    Console1=new TConsole(Owner);
    Console2=new TConsole(Owner);
    
    for (i=0;i<361;i++) ElMaskData[i]=0.0;
    
    Trace=0;
    ConnectState=OpenRaw=0;
    RtConnType=0;
    strinitcom();
    strinit(Stream  );
    strinit(Stream+1);
    
    FrqType->Items->Clear();
    FrqType->Items->Add("L1/LC");
    if (nfreq>=2) FrqType->Items->Add("L2");
    if (nfreq>=3) FrqType->Items->Add("L5");
    if (nfreq>=4) FrqType->Items->Add("L6");
    if (nfreq>=5) FrqType->Items->Add("L7");
    if (nfreq>=6) FrqType->Items->Add("L8");
    FrqType->ItemIndex=0;
    
    TLEData.n=TLEData.nmax=0;
    TLEData.data=NULL;
    
    // set current directory as commend search path
    AnsiString cd=GetCurrentDir();
    ::SetEnvironmentVariable("PATH",cd.c_str());
}
// callback on form-create --------------------------------------------------
void __fastcall TPlot::FormCreate(TObject *Sender)
{
    ::DragAcceptFiles(Handle,true);
}
// callback on form-show ----------------------------------------------------
void __fastcall TPlot::FormShow(TObject *Sender)
{
    AnsiString cmd,s;
    int i,argc=0;
    char *p,*argv[32],buff[1024],*path1="",*path2="",str_path[256];
    
    trace(3,"FormShow\n");
    
    cmd=GetCommandLine();
    strcpy(buff,cmd.c_str());
    
    for (p=buff;*p&&argc<32;p++) {
        if (*p==' ') continue;
        if (*p=='"') {
            argv[argc++]=p+1;
            if (!(p=strchr(p+1,'"'))) break;
        }
        else {
            argv[argc++]=p;
            if (!(p=strchr(p+1,' '))) break;
        }
        *p='\0';
    }
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-i")&&i+1<argc) IniFile=argv[++i];
    }
    LoadOpt();
    
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-r" )) OpenRaw=1;
        else if (!strcmp(argv[i],"-p" )&&i+1<argc) path1=argv[++i];
        else if (!strcmp(argv[i],"-p1")&&i+1<argc) path1=argv[++i];
        else if (!strcmp(argv[i],"-p2")&&i+1<argc) path2=argv[++i];
        else if (!strcmp(argv[i],"-t" )&&i+1<argc) Title=argv[++i];
        else if (!strcmp(argv[i],"-x" )&&i+1<argc) Trace=atoi(argv[++i]);
        else {
            OpenFiles->Add(argv[i]);
        }
    }
    UpdateType(PlotType>=PLOT_OBS?PLOT_TRK:PlotType);
    
    UpdateColor();
    UpdateSatMask();
    UpdateOrigin();
    
    if (*path1||*path2) {
        ConnectPath(path1,0);
        ConnectPath(path2,1);
        Connect();
    }
    else if (OpenFiles->Count<=0) {
        Caption=Title!=""?Title:s.sprintf("%s ver.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    }
    if (Trace>0) {
        traceopen(TRACEFILE);
        tracelevel(Trace);
    }
    if (TLEFile!="") {
        tle_read(TLEFile.c_str(),&TLEData);
    }
    if (TLESatFile!="") {
        tle_name_read(TLESatFile.c_str(),&TLEData);
    }
    if (MenuBrowse->Checked) {
        Splitter1->Left=PanelBrowse->Width;
        PanelBrowse->Visible=true;
        Splitter1->Visible=true;
    }
    else {
        PanelBrowse->Visible=false;
        Splitter1->Visible=false;
    }
    strinit(&StrTimeSync);
    NStrBuff=0;
    
    if (TimeSyncOut) {
        sprintf(str_path,":%d",TimeSyncPort);
        stropen(&StrTimeSync,STR_TCPSVR,STR_MODE_RW,str_path);
    }
    Timer->Interval=RefCycle;
    UpdatePlot();
    UpdateEnable();
}
// callback on form-close ---------------------------------------------------
void __fastcall TPlot::FormClose(TObject *Sender, TCloseAction &Action)
{
    trace(3,"FormClose\n");
    
    SaveOpt();
    
    if (Trace>0) traceclose();
}
// callback on form-activation ----------------------------------------------
void __fastcall TPlot::FormActivate(TObject *Sender)
{
    trace(3,"FormActivate\n");
    
    if (OpenFiles->Count>0) {
        if (CheckObs(OpenFiles->Strings[0])||OpenRaw) ReadObs(OpenFiles);
        else ReadSol(OpenFiles,0);
    }
}
// callback on form-resize --------------------------------------------------
void __fastcall TPlot::FormResize(TObject *Sender)
{
    trace(3,"FormResize\n");
    
    // suppress repeated resize callback
    if (FormWidth==Width&&FormHeight==Height) return;
    
    UpdateSize();
    Refresh();
    
    FormWidth =Width;
    FormHeight=Height;
}
// callback on drag-and-drop files ------------------------------------------
void __fastcall TPlot::DropFiles(TWMDropFiles msg)
{
    TStringList *files=new TStringList;
    AnsiString s;
    int i,n;
    char buff[1024],file[1024],*ext;
    
    trace(3,"DropFiles\n");
    
    if (ConnectState||
        (n=DragQueryFile((HDROP)msg.Drop,0xFFFFFFFF,NULL,0))<=0) {
        delete files;
        return;
    }
    for (i=0;i<n;i++) {
        DragQueryFile((HDROP)msg.Drop,i,buff,sizeof(buff));
        files->Add(buff);
    }
    strcpy(file,U2A(files->Strings[0]).c_str());
    
    if (n==1&&(ext=strrchr(file,'.'))&&
        (!strcmp(ext,".jpg")||!strcmp(ext,".jpeg")||
         !strcmp(ext,".JPG")||!strcmp(ext,".JPEG"))) {
        if (PlotType==PLOT_TRK) {
            ReadMapData(files->Strings[0]);
        }
        else if (PlotType==PLOT_SKY||PlotType==PLOT_MPS) {
            ReadSkyData(files->Strings[0]);
        }
    }
    else if (CheckObs((s=file))) {
        ReadObs(files);
    }
    else if (!BtnSol1->Down&&BtnSol2->Down) {
        ReadSol(files,1);
    }
    else {
        ReadSol(files,0);
    }
    delete files;
}
// callback on menu-open-solution-1 -----------------------------------------
void __fastcall TPlot::MenuOpenSol1Click(TObject *Sender)
{
    trace(3,"MenuOpenSol1Click\n");
    
    OpenSolDialog->Title="Open Solution 1";
    if (!OpenSolDialog->Execute()) return;
    ReadSol(OpenSolDialog->Files,0);
}
// callback on menu-open-solution-2 -----------------------------------------
void __fastcall TPlot::MenuOpenSol2Click(TObject *Sender)
{
    trace(3,"MenuOpenSol2Click\n");
    
    OpenSolDialog->Title="Open Solution 2";
    if (!OpenSolDialog->Execute()) return;
    ReadSol(OpenSolDialog->Files,1);
}
// callback on menu-open-map-image ------------------------------------------
void __fastcall TPlot::MenuOpenMapImageClick(TObject *Sender)
{
    trace(3,"MenuOpenMapImageClick\n");
    
    OpenMapDialog->Title="Open Map Image";
    OpenMapDialog->FileName=MapImageFile;
    if (!OpenMapDialog->Execute()) return;
    ReadMapData(OpenMapDialog->FileName);
}
// callback on menu-open-shapefile ------------------------------------------
void __fastcall TPlot::MenuOpenShapeClick(TObject *Sender)
{
    trace(3,"MenuOpenShapeClick\n");
    
    if (!OpenMapPathDialog->Execute()) return;
    ReadShapeFile(OpenMapPathDialog->Files);
}
// callback on menu-open-sky-image ------------------------------------------
void __fastcall TPlot::MenuOpenSkyImageClick(TObject *Sender)
{
    trace(3,"MenuOpenSkyImageClick\n");
    
    OpenMapDialog->Title="Open Sky Image";
    OpenMapDialog->FileName=SkyImageFile;
    if (!OpenMapDialog->Execute()) return;
    ReadSkyData(OpenMapDialog->FileName);
}
// callback on menu-oepn-waypoint -------------------------------------------
void __fastcall TPlot::MenuOpenWaypointClick(TObject *Sender)
{
    trace(3,"MenuOpenWaypointClick\n");
    
    if (!OpenWaypointDialog->Execute()) return;
    ReadWaypoint(OpenWaypointDialog->FileName);
}
// callback on menu-open-obs-data -------------------------------------------
void __fastcall TPlot::MenuOpenObsClick(TObject *Sender)
{
    trace(3,"MenuOpenObsClick\n");
    
    OpenObsDialog->FilterIndex=1;
    if (!OpenObsDialog->Execute()) return;
    ReadObs(OpenObsDialog->Files);
}
// callback on menu-open-nav-data -------------------------------------------
void __fastcall TPlot::MenuOpenNavClick(TObject *Sender)
{
    trace(3,"MenuOpenNavClick\n");
    
    OpenObsDialog->FilterIndex=2;
    if (!OpenObsDialog->Execute()) return;
    ReadNav(OpenObsDialog->Files);
}
// callback on menu-open-elev-mask ------------------------------------------
void __fastcall TPlot::MenuOpenElevMaskClick(TObject *Sender)
{
    trace(3,"MenuOpenElevMaskClick\n");
    
    if (!OpenElMaskDialog->Execute()) return;
    ReadElMaskData(OpenElMaskDialog->FileName);
}
// callback on menu-vis-analysis --------------------------------------------
void __fastcall TPlot::MenuVisAnaClick(TObject *Sender)
{
    if (RcvPos!=1) { // lat/lon/height
        ShowMsg("specify Receiver Position as Lat/Lon/Hgt");
        return;
    }
    if (SpanDialog->TimeStart.time==0) {
        int week;
        double tow=time2gpst(utc2gpst(timeget()),&week);
        SpanDialog->TimeStart=gpst2time(week,floor(tow/3600.0)*3600.0);
        SpanDialog->TimeEnd=timeadd(SpanDialog->TimeStart,86400.0);
        SpanDialog->TimeInt=30.0;
    }
    SpanDialog->TimeEna[0]=SpanDialog->TimeEna[1]=SpanDialog->TimeEna[2]=1;
    SpanDialog->TimeVal[0]=SpanDialog->TimeVal[1]=SpanDialog->TimeVal[2]=2;
    
    if (SpanDialog->ShowModal()==mrOk) {
        TimeStart=SpanDialog->TimeStart;
        TimeEnd=SpanDialog->TimeEnd;
        TimeInt=SpanDialog->TimeInt;
        GenVisData();
    }
    SpanDialog->TimeVal[0]=SpanDialog->TimeVal[1]=SpanDialog->TimeVal[2]=1;
}
// callback on menu-save image ----------------------------------------------
void __fastcall TPlot::MenuSaveImageClick(TObject *Sender)
{
    char file[1024],*ext;
    
    if (!SaveImageDialog->Execute()) return;
    
    strcpy(file,U2A(SaveImageDialog->FileName).c_str());
    
    if ((ext=strrchr(file,'.'))&&
        (!strcmp(ext,".jpg")||!strcmp(ext,".jpeg")||
         !strcmp(ext,".JPG")||!strcmp(ext,".JPEG"))) {
        TJPEGImage *image=new TJPEGImage;
        image->Assign(Buff);
        image->SaveToFile(SaveImageDialog->FileName);
        delete image;
    }
    else {
        Buff->SaveToFile(SaveImageDialog->FileName);
    }
}
// callback on menu-save-waypoint -------------------------------------------
void __fastcall TPlot::MenuSaveWaypointClick(TObject *Sender)
{
    trace(3,"MenuSaveWaypointClick\n");
    
    if (!SaveWaypointDialog->Execute()) return;
    
    SaveWaypoint(SaveWaypointDialog->FileName);
}
// callback on menu-save-# of sats/dop --------------------------------------
void __fastcall TPlot::MenuSaveDopClick(TObject *Sender)
{
    trace(3,"MenuSaveDopClick\n");
    
    if (!SaveDialog->Execute()) return;
    
    SaveDop(SaveDialog->FileName);
}
// callback on menu-save-snr,azel -------------------------------------------
void __fastcall TPlot::MenuSaveSnrMpClick(TObject *Sender)
{
    trace(3,"MenuSaveSnrMpClick\n");
    
    if (!SaveDialog->Execute()) return;
    
    SaveSnrMp(SaveDialog->FileName);
}
// callback on menu-save-elmask ---------------------------------------------
void __fastcall TPlot::MenuSaveElMaskClick(TObject *Sender)
{
    trace(3,"MenuSaveElMaskClick\n");
    
    if (!SaveDialog->Execute()) return;
    
    SaveElMask(SaveDialog->FileName);
}
// callback on menu-connect -------------------------------------------------
void __fastcall TPlot::MenuConnectClick(TObject *Sender)
{
    trace(3,"MenuConnectClick\n");
    
    Connect();
}
// callback on menu-disconnect ----------------------------------------------
void __fastcall TPlot::MenuDisconnectClick(TObject *Sender)
{
    trace(3,"MenuDisconnectClick\n");
    
    Disconnect();
}
// callback on menu-connection-settings -------------------------------------
void __fastcall TPlot::MenuPortClick(TObject *Sender)
{
    int i;
    
    trace(3,"MenuPortClick\n");
    
    ConnectDialog->Stream1 =RtStream[0];
    ConnectDialog->Stream2 =RtStream[1];
    ConnectDialog->Format1 =RtFormat[0];
    ConnectDialog->Format2 =RtFormat[1];
    ConnectDialog->TimeForm=RtTimeForm;
    ConnectDialog->DegForm =RtDegForm;
    ConnectDialog->FieldSep=RtFieldSep;
    ConnectDialog->TimeOutTime=RtTimeOutTime;
    ConnectDialog->ReConnTime =RtReConnTime;
    for (i=0;i< 3;i++) {
        ConnectDialog->Paths1[i]=StrPaths[0][i];
        ConnectDialog->Paths2[i]=StrPaths[1][i];
    }
    for (i=0;i< 2;i++) {
        ConnectDialog->Cmds1  [i]=StrCmds[0][i];
        ConnectDialog->Cmds2  [i]=StrCmds[1][i];
        ConnectDialog->CmdEna1[i]=StrCmdEna[0][i];
        ConnectDialog->CmdEna2[i]=StrCmdEna[1][i];
    }
    for (i=0;i<10;i++) ConnectDialog->TcpHistory [i]=StrHistory [i];
    for (i=0;i<10;i++) ConnectDialog->TcpMntpHist[i]=StrMntpHist[i];
    
    if (ConnectDialog->ShowModal()!=mrOk) return;
    
    RtStream[0]=ConnectDialog->Stream1;
    RtStream[1]=ConnectDialog->Stream2;
    RtFormat[0]=ConnectDialog->Format1;
    RtFormat[1]=ConnectDialog->Format2;
    RtTimeForm=ConnectDialog->TimeForm;
    RtDegForm =ConnectDialog->DegForm;
    RtFieldSep=ConnectDialog->FieldSep;
    RtTimeOutTime=ConnectDialog->TimeOutTime;
    RtReConnTime =ConnectDialog->ReConnTime;
    for (i=0;i< 3;i++) {
        StrPaths[0][i]=ConnectDialog->Paths1[i];
        StrPaths[1][i]=ConnectDialog->Paths2[i];
    }
    for (i=0;i< 2;i++) {
        StrCmds  [0][i]=ConnectDialog->Cmds1  [i];
        StrCmds  [1][i]=ConnectDialog->Cmds2  [i];
        StrCmdEna[0][i]=ConnectDialog->CmdEna1[i];
        StrCmdEna[1][i]=ConnectDialog->CmdEna2[i];
    }
    for (i=0;i<10;i++) StrHistory [i]=ConnectDialog->TcpHistory [i];
    for (i=0;i<10;i++) StrMntpHist[i]=ConnectDialog->TcpMntpHist[i];
}
// callback on menu-reload --------------------------------------------------
void __fastcall TPlot::MenuReloadClick(TObject *Sender)
{
    trace(3,"MenuReloadClick\n");
    
    Reload();
}
// callback on menu-clear ---------------------------------------------------
void __fastcall TPlot::MenuClearClick(TObject *Sender)
{
    trace(3,"MenuClearClick\n");
    
    Clear();
}
// callback on menu-exit-----------------------------------------------------
void __fastcall TPlot::MenuQuitClick(TObject *Sender)
{
    trace(3,"MenuQuitClick\n");
    
    Close();
}
// callback on menu-time-span/interval --------------------------------------
void __fastcall TPlot::MenuTimeClick(TObject *Sender)
{
    sol_t *sols,*sole;
    int i;
    
    trace(3,"MenuTimeClick\n");
    
    if (!TimeEna[0]) {
        if (Obs.n>0) {
            TimeStart=Obs.data[0].time;
        }
        else if (BtnSol2->Down&&SolData[1].n>0) {
            sols=getsol(SolData+1,0);
            TimeStart=sols->time;
        }
        else if (SolData[0].n>0) {
            sols=getsol(SolData,0);
            TimeStart=sols->time;
        }
    }
    if (!TimeEna[1]) {
        if (Obs.n>0) {
            TimeEnd=Obs.data[Obs.n-1].time;
        }
        else if (BtnSol2->Down&&SolData[1].n>0) {
            sole=getsol(SolData+1,SolData[1].n-1);
            TimeEnd=sole->time;
        }
        else if (SolData[0].n>0) {
            sole=getsol(SolData,SolData[0].n-1);
            TimeEnd=sole->time;
        }
    }
    for (i=0;i<3;i++) {
        SpanDialog->TimeEna[i]=TimeEna[i];
    }
    SpanDialog->TimeStart=TimeStart;
    SpanDialog->TimeEnd  =TimeEnd;
    SpanDialog->TimeInt  =TimeInt;
    SpanDialog->TimeVal[0]=!ConnectState;
    SpanDialog->TimeVal[1]=!ConnectState;
    
    if (SpanDialog->ShowModal()!=mrOk) return;
    
    if (TimeEna[0]!=SpanDialog->TimeEna[0]||
        TimeEna[1]!=SpanDialog->TimeEna[1]||
        TimeEna[2]!=SpanDialog->TimeEna[2]||
        timediff(TimeStart,SpanDialog->TimeStart)!=0.0||
        timediff(TimeEnd,SpanDialog->TimeEnd)!=0.0||
        TimeInt!=SpanDialog->TimeInt) {
        
        for (i=0;i<3;i++) TimeEna[i]=SpanDialog->TimeEna[i];
        
        TimeStart=SpanDialog->TimeStart;
        TimeEnd  =SpanDialog->TimeEnd;
        TimeInt  =SpanDialog->TimeInt;
        
        Reload();
    }
}
// callback on menu-map-image -----------------------------------------------
void __fastcall TPlot::MenuMapImgClick(TObject *Sender)
{
    trace(3,"MenuMapImgClick\n");
    
    MapAreaDialog->Show();
}
// callback on menu-sky image -----------------------------------------------
void __fastcall TPlot::MenuSkyImgClick(TObject *Sender)
{
    trace(3,"MenuSkyImgClick\n");
    
    SkyImgDialog->Show();
}
// callback on menu-vec map -------------------------------------------------
void __fastcall TPlot::MenuMapLayerClick(TObject *Sender)
{
    trace(3,"MenuMapLayerClick\n");
    
	if (VecMapDialog->ShowModal()!=mrOk) return;
	
	UpdatePlot();
    UpdateEnable();
}
// callback on menu-solution-source -----------------------------------------
void __fastcall TPlot::MenuSrcSolClick(TObject *Sender)
{
    TTextViewer *viewer=new TTextViewer(Application);
    int sel=!BtnSol1->Down&&BtnSol2->Down;
    
    trace(3,"MenuSrcSolClick\n");
    
    if (SolFiles[sel]->Count<=0) return;
    viewer->Caption=SolFiles[sel]->Strings[0];
    viewer->Option=0;
    viewer->Show();
    viewer->Read(SolFiles[sel]->Strings[0]);
}
// callback on menu-obs-data-source -----------------------------------------
void __fastcall TPlot::MenuSrcObsClick(TObject *Sender)
{
    TTextViewer *viewer;
    char file[1024],tmpfile[1024];
    int cstat;
    
    trace(3,"MenuSrcObsClick\n");
    
    if (ObsFiles->Count<=0) return;
    
    strcpy(file,U2A(ObsFiles->Strings[0]).c_str());
    cstat=rtk_uncompress(file,tmpfile);
    viewer=new TTextViewer(Application);
    viewer->Caption=ObsFiles->Strings[0];
    viewer->Option=0;
    viewer->Show();
    viewer->Read(!cstat?file:tmpfile);
    if (cstat) remove(tmpfile);
}
// callback on menu-data-qc -------------------------------------------------
void __fastcall TPlot::MenuQcObsClick(TObject *Sender)
{
    TTextViewer *viewer;
    AnsiString cmd=QcCmd,cmdexec,tmpfile=QCTMPFILE,errfile=QCERRFILE;
    int i,stat;
    
    trace(3,"MenuQcObsClick\n");
    
    if (ObsFiles->Count<=0||cmd=="") return;
    
    for (i=0;i<ObsFiles->Count;i++) cmd+=" \""+ObsFiles->Strings[i]+"\"";
    for (i=0;i<NavFiles->Count;i++) cmd+=" \""+NavFiles->Strings[i]+"\"";
    
    cmdexec=cmd+" > "+tmpfile;
    cmdexec+=" 2> "+errfile;
    stat=execcmd(cmdexec.c_str());
    
    viewer=new TTextViewer(Application);
    viewer->Option=0;
    viewer->Show();
    viewer->Read(stat?errfile:tmpfile);
    viewer->Caption=(stat?"QC Error: ":"")+cmd;
    remove(tmpfile.c_str());
    remove(errfile.c_str());
}
// callback on menu-copy-to-clipboard ---------------------------------------
void __fastcall TPlot::MenyCopyClick(TObject *Sender)
{
    trace(3,"MenuCopyClick\n");
    
    Clipboard()->Assign(Buff);
}
// callback on menu-options -------------------------------------------------
void __fastcall TPlot::MenuOptionsClick(TObject *Sender)
{
    AnsiString tlefile=TLEFile,tlesatfile=TLESatFile;
    double oopos[3],range;
    char file[1024],str_path[256];
    int timesyncout=TimeSyncOut;
    
    trace(3,"MenuOptionsClick\n");
    
    int i,rcvpos=RcvPos;
    for (i=0;i<3;i++) oopos[i]=OOPos[i];
    
    PlotOptDialog->Left=Left+Width/2-PlotOptDialog->Width/2;
    PlotOptDialog->Top=Top+Height/2-PlotOptDialog->Height/2;
    PlotOptDialog->Plot=this;
    
    if (PlotOptDialog->ShowModal()!=mrOk) return;
    
    SaveOpt();
    
    for (i=0;i<3;i++) oopos[i]-=OOPos[i];
    
    if (TLEFile!=tlefile) {
        free(TLEData.data);
        TLEData.data=NULL;
        TLEData.n=TLEData.nmax=0;
        tle_read(TLEFile.c_str(),&TLEData);
    }
    if (TLEFile!=tlefile||TLESatFile!=tlesatfile) {
        tle_name_read(TLESatFile.c_str(),&TLEData);
    }
    if (rcvpos!=RcvPos||norm(oopos,3)>1E-3||TLEFile!=tlefile) {
        if (SimObs) GenVisData(); else UpdateObs(NObs);
    }
    UpdateColor();
    UpdateSize();
    UpdateOrigin();
    UpdateInfo();
    UpdateSatMask();
    UpdateSatList();
    UpdateEnable();
    Refresh();
    Timer->Interval=RefCycle;
    
    for (i=0;i<RangeList->Count;i++) {
        strcpy(file,U2A(RangeList->Items->Strings[i]).c_str());
        
        if (sscanf(file,"%lf",&range)&&range==YRange) {
            RangeList->Selected[i]=true;
        }
    }
    if (!timesyncout&&TimeSyncOut) {
        sprintf(str_path,":%d",TimeSyncPort);
        stropen(&StrTimeSync,STR_TCPSVR,STR_MODE_RW,str_path);
    }
    else if (timesyncout&&!TimeSyncOut) {
        strclose(&StrTimeSync);
    }
}
// callback on menu-show-tool-bar -------------------------------------------
void __fastcall TPlot::MenuToolBarClick(TObject *Sender)
{
    trace(3,"MenuToolBarClick\n");
    
    MenuToolBar->Checked=!MenuToolBar->Checked;
    Panel1->Visible=MenuToolBar->Checked;
    UpdateSize();
    Refresh();
}
// callback on menu-show-status-bar -----------------------------------------
void __fastcall TPlot::MenuStatusBarClick(TObject *Sender)
{
    trace(3,"MenuStatusBarClick\n");
    
    MenuStatusBar->Checked=!MenuStatusBar->Checked;
    Panel2->Visible=MenuStatusBar->Checked;
    UpdateSize();
    Refresh();
}
// callback on menu-show-browse-panel ---------------------------------------
void __fastcall TPlot::MenuBrowseClick(TObject *Sender)
{
    trace(3,"MenuBrowseClick\n");
    
	MenuBrowse->Checked=!MenuBrowse->Checked;
    if (MenuBrowse->Checked) {
        Splitter1->Left=PanelBrowse->Width;
        PanelBrowse->Visible=true;
        Splitter1->Visible=true;
    }
    else {
        PanelBrowse->Visible=false;
        Splitter1->Visible=false;
    }
    UpdateSize();
    Refresh();
}
// callback on menu-waypoints -----------------------------------------------
void __fastcall TPlot::MenuWaypointClick(TObject *Sender)
{
    trace(3,"MenuWaypointClick\n");
    
    PntDialog->Show();
}
// callback on menu-input-monitor-1 -----------------------------------------
void __fastcall TPlot::MenuMonitor1Click(TObject *Sender)
{
    trace(3,"MenuMonitor1Click\n");
    
    Console1->Caption="Monitor RT Input 1";
    Console1->Show();
}
// callback on menu-input-monitor-2 -----------------------------------------
void __fastcall TPlot::MenuMonitor2Click(TObject *Sender)
{
    trace(3,"MenuMonitor2Click\n");
    
    Console2->Caption="Monitor RT Input 2";
    Console2->Show();
}
// callback on menu-google-earth-view ---------------------------------------
void __fastcall TPlot::MenuGEClick(TObject *Sender)
{
    AnsiString s;
    
    trace(3,"MenuGEClick\n");
    
    GoogleEarthView->Caption=
        s.sprintf("%s ver.%s %s: Google Earth View",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    GoogleEarthView->Show();
}
// callback on menu-google-map-view -----------------------------------------
void __fastcall TPlot::MenuGMClick(TObject *Sender)
{
    AnsiString s;
    GoogleMapView->Caption=
        s.sprintf("%s ver.%s %s: Google Map View",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    GoogleMapView->Show();
}
// callback on menu-center-origin -------------------------------------------
void __fastcall TPlot::MenuCenterOriClick(TObject *Sender)
{
    trace(3,"MenuCenterOriClick\n");
    
    SetRange(0,YRange);
    Refresh();
}
// callback on menu-fit-horizontal ------------------------------------------
void __fastcall TPlot::MenuFitHorizClick(TObject *Sender)
{
    trace(3,"MenuFitHorizClick\n");
    
    if (PlotType==PLOT_TRK) FitRange(0); else FitTime();
    Refresh();
}
// callback on menu-fit-vertical --------------------------------------------
void __fastcall TPlot::MenuFitVertClick(TObject *Sender)
{
    trace(3,"MenuFitVertClick\n");
    
    FitRange(0);
    Refresh();
}
// callback on menu-show-skyplot --------------------------------------------
void __fastcall TPlot::MenuShowSkyplotClick(TObject *Sender)
{
    trace(3,"MenuShowSkyplotClick\n");
    
    BtnShowSkyplot->Down=!BtnShowSkyplot->Down;
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-show-grid -----------------------------------------------
void __fastcall TPlot::MenuShowGridClick(TObject *Sender)
{
    trace(3,"MenuShowGrid\n");
    
    BtnShowGrid->Down=!BtnShowGrid->Down;
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-show-map-image ------------------------------------------
void __fastcall TPlot::MenuShowImgClick(TObject *Sender)
{
    trace(3,"MenuShowMapClick\n");
    
    BtnShowMap->Down=!BtnShowMap->Down;
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-show-track-points ---------------------------------------
void __fastcall TPlot::MenuShowTrackClick(TObject *Sender)
{
    trace(3,"MenuShowTrackClick\n");
    
    BtnShowTrack->Down=!BtnShowTrack->Down;
    if (!BtnShowTrack->Down) {
        BtnFixHoriz->Down=false;
        BtnFixVert ->Down=false;
    }
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-fix-center ----------------------------------------------
void __fastcall TPlot::MenuFixCentClick(TObject *Sender)
{
    trace(3,"MenuFixCentClick\n");
    
    BtnFixCent->Down=!BtnFixCent->Down;
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-fix-horizontal ------------------------------------------
void __fastcall TPlot::MenuFixHorizClick(TObject *Sender)
{
    trace(3,"MenuFixHorizClick\n");
    
    BtnFixHoriz->Down=!BtnFixHoriz->Down;
    Xcent=0.0;
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-fix-vertical --------------------------------------------
void __fastcall TPlot::MenuFixVertClick(TObject *Sender)
{
    trace(3,"MenuFixVertClick\n");
    
    BtnFixVert->Down=!BtnFixVert->Down;
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-show-map ------------------------------------------------
void __fastcall TPlot::MenuShowMapClick(TObject *Sender)
{
    trace(3,"MenuShowPointClick\n");
    
    BtnShowMap->Down=!BtnShowMap->Down;
    
#if 0
    if (BtnShowPoint->Down) UpdatePntsGE();
#endif
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-animation-start -----------------------------------------
void __fastcall TPlot::MenuAnimStartClick(TObject *Sender)
{
    trace(3,"MenuAnimStartClick\n");
    
    BtnAnimate->Down=true;
}
// callback on menu-animation-stop ------------------------------------------
void __fastcall TPlot::MenuAnimStopClick(TObject *Sender)
{
    trace(3,"MenuAnimStopClick\n");
    
    BtnAnimate->Down=false;
}
// callback on menu-windows-maximize ----------------------------------------
void __fastcall TPlot::MenuMaxClick(TObject *Sender)
{
    TRect rect;
    ::SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	Top=rect.Top;
	Left=rect.Left;
	Width=rect.Width();
	Height=rect.Height();
    GoogleEarthView->Hide();
    GoogleMapView->Hide();
}
// callback on menu-windows-plot-ge -----------------------------------------
void __fastcall TPlot::MenuPlotGEClick(TObject *Sender)
{
    TRect rect;
    ::SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	Top=rect.Top;
	Left=rect.Left;
	Width=rect.Width()/2;
	Height=rect.Height();
    GoogleEarthView->Top=Top;
    GoogleEarthView->Left=Width;
    GoogleEarthView->Width=Width;
    GoogleEarthView->Height=Height;
    GoogleMapView->Hide();
    GoogleEarthView->Show();
}
// callback on menu-windows-plot-gm -----------------------------------------
void __fastcall TPlot::MenuPlotGMClick(TObject *Sender)
{
    TRect rect;
    ::SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	Top=rect.Top;
	Left=rect.Left;
	Width=rect.Width()/2;
	Height=rect.Height();
    GoogleMapView->Top=Top;
    GoogleMapView->Left=Width;
    GoogleMapView->Width=Width;
    GoogleMapView->Height=Height;
    GoogleEarthView->Hide();
    GoogleMapView->Show();
}
// callback on menu-windows-plot-ge/gm --------------------------------------
void __fastcall TPlot::MenuPlotGEGMClick(TObject *Sender)
{
    TRect rect;
    ::SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	Top=rect.Top;
	Left=rect.Left;
	Width=rect.Width()/2;
	Height=rect.Height();
    GoogleEarthView->Top=Top;
    GoogleEarthView->Left=Width;
    GoogleEarthView->Width=Width;
    GoogleEarthView->Height=Height/2;
    GoogleMapView->Top=Top+Height/2;
    GoogleMapView->Left=Width;
    GoogleMapView->Width=Width;
    GoogleMapView->Height=Height/2;
    GoogleEarthView->Show();
    GoogleMapView->Show();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::DispGesture(TObject *Sender, const TGestureEventInfo &EventInfo,
          bool &Handled)
{
#if 0
    AnsiString s;
    int b,e;
    
    b=EventInfo.Flags.Contains(gfBegin);
    e=EventInfo.Flags.Contains(gfEnd);
    
	if (EventInfo.GestureID==Controls::igiZoom) {
        s.sprintf("zoom: Location=%d,%d,Flag=%d,%d,Angle=%.1f,Disnance=%d",
                  EventInfo.Location.X,EventInfo.Location.Y,b,e,
                  EventInfo.Angle,EventInfo.Distance);
        Message1->Caption=s;
    }
	else if (EventInfo.GestureID==Controls::igiPan) {
        s.sprintf("pan: Location=%d,%d,Flag=%d,%d,Angle=%.1f,Disnance=%d",
                  EventInfo.Location.X,EventInfo.Location.Y,b,e,
                  EventInfo.Angle,EventInfo.Distance);
        Message1->Caption=s;
    }
	else if (EventInfo.GestureID==Controls::igiRotate) {
        s.sprintf("rotate: Location=%d,%d,Flag=%d,%d,Angle=%.1f,Disnance=%d",
                  EventInfo.Location.X,EventInfo.Location.Y,b,e,
                  EventInfo.Angle,EventInfo.Distance);
        Message1->Caption=s;
    }
#endif
}
// callback on menu-about ---------------------------------------------------
void __fastcall TPlot::MenuAboutClick(TObject *Sender)
{
    trace(3,"MenuAboutClick\n");
    
    AboutDialog->About=PRGNAME;
    AboutDialog->IconIndex=2;
    AboutDialog->ShowModal();
}
// callback on button-connect/disconnect ------------------------------------
void __fastcall TPlot::BtnConnectClick(TObject *Sender)
{
    trace(3,"BtnConnectClick\n");
    
    if (!ConnectState) MenuConnectClick(Sender);
    else MenuDisconnectClick(Sender);
}
// callback on button-solution-1 --------------------------------------------
void __fastcall TPlot::BtnSol1Click(TObject *Sender)
{
    trace(3,"BtnSol1Click\n");
    
    BtnSol12->Down=false;
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// callback on button-solution-2 --------------------------------------------
void __fastcall TPlot::BtnSol2Click(TObject *Sender)
{
    trace(3,"BtnSol2Click\n");
    
    BtnSol12->Down=false;
    UpdateTime();
    UpdatePlot(); 
    UpdateEnable();
}
// callback on button-solution-1-2 ------------------------------------------
void __fastcall TPlot::BtnSol12Click(TObject *Sender)
{
    trace(3,"BtnSol12Click\n");
    
    BtnSol1->Down=false;
    BtnSol2->Down=false;
    UpdateTime();
    UpdatePlot(); 
    UpdateEnable();
}
// callback on button-solution-1 double-click -------------------------------
void __fastcall TPlot::BtnSol1DblClick(TObject *Sender)
{
    trace(3,"BtnSol1DblClick\n");
    
    MenuOpenSol1Click(Sender);
}
// callback on button-solution-2 double-click -------------------------------
void __fastcall TPlot::BtnSol2DblClick(TObject *Sender)
{
    trace(3,"BtnSol2DblClick\n");
    
    MenuOpenSol2Click(Sender);
}
// callback on button-show-map-image ----------------------------------------
void __fastcall TPlot::BtnShowImgClick(TObject *Sender)
{
    trace(3,"BtnShowMapClick\n");
    
    UpdateEnable();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnShowSkyplotClick(TObject *Sender)
{
    trace(3,"BtnShowSkyplotClick\n");
    
	UpdateEnable();
    Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TPlot::BtnShowGridClick(TObject *Sender)
{
    trace(3,"BtnShowGridClick\n");
    
	UpdateEnable();
    Refresh();
}
// callback on button-plot-1-onoff ------------------------------------------
void __fastcall TPlot::BtnOn1Click(TObject *Sender)
{
    trace(3,"BtnOn1Click\n");
    
    UpdateSize();
    Refresh();
}
// callback on button-plot-2-onoff-------------------------------------------
void __fastcall TPlot::BtnOn2Click(TObject *Sender)
{
    trace(3,"BtnOn2Click\n");
    
    UpdateSize();
    Refresh();
}
// callback on button-plot-3-onoff ------------------------------------------
void __fastcall TPlot::BtnOn3Click(TObject *Sender)
{
    trace(3,"BtnOn3Click\n");
    
    UpdateSize();
    Refresh();
}
// callback on button-range-list --------------------------------------------
void __fastcall TPlot::BtnRangeListClick(TObject *Sender)
{
    trace(3,"BtnRangeListClick\n");
    
    RangeList->Visible=!RangeList->Visible;
}
// callback on button-range-list --------------------------------------------
void __fastcall TPlot::RangeListClick(TObject *Sender)
{
    double range;
    char file[1024];
    int i;
    
    trace(3,"RangeListClick\n");
    
    RangeList->Visible=false;
    if ((i=RangeList->ItemIndex)<0) return;
    
    strcpy(file,U2A(RangeList->Items->Strings[i]).c_str());
    
    if (!sscanf(file,"%lf",&range)) return;
    
    YRange=range;
    SetRange(0,YRange);
    UpdatePlot();
    UpdateEnable();
}
// callback on button-center-origin -----------------------------------------
void __fastcall TPlot::BtnCenterOriClick(TObject *Sender)
{
    trace(3,"BtnCenterOriClick\n");
    
    RangeList->Visible=false;
    MenuCenterOriClick(Sender);
}
// callback on button-fit-horzontal------------------------------------------
void __fastcall TPlot::BtnFitHorizClick(TObject *Sender)
{
    trace(3,"BtnFitHorizClick\n");
    
    MenuFitHorizClick(Sender);
}
// callback on button-fit-vertical ------------------------------------------
void __fastcall TPlot::BtnFitVertClick(TObject *Sender)
{
    trace(3,"BtnFitVertClick\n");
    
    MenuFitVertClick(Sender);
}
// callback on button show-track-points -------------------------------------
void __fastcall TPlot::BtnShowTrackClick(TObject *Sender)
{
    trace(3,"BtnShowTrackClick\n");
    
    if (!BtnShowTrack->Down) {
        BtnFixHoriz->Down=false;
        BtnFixVert ->Down=false;
    }
    UpdatePlot();
    UpdateEnable();
}
// callback on button-fix-horizontal ----------------------------------------
void __fastcall TPlot::BtnFixHorizClick(TObject *Sender)
{
    trace(3,"BtnFixHorizClick\n");
    
    Xcent=0.0;
    UpdatePlot();
    UpdateEnable();
}
// callback on button-fix-vertical ------------------------------------------
void __fastcall TPlot::BtnFixVertClick(TObject *Sender)
{
    trace(3,"BtnFixVertClick\n");
    
    UpdatePlot();
    UpdateEnable();
}
// callback on button-fix-center --------------------------------------------
void __fastcall TPlot::BtnFixCentClick(TObject *Sender)
{
    trace(3,"BtnFixCentClick\n");
    
    UpdatePlot();
    UpdateEnable();
}
// callback on button-show-waypoints ----------------------------------------
void __fastcall TPlot::BtnShowMapClick(TObject *Sender)
{
    trace(3,"BtnShowPointClick\n");
    
    UpdatePlot();
    UpdateEnable();
}
// callback on button-options -----------------------------------------------
void __fastcall TPlot::BtnOptionsClick(TObject *Sender)
{
    trace(3,"BtnOptionsClick\n");
    
    MenuOptionsClick(Sender);
}
// callback on button-ge-view -----------------------------------------------
void __fastcall TPlot::BtnGEClick(TObject *Sender)
{
    trace(3,"BtnGEClick\n");
    
    MenuGEClick(Sender);
}
// callback on button-gm-view -----------------------------------------------
void __fastcall TPlot::BtnGMClick(TObject *Sender)
{
    trace(3,"BtnGMClick\n");
    
    MenuGMClick(Sender);
}
// callback on button-animation ---------------------------------------------
void __fastcall TPlot::BtnAnimateClick(TObject *Sender)
{
    trace(3,"BtnAnimateClick\n");
    
    UpdateEnable();
}
// callback on button-clear -------------------------------------------------
void __fastcall TPlot::BtnClearClick(TObject *Sender)
{
    trace(3,"BtnClearClick\n");
    
    MenuClearClick(Sender);
}
// callback on button-reload ------------------------------------------------
void __fastcall TPlot::BtnReloadClick(TObject *Sender)
{
    trace(3,"BtnReloadClick\n");
    
    MenuReloadClick(Sender);
}
// callback on button-message 2 ---------------------------------------------
void __fastcall TPlot::BtnMessage2Click(TObject *Sender)
{
	if (++PointType>2) PointType=0;
}
// callback on plot-type selection change -----------------------------------
void __fastcall TPlot::PlotTypeSChange(TObject *Sender)
{
    int i;
    
    trace(3,"PlotTypeSChnage\n");
    
    for (i=0;*PTypes[i];i++) {
        if (PlotTypeS->Text==PTypes[i]) UpdateType(i);
    }
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// callback on quality-flag selection change --------------------------------
void __fastcall TPlot::QFlagChange(TObject *Sender)
{
    trace(3,"QFlagChange\n");
    
    UpdatePlot();
    UpdateEnable();
}
// callback on obs-type selection change ------------------------------------
void __fastcall TPlot::ObsTypeChange(TObject *Sender)
{
    trace(3,"ObsTypeChange\n");
    
    UpdatePlot();
    UpdateEnable();
}
// callback on dop-type selection change ------------------------------------
void __fastcall TPlot::DopTypeChange(TObject *Sender)
{
    trace(3,"DopTypeChange\n");
    
    UpdatePlot();
    UpdateEnable();
}
// callback on satellite-list selection change ------------------------------
void __fastcall TPlot::SatListChange(TObject *Sender)
{
    trace(3,"SatListChange\n");
    
    UpdateSatSel();
    UpdatePlot();
    UpdateEnable();
}
// callback on time scroll-bar change ---------------------------------------
void __fastcall TPlot::TimeScrollChange(TObject *Sender)
{
    int sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"TimeScrollChange\n");
    
    if (PlotType<=PLOT_NSAT||PlotType==PLOT_RES) {
        SolIndex[sel]=TimeScroll->Position;
    }
    else {
        ObsIndex=TimeScroll->Position;
    }
    UpdatePlot();
}
// callback on paint --------------------------------------------------------
void __fastcall TPlot::DispPaint(TObject *Sender)
{
    trace(3,"DispPaint\n");
    
    UpdateDisp();
}
// callback on mouse-down event ---------------------------------------------
void __fastcall TPlot::DispMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    trace(3,"DispMouseDown: X=%d Y=%d\n",X,Y);
    
    X0=X; Y0=Y; Xcent0=Xcent;
    
    Drag=Shift.Contains(ssLeft)?1:(Shift.Contains(ssRight)?11:0);
    
    if (PlotType==PLOT_TRK) {
        MouseDownTrk(X,Y);
    }
    else if (PlotType<=PLOT_NSAT||PlotType==PLOT_RES||PlotType==PLOT_SNR) {
        MouseDownSol(X,Y);
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP) {
        MouseDownObs(X,Y);
    }
    else Drag=0;
    
    RangeList->Visible=false;
}
// callback on mouse-move event ---------------------------------------------
void __fastcall TPlot::DispMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
    double x,y,xs,ys,dx,dy,dxs,dys;
    
    if (X==Xn&&Y==Yn) return;
    
    trace(4,"DispMouseMove: X=%d Y=%d\n",X,Y);
    
    Xn=X; Yn=Y;
    dx=(X0-X)*Xs;
    dy=(Y-Y0)*Ys;
    dxs=pow(2.0,(X0-X)/100.0);
    dys=pow(2.0,(Y-Y0)/100.0);
    
    if (Drag==0) {
        UpdatePoint(X,Y);
    }
    else if (PlotType==PLOT_TRK) {
        MouseMoveTrk(X,Y,dx,dy,dxs,dys);
    }
    else if (PlotType<=PLOT_NSAT||PlotType==PLOT_RES||PlotType==PLOT_SNR) {
        MouseMoveSol(X,Y,dx,dy,dxs,dys);
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP) {
        MouseMoveObs(X,Y,dx,dy,dxs,dys);
    }
}
// callback on mouse-up event -----------------------------------------------
void __fastcall TPlot::DispMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    trace(3,"DispMouseUp: X=%d Y=%d\n",X,Y);
    
    Drag=0;
    Screen->Cursor=crDefault;
    Refresh();
    Refresh_GEView();
}
// callback on mouse-double-click -------------------------------------------
void __fastcall TPlot::DispDblClick(TObject *Sender)
{
    TPoint p((int)X0,(int)Y0);
    double x,y;
    
    trace(3,"DispDblClick X=%d Y=%d\n",p.x,p.y);
    
    if (BtnFixHoriz->Down) return;
    
    if (PlotType==PLOT_TRK) {
        GraphT->ToPos(p,x,y);
        GraphT->SetCent(x,y);
        Refresh();
        Refresh_GEView();
    }
    else if (PlotType<=PLOT_NSAT||PlotType==PLOT_RES||PlotType==PLOT_SNR) {
        GraphG[0]->ToPos(p,x,y);
        SetCentX(x);
        Refresh();
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP) {
        GraphR->ToPos(p,x,y);
        SetCentX(x);
        Refresh();
    }
}
// callback on mouse-leave event --------------------------------------------
void __fastcall TPlot::DispMouseLeave(TObject *Sender)
{
    trace(3,"DispMouseLeave\n");
    
    Xn=Yn=-1;
    Panel22->Visible=false;
    Message2->Caption="";
}
// callback on mouse-down event on track-plot -------------------------------
void __fastcall TPlot::MouseDownTrk(int X, int Y)
{
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"MouseDownTrk: X=%d Y=%d\n",X,Y);
    
    if (Drag==1&&(i=SearchPos(X,Y))>=0) {
        SolIndex[sel]=i;
        UpdateTime();
        UpdateInfo();
        Drag=0;
        Refresh();
    }
    else {
        GraphT->GetCent(Xc,Yc);
        GraphT->GetScale(Xs,Ys);
        Screen->Cursor=Drag==1?crSizeAll:crVSplit;
    }
}
// callback on mouse-down event on solution-plot ----------------------------
void __fastcall TPlot::MouseDownSol(int X, int Y)
{
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TPoint pnt,p(X,Y);
    gtime_t time={0};
    sol_t *data;
    double x,xl[2],yl[2];
    int i,area=-1,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"MouseDownSol: X=%d Y=%d\n",X,Y);
    
    if (PlotType==PLOT_SNR) {
        if (0<=ObsIndex&&ObsIndex<NObs) time=Obs.data[IndexObs[ObsIndex]].time;
    }
    else {
        if ((data=getsol(SolData+sel,SolIndex[sel]))) time=data->time;
    }
    if (time.time&&!BtnFixHoriz->Down) {
        
        x=TimePos(time);
        
        GraphG[0]->GetLim(xl,yl);
        GraphG[0]->ToPoint(x,yl[1],pnt);
        
        if ((X-pnt.x)*(X-pnt.x)+(Y-pnt.y)*(Y-pnt.y)<25) {
            Screen->Cursor=crSizeWE;
            Drag=20;
            Refresh();
            return;
        }
    }
    for (i=0;i<3;i++) {
        if (!btn[i]->Down||(i!=1&&PlotType==PLOT_SNR)) continue;
        
        GraphG[i]->GetCent(Xc,Yc);
        GraphG[i]->GetScale(Xs,Ys);
        area=GraphG[i]->OnAxis(p);
        
        if (Drag==1&&area==0) {
            Screen->Cursor=crSizeAll;
            Drag+=i;
            return;
        }
        else if (area==1) {
            Screen->Cursor=Drag==1?crSizeNS:crVSplit;
            Drag+=i+4;
            return;
        }
        else if (area==0) break;
    }
    if (area==0||area==8) {
        Screen->Cursor=Drag==1?crSizeWE:crHSplit;
        Drag+=3;
    }
    else Drag=0;
}
// callback on mouse-down event on observation-data-plot --------------------
void __fastcall TPlot::MouseDownObs(int X, int Y)
{
    TPoint pnt,p(X,Y);
    double x,xl[2],yl[2];
    int area;
    
    trace(3,"MouseDownObs: X=%d Y=%d\n",X,Y);
    
    if (0<=ObsIndex&&ObsIndex<NObs&&!BtnFixHoriz->Down) {
        
        x=TimePos(Obs.data[IndexObs[ObsIndex]].time);
        
        GraphR->GetLim(xl,yl);
        GraphR->ToPoint(x,yl[1],pnt);
        
        if ((X-pnt.x)*(X-pnt.x)+(Y-pnt.y)*(Y-pnt.y)<25) {
            Screen->Cursor=crSizeWE;
            Drag=20;
            Refresh();
            return;
        }
    }
    GraphR->GetCent(Xc,Yc);
    GraphR->GetScale(Xs,Ys);
    area=GraphR->OnAxis(p);
    
    if (area==0||area==8) {
        Screen->Cursor=Drag==1?crSizeWE:crHSplit;
        Drag+=3;
    }
    else Drag=0;
}
// callback on mouse-move event on track-plot -------------------------------
void __fastcall TPlot::MouseMoveTrk(int X, int Y, double dx, double dy,
    double dxs, double dys)
{
    trace(4,"MouseMoveTrk: X=%d Y=%d\n",X,Y);
    
    if (Drag==1&&!BtnFixHoriz->Down) {
        GraphT->SetCent(Xc+dx,Yc+dy);
    }
    else if (Drag>1) {
        GraphT->SetScale(Xs*dys,Ys*dys);
    }
    BtnCenterOri->Down=false;
    Refresh();
}
// callback on mouse-move event on solution-plot ----------------------------
void __fastcall TPlot::MouseMoveSol(int X, int Y, double dx, double dy,
    double dxs, double dys)
{
    TPoint p1,p2,p(X,Y);
    double x,y,xs,ys;
    int i,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(4,"MouseMoveSol: X=%d Y=%d\n",X,Y);
    
    if (Drag<=4) {
        for (i=0;i<3;i++) {
            GraphG[i]->GetCent(x,y);
            if (!BtnFixHoriz->Down) {
                x=Xc+dx;
            }
            if (!BtnFixVert->Down||!BtnFixVert->Enabled) {
                y=i==Drag-1?Yc+dy:y;
            }
            GraphG[i]->SetCent(x,y);
            SetCentX(x);
        }
        if (BtnFixHoriz->Down) {
            GraphG[0]->GetPos(p1,p2);
            Xcent=Xcent0+2.0*(X-X0)/(p2.x-p1.x);
            if (Xcent> 1.0) Xcent= 1.0;
            if (Xcent<-1.0) Xcent=-1.0;
        }
    }
    else if (Drag<=7) {
        GraphG[Drag-5]->GetCent(x,y);
        if (!BtnFixVert->Down||!BtnFixVert->Enabled) {
            y=Yc+dy;
        }
        GraphG[Drag-5]->SetCent(x,y);
    }
    else if (Drag<=14) {
        for (i=0;i<3;i++) {
            GraphG[i]->GetScale(xs,ys);
            GraphG[i]->SetScale(Xs*dxs,ys);
        }
        SetScaleX(Xs*dxs);
    }
    else if (Drag<=17) {
        GraphG[Drag-15]->GetScale(xs,ys);
        GraphG[Drag-15]->SetScale(xs,Ys*dys);
    }
    else if (Drag==20) {
        GraphG[0]->ToPos(p,x,y);
        if (PlotType==PLOT_SNR) {
            for (i=0;i<NObs;i++) {
                if (TimePos(Obs.data[IndexObs[i]].time)>=x) break;
            }
            ObsIndex=i<NObs?i:NObs-1;
        }
        else {
            for (i=0;i<SolData[sel].n;i++) {
                if (TimePos(SolData[sel].data[i].time)>=x) break;
            }
            SolIndex[sel]=i<SolData[sel].n?i:SolData[sel].n-1;
        }
        UpdateTime();
    }
    BtnCenterOri->Down=false;
    Refresh();
}
// callback on mouse-move events on observataion-data-plot ------------------
void __fastcall TPlot::MouseMoveObs(int X, int Y, double dx, double dy,
    double dxs, double dys)
{
    TPoint p1,p2,p(X,Y);
    double x,y,xs,ys;
    int i;
    
    trace(4,"MouseMoveObs: X=%d Y=%d\n",X,Y);
    
    if (Drag<=4) {
        GraphR->GetCent(x,y);
        if (!BtnFixHoriz->Down) x=Xc+dx;
        if (!BtnFixVert ->Down) y=Yc+dy;
        GraphR->SetCent(x,y);
        SetCentX(x);
        
        if (BtnFixHoriz->Down) {
            GraphR->GetPos(p1,p2);
            Xcent=Xcent0+2.0*(X-X0)/(p2.x-p1.x);
            if (Xcent> 1.0) Xcent= 1.0;
            if (Xcent<-1.0) Xcent=-1.0;
        }
    }
    else if (Drag<=14) {
        GraphR->GetScale(xs,ys);
        GraphR->SetScale(Xs*dxs,ys);
        SetScaleX(Xs*dxs);
    }
    else if (Drag==20) {
        GraphR->ToPos(p,x,y);
        for (i=0;i<NObs;i++) {
            if (TimePos(Obs.data[IndexObs[i]].time)>=x) break;
        }
        ObsIndex=i<NObs?i:NObs-1;
        UpdateTime();
    }
    BtnCenterOri->Down=false;
    Refresh();
}
// callback on mouse-wheel events -------------------------------------------
void __fastcall TPlot::MouseWheel(TObject *Sender, TShiftState Shift,
      int WheelDelta, TPoint &MousePos, bool &Handled)
{
    TPoint p(Xn,Yn);
    double xs,ys,ds=pow(2.0,-WheelDelta/1200.0);
    int i,area=-1;
    
    Handled=true;
    
    trace(4,"MouseWheel: WheelDelta=%d\n",WheelDelta);
    
    if (Xn<0||Yn<0) return;
    
    if (PlotType==PLOT_TRK) { // track-plot
        GraphT->GetScale(xs,ys);
        GraphT->SetScale(xs*ds,ys*ds);
    }
    else if (PlotType<=PLOT_NSAT||PlotType==PLOT_RES||PlotType==PLOT_SNR) {
        
        for (i=0;i<3;i++) {
            if (PlotType==PLOT_SNR&&i!=1) continue;
            area=GraphG[i]->OnAxis(p);
            if (area==0||area==1||area==2) {
                GraphG[i]->GetScale(xs,ys);
                GraphG[i]->SetScale(xs,ys*ds);
            }
            else if (area==0) break;
        }
        if (area==8) {
            for (i=0;i<3;i++) {
                GraphG[i]->GetScale(xs,ys);
                GraphG[i]->SetScale(xs*ds,ys);
                SetScaleX(xs*ds);
            }
        }
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP) {
        area=GraphR->OnAxis(p);
        if (area==0||area==8) {
            GraphR->GetScale(xs,ys);
            GraphR->SetScale(xs*ds,ys);
            SetScaleX(xs*ds);
        }
    }
    else return;
    
    Refresh();
}
// callback on key-press events ---------------------------------------------
void __fastcall TPlot::CMDialogKey(Messages::TWMKey &Message)
{
    trace(3,"CMDialogKey:\n");
    
    if (Message.CharCode!=VK_UP  &&Message.CharCode!=VK_DOWN &&
        Message.CharCode!=VK_LEFT&&Message.CharCode!=VK_RIGHT) {
        TForm::Dispatch(&Message);
    }
}
// callback on key-down events ----------------------------------------------
void __fastcall TPlot::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    double sfact=1.05,fact=Shift.Contains(ssShift)?1.0:10.0;
    double xc,yc,yc1,yc2,yc3,xs,ys,ys1,ys2,ys3;
    int key=Shift.Contains(ssCtrl)?10:0;
    
    trace(3,"FormKeyDown:\n");
    
    switch (Key) {
        case VK_UP   : key+=1; break;
        case VK_DOWN : key+=2; break;
        case VK_LEFT : key+=3; break;
        case VK_RIGHT: key+=4; break;
        default: return;
    }
    if (Shift.Contains(ssAlt)) return;
    
    Key=0; // stop dispatch key event
    
    if (PlotType==PLOT_TRK) {
        GraphT->GetCent(xc,yc);
        GraphT->GetScale(xs,ys);
        if (key== 1) {if (!BtnFixHoriz->Down) yc+=fact*ys;}
        if (key== 2) {if (!BtnFixHoriz->Down) yc-=fact*ys;}
        if (key== 3) {if (!BtnFixHoriz->Down) xc-=fact*xs;}
        if (key== 4) {if (!BtnFixHoriz->Down) xc+=fact*xs;}
        if (key==11) {xs/=sfact; ys/=sfact;}
        if (key==12) {xs*=sfact; ys*=sfact;}
        GraphT->SetCent(xc,yc);
        GraphT->SetScale(xs,ys);
    }
    else if (PlotType<=PLOT_NSAT||PlotType==PLOT_RES) {
        GraphG[0]->GetCent(xc,yc1);
        GraphG[1]->GetCent(xc,yc2);
        GraphG[2]->GetCent(xc,yc3);
        GraphG[0]->GetScale(xs,ys1);
        GraphG[1]->GetScale(xs,ys2);
        GraphG[2]->GetScale(xs,ys3);
        if (key== 1) {if (!BtnFixVert ->Down) yc1+=fact*ys1; yc2+=fact*ys2; yc3+=fact*ys3;}
        if (key== 2) {if (!BtnFixVert ->Down) yc1-=fact*ys1; yc2-=fact*ys2; yc3-=fact*ys3;}
        if (key== 3) {if (!BtnFixHoriz->Down) xc-=fact*xs;}
        if (key== 4) {if (!BtnFixHoriz->Down) xc+=fact*xs;}
        if (key==11) {ys1/=sfact; ys2/=sfact; ys3/=sfact;}
        if (key==12) {ys1*=sfact; ys2*=sfact; ys3*=sfact;}
        if (key==13) xs*=sfact;
        if (key==14) xs/=sfact;
        GraphG[0]->SetCent(xc,yc1);
        GraphG[1]->SetCent(xc,yc2);
        GraphG[2]->SetCent(xc,yc3);
        GraphG[0]->SetScale(xs,ys1);
        GraphG[1]->SetScale(xs,ys2);
        GraphG[2]->SetScale(xs,ys3);
    }
    else if (PlotType==PLOT_OBS||PlotType==PLOT_DOP||PlotType==PLOT_SNR) {
        GraphR->GetCent(xc,yc);
        GraphR->GetScale(xs,ys);
        if (key== 1) {if (!BtnFixVert ->Down) yc+=fact*ys;}
        if (key== 2) {if (!BtnFixVert ->Down) yc-=fact*ys;}
        if (key== 3) {if (!BtnFixHoriz->Down) xc-=fact*xs;}
        if (key== 4) {if (!BtnFixHoriz->Down) xc+=fact*xs;}
        if (key==11) ys/=sfact;
        if (key==12) xs*=sfact;
        if (key==13) xs*=sfact;
        if (key==14) xs/=sfact;
        GraphR->SetCent(xc,yc);
        GraphR->SetScale(xs,ys);
    }
    Refresh();
}
// callback on interval-timer -----------------------------------------------
void __fastcall TPlot::TimerTimer(TObject *Sender)
{
    TColor color[]={clRed,clBtnFace,CLORANGE,clGreen,clLime};
    TPanel *strstatus[]={StrStatus1,StrStatus2};
    TConsole *console[]={Console1,Console2};
    AnsiString connectmsg="",s;
    static unsigned char buff[16384];
    solopt_t opt=solopt_default;
    sol_t *sol;
    const gtime_t ts={0};
    gtime_t time={0};
    double tint=TimeEna[2]?TimeInt:0.0,pos[3],ep[6];
    int i,j,n,inb,inr,cycle,nmsg[2]={0},stat,istat;
    int sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    char msg[MAXSTRMSG]="",tstr[32];
    
    trace(4,"TimeTimer\n");
    
    if (!ConnectState) {
        StrStatus1->Color=clBtnFace;
        StrStatus2->Color=clBtnFace;
        ConnectMsg->Caption="";
    }
    if (ConnectState) { // real-time input mode
        for (i=0;i<2;i++) {
            opt.posf =RtFormat[i];
            opt.times=RtTimeForm==0?0:RtTimeForm-1;
            opt.timef=RtTimeForm>=1;
            opt.degf =RtDegForm;
            strcpy(opt.sep,RtFieldSep.c_str());
            strsum(Stream+i,&inb,&inr,NULL,NULL);
            stat=strstat(Stream+i,msg);
            strstatus[i]->Color=color[stat<3?stat+1:3];
            if (*msg&&strcmp(msg,"localhost")) {
                connectmsg+=s.sprintf("(%d) %s ",i+1,msg);
            }
            while ((n=strread(Stream+i,buff,sizeof(buff)))>0) {
                
                for (j=0;j<n;j++) {
                    istat=inputsol(buff[j],ts,ts,tint,0,&opt,SolData+i);
                    if (istat==0) continue;
                    if (istat<0) { // disconnect received
                        Disconnect();
                        return;
                    }
                    if (Week==0&&SolData[i].n==1) { // first data
                        if (PlotType>PLOT_NSAT) {
                            UpdateType(PLOT_TRK);
                        }
                        time2gpst(SolData[i].time,&Week);
                        UpdateOrigin();
                        ecef2pos(SolData[i].data[0].rr,pos);
                        GoogleEarthView->SetView(pos[0]*R2D,pos[1]*R2D,0.0,0.0);
                        GoogleMapView->SetView(pos[0]*R2D,pos[1]*R2D,13);
                    }
                    nmsg[i]++;
                }
                console[i]->AddMsg(buff,n);
            }
            if (nmsg[i]>0) {
                strstatus[i]->Color=color[4];
                SolIndex[i]=SolData[i].n-1;
            }
        }
        ConnectMsg->Caption=connectmsg;
        
        if (nmsg[0]<=0&&nmsg[1]<=0) return;
    }
    else if (BtnAnimate->Enabled&&BtnAnimate->Down) { // animation mode
        cycle=AnimCycle<=0?1:AnimCycle;
        
        if (PlotType<=PLOT_NSAT||PlotType==PLOT_RES) {
            SolIndex[sel]+=cycle;
            if (SolIndex[sel]>=SolData[sel].n-1) {
                SolIndex[sel]=SolData[sel].n-1;
                BtnAnimate->Down=false;
            }
        }
        else {
            ObsIndex+=cycle;
            if (ObsIndex>=NObs-1) {
                ObsIndex=NObs-1;
                BtnAnimate->Down=false;
            }
        }
    }
    else if (TimeSyncOut) { // time sync
        time.time = 0;
        while (strread(&StrTimeSync, (unsigned char *)StrBuff+NStrBuff, 1)) {
            if (++NStrBuff >= 1023) {
                NStrBuff = 0;
                continue;
            }
            if (StrBuff[NStrBuff-1]=='\n') {
                StrBuff[NStrBuff-1]='\0';
                if (sscanf(StrBuff,"%lf/%lf/%lf %lf:%lf:%lf",ep,ep+1,ep+2,
                           ep+3,ep+4,ep+5)>=6) {
                    time=epoch2time(ep);
                }
                NStrBuff = 0;
            }
        }
        if (time.time&&(PlotType<=PLOT_NSAT||PlotType<=PLOT_RES)) {
           i=SolIndex[sel];
           if (!(sol=getsol(SolData+sel,i))) return;
           double tt=timediff(sol->time,time);
           if (tt<-DTTOL) {
               for (;i<SolData[sel].n;i++) {
                   if (!(sol=getsol(SolData+sel,i))) continue;
                   if (timediff(sol->time,time)>=-DTTOL) {
                       i--;
                       break;
                   }
               }
           }
           else if (tt>DTTOL) {
               for (;i>=0;i--) {
                   if (!(sol=getsol(SolData+sel,i))) continue;
                   if (timediff(sol->time,time)<=DTTOL) break;
               }
           }
           SolIndex[sel]=MAX(0,MIN(SolData[sel].n-1,i));
        }
        else return;
    }
    else return;
    
    UpdateTime();
    UpdatePlot();
}
// set center of x-axis -----------------------------------------------------
void __fastcall TPlot::SetCentX(double c)
{
    double x,y;
    int i;
    
    trace(3,"SetCentX: c=%.3f:\n",c);
    
    GraphR->GetCent(x,y);
    GraphR->SetCent(c,y);
    for (i=0;i<3;i++) {
        GraphG[i]->GetCent(x,y);
        GraphG[i]->SetCent(c,y);
    }
}
// set scale of x-axis ------------------------------------------------------
void __fastcall TPlot::SetScaleX(double s)
{
    double xs,ys;
    int i;
    
    trace(3,"SetScaleX: s=%.3f:\n",s);
    
    GraphR->GetScale(xs,ys);
    GraphR->SetScale(s ,ys);
    for (i=0;i<3;i++) {
        GraphG[i]->GetScale(xs,ys);
        GraphG[i]->SetScale(s, ys);
    }
}
// update plot-type with fit-range ------------------------------------------
void __fastcall TPlot::UpdateType(int type)
{
    trace(3,"UpdateType: type=%d\n",type);
    
    PlotType=type;
    
    if (AutoScale&&PlotType<=PLOT_SOLA&&(SolData[0].n>0||SolData[1].n>0)) {
        FitRange(0);
    }
    else {
        SetRange(0,YRange);
    }
    UpdatePlotType();
}
// update size of plot ------------------------------------------------------
void __fastcall TPlot::UpdateSize(void)
{
    TSpeedButton *btn[]={BtnOn1,BtnOn2,BtnOn3};
    TPoint p1(0,0),p2(Disp->Width,Disp->Height);
    double xs,ys;
    int i,n,h,tmargin,bmargin,rmargin,lmargin;
    
    trace(3,"UpdateSize\n");
    
    tmargin=5;                             // top margin
    bmargin=(int)(Disp->Font->Size*1.5)+3; // bottom
    rmargin=8;                             // right
    lmargin=Disp->Font->Size*3+15;         // left
    
    GraphT->SetPos(p1,p2);
    GraphS->SetPos(p1,p2);
    GraphS->GetScale(xs,ys);
    xs=MAX(xs,ys);
    GraphS->SetScale(xs,xs);
    p1.x+=lmargin; p1.y+=tmargin;
    p2.x-=rmargin; p2.y=p2.y-bmargin;
    GraphR->SetPos(p1,p2);
    
    p1.y=tmargin; p2.y=p1.y;
    for (i=n=0;i<3;i++) if (btn[i]->Down) n++;
    for (i=0;i<3;i++) {
        if (!btn[i]->Down||n<=0) continue;
        h=(Disp->Height-tmargin-bmargin)/n;
        p2.y+=h;
        GraphG[i]->SetPos(p1,p2);
        p1.y+=h;
    }
    p1.y=tmargin; p2.y=p1.y;
    for (i=n=0;i<2;i++) if (btn[i]->Down) n++;
    for (i=0;i<2;i++) {
        if (!btn[i]->Down||n<=0) continue;
        h=(Disp->Height-tmargin-bmargin)/n;
        p2.y+=h;
        GraphE[i]->SetPos(p1,p2);
        p1.y+=h;
    }
}
// update colors on plot ----------------------------------------------------
void __fastcall TPlot::UpdateColor(void)
{
    int i;
    
    trace(3,"UpdateColor\n");
    
    for (i=0;i<3;i++) {
        GraphT   ->Color[i]=CColor[i];
        GraphR   ->Color[i]=CColor[i];
        GraphS   ->Color[i]=CColor[i];
        GraphG[0]->Color[i]=CColor[i];
        GraphG[1]->Color[i]=CColor[i];
        GraphG[2]->Color[i]=CColor[i];
    }
    Disp->Font->Assign(Font);
}
// update time-cursor -------------------------------------------------------
void __fastcall TPlot::UpdateTime(void)
{
    gtime_t time;
    sol_t *sol;
    double tt;
    int i,j,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"UpdateTime\n");
    
    // time-cursor change on solution-plot
    if (PlotType<=PLOT_NSAT||PlotType<=PLOT_RES) {
        TimeScroll->Max=MAX(1,SolData[sel].n-1);
        TimeScroll->Position=SolIndex[sel];
        if (!(sol=getsol(SolData+sel,SolIndex[sel]))) return;
        time=sol->time;
    }
    else if (NObs>0) { // time-cursor change on observation-data-plot
        TimeScroll->Max=MAX(1,NObs-1);
        TimeScroll->Position=ObsIndex;
        time=Obs.data[IndexObs[ObsIndex]].time;
    }
    else return;
    
    // time-synchronization between solutions and observation-data
    for (sel=0;sel<2;sel++) {
       i=SolIndex[sel];
       if (!(sol=getsol(SolData+sel,i))) continue;
       tt=timediff(sol->time,time);
       if (tt<-DTTOL) {
           for (;i<SolData[sel].n;i++) {
               if (!(sol=getsol(SolData+sel,i))) continue;
               if (timediff(sol->time,time)>=-DTTOL) break;
           }
       }
       else if (tt>DTTOL) {
           for (;i>=0;i--) {
               if (!(sol=getsol(SolData+sel,i))) continue;
               if (timediff(sol->time,time)<=DTTOL) break;
           }
       }
       SolIndex[sel]=MAX(0,MIN(SolData[sel].n-1,i));
    }
    i=ObsIndex;
    if (i<=NObs-1) {
        tt=timediff(Obs.data[IndexObs[i]].time,time);
        if (tt<-DTTOL) {
            for (;i<NObs;i++) {
                if (timediff(Obs.data[IndexObs[i]].time,time)>=-DTTOL) break;
            }
        }
        else if (tt>DTTOL) {
            for (;i>=0;i--) {
                if (timediff(Obs.data[IndexObs[i]].time,time)<=DTTOL) break;
            }
        }
        ObsIndex=MAX(0,MIN(NObs-1,i));
    }
}
// update origin of plot ----------------------------------------------------
void __fastcall TPlot::UpdateOrigin(void)
{
    gtime_t time={0};
    sol_t *sol;
    double opos[3]={0},pos[3],ovel[3]={0};
    int i,j,n=0,sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    char file[1024],sta[16]="",*p;
    
    trace(3,"UpdateOrigin\n");
    
    if (Origin==ORG_STARTPOS) {
        if (!(sol=getsol(SolData,0))||sol->type!=0) return;
        for (i=0;i<3;i++) opos[i]=sol->rr[i];
    }
    else if (Origin==ORG_ENDPOS) {
        if (!(sol=getsol(SolData,SolData[0].n-1))||sol->type!=0) return;
        for (i=0;i<3;i++) opos[i]=sol->rr[i];
    }
    else if (Origin==ORG_AVEPOS) {
        for (i=0;sol=getsol(SolData,i);i++) {
            if (sol->type!=0) continue;
            for (j=0;j<3;j++) opos[j]+=sol->rr[j];
            n++;
        }
        if (n>0) for (i=0;i<3;i++) opos[i]/=n;
    }
    else if (Origin==ORG_FITPOS) {
        if (!FitPos(&time,opos,ovel)) return;
    }
    else if (Origin==ORG_REFPOS) {
        if (norm(SolData[0].rb,3)>0.0) {
            for (i=0;i<3;i++) opos[i]=SolData[0].rb[i];
        }
        else {
            if (!(sol=getsol(SolData,0))||sol->type!=0) return;
            for (i=0;i<3;i++) opos[i]=sol->rr[i];
        }
    }
    else if (Origin==ORG_LLHPOS) {
        pos2ecef(OOPos,opos);
    }
    else if (Origin==ORG_AUTOPOS) {
        if (SolFiles[sel]->Count>0) {
            
            strcpy(file,U2A(SolFiles[sel]->Strings[0]).c_str());
            
            if ((p=strrchr(file,'\\'))) strncpy(sta,p+1,4);
            else strncpy(sta,file,4);
            for (p=sta;*p;p++) *p=(char)toupper(*p);
            
            strcpy(file,U2A(RefDialog->StaPosFile).c_str());
            
            ReadStaPos(file,sta,opos);
        }
    }
    else if (Origin==ORG_IMGPOS) {
        pos[0]=MapLat*D2R;
        pos[1]=MapLon*D2R;
        pos[2]=0.0;
        pos2ecef(pos,opos);
    }
    else if (Origin==ORG_MAPPOS) {
        pos[0]=(Gis.bound[0]+Gis.bound[1])/2.0;
        pos[1]=(Gis.bound[2]+Gis.bound[3])/2.0;
        pos[2]=0.0;
        pos2ecef(pos,opos);
    }
    else if (Origin-ORG_PNTPOS<MAXWAYPNT) {
        for (i=0;i<3;i++) opos[i]=PntPos[Origin-ORG_PNTPOS][i];
    }
    if (norm(opos,3)<=0.0) {
        // default start position
        if (!(sol=getsol(SolData,0))||sol->type!=0) return;
        for (i=0;i<3;i++) opos[i]=sol->rr[i];
    }
    OEpoch=time;
    for (i=0;i<3;i++) {
        OPos[i]=opos[i];
        OVel[i]=ovel[i];
    }
    Refresh_GEView();
}
// update satellite mask ----------------------------------------------------
void __fastcall TPlot::UpdateSatMask(void)
{
    int sat,prn;
    char buff[256],*p;
    
    trace(3,"UpdateSatMask\n");
    
    for (sat=1;sat<=MAXSAT;sat++) SatMask[sat-1]=0;
    for (sat=1;sat<=MAXSAT;sat++) {
        if (!(satsys(sat,&prn)&NavSys)) SatMask[sat-1]=1;
    }
    if (ExSats!="") {
        strcpy(buff,ExSats.c_str());
        
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if (*p=='+'&&(sat=satid2no(p+1))) SatMask[sat-1]=0; // included
            else if ((sat=satid2no(p)))       SatMask[sat-1]=1; // excluded
        }
    }
}
// update satellite select ---------------------------------------------------
void __fastcall TPlot::UpdateSatSel(void)
{
    AnsiString SatListText=SatList->Text;
    char id[16];
    int i,sys=0;
    
    if      (SatListText=="G") sys=SYS_GPS;
    else if (SatListText=="R") sys=SYS_GLO;
    else if (SatListText=="E") sys=SYS_GAL;
    else if (SatListText=="J") sys=SYS_QZS;
    else if (SatListText=="C") sys=SYS_CMP;
    else if (SatListText=="S") sys=SYS_SBS;
    for (i=0;i<MAXSAT;i++) {
        satno2id(i+1,id);
        SatSel[i]=SatListText=="ALL"||SatListText==id||satsys(i+1,NULL)==sys;
    }
}
// update enable/disable of widgets -----------------------------------------
void __fastcall TPlot::UpdateEnable(void)
{
    AnsiString s;
    double range;
    int i,data=BtnSol1->Down||BtnSol2->Down||BtnSol12->Down;
    int plot=PLOT_SOLP<=PlotType&&PlotType<=PLOT_NSAT;
    int sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"UpdateEnable\n");
    
    Panel1         ->Visible=MenuToolBar  ->Checked;
    Panel2         ->Visible=MenuStatusBar->Checked;
    
    BtnConnect     ->Down   = ConnectState;
    BtnSol2        ->Enabled=PlotType<=PLOT_NSAT||PlotType==PLOT_RES;
    BtnSol12       ->Enabled=!ConnectState&&PlotType<=PLOT_SOLA&&SolData[0].n>0&&SolData[1].n>0;
    
    QFlag          ->Visible=PlotType==PLOT_TRK ||PlotType==PLOT_SOLP||
                             PlotType==PLOT_SOLV||PlotType==PLOT_SOLA||
                             PlotType==PLOT_NSAT;
    ObsType        ->Visible=PlotType==PLOT_OBS||PlotType==PLOT_SKY;
    ObsType2       ->Visible=PlotType==PLOT_SNR||PlotType==PLOT_SNRE||PlotType==PLOT_MPS;
    FrqType        ->Visible=PlotType==PLOT_RES;
    DopType        ->Visible=PlotType==PLOT_DOP;
    SatList        ->Visible=PlotType==PLOT_RES||PlotType>=PLOT_OBS||
                             PlotType==PLOT_SKY||PlotType==PLOT_DOP||
                             PlotType==PLOT_SNR||PlotType==PLOT_SNRE||
                             PlotType==PLOT_MPS;
    QFlag          ->Enabled=data;
    ObsType        ->Enabled=data&&!SimObs;
    ObsType2       ->Enabled=data&&!SimObs;
    
    BtnOn1         ->Enabled=plot||PlotType==PLOT_SNR||PlotType==PLOT_RES||PlotType==PLOT_SNRE;
    BtnOn2         ->Enabled=plot||PlotType==PLOT_SNR||PlotType==PLOT_RES||PlotType==PLOT_SNRE;
    BtnOn3         ->Enabled=plot||PlotType==PLOT_SNR||PlotType==PLOT_RES;
    
    BtnRangeList   ->Left=23;
    BtnCenterOri   ->Visible=PlotType==PLOT_TRK ||PlotType==PLOT_SOLP||
                             PlotType==PLOT_SOLV||PlotType==PLOT_SOLA||
                             PlotType==PLOT_NSAT;
    BtnRangeList   ->Visible=PlotType==PLOT_TRK ||PlotType==PLOT_SOLP||
                             PlotType==PLOT_SOLV||PlotType==PLOT_SOLA||
                             PlotType==PLOT_NSAT;
    BtnCenterOri   ->Enabled=PlotType!=PLOT_NSAT;
    BtnRangeList   ->Enabled=PlotType!=PLOT_NSAT;
    
    Panel102       ->Left=156;
    Panel103       ->Left=170;
    Panel104       ->Left=220;
    BtnFitHoriz    ->Left=250;
    BtnFitVert     ->Left=275;
    BtnShowTrack   ->Left=300;
    BtnFixCent     ->Left=325;
    BtnFixHoriz    ->Left=350;
    BtnFixVert     ->Left=375;
    BtnShowGrid    ->Left=400;
    BtnShowSkyplot ->Left=425;
    BtnShowMap     ->Left=450;
    BtnShowImg     ->Left=475;
    BtnGE          ->Left=500;
    BtnGM          ->Left=525;
    
    Panel102       ->Visible=PlotType==PLOT_SOLP||PlotType==PLOT_SOLV||
                             PlotType==PLOT_SOLA||PlotType==PLOT_NSAT||
                             PlotType==PLOT_RES ||
                             PlotType==PLOT_SNR ||PlotType==PLOT_SNRE;
    BtnFitHoriz    ->Visible=PlotType==PLOT_SOLP||PlotType==PLOT_SOLV||
                             PlotType==PLOT_SOLA||PlotType==PLOT_NSAT||
                             PlotType==PLOT_RES ||PlotType==PLOT_OBS ||
                             PlotType==PLOT_DOP ||PlotType==PLOT_SNR ||
                             PlotType==PLOT_SNRE;
    BtnFitHoriz    ->Enabled=data;
    BtnFitVert     ->Visible=PlotType==PLOT_TRK ||PlotType==PLOT_SOLP||
                             PlotType==PLOT_SOLV||PlotType==PLOT_SOLA;
    BtnFitVert     ->Enabled=data;
    
    BtnShowTrack   ->Enabled=data;
    
    BtnFixCent     ->Visible=PlotType==PLOT_TRK;
    BtnFixCent     ->Enabled=data;
    BtnFixHoriz    ->Visible=PlotType==PLOT_SOLP||PlotType==PLOT_SOLV||
                             PlotType==PLOT_SOLA||PlotType==PLOT_NSAT||
                             PlotType==PLOT_RES ||PlotType==PLOT_OBS ||
                             PlotType==PLOT_DOP ||PlotType==PLOT_RES ||
                             PlotType==PLOT_SNR;
    BtnFixHoriz    ->Enabled=data;
    BtnFixVert     ->Visible=PlotType==PLOT_SOLP||PlotType==PLOT_SOLV||
                             PlotType==PLOT_SOLA;
    BtnFixVert     ->Enabled=data;
    BtnShowGrid    ->Visible=PlotType==PLOT_TRK;
    BtnShowSkyplot ->Visible=PlotType==PLOT_SKY||PlotType==PLOT_MPS;
    BtnShowMap     ->Visible=PlotType==PLOT_TRK;
    BtnShowMap     ->Enabled=!BtnSol12->Down;
    BtnShowImg     ->Visible=PlotType==PLOT_TRK||PlotType==PLOT_SKY||
                             PlotType==PLOT_MPS;
    BtnAnimate     ->Visible=data&&BtnShowTrack->Down;
    BtnGE          ->Visible=PlotType==PLOT_TRK;
    BtnGM          ->Visible=PlotType==PLOT_TRK;
    TimeScroll     ->Visible=data&&BtnShowTrack->Down;
    
    if (!BtnShowTrack->Down) {
        BtnFixHoriz->Enabled=false;
        BtnFixVert ->Enabled=false;
        BtnFixCent ->Enabled=false;
        BtnAnimate ->Down   =false;
    }
    MenuMapImg     ->Enabled=MapImage->Height>0;
    MenuSkyImg     ->Enabled=SkyImageI->Height>0;
    MenuSrcSol     ->Enabled=SolFiles[sel]->Count>0;
    MenuSrcObs     ->Enabled=ObsFiles->Count>0;
    MenuQcObs      ->Enabled=ObsFiles->Count>0;
    int n=0;
    for (i=0;i<MAXMAPLAYER;i++) {
        if (Gis.data[i]) n++;
    }
    MenuMapLayer   ->Enabled=n>0;
    
    MenuShowTrack  ->Enabled=BtnShowTrack->Enabled;
    MenuFitHoriz   ->Enabled=BtnFitHoriz ->Enabled;
    MenuFitVert    ->Enabled=BtnFitVert  ->Enabled;
    MenuCenterOri  ->Enabled=BtnCenterOri->Enabled;
    MenuFixCent    ->Enabled=BtnFixCent  ->Enabled;
    MenuFixHoriz   ->Enabled=BtnFixHoriz ->Enabled;
    MenuFixVert    ->Enabled=BtnFixVert  ->Enabled;
    MenuShowMap    ->Enabled=BtnShowMap  ->Enabled;
    MenuShowImg    ->Enabled=BtnShowImg  ->Enabled;
    MenuShowSkyplot->Enabled=BtnShowSkyplot->Visible;
    MenuShowGrid   ->Enabled=BtnShowGrid ->Visible;
    MenuGE         ->Enabled=BtnGE       ->Enabled;
    MenuGM         ->Enabled=BtnGM       ->Enabled;
    
    MenuShowTrack  ->Checked=BtnShowTrack->Down;
    MenuFixCent    ->Checked=BtnFixCent  ->Down;
    MenuFixHoriz   ->Checked=BtnFixHoriz ->Down;
    MenuFixVert    ->Checked=BtnFixVert  ->Down;
    MenuShowSkyplot->Checked=BtnShowSkyplot->Down;
    MenuShowGrid   ->Checked=BtnShowGrid ->Down;
    MenuShowMap    ->Checked=BtnShowMap  ->Down;
    MenuShowImg    ->Checked=BtnShowImg  ->Down;
    
    MenuAnimStart  ->Enabled=!ConnectState&&BtnAnimate->Enabled&&!BtnAnimate->Down;
    MenuAnimStop   ->Enabled=!ConnectState&&BtnAnimate->Enabled&& BtnAnimate->Down;
    TimeScroll     ->Enabled=data&&BtnShowTrack->Down;
    
    MenuOpenSol1   ->Enabled=!ConnectState;
    MenuOpenSol2   ->Enabled=!ConnectState;
    MenuConnect    ->Enabled=!ConnectState;
    MenuDisconnect ->Enabled= ConnectState;
    MenuPort       ->Enabled=!ConnectState;
    MenuOpenObs    ->Enabled=!ConnectState;
    MenuOpenNav    ->Enabled=!ConnectState;
    MenuOpenElevMask->Enabled=!ConnectState;
    MenuReload     ->Enabled=!ConnectState;
    
    BtnReload      ->Visible=!ConnectState;
    StrStatus1     ->Visible= ConnectState;
    StrStatus2     ->Visible= ConnectState;
    Panel12        ->Visible=!ConnectState;
}
// linear-fitting of positions ----------------------------------------------
int __fastcall TPlot::FitPos(gtime_t *time, double *opos, double *ovel)
{
    sol_t *sol;
    int i,j;
    double t,x[2],Ay[3][2]={{0}},AA[3][4]={{0}};
    
    trace(3,"FitPos\n");
    
    if (SolData[0].n<=0) return 0;
    
    for (i=0;sol=getsol(SolData,i);i++) {
        if (sol->type!=0) continue;
        if (time->time==0) *time=sol->time;
        t=timediff(sol->time,*time);
        
        for (j=0;j<3;j++) {
            Ay[j][0]+=sol->rr[j];
            Ay[j][1]+=sol->rr[j]*t;
            AA[j][0]+=1.0;
            AA[j][1]+=t;
            AA[j][2]+=t;
            AA[j][3]+=t*t;
        }
    }
    for (i=0;i<3;i++) {
        if (solve("N",AA[i],Ay[i],2,1,x)) return 0;
        opos[i]=x[0];
        ovel[i]=x[1];
    }
    return 1;
}
// fit time-range of plot ---------------------------------------------------
void __fastcall TPlot::FitTime(void)
{
    sol_t *sols,*sole;
    double tl[2]={86400.0*7,0.0},tp[2],xl[2],yl[2],zl[2];
    int sel=!BtnSol1->Down&&BtnSol2->Down?1:0;
    
    trace(3,"FitTime\n");
    
    sols=getsol(SolData+sel,0);
    sole=getsol(SolData+sel,SolData[sel].n-1);
    if (sols&&sole) {
        tl[0]=MIN(tl[0],TimePos(sols->time));
        tl[1]=MAX(tl[1],TimePos(sole->time));
    }
    if (Obs.n>0) {
        tl[0]=MIN(tl[0],TimePos(Obs.data[0].time));
        tl[1]=MAX(tl[1],TimePos(Obs.data[Obs.n-1].time));
    }
    if (TimeEna[0]) tl[0]=TimePos(TimeStart);
    if (TimeEna[1]) tl[1]=TimePos(TimeEnd  );
    
    if (tl[0]==tl[1]) {
        tl[0]=tl[0]-DEFTSPAN/2.0;
        tl[1]=tl[0]+DEFTSPAN/2.0;
    }
    else if (tl[0]>tl[1]) {
        tl[0]=-DEFTSPAN/2.0;
        tl[1]= DEFTSPAN/2.0;
    }
    GraphG[0]->GetLim(tp,xl);
    GraphG[1]->GetLim(tp,yl);
    GraphG[2]->GetLim(tp,zl);
    GraphG[0]->SetLim(tl,xl);
    GraphG[1]->SetLim(tl,yl);
    GraphG[2]->SetLim(tl,zl);
    GraphR   ->GetLim(tp,xl);
    GraphR   ->SetLim(tl,xl);
}
// set x/y-range of plot ----------------------------------------------------
void __fastcall TPlot::SetRange(int all, double range)
{
    double xl[]={-range,range};
    double yl[]={-range,range};
    double zl[]={-range,range};
    double xs,ys,tl[2],xp[2],pos[3];
    int w,h;
    
    trace(3,"SetRange: all=%d range=%.3f\n",all,range);
    
    if (all||PlotType==PLOT_TRK) {
        GraphT->SetLim(xl,yl);
        GraphT->GetScale(xs,ys);
        GraphT->SetScale(MAX(xs,ys),MAX(xs,ys));
        if (norm(OPos,3)>0.0) {
            ecef2pos(OPos,pos);
            GoogleEarthView->SetView(pos[0]*R2D,pos[1]*R2D,0.0,0.0);
            GoogleMapView->SetView(pos[0]*R2D,pos[1]*R2D,13);
        }
    }
    if (PLOT_SOLP<=PlotType&&PlotType<=PLOT_SOLA) {
        GraphG[0]->GetLim(tl,xp);
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
    else if (PlotType==PLOT_NSAT) {
        GraphG[0]->GetLim(tl,xp);
        xl[0]=yl[0]=zl[0]=0.0;
        xl[1]=MaxDop;
        yl[1]=YLIM_AGE;
        zl[1]=YLIM_RATIO;
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
    else if (PlotType<PLOT_SNR) {
        GraphG[0]->GetLim(tl,xp);
        xl[0]=-10.0; xl[1]=10.0;
        yl[0]= -0.1; yl[1]= 0.1;
        zl[0]=  0.0; zl[1]=90.0;
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
    else {
        GraphG[0]->GetLim(tl,xp);
        xl[0]=10.0; xl[1]= 60.0;
        yl[0]=-MaxMP; yl[1]=MaxMP;
        zl[0]= 0.0; zl[1]= 90.0;
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
}
// fit x/y-range of plot ----------------------------------------------------
void __fastcall TPlot::FitRange(int all)
{
    TIMEPOS *pos,*pos1,*pos2;
    sol_t *data;
    double xs,ys,xp[2],tl[2],xl[]={1E8,-1E8},yl[2]={1E8,-1E8},zl[2]={1E8,-1E8};
    double lat,lon,lats[2]={90,-90},lons[2]={180,-180},llh[3];
    int i,j,n,w,h,type=PlotType-PLOT_SOLP;
    
    trace(3,"FitRange: all=%d\n",all);
    
    BtnFixHoriz->Down=false;
    MenuFixHoriz->Checked=false;
    
    if (BtnSol1->Down) {
        
        pos=SolToPos(SolData,-1,QFlag->ItemIndex,type);
        
        for (i=0;i<pos->n;i++) {
            xl[0]=MIN(xl[0],pos->x[i]);
            yl[0]=MIN(yl[0],pos->y[i]);
            zl[0]=MIN(zl[0],pos->z[i]);
            xl[1]=MAX(xl[1],pos->x[i]);
            yl[1]=MAX(yl[1],pos->y[i]);
            zl[1]=MAX(zl[1],pos->z[i]);
        }
        delete pos;
    }
    if (BtnSol2->Down) {
        
        pos=SolToPos(SolData+1,-1,QFlag->ItemIndex,type);
        
        for (i=0;i<pos->n;i++) {
            xl[0]=MIN(xl[0],pos->x[i]);
            yl[0]=MIN(yl[0],pos->y[i]);
            zl[0]=MIN(zl[0],pos->z[i]);
            xl[1]=MAX(xl[1],pos->x[i]);
            yl[1]=MAX(yl[1],pos->y[i]);
            zl[1]=MAX(zl[1],pos->z[i]);
        }
        delete pos;
    }
    if (BtnSol12->Down) {
        
        pos1=SolToPos(SolData  ,-1,0,type);
        pos2=SolToPos(SolData+1,-1,0,type);
        pos=pos1->diff(pos2,QFlag->ItemIndex);
        
        for (i=0;i<pos->n;i++) {
            xl[0]=MIN(xl[0],pos->x[i]);
            yl[0]=MIN(yl[0],pos->y[i]);
            zl[0]=MIN(zl[0],pos->z[i]);
            xl[1]=MAX(xl[1],pos->x[i]);
            yl[1]=MAX(yl[1],pos->y[i]);
            zl[1]=MAX(zl[1],pos->z[i]);
        }
        delete pos1;
        delete pos2;
        delete pos;
    }
    xl[0]-=0.05;
    xl[1]+=0.05;
    yl[0]-=0.05;
    yl[1]+=0.05;
    zl[0]-=0.05;
    zl[1]+=0.05;
    
    if (all||PlotType==PLOT_TRK) {
        GraphT->SetLim(xl,yl);
        GraphT->GetScale(xs,ys);
        GraphT->SetScale(MAX(xs,ys),MAX(xs,ys));
    }
    if (all||PlotType<=PLOT_SOLA||PlotType==PLOT_RES) {
        GraphG[0]->GetLim(tl,xp);
        GraphG[0]->SetLim(tl,xl);
        GraphG[1]->SetLim(tl,yl);
        GraphG[2]->SetLim(tl,zl);
    }
    if (all) {
        if (BtnSol1->Down) {
            for (i=0;data=getsol(SolData,i);i++) {
                ecef2pos(data->rr,llh); 
                lats[0]=MIN(lats[0],llh[0]*R2D);
                lons[0]=MIN(lons[0],llh[1]*R2D);
                lats[1]=MAX(lats[1],llh[0]*R2D);
                lons[1]=MAX(lons[1],llh[1]*R2D);
            }
        }
        if (BtnSol2->Down) {
            for (i=0;data=getsol(SolData+1,i);i++) {
                ecef2pos(data->rr,llh); 
                lats[0]=MIN(lats[0],llh[0]*R2D);
                lons[0]=MIN(lons[0],llh[1]*R2D);
                lats[1]=MAX(lats[1],llh[0]*R2D);
                lons[1]=MAX(lons[1],llh[1]*R2D);
            }
        }
        if (lats[0]<=lats[1]&&lons[0]<=lons[1]) {
            lat=(lats[0]+lats[1])/2.0;
            lon=(lons[0]+lons[1])/2.0;
//            GoogleEarthView->SetView(lat,lon,0.0,0.0);
        }
    }
}
// set center of track plot -------------------------------------------------
void __fastcall TPlot::SetTrkCent(double lat, double lon)
{
    gtime_t time={0};
    double pos[3]={0},rr[3],xyz[3];
    
    if (PlotType!=PLOT_TRK) return;
    pos[0]=lat*D2R;
    pos[1]=lon*D2R;
    pos2ecef(pos,rr);
    PosToXyz(time,rr,0,xyz);
    GraphT->SetCent(xyz[0],xyz[1]);
    UpdatePlot();
}
// load options from ini-file -----------------------------------------------
void __fastcall TPlot::LoadOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s,s1;
    double range;
    char rangelist[64];
    int i,geopts[12];
    
    trace(3,"LoadOpt\n");
    
    PlotType     =ini->ReadInteger("plot","plottype",      0);
    TimeLabel    =ini->ReadInteger("plot","timelabel",     1);
    LatLonFmt    =ini->ReadInteger("plot","latlonfmt",     0);
    AutoScale    =ini->ReadInteger("plot","autoscale",     1);
    ShowStats    =ini->ReadInteger("plot","showstats",     0);
    ShowLabel    =ini->ReadInteger("plot","showlabel",     1);
    ShowGLabel   =ini->ReadInteger("plot","showglabel",    1);
    ShowCompass  =ini->ReadInteger("plot","showcompass",   0);
    ShowScale    =ini->ReadInteger("plot","showscale",     1);
    ShowArrow    =ini->ReadInteger("plot","showarrow",     0);
    ShowSlip     =ini->ReadInteger("plot","showslip",      0);
    ShowHalfC    =ini->ReadInteger("plot","showhalfc",     0);
    ShowErr      =ini->ReadInteger("plot","showerr",       0);
    ShowEph      =ini->ReadInteger("plot","showeph",       0);
    PlotStyle    =ini->ReadInteger("plot","plotstyle",     0);
    MarkSize     =ini->ReadInteger("plot","marksize",      2);
    NavSys       =ini->ReadInteger("plot","navsys",  SYS_GPS);
    AnimCycle    =ini->ReadInteger("plot","animcycle",    10);
    RefCycle     =ini->ReadInteger("plot","refcycle",    100);
    HideLowSat   =ini->ReadInteger("plot","hidelowsat",    0);
    ElMaskP      =ini->ReadInteger("plot","elmaskp",       0);
    ExSats       =ini->ReadString ("plot","exsats",       "");
    RtBuffSize   =ini->ReadInteger("plot","rtbuffsize",10800);
    TimeSyncOut  =ini->ReadInteger("plot","timesyncout",   0);
    TimeSyncPort =ini->ReadInteger("plot","timesyncport",10071);
    RtStream[0]  =ini->ReadInteger("plot","rtstream1",     0);
    RtStream[1]  =ini->ReadInteger("plot","rtstream2",     0);
    RtFormat[0]  =ini->ReadInteger("plot","rtformat1",     0);
    RtFormat[1]  =ini->ReadInteger("plot","rtformat2",     0);
    RtTimeForm   =ini->ReadInteger("plot","rttimeform",    0);
    RtDegForm    =ini->ReadInteger("plot","rtdegform",     0);
    RtFieldSep   =ini->ReadString ("plot","rtfieldsep",   "");
    RtTimeOutTime=ini->ReadInteger("plot","rttimeouttime", 0);
    RtReConnTime =ini->ReadInteger("plot","rtreconntime",10000);
    
    MColor[0][0]=(TColor)ini->ReadInteger("plot","mcolor0", (int)clSilver );
    MColor[0][1]=(TColor)ini->ReadInteger("plot","mcolor1", (int)clGreen  );
    MColor[0][2]=(TColor)ini->ReadInteger("plot","mcolor2",      0x00AAFF );
    MColor[0][3]=(TColor)ini->ReadInteger("plot","mcolor3", (int)clFuchsia);
    MColor[0][4]=(TColor)ini->ReadInteger("plot","mcolor4", (int)clBlue   );
    MColor[0][5]=(TColor)ini->ReadInteger("plot","mcolor5", (int)clRed    );
    MColor[0][6]=(TColor)ini->ReadInteger("plot","mcolor6", (int)clTeal   );
    MColor[0][7]=(TColor)ini->ReadInteger("plot","mcolor7", (int)clGray   );
    MColor[1][0]=(TColor)ini->ReadInteger("plot","mcolor8", (int)clSilver );
    MColor[1][1]=(TColor)ini->ReadInteger("plot","mcolor9",      0x804000 );
    MColor[1][2]=(TColor)ini->ReadInteger("plot","mcolor10",     0x008080 );
    MColor[1][3]=(TColor)ini->ReadInteger("plot","mcolor11",     0xFF0080 );
    MColor[1][4]=(TColor)ini->ReadInteger("plot","mcolor12",     0xFF8000 );
    MColor[1][5]=(TColor)ini->ReadInteger("plot","mcolor13",     0x8080FF );
    MColor[1][6]=(TColor)ini->ReadInteger("plot","mcolor14",     0xFF8080 );
    MColor[1][7]=(TColor)ini->ReadInteger("plot","mcolor15",(int)clGray   );
    MapColor[0]=(TColor)ini->ReadInteger("plot","mapcolor1",     clSilver );
    MapColor[1]=(TColor)ini->ReadInteger("plot","mapcolor2",     clSilver );
    MapColor[2]=(TColor)ini->ReadInteger("plot","mapcolor3",     0xF0D0D0 );
    MapColor[3]=(TColor)ini->ReadInteger("plot","mapcolor4",     0xD0F0D0 );
    MapColor[4]=(TColor)ini->ReadInteger("plot","mapcolor5",     0xD0D0F0 );
    MapColor[5]=(TColor)ini->ReadInteger("plot","mapcolor6",     0xD0F0F0 );
    MapColor[6]=(TColor)ini->ReadInteger("plot","mapcolor7",     0xF8F8D0 );
    MapColor[7]=(TColor)ini->ReadInteger("plot","mapcolor8",     0xF0F0F0 );
    MapColor[8]=(TColor)ini->ReadInteger("plot","mapcolor9",     0xF0F0F0 );
    MapColor[9]=(TColor)ini->ReadInteger("plot","mapcolor10",    0xF0F0F0 );
    MapColor[10]=(TColor)ini->ReadInteger("plot","mapcolor11",   0xF0F0F0 );
    MapColor[11]=(TColor)ini->ReadInteger("plot","mapcolor12",   0xF0F0F0 );
    CColor[0]=(TColor)ini->ReadInteger("plot","color1", (int)clWhite  );
    CColor[1]=(TColor)ini->ReadInteger("plot","color2", (int)clSilver );
    CColor[2]=(TColor)ini->ReadInteger("plot","color3", (int)clBlack  );
    CColor[3]=(TColor)ini->ReadInteger("plot","color4", (int)clSilver );
    
    RefDialog->StaPosFile=ini->ReadString ("plot","staposfile","");
    RefDialog->Format    =ini->ReadInteger("plot","staposformat",0);
    
    ElMask    =ini->ReadFloat  ("plot","elmask", 0.0);
    MaxDop    =ini->ReadFloat  ("plot","maxdop",30.0);
    MaxMP     =ini->ReadFloat  ("plot","maxmp" ,10.0);
    YRange    =ini->ReadFloat  ("plot","yrange", 5.0);
    Origin    =ini->ReadInteger("plot","orgin",    2);
    RcvPos    =ini->ReadInteger("plot","rcvpos",   0);
    OOPos[0]  =ini->ReadFloat  ("plot","oopos1",   0);
    OOPos[1]  =ini->ReadFloat  ("plot","oopos2",   0);
    OOPos[2]  =ini->ReadFloat  ("plot","oopos3",   0);
    QcCmd     =ini->ReadString ("plot","qccmd","teqc +qc +sym +l -rep -plot");
    TLEFile   =ini->ReadString ("plot","tlefile", "");
    TLESatFile=ini->ReadString ("plot","tlesatfile","");
    
    Font->Charset=ANSI_CHARSET;
    Font->Name=ini->ReadString ("plot","fontname","Tahoma");
    Font->Size=ini->ReadInteger("plot","fontsize",8);
    
    RnxOpts   =ini->ReadString ("plot","rnxopts","");
    ApiKey    =ini->ReadString ("plot","apikey" ,"");
    
    for (i=0;i<11;i++) {
        geopts[i]=ini->ReadInteger("ge",s.sprintf("geopts_%d",i),0);
    }
    GoogleEarthView->SetOpts(geopts);
    
    for (i=0;i<2;i++) {
        StrCmds  [0][i]=ini->ReadString ("str",s.sprintf("strcmd1_%d",    i),"");
        StrCmds  [1][i]=ini->ReadString ("str",s.sprintf("strcmd2_%d",    i),"");
        StrCmdEna[0][i]=ini->ReadInteger("str",s.sprintf("strcmdena1_%d", i), 0);
        StrCmdEna[1][i]=ini->ReadInteger("str",s.sprintf("strcmdena2_%d", i), 0);
    }
    for (i=0;i<3;i++) {
        StrPaths[0][i]=ini->ReadString ("str",s.sprintf("strpath1_%d",   i),"");
        StrPaths[1][i]=ini->ReadString ("str",s.sprintf("strpath2_%d",   i),"");
    }
    for (i=0;i<10;i++) {
        StrHistory [i]=ini->ReadString ("str",s.sprintf("strhistry_%d",  i),"");
        StrMntpHist[i]=ini->ReadString ("str",s.sprintf("strmntphist_%d",i),"");
    }
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    
    MenuBrowse->Checked=ini->ReadInteger("solbrows","show",       0);
    PanelBrowse->Width =ini->ReadInteger("solbrows","split1",   100);
    DirSel->Height     =ini->ReadInteger("solbrows","split2",   150);
    DirSel->Directory  =ini->ReadString ("solbrows","dir",  "C:\\");
    
    delete ini;
    
    for (i=0;i<RangeList->Count;i++) {
        
        strcpy(rangelist,U2A(RangeList->Items->Strings[i]).c_str());
        
        if (sscanf(rangelist,"%lf",&range)&&range==YRange) {
            RangeList->Selected[i]=true;
        }
    }
}
// save options to ini-file -------------------------------------------------
void __fastcall TPlot::SaveOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s,s1;
    int i,geopts[12];
    
    trace(3,"SaveOpt\n");
    
    ini->WriteInteger("plot","plottype",     PlotType     );
    ini->WriteInteger("plot","timelabel",    TimeLabel    );
    ini->WriteInteger("plot","latlonfmt",    LatLonFmt    );
    ini->WriteInteger("plot","autoscale",    AutoScale    );
    ini->WriteInteger("plot","showstats",    ShowStats    );
    ini->WriteInteger("plot","showlabel",    ShowLabel    );
    ini->WriteInteger("plot","showglabel",   ShowGLabel   );
    ini->WriteInteger("plot","showcompass",  ShowCompass  );
    ini->WriteInteger("plot","showscale",    ShowScale    );
    ini->WriteInteger("plot","showarrow",    ShowArrow    );
    ini->WriteInteger("plot","showslip",     ShowSlip     );
    ini->WriteInteger("plot","showhalfc",    ShowHalfC    );
    ini->WriteInteger("plot","showerr",      ShowErr      );
    ini->WriteInteger("plot","showeph",      ShowEph      );
    ini->WriteInteger("plot","plotstyle",    PlotStyle    );
    ini->WriteInteger("plot","marksize",     MarkSize     );
    ini->WriteInteger("plot","navsys",       NavSys       );
    ini->WriteInteger("plot","animcycle",    AnimCycle    );
    ini->WriteInteger("plot","refcycle",     RefCycle     );
    ini->WriteInteger("plot","hidelowsat",   HideLowSat   );
    ini->WriteInteger("plot","elmaskp",      ElMaskP      );
    ini->WriteString ("plot","exsats",       ExSats       );
    ini->WriteInteger("plot","rtbuffsize",   RtBuffSize   );
    ini->WriteInteger("plot","timesyncout",  TimeSyncOut  );
    ini->WriteInteger("plot","timesyncport", TimeSyncPort );
    ini->WriteInteger("plot","rtstream1",    RtStream[0]  );
    ini->WriteInteger("plot","rtstream2",    RtStream[1]  );
    ini->WriteInteger("plot","rtformat1",    RtFormat[0]  );
    ini->WriteInteger("plot","rtformat2",    RtFormat[1]  );
    ini->WriteInteger("plot","rttimeform",   RtTimeForm   );
    ini->WriteInteger("plot","rtdegform",    RtDegForm    );
    ini->WriteString ("plot","rtfieldsep",   RtFieldSep   );
    ini->WriteInteger("plot","rttimeouttime",RtTimeOutTime);
    ini->WriteInteger("plot","rtreconntime", RtReConnTime );
    
    ini->WriteInteger("plot","mcolor0",     (int)MColor[0][0]);
    ini->WriteInteger("plot","mcolor1",     (int)MColor[0][1]);
    ini->WriteInteger("plot","mcolor2",     (int)MColor[0][2]);
    ini->WriteInteger("plot","mcolor3",     (int)MColor[0][3]);
    ini->WriteInteger("plot","mcolor4",     (int)MColor[0][4]);
    ini->WriteInteger("plot","mcolor5",     (int)MColor[0][5]);
    ini->WriteInteger("plot","mcolor6",     (int)MColor[0][6]);
    ini->WriteInteger("plot","mcolor7",     (int)MColor[0][7]);
    ini->WriteInteger("plot","mcolor8",     (int)MColor[0][0]);
    ini->WriteInteger("plot","mcolor9",     (int)MColor[1][1]);
    ini->WriteInteger("plot","mcolor10",    (int)MColor[1][2]);
    ini->WriteInteger("plot","mcolor11",    (int)MColor[1][3]);
    ini->WriteInteger("plot","mcolor12",    (int)MColor[1][4]);
    ini->WriteInteger("plot","mcolor13",    (int)MColor[1][5]);
    ini->WriteInteger("plot","mcolor14",    (int)MColor[1][6]);
    ini->WriteInteger("plot","mcolor15",    (int)MColor[1][7]);
    ini->WriteInteger("plot","mapcolor1",   (int)MapColor [0]);
    ini->WriteInteger("plot","mapcolor2",   (int)MapColor [1]);
    ini->WriteInteger("plot","mapcolor3",   (int)MapColor [2]);
    ini->WriteInteger("plot","mapcolor4",   (int)MapColor [3]);
    ini->WriteInteger("plot","mapcolor5",   (int)MapColor [4]);
    ini->WriteInteger("plot","mapcolor6",   (int)MapColor [5]);
    ini->WriteInteger("plot","mapcolor7",   (int)MapColor [6]);
    ini->WriteInteger("plot","mapcolor8",   (int)MapColor [7]);
    ini->WriteInteger("plot","mapcolor9",   (int)MapColor [8]);
    ini->WriteInteger("plot","mapcolor10",  (int)MapColor [9]);
    ini->WriteInteger("plot","mapcolor11",  (int)MapColor[10]);
    ini->WriteInteger("plot","mapcolor12",  (int)MapColor[11]);
    ini->WriteInteger("plot","color1",      (int)CColor[0]);
    ini->WriteInteger("plot","color2",      (int)CColor[1]);
    ini->WriteInteger("plot","color3",      (int)CColor[2]);
    ini->WriteInteger("plot","color4",      (int)CColor[3]);
    
    ini->WriteString ("plot","staposfile",   RefDialog->StaPosFile);
    ini->WriteInteger("plot","staposformat", RefDialog->Format);
    
    ini->WriteFloat  ("plot","elmask",       ElMask        );
    ini->WriteFloat  ("plot","maxdop",       MaxDop        );
    ini->WriteFloat  ("plot","maxmp",        MaxMP         );
    ini->WriteFloat  ("plot","yrange",       YRange        );
    ini->WriteInteger("plot","orgin",        Origin        );
    ini->WriteInteger("plot","rcvpos",       RcvPos        );
    ini->WriteFloat  ("plot","oopos1",       OOPos[0]      );
    ini->WriteFloat  ("plot","oopos2",       OOPos[1]      );
    ini->WriteFloat  ("plot","oopos3",       OOPos[2]      );
    ini->WriteString ("plot","qccmd",        QcCmd         );
    ini->WriteString ("plot","tlefile",      TLEFile       );
    ini->WriteString ("plot","tlesatfile",   TLESatFile    );
    
    ini->WriteString ("plot","fontname",     Font->Name    );
    ini->WriteInteger("plot","fontsize",     Font->Size    );
    
    ini->WriteString ("plot","rnxopts",      RnxOpts       );
    ini->WriteString ("plot","apikey",       ApiKey        );
    
    GoogleEarthView->GetOpts(geopts);
    for (i=0;i<11;i++) {
        ini->WriteInteger("ge",s.sprintf("geopts_%d",i),geopts[i]);
    }
    for (i=0;i<2;i++) {
        ini->WriteString ("str",s.sprintf("strcmd1_%d",    i),StrCmds  [0][i]);
        ini->WriteString ("str",s.sprintf("strcmd2_%d",    i),StrCmds  [1][i]);
        ini->WriteInteger("str",s.sprintf("strcmdena1_%d", i),StrCmdEna[0][i]);
        ini->WriteInteger("str",s.sprintf("strcmdena2_%d", i),StrCmdEna[1][i]);
    }
    for (i=0;i<3;i++) {
        ini->WriteString ("str",s.sprintf("strpath1_%d",   i),StrPaths[0][i]);
        ini->WriteString ("str",s.sprintf("strpath2_%d",   i),StrPaths[1][i]);
    }
    for (i=0;i<12;i++) {
        ini->WriteString ("str",s.sprintf("strhistry_%d",  i),StrHistory [i]);
        ini->WriteString ("str",s.sprintf("strmntphist_%d",i),StrMntpHist[i]);
    }
    ini->WriteInteger("viewer","color1",(int)TTextViewer::Color1  );
    ini->WriteInteger("viewer","color2",(int)TTextViewer::Color2  );
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    
    ini->WriteInteger("solbrows","show", MenuBrowse->Checked);
    ini->WriteInteger("solbrows","split1",PanelBrowse->Width);
    ini->WriteInteger("solbrows","split2",    DirSel->Height);
    ini->WriteString ("solbrows","dir",    DirSel->Directory);
    
    delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TPlot::FileMaskChange(TObject *Sender)
{
	switch (FileMask->ItemIndex) {
		case 0 : FileList->Mask="*.pos" ; break;
		case 1 : FileList->Mask="*.nmea"; break;
		case 2 : FileList->Mask="*.stat"; break;
		default: FileList->Mask="*.*"   ; break;
	}
}
//---------------------------------------------------------------------------

void __fastcall TPlot::FileListClick(TObject *Sender)
{
	TStringList *file=new TStringList;
	file->Add(FileList->FileName);
	Plot->ReadSol(file,0);
	delete file;
}
//---------------------------------------------------------------------------

void __fastcall TPlot::Splitter1Moved(TObject *Sender)
{
    UpdateSize();
    Refresh();
}
//---------------------------------------------------------------------------

void __fastcall TPlot::BtnUdListClick(TObject *Sender)
{
    FileList->Update();
}
//---------------------------------------------------------------------------

