//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "freqdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFreqDialog *FreqDialog;
//---------------------------------------------------------------------------
__fastcall TFreqDialog::TFreqDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
