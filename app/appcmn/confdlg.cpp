//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "confdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfDialog *ConfDialog;
//---------------------------------------------------------------------------
__fastcall TConfDialog::TConfDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
