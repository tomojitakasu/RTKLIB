//---------------------------------------------------------------------------
#include <vcl.h>
#include <ctype.h>
#include <stdio.h>
#pragma hdrstop

#include "mondlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define MAXLEN		200
#define MAXLINE		2048
#define TOPMARGIN	2
#define LEFTMARGIN	3

TStrMonDialog *StrMonDialog;

//---------------------------------------------------------------------------
__fastcall TStrMonDialog::TStrMonDialog(TComponent* Owner)
	: TForm(Owner)
{
	ConBuff=new TStringList;
	ConBuff->Add("");
	DoubleBuffered=true;
	Stop=0;
	ScrollPos=0;
	StrFmt=0;
	for (int i=0;i<=MAXRCVFMT;i++) {
		SelFmt->Items->Add(formatstrs[i]);
	}
	rtcm.outtype=raw.outtype=1;
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::FormResize(TObject *Sender)
{
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::SelFmtChange(TObject *Sender)
{
	if (StrFmt-3==STRFMT_RTCM2||StrFmt-3==STRFMT_RTCM3) {
		free_rtcm(&rtcm);
	}
	else if (StrFmt>=3) {
		free_raw(&raw);
	}
	StrFmt=SelFmt->ItemIndex;
	ConBuff->Clear();
	ConBuff->Add("");
	
	if (StrFmt-3==STRFMT_RTCM2||StrFmt-3==STRFMT_RTCM3) {
		init_rtcm(&rtcm);
		rtcm.outtype=1;
	}
	else if (StrFmt>=3) {
		init_raw(&raw,StrFmt-2);
		raw.outtype=1;
	}
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::AddMsg(unsigned char *msg, int len)
{
	char buff[256];
	int i,n;
	
	if (len<=0) return;
	
	else if (StrFmt-3==STRFMT_RTCM2) {
		for (i=0;i<len;i++) {
			input_rtcm2(&rtcm,msg[i]);
			if (rtcm.msgtype[0]) {
				n=sprintf(buff,"%s\n",rtcm.msgtype);
				AddConsole((unsigned char *)buff,n,1);
				rtcm.msgtype[0]='\0';
			}
	    }
	}
	else if (StrFmt-3==STRFMT_RTCM3) {
		for (i=0;i<len;i++) {
			input_rtcm3(&rtcm,msg[i]);
			if (rtcm.msgtype[0]) {
				n=sprintf(buff,"%s\n",rtcm.msgtype);
				AddConsole((unsigned char *)buff,n,1);
				rtcm.msgtype[0]='\0';
			}
	    }
	}
	else if (StrFmt>=3) { // raw
		for (i=0;i<len;i++) {
			input_raw(&raw,StrFmt-3,msg[i]);
			if (raw.msgtype[0]) {
				n=sprintf(buff,"%s\n",raw.msgtype);
				AddConsole((unsigned char *)buff,n,1);
				raw.msgtype[0]='\0';
			}
	    }
	}
	else if (StrFmt>=1) { // HEX/ASC
		AddConsole(msg,len,StrFmt-1);
	}
	else { // Streams
		ConBuff->Clear();
		ConBuff->Add("");
		AddConsole(msg,len,1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::AddConsole(unsigned char *msg, int n, int mode)
{
	char buff[MAXLEN+16],*p=buff,c;
	
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
void __fastcall TStrMonDialog::ConsolePaint(TObject *Sender)
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
		//c->Font->Color=i<n-1?clGray:clBlack;
		c->Font->Color=clBlack;
		c->TextOut(LEFTMARGIN,y,ConBuff->Strings[i]);
	}
	Scroll->Max=n<=m?m-1:n-m;
	Scroll->Position=Scroll->Max-ScrollPos;
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::ScrollChange(TObject *Sender)
{
	ScrollPos=Scroll->Max-Scroll->Position;
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::BtnClearClick(TObject *Sender)
{
	ConBuff->Clear();
	ConBuff->Add("");
	Console->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::BtnStopClick(TObject *Sender)
{
	Stop=!Stop;
	BtnStop->Down=Stop;
}
//---------------------------------------------------------------------------
void __fastcall TStrMonDialog::BtnDownClick(TObject *Sender)
{
	ScrollPos=0;
	Console->Invalidate();
}
//---------------------------------------------------------------------------

