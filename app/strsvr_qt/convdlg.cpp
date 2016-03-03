//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "convdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConvDialog *ConvDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TConvDialog::TConvDialog(TComponent* Owner)
	: TForm(Owner)
{
	int i;
	for (i=0;i<=MAXRCVFMT;i++) {
		InFormat->Items->Add(formatstrs[i]);
	}
	InFormat->ItemIndex=0;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	Conversion->Checked=ConvEna;
	InFormat ->ItemIndex=ConvInp;
	OutFormat->ItemIndex=ConvOut;
	OutMsgs->Text=ConvMsg;
	Options->Text=ConvOpt;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnOkClick(TObject *Sender)
{
	ConvEna=Conversion->Checked;
	ConvInp=InFormat->ItemIndex;
	ConvOut=OutFormat->ItemIndex;
	ConvMsg=OutMsgs->Text;
	ConvOpt=Options->Text;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::ConversionClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::UpdateEnable(void)
{
	InFormat ->Enabled=Conversion->Checked;
	OutFormat->Enabled=Conversion->Checked;
	OutMsgs  ->Enabled=Conversion->Checked;
	Options  ->Enabled=Conversion->Checked;
}
//---------------------------------------------------------------------------

