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
	TextStr=NULL;
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
	wchar_t *p,*str=FindStr->Text.c_str();
	
	if (!TextStr) return;
	
	if (Text->SelLength>0) p=TextStr+Text->SelStart+1;
	else p=TextStr+Text->SelStart;
	
	if ((p=wcsstr(p,str))) {
		Text->SelStart=(int)(p-TextStr);
		Text->SelLength=wcslen(str);
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
	char s[256],*path[]={s};
	
	if (expath(file.c_str(),path,1)<1) return;
	AnsiString str(path[0]);
	Screen->Cursor=crHourGlass;
	try {
		Text->Lines->LoadFromFile(str);
		
		// read text for search
		ReadText(str);
	}
	catch (...) {
		Screen->Cursor=crDefault;
		return;
	}
	Screen->Cursor=crDefault;
	Caption=str;
	File=file;
}
//---------------------------------------------------------------------------
void __fastcall TTextViewer::ReadText(AnsiString file)
{
	FILE *fp;
	int i,len,n=0,nmax=0;
	char buff[1024];
	wchar_t wbuff[1024];
	
	free(TextStr); TextStr=NULL;
	
	if (!(fp=fopen(file.c_str(),"r"))) return;
		
	for (i=0;fgets(buff,sizeof(buff),fp)&&i<MAXLINE;i++) {
		::MultiByteToWideChar(CP_UTF8,0,buff,-1,wbuff,1024);
		len=wcslen(wbuff);
		if (n+len+1>=nmax) {
			nmax=nmax<=0?16384:nmax*2;
			if (!(TextStr=(wchar_t *)realloc(TextStr,sizeof(wchar_t)*nmax))) break;
		}
		wcscpy(TextStr+n,wbuff);
		n+=len;
	}
	fclose(fp);
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

