//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "launchmain.h"
#include "launchoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLaunchOptDialog *LaunchOptDialog;
//---------------------------------------------------------------------------
__fastcall TLaunchOptDialog::TLaunchOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TLaunchOptDialog::FormShow(TObject *Sender)
{
	if (MainForm->Option==1) {
		OptMkl->Checked=true;
	}
	else if (MainForm->Option==2) {
		OptWin64->Checked=true;
	}
	else {
		OptNormal->Checked=true;
	}
	Minimize->Checked=MainForm->Minimize;
}
//---------------------------------------------------------------------------
void __fastcall TLaunchOptDialog::BtnOkClick(TObject *Sender)
{
	if (OptMkl->Checked) {
		MainForm->Option=1;
	}
	else if (OptWin64->Checked) {
		MainForm->Option=2;
	}
	else {
		MainForm->Option=0;
	}
	MainForm->Minimize=Minimize->Checked;
}
//---------------------------------------------------------------------------
