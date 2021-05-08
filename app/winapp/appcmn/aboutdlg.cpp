//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "aboutdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutDialog *AboutDialog;
//---------------------------------------------------------------------------
__fastcall TAboutDialog::TAboutDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TAboutDialog::FormShow(TObject *Sender)
{
	TImage *icon[]={Icon1,Icon2,Icon3,Icon4,Icon5,Icon6,Icon7,Icon8};
	AnsiString s;
	if (IconIndex>0) icon[IconIndex-1]->Visible=true;
	LabelAbout->Caption=About;
	LabelVer->Caption=s.sprintf("with RTKLIB ver.%s %s",VER_RTKLIB,PATCH_LEVEL);
	LabelCopyright->Caption=COPYRIGHT_RTKLIB;
}
//---------------------------------------------------------------------------
void __fastcall TAboutDialog::BtnOkClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

