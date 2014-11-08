//---------------------------------------------------------------------------
// rtknavi : real-time positioning ap
//
//          Copyright (C) 2007-2014 by T.TAKASU, All rights reserved.
//
// options : rtknavi [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//           2010/07/18  1.1 rtklib 2.4.0
//           2010/08/16  1.2 fix bug on setting of satellite antenna model
//           2010/09/04  1.3 fix bug on setting of receiver antenna delta
//           2011/06/10  1.4 rtklib 2.4.1
//           2012/04/03  1.5 rtklib 2.4.2
//           2014/09/06  1.6 rtklib 2.4.3
//---------------------------------------------------------------------------
#include <vcl.h>
#include <inifiles.hpp>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>
#pragma hdrstop

#include "rtklib.h"
#include "instrdlg.h"
#include "outstrdlg.h"
#include "logstrdlg.h"
#include "mondlg.h"
#include "tcpoptdlg.h"
#include "confdlg.h"
#include "aboutdlg.h"
#include "viewer.h"
#include "naviopt.h"
#include "navimain.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define PRGNAME     "RTKNAVI"           // program name
#define TRACEFILE   "rtknavi_%Y%m%d%h%M.trace" // debug trace file
#define STATFILE    "rtknavi_%Y%m%d%h%M.stat"  // solution status file
#define CLORANGE    (TColor)0x00AAFF
#define CLLGRAY     (TColor)0xDDDDDD
#define CHARDEG     0x00B0              // character code of degree
#define SATSIZE     20                  // satellite circle size in skyplot
#define MINSNR      10                  // minimum snr
#define MAXSNR      60                  // maximum snr
#define KEYF6       0x75                // code of function key F6
#define KEYF7       0x76                // code of function key F7
#define KEYF8       0x77                // code of function key F8
#define KEYF9       0x78                // code of function key F9
#define KEYF10      0x79                // code of function key F10
#define POSFONTNAME "Palatino Linotype"
#define POSFONTSIZE 12
#define MINBLLEN    0.01                // minimum baseline length to show

#define KACYCLE     1000                // keep alive cycle (ms)
#define TIMEOUT     10000               // inactive timeout time (ms)
#define DEFAULTPORT 52001               // default monitor port number
#define MAXPORTOFF  9                   // max port number offset

#define SQRT(x)     ((x)<0.0?0.0:sqrt(x))
#define MIN(x,y)    ((x)<(y)?(x):(y))

//---------------------------------------------------------------------------

rtksvr_t rtksvr;                        // rtk server struct
stream_t monistr;                       // monitor stream

