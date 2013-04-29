//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "cmdoptdlg.h"
#include "conndlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConnectDialog *ConnectDialog;
//---------------------------------------------------------------------------
__fastcall TConnectDialog::TConnectDialog(TComponent* Owner)
	: TForm(Owner)
{
	Stream1=Stream2=Format1=Format2=0;
	CmdEna1[0]=CmdEna1[1]=CmdEna2[0]=CmdEna2[1]=0;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	int str[]={STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE};
	for (int i=0;i<6;i++) {
		if (str[i]==Stream1) SelStream1->ItemIndex=i;
		if (str[i]==Stream2) SelStream2->ItemIndex=i;
	}
	SolFormat1->ItemIndex=Format1;
	SolFormat2->ItemIndex=Format2;
	TimeFormS->ItemIndex=TimeForm;
	DegFormS ->ItemIndex=DegForm;
	FieldSepS->Text     =FieldSep;
	TimeOutTimeE->Text=s.sprintf("%d",TimeOutTime);
	ReConnTimeE ->Text=s.sprintf("%d",ReConnTime);
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnOkClick(TObject *Sender)
{
	int str[]={STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE};
	Stream1=str[SelStream1->ItemIndex];
	Stream2=str[SelStream2->ItemIndex];
	Format1=SolFormat1->ItemIndex;
	Format2=SolFormat2->ItemIndex;
	TimeForm=TimeFormS->ItemIndex;
	DegForm =DegFormS ->ItemIndex;
	FieldSep=FieldSepS->Text;
	TimeOutTime=TimeOutTimeE->Text.ToInt();
	ReConnTime =ReConnTimeE ->Text.ToInt();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnOpt1Click(TObject *Sender)
{
	switch (SelStream1->ItemIndex) {
		case 1: SerialOpt1(0); break;
		case 2: TcpOpt1 (1);   break;
		case 3: TcpOpt1 (0);   break;
		case 4: TcpOpt1 (3);   break;
		case 5: FileOpt1(0);   break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnOpt2Click(TObject *Sender)
{
	switch (SelStream2->ItemIndex) {
		case 1: SerialOpt2(0); break;
		case 2: TcpOpt2 (1);   break;
		case 3: TcpOpt2 (0);   break;
		case 4: TcpOpt2 (3);   break;
		case 5: FileOpt2(0);   break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnCmd1Click(TObject *Sender)
{
	CmdOptDialog->Cmds  [0]=Cmds1  [0];
	CmdOptDialog->Cmds  [1]=Cmds1  [1];
	CmdOptDialog->CmdEna[0]=CmdEna1[0];
	CmdOptDialog->CmdEna[1]=CmdEna1[1];
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	Cmds1  [0]=CmdOptDialog->Cmds  [0];
	Cmds1  [1]=CmdOptDialog->Cmds  [1];
	CmdEna1[0]=CmdOptDialog->CmdEna[0];
	CmdEna1[1]=CmdOptDialog->CmdEna[1];
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::BtnCmd2Click(TObject *Sender)
{
	CmdOptDialog->Cmds  [0]=Cmds2  [0];
	CmdOptDialog->Cmds  [1]=Cmds2  [1];
	CmdOptDialog->CmdEna[0]=CmdEna2[0];
	CmdOptDialog->CmdEna[1]=CmdEna2[1];
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	Cmds2  [0]=CmdOptDialog->Cmds  [0];
	Cmds2  [1]=CmdOptDialog->Cmds  [1];
	CmdEna2[0]=CmdOptDialog->CmdEna[0];
	CmdEna2[1]=CmdOptDialog->CmdEna[1];
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SelStream1Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SelStream2Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SolFormat1Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SolFormat2Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SerialOpt1(int opt)
{
	SerialOptDialog->Path=Paths1[0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths1[0]=SerialOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::SerialOpt2(int opt)
{
	SerialOptDialog->Path=Paths2[0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths2[0]=SerialOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::TcpOpt1(int opt)
{
	TcpOptDialog->Path=Paths1[1];
	TcpOptDialog->Opt=opt;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History [i]=TcpHistory [i];
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->MntpHist[i]=TcpMntpHist[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths1[1]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory [i]=TcpOptDialog->History [i];
	for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=TcpOptDialog->MntpHist[i];
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::TcpOpt2(int opt)
{
	TcpOptDialog->Path=Paths2[1];
	TcpOptDialog->Opt=opt;
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->History [i]=TcpHistory [i];
	for (int i=0;i<MAXHIST;i++) TcpOptDialog->MntpHist[i]=TcpMntpHist[i];
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths2[1]=TcpOptDialog->Path;
	for (int i=0;i<MAXHIST;i++) TcpHistory [i]=TcpOptDialog->History [i];
	for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=TcpOptDialog->MntpHist[i];
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::FileOpt1(int opt)
{
	FileOptDialog->Path=Paths1[2];
	FileOptDialog->Opt=opt;
	if (FileOptDialog->ShowModal()!=mrOk) return;
	Paths1[2]=FileOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::FileOpt2(int opt)
{
	FileOptDialog->Path=Paths2[2];
	FileOptDialog->Opt=opt;
	if (FileOptDialog->ShowModal()!=mrOk) return;
	Paths2[2]=FileOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TConnectDialog::UpdateEnable(void)
{
	BtnOpt1     ->Enabled=SelStream1->ItemIndex>0;
	BtnOpt2     ->Enabled=SelStream2->ItemIndex>0;
	BtnCmd1     ->Enabled=SelStream1->ItemIndex==1;
	BtnCmd2     ->Enabled=SelStream2->ItemIndex==1;
	SolFormat1  ->Enabled=SelStream1->ItemIndex>0;
	SolFormat2  ->Enabled=SelStream2->ItemIndex>0;
	TimeFormS   ->Enabled=SolFormat1->ItemIndex!=3||SolFormat2->ItemIndex!=3;
	DegFormS    ->Enabled=SolFormat1->ItemIndex==0||SolFormat2->ItemIndex==0;
	FieldSepS   ->Enabled=SolFormat1->ItemIndex!=3||SolFormat2->ItemIndex!=3;
	Label5      ->Enabled=SolFormat1->ItemIndex!=3||SolFormat2->ItemIndex!=3;
	Label6      ->Enabled=SolFormat1->ItemIndex==0||SolFormat2->ItemIndex==0;
	Label7      ->Enabled=SolFormat1->ItemIndex!=3||SolFormat2->ItemIndex!=3;
	Label8      ->Enabled=2<=SelStream1->ItemIndex&&SelStream1->ItemIndex<=4||
						  2<=SelStream2->ItemIndex&&SelStream2->ItemIndex<=4;
	TimeOutTimeE->Enabled=2<=SelStream1->ItemIndex&&SelStream1->ItemIndex<=4||
						  2<=SelStream2->ItemIndex&&SelStream2->ItemIndex<=4;
	ReConnTimeE ->Enabled=2<=SelStream1->ItemIndex&&SelStream1->ItemIndex<=4||
						  2<=SelStream2->ItemIndex&&SelStream2->ItemIndex<=4;
}
//---------------------------------------------------------------------------

