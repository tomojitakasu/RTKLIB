//---------------------------------------------------------------------------
// rtkpost : post-processing analysis
//
//          Copyright (C) 2007-2012 by T.TAKASU, All rights reserved.
//
// options : rtkpost [-t title][-i file][-r file][-b file][-n file ...]
//                   [-d dir][-o file]
//                   [-ts y/m/d h:m:s][-te y/m/d h:m:s][-ti tint][-tu tunit]
//
//           -t title   window title
//           -i file    ini file path
//           -r file    rinex obs rover file
//           -b file    rinex obs base station file
//           -n file    rinex nav/clk, sp3, ionex or sp3 file
//           -d dir     output directory
//           -o file    output file
//           -ts y/m/d h:m:s time start
//           -te y/m/d h:m:s time end
//           -ti tint   time interval (s)
//           -tu tunit  time unit (hr)
//
// version : $Revision: 1.1 $ $Date: 2008/07/17 22:14:45 $
// history : 2008/07/14  1.0 new
//           2008/11/17  1.1 rtklib 2.1.1
//           2008/04/03  1.2 rtklib 2.3.1
//           2010/07/18  1.3 rtklib 2.4.0
//           2010/09/07  1.3 rtklib 2.4.1
//           2011/04/03  1.4 rtklib 2.4.2
//           2016/06/11  1.5 rtklib 2.4.3
//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "postmain.h"
#include "postopt.h"
#include "kmzconv.h"
#include "refdlg.h"
#include "timedlg.h"
#include "confdlg.h"
#include "keydlg.h"
#include "aboutdlg.h"
#include "viewer.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

TMainForm *MainForm;

#define PRGNAME     "RTKPOST"
#define MAXHIST     20
#define GOOGLE_EARTH "C:\\Program Files\\Google\\Google Earth\\googleearth.exe"

static const char version[]="$Revision: 1.1 $ $Date: 2008/07/17 22:14:45 $";

// global variables ---------------------------------------------------------
static gtime_t tstart_={0};         // time start for progress-bar
static gtime_t tend_  ={0};         // time end for progress-bar
static char rov_ [256]="";          // rover name
static char base_[256]="";          // base-station name

extern "C" {

// show message in message area ---------------------------------------------
extern int showmsg(char *format, ...)
{
    va_list arg;
    char buff[1024];
    if (*format) {
        va_start(arg,format);
        vsprintf(buff,format,arg);
        va_end(arg);
        MainForm->ShowMsg(buff);
    }
    else Application->ProcessMessages();
    return MainForm->AbortFlag;
}
// set time span of progress bar --------------------------------------------
extern void settspan(gtime_t ts, gtime_t te)
{
    tstart_=ts;
    tend_  =te;
}
// set current time to show progress ----------------------------------------
extern void settime(gtime_t time)
{
    static int i=0;
    double tt;
    if (tend_.time!=0&&tstart_.time!=0&&(tt=timediff(tend_,tstart_))>0.0) {
        MainForm->Progress->Position=(int)(timediff(time,tstart_)/tt*100.0+0.5);
    }
    if (i++%23==0) Application->ProcessMessages();
}

} // extern "C"

