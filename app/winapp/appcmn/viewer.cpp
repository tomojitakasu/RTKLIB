//---------------------------------------------------------------------------
#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "viewer.h"
#include "vieweropt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTextViewer *TextViewer;
TColor TTextViewer::Color1,TTextViewer::Color2;
TFont *TTextViewer::FontD;
//---------------------------------------------------------------------------
__fastcall TTextViewer::TTextViewer(TComponent* Owner)
	: TForm(Owner)
{
	Option=1;
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::FormShow(TObject *Sender)
{
	if (Option==0) {
		BtnReload->Visible=false;
		BtnRead  ->Visible=false;
	}
	else if (Option==2) {
		BtnReload->Visible=false;
		BtnRead  ->Caption="Save...";
	}
	UpdateText();
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnReloadClick(TObject *Sender)
{
	Read(File);
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnReadClick(TObject *Sender)
{
	if (BtnRead->Caption=="Save...") {
		SaveDialog->FileName=File;
		if (!SaveDialog->Execute()) return;
		Save(SaveDialog->FileName);
	}
	else {
		OpenDialog->FileName=File;
		if (!OpenDialog->Execute()) return;
		Read(OpenDialog->FileName);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnOptClick(TObject *Sender)
{
	ViewerOptDialog->Left=Left+Width/2-ViewerOptDialog->Width/2;
	ViewerOptDialog->Top=Top+Height/2-ViewerOptDialog->Height/2;
	if (ViewerOptDialog->ShowModal()!=mrOk) return;
	UpdateText();
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnCloseClick(TObject *Sender)
{
	Release();
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::BtnFindClick(TObject *Sender)
{
    TSearchTypes opt;
	UnicodeString str=FindStr->Text;
	int p;
	
	if (Text->SelLength>0) {
		p=Text->SelStart+1;
	}
	else {
		p=0;
	}
    p=Text->FindText(str,p,Text->Text.Length()-p,opt);
	
	if (p>=0) {
		Text->SelStart=p;
		Text->SelLength=str.Length();
	}
	else {
		Text->SelStart=0;
		Text->SelLength=0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::FindStrKeyPress(TObject *Sender, char &Key)
{
	if (Key=='\r') BtnFindClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::Read(AnsiString file)
{
	char s[1024],*path[]={s};
	
	if (expath(file.c_str(),path,1)<1) return;
	
	Screen->Cursor=crHourGlass;
	try {
	    file=path[0];
		Text->Lines->LoadFromFile(file);
	}
	catch (...) {
		Screen->Cursor=crDefault;
		return;
	}
	Screen->Cursor=crDefault;
	Caption=file;
	File=file;
	Text->SelStart=0;
	Text->SelLength=0;
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::Save(AnsiString file)
{
	Screen->Cursor=crHourGlass;
	try {
		Text->Lines->SaveToFile(file);
	}
	catch (...) {
		Screen->Cursor=crDefault;
		return;
	}
	Screen->Cursor=crDefault;
	File=file;
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::UpdateText(void)
{
	Text->Font=FontD;
	Text->Font->Color=Color1;
	Text->Color=Color2;
}
//---------------------------------------------------------------------------