// show message in message area ---------------------------------------------
extern "C" {
extern int showmsg(char *format,...) {return 0;}
}
// convert degree to deg-min-sec --------------------------------------------
static void degtodms(double deg, double *dms)
{
    double sgn=1.0;
    if (deg<0.0) {deg=-deg; sgn=-1.0;}
    dms[0]=floor(deg);
    dms[1]=floor((deg-dms[0])*60.0);
    dms[2]=(deg-dms[0]-dms[1]/60.0)*3600;
    dms[0]*=sgn;
}
// execute command ----------------------------------------------------------
int __fastcall TMainForm::ExecCmd(AnsiString cmd, int show)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    if (!CreateProcess(NULL,p,NULL,NULL,false,show?0:CREATE_NO_WINDOW,NULL,
                       NULL,&si,&info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
// constructor --------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    SvrCycle=SvrBuffSize=0;
    SolBuffSize=1000;
    for (int i=0;i<8;i++) {
        StreamC[i]=Stream[i]=Format[i]=CmdEna[i][0]=CmdEna[i][1]=0;
    }
    TimeSys=SolType=PlotType1=PlotType2=FreqType1=FreqType2=0;
    PSol=PSolS=PSolE=Nsat[0]=Nsat[1]=0;
    NMapPnt=0;
    OpenPort=0;
    Time=NULL;
    SolStat=Nvsat=NULL;
    SolCurrentStat=0;
    SolRov=SolRef=Qr=VelRov=Age=Ratio=NULL;
    for (int i=0;i<2;i++) for (int j=0;j<MAXSAT;j++) {
        Sat[i][j]=Vsat[i][j]=0;
        Az[i][j]=El[i][j]=0.0;
        for (int k=0;k<NFREQ;k++) Snr[i][j][k]=0;
    }
    PrcOpt=prcopt_default;
    SolOpt=solopt_default;
    PosFont=new TFont;
    
    rtksvrinit(&rtksvr);
    strinit(&monistr);
    
    Caption=PRGNAME;
    Caption=Caption+" ver."+VER_RTKLIB;
    DoubleBuffered=true;
    
    TLEData.n=TLEData.nmax=0;
    TLEData.data=NULL;
    
    PanelStack=PanelMode=0;
}
// callback on form create --------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
    char *p,*argv[32],buff[1024],file[1024]="rtknavi.exe";
    int argc=0;
    
    trace(3,"FormCreate\n");
    
    ::GetModuleFileName(NULL,file,sizeof(file));
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".ini");
    IniFile=file;
    
    InitSolBuff();
    SetTrayIcon(1);
    strinitcom();
    
    strcpy(buff,GetCommandLine());
    
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
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-i")&&i+1<argc) IniFile=argv[++i];
    }
    LoadOpt();
    
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
    }
    LoadNav(&rtksvr.nav);
    
    OpenMoniPort(MoniPort);
}
// callback on form show ----------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    trace(3,"FormShow\n");
    
    if (TLEFileF!="") {
        tle_read(TLEFileF.c_str(),&TLEData);
    }
    if (TLESatFileF!="") {
        tle_name_read(TLESatFileF.c_str(),&TLEData);
    }
    UpdatePanel();
    UpdateTimeSys();
    UpdateSolType();
    UpdateFont();
    UpdatePos();
}
// callback on form close ---------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    trace(3,"FormClose\n");
    
    if (OpenPort>0) {
        // send disconnect message
        strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));
        
        strclose(&monistr);
    }
    SaveOpt();
    SaveNav(&rtksvr.nav);
}
// callback panel 21 resize --------------------------------------------------
void __fastcall TMainForm::Panel21Resize(TObject *Sender)
{
    trace(3,"Panel21Resize\n");
    
    BtnSolType->Left=Panel21->Width-BtnSolType->Width-2;
}
// callback panel 211 resize -------------------------------------------------
void __fastcall TMainForm::Panel211Resize(TObject *Sender)
{
    int h=abs(Pos1->Font->Height)-2,w=Panel211->Width;
    int y0,y1,y2,y3,y4,y5;
    
    trace(3,"Panel211Resize\n");
    
    y2=Panel211->Height/2-h/2-6; 
    y1=y2-h-10; y3=y2+h+10; y0=y1-h-12; y4=y3+h+10; y5=y4+14;
    
    PlabelA  ->Top=y0+2; Solution->Top=y0+5-h/2; IndSol->Top=y0+4;
    Plabel1  ->Top=y1; Pos1->Top=y1;
    Plabel2  ->Top=y2; Pos2->Top=y2;
    Plabel3  ->Top=y3; Pos3->Top=y3;
    LabelStd ->Top=y4;
    LabelNSat->Top=y5;
    Solution->Left=40; Solution->Width=w-75; IndSol->Left=w-24;
    Plabel1->Left=14; Pos1->Left=30; Pos1->Width=w-42;
    Plabel2->Left=14; Pos2->Left=30; Pos2->Width=w-42;
    Plabel3->Left=14; Pos3->Left=30; Pos3->Width=w-42;
    LabelStd ->Left=2; LabelStd ->Width=w-4;
    LabelNSat->Left=2; LabelNSat->Width=w-4;
}
// callback panel 221 resize -------------------------------------------------
void __fastcall TMainForm::Panel221Resize(TObject *Sender)
{
    trace(3,"Panel221Resize\n");
    
    BtnPlotType1->Left=Panel221->Width-BtnPlotType1->Width-2;
    BtnFreqType1->Left=BtnPlotType1->Left-BtnFreqType1->Width-2;
    UpdatePlot();
}
// callback panel 222 resize -------------------------------------------------
void __fastcall TMainForm::Panel222Resize(TObject *Sender)
{
    trace(3,"Panel222Resize\n");
    
    BtnPlotType2->Left=Panel222->Width-BtnPlotType2->Width-2;
    BtnFreqType2->Left=BtnPlotType2->Left-BtnFreqType2->Width-2;
    UpdatePlot();
}
// callback panel 4 resize ---------------------------------------------------
void __fastcall TMainForm::Panel4Resize(TObject *Sender)
{
    TButton *btn[]={BtnStart,BtnStop,BtnPlot,BtnOpt,BtnExit};
    TPanel *panel=(TPanel *)Sender;
    int w=panel->Width/5;
    for (int i=0;i<5;i++) {
        btn[i]->Left=w*i+1;
        btn[i]->Width=w-2;
    }
}
// callback panel 5 resize ---------------------------------------------------
void __fastcall TMainForm::Panel5Resize(TObject *Sender)
{
	BtnSolType2->Left=BtnSolType2->Parent->Width-BtnSolType2->Width-2;
}
// update panel -------------------------------------------------------------
void __fastcall TMainForm::UpdatePanel(void)
{
    if (PanelMode==0) {
        Panel21 ->Visible=true;
        Panel5  ->Visible=false;
        Panel221->Visible=false;
    }
    else if (PanelMode==1) {
        Panel21 ->Visible=true;
        Panel5  ->Visible=false;
        Panel221->Visible=true;
    }
    else if (PanelMode==2) {
        Panel21 ->Visible=false;
        Panel5  ->Visible=true;
        Panel221->Visible=true;
    }
    else {
        Panel21 ->Visible=false;
        Panel5  ->Visible=true;
        Panel221->Visible=false;
    }
    if (PanelStack==0) {
        Panel21 ->Align=alLeft;
        Panel221->Align=alLeft;
        Splitter1->Align=alNone;
        Splitter2->Align=alNone;
        Splitter1->Left=Panel21 ->Width;
        Splitter2->Left=Panel221->Width;
        Splitter1->Width=2;
        Splitter2->Width=2;
        Splitter1->Align=alLeft;
        Splitter2->Align=alLeft;
    }
    else {
        Panel21 ->Align=alTop;
        Panel221->Align=alTop;
        Splitter1->Align=alNone;
        Splitter2->Align=alNone;
        Splitter1->Top=Panel21 ->Height;
        Splitter2->Top=Panel221->Height;
        Splitter1->Height=2;
        Splitter2->Height=2;
        Splitter1->Align=alTop;
        Splitter2->Align=alTop;
    }
}
// callback on button-exit --------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    trace(3,"BtnExitClick\n");
    
    Close();
}
// callback on button-start -------------------------------------------------
void __fastcall TMainForm::BtnStartClick(TObject *Sender)
{
    trace(3,"BtnStartClick\n");
    
    SvrStart();
}
// callback on button-stop --------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
    trace(3,"BtnStopClick\n");
    
    SvrStop();
}
// callback on button-plot --------------------------------------------------
void __fastcall TMainForm::BtnPlotClick(TObject *Sender)
{
    AnsiString cmd,Ansi_Caption=Caption;
    
    trace(3,"BtnPlotClick\n");
    
    if (OpenPort<=0) {
        ShowMessage("monitor port not open");
        return;
    }
    cmd.sprintf("rtkplot -p tcpcli://localhost:%d -t \"%s %s\"",OpenPort,
                Ansi_Caption.c_str(),": RTKPLOT");
    if (!ExecCmd(cmd,1)) {
        ShowMessage("error : rtkplot execution");
    }
}
// callback on button-options -----------------------------------------------
void __fastcall TMainForm::BtnOptClick(TObject *Sender)
{
    int i,chgmoni=0;
    
    trace(3,"BtnOptClick\n");
    
    OptDialog->PrcOpt     =PrcOpt;
    OptDialog->SolOpt     =SolOpt;
    OptDialog->DebugStatusF=DebugStatusF;
    OptDialog->DebugTraceF=DebugTraceF;
    OptDialog->BaselineC  =BaselineC;
    OptDialog->Baseline[0]=Baseline[0];
    OptDialog->Baseline[1]=Baseline[1];
    
    OptDialog->RovPosTypeF=RovPosTypeF;
    OptDialog->RefPosTypeF=RefPosTypeF;
    OptDialog->RovAntPcvF =RovAntPcvF;
    OptDialog->RefAntPcvF =RefAntPcvF;
    OptDialog->RovAntF    =RovAntF;
    OptDialog->RefAntF    =RefAntF;
    
    OptDialog->SatPcvFileF=SatPcvFileF;
    OptDialog->AntPcvFileF=AntPcvFileF;
    OptDialog->StaPosFileF=StaPosFileF;
    OptDialog->GeoidDataFileF=GeoidDataFileF;
    OptDialog->DCBFileF   =DCBFileF;
    OptDialog->EOPFileF   =EOPFileF;
    OptDialog->TLEFileF   =TLEFileF;
    OptDialog->TLESatFileF=TLESatFileF;
    OptDialog->LocalDirectory=LocalDirectory;
    
    OptDialog->SvrCycle   =SvrCycle;
    OptDialog->TimeoutTime=TimeoutTime;
    OptDialog->ReconTime  =ReconTime;
    OptDialog->NmeaCycle  =NmeaCycle;
    OptDialog->FileSwapMargin=FileSwapMargin;
    OptDialog->SvrBuffSize=SvrBuffSize;
    OptDialog->SolBuffSize=SolBuffSize;
    OptDialog->SavedSol   =SavedSol;
    OptDialog->NavSelect  =NavSelect;
    OptDialog->DgpsCorr   =DgpsCorr;
    OptDialog->SbasCorr   =SbasCorr;
    OptDialog->ExSats     =ExSats;
    OptDialog->ProxyAddr  =ProxyAddr;
    OptDialog->MoniPort   =MoniPort;
    OptDialog->PanelStack =PanelStack;
    
    for (i=0;i<3;i++) {
        OptDialog->RovAntDel[i]=RovAntDel[i];
        OptDialog->RefAntDel[i]=RefAntDel[i];
        OptDialog->RovPos   [i]=RovPos   [i];
        OptDialog->RefPos   [i]=RefPos   [i];
    }
    OptDialog->PosFont->Assign(PosFont);
    
    if (OptDialog->ShowModal()!=mrOk) return;
    
    PrcOpt     =OptDialog->PrcOpt;
    SolOpt     =OptDialog->SolOpt;
    DebugStatusF=OptDialog->DebugStatusF;
    DebugTraceF=OptDialog->DebugTraceF;
    BaselineC  =OptDialog->BaselineC;
    Baseline[0]=OptDialog->Baseline[0];
    Baseline[1]=OptDialog->Baseline[1];
    
    RovPosTypeF=OptDialog->RovPosTypeF;
    RefPosTypeF=OptDialog->RefPosTypeF;
    RovAntPcvF =OptDialog->RovAntPcvF;
    RefAntPcvF =OptDialog->RefAntPcvF;
    RovAntF    =OptDialog->RovAntF;
    RefAntF    =OptDialog->RefAntF;
    
    SatPcvFileF=OptDialog->SatPcvFileF;
    AntPcvFileF=OptDialog->AntPcvFileF;
    StaPosFileF=OptDialog->StaPosFileF;
    GeoidDataFileF=OptDialog->GeoidDataFileF;
    DCBFileF   =OptDialog->DCBFileF;
    EOPFileF   =OptDialog->EOPFileF;
    TLEFileF   =OptDialog->TLEFileF;
    TLESatFileF=OptDialog->TLESatFileF;
    LocalDirectory=OptDialog->LocalDirectory;
    
    SvrCycle   =OptDialog->SvrCycle;
    TimeoutTime=OptDialog->TimeoutTime;
    ReconTime  =OptDialog->ReconTime;
    NmeaCycle  =OptDialog->NmeaCycle;
    FileSwapMargin=OptDialog->FileSwapMargin;
    SvrBuffSize=OptDialog->SvrBuffSize;
    SavedSol   =OptDialog->SavedSol;
    NavSelect  =OptDialog->NavSelect;
    DgpsCorr   =OptDialog->DgpsCorr;
    SbasCorr   =OptDialog->SbasCorr;
    ExSats     =OptDialog->ExSats;
    ProxyAddr  =OptDialog->ProxyAddr;
    if (MoniPort!=OptDialog->MoniPort) chgmoni=1;
    MoniPort   =OptDialog->MoniPort;
    PanelStack =OptDialog->PanelStack;
    
    if (SolBuffSize!=OptDialog->SolBuffSize) {
        SolBuffSize=OptDialog->SolBuffSize;
        InitSolBuff();
        UpdateTime();
        UpdatePos();
        UpdatePlot();
    }
    for (i=0;i<3;i++) {
        RovAntDel[i]=OptDialog->RovAntDel[i];
        RefAntDel[i]=OptDialog->RefAntDel[i];
        RovPos   [i]=OptDialog->RovPos   [i];
        RefPos   [i]=OptDialog->RefPos   [i];
    }
    PosFont->Assign(OptDialog->PosFont);
    
    UpdateFont();
    UpdatePanel();
    
    if (!chgmoni) return;
    
    // send disconnect message
    if (OpenPort>0) {
        strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));
        
        strclose(&monistr);
    }
    // reopen monitor stream
    OpenMoniPort(MoniPort);
}
// callback on button-input-streams -----------------------------------------
void __fastcall TMainForm::BtnInputStrClick(TObject *Sender)
{
    int i,j;
    
    trace(3,"BtnInputStrClick\n");
    
    for (i=0;i<3;i++) {
        InputStrDialog->StreamC[i]=StreamC[i];
        InputStrDialog->Stream [i]=Stream [i];
        InputStrDialog->Format [i]=Format [i];
        InputStrDialog->RcvOpt [i]=RcvOpt [i];
        
        /* Paths[0]:serial,[1]:tcp,[2]:file,[3]:ftp */
        for (j=0;j<4;j++) InputStrDialog->Paths[i][j]=Paths[i][j];
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        InputStrDialog->CmdEna   [i][j]=CmdEna   [i][j];
        InputStrDialog->Cmds     [i][j]=Cmds     [i][j];
        InputStrDialog->CmdEnaTcp[i][j]=CmdEnaTcp[i][j];
        InputStrDialog->CmdsTcp  [i][j]=CmdsTcp  [i][j];
    }
    for (i=0;i<10;i++) {
        InputStrDialog->History [i]=History [i];
        InputStrDialog->MntpHist[i]=MntpHist[i];
    }
    InputStrDialog->NmeaReq   =NmeaReq;
    InputStrDialog->TimeTag   =InTimeTag;
    InputStrDialog->TimeSpeed =InTimeSpeed;
    InputStrDialog->TimeStart =InTimeStart;
    InputStrDialog->NmeaPos[0]=NmeaPos[0];
    InputStrDialog->NmeaPos[1]=NmeaPos[1];
    
    if (InputStrDialog->ShowModal()!=mrOk) return;
    
    for (i=0;i<3;i++) {
        StreamC[i]=InputStrDialog->StreamC[i];
        Stream [i]=InputStrDialog->Stream[i];
        Format [i]=InputStrDialog->Format[i];
        RcvOpt [i]=InputStrDialog->RcvOpt[i];
        for (j=0;j<4;j++) Paths[i][j]=InputStrDialog->Paths[i][j];
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        CmdEna   [i][j]=InputStrDialog->CmdEna   [i][j];
        Cmds     [i][j]=InputStrDialog->Cmds     [i][j];
        CmdEnaTcp[i][j]=InputStrDialog->CmdEnaTcp[i][j];
        CmdsTcp  [i][j]=InputStrDialog->CmdsTcp  [i][j];
    }
    for (i=0;i<10;i++) {
        History [i]=InputStrDialog->History [i];
        MntpHist[i]=InputStrDialog->MntpHist[i];
    }
    NmeaReq=InputStrDialog->NmeaReq;
    InTimeTag  =InputStrDialog->TimeTag;
    InTimeSpeed=InputStrDialog->TimeSpeed;
    InTimeStart=InputStrDialog->TimeStart;
    NmeaPos[0] =InputStrDialog->NmeaPos[0];
    NmeaPos[1] =InputStrDialog->NmeaPos[1];
}
// confirm overwrite --------------------------------------------------------
int __fastcall TMainForm::ConfOverwrite(const char *path)
{
    AnsiString s;
    FILE *fp;
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int i;
    char buff1[1024],buff2[1024],*p;
    
    trace(3,"ConfOverwrite\n");
    
    strcpy(buff1,path);
    
    if ((p=strstr(buff1,"::"))) *p='\0';
    
    if (!(fp=fopen(buff1,"r"))) return 1; // file not exists
    fclose(fp);
    
    // check overwrite input files
    for (i=0;i<3;i++) {
        if (!StreamC[i]||itype[Stream[i]]!=STR_FILE) continue;
        
        strcpy(buff2,Paths[i][2].c_str());
        if ((p=strstr(buff2,"::"))) *p='\0';
        
        if (!strcmp(buff1,buff2)) {
            Message->Caption=s.sprintf("invalid output %s",buff1);
            Message->Parent->Hint=Message->Caption;
            return 0;
        }
    }
    ConfDialog->Label2->Caption=buff1;
    
    return ConfDialog->ShowModal()==mrOk;
}
// callback on button-output-streams ----------------------------------------
void __fastcall TMainForm::BtnOutputStrClick(TObject *Sender)
{
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,j,str,update[2]={0};
    char *path;
    
    trace(3,"BtnOutputStrClick\n");
    
    for (i=3;i<5;i++) {
        OutputStrDialog->StreamC[i-3]=StreamC[i];
        OutputStrDialog->Stream [i-3]=Stream[i];
        OutputStrDialog->Format [i-3]=Format[i];
        for (j=0;j<4;j++) OutputStrDialog->Paths[i-3][j]=Paths[i][j];
    }
    for (i=0;i<10;i++) {
        OutputStrDialog->History [i]=History [i];
        OutputStrDialog->MntpHist[i]=MntpHist[i];
    }
    OutputStrDialog->OutTimeTag=OutTimeTag;
    OutputStrDialog->OutAppend =OutAppend;
    OutputStrDialog->SwapInterval=OutSwapInterval;
    
    if (OutputStrDialog->ShowModal()!=mrOk) return;
    
    for (i=3;i<5;i++) {
        if (StreamC[i]!=OutputStrDialog->StreamC[i-3]||
            Stream [i]!=OutputStrDialog->Stream[i-3]||
            Format [i]!=OutputStrDialog->Format[i-3]||
            Paths[i][0]!=OutputStrDialog->Paths[i-3][0]||
            Paths[i][1]!=OutputStrDialog->Paths[i-3][1]||
            Paths[i][2]!=OutputStrDialog->Paths[i-3][2]||
            Paths[i][3]!=OutputStrDialog->Paths[i-3][3]) update[i-3]=1;
        StreamC[i]=OutputStrDialog->StreamC[i-3];
        Stream [i]=OutputStrDialog->Stream[i-3];
        Format [i]=OutputStrDialog->Format[i-3];
        for (j=0;j<4;j++) Paths[i][j]=OutputStrDialog->Paths[i-3][j];
    }
    for (i=0;i<10;i++) {
        History [i]=OutputStrDialog->History [i];
        MntpHist[i]=OutputStrDialog->MntpHist[i];
    }
    OutTimeTag=OutputStrDialog->OutTimeTag;
    OutAppend =OutputStrDialog->OutAppend;
    OutSwapInterval=OutputStrDialog->SwapInterval;
    
    if (BtnStart->Enabled) return;
    
    for (i=3;i<5;i++) {
        if (!update[i-3]) continue;
        
        rtksvrclosestr(&rtksvr,i);
        
        if (!StreamC[i]) continue;
        
        str=otype[Stream[i]];
        if      (str==STR_SERIAL)             path=Paths[i][0].c_str();
        else if (str==STR_FILE  )             path=Paths[i][2].c_str();
        else if (str==STR_FTP||str==STR_HTTP) path=Paths[i][3].c_str();
        else                                  path=Paths[i][1].c_str();
        if (str==STR_FILE&&!ConfOverwrite(path)) {
            StreamC[i]=0;
            continue;
        }
        SolOpt.posf=Format[i];
        rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
    }
}
// callback on button-log-streams -------------------------------------------
void __fastcall TMainForm::BtnLogStrClick(TObject *Sender)
{
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,j,str,update[3]={0};
    char *path;
    
    trace(3,"BtnLogStrClick\n");
    
    for (i=5;i<8;i++) {
        LogStrDialog->StreamC[i-5]=StreamC[i];
        LogStrDialog->Stream [i-5]=Stream [i];
        for (j=0;j<4;j++) LogStrDialog->Paths[i-5][j]=Paths[i][j];
    }
    for (i=0;i<10;i++) {
        LogStrDialog->History [i]=History [i];
        LogStrDialog->MntpHist[i]=MntpHist[i];
    }
    LogStrDialog->LogTimeTag=LogTimeTag;
    LogStrDialog->LogAppend =LogAppend;
    LogStrDialog->SwapInterval=LogSwapInterval;
    
    if (LogStrDialog->ShowModal()!=mrOk) return;
    
    for (i=5;i<8;i++) {
        if (StreamC[i]!=OutputStrDialog->StreamC[i-5]||
            Stream [i]!=OutputStrDialog->Stream[i-5]||
            Paths[i][0]!=OutputStrDialog->Paths[i-3][0]||
            Paths[i][1]!=OutputStrDialog->Paths[i-3][1]||
            Paths[i][2]!=OutputStrDialog->Paths[i-3][2]||
            Paths[i][3]!=OutputStrDialog->Paths[i-3][3]) update[i-5]=1;
        StreamC[i]=LogStrDialog->StreamC[i-5];
        Stream [i]=LogStrDialog->Stream [i-5];
        for (j=0;j<4;j++) Paths[i][j]=LogStrDialog->Paths[i-5][j];
    }
    for (i=0;i<10;i++) {
        History [i]=LogStrDialog->History [i];
        MntpHist[i]=LogStrDialog->MntpHist[i];
    }
    LogTimeTag=LogStrDialog->LogTimeTag;
    LogAppend =LogStrDialog->LogAppend;
    LogSwapInterval=LogStrDialog->SwapInterval;
    
    if (BtnStart->Enabled) return;
    
    for (i=5;i<8;i++) {
        if (!update[i-5]) continue;
        
        rtksvrclosestr(&rtksvr,i);
        
        if (!StreamC[i]) continue;
        
        str=otype[Stream[i]];
        if      (str==STR_SERIAL)             path=Paths[i][0].c_str();
        else if (str==STR_FILE  )             path=Paths[i][2].c_str();
        else if (str==STR_FTP||str==STR_HTTP) path=Paths[i][3].c_str();
        else                                  path=Paths[i][1].c_str();
        if (str==STR_FILE&&!ConfOverwrite(path)) {
            StreamC[i]=0;
            continue;
        }
        rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
    }
}
// callback on button-solution-show -----------------------------------------
void __fastcall TMainForm::BtnPanelClick(TObject *Sender)
{
    trace(3,"BtnPanelClick\n");
    
    if (++PanelMode>3) PanelMode=0;
    UpdatePanel();
}
// callback on button-plot-type-1 -------------------------------------------
void __fastcall TMainForm::BtnTimeSysClick(TObject *Sender)
{
    trace(3,"BtnTimeSysClick\n");
    
    if (++TimeSys>3) TimeSys=0;
    UpdateTimeSys();
}
// callback on button-solution-type -----------------------------------------
void __fastcall TMainForm::BtnSolTypeClick(TObject *Sender)
{
    trace(3,"BtnSolTypeClick\n");
    
    if (++SolType>4) SolType=0;
    UpdateSolType();
}
// callback on button-plottype-1 --------------------------------------------
void __fastcall TMainForm::BtnPlotType1Click(TObject *Sender)
{
    trace(3,"BtnPlotType1Click\n");
    
    if (++PlotType1>4) PlotType1=0;
    UpdatePlot();
    UpdatePos();
}
// callback on button-plottype-2 --------------------------------------------
void __fastcall TMainForm::BtnPlotType2Click(TObject *Sender)
{
    trace(3,"BtnPlotType2Click\n");
    
    if (++PlotType2>4) PlotType2=0;
    UpdatePlot();
    UpdatePos();
}
// callback on button frequency-type-1 --------------------------------------
void __fastcall TMainForm::BtnFreqType1Click(TObject *Sender)
{
    trace(3,"BtnFreqType1Click\n");
    
    if (++FreqType1>NFREQ+1) FreqType1=0;
    UpdateSolType();
}
// callback on button frequency-type-2 --------------------------------------
void __fastcall TMainForm::BtnFreqType2Click(TObject *Sender)
{
    trace(3,"BtnFreqType2Click\n");
    
    if (++FreqType2>NFREQ+1) FreqType2=0;
    UpdateSolType();
}
// callback on button-rtk-monitor -------------------------------------------
void __fastcall TMainForm::BtnMonitorClick(TObject *Sender)
{
    TMonitorDialog *monitor=new TMonitorDialog(Application);
    
    trace(3,"BtnMonitorClick\n");
    
    monitor->Caption=Caption+": RTK Monitor";
    monitor->Show();
}
// callback on scroll-solution change ---------------------------------------
void __fastcall TMainForm::ScbSolChange(TObject *Sender)
{
    trace(3,"ScbSolChange\n");
    
    PSol=PSolS+ScbSol->Position;
    if (PSol>=SolBuffSize) PSol-=SolBuffSize;
    UpdateTime();
    UpdatePos();
}
// callback on button-save --------------------------------------------------
void __fastcall TMainForm::BtnSaveClick(TObject *Sender)
{
    trace(3,"BtnSaveClick\n");
    
    SaveLog();
}
// callback on button-about -------------------------------------------------
void __fastcall TMainForm::BtnAboutClick(TObject *Sender)
{
    AnsiString prog=PRGNAME;
    
    trace(3,"BtnAboutClick\n");
#ifdef MKL
    prog+="_MKL";
#endif
    AboutDialog->About=prog;
    AboutDialog->IconIndex=5;
    AboutDialog->ShowModal();
}
// callback on button-tasktray ----------------------------------------------
void __fastcall TMainForm::BtnTaskTrayClick(TObject *Sender)
{
    trace(3,"BtnTaskTrayClick\n");
    
    Visible=false;
    TrayIcon->Hint=Caption;
    TrayIcon->Visible=true;
}
// callback on button-tasktray ----------------------------------------------
void __fastcall TMainForm::TrayIconDblClick(TObject *Sender)
{
    trace(3,"TaskIconDblClick\n");
    
    Visible=true;
    TrayIcon->Visible=false;
}
// callback on menu-expand --------------------------------------------------
void __fastcall TMainForm::MenuExpandClick(TObject *Sender)
{
    trace(3,"MenuExpandClick\n");
    
    Visible=true;
    TrayIcon->Visible=false;
}
// callback on menu-start ---------------------------------------------------
void __fastcall TMainForm::MenuStartClick(TObject *Sender)
{
    trace(3,"MenuStartClick\n");
    
    BtnStartClick(Sender);
}
// callback on menu-stop ----------------------------------------------------
void __fastcall TMainForm::MenuStopClick(TObject *Sender)
{
    trace(3,"MenuStopClick\n");
    
    BtnStopClick(Sender);
}
// callback on menu-monitor -------------------------------------------------
void __fastcall TMainForm::MenuMonitorClick(TObject *Sender)
{
    trace(3,"MenuMonitorClick\n");
    
    BtnMonitorClick(Sender);
}
// callback on menu-plot ----------------------------------------------------
void __fastcall TMainForm::MenuPlotClick(TObject *Sender)
{
    trace(3,"MenuPlotClick\n");
    
    BtnPlotClick(Sender);
}
// callback on menu-exit ----------------------------------------------------
void __fastcall TMainForm::MenuExitClick(TObject *Sender)
{
    trace(3,"MenuExitClick\n");
    
    BtnExitClick(Sender);
}
// start rtk server ---------------------------------------------------------
void __fastcall TMainForm::SvrStart(void)
{
    AnsiString s;
    solopt_t solopt[2];
    double pos[3],nmeapos[3];
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,strs[MAXSTRRTK]={0},sat,ex,stropt[8]={0};
    char *paths[8],*cmds[3]={0},*rcvopts[3]={0},buff[1024],*p;
    char file[1024],*type;
    FILE *fp;
    gtime_t time=timeget();
    pcvs_t pcvr={0},pcvs={0};
    pcv_t *pcv;
    
    trace(3,"SvrStart\n");
    
    Message->Caption=""; Message->Parent->Hint="";
    
    if (RovPosTypeF<=2) {
        PrcOpt.rovpos=0;
        PrcOpt.ru[0]=RovPos[0];
        PrcOpt.ru[1]=RovPos[1];
        PrcOpt.ru[2]=RovPos[2];
    }
    else {
        PrcOpt.rovpos=4;
        for (i=0;i<3;i++) PrcOpt.ru[i]=0.0;
    }
    if (RefPosTypeF<=2) {
        PrcOpt.refpos=0;
        PrcOpt.rb[0]=RefPos[0];
        PrcOpt.rb[1]=RefPos[1];
        PrcOpt.rb[2]=RefPos[2];
    }
    else {
        PrcOpt.refpos=4;
        for (i=0;i<3;i++) PrcOpt.rb[i]=0.0;
    }
    for (i=0;i<MAXSAT;i++) {
        PrcOpt.exsats[i]=0;
    }
    if (ExSats!="") { // excluded satellites
        strcpy(buff,ExSats.c_str());
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if (*p=='+') {ex=2; p++;} else ex=1;
            if (!(sat=satid2no(p))) continue;
            PrcOpt.exsats[sat-1]=ex;
        }
    }
    if ((RovAntPcvF||RefAntPcvF)&&!readpcv(AntPcvFileF.c_str(),&pcvr)) {
        Message->Caption=s.sprintf("rcv ant file read error %s",AntPcvFileF.c_str());
        Message->Parent->Hint=Message->Caption;
        return;
    }
    if (RovAntPcvF) {
        type=RovAntF.c_str();
        if ((pcv=searchpcv(0,type,time,&pcvr))) {
            PrcOpt.pcvr[0]=*pcv;
        }
        else {
            Message->Caption=s.sprintf("no antenna pcv %s",type);
            Message->Parent->Hint=Message->Caption;
        }
        for (i=0;i<3;i++) PrcOpt.antdel[0][i]=RovAntDel[i];
    }
    if (RefAntPcvF) {
        type=RefAntF.c_str();
        if ((pcv=searchpcv(0,type,time,&pcvr))) {
            PrcOpt.pcvr[1]=*pcv;
        }
        else {
            Message->Caption=s.sprintf("no antenna pcv %s",type);
            Message->Parent->Hint=Message->Caption;
        }
        for (i=0;i<3;i++) PrcOpt.antdel[1][i]=RefAntDel[i];
    }
    if (RovAntPcvF||RefAntPcvF) {
        free(pcvr.pcv);
    }
    if (PrcOpt.sateph==EPHOPT_PREC||PrcOpt.sateph==EPHOPT_SSRCOM) {
        if (!readpcv(SatPcvFileF.c_str(),&pcvs)) {
            Message->Caption=s.sprintf("sat ant file read error %s",SatPcvFileF.c_str());
            Message->Parent->Hint=Message->Caption;
            return;
        }
        for (i=0;i<MAXSAT;i++) {
            if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
            rtksvr.nav.pcvs[i]=*pcv;
        }
        free(pcvs.pcv);
    }
    if (BaselineC) {
        PrcOpt.baseline[0]=Baseline[0];
        PrcOpt.baseline[1]=Baseline[1];
    }
    else {
        PrcOpt.baseline[0]=0.0;
        PrcOpt.baseline[1]=0.0;
    }
    for (i=0;i<3;i++) strs[i]=StreamC[i]?itype[Stream[i]]:STR_NONE;
    for (i=3;i<5;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
    for (i=5;i<8;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
    for (i=0;i<8;i++) {
        if      (strs[i]==STR_NONE  ) paths[i]="";
        else if (strs[i]==STR_SERIAL) paths[i]=Paths[i][0].c_str();
        else if (strs[i]==STR_FILE  ) paths[i]=Paths[i][2].c_str();
        else if (strs[i]==STR_FTP||strs[i]==STR_HTTP) paths[i]=Paths[i][3].c_str();
        else paths[i]=Paths[i][1].c_str();
    }
    for (i=0;i<3;i++) {
        if (strs[i]==STR_SERIAL) {
            if (CmdEna[i][0]) cmds[i]=Cmds[i][0].c_str();
        }
        else if (strs[i]==STR_TCPCLI||strs[i]==STR_TCPSVR||
                 strs[i]==STR_NTRIPCLI) {
            if (CmdEnaTcp[i][0]) cmds[i]=CmdsTcp[i][0].c_str();
        }
        rcvopts[i]=RcvOpt[i].c_str();
    }
    NmeaCycle=NmeaCycle<1000?1000:NmeaCycle;
    pos[0]=NmeaPos[0]*D2R;
    pos[1]=NmeaPos[1]*D2R;
    pos[2]=0.0;
    pos2ecef(pos,nmeapos);
    
    strsetdir(LocalDirectory.c_str());
    strsetproxy(ProxyAddr.c_str());
    
    for (i=3;i<8;i++) {
        if (strs[i]==STR_FILE&&!ConfOverwrite(paths[i])) return;
    }
    if (DebugTraceF>0) {
        traceopen(TRACEFILE);
        tracelevel(DebugTraceF);
    }
    if (DebugStatusF>0) {
        rtkopenstat(STATFILE,DebugStatusF);
    }
    if (SolOpt.geoid>0&&GeoidDataFileF!="") {
        opengeoid(SolOpt.geoid,GeoidDataFileF.c_str());
    }
    if (DCBFileF!="") {
        readdcb(DCBFileF.c_str(),&rtksvr.nav);
    }
    for (i=0;i<2;i++) {
        solopt[i]=SolOpt;
        solopt[i].posf=Format[i+3];
    }
    stropt[0]=TimeoutTime;
    stropt[1]=ReconTime;
    stropt[2]=1000;
    stropt[3]=SvrBuffSize;
    stropt[4]=FileSwapMargin;
    strsetopt(stropt);
    
    // start rtk server
    if (!rtksvrstart(&rtksvr,SvrCycle,SvrBuffSize,strs,paths,Format,NavSelect,
                     cmds,rcvopts,NmeaCycle,NmeaReq,nmeapos,&PrcOpt,solopt,
                     &monistr)) {
        traceclose();
        return;
    }
    PSol=PSolS=PSolE=0;
    SolStat[0]=Nvsat[0]=0;
    for (i=0;i<3;i++) SolRov[i]=SolRef[i]=VelRov[i]=0.0;
    for (i=0;i<9;i++) Qr[i]=0.0;
    Age[0]=Ratio[0]=0.0;
    Nsat[0]=Nsat[1]=0;
    UpdatePos();
    UpdatePlot();
    BtnStart    ->Enabled=false;
    BtnOpt      ->Enabled=false;
    BtnExit     ->Enabled=false;
    BtnInputStr ->Enabled=false;
    MenuStart   ->Enabled=false;
    MenuExit    ->Enabled=false;
    ScbSol      ->Enabled=false;
    BtnStop     ->Enabled=true;
    MenuStop    ->Enabled=true;
    Svr->Color=CLORANGE;
    SetTrayIcon(0);
}
// strop rtk server ---------------------------------------------------------
void __fastcall TMainForm::SvrStop(void)
{
    char *cmds[3]={0};
    int i,n,m,str;
    
    trace(3,"SvrStop\n");
    
    for (i=0;i<3;i++) {
        str=rtksvr.stream[i].type;
        
        if (str==STR_SERIAL) {
            if (CmdEna[i][1]) cmds[i]=Cmds[i][1].c_str();
        }
        else if (str==STR_TCPCLI||str==STR_TCPSVR||str==STR_NTRIPCLI) {
            if (CmdEnaTcp[i][1]) cmds[i]=CmdsTcp[i][1].c_str();
        }
    }
    rtksvrstop(&rtksvr,cmds);
    
    BtnStart    ->Enabled=true;
    BtnOpt      ->Enabled=true;
    BtnExit     ->Enabled=true;
    BtnInputStr ->Enabled=true;
    MenuStart   ->Enabled=true;
    MenuExit    ->Enabled=true;
    ScbSol      ->Enabled=true;
    BtnStop     ->Enabled=false;
    MenuStop    ->Enabled=false;
    Svr->Color=clBtnFace;
    SetTrayIcon(1);
    
    LabelTime->Font->Color=clGray;
    IndSol->Color=clWhite;
    n=PSolE-PSolS; if (n<0) n+=SolBuffSize;
    m=PSol-PSolS;  if (m<0) m+=SolBuffSize;
    if (n>0) {
        ScbSol->Max=n-1; ScbSol->Position=m;
    }
    Message->Caption=""; Message->Parent->Hint="";
    
    if (DebugTraceF>0) traceclose();
    if (DebugStatusF>0) rtkclosestat();
    if (OutputGeoidF>0&&GeoidDataFileF!="") closegeoid();
}
// callback on interval timer -----------------------------------------------
void __fastcall TMainForm::TimerTimer(TObject *Sender)
{
    static int n=0,inactive=0;
    sol_t *sol;
    int i,update=0;
    unsigned char buff[8];
    
    trace(4,"TimerTimer\n");
    
    rtksvrlock(&rtksvr);
    
    for (i=0;i<rtksvr.nsol;i++) {
        sol=rtksvr.solbuf+i;
        UpdateLog(sol->stat,sol->time,sol->rr,sol->qr,rtksvr.rtk.rb,sol->ns,
                  sol->age,sol->ratio);
        update=1;
    }
    rtksvr.nsol=0;
    SolCurrentStat=rtksvr.state?rtksvr.rtk.sol.stat:0;
    
    rtksvrunlock(&rtksvr);
    
    if (update) {
        UpdateTime();
        UpdatePos();
        inactive=0;
    }
    else {
        if (++inactive*Timer->Interval>TIMEOUT) SolCurrentStat=0;
    }
    if (SolCurrentStat) {
        Svr->Color=clLime;
        LabelTime->Font->Color=clBlack;
    }
    else {
        IndSol->Color=clWhite;
        Solution->Font->Color=clGray;
        Svr->Color=rtksvr.state?clGreen:clBtnFace;
    }
    if (!(++n%5)) UpdatePlot();
	UpdateStr();
    
    // keep alive for monitor port
    if (!(++n%(KACYCLE/Timer->Interval))&&OpenPort) {
        strwrite(&monistr,"\r",1);
    }
}
// change plot type ---------------------------------------------------------
void __fastcall TMainForm::ChangePlot(void)
{
}
// update time-system -------------------------------------------------------
void __fastcall TMainForm::UpdateTimeSys(void)
{
    AnsiString label[]={"GPST","UTC","LT","GPST"};
    
    trace(3,"UpdateTimeSys\n");
    
    BtnTimeSys->Caption=label[TimeSys];
    UpdateTime();
}
// update solution type -----------------------------------------------------
void __fastcall TMainForm::UpdateSolType(void)
{
    AnsiString label[]={
        "Lat/Lon/Height","Lat/Lon/Height","X/Y/Z-ECEF","E/N/U-Baseline",
        "Pitch/Yaw/Length-Baseline",""
    };
    trace(3,"UpdateSolType\n");
    
    Plabel0->Caption=label[SolType];
    UpdatePos();
}
// update log ---------------------------------------------------------------
void __fastcall TMainForm::UpdateLog(int stat, gtime_t time, double *rr,
    float *qr, double *rb, int ns, double age, double ratio)
{
    int i,ena;
    
    if (!stat) return;
    
    trace(4,"UpdateLog\n");
    
    SolStat[PSolE]=stat; Time[PSolE]=time; Nvsat[PSolE]=ns; Age[PSolE]=age;
    Ratio[PSolE]=ratio;
    for (i=0;i<3;i++) {
        SolRov[i+PSolE*3]=rr[i];
        SolRef[i+PSolE*3]=rb[i];
        VelRov[i+PSolE*3]=rr[i+3];
    }
    Qr[  PSolE*9]=qr[0];
    Qr[4+PSolE*9]=qr[1];
    Qr[8+PSolE*9]=qr[2];
    Qr[1+PSolE*9]=Qr[3+PSolE*9]=qr[3];
    Qr[5+PSolE*9]=Qr[7+PSolE*9]=qr[4];
    Qr[2+PSolE*9]=Qr[6+PSolE*9]=qr[5];
    
    PSol=PSolE;
    if (++PSolE>=SolBuffSize) PSolE=0;
    if (PSolE==PSolS&&++PSolS>=SolBuffSize) PSolS=0;
}
// update font --------------------------------------------------------------
void __fastcall TMainForm::UpdateFont(void)
{
    TLabel *label[]={
        PlabelA,Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3,Solution,LabelStd,LabelNSat
    };
    TColor color=label[7]->Font->Color;
    int i;
    
    trace(4,"UpdateFont\n");
    
    for (i=0;i<10;i++) label[i]->Font->Assign(PosFont);
    label[0]->Font->Size=9; label[7]->Font->Color=color;
    label[8]->Font->Size=8; label[8]->Font->Color=clGray;
    label[9]->Font->Size=8; label[9]->Font->Color=clGray;
}
// update time --------------------------------------------------------------
void __fastcall TMainForm::UpdateTime(void)
{
    gtime_t time=Time[PSol];
    struct tm *t;
    double tow;
    int week;
    char tstr[64];
    
    trace(4,"UpdateTime\n");
    
    if      (TimeSys==0) time2str(time,tstr,1);
    else if (TimeSys==1) time2str(gpst2utc(time),tstr,1);
    else if (TimeSys==2) {
        time=gpst2utc(time);
        if (!(t=localtime(&time.time))) strcpy(tstr,"2000/01/01 00:00:00.0");
        else sprintf(tstr,"%04d/%02d/%02d %02d:%02d:%02d.%d",t->tm_year+1900,
                     t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,
                     (int)(time.sec*10));
    }
    else if (TimeSys==3) {
        tow=time2gpst(time,&week); sprintf(tstr,"week %04d %8.1f s",week,tow);
    }
    LabelTime->Caption=tstr;
}
// update solution display --------------------------------------------------
void __fastcall TMainForm::UpdatePos(void)
{
    TLabel *label[]={Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3,LabelStd,LabelNSat};
    AnsiString sol[]={"----","FIX","FLOAT","SBAS","DGPS","SINGLE","PPP"};
    UnicodeString s[8];
    TColor color[]={clSilver,clGreen,CLORANGE,clFuchsia,clBlue,clRed,clTeal};
    gtime_t time;
    double *rr=SolRov+PSol*3,*rb=SolRef+PSol*3,*qr=Qr+PSol*9,pos[3]={0},Qe[9]={0};
    double dms1[3]={0},dms2[3]={0},bl[3]={0},enu[3]={0},pitch=0.0,yaw=0.0,len;
    int i,stat=SolStat[PSol];
    
    trace(4,"UpdatePos\n");
    
    Solution->Caption=sol[stat];
    Solution->Font->Color=rtksvr.state?color[stat]:clGray;
    IndSol->Color=rtksvr.state&&stat?color[stat]:clWhite;
    if (norm(rr,3)>0.0&&norm(rb,3)>0.0) {
        for (i=0;i<3;i++) bl[i]=rr[i]-rb[i];
    }
    len=norm(bl,3);
    if (SolType==0) {
        if (norm(rr,3)>0.0) {
            ecef2pos(rr,pos); covenu(pos,qr,Qe);
            degtodms(pos[0]*R2D,dms1);
            degtodms(pos[1]*R2D,dms2);
            if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
        }
        s[0]=pos[0]<0?L"S:":L"N:"; s[1]=pos[1]<0?L"W:":L"E:";
        s[2]=SolOpt.height==1?L"H:":L"He:";
        s[3].sprintf(L"%.0f%c %02.0f' %07.4f\"",fabs(dms1[0]),CHARDEG,dms1[1],dms1[2]);
        s[4].sprintf(L"%.0f%c %02.0f' %07.4f\"",fabs(dms2[0]),CHARDEG,dms2[1],dms2[2]);
        s[5].sprintf(L"%.3f m",pos[2]);
        s[6].sprintf(L"N:%6.3f E:%6.3f U:%6.3f m",SQRT(Qe[4]),SQRT(Qe[0]),SQRT(Qe[8]));
    }
    else if (SolType==1) {
        if (norm(rr,3)>0.0) {
            ecef2pos(rr,pos); covenu(pos,qr,Qe);
            if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
        }
        s[0]=pos[0]<0?L"S:":L"N:"; s[1]=pos[1]<0?L"W:":L"E:";
        s[2]=SolOpt.height==1?L"H:":L"He:";
        s[3].sprintf(L"%.8f %c",fabs(pos[0])*R2D,CHARDEG);
        s[4].sprintf(L"%.8f %c",fabs(pos[1])*R2D,CHARDEG);
        s[5].sprintf(L"%.3f m",pos[2]);
        s[6].sprintf(L"E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
    }
    else if (SolType==2) {
        s[0]=L"X:"; s[1]=L"Y:"; s[2]=L"Z:";
        s[3].sprintf(L"%.3f m",rr[0]);
        s[4].sprintf(L"%.3f m",rr[1]);
        s[5].sprintf(L"%.3f m",rr[2]);
        s[6].sprintf(L"X:%6.3f Y:%6.3f Z:%6.3f m",SQRT(qr[0]),SQRT(qr[4]),SQRT(qr[8]));
    }
    else if (SolType==3) {
        if (len>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
        }
        s[0]=L"E:"; s[1]=L"N:"; s[2]=L"U:";
        s[3].sprintf(L"%.3f m",enu[0]);
        s[4].sprintf(L"%.3f m",enu[1]);
        s[5].sprintf(L"%.3f m",enu[2]);
        s[6].sprintf(L"E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
    }
    else {
        if (len>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
            pitch=asin(enu[2]/len);
            yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
        }
        s[0]=L"P:"; s[1]=L"Y:"; s[2]=L"L:";
        s[3].sprintf(L"%.3f %c",pitch*R2D,CHARDEG);
        s[4].sprintf(L"%.3f %c",yaw*R2D,CHARDEG);
        s[5].sprintf(L"%.3f m",len);
        s[6].sprintf(L"E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
    }
    s[7].sprintf(L"Age:%4.1f s Ratio:%4.1f # Sat:%2d",Age[PSol],Ratio[PSol],Nvsat[PSol]);
    
    for (i=0;i<8;i++) label[i]->Caption=s[i];
    for (i=3;i<6;i++) {
        label[i]->Font->Color=PrcOpt.mode==PMODE_MOVEB&&SolType<=2?clGray:clBlack;
    }
    IndQ->Color=IndSol->Color;
    SolS->Caption=Solution->Caption;
    SolS->Font->Color=Solution->Font->Color;
    SolQ->Caption=label[0]->Caption+L" "+label[3]->Caption+L" "+
                  label[1]->Caption+L" "+label[4]->Caption+L" "+
                  label[2]->Caption+L" "+label[5]->Caption;
}
// update stream status indicators ------------------------------------------
void __fastcall TMainForm::UpdateStr(void)
{
    TColor color[]={clRed,clBtnFace,CLORANGE,clGreen,clLime};
    TPanel *ind[MAXSTRRTK]={Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8};
    int i,sstat[MAXSTRRTK]={0};
    char msg[MAXSTRMSG]="";
    
    trace(4,"UpdateStr\n");
    
    rtksvrsstat(&rtksvr,sstat,msg);
    for (i=0;i<MAXSTRRTK;i++) {
        ind[i]->Color=color[sstat[i]+1];
        if (sstat[i]) {
            Message->Caption=msg;
            Message->Parent->Hint=Message->Caption;
        }
    }
}
// draw solution plot -------------------------------------------------------
void __fastcall TMainForm::DrawPlot(TImage *plot, int type, int freq)
{
    UnicodeString s;
    gtime_t time;
    TCanvas *c=plot->Canvas;
    TLabel *label[]={Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3};
    wchar_t *fstr[]={L"",L"L1 ",L"L2 ",L"L5 ",L"L6 ",L"L7 ",L"L8 ",L""};
    int w=plot->Parent->Width-2,h=plot->Parent->Height-2;
    int i,j,x,sat[2][MAXSAT],ns[2],snr[2][MAXSAT][NFREQ],vsat[2][MAXSAT];
    int *snr0[MAXSAT],*snr1[MAXSAT];
    char name[16];
    double az[2][MAXSAT],el[2][MAXSAT],rr[3],rs[6],e[3],pos[3],azel[2];
    
    trace(4,"DrawPlot\n");
    
    fstr[NFREQ+1]=L"SYS ";
    
    for (i=0;i<MAXSAT;i++) {
        snr0[i]=snr[0][i];
        snr1[i]=snr[1][i];
    }
    ns[0]=rtksvrostat(&rtksvr,0,&time,sat[0],az[0],el[0],snr0,vsat[0]);
    ns[1]=rtksvrostat(&rtksvr,1,&time,sat[1],az[1],el[1],snr1,vsat[1]);
    
    rtksvrlock(&rtksvr);
    matcpy(rr,rtksvr.rtk.sol.rr,3,1);
    ecef2pos(rr,pos);
    rtksvrunlock(&rtksvr);
    
    for (i=0;i<2;i++) {
        for (j=0;j<ns[i];j++) {
            if (az[i][j]!=0.0||el[i][j]!=0.0) continue;
            satno2id(sat[i][j],name);
            if (!tle_pos(time,name,"","",&TLEData,NULL,rs)) continue;
            if (geodist(rs,rr,e)>0.0) {
                satazel(pos,e,azel);
                az[i][j]=azel[0];
                el[i][j]=azel[1];
            }
        }
        if (ns[i]>0) {
            Nsat[i]=ns[i];
            for (int j=0;j<ns[i];j++) {
                Sat [i][j]=sat [i][j];
                Az  [i][j]=az  [i][j];
                El  [i][j]=el  [i][j];
                for (int k=0;k<NFREQ;k++) {
                    Snr[i][j][k]=snr[i][j][k];
                }
                Vsat[i][j]=vsat[i][j];
            }
        }
        else {
            for (j=0;j<Nsat[i];j++) {
                Vsat[i][j]=0;
                for (int k=0;k<NFREQ;k++) {
                    Snr[i][j][k]=0;
                }
            }
        }
    }
    c->Brush->Style=bsSolid;
    c->Brush->Color=clWhite;
    c->FillRect(plot->ClientRect);
    x=4;
    if (type==0) { // snr plot rover/base
        DrawSnr(c,w,(h-12)/2,15,0,freq);
        DrawSnr(c,w,(h-12)/2,14+(h-12)/2,1,freq);
        s.sprintf(L"Rover:Base %sSNR (dBHz)",fstr[freq]);
        DrawText(c,x,1,s,clGray,0);
    }
    else if (type==1) { // snr plot rover
        DrawSnr(c,w,h-15,15,0,freq);
        s.sprintf(L"Rover %s SNR (dBHz)",fstr[freq]);
        DrawText(c,x,1,s,clGray,0);
    }
    else if (type==2) { // skyplot rover
        DrawSat(c,w,h,0,0,0,freq);
        s.sprintf(L"Rover %s",fstr[!freq?1:freq]);
        DrawText(c,x,1,s,clGray,0);
    }
    else if (type==3) { // skyplot rover/base
        DrawSat(c,w/2,h,0  ,0,0,freq);
        DrawSat(c,w/2,h,w/2,0,1,freq);
        s.sprintf(L"Rover %s",fstr[!freq?1:freq]);
        DrawText(c,x,1,s,clGray,0);
        s.sprintf(L"Base %s",fstr[!freq?1:freq]);
        DrawText(c,x+w/2,1,s,clGray,0);
    }
    else if (type==4) { // baseline plot
        DrawBL(c,w,h);
        DrawText(c,x,1,L"Baseline",clGray,0);
    }
}
// update solution plot ------------------------------------------------------
void __fastcall TMainForm::UpdatePlot(void)
{
    DrawPlot(Plot1,PlotType1,FreqType1);
    DrawPlot(Plot2,PlotType2,FreqType2);
    Disp1->Canvas->CopyRect(Disp1->ClientRect,Plot1->Canvas,Disp1->ClientRect);
    Disp2->Canvas->CopyRect(Disp2->ClientRect,Plot2->Canvas,Disp2->ClientRect);
}
// snr color ----------------------------------------------------------------
TColor __fastcall TMainForm::SnrColor(int snr)
{
    TColor color[]={clGreen,CLORANGE,clFuchsia,clBlue,clRed,clGray};
    unsigned int c1,c2,r1,r2,g1,g2,b1,b2;
    double a;
    int i;
    
    if (snr<25) return color[5];
    if (snr<27) return color[4];
    if (snr>47) return color[0];
    a=(snr-27.5)/5.0;
    i=(int)a; a-=i;
    c1=(unsigned int)color[3-i];
    c2=(unsigned int)color[4-i];
    r1=c1&0xFF; g1=(c1>>8)&0xFF; b1=(c1>>16)&0xFF;
    r2=c2&0xFF; g2=(c2>>8)&0xFF; b2=(c2>>16)&0xFF;
    r1=(unsigned int)(a*r1+(1.0-a)*r2)&0xFF;
    g1=(unsigned int)(a*g1+(1.0-a)*g2)&0xFF;
    b1=(unsigned int)(a*b1+(1.0-a)*b2)&0xFF;
    
    return (TColor)((b1<<16)+(g1<<8)+r1);
}
// draw snr plot ------------------------------------------------------------
void __fastcall TMainForm::DrawSnr(TCanvas *c, int w, int h, int top,
	int index, int freq)
{
    static const TColor color[]={
        (TColor)0x00008000,(TColor)0x00008080,(TColor)0x00A000A0,
        (TColor)0x00800000,(TColor)0x00000080,(TColor)0x00808080
    };
    static const TColor color_sys[]={
        clGreen,(TColor)0xAAFF,clFuchsia,clBlue,clRed,clGray
    };
    UnicodeString s; 
    int i,j,k,l,n,x1,x2,y1,y2,y3,k1,hh=h-15,ww,www,snr[NFREQ+1],mask[6]={0};
    char id[16],sys[]="GREJCS",*q;
    
    trace(4,"DrawSnr: w=%d h=%d top=%d index=%d freq=%d\n",w,h,top,index,freq);
    
    c->Pen->Color=clSilver;
    for (snr[0]=MINSNR+10;snr[0]<MAXSNR;snr[0]+=10) {
        y1=top+hh-(snr[0]-MINSNR)*hh/(MAXSNR-MINSNR);
        c->MoveTo(3,y1); c->LineTo(w-13,y1);
        DrawText(c,w-9,y1,s.sprintf(L"%d",snr[0]),clGray,1);
    }
    y1=top+hh;
    TRect b(1,top,w-2,y1);
    c->Pen->Color=clGray;
    c->Brush->Style=bsClear;
    c->Rectangle(b);
    
    for (i=0;i<Nsat[index]&&i<MAXSAT;i++) {
        
        ww=(w-16)/Nsat[index];
        www=ww-2<8?ww-2:8;
        x1=i*(w-16)/Nsat[index]+ww/2;
        satno2id(Sat[index][i],id);
        l=(q=strchr(sys,id[0]))?(int)(q-sys):5;
        
        for (j=snr[0]=0;j<NFREQ;j++) {
            snr[j+1]=Snr[index][i][j];
            if ((freq&&freq==j+1)||((!freq||freq>NFREQ)&&snr[j+1]>snr[0])) {
                snr[0]=snr[j+1];
            }
        }
        for (j=0;j<NFREQ+2;j++) {
            k=j<NFREQ+1?j:0;
            y3=j<NFREQ+1?0:2;
            y2=y1-y3;
            if (snr[k]>0) y2-=(snr[k]-MINSNR)*hh/(MAXSNR-MINSNR)-y3;
            y2=y2<2?2:(y1<y2?y1:y2);
            
            TRect r1(x1,y1,x1+www,y2);
            if (j==0) {
                c->Brush->Style=bsSolid;
                c->Brush->Color=freq<NFREQ?SnrColor(snr[k]):color_sys[l];
                if (!Vsat[index][i]) c->Brush->Color=clSilver;
                c->Rectangle(r1);
            }
            else {
                c->Pen->Color=j<NFREQ+1?clSilver:clGray;
                c->Brush->Style=bsClear;
                c->Rectangle(r1);
            }
        }
        DrawText(c,x1+www/2,y1+6,(s=id+1),color[l],1);
        mask[l]=1;
    }
    for (i=n=0;i<6;i++) if (mask[i]) n++;
    for (i=j=0;i<6;i++) {
        if (!mask[i]) continue;
        sprintf(id,"%c",sys[i]);
        DrawText(c,w-15+8*(-n+j++),top+3,(s=id),color[i],0);
    }
}
// draw satellites in skyplot -----------------------------------------------
void __fastcall TMainForm::DrawSat(TCanvas *c, int w, int h, int x0, int y0,
    int index, int freq)
{
    static const TColor color_sys[]={
        clGreen,(TColor)0xAAFF,clFuchsia,clBlue,clRed,clGray
    };
    UnicodeString s;
    TPoint p(w/2,h/2);
    double r=MIN(w*0.95,h*0.95)/2,azel[MAXSAT*2],dop[4];
    int i,k,l,d,x[MAXSAT],y[MAXSAT],ns=0,f=!freq?0:freq-1;
    char id[16],sys[]="GREJCS",*q;
    
    trace(4,"DrawSat: w=%d h=%d index=%d freq=%d\n",w,h,index,freq);
    
    DrawSky(c,w,h,x0,y0);
    
    for (i=0,k=Nsat[index]-1;i<Nsat[index]&&i<MAXSAT;i++,k--) {
        if (El[index][k]<=0.0) continue;
        if (Vsat[index][k]) {
            azel[ns*2]=Az[index][k]; azel[1+ns*2]=El[index][k];
            ns++;
        }
        satno2id(Sat[index][k],id);
        l=(q=strchr(sys,id[0]))?(int)(q-sys):5;
        x[i]=(int)(p.x+r*(90-El[index][k]*R2D)/90*sin(Az[index][k]))+x0;
        y[i]=(int)(p.y-r*(90-El[index][k]*R2D)/90*cos(Az[index][k]))+y0;
        c->Pen->Color=clGray;
        c->Brush->Style=bsSolid;
        d=SATSIZE/2;
        c->Brush->Color=!Vsat[index][k]?clSilver:
                        (freq<NFREQ?SnrColor(Snr[index][k][f]):color_sys[l]);
        c->Ellipse(x[i]-d,y[i]-d,x[i]+d+1,y[i]+d+1);
        c->Brush->Style=bsClear;
        DrawText(c,x[i],y[i],s=id,clWhite,1);
    }
    c->Brush->Style=bsClear;
    dops(ns,azel,0.0,dop);
    DrawText(c,x0+3,y0+h-15,s.sprintf(L"# Sat:%2d",Nsat[index]),clGray,0);
    DrawText(c,x0+w-3,y0+h-15,s.sprintf(L"GDOP:%.1f",dop[0]),clGray,2);
}
// draw baseline plot -------------------------------------------------------
void __fastcall TMainForm::DrawBL(TCanvas *c, int w, int h)
{
    TColor color[]={clSilver,clGreen,CLORANGE,clFuchsia,clBlue,clRed,clTeal};
    UnicodeString s,label[]={"N","E","S","W"};
    TPoint p(w/2,h/2),p1,p2,pp;
    double r=MIN(w*0.95,h*0.95)/2;
    double *rr=SolRov+PSol*3,*rb=SolRef+PSol*3;
    double bl[3]={0},pos[3],enu[3],len=0.0,pitch=0.0,yaw=0.0;
    double cp,q;
    TColor col=clWhite;
    int i,d1=9,d2=16,d3=10,cy=0,sy=0,cya=0,sya=0,a,x1,x2,y1,y2,digit;
    
    trace(4,"DrawBL: w=%d h=%d\n",w,h);
    
    if (PMODE_DGPS<=PrcOpt.mode&&PrcOpt.mode<=PMODE_FIXED) {
        col=rtksvr.state&&SolStat[PSol]&SolCurrentStat?color[SolStat[PSol]]:clWhite;
        
        if (norm(rr,3)>0.0&&norm(rb,3)>0.0) {
            for (i=0;i<3;i++) bl[i]=rr[i]-rb[i];
        }
        if ((len=norm(bl,3))>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu);
            pitch=asin(enu[2]/len);
            yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
        }
    }
    if (len>=MINBLLEN) {
        cp =cos(pitch);
        cy =(int)((r-d1-d2/2)*cp*cos(yaw));
        sy =(int)((r-d1-d2/2)*cp*sin(yaw));
        cya=(int)(((r-d1-d2/2)*cp-d2/2-4)*cos(yaw));
        sya=(int)(((r-d1-d2/2)*cp-d2/2-4)*sin(yaw));
    }
    p1.x=p.x-sy; p1.y=p.y+cy; // base
    p2.x=p.x+sy; p2.y=p.y-cy; // rover
    
    c->Pen->Color=clGray;
    c->Ellipse(p.x-r,p.y-r,p.x+r+1,p.y+r+1);
    c->Pen->Color=clSilver;
    c->Ellipse(p.x-r+d1,p.y-r+d1,p.x+r-d1+1,p.y+r-d1+1);
    c->Brush->Style=bsSolid;
    
    pp=pitch<0.0?p2:p1;
    c->Pen->Color=clSilver;
    c->MoveTo(p.x,p.y); c->LineTo(pp.x,pp.y);
    if (pitch<0.0) {
        c->Brush->Color=clWhite;
        c->Ellipse(pp.x-d2/2,pp.y-d2/2,pp.x+d2/2+1,pp.y+d2/2+1);
        DrawArrow(c,p.x+sya,p.y-cya,d3,(int)(yaw*R2D),clSilver);
    }
    c->Brush->Color=col;
    c->Ellipse(pp.x-d2/2+2,pp.y-d2/2+2,pp.x+d2/2-1,pp.y+d2/2-1);
    c->Brush->Color=clWhite;
    for (a=0;a<360;a+=15) {
        q=a%45==0?r/2:r-d1;
        x1=(int)(r*sin(a*D2R));
        y1=(int)(r*cos(a*D2R));
        x2=(int)(q*sin(a*D2R));
        y2=(int)(q*cos(a*D2R));
        c->Pen->Color=clSilver;
        c->MoveTo(p.x+x1,p.y+y1); c->LineTo(p.x+x2,p.y+y2);
        if (a%90==0) DrawText(c,p.x+x1,p.y-y1,label[a/90],clGray,1);
    }
    pp=pitch>=0.0?p2:p1;
    c->Pen->Color=clGray;
    c->MoveTo(p.x,p.y); c->LineTo(pp.x,pp.y);
    if (pitch>=0.0) {
        c->Brush->Color=clWhite;
        c->Ellipse(pp.x-d2/2,pp.y-d2/2,pp.x+d2/2+1,pp.y+d2/2+1);
        DrawArrow(c,p.x+sya,p.y-cya,d3,(int)(yaw*R2D),clGray);
    }
    c->Brush->Color=col;
    c->Ellipse(pp.x-d2/2+2,pp.y-d2/2+2,pp.x+d2/2-1,pp.y+d2/2-1);
    c->Brush->Color=clWhite;
    digit=len<10.0?3:(len<100.0?2:(len<1000.0?1:0));
    DrawText(c,p.x,p.y ,s.sprintf(L"%.*f m",digit,len),clGray,1);
    DrawText(c,3,  h-15,s.sprintf(L"Y: %.1f%c",yaw*R2D,CHARDEG),clGray,0);
    DrawText(c,w-3,h-15,s.sprintf(L"P: %.1f%c",pitch*R2D,CHARDEG),clGray,2);
}
// draw skyplot -------------------------------------------------------------
void __fastcall TMainForm::DrawSky(TCanvas *c, int w, int h, int x0, int y0)
{
    UnicodeString label[]={"N","E","S","W"};
    TPoint p(x0+w/2,y0+h/2);
    double r=MIN(w*0.95,h*0.95)/2;
    int a,e,d,x,y;
    
    c->Brush->Color=clWhite;
    c->Brush->Style=bsSolid;
    for (e=0;e<90;e+=30) {
        d=(int)(r*(90-e)/90);
        c->Pen->Color=e==0?clGray:clSilver;
        c->Ellipse(p.x-d,p.y-d,p.x+d+1,p.y+d+1);
    }
    for (a=0;a<360;a+=45) {
        x=(int)(r*sin(a*D2R));
        y=(int)(r*cos(a*D2R));
        c->Pen->Color=clSilver;
        c->MoveTo(p.x,p.y); c->LineTo(p.x+x,p.y-y);
        if (a%90==0) DrawText(c,p.x+x,p.y-y,label[a/90],clGray,1);
    }
}
// draw text ----------------------------------------------------------------
void __fastcall TMainForm::DrawText(TCanvas *c, int x, int y, UnicodeString s,
    TColor color, int align)
{
    TSize off=c->TextExtent(s);
    c->Font->Charset=ANSI_CHARSET;
    if (align==1) {x-=off.cx/2; y-=off.cy/2;} else if (align==2) x-=off.cx;
    c->Font->Color=color;
    c->TextOut(x,y,s);
}
// draw arrow ---------------------------------------------------------------
void __fastcall TMainForm::DrawArrow(TCanvas *c, int x, int y, int siz,
    int ang, TColor color)
{
    TPoint p1[4],p2[4];
    int i;
    
    p1[0].x=0; p1[1].x=siz/2; p1[2].x=-siz/2; p1[3].x=0;
    p1[0].y=siz/2; p1[1].y=p1[2].y=-siz/2; p1[3].y=siz/2;
    
    for (i=0;i<4;i++) {
        p2[i].x=x+(int)(p1[i].x*cos(-ang*D2R)-p1[i].y*sin(-ang*D2R)+0.5);
        p2[i].y=y-(int)(p1[i].x*sin(-ang*D2R)+p1[i].y*cos(-ang*D2R)+0.5);
    }
    c->Brush->Style=bsSolid;
    c->Brush->Color=color;
    c->Pen->Color=color;
    c->Polygon(p2,3);
}
// open monitor port --------------------------------------------------------
void __fastcall TMainForm::OpenMoniPort(int port)
{
    AnsiString s;
    int i;
    char path[64];
    
    if (port<=0) return;
    
    trace(3,"OpenMoniPort: port=%d\n",port);
    
    for (i=0;i<=MAXPORTOFF;i++) {
        
        sprintf(path,":%d",port+i);
        
        if (stropen(&monistr,STR_TCPSVR,STR_MODE_RW,path)) {
            strsettimeout(&monistr,TimeoutTime,ReconTime);
            if (i>0) Caption=s.sprintf("%s ver.%s (%d)",PRGNAME,VER_RTKLIB,i+1);
            OpenPort=MoniPort+i;
            return;
        }
    }
    ShowMessage(s.sprintf("monitor port %d-%d open error",port,port+MAXPORTOFF));
    OpenPort=0;
}
// initialize solution buffer -----------------------------------------------
void __fastcall TMainForm::InitSolBuff(void)
{
    double ep[]={2000,1,1,0,0,0};
    int i,j;
    
    trace(3,"InitSolBuff\n");
    
    delete [] Time;   delete [] SolStat; delete [] Nvsat;  delete [] SolRov;
    delete [] SolRef; delete [] Qr;      delete [] VelRov; delete [] Age;
    delete [] Ratio;
    
    if (SolBuffSize<=0) SolBuffSize=1;
    Time   =new gtime_t[SolBuffSize];
    SolStat=new int[SolBuffSize];
    Nvsat  =new int[SolBuffSize];
    SolRov =new double[SolBuffSize*3];
    SolRef =new double[SolBuffSize*3];
    VelRov =new double[SolBuffSize*3];
    Qr     =new double[SolBuffSize*9];
    Age    =new double[SolBuffSize];
    Ratio  =new double[SolBuffSize];
    PSol=PSolS=PSolE=0;
    for (i=0;i<SolBuffSize;i++) {
        Time[i]=epoch2time(ep);
        SolStat[i]=Nvsat[i]=0;
        for (j=0;j<3;j++) SolRov[j+i*3]=SolRef[j+i*3]=VelRov[j+i*3]=0.0;
        for (j=0;j<9;j++) Qr[j+i*9]=0.0;
        Age[i]=Ratio[i]=0.0;
    }
    ScbSol->Max=0; ScbSol->Position=0;
}
// save log file ------------------------------------------------------------
void __fastcall TMainForm::SaveLog(void)
{
    AnsiString SaveDialog_FileName=SaveDialog->FileName;
    FILE *fp;
    int posf[]={SOLF_LLH,SOLF_LLH,SOLF_XYZ,SOLF_ENU,SOLF_ENU,SOLF_LLH};
    solopt_t opt;
    double  ep[6],pos[3];
    char file[1024];
    
    trace(3,"SaveLog\n");
    
    time2epoch(timeget(),ep);
    sprintf(file,"rtk_%04.0f%02.0f%02.0f%02.0f%02.0f%02.0f.txt",
            ep[0],ep[1],ep[2],ep[3],ep[4],ep[5]);
    SaveDialog->FileName=file;
    if (!SaveDialog->Execute()) return;
    if (!(fp=fopen(SaveDialog_FileName.c_str(),"wt"))) {
        Message->Caption="log file open error";
        Message->Parent->Hint=Message->Caption;
        return;
    }
    opt=SolOpt;
    opt.posf=posf[SolType];
    if (SolOpt.outhead) {
        fprintf(fp,"%% program   : %s ver.%s\n",PRGNAME,VER_RTKLIB);
        if (PrcOpt.mode==PMODE_DGPS||PrcOpt.mode==PMODE_KINEMA||
            PrcOpt.mode==PMODE_STATIC) {
            ecef2pos(PrcOpt.rb,pos);
            fprintf(fp,"%% ref pos   :%13.9f %14.9f %10.4f\n",pos[0]*R2D,
                    pos[1]*R2D,pos[2]);
        }
        fprintf(fp,"%%\n");
    }
    outsolhead(fp,&opt);
    fclose(fp);
}
// load navigation data -----------------------------------------------------
void __fastcall TMainForm::LoadNav(nav_t *nav)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString str,s;
    eph_t eph0={0};
    char buff[2049],id[32],*p;
    int i;
    
    trace(3,"LoadNav\n");
    
    for (i=0;i<MAXSAT;i++) {
        if ((str=ini->ReadString("navi",s.sprintf("eph_%02d",i),""))=="") continue;
        nav->eph[i]=eph0;
        strcpy(buff,str.c_str());
        if (!(p=strchr(buff,','))) continue;
        *p='\0';
        if (!(nav->eph[i].sat=satid2no(buff))) continue;
        sscanf(p+1,"%d,%d,%d,%d,%ld,%ld,%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d",
               &nav->eph[i].iode,
               &nav->eph[i].iodc,
               &nav->eph[i].sva ,
               &nav->eph[i].svh ,
               &nav->eph[i].toe.time,
               &nav->eph[i].toc.time,
               &nav->eph[i].ttr.time,
               &nav->eph[i].A   ,
               &nav->eph[i].e   ,
               &nav->eph[i].i0  ,
               &nav->eph[i].OMG0,
               &nav->eph[i].omg ,
               &nav->eph[i].M0  ,
               &nav->eph[i].deln,
               &nav->eph[i].OMGd,
               &nav->eph[i].idot,
               &nav->eph[i].crc ,
               &nav->eph[i].crs ,
               &nav->eph[i].cuc ,
               &nav->eph[i].cus ,
               &nav->eph[i].cic ,
               &nav->eph[i].cis ,
               &nav->eph[i].toes,
               &nav->eph[i].fit ,
               &nav->eph[i].f0  ,
               &nav->eph[i].f1  ,
               &nav->eph[i].f2  ,
               &nav->eph[i].tgd[0],
               &nav->eph[i].code,
               &nav->eph[i].flag);
    }
    str=ini->ReadString("navi","ion","");
    for (i=0;i<8;i++) nav->ion_gps[i]=0.0;
    sscanf(str.c_str(),"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
           nav->ion_gps  ,nav->ion_gps+1,nav->ion_gps+2,nav->ion_gps+3,
           nav->ion_gps+4,nav->ion_gps+5,nav->ion_gps+6,nav->ion_gps+7);
    str=ini->ReadString("navi","utc","");
    
    for (i=0;i<4;i++) nav->utc_gps[i]=0.0;
    sscanf(str.c_str(),"%lf,%lf,%lf,%lf",
           nav->utc_gps,nav->utc_gps+1,nav->utc_gps+2,nav->utc_gps+3);
    
    nav->leaps=ini->ReadInteger("navi","leaps",0);
    
    delete ini;
}
// save navigation data -----------------------------------------------------
void __fastcall TMainForm::SaveNav(nav_t *nav)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString str,s;
    char id[32];
    int i;
    
    trace(3,"SaveNav\n");
    
    for (i=0;i<MAXSAT;i++) {
        if (nav->eph[i].ttr.time==0) continue;
        str="";
        satno2id(nav->eph[i].sat,id);
        str=str+s.sprintf("%s,",id);
        str=str+s.sprintf("%d,",nav->eph[i].iode);
        str=str+s.sprintf("%d,",nav->eph[i].iodc);
        str=str+s.sprintf("%d,",nav->eph[i].sva);
        str=str+s.sprintf("%d,",nav->eph[i].svh);
        str=str+s.sprintf("%d,",(int)nav->eph[i].toe.time);
        str=str+s.sprintf("%d,",(int)nav->eph[i].toc.time);
        str=str+s.sprintf("%d,",(int)nav->eph[i].ttr.time);
        str=str+s.sprintf("%.14E,",nav->eph[i].A);
        str=str+s.sprintf("%.14E,",nav->eph[i].e);
        str=str+s.sprintf("%.14E,",nav->eph[i].i0);
        str=str+s.sprintf("%.14E,",nav->eph[i].OMG0);
        str=str+s.sprintf("%.14E,",nav->eph[i].omg);
        str=str+s.sprintf("%.14E,",nav->eph[i].M0);
        str=str+s.sprintf("%.14E,",nav->eph[i].deln);
        str=str+s.sprintf("%.14E,",nav->eph[i].OMGd);
        str=str+s.sprintf("%.14E,",nav->eph[i].idot);
        str=str+s.sprintf("%.14E,",nav->eph[i].crc);
        str=str+s.sprintf("%.14E,",nav->eph[i].crs);
        str=str+s.sprintf("%.14E,",nav->eph[i].cuc);
        str=str+s.sprintf("%.14E,",nav->eph[i].cus);
        str=str+s.sprintf("%.14E,",nav->eph[i].cic);
        str=str+s.sprintf("%.14E,",nav->eph[i].cis);
        str=str+s.sprintf("%.14E,",nav->eph[i].toes);
        str=str+s.sprintf("%.14E,",nav->eph[i].fit);
        str=str+s.sprintf("%.14E,",nav->eph[i].f0);
        str=str+s.sprintf("%.14E,",nav->eph[i].f1);
        str=str+s.sprintf("%.14E,",nav->eph[i].f2);
        str=str+s.sprintf("%.14E,",nav->eph[i].tgd[0]);
        str=str+s.sprintf("%d,",nav->eph[i].code);
        str=str+s.sprintf("%d,",nav->eph[i].flag);
        ini->WriteString("navi",s.sprintf("eph_%02d",i),str);
    }
    str="";
    for (i=0;i<8;i++) str=str+s.sprintf("%.14E,",nav->ion_gps[i]);
    ini->WriteString("navi","ion",str);
    
    str="";
    for (i=0;i<4;i++) str=str+s.sprintf("%.14E,",nav->utc_gps[i]);
    ini->WriteString("navi","utc",str);
    
    ini->WriteInteger("navi","leaps",nav->leaps);
    
    delete ini;
}
// set tray icon ------------------------------------------------------------
void __fastcall TMainForm::SetTrayIcon(int index)
{
    TIcon *icon=new TIcon;
    ImageList->GetIcon(index,icon);
    TrayIcon->Icon=icon;
    delete icon;
}
// load option from ini file ------------------------------------------------
void __fastcall TMainForm::LoadOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s;
    int i,j,no,strno[]={0,1,6,2,3,4,5,7};
    char *p;
    
    trace(3,"LoadOpt\n");
    
    for (i=0;i<8;i++) {
        no=strno[i];
        StreamC[i]=ini->ReadInteger("stream",s.sprintf("streamc%d",no),0);
        Stream [i]=ini->ReadInteger("stream",s.sprintf("stream%d", no),0);
        Format [i]=ini->ReadInteger("stream",s.sprintf("format%d", no),0);
        for (j=0;j<4;j++) {
            Paths[i][j]=ini->ReadString("stream",s.sprintf("path_%d_%d",no,j),"");
        }
    }
    for (i=0;i<3;i++) {
        RcvOpt [i]=ini->ReadString("stream",s.sprintf("rcvopt%d",i+1),"");
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        Cmds[i][j]=ini->ReadString("serial",s.sprintf("cmd_%d_%d",i,j),"");
        CmdEna[i][j]=ini->ReadInteger("serial",s.sprintf("cmdena_%d_%d",i,j),0);
        for (p=Cmds[i][j].c_str();*p;p++) {
            if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
        }
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        CmdsTcp[i][j]=ini->ReadString("tcpip",s.sprintf("cmd_%d_%d",i,j),"");
        CmdEnaTcp[i][j]=ini->ReadInteger("tcpip",s.sprintf("cmdena_%d_%d",i,j),0);
        for (p=CmdsTcp[i][j].c_str();*p;p++) {
            if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
        }
    }
    PrcOpt.mode     =ini->ReadInteger("prcopt", "mode",            0);
    PrcOpt.nf       =ini->ReadInteger("prcopt", "nf",              2);
    PrcOpt.elmin    =ini->ReadFloat  ("prcopt", "elmin",    15.0*D2R);
    PrcOpt.snrmask.ena[0]=ini->ReadInteger("prcopt","snrmask_ena1",0);
    PrcOpt.snrmask.ena[1]=ini->ReadInteger("prcopt","snrmask_ena2",0);
    for (i=0;i<NFREQ;i++) for (j=0;j<9;j++) {
        PrcOpt.snrmask.mask[i][j]=
            ini->ReadFloat("prcopt",s.sprintf("snrmask_%d_%d",i+1,j+1),0.0);
    }
    PrcOpt.dynamics =ini->ReadInteger("prcopt", "dynamics",        0);
    PrcOpt.tidecorr =ini->ReadInteger("prcopt", "tidecorr",        0);
    PrcOpt.modear   =ini->ReadInteger("prcopt", "modear",          1);
    PrcOpt.glomodear=ini->ReadInteger("prcopt", "glomodear",       0);
    PrcOpt.bdsmodear=ini->ReadInteger("prcopt", "bdsmodear",       0);
    PrcOpt.maxout   =ini->ReadInteger("prcopt", "maxout",          5);
    PrcOpt.minlock  =ini->ReadInteger("prcopt", "minlock",         0);
    PrcOpt.minfix   =ini->ReadInteger("prcopt", "minfix",         10);
    PrcOpt.ionoopt  =ini->ReadInteger("prcopt", "ionoopt",IONOOPT_BRDC);
    PrcOpt.tropopt  =ini->ReadInteger("prcopt", "tropopt",TROPOPT_SAAS);
    PrcOpt.sateph   =ini->ReadInteger("prcopt", "ephopt",  EPHOPT_BRDC);
    PrcOpt.niter    =ini->ReadInteger("prcopt", "niter",           1);
    PrcOpt.eratio[0]=ini->ReadFloat  ("prcopt", "eratio0",     100.0);
    PrcOpt.eratio[1]=ini->ReadFloat  ("prcopt", "eratio1",     100.0);
    PrcOpt.err[1]   =ini->ReadFloat  ("prcopt", "err1",        0.003);
    PrcOpt.err[2]   =ini->ReadFloat  ("prcopt", "err2",        0.003);
    PrcOpt.err[3]   =ini->ReadFloat  ("prcopt", "err3",          0.0);
    PrcOpt.err[4]   =ini->ReadFloat  ("prcopt", "err4",          1.0);
    PrcOpt.prn[0]   =ini->ReadFloat  ("prcopt", "prn0",         1E-4);
    PrcOpt.prn[1]   =ini->ReadFloat  ("prcopt", "prn1",         1E-3);
    PrcOpt.prn[2]   =ini->ReadFloat  ("prcopt", "prn2",         1E-4);
    PrcOpt.prn[3]   =ini->ReadFloat  ("prcopt", "prn3",         10.0);
    PrcOpt.prn[4]   =ini->ReadFloat  ("prcopt", "prn4",         10.0);
    PrcOpt.sclkstab =ini->ReadFloat  ("prcopt", "sclkstab",    5E-12);
    PrcOpt.thresar[0]=ini->ReadFloat ("prcopt", "thresar",       3.0);
    PrcOpt.elmaskar =ini->ReadFloat  ("prcopt", "elmaskar",      0.0);
    PrcOpt.elmaskhold=ini->ReadFloat ("prcopt", "elmaskhold",    0.0);
    PrcOpt.thresslip=ini->ReadFloat  ("prcopt", "thresslip",    0.05);
    PrcOpt.maxtdiff =ini->ReadFloat  ("prcopt", "maxtdiff",     30.0);
    PrcOpt.maxgdop  =ini->ReadFloat  ("prcopt", "maxgdop",      30.0);
    PrcOpt.maxinno  =ini->ReadFloat  ("prcopt", "maxinno",      30.0);
    PrcOpt.syncsol  =ini->ReadInteger("prcopt", "syncsol",         0);
    ExSats          =ini->ReadString ("prcopt", "exsats",         "");
    PrcOpt.navsys   =ini->ReadInteger("prcopt", "navsys",    SYS_GPS);
    PrcOpt.posopt[0]=ini->ReadInteger("prcopt", "posopt1",         0);
    PrcOpt.posopt[1]=ini->ReadInteger("prcopt", "posopt2",         0);
    PrcOpt.posopt[2]=ini->ReadInteger("prcopt", "posopt3",         0);
    PrcOpt.posopt[3]=ini->ReadInteger("prcopt", "posopt4",         0);
    PrcOpt.posopt[4]=ini->ReadInteger("prcopt", "posopt5",         0);
    
    BaselineC       =ini->ReadInteger("prcopt", "baselinec",       0);
    Baseline[0]     =ini->ReadFloat  ("prcopt", "baseline1",     0.0);
    Baseline[1]     =ini->ReadFloat  ("prcopt", "baseline2",     0.0);
    
    SolOpt.posf     =ini->ReadInteger("solopt", "posf",            0);
    SolOpt.times    =ini->ReadInteger("solopt", "times",           0);
    SolOpt.timef    =ini->ReadInteger("solopt", "timef",           1);
    SolOpt.timeu    =ini->ReadInteger("solopt", "timeu",           3);
    SolOpt.degf     =ini->ReadInteger("solopt", "degf",            0);
    s=ini->ReadString("solopt","sep"," ");
    strcpy(SolOpt.sep,s.c_str());
    SolOpt.outhead  =ini->ReadInteger("solopt", "outhead",         0);
    SolOpt.outopt   =ini->ReadInteger("solopt", "outopt",          0);
    SolOpt.datum    =ini->ReadInteger("solopt", "datum",           0);
    SolOpt.height   =ini->ReadInteger("solopt", "height",          0);
    SolOpt.geoid    =ini->ReadInteger("solopt", "geoid",           0);
    SolOpt.nmeaintv[0]=ini->ReadFloat("solopt", "nmeaintv1",     0.0);
    SolOpt.nmeaintv[1]=ini->ReadFloat("solopt", "nmeaintv2",     0.0);
    DebugStatusF    =ini->ReadInteger("setting","debugstatus",     0);
    DebugTraceF     =ini->ReadInteger("setting","debugtrace",      0);
    
    RovPosTypeF     =ini->ReadInteger("setting","rovpostype",      0);
    RefPosTypeF     =ini->ReadInteger("setting","refpostype",      0);
    RovAntPcvF      =ini->ReadInteger("setting","rovantpcv",       0);
    RefAntPcvF      =ini->ReadInteger("setting","refantpcv",       0);
    RovAntF         =ini->ReadString ("setting","rovant",         "");
    RefAntF         =ini->ReadString ("setting","refant",         "");
    SatPcvFileF     =ini->ReadString ("setting","satpcvfile",     "");
    AntPcvFileF     =ini->ReadString ("setting","antpcvfile",     "");
    StaPosFileF     =ini->ReadString ("setting","staposfile",     "");
    GeoidDataFileF  =ini->ReadString ("setting","geoiddatafile",  "");
    DCBFileF        =ini->ReadString ("setting","dcbfile",        "");
    EOPFileF        =ini->ReadString ("setting","eopfile",        "");
    TLEFileF        =ini->ReadString ("setting","tlefile",        "");
    TLESatFileF     =ini->ReadString ("setting","tlesatfile",     "");
    LocalDirectory  =ini->ReadString ("setting","localdirectory","C:\\Temp");
    
    SvrCycle        =ini->ReadInteger("setting","svrcycle",       10);
    TimeoutTime     =ini->ReadInteger("setting","timeouttime", 10000);
    ReconTime       =ini->ReadInteger("setting","recontime",   10000);
    NmeaCycle       =ini->ReadInteger("setting","nmeacycle",    5000);
    SvrBuffSize     =ini->ReadInteger("setting","svrbuffsize", 32768);
    SolBuffSize     =ini->ReadInteger("setting","solbuffsize",  1000);
    SavedSol        =ini->ReadInteger("setting","savedsol",      100);
    NavSelect       =ini->ReadInteger("setting","navselect",       0);
    PrcOpt.sbassatsel=ini->ReadInteger("setting","sbassat",        0);
    DgpsCorr        =ini->ReadInteger("setting","dgpscorr",        0);
    SbasCorr        =ini->ReadInteger("setting","sbascorr",        0);
    
    NmeaReq         =ini->ReadInteger("setting","nmeareq",         0);
    InTimeTag       =ini->ReadInteger("setting","intimetag",       0);
    InTimeSpeed     =ini->ReadString ("setting","intimespeed",  "x1");
    InTimeStart     =ini->ReadString ("setting","intimestart",   "0");
    OutTimeTag      =ini->ReadInteger("setting","outtimetag",      0);
    OutAppend       =ini->ReadInteger("setting","outappend",       0);
    OutSwapInterval =ini->ReadString ("setting","outswapinterval","");
    LogTimeTag      =ini->ReadInteger("setting","logtimetag",      0);
    LogAppend       =ini->ReadInteger("setting","logappend",       0);
    LogSwapInterval =ini->ReadString ("setting","logswapinterval","");
    NmeaPos[0]      =ini->ReadFloat  ("setting","nmeapos1",      0.0);
    NmeaPos[1]      =ini->ReadFloat  ("setting","nmeapos2",      0.0);
    FileSwapMargin  =ini->ReadInteger("setting","fswapmargin",    30);
    
    TimeSys         =ini->ReadInteger("setting","timesys",         0);
    SolType         =ini->ReadInteger("setting","soltype",         0);
    PlotType1       =ini->ReadInteger("setting","plottype",        0);
    PlotType2       =ini->ReadInteger("setting","plottype2",       0);
    PanelMode       =ini->ReadInteger("setting","panelmode",       0);
    ProxyAddr       =ini->ReadString ("setting","proxyaddr",      "");
    MoniPort        =ini->ReadInteger("setting","moniport",DEFAULTPORT);
    PanelStack      =ini->ReadInteger("setting","panelstack",      0);
    
    for (i=0;i<3;i++) {
        RovAntDel[i]=ini->ReadFloat("setting",s.sprintf("rovantdel_%d",i),0.0);
        RefAntDel[i]=ini->ReadFloat("setting",s.sprintf("refantdel_%d",i),0.0);
        RovPos   [i]=ini->ReadFloat("setting",s.sprintf("rovpos_%d",   i),0.0);
        RefPos   [i]=ini->ReadFloat("setting",s.sprintf("refpos_%d",   i),0.0);
    }
    for (i=0;i<10;i++) {
        History[i]=ini->ReadString ("tcpopt",s.sprintf("history%d", i),"");
    }
    for (i=0;i<10;i++) {
        MntpHist[i]=ini->ReadString("tcpopt",s.sprintf("mntphist%d",i),"");
    }
    NMapPnt        =ini->ReadInteger("mapopt","nmappnt",0);
    for (i=0;i<NMapPnt;i++) {
        PntName[i]=ini->ReadString("mapopt",s.sprintf("pntname%d",i+1),"");
        AnsiString pos=ini->ReadString("mapopt",s.sprintf("pntpos%d",i+1),"0,0,0");
        PntPos[i][0]=PntPos[i][1]=PntPos[i][2]=0.0;
        sscanf(pos.c_str(),"%lf,%lf,%lf",PntPos[i],PntPos[i]+1,PntPos[i]+2);
    }
    PosFont->Name=ini->ReadString ("setting","posfontname",POSFONTNAME);
    PosFont->Size=ini->ReadInteger("setting","posfontsize",POSFONTSIZE);
    PosFont->Color=(TColor)ini->ReadInteger("setting","posfontcolor",(int)clBlack);
    if (ini->ReadInteger("setting","posfontbold",  0)) PosFont->Style=PosFont->Style<<fsBold;
    if (ini->ReadInteger("setting","posfontitalic",0)) PosFont->Style=PosFont->Style<<fsItalic;
    PosFont->Charset=ANSI_CHARSET;
    
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    
    UpdatePanel();
    
    if (PanelStack==0) {
        Panel21 ->Width=ini->ReadInteger("window","splitpos" ,180);
        Panel222->Width=ini->ReadInteger("window","splitpos2",180);
    }
    else {
        Panel21 ->Height=ini->ReadInteger("window","splitpos" ,180);
        Panel222->Height=ini->ReadInteger("window","splitpos2",180);
    }
    Width         =ini->ReadInteger("window","width",   388);
    Height        =ini->ReadInteger("window","height",  284);
    delete ini;
}
// save option to ini file --------------------------------------------------
void __fastcall TMainForm::SaveOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s;
    int i,j,no,strno[]={0,1,6,2,3,4,5,7};
    char *p;
    
    trace(3,"SaveOpt\n");
    
    for (i=0;i<8;i++) {
        no=strno[i];
        ini->WriteInteger("stream",s.sprintf("streamc%d",no),StreamC[i]);
        ini->WriteInteger("stream",s.sprintf("stream%d" ,no),Stream [i]);
        ini->WriteInteger("stream",s.sprintf("format%d" ,no),Format [i]);
        for (j=0;j<4;j++) {
            ini->WriteString("stream",s.sprintf("path_%d_%d",no,j),Paths[i][j]);
        }
    }
    for (i=0;i<3;i++) {
        ini->WriteString("stream",s.sprintf("rcvopt%d",i+1),RcvOpt[i]);
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        for (p=Cmds[i][j].c_str();*p;p++) {
            if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
        }
        ini->WriteString ("serial",s.sprintf("cmd_%d_%d"   ,i,j),Cmds  [i][j]);
        ini->WriteInteger("serial",s.sprintf("cmdena_%d_%d",i,j),CmdEna[i][j]);
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        for (p=CmdsTcp[i][j].c_str();*p;p++) {
            if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
        }
        ini->WriteString ("tcpip",s.sprintf("cmd_%d_%d"   ,i,j),CmdsTcp  [i][j]);
        ini->WriteInteger("tcpip",s.sprintf("cmdena_%d_%d",i,j),CmdEnaTcp[i][j]);
    }
    ini->WriteInteger("prcopt", "mode",       PrcOpt.mode        );
    ini->WriteInteger("prcopt", "nf",         PrcOpt.nf          );
    ini->WriteFloat  ("prcopt", "elmin",      PrcOpt.elmin       );
    ini->WriteFloat  ("prcopt", "snrmask_ena1",PrcOpt.snrmask.ena[0]);
    ini->WriteFloat  ("prcopt", "snrmask_ena2",PrcOpt.snrmask.ena[1]);
    for (i=0;i<NFREQ;i++) for (j=0;j<9;j++) {
        ini->WriteFloat("prcopt",s.sprintf("snrmask_%d_%d",i+1,j+1),
                        PrcOpt.snrmask.mask[i][j]);
    }
    ini->WriteInteger("prcopt", "dynamics",   PrcOpt.dynamics    );
    ini->WriteInteger("prcopt", "tidecorr",   PrcOpt.tidecorr    );
    ini->WriteInteger("prcopt", "modear",     PrcOpt.modear      );
    ini->WriteInteger("prcopt", "glomodear",  PrcOpt.glomodear   );
    ini->WriteInteger("prcopt", "bdsmodear",  PrcOpt.bdsmodear   );
    ini->WriteInteger("prcopt", "maxout",     PrcOpt.maxout      );
    ini->WriteInteger("prcopt", "minlock",    PrcOpt.minlock     );
    ini->WriteInteger("prcopt", "minfix",     PrcOpt.minfix      );
    ini->WriteInteger("prcopt", "ionoopt",    PrcOpt.ionoopt     );
    ini->WriteInteger("prcopt", "tropopt",    PrcOpt.tropopt     );
    ini->WriteInteger("prcopt", "ephopt",     PrcOpt.sateph      );
    ini->WriteInteger("prcopt", "niter",      PrcOpt.niter       );
    ini->WriteFloat  ("prcopt", "eratio0",    PrcOpt.eratio[0]   );
    ini->WriteFloat  ("prcopt", "eratio1",    PrcOpt.eratio[1]   );
    ini->WriteFloat  ("prcopt", "err1",       PrcOpt.err[1]      );
    ini->WriteFloat  ("prcopt", "err2",       PrcOpt.err[2]      );
    ini->WriteFloat  ("prcopt", "err3",       PrcOpt.err[3]      );
    ini->WriteFloat  ("prcopt", "err4",       PrcOpt.err[4]      );
    ini->WriteFloat  ("prcopt", "prn0",       PrcOpt.prn[0]      );
    ini->WriteFloat  ("prcopt", "prn1",       PrcOpt.prn[1]      );
    ini->WriteFloat  ("prcopt", "prn2",       PrcOpt.prn[2]      );
    ini->WriteFloat  ("prcopt", "prn3",       PrcOpt.prn[3]      );
    ini->WriteFloat  ("prcopt", "prn4",       PrcOpt.prn[4]      );
    ini->WriteFloat  ("prcopt", "sclkstab",   PrcOpt.sclkstab    );
    ini->WriteFloat  ("prcopt", "thresar",    PrcOpt.thresar[0]  );
    ini->WriteFloat  ("prcopt", "elmaskar",   PrcOpt.elmaskar    );
    ini->WriteFloat  ("prcopt", "elmaskhold", PrcOpt.elmaskhold  );
    ini->WriteFloat  ("prcopt", "thresslip",  PrcOpt.thresslip   );
    ini->WriteFloat  ("prcopt", "maxtdiff",   PrcOpt.maxtdiff    );
    ini->WriteFloat  ("prcopt", "maxgdop",    PrcOpt.maxgdop     );
    ini->WriteFloat  ("prcopt", "maxinno",    PrcOpt.maxinno     );
    ini->WriteInteger("prcopt", "syncsol",    PrcOpt.syncsol     );
    ini->WriteString ("prcopt", "exsats",     ExSats             );
    ini->WriteInteger("prcopt", "navsys",     PrcOpt.navsys      );
    ini->WriteInteger("prcopt", "posopt1",    PrcOpt.posopt[0]   );
    ini->WriteInteger("prcopt", "posopt2",    PrcOpt.posopt[1]   );
    ini->WriteInteger("prcopt", "posopt3",    PrcOpt.posopt[2]   );
    ini->WriteInteger("prcopt", "posopt4",    PrcOpt.posopt[3]   );
    ini->WriteInteger("prcopt", "posopt5",    PrcOpt.posopt[4]   );
    
    ini->WriteFloat  ("prcopt", "baselinec",  BaselineC          );
    ini->WriteFloat  ("prcopt", "baseline1",  Baseline[0]        );
    ini->WriteFloat  ("prcopt", "baseline2",  Baseline[1]        );
    
    ini->WriteInteger("solopt", "posf",       SolOpt.posf        );
    ini->WriteInteger("solopt", "times",      SolOpt.times       );
    ini->WriteInteger("solopt", "timef",      SolOpt.timef       );
    ini->WriteInteger("solopt", "timeu",      SolOpt.timeu       );
    ini->WriteInteger("solopt", "degf",       SolOpt.degf        );
    ini->WriteString ("solopt", "sep",        SolOpt.sep         );
    ini->WriteInteger("solopt", "outhead",    SolOpt.outhead     );
    ini->WriteInteger("solopt", "outopt",     SolOpt.outopt      );
    ini->WriteInteger("solopt", "datum",      SolOpt.datum       );
    ini->WriteInteger("solopt", "height",     SolOpt.height      );
    ini->WriteInteger("solopt", "geoid",      SolOpt.geoid       );
    ini->WriteFloat  ("solopt", "nmeaintv1",  SolOpt.nmeaintv[0] );
    ini->WriteFloat  ("solopt", "nmeaintv2",  SolOpt.nmeaintv[1] );
    ini->WriteInteger("setting","debugstatus",DebugStatusF       );
    ini->WriteInteger("setting","debugtrace", DebugTraceF        );
    
    ini->WriteInteger("setting","rovpostype", RovPosTypeF        );
    ini->WriteInteger("setting","refpostype", RefPosTypeF        );
    ini->WriteInteger("setting","rovantpcv",  RovAntPcvF         );
    ini->WriteInteger("setting","refantpcv",  RefAntPcvF         );
    ini->WriteString ("setting","rovant",     RovAntF            );
    ini->WriteString ("setting","refant",     RefAntF            );
    ini->WriteString ("setting","satpcvfile", SatPcvFileF        );
    ini->WriteString ("setting","antpcvfile", AntPcvFileF        );
    ini->WriteString ("setting","staposfile", StaPosFileF        );
    ini->WriteString ("setting","geoiddatafile",GeoidDataFileF   );
    ini->WriteString ("setting","dcbfile",    DCBFileF           );
    ini->WriteString ("setting","eopfile",    EOPFileF           );
    ini->WriteString ("setting","tlefile",    TLEFileF           );
    ini->WriteString ("setting","tlesatfile", TLESatFileF        );
    ini->WriteString ("setting","localdirectory",LocalDirectory  );
    
    ini->WriteInteger("setting","svrcycle",   SvrCycle           );
    ini->WriteInteger("setting","timeouttime",TimeoutTime        );
    ini->WriteInteger("setting","recontime",  ReconTime          );
    ini->WriteInteger("setting","nmeacycle",  NmeaCycle          );
    ini->WriteInteger("setting","svrbuffsize",SvrBuffSize        );
    ini->WriteInteger("setting","solbuffsize",SolBuffSize        );
    ini->WriteInteger("setting","savedsol",   SavedSol           );
    ini->WriteInteger("setting","navselect",  NavSelect          );
    ini->WriteInteger("setting","sbassat",    PrcOpt.sbassatsel  );
    ini->WriteInteger("setting","dgpscorr",   DgpsCorr           );
    ini->WriteInteger("setting","sbascorr",   SbasCorr           );
    
    ini->WriteInteger("setting","nmeareq",    NmeaReq            );
    ini->WriteInteger("setting","intimetag",  InTimeTag          );
    ini->WriteString ("setting","intimespeed",InTimeSpeed        );
    ini->WriteString ("setting","intimestart",InTimeStart        );
    ini->WriteInteger("setting","outtimetag", OutTimeTag         );
    ini->WriteInteger("setting","outappend",  OutAppend          );
    ini->WriteString ("setting","outswapinterval",OutSwapInterval);
    ini->WriteInteger("setting","logtimetag", LogTimeTag         );
    ini->WriteInteger("setting","logappend",  LogAppend          );
    ini->WriteString ("setting","logswapinterval",LogSwapInterval);
    ini->WriteFloat  ("setting","nmeapos1",   NmeaPos[0]         );
    ini->WriteFloat  ("setting","nmeapos2",   NmeaPos[1]         );
    ini->WriteInteger("setting","fswapmargin",FileSwapMargin     );
    
    ini->WriteInteger("setting","timesys",    TimeSys            );
    ini->WriteInteger("setting","soltype",    SolType            );
    ini->WriteInteger("setting","plottype",   PlotType1          );
    ini->WriteInteger("setting","plottype2",  PlotType2          );
    ini->WriteInteger("setting","panelmode",  PanelMode          );
    ini->WriteString ("setting","proxyaddr",  ProxyAddr          );
    ini->WriteInteger("setting","moniport",   MoniPort           );
    ini->WriteInteger("setting","panelstack", PanelStack         );
    
    for (i=0;i<3;i++) {
        ini->WriteFloat("setting",s.sprintf("rovantdel_%d",i),RovAntDel[i]);
        ini->WriteFloat("setting",s.sprintf("refantdel_%d",i),RefAntDel[i]);
        ini->WriteFloat("setting",s.sprintf("rovpos_%d",i),   RovPos[i]);
        ini->WriteFloat("setting",s.sprintf("refpos_%d",i),   RefPos[i]);
    }
    for (i=0;i<10;i++) {
        ini->WriteString("tcpopt",s.sprintf("history%d" ,i),History [i]);
    }
    for (i=0;i<10;i++) {
        ini->WriteString("tcpopt",s.sprintf("mntphist%d",i),MntpHist[i]);
    }
    ini->WriteInteger("mapopt","nmappnt",NMapPnt);
    for (i=0;i<NMapPnt;i++) {
        AnsiString s1,s2;
        ini->WriteString("mapopt",s1.sprintf("pntname%d",i+1),PntName[i]);
        ini->WriteString("mapopt",s1.sprintf("pntpos%d" ,i+1),
            s2.sprintf("%.4f,%.4f,%.4f",PntPos[i][0],PntPos[i][1],PntPos[i][2]));
    }
    ini->WriteString ("setting","posfontname", PosFont->Name    );
    ini->WriteInteger("setting","posfontsize", PosFont->Size    );
    ini->WriteInteger("setting","posfontcolor",(int)PosFont->Color);
    ini->WriteInteger("setting","posfontbold",  PosFont->Style.Contains(fsBold));
    ini->WriteInteger("setting","posfontitalic",PosFont->Style.Contains(fsItalic));

    ini->WriteInteger("viewer","color1",  (int)TTextViewer::Color1);
    ini->WriteInteger("viewer","color2",  (int)TTextViewer::Color2);
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    
    ini->WriteInteger("window","width",    Width);
    ini->WriteInteger("window","height",   Height);
    if (PanelStack==0) {
        ini->WriteInteger("window","splitpos", Panel21 ->Width);
        ini->WriteInteger("window","splitpos2",Panel222->Width);
    }
    else {
        ini->WriteInteger("window","splitpos", Panel21 ->Height);
        ini->WriteInteger("window","splitpos2",Panel222->Height);
    }
    delete ini;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------



