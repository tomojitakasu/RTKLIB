//---------------------------------------------------------------------------
// strsvr : stream server
//
//			Copyright (C) 2007-2012 by T.TAKASU, All rights reserved.
//
// options : strsvr [-t title][-i file][-auto][-tray]
//
//			 -t title	window title
//			 -i file	ini file path
//			 -auto		auto start
//			 -tray		start as task tray icon
//
// version : $Revision:$ $Date:$
// history : 2008/04/03  1.1 rtklib 2.3.1
//			 2010/07/18  1.2 rtklib 2.4.0
//			 2011/06/10  1.3 rtklib 2.4.1
//			 2012/12/15  1.4 rtklib 2.4.2
//						 add stream conversion function
//						 add option -auto and -tray
//---------------------------------------------------------------------------
#include <vcl.h>
#include <inifiles.hpp>
#include <mmsystem.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "svroptdlg.h"
#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "ftpoptdlg.h"
#include "confdlg.h"
#include "cmdoptdlg.h"
#include "convdlg.h"
#include "aboutdlg.h"
#include "refdlg.h"
#include "mondlg.h"
#include "svrmain.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define PRGNAME		"STRSVR"		// program name
#define TRACEFILE	"strsvr.trace"	// debug trace file
#define CLORANGE	(TColor)0x00AAFF

#define MIN(x,y)	((x)<(y)?(x):(y))
#define MAX(x,y)	((x)>(y)?(x):(y))

static strsvr_t strsvr;

