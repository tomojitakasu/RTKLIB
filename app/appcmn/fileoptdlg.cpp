//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "keydlg.h"
#include "fileoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFileOptDialog *FileOptDialog;
//---------------------------------------------------------------------------
__fastcall TFileOptDialog::TFileOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	Opt=0;
}
//---------------------------------------------------------------------------
void __fastcall TFileOptDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	double speed=1.0,start=0.0,intv=0.0;
	char buff[1024];
	char *p;
	int size_fpos=4;
	strcpy(buff,Path.c_str());
	
	ChkTimeTag->Caption=Opt?"TimeTag":"Time";
	TimeSpeed->Visible=!Opt;
	TimeStart->Visible=!Opt;
	Chk64Bit ->Visible=!Opt;
	Label1   ->Caption=Opt?"Output File Path":"Input File Path";
	Label2   ->Visible=!Opt;
	Label3   ->Visible=!Opt;
	Label4   ->Visible=Opt;
	Label5   ->Visible=Opt;
	SwapIntv ->Visible=Opt;
	BtnKey   ->Visible=Opt;
	ChkTimeTag->Checked=false;
	SwapIntv ->Text="";
	if (!Opt) { // input
		for (p=buff;p=strstr(p,"::");p+=2) {
			if      (*(p+2)=='T') ChkTimeTag->Checked=true;
			else if (*(p+2)=='+') sscanf(p+2,"+%lf",&start);
			else if (*(p+2)=='x') sscanf(p+2,"x%lf",&speed);
			else if (*(p+2)=='P') sscanf(p+2,"P=%d",&size_fpos);
		}
		if (start<=0.0) start=0.0;
		if (speed<=0.0) speed=1.0;
		TimeSpeed->Text=s.sprintf("x%g",speed);
		TimeStart->Text=s.sprintf("%g", start);
		Chk64Bit->Checked=size_fpos==8;
		if ((p=strstr(buff,"::"))) *p='\0';
		FilePath->Text=buff;
	}
	else { // output
		for (p=buff;p=strstr(p,"::");p+=2) {
			if      (*(p+2)=='T') ChkTimeTag->Checked=true;
			else if (*(p+2)=='S') sscanf(p+2,"S=%lf",&intv);
		}
		if (intv>0.0) SwapIntv->Text=s.sprintf("%.3g",intv);
		if ((p=strstr(buff,"::"))) *p='\0';
		FilePath->Text=buff;
	}
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TFileOptDialog::BtnOkClick(TObject *Sender)
{
	AnsiString str;
	double swap;
	
	if (!Opt) { // input
		Path=FilePath->Text;
		if (ChkTimeTag->Checked) {
			Path=Path+"::T"+"::"+TimeSpeed->Text+"::+"+TimeStart->Text;
		}
		if (Chk64Bit->Checked) {
			Path=Path+"::P=8";
		}
	}
	else { // output
		Path=FilePath->Text;
		if (ChkTimeTag->Checked) Path+="::T";
		str=SwapIntv->Text;
		if (sscanf(str.c_str(),"%lf",&swap)>=1) {
			Path+="::S="+str;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileOptDialog::BtnFilePathClick(TObject *Sender)
{
	if (!Opt) {
		OpenDialog->FileName=FilePath->Text;
		if (!OpenDialog->Execute()) return;
		FilePath->Text=OpenDialog->FileName;
	}
	else {
		SaveDialog->FileName=FilePath->Text;
		if (!SaveDialog->Execute()) return;
		FilePath->Text=SaveDialog->FileName;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileOptDialog::ChkTimeTagClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TFileOptDialog::BtnKeyClick(TObject *Sender)
{
	KeyDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TFileOptDialog::UpdateEnable(void)
{
	TimeSpeed->Enabled=ChkTimeTag->Checked;
	TimeStart->Enabled=ChkTimeTag->Checked;
	Chk64Bit ->Enabled=ChkTimeTag->Checked;
	Label2   ->Enabled=ChkTimeTag->Checked;
	Label3   ->Enabled=ChkTimeTag->Checked;
}
//---------------------------------------------------------------------------

