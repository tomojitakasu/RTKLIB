//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "tcpoptdlg.h"
#include "mntpoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTcpOptDialog *TcpOptDialog;

#define NTRIP_TIMEOUT	5000				// response timeout (ms)
#define NTRIP_CYCLE		50					// processing cycle (ms)
#define MAXSRCTBL		512000				// max source table size (bytes)
#define ENDSRCTBL		"ENDSOURCETABLE"	// end marker of table
#define MAXLINE			1024				// max line size (byte)

static char buff[MAXSRCTBL];

//---------------------------------------------------------------------------
__fastcall TTcpOptDialog::TTcpOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::FormShow(TObject *Sender)
{
	char buff[2048],*p,*q;
	char *port=(char *)"",*mntpnt=(char *)"",*user=(char *)"";
	char *passwd=(char *)"",*str=(char *)"";
	const char *ti[]={"TCP Server Options ","TCP Client Options",
					  "NTRIP Server Options","NTRIP Client Options",
					  "NTRIP Caster Options",
					  "NTRIP Caster Server Options", "UDP Server Options",
					  "UDP Client Options"};
	int i;

	strcpy(buff,Path.c_str());
	
	if (!(p=strchr(buff,'@'))) p=buff;
	
	if ((p=strchr(p,'/'))) {
		if ((q=strchr(p+1,':'))) {
			*q='\0'; str=q+1;
		}
		*p='\0'; mntpnt=p+1;
	}
	if ((p=strrchr(buff,'@'))) {
		*p++='\0';
		if ((q=strchr(buff,':'))) {
			*q='\0'; passwd=q+1;
		}
		user=buff;
	}
	else p=buff;
	
	if ((q=strchr(p,':'))) {
		*q='\0'; port=q+1;
	}
	Caption=ti[Opt];
	LabelAddr  ->Caption=(Opt>=2&&Opt<=5)?"NTRIP Caster Address":"Server Address";
	LabelAddr  ->Enabled=(Opt>=1&&Opt<=3)||Opt==7;
	Addr       ->Enabled=(Opt>=1&&Opt<=3)||Opt==7;
	LabelMntPnt->Enabled=(Opt>=2&&Opt<=4);
	MntPnt     ->Enabled=(Opt>=2&&Opt<=4);
	LabelUser  ->Enabled=(Opt>=3&&Opt<=4);
	User       ->Enabled=(Opt>=3&&Opt<=4);
	LabelPasswd->Enabled=(Opt>=2&&Opt<=5);
	Passwd     ->Enabled=(Opt>=2&&Opt<=5);
	BtnNtrip   ->Visible=(Opt==3);
	BtnBrows   ->Visible=(Opt==3);
	BtnMountp  ->Visible=(Opt==2||Opt==4);
	
	Addr  ->Text=p;
	Port  ->Text=port;
	MntPnt->Text=mntpnt;
	User  ->Text=user;
	Passwd->Text=passwd;
	if (Opt==2||Opt==4) {
	    MntpStr=str;
	}
	Addr->Items->Clear();
	for (int i=0;i<MAXHIST;i++) {
		if (History[i]!="") Addr->Items->Add(History[i]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::BtnOkClick(TObject *Sender)
{
	AnsiString User_Text=User->Text,Passwd_Text=Passwd->Text;
	AnsiString Addr_Text=Addr->Text,Port_Text=Port->Text;
	AnsiString MntPnt_Text=MntPnt->Text,s;
	
	Path=s.sprintf("%s:%s@%s:%s/%s:%s",User_Text.c_str(),Passwd_Text.c_str(),
			Addr_Text.c_str(),Port_Text.c_str(),MntPnt_Text.c_str(),
			MntpStr.c_str());
	AddHist(Addr,History);
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::AddHist(TComboBox *list, AnsiString *hist)
{
	for (int i=0;i<MAXHIST;i++) {
		if (list->Text!=hist[i]) continue;
		for (int j=i+1;j<MAXHIST;j++) hist[j-1]=hist[j];
		hist[MAXHIST-1]="";
	}
	for (int i=MAXHIST-1;i>0;i--) hist[i]=hist[i-1];
	hist[0]=list->Text;
	
	list->Clear();
	for (int i=0;i<MAXHIST;i++) {
		if (hist[i]!="") list->Items->Add(hist[i]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::BtnNtripClick(TObject *Sender)
{
	TButton *btn=(TButton *)Sender;
    AnsiString path=Addr->Text+":"+Port->Text;
	stream_t str;
	uint32_t tick=tickget();
	char *p=buff,msg[MAXSTRMSG],mntpnt[256];
	
	strinit(&str);
	if (!stropen(&str,STR_NTRIPCLI,STR_MODE_R,path.c_str())) return;
	
	btn->Enabled=false;
	*p='\0';
	while (p<buff+MAXSRCTBL-1) {
		p+=strread(&str,(uint8_t *)p,buff+MAXSRCTBL-p-1);
		*p='\0';
		sleepms(NTRIP_CYCLE);
		if (strstr(buff,ENDSRCTBL)) break;
		if ((int)(tickget()-tick)>NTRIP_TIMEOUT) break;
	}
	strclose(&str);
	
	MntPnt->Clear();
	for (p=buff;(p=strstr(p,"STR;"));p+=4) {
		if (sscanf(p,"STR;%255[^;]",mntpnt)==1) {
			MntPnt->AddItem(mntpnt,NULL);
		}
	}
	btn->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::BtnBrowsClick(TObject *Sender)
{
    AnsiString Addr_Text=Addr->Text;
    AnsiString Port_Text=Port->Text;
	if (Port_Text!="") Addr_Text+=":"+Port_Text;
    ExecCmd("srctblbrows "+Addr_Text,1);
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::BtnMountpClick(TObject *Sender)
{
	MntpOptDialog->MntPnt=MntPnt->Text;
	MntpOptDialog->MntpStr=MntpStr;
    if (MntpOptDialog->ShowModal()!=mrOk) return;
	MntPnt->Text=MntpOptDialog->MntPnt;
	MntpStr=MntpOptDialog->MntpStr;
}
//---------------------------------------------------------------------------
int __fastcall TTcpOptDialog::ExecCmd(AnsiString cmd, int show)
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
//---------------------------------------------------------------------------