// number to comma-separated number -----------------------------------------
static void num2cnum(int num, char *str)
{
	char buff[256],*p=buff,*q=str;
	int i,n;
	n=sprintf(buff,"%u",(unsigned int)num);
	for (i=0;i<n;i++) {
		*q++=*p++;
		if ((n-i-1)%3==0&&i<n-1) *q++=',';
	}
	*q='\0';
}
// constructor --------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
	char file[1024]="strsvr.exe",*p;
	
	::GetModuleFileName(NULL,file,sizeof(file));
	if (!(p=strrchr(file,'.'))) p=file+strlen(file);
	strcpy(p,".ini");
	IniFile=file;
	
	DoubleBuffered=true;
}
// callback on form create --------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
	AnsiString s;
	int argc=0,autorun=0,tasktray=0;
	char *p,*argv[32],buff[1024];
	
	strsvrinit(&strsvr,3);
	
	Caption=s.sprintf("%s ver.%s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
	
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
		if		(!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
		else if (!strcmp(argv[i],"-auto")) autorun=1;
		else if (!strcmp(argv[i],"-tray")) tasktray=1;
	}
	SetTrayIcon(0);
	
	if (tasktray) {
		Application->ShowMainForm=false;
		TrayIcon->Visible=true;
	}
	if (autorun) {
		SvrStart();
	}
}
// callback on form show ----------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
	;
}
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	SaveOpt();
}
// callback on button-exit --------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
	Close();
}
// callback on button-start -------------------------------------------------
void __fastcall TMainForm::BtnStartClick(TObject *Sender)
{
	SvrStart();
}
// callback on button-stop --------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
	SvrStop();
}
// callback on button-options -----------------------------------------------
void __fastcall TMainForm::BtnOptClick(TObject *Sender)
{
	for (int i=0;i<6;i++) SvrOptDialog->SvrOpt[i]=SvrOpt[i];
	for (int i=0;i<3;i++) SvrOptDialog->AntPos[i]=AntPos[i];
	for (int i=0;i<3;i++) SvrOptDialog->AntOff[i]=AntOff[i];
	SvrOptDialog->TraceLevel=TraceLevel;
	SvrOptDialog->NmeaReq=NmeaReq;
	SvrOptDialog->FileSwapMargin=FileSwapMargin;
	SvrOptDialog->RelayBack=RelayBack;
	SvrOptDialog->ProgBarRange=ProgBarRange;
	SvrOptDialog->StaPosFile=StaPosFile;
	SvrOptDialog->ExeDirectory=ExeDirectory;
	SvrOptDialog->LocalDirectory=LocalDirectory;
	SvrOptDialog->ProxyAddress=ProxyAddress;
	SvrOptDialog->StaId=StaId;
	SvrOptDialog->StaSel=StaSel;
	SvrOptDialog->AntType=AntType;
	SvrOptDialog->RcvType=RcvType;
	SvrOptDialog->SrcTblFile=SrcTblFile;
	SvrOptDialog->LogFile=LogFile;
	
	if (SvrOptDialog->ShowModal()!=mrOk) return;
	
	for (int i=0;i<6;i++) SvrOpt[i]=SvrOptDialog->SvrOpt[i];
	for (int i=0;i<3;i++) AntPos[i]=SvrOptDialog->AntPos[i];
	for (int i=0;i<3;i++) AntOff[i]=SvrOptDialog->AntOff[i];
	TraceLevel=SvrOptDialog->TraceLevel;
	NmeaReq=SvrOptDialog->NmeaReq;
	FileSwapMargin=SvrOptDialog->FileSwapMargin;
	RelayBack=SvrOptDialog->RelayBack;
	ProgBarRange=SvrOptDialog->ProgBarRange;
	StaPosFile=SvrOptDialog->StaPosFile;
	ExeDirectory=SvrOptDialog->ExeDirectory;
	LocalDirectory=SvrOptDialog->LocalDirectory;
	ProxyAddress=SvrOptDialog->ProxyAddress;
	StaId=SvrOptDialog->StaId;
	StaSel=SvrOptDialog->StaSel;
	AntType=SvrOptDialog->AntType;
	RcvType=SvrOptDialog->RcvType;
	SrcTblFile=SvrOptDialog->SrcTblFile;
	LogFile=SvrOptDialog->LogFile;
}
// callback on button-input-opt ---------------------------------------------
void __fastcall TMainForm::BtnInputClick(TObject *Sender)
{
	switch (Input->ItemIndex) {
		case 0: SerialOpt(0,0); break;
		case 1: TcpOpt(0,1); break; // TCP Client
		case 2: TcpOpt(0,0); break; // TCP Server
		case 3: TcpOpt(0,3); break; // NTRIP Client
		case 4: TcpOpt(0,5); break; // NTRIP Caster Server
		case 5: TcpOpt(0,6); break; // UDP Server
		case 6: FileOpt(0,0); break;
		case 7: FtpOpt(0,0); break; // FTP
		case 8: FtpOpt(0,1); break; // HTTP
	}
}
// callback on button-cmd ---------------------------------------------------
void __fastcall TMainForm::BtnCmdClick(TObject *Sender)
{
	TButton *btn[]={BtnCmd,BtnCmd1,BtnCmd2,BtnCmd3};
	TComboBox *type[]={Input,Output1,Output2,Output3};
	int i,j;
	
	for (i=0;i<MAXSTR;i++) {
		if (btn[i]==(TButton *)Sender) break;
	}
	if (i>=MAXSTR) return;
	
	for (j=0;j<3;j++) {
		if (type[i]->Text=="Serial") {
			CmdOptDialog->Cmds[j]=Cmds[i][j];
			CmdOptDialog->CmdEna[j]=CmdEna[i][j];
		}
		else {
			CmdOptDialog->Cmds[j]=CmdsTcp[i][j];
			CmdOptDialog->CmdEna[j]=CmdEnaTcp[i][j];
		}
	}
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	for (j=0;j<3;j++) {
		if (type[i]->Text=="Serial") {
			Cmds[i][j]=CmdOptDialog->Cmds[j];
			CmdEna[i][j]=CmdOptDialog->CmdEna[j];
		}
		else {
			CmdsTcp[i][j]=CmdOptDialog->Cmds[j];
			CmdEnaTcp[i][j]=CmdOptDialog->CmdEna[j];
		}
	}
}
// callback on button-output1-opt -------------------------------------------
void __fastcall TMainForm::BtnOutput1Click(TObject *Sender)
{
	switch (Output1->ItemIndex) {
		case 1: SerialOpt(1,0); break;
		case 2: TcpOpt(1,1); break; // TCP Client
		case 3: TcpOpt(1,0); break; // TCP Server
		case 4: TcpOpt(1,2); break; // NTRIP Server
		case 5: TcpOpt(1,4); break; // NTRIP Caster Client
		case 6: TcpOpt(1,7); break; // UDP Client
		case 7: FileOpt(1,1); break;
	}
}
// callback on button-output2-opt -------------------------------------------
void __fastcall TMainForm::BtnOutput2Click(TObject *Sender)
{
	switch (Output2->ItemIndex) {
		case 1: SerialOpt(2,0); break;
		case 2: TcpOpt(2,1); break;
		case 3: TcpOpt(2,0); break;
		case 4: TcpOpt(2,2); break;
		case 5: TcpOpt(2,4); break;
		case 6: TcpOpt(2,7); break;
		case 7: FileOpt(2,1); break;
	}
}
// callback on button-output3-opt -------------------------------------------
void __fastcall TMainForm::BtnOutput3Click(TObject *Sender)
{
	switch (Output3->ItemIndex) {
		case 1: SerialOpt(3,0); break;
		case 2: TcpOpt(3,1); break;
		case 3: TcpOpt(3,0); break;
		case 4: TcpOpt(3,2); break;
		case 5: TcpOpt(3,4); break;
		case 6: TcpOpt(3,7); break;
		case 7: FileOpt(3,1); break; 
	}
}
// callback on button-output1-conv ------------------------------------------
void __fastcall TMainForm::BtnConv1Click(TObject *Sender)
{
	ConvDialog->ConvEna=ConvEna[0];
	ConvDialog->ConvInp=ConvInp[0];
	ConvDialog->ConvOut=ConvOut[0];
	ConvDialog->ConvMsg=ConvMsg[0];
	ConvDialog->ConvOpt=ConvOpt[0];
	if (ConvDialog->ShowModal()!=mrOk) return;
	ConvEna[0]=ConvDialog->ConvEna;
	ConvInp[0]=ConvDialog->ConvInp;
	ConvOut[0]=ConvDialog->ConvOut;
	ConvMsg[0]=ConvDialog->ConvMsg;
	ConvOpt[0]=ConvDialog->ConvOpt;
}
// callback on button-output2-conv ------------------------------------------
void __fastcall TMainForm::BtnConv2Click(TObject *Sender)
{
	ConvDialog->ConvEna=ConvEna[1];
	ConvDialog->ConvInp=ConvInp[1];
	ConvDialog->ConvOut=ConvOut[1];
	ConvDialog->ConvMsg=ConvMsg[1];
	ConvDialog->ConvOpt=ConvOpt[1];
	if (ConvDialog->ShowModal()!=mrOk) return;
	ConvEna[1]=ConvDialog->ConvEna;
	ConvInp[1]=ConvDialog->ConvInp;
	ConvOut[1]=ConvDialog->ConvOut;
	ConvMsg[1]=ConvDialog->ConvMsg;
	ConvOpt[1]=ConvDialog->ConvOpt;
}
// callback on button-output3-conv ------------------------------------------
void __fastcall TMainForm::BtnConv3Click(TObject *Sender)
{
	ConvDialog->ConvEna=ConvEna[2];
	ConvDialog->ConvInp=ConvInp[2];
	ConvDialog->ConvOut=ConvOut[2];
	ConvDialog->ConvMsg=ConvMsg[2];
	ConvDialog->ConvOpt=ConvOpt[2];
	if (ConvDialog->ShowModal()!=mrOk) return;
	ConvEna[2]=ConvDialog->ConvEna;
	ConvInp[2]=ConvDialog->ConvInp;
	ConvOut[2]=ConvDialog->ConvOut;
	ConvMsg[2]=ConvDialog->ConvMsg;
	ConvOpt[2]=ConvDialog->ConvOpt;
}
// callback on buttn-about --------------------------------------------------
void __fastcall TMainForm::BtnAboutClick(TObject *Sender)
{
	AboutDialog->About=PRGNAME;
	AboutDialog->IconIndex=6;
	AboutDialog->ShowModal();
}
// callback on task-icon ----------------------------------------------------
void __fastcall TMainForm::BtnTaskIconClick(TObject *Sender)
{
	Visible=false;
	TrayIcon->Visible=true;
}
// callback on task-icon double-click ---------------------------------------
void __fastcall TMainForm::TrayIconDblClick(TObject *Sender)
{
	Visible=true;
	TrayIcon->Visible=false;
}
// callback on task-tray-icon click -----------------------------------------
void __fastcall TMainForm::TrayIconMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	;
}
// callback on menu-expand --------------------------------------------------
void __fastcall TMainForm::MenuExpandClick(TObject *Sender)
{
	Visible=true;
	TrayIcon->Visible=false;
}
// callback on menu-start ---------------------------------------------------
void __fastcall TMainForm::MenuStartClick(TObject *Sender)
{
	SvrStart();
}
// callback on menu-stop ----------------------------------------------------
void __fastcall TMainForm::MenuStopClick(TObject *Sender)
{
	SvrStop();
}
// callback on menu-exit ----------------------------------------------------
void __fastcall TMainForm::MenuExitClick(TObject *Sender)
{
	Close();
}
// callback on stream-monitor -----------------------------------------------
void __fastcall TMainForm::BtnStrMonClick(TObject *Sender)
{
	StrMonDialog->Show();
}
// callback on output1 enable -----------------------------------------------
void __fastcall TMainForm::EnaOut1Click(TObject *Sender)
{
	UpdateEnable();
}
// callback on output2 enable -----------------------------------------------
void __fastcall TMainForm::EnaOut2Click(TObject *Sender)
{
	UpdateEnable();
}
// callback on output3 enable -----------------------------------------------
void __fastcall TMainForm::EnaOut3Click(TObject *Sender)
{
	UpdateEnable();
}
// callback on input type change --------------------------------------------
void __fastcall TMainForm::InputChange(TObject *Sender)
{
	UpdateEnable();
}
// callback on output1 type change ------------------------------------------
void __fastcall TMainForm::Output1Change(TObject *Sender)
{
	UpdateEnable(); 
}
// callback on output2 type change ------------------------------------------
void __fastcall TMainForm::Output2Change(TObject *Sender)
{
	UpdateEnable();
}
// callback on output3 type change ------------------------------------------
void __fastcall TMainForm::Output3Change(TObject *Sender)
{
	UpdateEnable();
}
// callback on interval timer -----------------------------------------------
void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
	TColor color[]={clRed,clWhite,CLORANGE,clGreen,clLime,clAqua};
	TPanel *e0[]={IndInput,IndOutput1,IndOutput2,IndOutput3};
	TLabel *e1[]={InputByte,Output1Byte,Output2Byte,Output3Byte};
	TLabel *e2[]={InputBps,Output1Bps,Output2Bps,Output3Bps};
	AnsiString s;
	gtime_t time=utc2gpst(timeget());
	int stat[MAXSTR]={0},byte[MAXSTR]={0},bps[MAXSTR]={0};
	char msg[MAXSTRMSG*MAXSTR]="",s1[256],s2[256];
	double ctime,t[4],pos,range;
	
	strsvrstat(&strsvr,stat,byte,bps,msg);
	for (int i=0;i<MAXSTR;i++) {
		num2cnum(byte[i],s1);
		num2cnum(bps[i],s2);
		e0[i]->Color=color[stat[i]+1];
		e1[i]->Caption=s1;
		e2[i]->Caption=s2;
	}
	pos=fmod(byte[0]/1e3/MAX(ProgBarRange,1),1.0)*110.0;
	Progress->Position=!stat[0]?0:MIN((int)pos,100);
	
	time2str(time,s1,0);
	Time->Caption=s.sprintf("%s GPST",s1);
	
	if (Panel1->Enabled) {
		ctime=timediff(EndTime,StartTime);
	}
	else {
		ctime=timediff(time,StartTime);
	}
	ctime=floor(ctime);
	t[0]=floor(ctime/86400.0); ctime-=t[0]*86400.0;
	t[1]=floor(ctime/3600.0 ); ctime-=t[1]*3600.0;
	t[2]=floor(ctime/60.0	); ctime-=t[2]*60.0;
	t[3]=ctime;
	ConTime->Caption=s.sprintf("%.0fd %02.0f:%02.0f:%02.0f",t[0],t[1],t[2],t[3]);
	
	num2cnum(byte[0],s1); num2cnum(bps[0],s2);
	TrayIcon->Hint=s.sprintf("%s bytes %s bps",s1,s2);
	SetTrayIcon(stat[0]<=0?0:(stat[0]==3?2:1));
	
	Message->Caption=msg;
}
// start stream server ------------------------------------------------------
void __fastcall TMainForm::SvrStart(void)
{
	strconv_t *conv[3]={0};
	static char str[MAXSTR][1024];
	int itype[]={
		STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_NTRIPC_S,STR_UDPSVR,
		STR_FILE,STR_FTP,STR_HTTP
	};
	int otype[]={
		STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPC_C,
		STR_UDPCLI,STR_FILE
	};
	int ip[]={0,1,1,1,1,1,2,3,3},strs[4]={0},opt[8]={0},n;
	char *paths[MAXSTR],*cmds[MAXSTR]={0},*cmds_periodic[MAXSTR]={0};
	char filepath[1024],buff[1024];
	char *ant[3]={"","",""},*rcv[3]={"","",""},*p;
	FILE *fp;
	
	if (TraceLevel>0) {
		traceopen(*LogFile.c_str()?LogFile.c_str():TRACEFILE);
		tracelevel(TraceLevel);
	}
	for (int i=0;i<4;i++) paths[i]=str[i];
	
	strs[0]=itype[Input->ItemIndex];
	strs[1]=otype[Output1->ItemIndex];
	strs[2]=otype[Output2->ItemIndex];
	strs[3]=otype[Output3->ItemIndex];
	
	strcpy(paths[0],Paths[0][ip[Input->ItemIndex]].c_str());
	strcpy(paths[1],!Output1->ItemIndex?"":Paths[1][ip[Output1->ItemIndex-1]].c_str());
	strcpy(paths[2],!Output2->ItemIndex?"":Paths[2][ip[Output2->ItemIndex-1]].c_str());
	strcpy(paths[3],!Output3->ItemIndex?"":Paths[3][ip[Output3->ItemIndex-1]].c_str());
	
	for (int i=0;i<MAXSTR;i++) {
		if (strs[i]==STR_SERIAL) {
			if (CmdEna[i][0]) cmds[i]=MainForm->Cmds[i][0].c_str();
			if (CmdEna[i][2]) cmds_periodic[i]=MainForm->Cmds[i][2].c_str();
		}
		else if (strs[i]==STR_TCPCLI||strs[i]==STR_NTRIPCLI) {
			if (CmdEnaTcp[i][0]) cmds[i]=MainForm->CmdsTcp[i][0].c_str();
			if (CmdEnaTcp[i][2]) cmds_periodic[i]=MainForm->CmdsTcp[i][2].c_str();
		}
	}
	for (int i=0;i<5;i++) {
		opt[i]=SvrOpt[i];
	}
	opt[5]=NmeaReq?SvrOpt[5]:0;
	opt[6]=FileSwapMargin;
	opt[7]=RelayBack;
	
	for (int i=1;i<MAXSTR;i++) { // for each out stream
		if (strs[i]!=STR_FILE) continue;
		strcpy(filepath,paths[i]);
		if (strstr(filepath,"::A")) continue;
		if ((p=strstr(filepath,"::"))) *p='\0';
		if (!(fp=fopen(filepath,"r"))) continue;
		fclose(fp);
		ConfDialog->Label2->Caption=filepath;
		if (ConfDialog->ShowModal()!=mrOk) return;
	}
	strsetdir(LocalDirectory.c_str());
	strsetproxy(ProxyAddress.c_str());
	
	for (int i=0;i<MAXSTR-1;i++) { // for each out stream
		if (Input->ItemIndex==2||Input->ItemIndex==4) continue;
		if (!ConvEna[i]) continue;
		if (!(conv[i]=strconvnew(ConvInp[i],ConvOut[i],ConvMsg[i].c_str(),
								 StaId,StaSel,ConvOpt[i].c_str()))) continue;
		strcpy(buff,AntType.c_str());
		for (p=strtok(buff,","),n=0;p&&n<3;p=strtok(NULL,",")) ant[n++]=p;
		strcpy(conv[i]->out.sta.antdes,ant[0]);
		strcpy(conv[i]->out.sta.antsno,ant[1]);
		conv[i]->out.sta.antsetup=atoi(ant[2]);
		strcpy(buff,RcvType.c_str());
		for (p=strtok(buff,","),n=0;p&&n<3;p=strtok(NULL,",")) rcv[n++]=p;
		strcpy(conv[i]->out.sta.rectype,rcv[0]);
		strcpy(conv[i]->out.sta.recver ,rcv[1]);
		strcpy(conv[i]->out.sta.recsno ,rcv[2]);
		matcpy(conv[i]->out.sta.pos,AntPos,3,1);
		matcpy(conv[i]->out.sta.del,AntOff,3,1);
	}
	// stream server start
	if (!strsvrstart(&strsvr,opt,strs,paths,conv,cmds,cmds_periodic,AntPos)) {
		return;
	}
	// set ntrip source table
	strsvrsetsrctbl(&strsvr,SrcTblFile.c_str());
	
	StartTime=utc2gpst(timeget());
	Panel1	  ->Enabled=false;
	BtnStart  ->Visible=false;
	BtnStop   ->Visible=true;
	BtnOpt	  ->Enabled=false;
	BtnExit   ->Enabled=false;
	MenuStart ->Enabled=false;
	MenuStop  ->Enabled=true;
	MenuExit  ->Enabled=false;
	SetTrayIcon(1);
}
// stop stream server -------------------------------------------------------
void __fastcall TMainForm::SvrStop(void)
{
	int itype[]={
		STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_NTRIPC_S,STR_UDPSVR,
		STR_FILE,STR_FTP,STR_HTTP
	};
	int otype[]={
		STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPC_C,
		STR_UDPCLI,STR_FILE
	};
	char *cmds[MAXSTR]={0};
	int strs[MAXSTR];
	
	strs[0]=itype[Input->ItemIndex];
	strs[1]=otype[Output1->ItemIndex];
	strs[2]=otype[Output2->ItemIndex];
	strs[3]=otype[Output3->ItemIndex];
	
	for (int i=0;i<MAXSTR;i++) {
		if (strs[i]==STR_SERIAL) {
			if (CmdEna[i][1]) cmds[i]=MainForm->Cmds[i][1].c_str();
		}
		else if (strs[i]==STR_TCPCLI||strs[i]==STR_NTRIPCLI) {
			if (CmdEnaTcp[i][1]) cmds[i]=MainForm->CmdsTcp[i][1].c_str();
		}
	}
	strsvrstop(&strsvr,cmds);
	
	EndTime=utc2gpst(timeget());
	Panel1	  ->Enabled=true;
	BtnStart  ->Visible=true;
	BtnStop   ->Visible=false;
	BtnOpt	  ->Enabled=true;
	BtnExit   ->Enabled=true;
	MenuStart ->Enabled=true;
	MenuStop  ->Enabled=false;
	MenuExit  ->Enabled=true;
	SetTrayIcon(0);
	
	for (int i=0;i<MAXSTR-1;i++) {
		if (ConvEna[i]) strconvfree(strsvr.conv[i]);
	}
	if (TraceLevel>0) traceclose();
}
// callback on interval timer for stream monitor ----------------------------
void __fastcall TMainForm::Timer2Timer(TObject *Sender)
{
	const char *types[]={
		"None","Serial","File","TCP Server","TCP Client","Ntrip Sever",
		"Ntrip Client","FTP","HTTP","Ntrip Cast S","Ntrip Cast C","UDP Sever",
		"UDP Client"
	};
	const char *modes[]={"-","R","W","R/W"};
	const char *states[]={"ERR","-","WAIT","CONN"};
	stream_t *str;
	unsigned char *msg;
	char *p;
	int i,len,inb,inr,outb,outr;
	
	if (StrMonDialog->StrFmt) {
		lock(&strsvr.lock);
		len=strsvr.npb;
		if (len>0&&(msg=(unsigned char *)malloc(len))) {
			memcpy(msg,strsvr.pbuf,len);
			strsvr.npb=0;
		}
		unlock(&strsvr.lock);
		if (len<=0||!msg) return;
		StrMonDialog->AddMsg(msg,len);
		free(msg);
	}
	else {
		if (!(msg=(unsigned char *)malloc(16000))) return;
		
		for (i=0,p=msg;i<MAXSTR;i++) {
			p+=sprintf(p,"[STREAM %d]\n",i);
			strsum(strsvr.stream+i,&inb,&inr,&outb,&outr);
			strstatx(strsvr.stream+i,p);
			p+=strlen(p);
			if (inb>0) {
				p+=sprintf(p,"	inb		= %d\n",inb);
				p+=sprintf(p,"	inr		= %d\n",inr);
			}
			if (outb>0) {
				p+=sprintf(p,"	outb	= %d\n",outb);
				p+=sprintf(p,"	outr	= %d\n",outr);
			}
		}
		StrMonDialog->AddMsg(msg,strlen(msg));
		
		free(msg);
	}
}
// set serial options -------------------------------------------------------
void __fastcall TMainForm::SerialOpt(int index, int opt)
{
	SerialOptDialog->Path=Paths[index][0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths[index][0]=SerialOptDialog->Path;
}
// set tcp/ip options -------------------------------------------------------
void __fastcall TMainForm::TcpOpt(int index, int opt)
{
	TcpOptDialog->Path=Paths[index][1];
	TcpOptDialog->Opt=opt;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History[i]=TcpHistory[i];
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->MntpHist[i]=TcpMntpHist[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][1]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory[i]=TcpOptDialog->History[i];
	for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=TcpOptDialog->MntpHist[i];
}
// set file options ---------------------------------------------------------
void __fastcall TMainForm::FileOpt(int index, int opt)
{
	FileOptDialog->Path=Paths[index][2];
	FileOptDialog->Opt=opt;
	if (FileOptDialog->ShowModal()!=mrOk) return;
	Paths[index][2]=FileOptDialog->Path;
}
// set ftp/http options -----------------------------------------------------
void __fastcall TMainForm::FtpOpt(int index, int opt)
{
	FtpOptDialog->Path=Paths[index][3];
	FtpOptDialog->Opt=opt;
	if (FtpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][3]=FtpOptDialog->Path;
}
// undate enable of widgets -------------------------------------------------
void __fastcall TMainForm::UpdateEnable(void)
{
	BtnCmd->Enabled=Input->ItemIndex<2||Input->ItemIndex==3;
	LabelOutput1->Font->Color=Output1->ItemIndex>0?clBlack:clGray;
	LabelOutput2->Font->Color=Output2->ItemIndex>0?clBlack:clGray;
	LabelOutput3->Font->Color=Output3->ItemIndex>0?clBlack:clGray;
	Output1Byte ->Font->Color=Output1->ItemIndex>0?clBlack:clGray;
	Output2Byte ->Font->Color=Output2->ItemIndex>0?clBlack:clGray;
	Output3Byte ->Font->Color=Output3->ItemIndex>0?clBlack:clGray;
	Output1Bps	->Font->Color=Output1->ItemIndex>0?clBlack:clGray;
	Output2Bps	->Font->Color=Output2->ItemIndex>0?clBlack:clGray;
	Output3Bps	->Font->Color=Output3->ItemIndex>0?clBlack:clGray;
	BtnOutput1->Enabled=Output1->ItemIndex>0;
	BtnOutput2->Enabled=Output2->ItemIndex>0;
	BtnOutput3->Enabled=Output3->ItemIndex>0;
	BtnCmd1   ->Enabled=BtnOutput1->Enabled&&(Output1->ItemIndex==1||Output1->ItemIndex==2);
	BtnCmd2   ->Enabled=BtnOutput2->Enabled&&(Output2->ItemIndex==1||Output2->ItemIndex==2);
	BtnCmd3   ->Enabled=BtnOutput3->Enabled&&(Output3->ItemIndex==1||Output3->ItemIndex==2);
	BtnConv1  ->Enabled=BtnOutput1->Enabled&&Input->ItemIndex!=2&&Input->ItemIndex!=4;
	BtnConv2  ->Enabled=BtnOutput2->Enabled&&Input->ItemIndex!=2&&Input->ItemIndex!=4;
	BtnConv3  ->Enabled=BtnOutput3->Enabled&&Input->ItemIndex!=2&&Input->ItemIndex!=4;
}
// set task-tray icon -------------------------------------------------------
void __fastcall TMainForm::SetTrayIcon(int index)
{
	TIcon *icon=new TIcon;
	ImageList->GetIcon(index,icon);
	TrayIcon->Icon=icon;
	delete icon;
}
// load options -------------------------------------------------------------
void __fastcall TMainForm::LoadOpt(void)
{
	TIniFile *ini=new TIniFile(IniFile);
	AnsiString s;
	int optdef[]={10000,10000,1000,32768,10,0};
	
	Input  ->ItemIndex=ini->ReadInteger("set","input",		 0);
	Output1->ItemIndex=ini->ReadInteger("set","output1",	 0);
	Output2->ItemIndex=ini->ReadInteger("set","output2",	 0);
	Output3->ItemIndex=ini->ReadInteger("set","output3",	 0);
	TraceLevel		  =ini->ReadInteger("set","tracelevel",  0);
	NmeaReq			  =ini->ReadInteger("set","nmeareq",	 0);
	FileSwapMargin	  =ini->ReadInteger("set","fswapmargin",30);
	RelayBack		  =ini->ReadInteger("set","relayback",	 0);
	ProgBarRange	  =ini->ReadInteger("set","progbarrange",2000);
	StaId			  =ini->ReadInteger("set","staid"		,0);
	StaSel			  =ini->ReadInteger("set","stasel"		,0);
	AntType			  =ini->ReadString ("set","anttype",	"");
	RcvType			  =ini->ReadString ("set","rcvtype",	"");
	
	for (int i=0;i<6;i++) {
		SvrOpt[i]=ini->ReadInteger("set",s.sprintf("svropt_%d",i),optdef[i]);
	}
	for (int i=0;i<3;i++) {
		AntPos[i]=ini->ReadFloat("set",s.sprintf("antpos_%d",i),0.0);
		AntOff[i]=ini->ReadFloat("set",s.sprintf("antoff_%d",i),0.0);
	}
	for (int i=0;i<MAXSTR-1;i++) {
		ConvEna[i]=ini->ReadInteger("conv",s.sprintf("ena_%d",i), 0);
		ConvInp[i]=ini->ReadInteger("conv",s.sprintf("inp_%d",i), 0);
		ConvOut[i]=ini->ReadInteger("conv",s.sprintf("out_%d",i), 0);
		ConvMsg[i]=ini->ReadString ("conv",s.sprintf("msg_%d",i),"");
		ConvOpt[i]=ini->ReadString ("conv",s.sprintf("opt_%d",i),"");
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<3;j++) {
		CmdEna	 [i][j]=ini->ReadInteger("serial",s.sprintf("cmdena_%d_%d",i,j),1);
		CmdEnaTcp[i][j]=ini->ReadInteger("tcpip" ,s.sprintf("cmdena_%d_%d",i,j),1);
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<4;j++) {
		Paths[i][j]=ini->ReadString("path",s.sprintf("path_%d_%d",i,j),"");
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<3;j++) {
		Cmds[i][j]=ini->ReadString("serial",s.sprintf("cmd_%d_%d",i,j),"");
		for (char *p=Cmds[i][j].c_str();*p;p++) {
			if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
		}
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<3;j++) {
		CmdsTcp[i][j]=ini->ReadString("tcpip",s.sprintf("cmd_%d_%d",i,j),"");
		for (char *p=CmdsTcp[i][j].c_str();*p;p++) {
			if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
		}
	}
	for (int i=0;i<MAXHIST;i++) {
		TcpHistory[i]=ini->ReadString("tcpopt",s.sprintf("history%d",i),"");
	}
	for (int i=0;i<MAXHIST;i++) {
		TcpMntpHist[i]=ini->ReadString("tcpopt",s.sprintf("mntphist%d",i),"");
	}
	StaPosFile	  =ini->ReadString("stapos","staposfile",	 "");
	ExeDirectory  =ini->ReadString("dirs",	"exedirectory",  "");
	LocalDirectory=ini->ReadString("dirs",	"localdirectory","");
	ProxyAddress  =ini->ReadString("dirs",	"proxyaddress",  "");
	SrcTblFile	  =ini->ReadString("file",	"srctblfile",	 "");
	LogFile		  =ini->ReadString("file",	"logfile",		 "");
	delete ini;
	
	UpdateEnable();
}
// save options--------------------------------------------------------------
void __fastcall TMainForm::SaveOpt(void)
{
	TIniFile *ini=new TIniFile(IniFile);
	AnsiString s;
	
	ini->WriteInteger("set","input",	  Input  ->ItemIndex);
	ini->WriteInteger("set","output1",	  Output1->ItemIndex);
	ini->WriteInteger("set","output2",	  Output2->ItemIndex);
	ini->WriteInteger("set","output3",	  Output3->ItemIndex);
	ini->WriteInteger("set","tracelevel", TraceLevel);
	ini->WriteInteger("set","nmeareq",	  NmeaReq);
	ini->WriteInteger("set","fswapmargin",FileSwapMargin);
	ini->WriteInteger("set","relayback",  RelayBack);
	ini->WriteInteger("set","progbarrange",ProgBarRange);
	ini->WriteInteger("set","staid",	  StaId);
	ini->WriteInteger("set","stasel",	  StaSel);
	ini->WriteString ("set","anttype",	  AntType);
	ini->WriteString ("set","rcvtype",	  RcvType);
	
	for (int i=0;i<6;i++) {
		ini->WriteInteger("set",s.sprintf("svropt_%d",i),SvrOpt[i]);
	}
	for (int i=0;i<3;i++) {
		ini->WriteFloat("set",s.sprintf("antpos_%d",i),AntPos[i]);
		ini->WriteFloat("set",s.sprintf("antoff_%d",i),AntOff[i]);
	}
	for (int i=0;i<MAXSTR-1;i++) {
		ini->WriteInteger("conv",s.sprintf("ena_%d",i),ConvEna[i]);
		ini->WriteInteger("conv",s.sprintf("inp_%d",i),ConvInp[i]);
		ini->WriteInteger("conv",s.sprintf("out_%d",i),ConvOut[i]);
		ini->WriteString ("conv",s.sprintf("msg_%d",i),ConvMsg[i]);
		ini->WriteString ("conv",s.sprintf("opt_%d",i),ConvOpt[i]);
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<3;j++) {
		ini->WriteInteger("serial",s.sprintf("cmdena_%d_%d",i,j),CmdEna   [i][j]);
		ini->WriteInteger("tcpip" ,s.sprintf("cmdena_%d_%d",i,j),CmdEnaTcp[i][j]);
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<4;j++) {
		ini->WriteString("path",s.sprintf("path_%d_%d",i,j),Paths[i][j]);
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<3;j++) {
		for (char *p=Cmds[i][j].c_str();*p;p++) {
			if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
		}
		ini->WriteString("serial",s.sprintf("cmd_%d_%d",i,j),Cmds[i][j]);
	}
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<3;j++) {
		for (char *p=CmdsTcp[i][j].c_str();*p;p++) {
			if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
		}
		ini->WriteString("tcpip",s.sprintf("cmd_%d_%d",i,j),CmdsTcp[i][j]);
	}
	for (int i=0;i<MAXHIST;i++) {
		ini->WriteString("tcpopt",s.sprintf("history%d",i),TcpOptDialog->History[i]);
	}
	for (int i=0;i<MAXHIST;i++) {
		ini->WriteString("tcpopt",s.sprintf("mntphist%d",i),TcpOptDialog->MntpHist[i]);
	}
	ini->WriteString("stapos","staposfile"	  ,StaPosFile	 );
	ini->WriteString("dirs"  ,"exedirectory"  ,ExeDirectory  );
	ini->WriteString("dirs"  ,"localdirectory",LocalDirectory);
	ini->WriteString("dirs"  ,"proxyaddress"  ,ProxyAddress  );
	ini->WriteString("file",  "srctblfile"	  ,SrcTblFile	 );
	ini->WriteString("file",  "logfile"		  ,LogFile		 );
	delete ini;
}
//---------------------------------------------------------------------------