// convert string to double -------------------------------------------------
static double str2dbl(AnsiString str)
{
    double val=0.0;
    sscanf(str.c_str(),"%lf",&val);
    return val;
}
// constructor --------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    char file[1024]="rtkpost.exe",*p;
    int i;
    
    ::GetModuleFileName(NULL,file,sizeof(file));
    if (!(p=strrchr(file,'.'))) p=file+strlen(file);
    strcpy(p,".ini");
    IniFile=file;
    
    DynamicModel=IonoOpt=TropOpt=RovAntPcv=RefAntPcv=AmbRes=0;
    RovPosType=RefPosType=0;
    OutCntResetAmb=5; LockCntFixAmb=5; FixCntHoldAmb=10;
    MaxAgeDiff=30.0; RejectThres=30.0; RejectGdop=30.0;
    MeasErrR1=MeasErrR2=100.0; MeasErr2=0.004; MeasErr3=0.003; MeasErr4=1.0;
    SatClkStab=1E-11; ValidThresAR=3.0;
    RovAntE=RovAntN=RovAntU=RefAntE=RefAntN=RefAntU=0.0;
    for (i=0;i<3;i++) RovPos[i]=0.0;
    for (i=0;i<3;i++) RefPos[i]=0.0;
    
    DoubleBuffered=true;
}
// callback on form create --------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
    AnsiString s;
    
    Caption=s.sprintf("%s ver.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    
    ::DragAcceptFiles(Handle,true);
}
// callback on form show ----------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    TComboBox *ifile[]={InputFile3,InputFile4,InputFile5,InputFile6};
    char *p,*argv[32],buff[1024];
    int argc=0,n=0,inputflag=0;;
    
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
    for (int i=1;i<argc;i++) { // get ini file option
        if (!strcmp(argv[i],"-i")&&i+1<argc) IniFile=argv[++i];
    }
    LoadOpt();
    
    for (int i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
        else if (!strcmp(argv[i],"-r")&&i+1<argc) {
            InputFile1->Text=argv[++i];
            inputflag=1;
        }
        else if (!strcmp(argv[i],"-b")&&i+1<argc) InputFile2->Text=argv[++i];
        else if (!strcmp(argv[i],"-d")&&i+1<argc) {
            OutDirEna->Checked=true;
            OutDir->Text=argv[++i];
        }
        else if (!strcmp(argv[i],"-o")&&i+1<argc) OutputFile->Text=argv[++i];
        else if (!strcmp(argv[i],"-n")&&i+1<argc) {
            if (n<4) ifile[n++]->Text=argv[++i];
        }
        else if (!strcmp(argv[i],"-ts")&&i+2<argc) {
            TimeStart->Checked=true;
            TimeY1->Text=argv[++i]; TimeH1->Text=argv[++i];
        }
        else if (!strcmp(argv[i],"-te")&&i+2<argc) {
            TimeEnd->Checked=true;
            TimeY2->Text=argv[++i]; TimeH2->Text=argv[++i];
        }
        else if (!strcmp(argv[i],"-ti")&&i+1<argc) {
            TimeIntF->Checked=true;
            TimeInt->Text=argv[++i];
        }
        else if (!strcmp(argv[i],"-tu")&&i+1<argc) {
            TimeUnitF->Checked=true;
            TimeUnit->Text=argv[++i];
        }
    }
    if (inputflag) SetOutFile();
    
    UpdateEnable();
}
// callback on form close ---------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    SaveOpt();
}
// callback on drop files ---------------------------------------------------
void __fastcall TMainForm::DropFiles(TWMDropFiles msg)
{
    POINT point={0};
    int top,y;
    char *p,file[1024];
    
    if (DragQueryFile((HDROP)msg.Drop,0xFFFFFFFF,NULL,0)<=0) return;
    DragQueryFile((HDROP)msg.Drop,0,file,sizeof(file));
    if (!DragQueryPoint((HDROP)msg.Drop,&point)) return;
    
    y=point.y;
    top=Panel1->Top+Panel4->Top;
    if (y<=top+InputFile1->Top+InputFile1->Height) {
        InputFile1->Text=file;
        SetOutFile();
    }
    else if (y<=top+InputFile2->Top+InputFile2->Height) {
        InputFile2->Text=file;
    }
    else if (y<=top+InputFile3->Top+InputFile3->Height) {
        InputFile3->Text=file;
    }
    else if (y<=top+InputFile4->Top+InputFile4->Height) {
        InputFile4->Text=file;
    }
    else if (y<=top+InputFile5->Top+InputFile5->Height) {
        InputFile5->Text=file;
    }
    else if (y<=top+InputFile6->Top+InputFile6->Height) {
        InputFile6->Text=file;
    }
}
// callback on button-plot --------------------------------------------------
void __fastcall TMainForm::BtnPlotClick(TObject *Sender)
{
    AnsiString OutputFile_Text=OutputFile->Text;
    AnsiString file=FilePath(OutputFile_Text);
    AnsiString cmd1="rtkplot",cmd2="..\\..\\..\\bin\\rtkplot",opts="";
    
    opts+=" \""+file+"\"";
    
    if (!ExecCmd(cmd1+opts,1)&&!ExecCmd(cmd2+opts,1)) {
        ShowMsg((char *)"error : rtkplot execution");
    }
}
// callback on button-view --------------------------------------------------
void __fastcall TMainForm::BtnViewClick(TObject *Sender)
{
    AnsiString OutputFile_Text=OutputFile->Text;
    ViewFile(FilePath(OutputFile_Text));
}
// callback on button-to-kml ------------------------------------------------
void __fastcall TMainForm::BtnToKMLClick(TObject *Sender)
{
    AnsiString OutputFile_Text=OutputFile->Text;
    ConvDialog->Show(); 
    ConvDialog->SetInput(FilePath(OutputFile_Text));
}
// callback on button-options -----------------------------------------------
void __fastcall TMainForm::BtnOptionClick(TObject *Sender)
{
    int format=SolFormat;
    if (OptDialog->ShowModal()!=mrOk) return;
    if ((format==SOLF_NMEA)!=(SolFormat==SOLF_NMEA)) {
        SetOutFile();
    }
    UpdateEnable();
}
// callback on button-execute -----------------------------------------------
void __fastcall TMainForm::BtnExecClick(TObject *Sender)
{
    AnsiString OutputFile_Text=OutputFile->Text;
    char *p;
    
    if (InputFile1->Text=="") {
        showmsg((char *)"error : no rinex obs file (rover)");
        return;
    }
    if (InputFile2->Text==""&&PMODE_DGPS<=PosMode&&PosMode<=PMODE_FIXED) {
        showmsg((char *)"error : no rinex obs file (base station)");
        return;
    }
    if (OutputFile->Text=="") {
        showmsg((char *)"error : no output file");
        return;
    }
    if ((p=strrchr(OutputFile_Text.c_str(),'.'))) {
        if (!strcmp(p,".obs")||!strcmp(p,".OBS")||!strcmp(p,".nav")||
            !strcmp(p,".NAV")||!strcmp(p,".gnav")||!strcmp(p,".GNAV")||
            !strcmp(p,".gz")||!strcmp(p,".Z")||
            !strcmp(p+3,"o")||!strcmp(p+3,"O")||!strcmp(p+3,"d")||
            !strcmp(p+3,"D")||!strcmp(p+3,"n")||!strcmp(p+3,"N")||
            !strcmp(p+3,"g")||!strcmp(p+3,"G")) {
            showmsg((char *)"error : invalid extension of output file (%s)",p);
            return;
        }
    }
    showmsg((char *)"");
    BtnExec  ->Visible=false;
    BtnAbort ->Visible=true;
    AbortFlag=0;
    BtnExit  ->Enabled=false;
    BtnView  ->Enabled=false;
    BtnToKML ->Enabled=false;
    BtnPlot  ->Enabled=false;
    BtnOption->Enabled=false;
    Panel1   ->Enabled=false;
    
    if (ExecProc()>=0) {
        AddHist(InputFile1);
        AddHist(InputFile2);
        AddHist(InputFile3);
        AddHist(InputFile4);
        AddHist(InputFile5);
        AddHist(InputFile6);
        AddHist(OutputFile);
    }
    AnsiString Message_Caption=Message->Caption;
    if (strstr(Message_Caption.c_str(),"processing")) {
        showmsg((char *)"done");
    }
    BtnAbort ->Visible=false;
    BtnExec  ->Visible=true;
    BtnExec  ->Enabled=true;
    BtnExit  ->Enabled=true;
    BtnView  ->Enabled=true;
    BtnToKML ->Enabled=true;
    BtnPlot  ->Enabled=true;
    BtnOption->Enabled=true;
    Panel1   ->Enabled=true;
}
// callback on button-abort -------------------------------------------------
void __fastcall TMainForm::BtnAbortClick(TObject *Sender)
{
    AbortFlag=1;
    showmsg((char *)"aborted");
}
// callback on button-exit --------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    Close();
}
// callback on button-about -------------------------------------------------
void __fastcall TMainForm::BtnAboutClick(TObject *Sender)
{
    AnsiString prog=PRGNAME;
#ifdef _WIN64
    prog+="_WIN64";
#endif
#ifdef MKL
    prog+="_MKL";
#endif
    AboutDialog->About=prog;
    AboutDialog->IconIndex=1;
    AboutDialog->ShowModal();
}
// callback on button-time-1 ------------------------------------------------
void __fastcall TMainForm::BtnTime1Click(TObject *Sender)
{
    TimeDialog->Time=GetTime1();
    TimeDialog->ShowModal();
}
// callback on button-time-2 ------------------------------------------------
void __fastcall TMainForm::BtnTime2Click(TObject *Sender)
{
    TimeDialog->Time=GetTime2();
    TimeDialog->ShowModal();
}
// callback on button-inputfile-1 -------------------------------------------
void __fastcall TMainForm::BtnInputFile1Click(TObject *Sender)
{
    char file[1024],*p;
    
    OpenDialog->Title="RINEX OBS (Rover) File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=2;
    if (!OpenDialog->Execute()) return;
    InputFile1->Text=OpenDialog->FileName;
    SetOutFile();
}
// callback on button-inputfile-2 -------------------------------------------
void __fastcall TMainForm::BtnInputFile2Click(TObject *Sender)
{
    OpenDialog->Title="RINEX OBS (Base Station) File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=2;
    if (!OpenDialog->Execute()) return;
    InputFile2->Text=OpenDialog->FileName;
}
// callback on button-inputfile-3 -------------------------------------------
void __fastcall TMainForm::BtnInputFile3Click(TObject *Sender)
{
    OpenDialog->Title="RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=3;
    if (!OpenDialog->Execute()) return;
    InputFile3->Text=OpenDialog->FileName;
}
// callback on button-inputfile-4 -------------------------------------------
void __fastcall TMainForm::BtnInputFile4Click(TObject *Sender)
{
    OpenDialog->Title="RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=4;
    if (!OpenDialog->Execute()) return;
    InputFile4->Text=OpenDialog->FileName;
}
// callback on button-inputfile-5 -------------------------------------------
void __fastcall TMainForm::BtnInputFile5Click(TObject *Sender)
{
    OpenDialog->Title="RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=4;
    if (!OpenDialog->Execute()) return;
    InputFile5->Text=OpenDialog->FileName;
}
// callback on button-inputfile-6 -------------------------------------------
void __fastcall TMainForm::BtnInputFile6Click(TObject *Sender)
{
    OpenDialog->Title="RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File";
    OpenDialog->FileName="";
    OpenDialog->FilterIndex=5;
    if (!OpenDialog->Execute()) return;
    InputFile6->Text=OpenDialog->FileName;
}
// callback on button-outputfile --------------------------------------------
void __fastcall TMainForm::BtnOutputFileClick(TObject *Sender)
{
    SaveDialog->Title="Output File";
    OpenDialog->FileName="";
    if (!SaveDialog->Execute()) return;
    OutputFile->Text=SaveDialog->FileName;
}
// callback on button-inputview-1 -------------------------------------------
void __fastcall TMainForm::BtnInputView1Click(TObject *Sender)
{
    AnsiString InputFile1_Text=InputFile1->Text;
    ViewFile(FilePath(InputFile1_Text));
}
// callback on button-inputview-2 -------------------------------------------
void __fastcall TMainForm::BtnInputView2Click(TObject *Sender)
{
    AnsiString InputFile2_Text=InputFile2->Text;
    ViewFile(FilePath(InputFile2_Text));
}
// callback on button-inputview-3 -------------------------------------------
void __fastcall TMainForm::BtnInputView3Click(TObject *Sender)
{
    AnsiString InputFile1_Text=InputFile1->Text;
    AnsiString InputFile3_Text=InputFile3->Text;
    AnsiString file=FilePath(InputFile3_Text);
    char f[1024];
    
    if (file=="") {
        file=FilePath(InputFile1_Text);
        if (!ObsToNav(file.c_str(),f)) return;
        file=f;
    }
    ViewFile(file);
}
// callback on button-inputview-4 -------------------------------------------
void __fastcall TMainForm::BtnInputView4Click(TObject *Sender)
{
    AnsiString InputFile4_Text=InputFile4->Text;
    ViewFile(FilePath(InputFile4_Text));
}
// callback on button-inputview-5 -------------------------------------------
void __fastcall TMainForm::BtnInputView5Click(TObject *Sender)
{
    AnsiString InputFile5_Text=InputFile5->Text;
    ViewFile(FilePath(InputFile5_Text));
}
// callback on button-inputview-6 -------------------------------------------
void __fastcall TMainForm::BtnInputView6Click(TObject *Sender)
{
    AnsiString InputFile6_Text=InputFile6->Text;
    ViewFile(FilePath(InputFile6_Text));
}
// callback on button-outputview-1 ------------------------------------------
void __fastcall TMainForm::BtnOutputView1Click(TObject *Sender)
{
    AnsiString OutputFile_Text=OutputFile->Text;
    AnsiString file=FilePath(OutputFile_Text)+".stat";
    FILE *fp=fopen(file.c_str(),"r");
    if (fp) fclose(fp); else return;
    ViewFile(file);
}
// callback on button-outputview-2 ------------------------------------------
void __fastcall TMainForm::BtnOutputView2Click(TObject *Sender)
{
    AnsiString OutputFile_Text=OutputFile->Text;
    AnsiString file=FilePath(OutputFile_Text)+".trace";
    FILE *fp=fopen(file.c_str(),"r");
    if (fp) fclose(fp); else return;
    ViewFile(file);
}
// callback on button-inputplot-1 -------------------------------------------
void __fastcall TMainForm::BtnInputPlot1Click(TObject *Sender)
{
    AnsiString InputFile1_Text=InputFile1->Text;
    AnsiString InputFile2_Text=InputFile2->Text;
    AnsiString InputFile3_Text=InputFile3->Text;
    AnsiString InputFile4_Text=InputFile4->Text;
    AnsiString InputFile5_Text=InputFile5->Text;
    AnsiString InputFile6_Text=InputFile6->Text;
    AnsiString files[6];
    AnsiString cmd1="rtkplot",cmd2="..\\..\\..\\bin\\rtkplot",opts="";
    char navfile[1024];
    
    files[0]=FilePath(InputFile1_Text); /* obs rover */
    files[1]=FilePath(InputFile2_Text); /* obs base */
    files[2]=FilePath(InputFile3_Text);
    files[3]=FilePath(InputFile4_Text);
    files[4]=FilePath(InputFile5_Text);
    files[5]=FilePath(InputFile6_Text);
    
    if (files[2]=="") {
        if (ObsToNav(files[0].c_str(),navfile)) files[2]=navfile;
    }
    opts=" -r \""+files[0]+"\" \""+files[2]+"\" \""+files[3]+"\" \""+
        files[4]+"\" \""+files[5]+"\"";
    
    if (!ExecCmd(cmd1+opts,1)&&!ExecCmd(cmd2+opts,1)) {
        ShowMsg((char *)"error : rtkplot execution");
    }
}
// callback on button-inputplot-2 -------------------------------------------
void __fastcall TMainForm::BtnInputPlot2Click(TObject *Sender)
{
    AnsiString InputFile1_Text=InputFile1->Text;
    AnsiString InputFile2_Text=InputFile2->Text;
    AnsiString InputFile3_Text=InputFile3->Text;
    AnsiString InputFile4_Text=InputFile4->Text;
    AnsiString InputFile5_Text=InputFile5->Text;
    AnsiString InputFile6_Text=InputFile6->Text;
    AnsiString files[6];
    AnsiString cmd1="rtkplot",cmd2="..\\..\\..\\bin\\rtkplot",opts="";
    char navfile[1024],gnavfile[1024];
    
    files[0]=FilePath(InputFile1_Text); /* obs rover */
    files[1]=FilePath(InputFile2_Text); /* obs base */
    files[2]=FilePath(InputFile3_Text);
    files[3]=FilePath(InputFile4_Text);
    files[4]=FilePath(InputFile5_Text);
    files[5]=FilePath(InputFile6_Text);
    
    if (files[2]=="") {
        if (ObsToNav(files[0].c_str(),navfile)) files[2]=navfile;
    }
    opts=" -r \""+files[1]+"\" \""+files[2]+"\" \""+files[3]+"\" \""+
         files[4]+"\" \""+files[5]+"\"";
    
    if (!ExecCmd(cmd1+opts,1)&&!ExecCmd(cmd2+opts,1)) {
        ShowMsg((char *)"error : rtkplot execution");
    }
}
// callback on button-output-directory --------------------------------------
void __fastcall TMainForm::BtnOutDirClick(TObject *Sender)
{
#ifdef TCPP
    AnsiString dir=OutDir->Text;
    if (!SelectDirectory("Output Directory","",dir)) return;
    OutDir->Text=dir;
#else
    UnicodeString dir=OutDir->Text;
    TSelectDirExtOpts opt=TSelectDirExtOpts()<<sdNewUI<<sdNewFolder;
    if (!SelectDirectory(L"Output Directory",L"",dir,opt)) return;
    OutDir->Text=dir;
#endif
}
// callback on button keyword -----------------------------------------------
void __fastcall TMainForm::BtnKeywordClick(TObject *Sender)
{
    KeyDialog->Flag=2;
    KeyDialog->Show();
}
// callback on time-start/end check -----------------------------------------
void __fastcall TMainForm::TimeStartClick(TObject *Sender)
{
    UpdateEnable();
}
// callback on time-interval check ------------------------------------------
void __fastcall TMainForm::TimeIntFClick(TObject *Sender)
{
    UpdateEnable();
}
// callback on time-unit check ----------------------------------------------
void __fastcall TMainForm::TimeUnitFClick(TObject *Sender)
{
    UpdateEnable();
}
// callback on time-ymd-1 updown --------------------------------------------
void __fastcall TMainForm::TimeY1UDChangingEx(TObject *Sender,
      bool &AllowChange, int NewValue, TUpDownDirection Direction)
{
    AnsiString TimeY1_Text=TimeY1->Text,s;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY1->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY1->SelStart=p>7||p==0?10:(p>4?7:4);
}
// callback on time-hms-1 updown --------------------------------------------
void __fastcall TMainForm::TimeH1UDChangingEx(TObject *Sender,
      bool &AllowChange, int NewValue, TUpDownDirection Direction)
{
    AnsiString TimeH1_Text=TimeH1->Text,s;
    int hms[3]={0},sec,p=TimeH1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH1_Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH1->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
    TimeH1->SelStart=p>5||p==0?8:(p>2?5:2);
}
// callback on time-ymd-2 updown --------------------------------------------
void __fastcall TMainForm::TimeY2UDChangingEx(TObject *Sender,
      bool &AllowChange, int NewValue, TUpDownDirection Direction)
{
    AnsiString TimeY2_Text=TimeY2->Text,s;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY2->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY2->SelStart=p>7||p==0?10:(p>4?7:4);
}
// callback on time-hms-2 updown --------------------------------------------
void __fastcall TMainForm::TimeH2UDChangingEx(TObject *Sender,
      bool &AllowChange, int NewValue, TUpDownDirection Direction)
{
    AnsiString TimeH2_Text=TimeH2->Text,s;
    int hms[3]={0},sec,p=TimeH2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH2_Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH2->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
    TimeH2->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeY1UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeH1UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeY2UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeH2UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
// callback on inputfile-1 change -------------------------------------------
void __fastcall TMainForm::InputFile1Change(TObject *Sender)
{
    SetOutFile();
}
// callback on output-directory checked -------------------------------------
void __fastcall TMainForm::OutDirEnaClick(TObject *Sender)
{
	UpdateEnable();
    SetOutFile();
}
// callback on output-directory change --------------------------------------
void __fastcall TMainForm::OutDirChange(TObject *Sender)
{
    SetOutFile();
}
// set output file path -----------------------------------------------------
void __fastcall TMainForm::SetOutFile(void)
{
    AnsiString InputFile1_Text=InputFile1->Text;
    AnsiString OutDir_Text=OutDir->Text;
    char *p,ofile[1024],ifile[1024];
    
    if (InputFile1->Text=="") return;
    
    strcpy(ifile,InputFile1_Text.c_str());
    
    if (OutDirEna->Checked) {
        if ((p=strrchr(ifile,'\\'))) p++; else p=ifile;
        sprintf(ofile,"%s\\%s",OutDir_Text.c_str(),p);
    }
    else {
        strcpy(ofile,ifile);
    }
    if (!(p=strrchr(ofile,'.'))) p=ofile+strlen(ofile);
    strcpy(p,SolFormat==SOLF_NMEA?".nmea":".pos");
    for (p=ofile;*p;p++) if (*p=='*') *p='0';
    OutputFile->Text=ofile;
}
// execute post-processing --------------------------------------------------
int __fastcall TMainForm::ExecProc(void)
{
    AnsiString InputFile1_Text=InputFile1->Text,InputFile2_Text=InputFile2->Text;
    AnsiString InputFile3_Text=InputFile3->Text,InputFile4_Text=InputFile4->Text;
    AnsiString InputFile5_Text=InputFile5->Text,InputFile6_Text=InputFile6->Text;
    AnsiString OutputFile_Text=OutputFile->Text;
    FILE *fp;
    prcopt_t prcopt=prcopt_default;
    solopt_t solopt=solopt_default;
    filopt_t filopt={""};
    gtime_t ts={0},te={0};
    double ti=0.0,tu=0.0;
    int i,n=0,stat;
    char infile_[6][1024]={""},*infile[6],outfile[1024];
    char *rov,*base,*p,*q,*r;
    
    // get processing options
    if (TimeStart->Checked) ts=GetTime1();
    if (TimeEnd  ->Checked) te=GetTime2();
    if (TimeIntF ->Checked) ti=str2dbl(TimeInt ->Text);
    if (TimeUnitF->Checked) tu=str2dbl(TimeUnit->Text)*3600.0;
    
    if (!GetOption(prcopt,solopt,filopt)) return 0;
    
    // set input/output files
    for (i=0;i<6;i++) infile[i]=infile_[i];
    
    strcpy(infile[n++],InputFile1_Text.c_str());
    
    if (PMODE_DGPS<=prcopt.mode&&prcopt.mode<=PMODE_FIXED) {
        strcpy(infile[n++],InputFile2_Text.c_str());
    }
    if (InputFile3->Text!="") {
        strcpy(infile[n++],InputFile3_Text.c_str());
    }
    else if (!ObsToNav(InputFile1_Text.c_str(),infile[n++])) {
        showmsg((char *)"error: no navigation data");
        return 0;
    }
    if (InputFile4_Text!="") {
        strcpy(infile[n++],InputFile4_Text.c_str());
    }
    if (InputFile5_Text!="") {
        strcpy(infile[n++],InputFile5_Text.c_str());
    }
    if (InputFile6_Text!="") {
        strcpy(infile[n++],InputFile6_Text.c_str());
    }
    strcpy(outfile,OutputFile_Text.c_str());
    
    // confirm overwrite
    if (!TimeStart->Checked||!TimeEnd->Checked) {
        if ((fp=fopen(outfile,"r"))) {
            fclose(fp);
            ConfDialog->Label2->Caption=outfile;
            if (ConfDialog->ShowModal()!=mrOk) return 0;
        }
    }
    // set rover and base station list
    rov =new char [strlen(RovList .c_str())];
    base=new char [strlen(BaseList.c_str())];
    
    for (p=RovList.c_str(),r=rov;*p;p=q+2) {
        
        if (!(q=strstr(p,"\r\n"))) {
            if (*p!='#') strcpy(r,p); break;
        }
        else if (*p!='#') {
            strncpy(r,p,q-p); r+=q-p;
            strcpy(r++," ");
        }
    }
    for (p=BaseList.c_str(),r=base;*p;p=q+2) {
        
        if (!(q=strstr(p,"\r\n"))) {
            if (*p!='#') strcpy(r,p); break;
        }
        else if (*p!='#') {
            strncpy(r,p,q-p); r+=q-p;
            strcpy(r++," ");
        }
    }
    Progress->Position=0;
    showmsg((char *)"reading...");
    
    // post processing positioning
    if ((stat=postpos(ts,te,ti,tu,&prcopt,&solopt,&filopt,infile,n,outfile,
                      rov,base))==1) {
        showmsg((char *)"aborted");
    }
    delete [] rov ;
    delete [] base;
    
    return stat;
}
// get processing and solution options --------------------------------------
int __fastcall TMainForm::GetOption(prcopt_t &prcopt, solopt_t &solopt,
                                    filopt_t &filopt)
{
    char buff[1024],id[32],*p;
    int sat,ex;
    
    // processing options
    prcopt.mode     =PosMode;
    prcopt.soltype  =Solution;
    prcopt.nf       =Freq+1;
    prcopt.navsys   =NavSys;
    prcopt.elmin    =ElMask*D2R;
    prcopt.snrmask  =SnrMask;
    prcopt.sateph   =SatEphem;
    prcopt.modear   =AmbRes;
    prcopt.glomodear=GloAmbRes;
    prcopt.bdsmodear=BdsAmbRes;
    prcopt.maxout   =OutCntResetAmb;
    prcopt.minfix   =FixCntHoldAmb;
    prcopt.minlock  =LockCntFixAmb;
    prcopt.ionoopt  =IonoOpt;
    prcopt.tropopt  =TropOpt;
    prcopt.posopt[0]=PosOpt[0];
    prcopt.posopt[1]=PosOpt[1];
    prcopt.posopt[2]=PosOpt[2];
    prcopt.posopt[3]=PosOpt[3];
    prcopt.posopt[4]=PosOpt[4];
    prcopt.posopt[5]=PosOpt[5];
    prcopt.dynamics =DynamicModel;
    prcopt.tidecorr =TideCorr;
    prcopt.armaxiter=ARIter;
    prcopt.niter    =NumIter;
    prcopt.intpref  =IntpRefObs;
    prcopt.sbassatsel=SbasSat;
    prcopt.eratio[0]=MeasErrR1;
    prcopt.eratio[1]=MeasErrR2;
    prcopt.err[1]   =MeasErr2;
    prcopt.err[2]   =MeasErr3;
    prcopt.err[3]   =MeasErr4;
    prcopt.err[4]   =MeasErr5;
    prcopt.prn[0]   =PrNoise1;
    prcopt.prn[1]   =PrNoise2;
    prcopt.prn[2]   =PrNoise3;
    prcopt.prn[3]   =PrNoise4;
    prcopt.prn[4]   =PrNoise5;
    prcopt.sclkstab =SatClkStab;
    prcopt.thresar[0]=ValidThresAR;
    prcopt.thresar[1]=ThresAR2;
    prcopt.thresar[2]=ThresAR3;
    prcopt.elmaskar =ElMaskAR*D2R;
    prcopt.elmaskhold=ElMaskHold*D2R;
    prcopt.thresslip=SlipThres;
    prcopt.maxtdiff =MaxAgeDiff;
    prcopt.maxgdop  =RejectGdop;
    prcopt.maxinno  =RejectThres;
    prcopt.outsingle=OutputSingle;
    if (BaseLineConst) {
        prcopt.baseline[0]=BaseLine[0];
        prcopt.baseline[1]=BaseLine[1];
    }
    else {
        prcopt.baseline[0]=0.0;
        prcopt.baseline[1]=0.0;
    }
    if (PosMode!=PMODE_FIXED&&PosMode!=PMODE_PPP_FIXED) {
        for (int i=0;i<3;i++) prcopt.ru[i]=0.0;
    }
    else if (RovPosType<=2) {
        for (int i=0;i<3;i++) prcopt.ru[i]=RovPos[i];
    }
    else prcopt.rovpos=RovPosType-2; /* 1:single,2:posfile,3:rinex */
    
    if (PosMode==PMODE_SINGLE||PosMode==PMODE_MOVEB) {
        for (int i=0;i<3;i++) prcopt.rb[i]=0.0;
    }
    else if (RefPosType<=2) {
        for (int i=0;i<3;i++) prcopt.rb[i]=RefPos[i];
    }
    else prcopt.refpos=RefPosType-2;
    
    if (RovAntPcv) {
        strcpy(prcopt.anttype[0],RovAnt.c_str());
        prcopt.antdel[0][0]=RovAntE;
        prcopt.antdel[0][1]=RovAntN;
        prcopt.antdel[0][2]=RovAntU;
    }
    if (RefAntPcv) {
        strcpy(prcopt.anttype[1],RefAnt.c_str());
        prcopt.antdel[1][0]=RefAntE;
        prcopt.antdel[1][1]=RefAntN;
        prcopt.antdel[1][2]=RefAntU;
    }
    if (ExSats!="") { // excluded satellites
        strcpy(buff,ExSats.c_str());
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if (*p=='+') {ex=2; p++;} else ex=1;
            if (!(sat=satid2no(p))) continue;
            prcopt.exsats[sat-1]=ex;
        }
    }
    // extended receiver error model option
    prcopt.exterr=ExtErr;
    
    strcpy(prcopt.rnxopt[0],RnxOpts1.c_str());
    strcpy(prcopt.rnxopt[1],RnxOpts2.c_str());
    strcpy(prcopt.pppopt,PPPOpts.c_str());
    
    // solution options
    solopt.posf     =SolFormat;
    solopt.times    =TimeFormat==0?0:TimeFormat-1;
    solopt.timef    =TimeFormat==0?0:1;
    solopt.timeu    =TimeDecimal<=0?0:TimeDecimal;
    solopt.degf     =LatLonFormat;
    solopt.outhead  =OutputHead;
    solopt.outopt   =OutputOpt;
    solopt.maxsolstd=MaxSolStd;
    solopt.datum    =OutputDatum;
    solopt.height   =OutputHeight;
    solopt.geoid    =OutputGeoid;
    solopt.solstatic=SolStatic;
    solopt.sstat    =DebugStatus;
    solopt.trace    =DebugTrace;
    strcpy(solopt.sep,FieldSep!=""?FieldSep.c_str():" ");
    sprintf(solopt.prog,"%s ver.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    
    // file options
    strcpy(filopt.satantp,SatPcvFile.c_str());
    strcpy(filopt.rcvantp,AntPcvFile.c_str());
    strcpy(filopt.stapos, StaPosFile.c_str());
    strcpy(filopt.geoid,  GeoidDataFile.c_str());
    strcpy(filopt.iono,   IonoFile.c_str());
    strcpy(filopt.eop,    EOPFile.c_str());
    strcpy(filopt.dcb,    DCBFile.c_str());
    strcpy(filopt.blq,    BLQFile.c_str());
    
    return 1;
}
// observation file to nav file ---------------------------------------------
int __fastcall TMainForm::ObsToNav(const char *obsfile, char *navfile)
{
    char *p;
    strcpy(navfile,obsfile);
    if (!(p=strrchr(navfile,'.'))) return 0;
    if      (strlen(p)==4&&*(p+3)=='o') *(p+3)='*';
    else if (strlen(p)==4&&*(p+3)=='d') *(p+3)='*';
    else if (strlen(p)==4&&*(p+3)=='O') *(p+3)='*';
    else if (!strcmp(p,".obs")) strcpy(p,".*nav");
    else if (!strcmp(p,".OBS")) strcpy(p,".*NAV");
    else if (!strcmp(p,".gz")||!strcmp(p,".Z")) {
        if      (*(p-1)=='o') *(p-1)='*';
        else if (*(p-1)=='d') *(p-1)='*';
        else if (*(p-1)=='O') *(p-1)='*';
        else return 0;
    }
    else return 0;
    return 1;
}
// replace file path with keywords ------------------------------------------
AnsiString __fastcall TMainForm::FilePath(AnsiString file)
{
    AnsiString s;
    gtime_t ts={0};
    char rov[256]="",base[256]="",path[1024],*p,*q;
    
    if (TimeStart->Checked) ts=GetTime1();
    
    for (p=RovList.c_str();(q=strstr(p,"\r\n"));p=q+2) {
        if (*p&&*p!='#') break;
    }
    if (!q) strcpy(rov,p); else strncpy(rov,p,q-p);
    
    for (p=BaseList.c_str();(q=strstr(p,"\r\n"));p=q+2) {
        if (*p&&p[0]!='#') break;
    }
    if (!q) strcpy(base,p); else strncpy(base,p,q-p);
    
    reppath(file.c_str(),path,ts,rov,base);
    
    return (s=path);
}
// read history -------------------------------------------------------------
TStringList * __fastcall TMainForm::ReadList(TIniFile *ini, AnsiString cat,
    AnsiString key)
{
    TStringList *list=new TStringList;
    AnsiString s,item;
    int i;
    
    for (i=0;i<100;i++) {
        item=ini->ReadString(cat,s.sprintf("%s_%03d",key.c_str(),i),"");
        if (item!="") list->Add(item); else break;
    }
    return list;
}
// write history ------------------------------------------------------------
void __fastcall TMainForm::WriteList(TIniFile *ini, AnsiString cat,
    AnsiString key, TStrings *list)
{
    AnsiString s;
    int i;
    
    for (i=0;i<list->Count;i++) {
        ini->WriteString(cat,s.sprintf("%s_%03d",key.c_str(),i),list->Strings[i]);
    }
}
// add history --------------------------------------------------------------
void __fastcall TMainForm::AddHist(TComboBox *combo)
{
    AnsiString hist=combo->Text;
    if (hist=="") return;
    TStrings *list=combo->Items;
    int i=list->IndexOf(hist);
    if (i>=0) list->Delete(i);
    list->Insert(0,hist);
    for (int i=list->Count-1;i>=MAXHIST;i--) list->Delete(i);
    combo->ItemIndex=0;
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
// view file ----------------------------------------------------------------
void __fastcall TMainForm::ViewFile(AnsiString file)
{
    TTextViewer *viewer;
    AnsiString f;
    char tmpfile[1024];
    int cstat;
    
    if (file=="") return;
    cstat=rtk_uncompress(file.c_str(),tmpfile);
    f=!cstat?file.c_str():tmpfile;
    
    viewer=new TTextViewer(Application);
    viewer->Caption=file;
    viewer->Show();
    viewer->Read(f);
    if (cstat==1) remove(tmpfile);
}
// show message in message area ---------------------------------------------
void __fastcall TMainForm::ShowMsg(char *msg)
{
    Message->Caption=msg;
}
// get time from time-1 -----------------------------------------------------
gtime_t _fastcall TMainForm::GetTime1(void)
{
    AnsiString TimeY1_Text=TimeY1->Text,TimeH1_Text=TimeH1->Text;
    double ep[]={2000,1,1,0,0,0};
    
    sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    sscanf(TimeH1_Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
    return epoch2time(ep);
}
// get time from time-2 -----------------------------------------------------
gtime_t _fastcall TMainForm::GetTime2(void)
{
    AnsiString TimeY2_Text=TimeY2->Text,TimeH2_Text=TimeH2->Text;
    double ep[]={2000,1,1,0,0,0};
    
    sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    sscanf(TimeH2_Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
    return epoch2time(ep);
}
// set time to time-1 -------------------------------------------------------
void _fastcall TMainForm::SetTime1(gtime_t time)
{
    AnsiString s;
    double ep[6];
    
    time2epoch(time,ep);
    TimeY1->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeH1->Text=s.sprintf("%02.0f:%02.0f:%02.0f",ep[3],ep[4],ep[5]);
    TimeY1->SelStart=10; TimeH1->SelStart=10;
}
// set time to time-2 -------------------------------------------------------
void _fastcall TMainForm::SetTime2(gtime_t time)
{
    AnsiString s;
    double ep[6];
    
    time2epoch(time,ep);
    TimeY2->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeH2->Text=s.sprintf("%02.0f:%02.0f:%02.0f",ep[3],ep[4],ep[5]);
    TimeY2->SelStart=10; TimeH2->SelStart=10;
}
// update enable/disable of widgets -----------------------------------------
void __fastcall TMainForm::UpdateEnable(void)
{
    int moder=PMODE_DGPS<=PosMode&&PosMode<=PMODE_FIXED;
    
    LabelInputFile1->Caption=moder?"RINEX OBS: Rover":"RINEX OBS";
    InputFile2     ->Enabled=moder;
    BtnInputFile2  ->Enabled=moder;
    BtnInputPlot2  ->Enabled=moder;
    BtnInputView2  ->Enabled=moder;
    BtnOutputView1 ->Enabled=DebugStatus>0;
    BtnOutputView2 ->Enabled=DebugTrace >0;
    LabelInputFile3->Enabled=moder;
    TimeY1         ->Enabled=TimeStart->Checked;
    TimeH1         ->Enabled=TimeStart->Checked;
    TimeY1UD       ->Enabled=TimeStart->Checked;
    TimeH1UD       ->Enabled=TimeStart->Checked;
    BtnTime1       ->Enabled=TimeStart->Checked;
    TimeY2         ->Enabled=TimeEnd  ->Checked;
    TimeH2         ->Enabled=TimeEnd  ->Checked;
    TimeY2UD       ->Enabled=TimeEnd  ->Checked;
    TimeH2UD       ->Enabled=TimeEnd  ->Checked;
    BtnTime2       ->Enabled=TimeEnd  ->Checked;
    TimeInt        ->Enabled=TimeIntF ->Checked;
    LabelTimeInt   ->Enabled=TimeIntF ->Checked;
    TimeUnitF      ->Enabled=TimeStart->Checked&&TimeEnd  ->Checked;
    TimeUnit       ->Enabled=TimeUnitF->Enabled&&TimeUnitF->Checked;
    LabelTimeUnit  ->Enabled=TimeUnitF->Enabled&&TimeUnitF->Checked;
    OutDir         ->Enabled=OutDirEna->Checked;
    BtnOutDir      ->Enabled=OutDirEna->Checked;
    LabelOutDir    ->Enabled=OutDirEna->Checked;
}
// load options from ini file -----------------------------------------------
void __fastcall TMainForm::LoadOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s;
    char *p;
    
    TimeStart->Checked =ini->ReadInteger("set","timestart",   0);
    TimeEnd->Checked   =ini->ReadInteger("set","timeend",     0);
    TimeY1->Text       =ini->ReadString ("set","timey1",      "2000/01/01");
    TimeY1->Text       =ini->ReadString ("set","timey1",      "2000/01/01");
    TimeH1->Text       =ini->ReadString ("set","timeh1",      "00:00:00");
    TimeY2->Text       =ini->ReadString ("set","timey2",      "2000/01/01");
    TimeH2->Text       =ini->ReadString ("set","timeh2",      "00:00:00");
    TimeIntF ->Checked =ini->ReadInteger("set","timeintf",    0);
    TimeInt->Text      =ini->ReadString ("set","timeint",     "0");
    TimeUnitF->Checked =ini->ReadInteger("set","timeunitf",   0);
    TimeUnit->Text     =ini->ReadString ("set","timeunit",    "24");
    InputFile1->Text   =ini->ReadString ("set","inputfile1",  "");
    InputFile2->Text   =ini->ReadString ("set","inputfile2",  "");
    InputFile3->Text   =ini->ReadString ("set","inputfile3",  "");
    InputFile4->Text   =ini->ReadString ("set","inputfile4",  "");
    InputFile5->Text   =ini->ReadString ("set","inputfile5",  "");
    InputFile6->Text   =ini->ReadString ("set","inputfile6",  "");
    OutDirEna->Checked =ini->ReadInteger("set","outputdirena", 0);
    OutDir->Text       =ini->ReadString ("set","outputdir",   "");
    OutputFile->Text   =ini->ReadString ("set","outputfile",  "");
    
    InputFile1->Items  =ReadList(ini,"hist","inputfile1");
    InputFile2->Items  =ReadList(ini,"hist","inputfile2");
    InputFile3->Items  =ReadList(ini,"hist","inputfile3");
    InputFile4->Items  =ReadList(ini,"hist","inputfile4");
    InputFile5->Items  =ReadList(ini,"hist","inputfile5");
    InputFile6->Items  =ReadList(ini,"hist","inputfile6");
    OutputFile->Items  =ReadList(ini,"hist","outputfile");
    
    PosMode            =ini->ReadInteger("opt","posmode",        0);
    Freq               =ini->ReadInteger("opt","freq",           1);
    Solution           =ini->ReadInteger("opt","solution",       0);
    ElMask             =ini->ReadFloat  ("opt","elmask",      15.0);
    SnrMask.ena[0]     =ini->ReadInteger("opt","snrmask_ena1",   0);
    SnrMask.ena[1]     =ini->ReadInteger("opt","snrmask_ena2",   0);
    for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
        SnrMask.mask[i][j]=
            ini->ReadFloat("opt",s.sprintf("snrmask_%d_%d",i+1,j+1),0.0);
    }
    IonoOpt            =ini->ReadInteger("opt","ionoopt",     IONOOPT_BRDC);
    TropOpt            =ini->ReadInteger("opt","tropopt",     TROPOPT_SAAS);
    RcvBiasEst         =ini->ReadInteger("opt","rcvbiasest",     0);
    DynamicModel       =ini->ReadInteger("opt","dynamicmodel",   0);
    TideCorr           =ini->ReadInteger("opt","tidecorr",       0);
    SatEphem           =ini->ReadInteger("opt","satephem",       0);
    ExSats             =ini->ReadString ("opt","exsats",        "");
    NavSys             =ini->ReadInteger("opt","navsys",   SYS_GPS);
    PosOpt[0]          =ini->ReadInteger("opt","posopt1",        0);
    PosOpt[1]          =ini->ReadInteger("opt","posopt2",        0);
    PosOpt[2]          =ini->ReadInteger("opt","posopt3",        0);
    PosOpt[3]          =ini->ReadInteger("opt","posopt4",        0);
    PosOpt[4]          =ini->ReadInteger("opt","posopt5",        0);
    PosOpt[5]          =ini->ReadInteger("opt","posopt6",        0);
    MapFunc            =ini->ReadInteger("opt","mapfunc",        0);
    
    AmbRes             =ini->ReadInteger("opt","ambres",         1);
    GloAmbRes          =ini->ReadInteger("opt","gloambres",      1);
    BdsAmbRes          =ini->ReadInteger("opt","bdsambres",      1);
    ValidThresAR       =ini->ReadFloat  ("opt","validthresar", 3.0);
    ThresAR2           =ini->ReadFloat  ("opt","thresar2",  0.9999);
    ThresAR3           =ini->ReadFloat  ("opt","thresar3",    0.25);
    LockCntFixAmb      =ini->ReadInteger("opt","lockcntfixamb",  0);
    FixCntHoldAmb      =ini->ReadInteger("opt","fixcntholdamb", 10);
    ElMaskAR           =ini->ReadFloat  ("opt","elmaskar",     0.0);
    ElMaskHold         =ini->ReadFloat  ("opt","elmaskhold",   0.0);
    OutCntResetAmb     =ini->ReadInteger("opt","outcntresetbias",5);
    SlipThres          =ini->ReadFloat  ("opt","slipthres",   0.05);
    MaxAgeDiff         =ini->ReadFloat  ("opt","maxagediff",  30.0);
    RejectThres        =ini->ReadFloat  ("opt","rejectthres", 30.0);
    RejectGdop         =ini->ReadFloat  ("opt","rejectgdop",  30.0);
    ARIter             =ini->ReadInteger("opt","ariter",         1);
    NumIter            =ini->ReadInteger("opt","numiter",        1);
    CodeSmooth         =ini->ReadInteger("opt","codesmooth",     0);
    BaseLine[0]        =ini->ReadFloat  ("opt","baselinelen",  0.0);
    BaseLine[1]        =ini->ReadFloat  ("opt","baselinesig",  0.0);
    BaseLineConst      =ini->ReadInteger("opt","baselineconst",  0);
    
    SolFormat          =ini->ReadInteger("opt","solformat",      0);
    TimeFormat         =ini->ReadInteger("opt","timeformat",     1);
    TimeDecimal        =ini->ReadInteger("opt","timedecimal",    3);
    LatLonFormat       =ini->ReadInteger("opt","latlonformat",   0);
    FieldSep           =ini->ReadString ("opt","fieldsep",      "");
    OutputHead         =ini->ReadInteger("opt","outputhead",     1);
    OutputOpt          =ini->ReadInteger("opt","outputopt",      1);
    OutputSingle       =ini->ReadInteger("opt","outputsingle",   0);
    MaxSolStd          =ini->ReadFloat  ("opt","maxsolstd",    0.0);
    OutputDatum        =ini->ReadInteger("opt","outputdatum",    0);
    OutputHeight       =ini->ReadInteger("opt","outputheight",   0);
    OutputGeoid        =ini->ReadInteger("opt","outputgeoid",    0);
    SolStatic          =ini->ReadInteger("opt","solstatic",      0);
    DebugTrace         =ini->ReadInteger("opt","debugtrace",     0);
    DebugStatus        =ini->ReadInteger("opt","debugstatus",    0);
    
    MeasErrR1          =ini->ReadFloat  ("opt","measeratio1",100.0);
    MeasErrR2          =ini->ReadFloat  ("opt","measeratio2",100.0);
    MeasErr2           =ini->ReadFloat  ("opt","measerr2",   0.003);
    MeasErr3           =ini->ReadFloat  ("opt","measerr3",   0.003);
    MeasErr4           =ini->ReadFloat  ("opt","measerr4",   0.000);
    MeasErr5           =ini->ReadFloat  ("opt","measerr5",  10.000);
    SatClkStab         =ini->ReadFloat  ("opt","satclkstab", 5E-12);
    PrNoise1           =ini->ReadFloat  ("opt","prnoise1",    1E-4);
    PrNoise2           =ini->ReadFloat  ("opt","prnoise2",    1E-3);
    PrNoise3           =ini->ReadFloat  ("opt","prnoise3",    1E-4);
    PrNoise4           =ini->ReadFloat  ("opt","prnoise4",    1E+1);
    PrNoise5           =ini->ReadFloat  ("opt","prnoise5",    1E+1);
    
    RovPosType         =ini->ReadInteger("opt","rovpostype",     0);
    RefPosType         =ini->ReadInteger("opt","refpostype",     0);
    RovPos[0]          =ini->ReadFloat  ("opt","rovpos1",      0.0);
    RovPos[1]          =ini->ReadFloat  ("opt","rovpos2",      0.0);
    RovPos[2]          =ini->ReadFloat  ("opt","rovpos3",      0.0);
    RefPos[0]          =ini->ReadFloat  ("opt","refpos1",      0.0);
    RefPos[1]          =ini->ReadFloat  ("opt","refpos2",      0.0);
    RefPos[2]          =ini->ReadFloat  ("opt","refpos3",      0.0);
    RovAntPcv          =ini->ReadInteger("opt","rovantpcv",      0);
    RefAntPcv          =ini->ReadInteger("opt","refantpcv",      0);
    RovAnt             =ini->ReadString ("opt","rovant",        "");
    RefAnt             =ini->ReadString ("opt","refant",        "");
    RovAntE            =ini->ReadFloat  ("opt","rovante",      0.0);
    RovAntN            =ini->ReadFloat  ("opt","rovantn",      0.0);
    RovAntU            =ini->ReadFloat  ("opt","rovantu",      0.0);
    RefAntE            =ini->ReadFloat  ("opt","refante",      0.0);
    RefAntN            =ini->ReadFloat  ("opt","refantn",      0.0);
    RefAntU            =ini->ReadFloat  ("opt","refantu",      0.0);
    
    RnxOpts1           =ini->ReadString ("opt","rnxopts1",      "");
    RnxOpts2           =ini->ReadString ("opt","rnxopts2",      "");
    PPPOpts            =ini->ReadString ("opt","pppopts",       "");
    
    AntPcvFile         =ini->ReadString ("opt","antpcvfile",    "");
    IntpRefObs         =ini->ReadInteger("opt","intprefobs",     0);
    SbasSat            =ini->ReadInteger("opt","sbassat",        0);
    NetRSCorr          =ini->ReadInteger("opt","netrscorr",      0);
    SatClkCorr         =ini->ReadInteger("opt","satclkcorr",     0);
    SbasCorr           =ini->ReadInteger("opt","sbascorr",       0);
    SbasCorr1          =ini->ReadInteger("opt","sbascorr1",      0);
    SbasCorr2          =ini->ReadInteger("opt","sbascorr2",      0);
    SbasCorr3          =ini->ReadInteger("opt","sbascorr3",      0);
    SbasCorr4          =ini->ReadInteger("opt","sbascorr4",      0);
    SbasCorrFile       =ini->ReadString ("opt","sbascorrfile",  "");
    PrecEphFile        =ini->ReadString ("opt","precephfile",   "");
    SatPcvFile         =ini->ReadString ("opt","satpcvfile",    "");
    StaPosFile         =ini->ReadString ("opt","staposfile",    "");
    GeoidDataFile      =ini->ReadString ("opt","geoiddatafile", "");
    IonoFile           =ini->ReadString ("opt","ionofile",      "");
    EOPFile            =ini->ReadString ("opt","eopfile",       "");
    DCBFile            =ini->ReadString ("opt","dcbfile",       "");
    BLQFile            =ini->ReadString ("opt","blqfile",       "");
    GoogleEarthFile    =ini->ReadString ("opt","googleearthfile",GOOGLE_EARTH);
    
    RovList="";
    for (int i=0;i<10;i++) {
        RovList +=ini->ReadString("opt",s.sprintf("rovlist%d",i+1),"");
    }
    BaseList="";
    for (int i=0;i<10;i++) {
        BaseList+=ini->ReadString("opt",s.sprintf("baselist%d",i+1),"");
    }
    for (p=RovList.c_str();*p;p++) {
        if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
    }
    for (p=BaseList.c_str();*p;p++) {
        if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
    }
    ExtErr.ena[0]      =ini->ReadInteger("opt","exterr_ena0",    0);
    ExtErr.ena[1]      =ini->ReadInteger("opt","exterr_ena1",    0);
    ExtErr.ena[2]      =ini->ReadInteger("opt","exterr_ena2",    0);
    ExtErr.ena[3]      =ini->ReadInteger("opt","exterr_ena3",    0);
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ExtErr.cerr[i][j]=ini->ReadFloat("opt",s.sprintf("exterr_cerr%d%d",i,j),0.3);
    }
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ExtErr.perr[i][j]=ini->ReadFloat("opt",s.sprintf("exterr_perr%d%d",i,j),0.003);
    }
    ExtErr.gloicb[0]   =ini->ReadFloat  ("opt","exterr_gloicb0",0.0);
    ExtErr.gloicb[1]   =ini->ReadFloat  ("opt","exterr_gloicb1",0.0);
    ExtErr.gpsglob[0]  =ini->ReadFloat  ("opt","exterr_gpsglob0",0.0);
    ExtErr.gpsglob[1]  =ini->ReadFloat  ("opt","exterr_gpsglob1",0.0);
    
    ConvDialog->TimeSpan  ->Checked  =ini->ReadInteger("conv","timespan",  0);
    ConvDialog->TimeIntF  ->Checked  =ini->ReadInteger("conv","timeintf",  0);
    ConvDialog->TimeY1    ->Text     =ini->ReadString ("conv","timey1","2000/01/01");
    ConvDialog->TimeH1    ->Text     =ini->ReadString ("conv","timeh1","00:00:00"  );
    ConvDialog->TimeY2    ->Text     =ini->ReadString ("conv","timey2","2000/01/01");
    ConvDialog->TimeH2    ->Text     =ini->ReadString ("conv","timeh2","00:00:00"  );
    ConvDialog->TimeInt   ->Text     =ini->ReadString ("conv","timeint", "0");
    ConvDialog->TrackColor->ItemIndex=ini->ReadInteger("conv","trackcolor",5);
    ConvDialog->PointColor->ItemIndex=ini->ReadInteger("conv","pointcolor",5);
    ConvDialog->OutputAlt ->ItemIndex=ini->ReadInteger("conv","outputalt", 0);
    ConvDialog->OutputTime->ItemIndex=ini->ReadInteger("conv","outputtime",0);
    ConvDialog->AddOffset ->Checked  =ini->ReadInteger("conv","addoffset", 0);
    ConvDialog->Offset1   ->Text     =ini->ReadString ("conv","offset1", "0");
    ConvDialog->Offset2   ->Text     =ini->ReadString ("conv","offset2", "0");
    ConvDialog->Offset3   ->Text     =ini->ReadString ("conv","offset3", "0");
    ConvDialog->Compress  ->Checked  =ini->ReadInteger("conv","compress",  0);
    ConvDialog->FormatKML ->Checked  =ini->ReadInteger("conv","format",    0);
    
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    Width=ini->ReadInteger("window","width",486);
    delete ini;
}
// save options to ini file -------------------------------------------------
void __fastcall TMainForm::SaveOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s;
    char *p;
    
    ini->WriteInteger("set","timestart",   TimeStart ->Checked?1:0);
    ini->WriteInteger("set","timeend",     TimeEnd   ->Checked?1:0);
    ini->WriteString ("set","timey1",      TimeY1    ->Text);
    ini->WriteString ("set","timeh1",      TimeH1    ->Text);
    ini->WriteString ("set","timey2",      TimeY2    ->Text);
    ini->WriteString ("set","timeh2",      TimeH2    ->Text);
    ini->WriteInteger("set","timeintf",    TimeIntF  ->Checked?1:0);
    ini->WriteString ("set","timeint",     TimeInt   ->Text);
    ini->WriteInteger("set","timeunitf",   TimeUnitF ->Checked?1:0);
    ini->WriteString ("set","timeunit",    TimeUnit  ->Text);
    ini->WriteString ("set","inputfile1",  InputFile1->Text);
    ini->WriteString ("set","inputfile2",  InputFile2->Text);
    ini->WriteString ("set","inputfile3",  InputFile3->Text);
    ini->WriteString ("set","inputfile4",  InputFile4->Text);
    ini->WriteString ("set","inputfile5",  InputFile5->Text);
    ini->WriteString ("set","inputfile6",  InputFile6->Text);
    ini->WriteInteger("set","outputdirena",OutDirEna ->Checked);
    ini->WriteString ("set","outputdir",   OutDir    ->Text);
    ini->WriteString ("set","outputfile",  OutputFile->Text);
    
    WriteList(ini,"hist","inputfile1",     InputFile1->Items);
    WriteList(ini,"hist","inputfile2",     InputFile2->Items);
    WriteList(ini,"hist","inputfile3",     InputFile3->Items);
    WriteList(ini,"hist","inputfile4",     InputFile4->Items);
    WriteList(ini,"hist","inputfile5",     InputFile5->Items);
    WriteList(ini,"hist","inputfile6",     InputFile6->Items);
    WriteList(ini,"hist","outputfile",     OutputFile->Items);
    
    ini->WriteInteger("opt","posmode",     PosMode     );
    ini->WriteInteger("opt","freq",        Freq        );
    ini->WriteInteger("opt","solution",    Solution    );
    ini->WriteFloat  ("opt","elmask",      ElMask      );
    ini->WriteInteger("opt","snrmask_ena1",SnrMask.ena[0]);
    ini->WriteInteger("opt","snrmask_ena2",SnrMask.ena[1]);
    for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
        ini->WriteFloat("opt",s.sprintf("snrmask_%d_%d",i+1,j+1),
                        SnrMask.mask[i][j]);
    }
    ini->WriteInteger("opt","ionoopt",     IonoOpt     );
    ini->WriteInteger("opt","tropopt",     TropOpt     );
    ini->WriteInteger("opt","rcvbiasest",  RcvBiasEst  );
    ini->WriteInteger("opt","dynamicmodel",DynamicModel);
    ini->WriteInteger("opt","tidecorr",    TideCorr    );
    ini->WriteInteger("opt","satephem",    SatEphem    );
    ini->WriteString ("opt","exsats",      ExSats      );
    ini->WriteInteger("opt","navsys",      NavSys      );
    ini->WriteInteger("opt","posopt1",     PosOpt[0]   );
    ini->WriteInteger("opt","posopt2",     PosOpt[1]   );
    ini->WriteInteger("opt","posopt3",     PosOpt[2]   );
    ini->WriteInteger("opt","posopt4",     PosOpt[3]   );
    ini->WriteInteger("opt","posopt5",     PosOpt[4]   );
    ini->WriteInteger("opt","posopt6",     PosOpt[5]   );
    ini->WriteInteger("opt","mapfunc",     MapFunc     );
    
    ini->WriteInteger("opt","ambres",      AmbRes      );
    ini->WriteInteger("opt","gloambres",   GloAmbRes   );
    ini->WriteInteger("opt","bdsambres",   BdsAmbRes   );
    ini->WriteFloat  ("opt","validthresar",ValidThresAR);
    ini->WriteFloat  ("opt","thresar2",    ThresAR2    );
    ini->WriteFloat  ("opt","thresar3",    ThresAR3    );
    ini->WriteInteger("opt","lockcntfixamb",LockCntFixAmb);
    ini->WriteInteger("opt","fixcntholdamb",FixCntHoldAmb);
    ini->WriteFloat  ("opt","elmaskar",    ElMaskAR    );
    ini->WriteFloat  ("opt","elmaskhold",  ElMaskHold  );
    ini->WriteInteger("opt","outcntresetbias",OutCntResetAmb);
    ini->WriteFloat  ("opt","slipthres",   SlipThres   );
    ini->WriteFloat  ("opt","maxagediff",  MaxAgeDiff  );
    ini->WriteFloat  ("opt","rejectgdop",  RejectGdop  );
    ini->WriteFloat  ("opt","rejectthres", RejectThres );
    ini->WriteInteger("opt","ariter",      ARIter      );
    ini->WriteInteger("opt","numiter",     NumIter     );
    ini->WriteInteger("opt","codesmooth",  CodeSmooth  );
    ini->WriteFloat  ("opt","baselinelen", BaseLine[0] );
    ini->WriteFloat  ("opt","baselinesig", BaseLine[1] );
    ini->WriteInteger("opt","baselineconst",BaseLineConst);
    
    ini->WriteInteger("opt","solformat",   SolFormat   );
    ini->WriteInteger("opt","timeformat",  TimeFormat  );
    ini->WriteInteger("opt","timedecimal", TimeDecimal );
    ini->WriteInteger("opt","latlonformat",LatLonFormat);
    ini->WriteString ("opt","fieldsep",    FieldSep    );
    ini->WriteInteger("opt","outputhead",  OutputHead  );
    ini->WriteInteger("opt","outputopt",   OutputOpt   );
    ini->WriteInteger("opt","outputsingle",OutputSingle);
    ini->WriteFloat  ("opt","maxsolstd",   MaxSolStd   );
    ini->WriteInteger("opt","outputdatum", OutputDatum );
    ini->WriteInteger("opt","outputheight",OutputHeight);
    ini->WriteInteger("opt","outputgeoid", OutputGeoid );
    ini->WriteInteger("opt","solstatic",   SolStatic   );
    ini->WriteInteger("opt","debugtrace",  DebugTrace  );
    ini->WriteInteger("opt","debugstatus", DebugStatus );
    
    ini->WriteFloat  ("opt","measeratio1", MeasErrR1   );
    ini->WriteFloat  ("opt","measeratio2", MeasErrR2   );
    ini->WriteFloat  ("opt","measerr2",    MeasErr2    );
    ini->WriteFloat  ("opt","measerr3",    MeasErr3    );
    ini->WriteFloat  ("opt","measerr4",    MeasErr4    );
    ini->WriteFloat  ("opt","measerr5",    MeasErr5    );
    ini->WriteFloat  ("opt","satclkstab",  SatClkStab  );
    ini->WriteFloat  ("opt","prnoise1",    PrNoise1    );
    ini->WriteFloat  ("opt","prnoise2",    PrNoise2    );
    ini->WriteFloat  ("opt","prnoise3",    PrNoise3    );
    ini->WriteFloat  ("opt","prnoise4",    PrNoise4    );
    ini->WriteFloat  ("opt","prnoise5",    PrNoise5    );
    
    ini->WriteInteger("opt","rovpostype",  RovPosType  );
    ini->WriteInteger("opt","refpostype",  RefPosType  );
    ini->WriteFloat  ("opt","rovpos1",     RovPos[0]   );
    ini->WriteFloat  ("opt","rovpos2",     RovPos[1]   );
    ini->WriteFloat  ("opt","rovpos3",     RovPos[2]   );
    ini->WriteFloat  ("opt","refpos1",     RefPos[0]   );
    ini->WriteFloat  ("opt","refpos2",     RefPos[1]   );
    ini->WriteFloat  ("opt","refpos3",     RefPos[2]   );
    ini->WriteInteger("opt","rovantpcv",   RovAntPcv   );
    ini->WriteInteger("opt","refantpcv",   RefAntPcv   );
    ini->WriteString ("opt","rovant",      RovAnt      );
    ini->WriteString ("opt","refant",      RefAnt      );
    ini->WriteFloat  ("opt","rovante",     RovAntE     );
    ini->WriteFloat  ("opt","rovantn",     RovAntN     );
    ini->WriteFloat  ("opt","rovantu",     RovAntU     );
    ini->WriteFloat  ("opt","refante",     RefAntE     );
    ini->WriteFloat  ("opt","refantn",     RefAntN     );
    ini->WriteFloat  ("opt","refantu",     RefAntU     );
    
    ini->WriteString ("opt","rnxopts1",    RnxOpts1    );
    ini->WriteString ("opt","rnxopts2",    RnxOpts2    );
    ini->WriteString ("opt","pppopts",     PPPOpts     );
    
    ini->WriteString ("opt","antpcvfile",  AntPcvFile  );
    ini->WriteInteger("opt","intprefobs",  IntpRefObs  );
    ini->WriteInteger("opt","sbassat",     SbasSat     );
    ini->WriteInteger("opt","netrscorr",   NetRSCorr   );
    ini->WriteInteger("opt","satclkcorr",  SatClkCorr  );
    ini->WriteInteger("opt","sbascorr",    SbasCorr    );
    ini->WriteInteger("opt","sbascorr1",   SbasCorr1   );
    ini->WriteInteger("opt","sbascorr2",   SbasCorr2   );
    ini->WriteInteger("opt","sbascorr3",   SbasCorr3   );
    ini->WriteInteger("opt","sbascorr4",   SbasCorr4   );
    ini->WriteString ("opt","sbascorrfile",SbasCorrFile);
    ini->WriteString ("opt","precephfile", PrecEphFile );
    ini->WriteString ("opt","satpcvfile",  SatPcvFile  );
    ini->WriteString ("opt","staposfile",  StaPosFile  );
    ini->WriteString ("opt","geoiddatafile",GeoidDataFile);
    ini->WriteString ("opt","ionofile",    IonoFile    );
    ini->WriteString ("opt","eopfile",     EOPFile     );
    ini->WriteString ("opt","dcbfile",     DCBFile     );
    ini->WriteString ("opt","blqfile",     BLQFile     );
    ini->WriteString ("opt","googleearthfile",GoogleEarthFile);
    
    for (p=RovList.c_str();*p;p++) {
        if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
    }
    for (int i=0;i<10;i++) {
        ini->WriteString("opt",s.sprintf("rovlist%d",i+1),RovList.SubString(i*2000,2000));
    }
    for (p=BaseList.c_str();*p;p++) {
        if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
    }
    for (int i=0;i<10;i++) {
        ini->WriteString("opt",s.sprintf("baselist%d",i+1),BaseList.SubString(i*2000,2000));
    }
    ini->WriteInteger("opt","exterr_ena0", ExtErr.ena[0]);
    ini->WriteInteger("opt","exterr_ena1", ExtErr.ena[1]);
    ini->WriteInteger("opt","exterr_ena2", ExtErr.ena[2]);
    ini->WriteInteger("opt","exterr_ena3", ExtErr.ena[3]);
    
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ini->WriteFloat("opt",s.sprintf("exterr_cerr%d%d",i,j),ExtErr.cerr[i][j]);
    }
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ini->WriteFloat("opt",s.sprintf("exterr_perr%d%d",i,j),ExtErr.perr[i][j]);
    }
    ini->WriteFloat  ("opt","exterr_gloicb0",ExtErr.gloicb[0]);
    ini->WriteFloat  ("opt","exterr_gloicb1",ExtErr.gloicb[1]);
    ini->WriteFloat  ("opt","exterr_gpsglob0",ExtErr.gpsglob[0]);
    ini->WriteFloat  ("opt","exterr_gpsglob1",ExtErr.gpsglob[1]);
    
    ini->WriteInteger("conv","timespan",   ConvDialog->TimeSpan  ->Checked  );
    ini->WriteString ("conv","timey1",     ConvDialog->TimeY1    ->Text     );
    ini->WriteString ("conv","timeh1",     ConvDialog->TimeH1    ->Text     );
    ini->WriteString ("conv","timey2",     ConvDialog->TimeY2    ->Text     );
    ini->WriteString ("conv","timeh2",     ConvDialog->TimeH2    ->Text     );
    ini->WriteInteger("conv","timeintf",   ConvDialog->TimeIntF  ->Checked  );
    ini->WriteString ("conv","timeint",    ConvDialog->TimeInt   ->Text     );
    ini->WriteInteger("conv","trackcolor", ConvDialog->TrackColor->ItemIndex);
    ini->WriteInteger("conv","pointcolor", ConvDialog->PointColor->ItemIndex);
    ini->WriteInteger("conv","outputalt",  ConvDialog->OutputAlt ->ItemIndex);
    ini->WriteInteger("conv","outputtime", ConvDialog->OutputTime->ItemIndex);
    ini->WriteInteger("conv","addoffset",  ConvDialog->AddOffset ->Checked  );
    ini->WriteString ("conv","offset1",    ConvDialog->Offset1   ->Text     );
    ini->WriteString ("conv","offset2",    ConvDialog->Offset2   ->Text     );
    ini->WriteString ("conv","offset3",    ConvDialog->Offset3   ->Text     );
    ini->WriteInteger("conv","compress",   ConvDialog->Compress  ->Checked  );
    ini->WriteInteger("conv","format",     ConvDialog->FormatKML ->Checked  );
    
    ini->WriteInteger("viewer","color1",(int)TTextViewer::Color1  );
    ini->WriteInteger("viewer","color2",(int)TTextViewer::Color2  );
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    
    ini->WriteInteger("window","width",Width);
    delete ini;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::Panel4Resize(TObject *Sender)
{
	TButton *btns[]={
		BtnInputFile1,BtnInputFile2,BtnInputFile3,BtnInputFile4,
		BtnInputFile5,BtnInputFile6
	};
	TComboBox *boxes[]={
		InputFile1,InputFile2,InputFile3,InputFile4,InputFile5,InputFile6
	};
	int w=Panel4->Width;
	
	for (int i=0;i<6;i++) {
		btns[i]->Left=w-btns[i]->Width-5;
		boxes[i]->Width=w-btns[i]->Width-boxes[i]->Left-6;
	}
	BtnInputView1->Left=InputFile1->Left+InputFile1->Width-BtnInputView1->Width;
	BtnInputPlot1->Left=BtnInputView1->Left-BtnInputPlot1->Width;
	BtnInputView2->Left=InputFile2->Left+InputFile2->Width-BtnInputView2->Width;
	BtnInputPlot2->Left=BtnInputView2->Left-BtnInputPlot2->Width;
	BtnInputView6->Left=InputFile3->Left+InputFile3->Width-BtnInputView6->Width;
	BtnInputView5->Left=BtnInputView6->Left-BtnInputView5->Width;
	BtnInputView4->Left=BtnInputView5->Left-BtnInputView4->Width;
	BtnInputView3->Left=BtnInputView4->Left-BtnInputView3->Width;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Panel5Resize(TObject *Sender)
{
	int w=Panel5->Width;
	
	BtnOutDir->Left=w-BtnOutDir->Width-5;
	OutDir->Width=w-BtnOutDir->Width-OutDir->Left-6;
	BtnOutputFile->Left=w-BtnOutputFile->Width-5;
	OutputFile->Width=w-BtnOutputFile->Width-OutputFile->Left-6;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Panel2Resize(TObject *Sender)
{
	TBitBtn *btns[]={
		BtnPlot,BtnView,BtnToKML,BtnOption,BtnExec,BtnExit
	};
	int w=(Panel2->Width-2)/6;
	
	for (int i=0;i<6;i++) {
		btns[i]->Width=w;
		btns[i]->Left=i*w+1;
	}
	BtnAbort->Width=BtnExec->Width;
	BtnAbort->Left =BtnExec->Left;
}
//---------------------------------------------------------------------------

