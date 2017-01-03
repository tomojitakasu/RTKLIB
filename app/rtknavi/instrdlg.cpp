//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "refdlg.h"
#include "navimain.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "ftpoptdlg.h"
#include "rcvoptdlg.h"
#include "instrdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TInputStrDialog *InputStrDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TInputStrDialog::TInputStrDialog(TComponent* Owner)
	: TForm(Owner)
{
	int i;
	Format1->Items->Clear();
	Format2->Items->Clear();
	NRcv=0;
	for (i=0;i<=MAXRCVFMT;i++) {
		Format1->Items->Add(formatstrs[i]);
		Format2->Items->Add(formatstrs[i]);
		Format3->Items->Add(formatstrs[i]);
		NRcv++;
	}
	Format3->Items->Add("SP3");
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	StreamC1  ->Checked  =StreamC[0];
	StreamC2  ->Checked  =StreamC[1];
	StreamC3  ->Checked  =StreamC[2];
	Stream1   ->ItemIndex=Stream[0];
	Stream2   ->ItemIndex=Stream[1];
	Stream3   ->ItemIndex=Stream[2];
	Format1   ->ItemIndex=Format[0];
	Format2   ->ItemIndex=Format[1]<NRcv?Format[1]:NRcv+Format[1]-STRFMT_SP3;
	Format3   ->ItemIndex=Format[2]<NRcv?Format[2]:NRcv+Format[2]-STRFMT_SP3;
	FilePath1 ->Text     =GetFilePath(Paths[0][2]);
	FilePath2 ->Text     =GetFilePath(Paths[1][2]);
	FilePath3 ->Text     =GetFilePath(Paths[2][2]);
	NmeaReqL  ->ItemIndex=NmeaReq;
	TimeTagC  ->Checked  =TimeTag;
	TimeSpeedL->Text     =TimeSpeed;
	TimeStartE->Text     =TimeStart;
	Chk64Bit  ->Checked  =Time64Bit;
	NmeaPos1  ->Text     =s.sprintf("%.9f",NmeaPos[0]);
	NmeaPos2  ->Text     =s.sprintf("%.9f",NmeaPos[1]);
	NmeaPos3  ->Text     =s.sprintf("%.3f",NmeaPos[2]);
	EditMaxBL ->Text     =s.sprintf("%.0f",MaxBL);
	EditResetCmd->Text   =ResetCmd;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnOkClick(TObject *Sender)
{
	StreamC[0] =StreamC1  ->Checked;
	StreamC[1] =StreamC2  ->Checked;
	StreamC[2] =StreamC3  ->Checked;
	Stream[0]  =Stream1   ->ItemIndex;
	Stream[1]  =Stream2   ->ItemIndex;
	Stream[2]  =Stream3   ->ItemIndex;
	Format[0]  =Format1   ->ItemIndex;
	Format[1]  =Format2->ItemIndex<NRcv?Format2->ItemIndex:STRFMT_SP3+Format2->ItemIndex-NRcv;
	Format[2]  =Format3->ItemIndex<NRcv?Format3->ItemIndex:STRFMT_SP3+Format3->ItemIndex-NRcv;
	Paths[0][2]=SetFilePath(FilePath1->Text);
	Paths[1][2]=SetFilePath(FilePath2->Text);
	Paths[2][2]=SetFilePath(FilePath3->Text);
	NmeaReq    =NmeaReqL  ->ItemIndex;
	TimeTag    =TimeTagC  ->Checked;
	TimeSpeed  =TimeSpeedL->Text;
	TimeStart  =TimeStartE->Text;
	Time64Bit  =Chk64Bit  ->Checked;
	NmeaPos[0] =str2dbl(NmeaPos1->Text);
	NmeaPos[1] =str2dbl(NmeaPos2->Text);
	NmeaPos[2] =str2dbl(NmeaPos3->Text);
	MaxBL      =str2dbl(EditMaxBL->Text);
	ResetCmd   =EditResetCmd->Text;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::StreamC1Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------

void __fastcall TInputStrDialog::StreamC2Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::Stream1Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::Stream2Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::Stream3Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::NmeaReqCClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::TimeTagCClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::NmeaReqLChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
AnsiString __fastcall TInputStrDialog::GetFilePath(AnsiString path)
{
	char *p,*q,buff[1024];
	strcpy(buff,path.c_str());
	if ((p=strstr(buff,"::"))) *p='\0';
	return (path=buff);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TInputStrDialog::SetFilePath(AnsiString path)
{
	if (TimeTagC->Checked     ) path+="::T";
	if (TimeStartE->Text!="0" ) path+="::+"+TimeStartE->Text;
	path+="::"+TimeSpeedL->Text;
	if (Chk64Bit->Checked     ) path+="::P=8";
	return path;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnStr1Click(TObject *Sender)
{
	switch (Stream1->ItemIndex) {
		case 0: SerialOpt(0,0); break;
		case 1: TcpOpt(0,1); break;
		case 2: TcpOpt(0,0); break;
		case 3: TcpOpt(0,3); break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnStr2Click(TObject *Sender)
{
	switch (Stream2->ItemIndex) {
		case 0: SerialOpt(1,0); break;
		case 1: TcpOpt(1,1); break;
		case 2: TcpOpt(1,0); break;
		case 3: TcpOpt(1,3); break;
		case 5: FtpOpt(1,0); break;
		case 6: FtpOpt(1,1); break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnStr3Click(TObject *Sender)
{
	switch (Stream3->ItemIndex) {
		case 0: SerialOpt(2,0); break;
		case 1: TcpOpt(2,1); break;
		case 2: TcpOpt(2,0); break;
		case 3: TcpOpt(2,3); break;
		case 5: FtpOpt(2,0); break;
		case 6: FtpOpt(2,1); break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnCmd1Click(TObject *Sender)
{
	for (int i=0;i<3;i++) {
		if (Stream1->ItemIndex==0) {
			CmdOptDialog->Cmds  [i]=Cmds  [0][i];
			CmdOptDialog->CmdEna[i]=CmdEna[0][i];
		}
		else {
			CmdOptDialog->Cmds  [i]=CmdsTcp  [0][i];
			CmdOptDialog->CmdEna[i]=CmdEnaTcp[0][i];
		}
	}
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	for (int i=0;i<3;i++) {
		if (Stream1->ItemIndex==0) {
			Cmds  [0][i]=CmdOptDialog->Cmds  [i];
			CmdEna[0][i]=CmdOptDialog->CmdEna[i];
		}
		else {
			CmdsTcp  [0][i]=CmdOptDialog->Cmds  [i];
			CmdEnaTcp[0][i]=CmdOptDialog->CmdEna[i];
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnCmd2Click(TObject *Sender)
{
	for (int i=0;i<3;i++) {
		if (Stream2->ItemIndex==0) {
			CmdOptDialog->Cmds  [i]=Cmds  [1][i];
			CmdOptDialog->CmdEna[i]=CmdEna[1][i];
		}
		else {
			CmdOptDialog->Cmds  [i]=CmdsTcp  [1][i];
			CmdOptDialog->CmdEna[i]=CmdEnaTcp[1][i];
		}
	}
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	for (int i=0;i<3;i++) {
		if (Stream2->ItemIndex==0) {
			Cmds  [1][i]=CmdOptDialog->Cmds  [i];
			CmdEna[1][i]=CmdOptDialog->CmdEna[i];
		}
		else {
			CmdsTcp  [1][i]=CmdOptDialog->Cmds  [i];
			CmdEnaTcp[1][i]=CmdOptDialog->CmdEna[i];
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnCmd3Click(TObject *Sender)
{
	for (int i=0;i<3;i++) {
		if (Stream3->ItemIndex==0) {
			CmdOptDialog->Cmds  [i]=Cmds  [2][i];
			CmdOptDialog->CmdEna[i]=CmdEna[2][i];
		}
		else {
			CmdOptDialog->Cmds  [i]=CmdsTcp  [2][i];
			CmdOptDialog->CmdEna[i]=CmdEnaTcp[2][i];
		}
	}
	if (CmdOptDialog->ShowModal()!=mrOk) return;
	for (int i=0;i<3;i++) {
		if (Stream3->ItemIndex==0) {
			Cmds  [2][i]=CmdOptDialog->Cmds  [i];
			CmdEna[2][i]=CmdOptDialog->CmdEna[i];
		}
		else {
			CmdsTcp  [2][i]=CmdOptDialog->Cmds  [i];
			CmdEnaTcp[2][i]=CmdOptDialog->CmdEna[i];
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnRcvOpt1Click(TObject *Sender)
{
	RcvOptDialog->Option=RcvOpt[0];
	if (RcvOptDialog->ShowModal()!=mrOk) return;
	RcvOpt[0]=RcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnRcvOpt2Click(TObject *Sender)
{
	RcvOptDialog->Option=RcvOpt[1];
	if (RcvOptDialog->ShowModal()!=mrOk) return;
	RcvOpt[1]=RcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnRcvOpt3Click(TObject *Sender)
{
	RcvOptDialog->Option=RcvOpt[2];
	if (RcvOptDialog->ShowModal()!=mrOk) return;
	RcvOpt[2]=RcvOptDialog->Option;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnPosClick(TObject *Sender)
{
	AnsiString s;
	RefDialog->RovPos[0]=str2dbl(NmeaPos1->Text);
	RefDialog->RovPos[1]=str2dbl(NmeaPos2->Text);
	RefDialog->RovPos[2]=str2dbl(NmeaPos3->Text);
	RefDialog->StaPosFile=MainForm->StaPosFileF;
	if (RefDialog->ShowModal()!=mrOk) return;
	NmeaPos1->Text=s.sprintf("%.9f",RefDialog->Pos[0]);
	NmeaPos2->Text=s.sprintf("%.9f",RefDialog->Pos[1]);
	NmeaPos3->Text=s.sprintf("%.3f",RefDialog->Pos[2]);
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::SerialOpt(int index, int opt)
{
	SerialOptDialog->Path=Paths[index][0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths[index][0]=SerialOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnFile1Click(TObject *Sender)
{
	//OpenDialog->FileName=FilePath1->Text;
	if (!OpenDialog->Execute()) return;
	FilePath1->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnFile2Click(TObject *Sender)
{
	//OpenDialog->FileName=FilePath2->Text;
	if (!OpenDialog->Execute()) return;
	FilePath2->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::BtnFile3Click(TObject *Sender)
{
	//OpenDialog->FileName=FilePath3->Text;
	if (!OpenDialog->Execute()) return;
	FilePath3->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::TcpOpt(int index, int opt)
{
	TcpOptDialog->Path=Paths[index][1];
	TcpOptDialog->Opt=opt;
	for (int i=0;i<10;i++) {
		TcpOptDialog->History[i]=History[i];
		TcpOptDialog->MntpHist[i]=MntpHist[i];
	}
	if (TcpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][1]=TcpOptDialog->Path;
	for (int i=0;i<10;i++) {
		History[i]=TcpOptDialog->History[i];
		MntpHist[i]=TcpOptDialog->MntpHist[i];
	}
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::FtpOpt(int index, int opt)
{
	FtpOptDialog->Path=Paths[index][3];
	FtpOptDialog->Opt=opt;
	if (FtpOptDialog->ShowModal()!=mrOk) return;
	Paths[index][3]=FtpOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TInputStrDialog::UpdateEnable(void)
{
	int ena1=(StreamC1->Checked&&Stream1->ItemIndex==4)||
             (StreamC2->Checked&&Stream2->ItemIndex==4)||
             (StreamC3->Checked&&Stream3->ItemIndex==4);
	int ena2=StreamC2->Checked&&Stream2->ItemIndex<=3;
	
	Stream1   ->Enabled=StreamC1->Checked;
	Stream2   ->Enabled=StreamC2->Checked;
	Stream3   ->Enabled=StreamC3->Checked;
	BtnStr1   ->Enabled=StreamC1->Checked&&Stream1->ItemIndex!=4;
	BtnStr2   ->Enabled=StreamC2->Checked&&Stream2->ItemIndex!=4;
	BtnStr3   ->Enabled=StreamC3->Checked&&Stream3->ItemIndex!=4;
	BtnCmd1   ->Enabled=StreamC1->Checked&&Stream1->ItemIndex!=4;
	BtnCmd2   ->Enabled=StreamC2->Checked&&Stream2->ItemIndex!=4;
	BtnCmd3   ->Enabled=StreamC3->Checked&&Stream3->ItemIndex!=4;
	Format1   ->Enabled=StreamC1->Checked;
	Format2   ->Enabled=StreamC2->Checked;
	Format3   ->Enabled=StreamC3->Checked;
	BtnRcvOpt1->Enabled=StreamC1->Checked;
	BtnRcvOpt2->Enabled=StreamC2->Checked;
	BtnRcvOpt3->Enabled=StreamC3->Checked;
	
	LabelNmea ->Enabled=ena2;
	NmeaReqL  ->Enabled=ena2;
	NmeaPos1  ->Enabled=ena2&&NmeaReqL->ItemIndex==1;
	NmeaPos2  ->Enabled=ena2&&NmeaReqL->ItemIndex==1;
	NmeaPos3  ->Enabled=ena2&&NmeaReqL->ItemIndex==1;
	BtnPos    ->Enabled=ena2&&NmeaReqL->ItemIndex==1;
	LabelResetCmd->Enabled=ena2&&NmeaReqL->ItemIndex==3;
	EditResetCmd->Enabled=ena2&&NmeaReqL->ItemIndex==3;
	LabelMaxBL->Enabled=ena2&&NmeaReqL->ItemIndex==3;
	EditMaxBL ->Enabled=ena2&&NmeaReqL->ItemIndex==3;
	LabelKm   ->Enabled=ena2&&NmeaReqL->ItemIndex==3;
	
	LabelF1   ->Enabled=ena1;
	FilePath1 ->Enabled=StreamC1->Checked&&Stream1->ItemIndex==4;
	FilePath2 ->Enabled=StreamC2->Checked&&Stream2->ItemIndex==4;
	FilePath3 ->Enabled=StreamC3->Checked&&Stream3->ItemIndex==4;
	BtnFile1  ->Enabled=StreamC1->Checked&&Stream1->ItemIndex==4;
	BtnFile2  ->Enabled=StreamC2->Checked&&Stream2->ItemIndex==4;
	BtnFile3  ->Enabled=StreamC3->Checked&&Stream3->ItemIndex==4;
	TimeTagC  ->Enabled=ena1;
	TimeStartE->Enabled=ena1&&TimeTagC->Checked;
	TimeSpeedL->Enabled=ena1&&TimeTagC->Checked;
	LabelF2   ->Enabled=ena1&&TimeTagC->Checked;
	LabelF3   ->Enabled=ena1&&TimeTagC->Checked;
	Chk64Bit  ->Enabled=ena1&&TimeTagC->Checked;
}
//---------------------------------------------------------------------------

