//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "serioptdlg.h"
#include "cmdoptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "logstrdlg.h"
#include "keydlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLogStrDialog *LogStrDialog;
//---------------------------------------------------------------------------
__fastcall TLogStrDialog::TLogStrDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::FormShow(TObject *Sender)
{
	Stream1C ->Checked  =StreamC[0];
	Stream2C ->Checked  =StreamC[1];
	Stream3C ->Checked  =StreamC[2];
	Stream1  ->ItemIndex=Stream[0];
	Stream2  ->ItemIndex=Stream[1];
	Stream3  ->ItemIndex=Stream[2];
	FilePath1->Text     =GetFilePath(Paths[0][2]);
	FilePath2->Text     =GetFilePath(Paths[1][2]);
	FilePath3->Text     =GetFilePath(Paths[2][2]);
	SwapIntv ->Text     =SwapInterval;
	TimeTagC ->Checked  =LogTimeTag;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::BtnOkClick(TObject *Sender)
{
	StreamC [0]=Stream1C->Checked;
	StreamC [1]=Stream2C->Checked;
	StreamC [2]=Stream3C->Checked;
	Stream  [0]=Stream1 ->ItemIndex;
	Stream  [1]=Stream2 ->ItemIndex;
	Stream  [2]=Stream3 ->ItemIndex;
	Paths[0][2]=SetFilePath(FilePath1->Text);
	Paths[1][2]=SetFilePath(FilePath2->Text);
	Paths[2][2]=SetFilePath(FilePath3->Text);
	SwapInterval=SwapIntv->Text;
	LogTimeTag =TimeTagC->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::BtnFile1Click(TObject *Sender)
{
	SaveDialog->FileName=FilePath1->Text;
	if (!SaveDialog->Execute()) return;
	FilePath1->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::BtnFile2Click(TObject *Sender)
{
	SaveDialog->FileName=FilePath2->Text;
	if (!SaveDialog->Execute()) return;
	FilePath2->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::BtnFile3Click(TObject *Sender)
{
	SaveDialog->FileName=FilePath3->Text;
	if (!SaveDialog->Execute()) return;
	FilePath3->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::Stream1Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::Stream2Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::Stream3CClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::Stream1CClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::Stream2CClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::Stream3Change(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::BtnKeyClick(TObject *Sender)
{
	KeyDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::BtnStr1Click(TObject *Sender)
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
void __fastcall TLogStrDialog::BtnStr2Click(TObject *Sender)
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
void __fastcall TLogStrDialog::BtnStr3Click(TObject *Sender)
{
	switch (Stream3->ItemIndex) {
		case 0: SerialOpt(2,0); break;
		case 1: TcpOpt(2,1); break;
		case 2: TcpOpt(2,0); break;
		case 3: TcpOpt(2,2); break;
		case 4: TcpOpt(2,4); break;
	}
}
//---------------------------------------------------------------------------
AnsiString __fastcall TLogStrDialog::GetFilePath(AnsiString path)
{
	char *p,*q,buff[1024];
	strcpy(buff,path.c_str());
	if ((p=strstr(buff,"::"))) *p='\0';
	return (path=buff);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TLogStrDialog::SetFilePath(AnsiString path)
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
void __fastcall TLogStrDialog::SerialOpt(int index, int opt)
{
	SerialOptDialog->Path=Paths[index][0];
	SerialOptDialog->Opt=opt;
	if (SerialOptDialog->ShowModal()!=mrOk) return;
	Paths[index][0]=SerialOptDialog->Path;
}
//---------------------------------------------------------------------------
void __fastcall TLogStrDialog::TcpOpt(int index, int opt)
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
void __fastcall TLogStrDialog::UpdateEnable(void)
{
	int ena=(Stream1C->Checked&&Stream1->ItemIndex==5)||
			(Stream2C->Checked&&Stream2->ItemIndex==5)||
			(Stream3C->Checked&&Stream3->ItemIndex==5);
	Stream1  ->Enabled=Stream1C->Checked;
	Stream2  ->Enabled=Stream2C->Checked;
	Stream3  ->Enabled=Stream3C->Checked;
	BtnStr1  ->Enabled=Stream1C->Checked&&Stream1->ItemIndex<=4;
	BtnStr2  ->Enabled=Stream2C->Checked&&Stream2->ItemIndex<=4;
	BtnStr3  ->Enabled=Stream3C->Checked&&Stream3->ItemIndex<=4;
	FilePath1->Enabled=Stream1C->Checked&&Stream1->ItemIndex==5;
	FilePath2->Enabled=Stream2C->Checked&&Stream2->ItemIndex==5;
	FilePath3->Enabled=Stream3C->Checked&&Stream3->ItemIndex==5;
	BtnFile1 ->Enabled=Stream1C->Checked&&Stream1->ItemIndex==5;
	BtnFile2 ->Enabled=Stream2C->Checked&&Stream2->ItemIndex==5;
	BtnFile3 ->Enabled=Stream3C->Checked&&Stream3->ItemIndex==5;
	Label1   ->Enabled=ena;
	Label2   ->Enabled=ena;
	LabelF1  ->Enabled=ena;
	TimeTagC ->Enabled=ena;
	SwapIntv ->Enabled=ena;
	BtnKey   ->Enabled=ena;
	OutEventC->Enabled=FilePath1->Enabled;
}
//---------------------------------------------------------------------------

