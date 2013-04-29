//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "cmdoptdlg.h"
#include "serioptdlg.h"

#define MAXCOMNO	99				// maximun com number

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSerialOptDialog *SerialOptDialog;
//---------------------------------------------------------------------------
__fastcall TSerialOptDialog::TSerialOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	Opt=0;
}
//---------------------------------------------------------------------------
void __fastcall TSerialOptDialog::FormShow(TObject *Sender)
{
	char *p,*q,path[1024];
	
	UpdatePortList();
	strcpy(path,Path.c_str());
	if (!(q=strchr(p=path,':'))) return; else *q='\0';
	Port->Text=p;
	if (!(q=strchr(p=q+1,':'))) return; else *q='\0';
	BitRate->Text=p;
	if (!(q=strchr(p=q+1,':'))) return; else *q='\0';
	ByteSize->ItemIndex=!strcmp(p,"7")?0:1;
	if (!(q=strchr(p=q+1,':'))) return; else *q='\0';
	Parity->ItemIndex=!strcmp(p,"n")?0:(!strcmp(p,"e")?1:2);
	if (!(q=strchr(p=q+1,':'))) return; else *q='\0';
	StopBits->ItemIndex=!strcmp(p,"1")?0:1;
	p=q+1;
	FlowCtr->ItemIndex=!strcmp(p,"off")?0:(!strcmp(p,"xon")?1:2);
	BtnCmd->Visible=Opt;
}
//---------------------------------------------------------------------------
void __fastcall TSerialOptDialog::BtnCmdClick(TObject *Sender)
{
	for (int i=0;i<2;i++) {
		CmdOptDialog->Cmds[i]=Cmds[i];
		CmdOptDialog->CmdEna[i]=CmdEna[i];
	}
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	for (int i=0;i<2;i++) {
		Cmds[i]=CmdOptDialog->Cmds[i];
		CmdEna[i]=CmdOptDialog->CmdEna[i];
	}
}
//---------------------------------------------------------------------------
void __fastcall TSerialOptDialog::BtnOkClick(TObject *Sender)
{
   	char *parity[]={"n","e","o"},*fctr[]={"off","rts"};
	AnsiString s,Port_Text=Port->Text,BitRate_Text=BitRate->Text;
	Path=s.sprintf("%s:%s:%d:%s:%d:%s",Port_Text.c_str(),BitRate_Text.c_str(),
			ByteSize->ItemIndex?8:7,parity[Parity->ItemIndex],
			StopBits->ItemIndex?2:1,fctr[FlowCtr->ItemIndex]);
}
//---------------------------------------------------------------------------
void __fastcall TSerialOptDialog::UpdatePortList(void)
{
	HANDLE h;
	char port[64];
	
	Port->Items->Clear();
	for (int i=1;i<=MAXCOMNO;i++) {
		sprintf(port,"\\\\.\\COM%d",i);
		h=CreateFile(port,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,NULL);
		if (h==INVALID_HANDLE_VALUE) continue;
		Port->Items->Add(port+4);
		CloseHandle(h);
	}
}
//---------------------------------------------------------------------------
