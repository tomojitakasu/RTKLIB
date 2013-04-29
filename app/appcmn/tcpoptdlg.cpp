//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "tcpoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTcpOptDialog *TcpOptDialog;

#define NTRIP_TIMEOUT	10000				// response timeout (ms)
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
	char *port="",*mntpnt="",*user="",*passwd="",*str="";
	char *ti[]={"TCP Server Options ","TCP Client Options",
			    "NTRIP Server Options","NTRIP Client Options"};
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
	AnsiString Addr_Text=p,Port_Text=port,MntPnt_Text=mntpnt;
	AnsiString User_Text=user,Passwd_Text=passwd,Str_Text=str;
	Addr->Text=Addr_Text;
	Port->Text=Port_Text;
	MntPnt->Text=MntPnt_Text;
	User->Text=User_Text;
	Passwd->Text=Passwd_Text;
	Str->Text=Str_Text;
	Addr->Enabled=Opt>=1;
	MntPnt->Enabled=Opt>=2;
	User->Enabled=Opt==3;
	Passwd->Enabled=Opt>=2;
	Str->Enabled=Opt==2;
	LabelAddr->Caption=Opt>=2?"NTRIP Caster Host":"TCP Server Address";
	LabelAddr->Enabled=Opt>=1;
	LabelMntPnt->Enabled=Opt>=2;
	LabelUser->Enabled=Opt==3;
	LabelPasswd->Enabled=Opt>=2;
	LabelStr->Enabled=Opt==2;
	Caption=ti[Opt];
	Addr->Items->Clear();
	MntPnt->Items->Clear();
	
	for (int i=0;i<MAXHIST;i++) {
		if (History[i]!="") Addr->Items->Add(History[i]);
	}
	for (int i=0;i<MAXHIST;i++) {
		if (MntpHist[i]!="") MntPnt->Items->Add(MntpHist[i]);
	}
	BtnNtrip->Visible=Opt>=2;
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::BtnOkClick(TObject *Sender)
{
	AnsiString User_Text=User->Text,Passwd_Text=Passwd->Text;
	AnsiString Addr_Text=Addr->Text,Port_Text=Port->Text;
	AnsiString MntPnt_Text=MntPnt->Text,Str_Text=Str->Text,s;
	
	Path=s.sprintf("%s:%s@%s:%s/%s:%s",User_Text.c_str(),Passwd_Text.c_str(),
			Addr_Text.c_str(),Port_Text.c_str(),MntPnt_Text.c_str(),
			Str_Text.c_str());
	AddHist(Addr,History);
	AddHist(MntPnt,MntpHist);
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
    AnsiString Addr_Text=Addr->Text;
    AnsiString Port_Text=Port->Text;
    ExecCmd("srctblbrows "+Addr_Text+":"+Port_Text,1);
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
