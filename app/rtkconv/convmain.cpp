//---------------------------------------------------------------------------
// rtkconv : rinex translator
//
//			Copyright (C) 2007-2016 by T.TAKASU, All rights reserved.
//
// options : rtkconv [-t title][-i file]
//
//			 -t title	window title
//			 -i file	ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//			 2010/07/18  1.1 rtklib 2.4.0
//			 2011/06/10  1.2 rtklib 2.4.1
//			 2013/04/01  1.3 rtklib 2.4.2
//			 2016/10/10  1.4 rtklib 2.4.3
//---------------------------------------------------------------------------
#include <vcl.h>

#pragma hdrstop

#include "convmain.h"
#include "timedlg.h"
#include "confdlg.h"
#include "aboutdlg.h"
#include "startdlg.h"
#include "keydlg.h"
#include "convopt.h"
#include "viewer.h"
#include "rtklib.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TMainWindow *MainWindow;

#define PRGNAME		"RTKCONV"  // program name
#define MAXHIST		20		   // max number of histories
#define TSTARTMARGIN 60.0	   // time margin for file name replacement
#define TRACEFILE	"rtkconv.trace" // trace file

static int abortf=0;

// show message in message area ---------------------------------------------
extern "C" {
extern int showmsg(char *format,...)
{
	static int i=0;
	va_list arg;
	char buff[1024];
	va_start(arg,format); vsprintf(buff,format,arg); va_end(arg);
	MainWindow->Message->Caption=buff;
	if (++i%100==0) Application->ProcessMessages();
	return abortf;
}
}
// constructor --------------------------------------------------------------
__fastcall TMainWindow::TMainWindow(TComponent* Owner)
	: TForm(Owner)
{
	gtime_t time0={0};
	int i;
	char file[1024]="rtkconv.exe",*p;
	
	::GetModuleFileName(NULL,file,sizeof(file));
	if (!(p=strrchr(file,'.'))) p=file+strlen(file);
	strcpy(p,".ini");
	IniFile=file;
	
	DoubleBuffered=true;
	Format->Items->Clear();
	Format->Items->Add("Auto");
	for (i=0;i<=MAXRCVFMT;i++) {
		Format->Items->Add(formatstrs[i]);
	}
	Format->Items->Add(formatstrs[STRFMT_RINEX]);
	RnxTime=time0;
	EventEna=0;
}
// callback on form create --------------------------------------------------
void __fastcall TMainWindow::FormCreate(TObject *Sender)
{
	AnsiString s;
	
	Caption=s.sprintf("%s ver.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
	
	::DragAcceptFiles(Handle,true);
}
// callback on form show ----------------------------------------------------
void __fastcall TMainWindow::FormShow(TObject *Sender)
{
	char *p,*argv[32],buff[1024];
	int argc=0;
	
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
	EventEna=1;
}
// callback on form close ---------------------------------------------------
void __fastcall TMainWindow::FormClose(TObject *Sender, TCloseAction &Action)
{
	SaveOpt();
}
// set output file paths ----------------------------------------------------
void __fastcall TMainWindow::SetOutFiles(AnsiString infile)
{
	TEdit *edit[]={
		OutFile1,OutFile2,OutFile3,OutFile4,OutFile5,OutFile6,OutFile7,
		OutFile8,OutFile9
	};
	AnsiString Format_Text=Format->Text;
	AnsiString OutDir_Text=OutDir->Text;
	char *ifile,ofile[10][1024],*code,*p;
	int i,lex=strstr(Format_Text.c_str(),"LEX")!=NULL;
	
	if (!EventEna) return;
	
	ifile=infile.c_str();
	if (OutDirEna->Checked) {
		if ((p=strrchr(ifile,'\\'))) p++; else p=ifile;
		sprintf(ofile[0],"%s\\%s",OutDir_Text.c_str(),p);
	}
	else {
		strcpy(ofile[0],ifile);
	}
	for (p=ofile[0];*p;p++) {
		if (*p=='*'||*p=='?') *p='0';
	}
	if (!RnxFile) {
		if ((p=strrchr(ofile[0],'.'))) *p='\0';
		sprintf(ofile[1],"%s.obs" ,ofile[0]);
		sprintf(ofile[2],"%s.nav" ,ofile[0]);
		sprintf(ofile[3],"%s.gnav",ofile[0]);
		sprintf(ofile[4],"%s.hnav",ofile[0]);
		sprintf(ofile[5],"%s.qnav",ofile[0]);
		sprintf(ofile[6],"%s.lnav",ofile[0]);
		sprintf(ofile[7],"%s.cnav",ofile[0]);
		sprintf(ofile[8],"%s.inav",ofile[0]);
		sprintf(ofile[9],lex?"%s.lex":"%s.sbs",ofile[0]);
	}
	else {
		if ((p=strrchr(ofile[0],'\\'))) *(p+1)='\0';
		else ofile[0][0]='\0';
		sprintf(ofile[1],"%s%%r%%n0.%%yO",ofile[0]);
		if (RnxVer>=3&&NavSys&&(NavSys!=SYS_GPS)) { /* ver.3 and mixed system */
			sprintf(ofile[2],"%s%%r%%n0.%%yP",ofile[0]);
		}
		else {
			sprintf(ofile[2],"%s%%r%%n0.%%yN",ofile[0]);
		}
		sprintf(ofile[3],"%s%%r%%n0.%%yG",ofile[0]);
		sprintf(ofile[4],"%s%%r%%n0.%%yH",ofile[0]);
		sprintf(ofile[5],"%s%%r%%n0.%%yQ",ofile[0]);
		sprintf(ofile[6],"%s%%r%%n0.%%yL",ofile[0]);
		sprintf(ofile[7],"%s%%r%%n0.%%yC",ofile[0]);
		sprintf(ofile[8],"%s%%r%%n0.%%yI",ofile[0]);
		sprintf(ofile[9],lex?"%s%%r%%n0_%%y.lex":"%s%%r%%n0_%%y.sbs",ofile[0]);
	}
	for (i=0;i<9;i++) {
		if (!strcmp(ofile[i+1],ifile)) strcat(ofile[i+1],"_");
		edit[i]->Text=ofile[i+1];
	}
}
// callback on file drag and drop -------------------------------------------
void __fastcall TMainWindow::DropFiles(TWMDropFiles msg)
{
	char *p,str[1024];
	
	if (DragQueryFile((HDROP)msg.Drop,0xFFFFFFFF,NULL,0)<=0) return;
	DragQueryFile((HDROP)msg.Drop,0,str,sizeof(str));
	InFile->Text=str;
	SetOutFiles(InFile->Text);
}
// add history --------------------------------------------------------------
void __fastcall TMainWindow::AddHist(TComboBox *combo)
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
// read histroy from ini-file -----------------------------------------------
TStringList * __fastcall TMainWindow::ReadList(TIniFile *ini, AnsiString cat,
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
// save history to ini-file -------------------------------------------------
void __fastcall TMainWindow::WriteList(TIniFile *ini, AnsiString cat,
	AnsiString key, TStrings *list)
{
	AnsiString s;
	int i;
	
	for (i=0;i<list->Count;i++) {
		ini->WriteString(cat,s.sprintf("%s_%03d",key.c_str(),i),list->Strings[i]);
	}
}
// callback on button-plot --------------------------------------------------
void __fastcall TMainWindow::BtnPlotClick(TObject *Sender)
{
	AnsiString file1=OutFile1->Text;
	AnsiString file2=OutFile2->Text;
	AnsiString file3=OutFile3->Text;
	AnsiString file4=OutFile4->Text;
	AnsiString file5=OutFile5->Text;
	AnsiString file6=OutFile6->Text;
	AnsiString file7=OutFile7->Text;
	AnsiString file8=OutFile8->Text;
	AnsiString file[]={file1,file2,file3,file4,file5,file6,file7,file8};
	AnsiString cmd1="rtkplot",cmd2="..\\..\\..\\bin\\rtkplot",opts=" -r";
	TCheckBox *cb[]={
		OutFileEna1,OutFileEna2,OutFileEna3,OutFileEna4,OutFileEna5,OutFileEna6,
		OutFileEna7,OutFileEna8
	};
	int i,ena[8];
	
	for (i=0;i<8;i++) ena[i]=cb[i]->Enabled&&cb[i]->Checked;
	
	for (i=0;i<8;i++) {
		if (ena[i]) opts=opts+" \""+RepPath(file[i])+"\"";
	}
	if (opts==" -r") return;
	
	if (!ExecCmd(cmd1+opts)&&!ExecCmd(cmd2+opts)) {
		Message->Caption="error : rtkplot execution";
	}
}
// callback on button-post-proc ---------------------------------------------
void __fastcall TMainWindow::BtnPostClick(TObject *Sender)
{
	AnsiString path2="..\\..\\..\\bin\\";
	AnsiString cmd1=CmdPostExe,cmd2=path2+CmdPostExe,opts=" ";
	
	if (!OutFileEna1->Checked) return;
	
	opts=opts+" -r \""+OutFile1->Text+"\"";
	opts=opts+" -n \"\" -n \"\"";
	
	if (OutFileEna7->Checked) {
		opts=opts+" -n \""+OutFile7->Text+"\"";
	}
	if (TimeStartF->Checked) opts=opts+" -ts "+TimeY1->Text+" "+TimeH1->Text;
	if (TimeEndF  ->Checked) opts=opts+" -te "+TimeY2->Text+" "+TimeH2->Text;
	if (TimeIntF  ->Checked) opts=opts+" -ti "+TimeInt->Text;
	if (TimeUnitF ->Checked) opts=opts+" -tu "+TimeUnit->Text;
	
	if (!ExecCmd(cmd1+opts)&&!ExecCmd(cmd2+opts)) {
		Message->Caption="error : rtkpost execution";
	}
}
// callback on button-options -----------------------------------------------
void __fastcall TMainWindow::BtnOptionsClick(TObject *Sender)
{
	int rnxfile=RnxFile;
	if (ConvOptDialog->ShowModal()!=mrOk) return;
	if (RnxFile!=rnxfile) {
		SetOutFiles(InFile->Text);
	}
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BtnAbortClick(TObject *Sender)
{
	abortf=1;
}
// callback on button-convert -----------------------------------------------
void __fastcall TMainWindow::BtnConvertClick(TObject *Sender)
{
	ConvertFile();
}
// callback on button-exit --------------------------------------------------
void __fastcall TMainWindow::BtnExitClick(TObject *Sender)
{
	Close();
}
// callbck on button-time-1 -------------------------------------------------
void __fastcall TMainWindow::BtnTime1Click(TObject *Sender)
{
	gtime_t ts={0},te={0};
	double tint=0.0,tunit=0.0;
	GetTime(&ts,&te,&tint,&tunit);
	TimeDialog->Time=ts;
	TimeDialog->ShowModal();
}
// callbck on button-time-2 -------------------------------------------------
void __fastcall TMainWindow::BtnTime2Click(TObject *Sender)
{
	gtime_t ts={0},te={0};
	double tint=0.0,tunit=0.0;
	GetTime(&ts,&te,&tint,&tunit);
	TimeDialog->Time=te;
	TimeDialog->ShowModal();
}
// callback on button-input-file --------------------------------------------
void __fastcall TMainWindow::BtnInFileClick(TObject *Sender)
{
	OpenDialog->Title="Input RTCM, RCV RAW or RINEX OBS File";
	OpenDialog->FileName="";
	if (!OpenDialog->Execute()) return;
	InFile->Text=OpenDialog->FileName;
	SetOutFiles(InFile->Text);
}
// callback on output-directory change --------------------------------------
void __fastcall TMainWindow::OutDirChange(TObject *Sender)
{
	SetOutFiles(InFile->Text);
}
// callback on button-output-directory --------------------------------------
void __fastcall TMainWindow::BtnOutDirClick(TObject *Sender)
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
// callback on button-keyword -----------------------------------------------
void __fastcall TMainWindow::BtnKeyClick(TObject *Sender)
{
	KeyDialog->Flag=1;
	KeyDialog->Show();
}
// callback on button-output-file-1 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile1Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX OBS File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=2;
	if (!OpenDialog2->Execute()) return;
	OutFile1->Text=OpenDialog2->FileName;
}
// callback on button-output-file-2 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile2Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX NAV File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=3;
	if (!OpenDialog2->Execute()) return;
	OutFile2->Text=OpenDialog2->FileName;
}
// callback on button-output-file-3 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile3Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX GNAV File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=4;
	if (!OpenDialog2->Execute()) return;
	OutFile3->Text=OpenDialog2->FileName;
}
// callback on button-output-file-4 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile4Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX HNAV File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=5;
	if (!OpenDialog2->Execute()) return;
	OutFile4->Text=OpenDialog2->FileName;
}
// callback on button-output-file-5 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile5Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX QNAV File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=6;
	if (!OpenDialog2->Execute()) return;
	OutFile5->Text=OpenDialog2->FileName;
}
// callback on button-output-file-6 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile6Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX LNAV File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=7;
	if (!OpenDialog2->Execute()) return;
	OutFile6->Text=OpenDialog2->FileName;
}
// callback on button-output-file-7 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile7Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX CNAV File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=8;
	if (!OpenDialog2->Execute()) return;
	OutFile7->Text=OpenDialog2->FileName;
}
// callback on button-output-file-8 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile8Click(TObject *Sender)
{
	OpenDialog2->Title="Output RINEX INAV File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=9;
	if (!OpenDialog2->Execute()) return;
	OutFile8->Text=OpenDialog2->FileName;
}
// callback on button-output-file-9 -----------------------------------------
void __fastcall TMainWindow::BtnOutFile9Click(TObject *Sender)
{
	OpenDialog2->Title="Output SBAS/LEX Log File";
	OpenDialog2->FileName="";
	OpenDialog2->FilterIndex=10;
	if (!OpenDialog2->Execute()) return;
	OutFile9->Text=OpenDialog2->FileName;
}
// callback on button-view-input-file ----------------------------------------
void __fastcall TMainWindow::BtnInFileViewClick(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString InFile_Text=InFile->Text;
	char *ext=strrchr(InFile_Text.c_str(),'.');
	if (!*ext||strlen(ext)<4) return;
	if (!strcmp(ext,".obs" )||!strcmp(ext,".OBS" )||!strcmp(ext,".nav")||
		!strcmp(ext,".NAV" )||!strcmp(ext+2,"nav")||!strcmp(ext,"NAV" )||
		!strcmp(ext+3,"o"  )||!strcmp(ext+3,"O"  )||!strcmp(ext+3,"n" )||
		!strcmp(ext+3,"N"  )||!strcmp(ext+3,"p"  )||!strcmp(ext+3,"P" )||
		!strcmp(ext+3,"g"  )||!strcmp(ext+3,"G"  )||!strcmp(ext+3,"h" )||
		!strcmp(ext+3,"H"  )||!strcmp(ext+3,"q"  )||!strcmp(ext+3,"Q" )||
		!strcmp(ext+3,"l"  )||!strcmp(ext+3,"L"  )||!strcmp(ext+3,"c" )||
        !strcmp(ext+3,"C"  )) {
		viewer->Show();
		viewer->Read(RepPath(InFile_Text));
	}
}
// callback on button-view-file-1 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView1Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile1_Text=OutFile1->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile1_Text));
}
// callback on button-view-file-2 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView2Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile2_Text=OutFile2->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile2_Text));
}
// callback on button-view-file-3 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView3Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile3_Text=OutFile3->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile3_Text));
}
// callback on button-view-file-4 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView4Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile4_Text=OutFile4->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile4_Text));
}
// callback on button-view-file-5 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView5Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile5_Text=OutFile5->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile5_Text));
}
// callback on button-view-file-6 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView6Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile6_Text=OutFile6->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile6_Text));
}
// callback on button-view-file-7 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView7Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile7_Text=OutFile7->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile7_Text));
}
// callback on button-view-file-8 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView8Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile8_Text=OutFile8->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile8_Text));
}
// callback on button-view-file-9 -------------------------------------------
void __fastcall TMainWindow::BtnOutFileView9Click(TObject *Sender)
{
	TTextViewer *viewer=new TTextViewer(Application);
	AnsiString OutFile9_Text=OutFile9->Text;
	viewer->Show();
	viewer->Read(RepPath(OutFile9_Text));
}
// callback on button-about -------------------------------------------------
void __fastcall TMainWindow::BtnAboutClick(TObject *Sender)
{
	AboutDialog->About=PRGNAME;
	AboutDialog->IconIndex=3;
	AboutDialog->ShowModal();
}
// callback on button-time-start --------------------------------------------
void __fastcall TMainWindow::TimeStartFClick(TObject *Sender)
{
	UpdateEnable();
}
// callback on button-time-end ----------------------------------------------
void __fastcall TMainWindow::TimeEndFClick(TObject *Sender)
{
	UpdateEnable();
}
// callback on button-time-interval -----------------------------------------
void __fastcall TMainWindow::TimeIntFClick(TObject *Sender)
{
	UpdateEnable();
}
// callback on output-file check/uncheck ------------------------------------
void __fastcall TMainWindow::OutDirEnaClick(TObject *Sender)
{
	SetOutFiles(InFile->Text);
	UpdateEnable();
}
// callback on input-file-change --------------------------------------------
void __fastcall TMainWindow::InFileChange(TObject *Sender)
{
	SetOutFiles(InFile->Text);
}
// callback on format change ------------------------------------------------
void __fastcall TMainWindow::FormatChange(TObject *Sender)
{
	UpdateEnable();
}
// get time -----------------------------------------------------------------
void __fastcall TMainWindow::GetTime(gtime_t *ts, gtime_t *te, double *tint,
		double *tunit)
{
	AnsiString TimeY1_Text=TimeY1->Text,TimeH1_Text=TimeH1->Text;
	AnsiString TimeY2_Text=TimeY2->Text,TimeH2_Text=TimeH2->Text;
	AnsiString TimeInt_Text=TimeInt->Text,TimeUnit_Text=TimeUnit->Text;
	double eps[]={2000,1,1,0,0,0},epe[]={2000,1,1,0,0,0};
	
	if (TimeStartF->Checked) {
		sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",eps,eps+1,eps+2);
		sscanf(TimeH1_Text.c_str(),"%lf:%lf:%lf",eps+3,eps+4,eps+5);
		*ts=epoch2time(eps);
	}
	if (TimeEndF->Checked) {
		sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",epe,epe+1,epe+2);
		sscanf(TimeH2_Text.c_str(),"%lf:%lf:%lf",epe+3,epe+4,epe+5);
		*te=epoch2time(epe);
	}
	if (TimeIntF->Checked) {
		sscanf(TimeInt_Text.c_str(),"%lf",tint);
	}
	if (TimeUnitF->Checked) {
		if (sscanf(TimeUnit_Text.c_str(),"%lf",tunit)>=1) *tunit*=3600.0;
	}
}
// callback on time-start-ymd change ----------------------------------------
void __fastcall TMainWindow::TimeY1UDChangingEx(TObject *Sender,
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
// callback on time-start-hms change ----------------------------------------
void __fastcall TMainWindow::TimeH1UDChangingEx(TObject *Sender,
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
// callback on time-end-ymd change ------------------------------------------
void __fastcall TMainWindow::TimeY2UDChangingEx(TObject *Sender,
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
// callback on time-end-hms change ------------------------------------------
void __fastcall TMainWindow::TimeH2UDChangingEx(TObject *Sender,
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
// callback on time-start-ymd key press -------------------------------------
void __fastcall TMainWindow::TimeY1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	bool allowchange;
	if (Key==VK_UP||Key==VK_DOWN) {
		TimeY1UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
		Key=0;
	}
}
// callback on time-start-hms key press -------------------------------------
void __fastcall TMainWindow::TimeH1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	bool allowchange;
	if (Key==VK_UP||Key==VK_DOWN) {
		TimeH1UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
		Key=0;
	}
}
// callback on time-end-ymd key press ---------------------------------------
void __fastcall TMainWindow::TimeY2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	bool allowchange;
	if (Key==VK_UP||Key==VK_DOWN) {
		TimeY2UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
		Key=0;
	}
}
// callback on time-end-hms key press ---------------------------------------
void __fastcall TMainWindow::TimeH2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	bool allowchange;
	if (Key==VK_UP||Key==VK_DOWN) {
		TimeH2UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
		Key=0;
	}
}
// replace keywords in file path --------------------------------------------
AnsiString __fastcall TMainWindow::RepPath(AnsiString File)
{
	AnsiString Path;
	char path[1024];
	reppath(File.c_str(),path,timeadd(RnxTime,TSTARTMARGIN),RnxCode.c_str(),"");
	return Path=path;
}
// execute command ----------------------------------------------------------
int __fastcall TMainWindow::ExecCmd(AnsiString cmd)
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
// undate enable/disable of widgets -----------------------------------------
void __fastcall TMainWindow::UpdateEnable(void)
{
	AnsiString FormatText=Format->Text;
	int rnx=strstr(FormatText.c_str(),"RINEX")!=NULL;
	int sep_nav=RnxVer<3||SepNav;
	
	TimeY1		   ->Enabled=TimeStartF ->Checked;
	TimeH1		   ->Enabled=TimeStartF ->Checked;
	TimeY1UD	   ->Enabled=TimeStartF ->Checked;
	TimeH1UD	   ->Enabled=TimeStartF ->Checked;
	BtnTime1	   ->Enabled=TimeStartF ->Checked;
	TimeY2		   ->Enabled=TimeEndF	->Checked;
	TimeH2		   ->Enabled=TimeEndF	->Checked;
	TimeY2UD	   ->Enabled=TimeEndF	->Checked;
	TimeH2UD	   ->Enabled=TimeEndF	->Checked;
	BtnTime2	   ->Enabled=TimeEndF	->Checked;
	TimeInt		   ->Enabled=TimeIntF	->Checked;
	LabelTimeInt   ->Enabled=TimeInt	->Enabled;
	TimeUnitF	   ->Enabled=TimeStartF->Checked&&TimeEndF->Checked;
	TimeUnit	   ->Enabled=TimeStartF->Checked&&TimeEndF->Checked&&TimeUnitF->Checked;
	LabelTimeUnit  ->Enabled=TimeUnit  ->Enabled;
	OutFileEna3    ->Enabled=sep_nav&&(NavSys&SYS_GLO);
	OutFileEna4    ->Enabled=sep_nav&&(NavSys&SYS_SBS);
	OutFileEna5    ->Enabled=sep_nav&&(NavSys&SYS_QZS);
	OutFileEna6    ->Enabled=sep_nav&&(NavSys&SYS_GAL);
	OutFileEna7    ->Enabled=sep_nav&&(NavSys&SYS_CMP);
	OutFileEna8    ->Enabled=sep_nav&&(NavSys&SYS_IRN);
	OutFileEna9    ->Enabled=!rnx;
	OutDir		   ->Enabled=OutDirEna	->Checked;
	LabelOutDir    ->Enabled=OutDirEna	->Checked;
	OutFile1	   ->Enabled=OutFileEna1->Checked;
	OutFile2	   ->Enabled=OutFileEna2->Checked;
	OutFile3	   ->Enabled=OutFileEna3->Checked&&sep_nav&&(NavSys&SYS_GLO);
	OutFile4	   ->Enabled=OutFileEna4->Checked&&sep_nav&&(NavSys&SYS_SBS);
	OutFile5	   ->Enabled=OutFileEna5->Checked&&sep_nav&&(NavSys&SYS_QZS);
	OutFile6	   ->Enabled=OutFileEna6->Checked&&sep_nav&&(NavSys&SYS_GAL);
	OutFile7	   ->Enabled=OutFileEna7->Checked&&sep_nav&&(NavSys&SYS_CMP);
	OutFile8	   ->Enabled=OutFileEna8->Checked&&sep_nav&&(NavSys&SYS_IRN);
	OutFile9	   ->Enabled=OutFileEna9->Checked&&!rnx;
	BtnOutDir	   ->Enabled=OutDirEna	->Checked;
	BtnOutFile1    ->Enabled=OutFile1->Enabled;
	BtnOutFile2    ->Enabled=OutFile2->Enabled;
	BtnOutFile3    ->Enabled=OutFile3->Enabled;
	BtnOutFile4    ->Enabled=OutFile4->Enabled;
	BtnOutFile5    ->Enabled=OutFile5->Enabled;
	BtnOutFile6    ->Enabled=OutFile6->Enabled;
	BtnOutFile7    ->Enabled=OutFile7->Enabled;
	BtnOutFile8    ->Enabled=OutFile8->Enabled;
	BtnOutFile9    ->Enabled=OutFile9->Enabled;
	BtnOutFileView1->Enabled=OutFile1->Enabled;
	BtnOutFileView2->Enabled=OutFile2->Enabled;
	BtnOutFileView3->Enabled=OutFile3->Enabled;
	BtnOutFileView4->Enabled=OutFile4->Enabled;
	BtnOutFileView5->Enabled=OutFile5->Enabled;
	BtnOutFileView6->Enabled=OutFile6->Enabled;
	BtnOutFileView7->Enabled=OutFile7->Enabled;
	BtnOutFileView8->Enabled=OutFile8->Enabled;
	BtnOutFileView9->Enabled=OutFile9->Enabled;
}
// convert file -------------------------------------------------------------
void __fastcall TMainWindow::ConvertFile(void)
{
	rnxopt_t rnxopt={0};
	AnsiString InFile_Text=InFile->Text;
	AnsiString OutFile1_Text=OutFile1->Text,OutFile2_Text=OutFile2->Text;
	AnsiString OutFile3_Text=OutFile3->Text,OutFile4_Text=OutFile4->Text;
	AnsiString OutFile5_Text=OutFile5->Text,OutFile6_Text=OutFile6->Text;
	AnsiString OutFile7_Text=OutFile7->Text,OutFile8_Text=OutFile8->Text;
	AnsiString OutFile9_Text=OutFile9->Text;
	int i,format,sat;
	char file[1024]="",*ofile[9],ofile_[9][1024]={""},msg[256],*p;
	char buff[256],tstr[32];
	double RNXVER[]={2.10,2.11,2.12,3.00,3.01,3.02,3.03};
	FILE *fp;
	
	for (i=0;i<9;i++) ofile[i]=ofile_[i];
	
	// recognize input file format
	strcpy(file,InFile_Text.c_str());
	if (!(p=strrchr(file,'.'))) p=file;
	if (Format->ItemIndex==0) { // auto
		if		(!strcmp(p,".rtcm2")) format=STRFMT_RTCM2;
		else if (!strcmp(p,".rtcm3")) format=STRFMT_RTCM3;
		else if (!strcmp(p,".gps"  )) format=STRFMT_OEM4;
		else if (!strcmp(p,".ubx"  )) format=STRFMT_UBX;
		else if (!strcmp(p,".log"  )) format=STRFMT_SS2;
		else if (!strcmp(p,".bin"  )) format=STRFMT_CRES;
		else if (!strcmp(p,".jps"  )) format=STRFMT_JAVAD;
		else if (!strcmp(p,".bnx"  )) format=STRFMT_BINEX;
		else if (!strcmp(p,".binex")) format=STRFMT_BINEX;
		else if (!strcmp(p,".rt17" )) format=STRFMT_RT17;
		else if (!strcmp(p,".cmr"  )) format=STRFMT_CMR;
		else if (!strcmp(p,".trs"  )) format=STRFMT_TERSUS;
		else if (!strcmp(p,".obs"  )) format=STRFMT_RINEX;
		else if (!strcmp(p,".OBS"  )) format=STRFMT_RINEX;
		else if (!strcmp(p,".nav"  )) format=STRFMT_RINEX;
		else if (!strcmp(p,".NAV"  )) format=STRFMT_RINEX;
		else if (!strcmp(p+2,"nav" )) format=STRFMT_RINEX;
		else if (!strcmp(p+2,"NAV" )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"o"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"O"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"n"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"N"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"p"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"P"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"g"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"G"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"h"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"H"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"q"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"Q"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"l"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"L"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"c"   )) format=STRFMT_RINEX;
		else if (!strcmp(p+3,"C"   )) format=STRFMT_RINEX;
		else {
			showmsg("file format can not be recognized");
			return;
		}
	}
	else {
		for (i=0;formatstrs[i];i++) {
			if (Format->Text==formatstrs[i]) break;
		}
		if (formatstrs[i]) format=i; else return;
	}
	rnxopt.rnxver=RNXVER[RnxVer];
	
	if (format==STRFMT_RTCM2||format==STRFMT_RTCM3||format==STRFMT_RT17||
		format==STRFMT_CMR) {
		
		// input start date/time for rtcm 2, rtcm 3, RT17 or CMR
		StartDialog->FileName=file;
		if (StartDialog->ShowModal()!=mrOk) return;
		rnxopt.trtcm=StartDialog->Time;
	}
	if (OutFile1->Enabled&&OutFileEna1->Checked) strcpy(ofile[0],OutFile1_Text.c_str());
	if (OutFile2->Enabled&&OutFileEna2->Checked) strcpy(ofile[1],OutFile2_Text.c_str());
	if (OutFile3->Enabled&&OutFileEna3->Checked) strcpy(ofile[2],OutFile3_Text.c_str());
	if (OutFile4->Enabled&&OutFileEna4->Checked) strcpy(ofile[3],OutFile4_Text.c_str());
	if (OutFile5->Enabled&&OutFileEna5->Checked) strcpy(ofile[4],OutFile5_Text.c_str());
	if (OutFile6->Enabled&&OutFileEna6->Checked) strcpy(ofile[5],OutFile6_Text.c_str());
	if (OutFile7->Enabled&&OutFileEna7->Checked) strcpy(ofile[6],OutFile7_Text.c_str());
	if (OutFile8->Enabled&&OutFileEna8->Checked) strcpy(ofile[7],OutFile8_Text.c_str());
	if (OutFile9->Enabled&&OutFileEna9->Checked) strcpy(ofile[8],OutFile9_Text.c_str());
	
	// check overwrite output file
	for (i=0;i<9;i++) {
		if (!*ofile[i]||!(fp=fopen(ofile[i],"r"))) continue;
		fclose(fp);
		ConfDialog->Label2->Caption=ofile[i];
		if (ConfDialog->ShowModal()!=mrOk) return;
	}
	GetTime(&rnxopt.ts,&rnxopt.te,&rnxopt.tint,&rnxopt.tunit);
	strncpy(rnxopt.staid,RnxCode.c_str(),31);
	sprintf(rnxopt.prog,"%s %s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
	strncpy(rnxopt.runby,RunBy.c_str(),31);
	strncpy(rnxopt.marker,Marker.c_str(),63);
	strncpy(rnxopt.markerno,MarkerNo.c_str(),31);
	strncpy(rnxopt.markertype,MarkerType.c_str(),31);
	for (i=0;i<2;i++) strncpy(rnxopt.name[i],Name[i].c_str(),31);
	for (i=0;i<3;i++) strncpy(rnxopt.rec [i],Rec [i].c_str(),31);
	for (i=0;i<3;i++) strncpy(rnxopt.ant [i],Ant [i].c_str(),31);
	if (AutoPos) {
		for (i=0;i<3;i++) rnxopt.apppos[i]=AppPos[i];
	}
	for (i=0;i<3;i++) rnxopt.antdel[i]=AntDel[i];
	strncpy(rnxopt.rcvopt,RcvOption.c_str(),255);
	rnxopt.navsys=NavSys;
	rnxopt.obstype=ObsType;
	rnxopt.freqtype=FreqType;
	p=rnxopt.comment[0];
	sprintf(p,"log: %-53.53s",file);
	p=rnxopt.comment[1];
	p+=sprintf(p,"format: %s",formatstrs[format]);
	if (*rnxopt.rcvopt) sprintf(p,", option: %s",rnxopt.rcvopt);
	for (i=0;i<2;i++) strncpy(rnxopt.comment[i+2],Comment[i].c_str(),63);
	for (i=0;i<7;i++) strcpy(rnxopt.mask[i],CodeMask[i].c_str());
	rnxopt.autopos=AutoPos;
	rnxopt.scanobs=ScanObs;
	rnxopt.halfcyc=HalfCyc;
	rnxopt.outiono=OutIono;
	rnxopt.outtime=OutTime;
	rnxopt.outleaps=OutLeaps;
	rnxopt.sep_nav=SepNav;
	rnxopt.ttol=TimeTol;
	
	strcpy(buff,ExSats.c_str());
	for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
		if (!(sat=satid2no(p))) continue;
		rnxopt.exsats[sat-1]=1;
	}
	abortf=0;
	BtnConvert	->Visible=false;
	BtnAbort	->Visible=true;
	Panel1		->Enabled=false;
	Panel2		->Enabled=false;
	BtnPlot		->Enabled=false;
	BtnPost		->Enabled=false;
	BtnOptions	->Enabled=false;
	BtnExit		->Enabled=false;
	Format		->Enabled=false;
	BtnKey		->Enabled=false;
	LabelInFile ->Enabled=false;
	LabelOutDir ->Enabled=false;
	LabelOutFile->Enabled=false;
	LabelFormat ->Enabled=false;
	Message		->Caption="";
	
	if (TraceLevel>0) {
		traceopen(TRACEFILE);
		tracelevel(TraceLevel);
	}
	// convert to rinex
	(void)convrnx(format,&rnxopt,file,ofile);
	
	if (TraceLevel>0) {
		traceclose();
	}
	BtnAbort	->Visible=false;
	BtnConvert	->Visible=true;
	Panel1		->Enabled=true;
	Panel2		->Enabled=true;
	BtnPlot		->Enabled=true;
	BtnPost		->Enabled=true;
	BtnOptions	->Enabled=true;
	BtnExit		->Enabled=true;
	Format		->Enabled=true;
	BtnKey		->Enabled=true;
	LabelInFile ->Enabled=true;
	LabelOutDir ->Enabled=true;
	LabelOutFile->Enabled=true;
	LabelFormat ->Enabled=true;
	
