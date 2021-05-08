//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rcvoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TRcvOptDialog *RcvOptDialog;
//---------------------------------------------------------------------------
__fastcall TRcvOptDialog::TRcvOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TRcvOptDialog::FormShow(TObject *Sender)
{
	OptionE->Text=Option;
}
//---------------------------------------------------------------------------
void __fastcall TRcvOptDialog::BtnOkClick(TObject *Sender)
{
	Option=OptionE->Text;
}
//---------------------------------------------------------------------------
