//---------------------------------------------------------------------------
// rtklaunch : rtklib ap launcher
//
//          Copyright (C) 2013 by T.TAKASU, All rights reserved.
//
// options : rtklib launcher [-t title][-tray]
//
//           -t title   window title
//           -tray      start as task tray icon
//           -mkl       use rtkpost_mkl and rtknavi_mkl
//
// version : $Revision:$ $Date:$
// history : 2013/01/10  1.1 new
//---------------------------------------------------------------------------
#include <vcl.h>
#include <inifiles.hpp>
#pragma hdrstop

#include "rtklib.h"
#include "launchmain.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define BTN_SIZE        42
#define BTN_COUNT       7
#define MAX(x,y)        ((x)>(y)?(x):(y))

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
    Mkl=0;
    
    Caption="RTKLIB v." VER_RTKLIB;
    TrayIcon->Hint=Caption;
    Constraints->MinWidth=(Width-ClientWidth)+BTN_SIZE;
    Constraints->MaxWidth=(Width-ClientWidth)+BTN_SIZE*BTN_COUNT;
    
    strcpy(buff,GetCommandLine());
    for (p=strtok(buff," ");p&&argc<32;p=strtok(NULL," ")) {
        argv[argc++]=p;
    }
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
        else if (!strcmp(argv[i],"-tray")) tray=1;
        else if (!strcmp(argv[i],"-mkl")) Mkl=1;
    }
    if (tray) {
        Application->ShowMainForm=false;
        TrayIcon->Visible=true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    TIniFile *ini=new TIniFile(IniFile);
    Left  =ini->ReadInteger("pos","left",    0);
    Top   =ini->ReadInteger("pos","top",     0);
    Width =ini->ReadInteger("pos","width", 310);
    Height=ini->ReadInteger("pos","height", 79);
    delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    TIniFile *ini=new TIniFile(IniFile);
    ini->WriteInteger("pos","left",    Left);
    ini->WriteInteger("pos","top",      Top);
    ini->WriteInteger("pos","width",  Width);
    ini->WriteInteger("pos","height",Height);
    delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject *Sender)
{
    TSpeedButton *btn[]={
        BtnPlot,BtnConv,BtnStr,BtnPost,BtnNtrip,BtnNavi,BtnGet
    };
    int i,j,k,n,m,h;
    
    n=MAX(1,(Width-10)/BTN_SIZE);
    m=(BTN_COUNT-1)/n+1;
    h=(Height-ClientHeight)+BTN_SIZE*m;
    Constraints->MinHeight=h;
    Constraints->MaxHeight=h;
    
    for (i=k=0;k<7;i++) for (j=0;j<n&&k<BTN_COUNT;j++,k++) {
        btn[k]->Top =BTN_SIZE*i;
        btn[k]->Left=BTN_SIZE*j;
        btn[k]->Height=BTN_SIZE;
        btn[k]->Width =BTN_SIZE;
    }
    BtnTray->Left=ClientWidth -BtnTray->Width;
    BtnTray->Top =ClientHeight-BtnTray->Height;
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
    
    if (Mkl) {
        cmd1=cmd1+"_mkl"; cmd2=cmd2+"_mkl";
    }
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
    
    if (Mkl) {
        cmd1=cmd1+"_mkl"; cmd2=cmd2+"_mkl";
    }
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

