//---------------------------------------------------------------------------
#include <vcl.h>
#include <ctype.h>
#include <stdio.h>
#pragma hdrstop

#include "console.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define MAXLEN		200
#define MAXLINE		2048
#define TOPMARGIN	2
#define LEFTMARGIN	3

TConsole *Console;
//---------------------------------------------------------------------------
__fastcall TConsole::TConsole(TComponent* Owner)
	: TForm(Owner)
{
	ConBuff=new TStringList;
	ConBuff->Add("");
	DoubleBuffered=true;
	Stop=0;
	ScrollPos=0;
}
//---------------------------------------------------------------------------
void __fastcall TConsole::ConsolePaint(TObject *Sender)
{
	TCanvas *c=Console->Canvas;
	TSize off=c->TextExtent(" ");
	int n,m,p,y=TOPMARGIN;
	
	c->Brush->Style=bsSolid;
	c->Brush->Color=clWhite;
	c->FillRect(Console->ClientRect);
	
	n=ConBuff->Count; if (ConBuff->Strings[n-1]=="") n--;
	m=(Console->Height-TOPMARGIN*2)/off.cy;
	p=m>=n?0:n-m-ScrollPos;
	
	for (int i=p<0?0:p;i<ConBuff->Count;i++,y+=off.cy) {
		if (y+off.cy>Console->Height-TOPMARGIN) break;
		c->Font->Color=i<n-1?clGray:clBlack;
		c->TextOut(LEFTMARGIN,y,ConBuff->Strings[i]);
	}
	Scroll->Max=n<=m?m-1:n-m;
	Scroll->Position=Scroll->Max-ScrollPos;
}
//---------------------------------------------------------------------------
void __fastcall TConsole::ScrollChange(TObject *Sender)
{
	ScrollPos=Scroll->Max-Scroll->Position;
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConsole::FormResize(TObject *Sender)
{
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConsole::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TConsole::BtnAscClick(TObject *Sender)
{
	if (ConBuff->Strings[ConBuff->Count-1]!="") ConBuff->Add("");
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConsole::BtnHexClick(TObject *Sender)
{
	if (ConBuff->Strings[ConBuff->Count-1]!="") ConBuff->Add("");
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConsole::BtnClearClick(TObject *Sender)
{
	ConBuff->Clear();
	ConBuff->Add("");
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConsole::BtnStopClick(TObject *Sender)
{
	Stop=!Stop;
	BtnStop->Down=Stop;
}
//---------------------------------------------------------------------------
void __fastcall TConsole::BtnDownClick(TObject *Sender)
{
	ScrollPos=0;
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConsole::AddMsg(unsigned char *msg, int n)
{
	char buff[MAXLEN+16],*p=buff,c;
	int mode=BtnAsc->Down;
	
	if (n<=0||Stop) return;
	
	p+=sprintf(p,"%s",ConBuff->Strings[ConBuff->Count-1].c_str());
	
	for (int i=0;i<n;i++) {
		if (mode) {
			if (msg[i]=='\r') continue;
			p+=sprintf(p,"%c",msg[i]=='\n'||isprint(msg[i])?msg[i]:'.');
		}
		else {
			p+=sprintf(p,"%s%02X",(p-buff)%17==16?" ":"",msg[i]);
			if (p-buff>=67) p+=sprintf(p,"\n");
		}
		if (p-buff>=MAXLEN) p+=sprintf(p,"\n");
		
		if (*(p-1)=='\n') {
			ConBuff->Strings[ConBuff->Count-1]=buff;
			ConBuff->Add("");
			*(p=buff)=0;
			if (ConBuff->Count>=MAXLINE) ConBuff->Delete(0);
		}
	}
	ConBuff->Strings[ConBuff->Count-1]=buff;
	Console->Invalidate();
}
//---------------------------------------------------------------------------

