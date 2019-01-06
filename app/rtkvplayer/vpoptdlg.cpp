//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "vplayermain.h"
#include "vpoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TVideoPlayerOptDialog *VideoPlayerOptDialog;
//---------------------------------------------------------------------------
__fastcall TVideoPlayerOptDialog::TVideoPlayerOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TVideoPlayerOptDialog::FormShow(TObject *Sender)
{
	UnicodeString s;
	
	EditMjpgRate->Text = s.sprintf(L"%.0f", MainForm->MjpgRate);
	EditSyncAddr->Text = MainForm->SyncAddr;
	EditSyncPort->Text = s.sprintf(L"%d", MainForm->SyncPort);
}
//---------------------------------------------------------------------------
void __fastcall TVideoPlayerOptDialog::BtnOkClick(TObject *Sender)
{
	MainForm->MjpgRate = EditMjpgRate->Text.ToDouble();
	MainForm->SyncAddr = EditSyncAddr->Text;
	MainForm->SyncPort = EditSyncPort->Text.ToInt();
}
//---------------------------------------------------------------------------
