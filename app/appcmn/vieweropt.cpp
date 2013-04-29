//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "viewer.h"
#include "vieweropt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TViewerOptDialog *ViewerOptDialog;
//---------------------------------------------------------------------------
__fastcall TViewerOptDialog::TViewerOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TViewerOptDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	FontLabel->Font->Assign(TTextViewer::FontD);
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
	Color1->Color=TTextViewer::Color1;
	Color2->Color=TTextViewer::Color2;
}
//---------------------------------------------------------------------------
void __fastcall TViewerOptDialog::BtnOkClick(TObject *Sender)
{
	TTextViewer::FontD->Assign(FontLabel->Font);
	TTextViewer::Color1=Color1->Color;
	TTextViewer::Color2=Color2->Color;
}
//---------------------------------------------------------------------------
void __fastcall TViewerOptDialog::BtnColor1Click(TObject *Sender)
{
	ColorDialog->Color=Color1->Color;
	if (!ColorDialog->Execute()) return;
	Color1->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TViewerOptDialog::BtnColor2Click(TObject *Sender)
{
	ColorDialog->Color=Color2->Color;
	if (!ColorDialog->Execute()) return;
	Color2->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TViewerOptDialog::BtnFontClick(TObject *Sender)
{
	AnsiString s;
	FontDialog->Font=FontLabel->Font;
	if (!FontDialog->Execute()) return;
	FontLabel->Font=FontDialog->Font;
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
}
//---------------------------------------------------------------------------
