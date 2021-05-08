//---------------------------------------------------------------------------
// rtklaunch : rtklib ap launcher
//
//          Copyright (C) 2013-2020 by T.TAKASU, All rights reserved.
//
// options : rtklib launcher [-t title][-tray][-trace level]
//
//           -t title     window title
//           -tray        start as task tray icon
//           -trace level enable debug trace
//
// version : $Revision:$ $Date:$
// history : 2013/01/10  1.1 new
//           2016/09/03  1.2 add option -win64
//           2020/11/30  1.3 force initial window in main-screen
//                           delete RTKVIDEO button
//                           delete Options button
//                           delete commandline option -win64, -mkl
//                           add commandline option -trace
//---------------------------------------------------------------------------
#include <vcl.h>
#include <inifiles.hpp>
#pragma hdrstop

#include "rtklib.h"
#include "launchmain.h"
#include "launchoptdlg.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define BTN_SIZE        42
#define BTN_COUNT       7
#define MAX(x,y)        ((x)>(y)?(x):(y))
#define TRACE_FILE      "rtklaunch.trace"

//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    char file[1024]="rtklaunch.exe",buff[1024],*argv[32],*p;
    int i,argc=0,tray=0;
    
    ::GetModuleFileName(NULL,file,sizeof(file));
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".ini");
    IniFile=file;
    Option=0;
    Minimize=0;
    Trace=0;
    
    TIniFile *ini=new TIniFile(IniFile);
    Left  =ini->ReadInteger("pos","left",    0);
    Top   =ini->ReadInteger("pos","top",     0);
    Width =ini->ReadInteger("pos","width", 310);
    Height=ini->ReadInteger("pos","height", 79);
    Option=ini->ReadInteger("pos","option",  0);
    Minimize=ini->ReadInteger("pos","minimize",0);
    delete ini;
    
    // move window inside main-screen
    TRect rect;
    ::SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	if (Top<0) Top=0;
    if (Top+Height>rect.Height()) Top=rect.Height()-Height;
	if (Left<0) Left=0;
	if (Left+Width>rect.Width()) Left=rect.Width()-Width;

    Caption="RTKLIB v." VER_RTKLIB " " PATCH_LEVEL;
    BtnRtklib->Hint="RTKLIB v." VER_RTKLIB " " PATCH_LEVEL;
    TrayIcon->Hint=Caption;
    Panel1->Constraints->MinWidth=BTN_SIZE+2;
    Panel1->Constraints->MaxWidth=BTN_SIZE*BTN_COUNT+2;
    
    strcpy(buff,GetCommandLine());
    for (p=strtok(buff," ");p&&argc<32;p=strtok(NULL," ")) {
        argv[argc++]=p;
    }
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
        else if (!strcmp(argv[i],"-tray" )) tray    =1;
        else if (!strcmp(argv[i],"-min"  )) Minimize=1;
        else if (!strcmp(argv[i],"-trace")&&i+1<argc) Trace=atoi(argv[++i]);
    }
    if (Trace>=1) {
        traceopen(TRACE_FILE);
        tracelevel(Trace);
    }
    UpdatePanel();
    
    if (tray) {
        Application->ShowMainForm=false;
        TrayIcon->Visible=true;
    }
    trace(2,"screen: width=%d height=%d\n",rect.Width(),rect.Height());
    trace(2,"window: left=%d top=%d width=%d height=%d\n",Left,Top,Width,Height);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    TIniFile *ini=new TIniFile(IniFile);
    ini->WriteInteger("pos","left",    Left);
    ini->WriteInteger("pos","top",      Top);
    ini->WriteInteger("pos","width",  Width);
    ini->WriteInteger("pos","height",Height);
    ini->WriteInteger("pos","option",Option);
    ini->WriteInteger("pos","minimize",Minimize);
    delete ini;

    if (Trace>=1) {
        traceclose();
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnPlotClick(TObject *Sender)
{
    UnicodeString cmd1="rtkplot",cmd2="..\\..\\..\\bin\\rtkplot",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnConvClick(TObject *Sender)
{
    UnicodeString cmd1="rtkconv",cmd2="..\\..\\..\\bin\\rtkconv",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStrClick(TObject *Sender)
{
    UnicodeString cmd1="strsvr",cmd2="..\\..\\..\\bin\\strsvr",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnPostClick(TObject *Sender)
{
    UnicodeString cmd1="rtkpost",cmd2="..\\..\\..\\bin\\rtkpost",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnNtripClick(TObject *Sender)
{
    UnicodeString cmd1="srctblbrows",cmd2="..\\..\\..\\bin\\srctblbrows",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnNaviClick(TObject *Sender)
{
    UnicodeString cmd1="rtknavi",cmd2="..\\..\\..\\bin\\rtknavi",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnGetClick(TObject *Sender)
{
    UnicodeString cmd1="rtkget",cmd2="..\\..\\..\\bin\\rtkget",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::ExecCmd(AnsiString cmd)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    if (!CreateProcess(NULL,p,NULL,NULL,false,0,NULL,NULL,&si,&info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnTrayClick(TObject *Sender)
{
    Visible=false;
    TrayIcon->Visible=true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TrayIconDblClick(TObject *Sender)
{
    Visible=true;
    TrayIcon->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuExpandClick(TObject *Sender)
{
    Visible=true;
    TrayIcon->Visible=false;
    Minimize=0;
    UpdatePanel();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuPlotClick(TObject *Sender)
{
    BtnPlotClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuConvClick(TObject *Sender)
{
    BtnConvClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuStrClick(TObject *Sender)
{
    BtnStrClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuPostClick(TObject *Sender)
{
    BtnPostClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuNtripClick(TObject *Sender)
{
    BtnNtripClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuNaviClick(TObject *Sender)
{
    BtnNaviClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuGetClick(TObject *Sender)
{
    BtnGetClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MenuExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Panel1Resize(TObject *Sender)
{
    TSpeedButton *btn[]={
        BtnPlot,BtnConv,BtnStr,BtnPost,BtnNtrip,BtnNavi,BtnGet
    };
    int i,j,k,n,m,h;
    int btn_size=BtnPlot->Width;

    if (Minimize) return;
    n=MAX(1,Panel1->ClientWidth/btn_size);
	m=(BTN_COUNT-1)/n+1;
    h=btn_size*m+2;
    Panel1->Constraints->MinHeight=h;
    Panel1->Constraints->MaxHeight=h;
    Panel1->Constraints->MinWidth=btn_size+2;
    Panel1->Constraints->MaxWidth=btn_size*BTN_COUNT+2;
    
    for (i=k=0;k<BTN_COUNT;i++) for (j=0;j<n&&k<BTN_COUNT;j++,k++) {
        btn[k]->Top =btn_size*i+1;
        btn[k]->Left=btn_size*j+1;
        btn[k]->Height=btn_size;
        btn[k]->Width =btn_size;
    }
    BtnTray->Left=Panel1->ClientWidth -BtnTray->Width;
    BtnTray->Top =Panel1->ClientHeight-BtnTray->Height;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdatePanel(void)
{
    if (Minimize) {
        BorderStyle=bsToolWindow;
        Panel1->Visible=false;
        Panel2->Visible=true;
        AutoSize=true;
    }
    else {
        BorderStyle=bsSizeToolWin;
        Panel1->Visible=true;
        Panel2->Visible=false;
        AutoSize=false;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnRtklibMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y)
{
	PopupMenu->Popup(Left+X,Top+Y);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------