#if 0
	// set time-start/end if time not specified
	if (!TimeStartF->Checked&&rnxopt.tstart.time!=0) {
		time2str(rnxopt.tstart,tstr,0);
		tstr[10]='\0';
		TimeY1->Text=tstr;
		TimeH1->Text=tstr+11;
	}
	if (!TimeEndF->Checked&&rnxopt.tend.time!=0) {
		time2str(rnxopt.tend,tstr,0);
		tstr[10]='\0';
		TimeY2->Text=tstr;
		TimeH2->Text=tstr+11;
	}
#endif
	RnxTime=rnxopt.tstart;
	
	AddHist(InFile);
}
// load options -------------------------------------------------------------
void __fastcall TMainWindow::LoadOpt(void)
{
	TIniFile *ini=new TIniFile(IniFile);
	AnsiString mask="1111111111111111111111111111111111111111111111111111111";
	
	RnxVer				=ini->ReadInteger("opt","rnxver",	   0);
	RnxFile				=ini->ReadInteger("opt","rnxfile",	   0);
	RnxCode				=ini->ReadString ("opt","rnxcode","0000");
	RunBy				=ini->ReadString ("opt","runby",	  "");
	Marker				=ini->ReadString ("opt","marker",	  "");
	MarkerNo			=ini->ReadString ("opt","markerno",   "");
	MarkerType			=ini->ReadString ("opt","markertype", "");
	Name[0]				=ini->ReadString ("opt","name0",	  "");
	Name[1]				=ini->ReadString ("opt","name1",	  "");
	Rec[0]				=ini->ReadString ("opt","rec0",		  "");
	Rec[1]				=ini->ReadString ("opt","rec1",		  "");
	Rec[2]				=ini->ReadString ("opt","rec2",		  "");
	Ant[0]				=ini->ReadString ("opt","ant0",		  "");
	Ant[1]				=ini->ReadString ("opt","ant1",		  "");
	Ant[2]				=ini->ReadString ("opt","ant2",		  "");
	AppPos[0]			=ini->ReadFloat  ("opt","apppos0",	 0.0);
	AppPos[1]			=ini->ReadFloat  ("opt","apppos1",	 0.0);
	AppPos[2]			=ini->ReadFloat  ("opt","apppos2",	 0.0);
	AntDel[0]			=ini->ReadFloat  ("opt","antdel0",	 0.0);
	AntDel[1]			=ini->ReadFloat  ("opt","antdel1",	 0.0);
	AntDel[2]			=ini->ReadFloat  ("opt","antdel2",	 0.0);
	Comment[0]			=ini->ReadString ("opt","comment0",   "");
	Comment[1]			=ini->ReadString ("opt","comment1",   "");
	RcvOption			=ini->ReadString ("opt","rcvoption",  "");
	NavSys				=ini->ReadInteger("opt","navsys",	 0x3);
	ObsType				=ini->ReadInteger("opt","obstype",	 0xF);
	FreqType			=ini->ReadInteger("opt","freqtype",  0x3);
	ExSats				=ini->ReadString ("opt","exsats",	  "");
	TraceLevel			=ini->ReadInteger("opt","tracelevel",  0);
	RnxTime.time		=ini->ReadInteger("opt","rnxtime",	   0);
	CodeMask[0]			=ini->ReadString ("opt","codemask_1",mask);
	CodeMask[1]			=ini->ReadString ("opt","codemask_2",mask);
	CodeMask[2]			=ini->ReadString ("opt","codemask_3",mask);
	CodeMask[3]			=ini->ReadString ("opt","codemask_4",mask);
	CodeMask[4]			=ini->ReadString ("opt","codemask_5",mask);
	CodeMask[5]			=ini->ReadString ("opt","codemask_6",mask);
	CodeMask[6]			=ini->ReadString ("opt","codemask_7",mask);
	AutoPos				=ini->ReadInteger("opt","autopos",	   0);
	ScanObs				=ini->ReadInteger("opt","scanobs",	   0);
	HalfCyc				=ini->ReadInteger("opt","halfcyc",	   0);
	OutIono				=ini->ReadInteger("opt","outiono",	   0);
	OutTime				=ini->ReadInteger("opt","outtime",	   0);
	OutLeaps			=ini->ReadInteger("opt","outleaps",    0);
	SepNav				=ini->ReadInteger("opt","sepnav",	   0);
	TimeTol				=ini->ReadFloat  ("opt","timetol", 0.005);
	
	TimeStartF ->Checked=ini->ReadInteger("set","timestartf",  0);
	TimeEndF   ->Checked=ini->ReadInteger("set","timeendf",    0);
	TimeIntF   ->Checked=ini->ReadInteger("set","timeintf",    0);
	TimeY1	   ->Text	=ini->ReadString ("set","timey1",	  "2000/01/01");
	TimeH1	   ->Text	=ini->ReadString ("set","timeh1",	  "00:00:00"  );
	TimeY2	   ->Text	=ini->ReadString ("set","timey2",	  "2000/01/01");
	TimeH2	   ->Text	=ini->ReadString ("set","timeh2",	  "00:00:00"  );
	TimeInt    ->Text	=ini->ReadString ("set","timeint",	 "1");
	TimeUnitF  ->Checked=ini->ReadInteger("set","timeunitf",   0);
	TimeUnit   ->Text	=ini->ReadString ("set","timeunit", "24");
	InFile	   ->Text	=ini->ReadString ("set","infile",	  "");
	OutDir	   ->Text	=ini->ReadString ("set","outdir",	  "");
	OutFile1   ->Text	=ini->ReadString ("set","outfile1",   "");
	OutFile2   ->Text	=ini->ReadString ("set","outfile2",   "");
	OutFile3   ->Text	=ini->ReadString ("set","outfile3",   "");
	OutFile4   ->Text	=ini->ReadString ("set","outfile4",   "");
	OutFile5   ->Text	=ini->ReadString ("set","outfile5",   "");
	OutFile6   ->Text	=ini->ReadString ("set","outfile6",   "");
	OutFile7   ->Text	=ini->ReadString ("set","outfile7",   "");
	OutFile8   ->Text	=ini->ReadString ("set","outfile8",   "");
	OutFile9   ->Text	=ini->ReadString ("set","outfile9",   "");
	OutDirEna  ->Checked=ini->ReadInteger("set","outdirena",   0);
	OutFileEna1->Checked=ini->ReadInteger("set","outfileena1", 1);
	OutFileEna2->Checked=ini->ReadInteger("set","outfileena2", 1);
	OutFileEna3->Checked=ini->ReadInteger("set","outfileena3", 1);
	OutFileEna4->Checked=ini->ReadInteger("set","outfileena4", 1);
	OutFileEna5->Checked=ini->ReadInteger("set","outfileena5", 1);
	OutFileEna6->Checked=ini->ReadInteger("set","outfileena6", 1);
	OutFileEna7->Checked=ini->ReadInteger("set","outfileena7", 1);
	OutFileEna8->Checked=ini->ReadInteger("set","outfileena8", 1);
	OutFileEna9->Checked=ini->ReadInteger("set","outfileena9", 1);
	Format	 ->ItemIndex=ini->ReadInteger("set","format",	   0);
	
	InFile->Items=ReadList(ini,"hist","inputfile");
	
	TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
	TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
	TTextViewer::FontD=new TFont;
	TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
	TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
	
	CmdPostExe		   =ini->ReadString  ("set","cmdpostexe","rtkpost_mkl");
	Width			   =ini->ReadInteger ("window","width", 488);
	
	delete ini;
	
	UpdateEnable();
}
// save options -------------------------------------------------------------
void __fastcall TMainWindow::SaveOpt(void)
{
	TIniFile *ini=new TIniFile(IniFile);
	
	ini->WriteInteger("opt","rnxver",	  RnxVer);
	ini->WriteInteger("opt","rnxfile",	  RnxFile);
	ini->WriteString ("opt","rnxcode",	  RnxCode);
	ini->WriteString ("opt","runby",	  RunBy);
	ini->WriteString ("opt","marker",	  Marker);
	ini->WriteString ("opt","markerno",   MarkerNo);
	ini->WriteString ("opt","markertype", MarkerType);
	ini->WriteString ("opt","name0",	  Name[0]);
	ini->WriteString ("opt","name1",	  Name[1]);
	ini->WriteString ("opt","rec0",		  Rec[0]);
	ini->WriteString ("opt","rec1",		  Rec[1]);
	ini->WriteString ("opt","rec2",		  Rec[2]);
	ini->WriteString ("opt","ant0",		  Ant[0]);
	ini->WriteString ("opt","ant1",		  Ant[1]);
	ini->WriteString ("opt","ant2",		  Ant[2]);
	ini->WriteFloat  ("opt","apppos0",	  AppPos[0]);
	ini->WriteFloat  ("opt","apppos1",	  AppPos[1]);
	ini->WriteFloat  ("opt","apppos2",	  AppPos[2]);
	ini->WriteFloat  ("opt","antdel0",	  AntDel[0]);
	ini->WriteFloat  ("opt","antdel1",	  AntDel[1]);
	ini->WriteFloat  ("opt","antdel2",	  AntDel[2]);
	ini->WriteString ("opt","comment0",   Comment[0]);
	ini->WriteString ("opt","comment1",   Comment[1]);
	ini->WriteString ("opt","rcvoption",  RcvOption);
	ini->WriteInteger("opt","navsys",	  NavSys);
	ini->WriteInteger("opt","obstype",	  ObsType);
	ini->WriteInteger("opt","freqtype",   FreqType);
	ini->WriteString ("opt","exsats",	  ExSats);
	ini->WriteInteger("opt","tracelevel", TraceLevel);
	ini->WriteInteger("opt","rnxtime",(int)RnxTime.time);
	ini->WriteString ("opt","codemask_1", CodeMask[0]);
	ini->WriteString ("opt","codemask_2", CodeMask[1]);
	ini->WriteString ("opt","codemask_3", CodeMask[2]);
	ini->WriteString ("opt","codemask_4", CodeMask[3]);
	ini->WriteString ("opt","codemask_5", CodeMask[4]);
	ini->WriteString ("opt","codemask_6", CodeMask[5]);
	ini->WriteString ("opt","codemask_7", CodeMask[6]);
	ini->WriteInteger("opt","autopos",	  AutoPos);
	ini->WriteInteger("opt","scanobs",	  ScanObs);
	ini->WriteInteger("opt","halfcyc",	  HalfCyc);
	ini->WriteInteger("opt","outiono",	  OutIono);
	ini->WriteInteger("opt","outtime",	  OutTime);
	ini->WriteInteger("opt","outleaps",   OutLeaps);
	ini->WriteInteger("opt","sepnav",	  SepNav);
	ini->WriteFloat  ("opt","timetol",	  TimeTol);
	
	ini->WriteInteger("set","timestartf", TimeStartF ->Checked);
	ini->WriteInteger("set","timeendf",   TimeEndF	 ->Checked);
	ini->WriteInteger("set","timeintf",   TimeIntF	 ->Checked);
	ini->WriteString ("set","timey1",	  TimeY1	 ->Text);
	ini->WriteString ("set","timeh1",	  TimeH1	 ->Text);
	ini->WriteString ("set","timey2",	  TimeY2	 ->Text);
	ini->WriteString ("set","timeh2",	  TimeH2	 ->Text);
	ini->WriteString ("set","timeint",	  TimeInt	 ->Text);
	ini->WriteInteger("set","timeunitf",  TimeUnitF  ->Checked);
	ini->WriteString ("set","timeunit",   TimeUnit	 ->Text);
	ini->WriteString ("set","infile",	  InFile	 ->Text);
	ini->WriteString ("set","outdir",	  OutDir	 ->Text);
	ini->WriteString ("set","outfile1",   OutFile1	 ->Text);
	ini->WriteString ("set","outfile2",   OutFile2	 ->Text);
	ini->WriteString ("set","outfile3",   OutFile3	 ->Text);
	ini->WriteString ("set","outfile4",   OutFile4	 ->Text);
	ini->WriteString ("set","outfile5",   OutFile5	 ->Text);
	ini->WriteString ("set","outfile6",   OutFile6	 ->Text);
	ini->WriteString ("set","outfile7",   OutFile7	 ->Text);
	ini->WriteString ("set","outfile8",   OutFile8	 ->Text);
	ini->WriteString ("set","outfile9",   OutFile9	 ->Text);
	ini->WriteInteger("set","outdirena",  OutDirEna  ->Checked);
	ini->WriteInteger("set","outfileena1",OutFileEna1->Checked);
	ini->WriteInteger("set","outfileena2",OutFileEna2->Checked);
	ini->WriteInteger("set","outfileena3",OutFileEna3->Checked);
	ini->WriteInteger("set","outfileena4",OutFileEna4->Checked);
	ini->WriteInteger("set","outfileena5",OutFileEna5->Checked);
	ini->WriteInteger("set","outfileena6",OutFileEna6->Checked);
	ini->WriteInteger("set","outfileena7",OutFileEna7->Checked);
	ini->WriteInteger("set","outfileena8",OutFileEna8->Checked);
	ini->WriteInteger("set","outfileena9",OutFileEna9->Checked);
	ini->WriteInteger("set","format",	  Format	 ->ItemIndex);
	
	WriteList(ini,"hist","inputfile",InFile->Items);
	
	ini->WriteInteger("viewer","color1",  (int)TTextViewer::Color1);
	ini->WriteInteger("viewer","color2",  (int)TTextViewer::Color2);
	ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
	ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
	ini->WriteInteger("window","width",				  Width);
	delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TMainWindow::Panel4Resize(TObject *Sender)
{
	TBitBtn *btns[]={BtnPlot,BtnPost,BtnOptions,BtnConvert,BtnExit};
	int w=(Panel4->Width-2)/5;
	
	for (int i=0;i<5;i++) {
		btns[i]->Width=w;
		btns[i]->Left=i*w+1;
	}
	BtnAbort->Width=BtnConvert->Width;
	BtnAbort->Left =BtnConvert->Left;
}
//---------------------------------------------------------------------------

void __fastcall TMainWindow::Panel2Resize(TObject *Sender)
{
	TButton *btns1[]={
		BtnOutFile1,BtnOutFile2,BtnOutFile3,BtnOutFile4,BtnOutFile5,
		BtnOutFile6,BtnOutFile7,BtnOutFile8,BtnOutFile9
	};
	TSpeedButton *btns2[]={
		BtnOutFileView1,BtnOutFileView2,BtnOutFileView3,BtnOutFileView4,
		BtnOutFileView5,BtnOutFileView6,BtnOutFileView7,BtnOutFileView8,
		BtnOutFileView9
	};
	TEdit *inps[]={
		OutFile1,OutFile2,OutFile3,OutFile4,OutFile5,OutFile6,OutFile7,
		OutFile8,OutFile9
	};
	int w=Panel2->Width;
	
	BtnInFile->Left=w-BtnInFile->Width-5;
	BtnInFileView->Left=w-BtnInFile->Width-BtnInFileView->Width-5;
	InFile->Width=w-BtnInFile->Width-BtnInFileView->Width-6-InFile->Left;
	
	Format->Left=w-Format->Width-5;
	LabelFormat->Left=Format->Left+3;
	BtnOutDir->Left=w-BtnOutDir->Width-Format->Width-6;
	OutDir->Width=w-BtnOutDir->Width-Format->Width-7-OutDir->Left;
	
	for (int i=0;i<9;i++) {
		btns1[i]->Left=w-btns1[i]->Width-5;
		btns2[i]->Left=w-btns1[i]->Width-btns2[i]->Width-5;
		inps[i]->Width=w-btns1[i]->Width-btns2[i]->Width-6-inps[i]->Left;
	}
}
//---------------------------------------------------------------------------





