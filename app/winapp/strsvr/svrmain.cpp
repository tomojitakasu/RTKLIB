//---------------------------------------------------------------------------
// strsvr : stream server
//
//			Copyright (C) 2007-2020 by T.TAKASU, All rights reserved.
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
//			 2020/11/30  1.5 number of output channels 3 -> 5
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
	n=sprintf(buff,"%u",(uint32_t)num);
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
	
	strsvrinit(&strsvr,MAXSTR-1);
	
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
	LogFile=SvrOptDialog->LogFile;
}
// callback on button-input-opt ---------------------------------------------
void __fastcall TMainForm::BtnInputClick(TObject *Sender)
{
	switch (Input->ItemIndex) {
		case 0: SerialOpt  (0,0); break;
		case 1: TcpCliOpt  (0,1); break;
		case 2: TcpSvrOpt  (0,2); break;
		case 3: NtripCliOpt(0,3); break;
		case 4: UdpSvrOpt  (0,4); break;
		case 5: FileOpt    (0,5); break;
	}
}
// callback on button-cmd ---------------------------------------------------
void __fastcall TMainForm::BtnCmdClick(TObject *Sender)
{
	TButton *btn[]={BtnCmd,BtnCmd1,BtnCmd2,BtnCmd3,BtnCmd4,BtnCmd5,BtnCmd6};
	TComboBox *type[]={Input,Output1,Output2,Output3,Output4,Output5,Output6};
	AnsiString s;
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
	if (i==0) CmdOptDialog->Caption=s.sprintf("Input Serial/TCP Commands");
	else CmdOptDialog->Caption=s.sprintf("Output%d Serial/TCP Commands",i);
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
// callback on button-output-opt --------------------------------------------
void __fastcall TMainForm::BtnOutputClick(TObject *Sender)
{
	TButton *btn[]={BtnOutput1,BtnOutput2,BtnOutput3,BtnOutput4,BtnOutput5,BtnOutput6};
	TComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};
	int i;

	for (i=0;i<MAXSTR-1;i++) {
		if ((TButton *)Sender==btn[i]) break;
	}
	if (i>=MAXSTR-1) return;

	switch (type[i]->ItemIndex) {
		case 1: SerialOpt  (i+1,0); break;
		case 2: TcpCliOpt  (i+1,1); break;
		case 3: TcpSvrOpt  (i+1,2); break; 
		case 4: NtripSvrOpt(i+1,3); break;
		case 5: NtripCasOpt(i+1,4); break;
		case 6: UdpCliOpt  (i+1,5); break;
		case 7: FileOpt    (i+1,6); break;
	}
}
// callback on button-conv --------------------------------------------------
void __fastcall TMainForm::BtnConvClick(TObject *Sender)
{
	TButton *btn[]={BtnConv1,BtnConv2,BtnConv3,BtnConv4,BtnConv5,BtnConv6};
	AnsiString s;
	int i;

	for (i=0;i<MAXSTR-1;i++) {
		if ((TButton *)Sender==btn[i]) break;
	}
	if (i>=MAXSTR-1) return;
	
	ConvDialog->ConvEna=ConvEna[i];
	ConvDialog->ConvInp=ConvInp[i];
	ConvDialog->ConvOut=ConvOut[i];
	ConvDialog->ConvMsg=ConvMsg[i];
	ConvDialog->ConvOpt=ConvOpt[i];
	ConvDialog->Caption=s.sprintf("Output%d Conversion Options",i+1);
	if (ConvDialog->ShowModal()!=mrOk) return;
	ConvEna[i]=ConvDialog->ConvEna;
	ConvInp[i]=ConvDialog->ConvInp;
	ConvOut[i]=ConvDialog->ConvOut;
	ConvMsg[i]=ConvDialog->ConvMsg;
	ConvOpt[i]=ConvDialog->ConvOpt;
}
// callback on about --------------------------------------------------------
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
// callback on output2 enable -----------------------------------------------// callback on output3 enable -----------------------------------------------// callback on input type change --------------------------------------------
void __fastcall TMainForm::InputChange(TObject *Sender)
{
	UpdateEnable();
}
// callback on output1 type change ------------------------------------------
void __fastcall TMainForm::OutputChange(TObject *Sender)
{
	UpdateEnable(); 
}
// callback on interval timer -----------------------------------------------
void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
	TColor color[]={clRed,clWhite,CLORANGE,clGreen,clLime,clAqua};
	TPanel *e0[]={IndInput,IndOutput1,IndOutput2,IndOutput3,IndOutput4,IndOutput5,IndOutput6};
	TLabel *e1[]={InputByte,Output1Byte,Output2Byte,Output3Byte,Output4Byte,Output5Byte,Output6Byte};
	TLabel *e2[]={InputBps,Output1Bps,Output2Bps,Output3Bps,Output4Bps,Output5Bps,Output6Bps};
	TPanel *e3[]={IndLog,IndLog1,IndLog2,IndLog3,IndLog4,IndLog5,IndLog6};
	AnsiString s;
	gtime_t time=utc2gpst(timeget());
	int stat[MAXSTR]={0},byte[MAXSTR]={0},bps[MAXSTR]={0},log_stat[MAXSTR]={0};
	char msg[MAXSTRMSG*MAXSTR]="",s1[256],s2[256];
	double ctime,t[4],pos,range;
	
	strsvrstat(&strsvr,stat,log_stat,byte,bps,msg);
	for (int i=0;i<MAXSTR;i++) {
		num2cnum(byte[i],s1);
		num2cnum(bps[i],s2);
		e0[i]->Color=color[stat[i]+1];
		e1[i]->Caption=s1;
		e2[i]->Caption=s2;
		e3[i]->Color=color[log_stat[i]+1];
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
	Message->Hint=msg;
}
// start stream server ------------------------------------------------------
void __fastcall TMainForm::SvrStart(void)
{
	TComboBox *type[]={Input,Output1,Output2,Output3,Output4,Output5,Output6};
	strconv_t *conv[MAXSTR-1]={0};
	static char str1[MAXSTR][1024],str2[MAXSTR][1024];
	int itype[]={
		STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_UDPSVR,STR_FILE,
		STR_FTP,STR_HTTP
	};
	int otype[]={
		STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPCAS,
		STR_UDPCLI,STR_FILE
	};
	int strs[MAXSTR]={0},opt[8]={0},n;
	char *paths[MAXSTR],*logs[MAXSTR],*cmds[MAXSTR]={0},*cmds_periodic[MAXSTR]={0};
	char filepath[1024],buff[1024],*p;
	const char *ant[3]={"","",""},*rcv[3]={"","",""};
	FILE *fp;
	
	if (TraceLevel>0) {
		traceopen(*LogFile.c_str()?LogFile.c_str():TRACEFILE);
		tracelevel(TraceLevel);
	}
	for (int i=0;i<MAXSTR;i++) {
		paths[i]=str1[i];
		logs [i]=str2[i];
	}
	strs[0]=itype[type[0]->ItemIndex];
	strcpy(paths[0],Paths[0][type[0]->ItemIndex].c_str());
	strcpy(logs[0],type[0]->ItemIndex>5||!PathEna[0]?"":PathLog[0].c_str());
	
	for (int i=1;i<MAXSTR;i++) {
	    strs[i]=otype[type[i]->ItemIndex];
	    strcpy(paths[i],!type[i]->ItemIndex?"":Paths[i][type[i]->ItemIndex-1].c_str());
	    strcpy(logs[i],!PathEna[i]?"":PathLog[i].c_str());
    }
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
	for (int i=0;i<MAXSTR;i++) { // for each log stream
		if (!*logs[i]) continue;
		strcpy(filepath,logs[i]);
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
	if (!strsvrstart(&strsvr,opt,strs,paths,logs,conv,cmds,cmds_periodic,AntPos)) {
		return;
	}
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
	TComboBox *type[]={Input,Output1,Output2,Output3,Output4,Output5,Output6};
	int itype[]={
		STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_UDPSVR,STR_FILE,
		STR_FTP,STR_HTTP
	};
	int otype[]={
		STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPCAS,
		STR_UDPCLI,STR_FILE
	};
	char *cmds[MAXSTR]={0};
	int strs[MAXSTR];
	
	strs[0]=itype[Input->ItemIndex];
	for (int i=1;i<MAXSTR;i++) {
	    strs[i]=otype[type[i]->ItemIndex];
    }
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
		"Ntrip Client","FTP","HTTP","Ntrip Cast","UDP Sever","UDP Client"
	};
	const char *modes[]={"-","R","W","R/W"};
	const char *states[]={"ERR","-","WAIT","CONN"};
	stream_t *str;
	char *msg, *p;
	int i,len,inb,inr,outb,outr;
	
	if (StrMonDialog->StrFmt) {
		lock(&strsvr.lock);
		len=strsvr.npb;
		if (len>0&&(msg=(char *)malloc(len))) {
			memcpy(msg,strsvr.pbuf,len);
			strsvr.npb=0;
		}
		unlock(&strsvr.lock);
		if (len<=0||!msg) return;
		StrMonDialog->AddMsg((uint8_t *)msg,len);
		free(msg);
	}
	else {
		if (!(msg=(char *)malloc(16000))) return;
		
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
		StrMonDialog->AddMsg((uint8_t *)msg,strlen(msg));
		
		free(msg);
	}
}
// set serial options -------------------------------------------------------
void __fastcall TMainForm::SerialOpt(int index, int path)
{
	SerialOptDialog->Path=Paths[index][path];
	SerialOptDialog->Opt=(index==0)?0:1;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=SerialOptDialog->Path;
}
// set tcp server options ---------------------------------------------------
void __fastcall TMainForm::TcpSvrOpt(int index, int path)
{
	TcpOptDialog->Path=Paths[index][path];
	TcpOptDialog->Opt=0;
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=TcpOptDialog->Path;
}
// set tcp client options ---------------------------------------------------
void __fastcall TMainForm::TcpCliOpt(int index, int path)
{
	TcpOptDialog->Path=Paths[index][path];
	TcpOptDialog->Opt=1;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History[i]=TcpHistory[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory[i]=TcpOptDialog->History[i];
}
// set ntrip server options -------------------------------------------------
void __fastcall TMainForm::NtripSvrOpt(int index, int path)
{
	TcpOptDialog->Path=Paths[index][path];
	TcpOptDialog->Opt=2;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History[i]=TcpHistory[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory[i]=TcpOptDialog->History[i];
}
// set ntrip client options -------------------------------------------------
void __fastcall TMainForm::NtripCliOpt(int index, int path)
{
	TcpOptDialog->Path=Paths[index][path];
	TcpOptDialog->Opt=3;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History[i]=TcpHistory[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory[i]=TcpOptDialog->History[i];
}
// set ntrip caster options -------------------------------------------------
void __fastcall TMainForm::NtripCasOpt(int index, int path)
{
	TcpOptDialog->Path=Paths[index][path];
	TcpOptDialog->Opt=4;
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=TcpOptDialog->Path;
}
// set udp server options ---------------------------------------------------
void __fastcall TMainForm::UdpSvrOpt(int index, int path)
{
	TcpOptDialog->Path=Paths[index][path];
	TcpOptDialog->Opt=6;
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=TcpOptDialog->Path;
}
// set udp client options ---------------------------------------------------
void __fastcall TMainForm::UdpCliOpt(int index, int path)
{
	TcpOptDialog->Path=Paths[index][path];
	TcpOptDialog->Opt=7;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History[i]=TcpHistory[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory[i]=TcpOptDialog->History[i];
}
// set file options ---------------------------------------------------------
void __fastcall TMainForm::FileOpt(int index, int path)
{
	FileOptDialog->Path=Paths[index][path];
	FileOptDialog->Caption="File Options";
	FileOptDialog->Opt=(index==0)?0:1;
	if (FileOptDialog->ShowModal()!=mrOk) return;
	Paths[index][path]=FileOptDialog->Path;
}
// undate enable of widgets -------------------------------------------------
void __fastcall TMainForm::UpdateEnable(void)
{
	TComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};
	TLabel *label1[]={LabelOutput1,LabelOutput2,LabelOutput3,LabelOutput4,LabelOutput5,LabelOutput6};
	TLabel *label2[]={Output1Byte,Output2Byte,Output3Byte,Output4Byte,Output5Byte,Output6Byte};
	TLabel *label3[]={Output1Bps,Output2Bps,Output3Bps,Output4Bps,Output5Bps,Output6Bps};
	TButton *btn1[]={BtnOutput1,BtnOutput2,BtnOutput3,BtnOutput4,BtnOutput5,BtnOutput6};
	TButton *btn2[]={BtnCmd1,BtnCmd2,BtnCmd3,BtnCmd4,BtnCmd5,BtnCmd6};
	TButton *btn3[]={BtnConv1,BtnConv2,BtnConv3,BtnConv4,BtnConv5,BtnConv6};
	TButton *btn4[]={BtnLog1,BtnLog2,BtnLog3,BtnLog4,BtnLog5,BtnLog6};
	
	BtnCmd->Enabled=Input->ItemIndex<2||Input->ItemIndex==3;
	BtnLog->Enabled=Input->ItemIndex<6;
	for (int i=0;i<MAXSTR-1;i++) {
	    label1[i]->Font->Color=type[i]->ItemIndex>0?clBlack:clGray;
	    label2[i]->Font->Color=type[i]->ItemIndex>0?clBlack:clGray;
	    label3[i]->Font->Color=type[i]->ItemIndex>0?clBlack:clGray;
	    btn1[i]->Enabled=type[i]->ItemIndex>0;
	    btn2[i]->Enabled=btn1[i]->Enabled&&(type[i]->ItemIndex==1||type[i]->ItemIndex==2);
	    btn3[i]->Enabled=btn1[i]->Enabled&&Input->ItemIndex!=2&&Input->ItemIndex!=4;
	    btn4[i]->Enabled=btn1[i]->Enabled&&(type[i]->ItemIndex==1||type[i]->ItemIndex==2);
    }
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
	TComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};
	AnsiString s;
	int optdef[]={10000,10000,1000,32768,10,0};
	
	Input  ->ItemIndex=ini->ReadInteger("set","input",		 0);
	for (int i=0;i<MAXSTR-1;i++) {
		s.sprintf("output_%d",i+1);
		type[i]->ItemIndex=ini->ReadInteger("set",s,0);
	}
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
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<7;j++) {
		Paths[i][j]=ini->ReadString("path",s.sprintf("path_%d_%d",i,j),"");
	}
	for (int i=0;i<MAXSTR;i++) {
		PathLog[i]=ini->ReadString ("path",s.sprintf("path_log_%d",i),"");
		PathEna[i]=ini->ReadInteger("path",s.sprintf("path_ena_%d",i),0);
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
	StaPosFile	  =ini->ReadString("stapos","staposfile",	 "");
	ExeDirectory  =ini->ReadString("dirs",	"exedirectory",  "");
	LocalDirectory=ini->ReadString("dirs",	"localdirectory","");
	ProxyAddress  =ini->ReadString("dirs",	"proxyaddress",  "");
	LogFile		  =ini->ReadString("file",	"logfile",		 "");
	Height        =ini->ReadInteger("window","height",      271);
	delete ini;
	
	UpdateEnable();
}
// save options--------------------------------------------------------------
void __fastcall TMainForm::SaveOpt(void)
{
	TIniFile *ini=new TIniFile(IniFile);
	TComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};
	AnsiString s;
	
	ini->WriteInteger("set","input",	  Input  ->ItemIndex);
	for (int i=0;i<MAXSTR-1;i++) {
	    s.printf("output_%d",i+1);
	    ini->WriteInteger("set",s,type[i]->ItemIndex);
	}
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
	for (int i=0;i<MAXSTR;i++) for (int j=0;j<7;j++) {
		ini->WriteString("path",s.sprintf("path_%d_%d",i,j),Paths[i][j]);
	}
	for (int i=0;i<MAXSTR;i++) {
		ini->WriteString ("path",s.sprintf("path_log_%d",i),PathLog[i]);
		ini->WriteInteger("path",s.sprintf("path_ena_%d",i),PathEna[i]);
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
	ini->WriteString("stapos","staposfile"	  ,StaPosFile	 );
	ini->WriteString("dirs"  ,"exedirectory"  ,ExeDirectory  );
	ini->WriteString("dirs"  ,"localdirectory",LocalDirectory);
	ini->WriteString("dirs"  ,"proxyaddress"  ,ProxyAddress  );
	ini->WriteString("file",  "logfile"		  ,LogFile		 );
	ini->WriteInteger("window","height"		  ,Height		 );
	delete ini;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnLogClick(TObject *Sender)
{
	TButton *btn[]={BtnLog,BtnLog1,BtnLog2,BtnLog3,BtnLog4,BtnLog5,BtnLog6};
	int i;

	for (i=0;i<MAXSTR;i++) {
		if ((TButton *)Sender==btn[i]) break;
	}
	if (i>=MAXSTR) return;

	FileOptDialog->Path=PathLog[i];
	FileOptDialog->PathEna=PathEna[i];
	FileOptDialog->Caption=(i==0)?"Input Log Options":"Return Log Options";
	FileOptDialog->Opt=2;
	if (FileOptDialog->ShowModal()!=mrOk) return;
	PathLog[i]=FileOptDialog->Path;
	PathEna[i]=FileOptDialog->PathEna;
}
//---------------------------------------------------------------------------

