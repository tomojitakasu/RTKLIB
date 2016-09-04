//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "outstrdlg.h"
#include "keydlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOutputStrDialog *OutputStrDialog;
//---------------------------------------------------------------------------
__fastcall TOutputStrDialog::TOutputStrDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::FormShow(TObject *Sender)
{
	Stream1C ->Checked  =StreamC[0];
	Stream2C ->Checked  =StreamC[1];
	Stream1  ->ItemIndex=Stream[0];
	Stream2  ->ItemIndex=Stream[1];
	Format1  ->ItemIndex=Format[0];
	Format2  ->ItemIndex=Format[1];
	FilePath1->Text     =GetFilePath(Paths[0][2]);
	FilePath2->Text     =GetFilePath(Paths[1][2]);
	SwapIntv->Text      =SwapInterval;
	TimeTagC ->Checked  =OutTimeTag;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::BtnOkClick(TObject *Sender)
{
	StreamC[0]  =Stream1C->Checked;
	StreamC[1]  =Stream2C->Checked;
	Stream[0]   =Stream1->ItemIndex;
	Stream[1]   =Stream2->ItemIndex;
	Format[0]   =Format1->ItemIndex;
	Format[1]   =Format2->ItemIndex;
	Paths [0][2]=SetFilePath(FilePath1->Text);
	Paths [1][2]=SetFilePath(FilePath2->Text);
	SwapInterval=SwapIntv->Text;
	OutTimeTag  =TimeTagC->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::BtnFile1Click(TObject *Sender)
{
	SaveDialog->FileName=FilePath1->Text;
	if (!SaveDialog->Execute()) return;
	FilePath1->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::BtnFile2Click(TObject *Sender)
{
	SaveDialog->FileName=FilePath2->Text;
	if (!SaveDialog->Execute()) return;
	FilePath2->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::Stream1Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::Stream2Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::Stream1CClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::Stream2CClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::BtnKeyClick(TObject *Sender)
{
	KeyDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::BtnStr1Click(TObject *Sender)
{
	switch (Stream1->ItemIndex) {
		case 0: SerialOpt(0,0); break;
		case 1: TcpOpt(0,1); break;
		case 2: TcpOpt(0,0); break;
		case 3: TcpOpt(0,2); break;
		case 4: TcpOpt(0,4); break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::BtnStr2Click(TObject *Sender)
{
	switch (Stream2->ItemIndex) {
		case 0: SerialOpt(1,0); break;
		case 1: TcpOpt(1,1); break;
		case 2: TcpOpt(1,0); break;
		case 3: TcpOpt(1,2); break;
		case 4: TcpOpt(1,4); break;
	}
}
//---------------------------------------------------------------------------
AnsiString __fastcall TOutputStrDialog::GetFilePath(AnsiString path)
{
	char *p,*q,buff[1024];
	strcpy(buff,path.c_str());
	if ((p=strstr(buff,"::"))) *p='\0';
	return (path=buff);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TOutputStrDialog::SetFilePath(AnsiString path)
{
	AnsiString str;
	double swap;
	if (TimeTagC->Checked) path+="::T";
	str=SwapIntv->Text;
	if (sscanf(str.c_str(),"%lf",&swap)>=1) {
		path+="::S="+str;
	}
	return path;
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::SerialOpt(int index, int opt)
{
	SerialOptDialog->Path=Paths[index][0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths[index][0]=SerialOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TOutputStrDialog::TcpOpt(int index, int opt)
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
void __fastcall TOutputStrDialog::UpdateEnable(void)
{
	int ena=(Stream1C->Checked&&Stream1->ItemIndex==5)||
			(Stream2C->Checked&&Stream2->ItemIndex==5);
	Stream1  ->Enabled=Stream1C->Checked;
	Stream2  ->Enabled=Stream2C->Checked;
	BtnStr1  ->Enabled=Stream1C->Checked&&Stream1->ItemIndex<=4;
	BtnStr2  ->Enabled=Stream2C->Checked&&Stream2->ItemIndex<=4;
	FilePath1->Enabled=Stream1C->Checked&&Stream1->ItemIndex==5;
	FilePath2->Enabled=Stream2C->Checked&&Stream2->ItemIndex==5;
	BtnFile1 ->Enabled=Stream1C->Checked&&Stream1->ItemIndex==5;
	BtnFile2 ->Enabled=Stream2C->Checked&&Stream2->ItemIndex==5;
	LabelF1  ->Enabled=ena;
	Label1   ->Enabled=ena;
	Label2   ->Enabled=ena;
	TimeTagC ->Enabled=ena;
	SwapIntv ->Enabled=ena;
	BtnKey   ->Enabled=ena;
}
//---------------------------------------------------------------------------

