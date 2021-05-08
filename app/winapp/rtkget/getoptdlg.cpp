//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "getoptdlg.h"
#include "getmain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TDownOptDialog *DownOptDialog;
//---------------------------------------------------------------------------
__fastcall TDownOptDialog::TDownOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TDownOptDialog::BtnUrlFileClick(TObject *Sender)
{
    OpenDialog->Title="GNSS Data URL File";
    OpenDialog->FileName="";
    if (!OpenDialog->Execute()) return;
    UrlFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TDownOptDialog::BtnLogFileClick(TObject *Sender)
{
    SaveDialog->Title="Download Log File";
    SaveDialog->FileName="";
    if (!SaveDialog->Execute()) return;
    LogFile->Text=SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TDownOptDialog::FormShow(TObject *Sender)
{
	HoldErr  ->Checked=MainForm->HoldErr;
	HoldList ->Checked=MainForm->HoldList;
	NCol     ->Text   =UnicodeString(MainForm->NCol);
	Proxy    ->Text   =MainForm->ProxyAddr;
	UrlFile  ->Text   =MainForm->UrlFile;
	LogFile  ->Text   =MainForm->LogFile;
	LogAppend->Checked=MainForm->LogAppend;
	DateFormat->ItemIndex=MainForm->DateFormat;
	TraceLevel->ItemIndex=MainForm->TraceLevel;
}
//---------------------------------------------------------------------------
void __fastcall TDownOptDialog::BtnOkClick(TObject *Sender)
{
	MainForm->HoldErr  =HoldErr  ->Checked;
	MainForm->HoldList =HoldList ->Checked;
	MainForm->NCol     =NCol     ->Text.ToInt();
	MainForm->ProxyAddr=Proxy    ->Text;
	MainForm->UrlFile  =UrlFile  ->Text;
	MainForm->LogFile  =LogFile  ->Text;
	MainForm->LogAppend=LogAppend->Checked;
	MainForm->DateFormat=DateFormat->ItemIndex;
	MainForm->TraceLevel=TraceLevel->ItemIndex;
}
//---------------------------------------------------------------------------

