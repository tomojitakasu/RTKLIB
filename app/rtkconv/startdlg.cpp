//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "startdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TStartDialog *StartDialog;
//---------------------------------------------------------------------------
__fastcall TStartDialog::TStartDialog(TComponent* Owner)
	: TForm(Owner)
{
	Time.time=0;
	Time.sec=0.0;
}
//---------------------------------------------------------------------------
void __fastcall TStartDialog::FormShow(TObject *Sender)
{
	char tstr[64];
	if (Time.time==0) {
		Time=utc2gpst(timeget());
	}
	time2str(Time,tstr,0);
	tstr[10]='\0';
	TimeY1->Text=tstr;
	TimeH1->Text=tstr+11;
}
//---------------------------------------------------------------------------
void __fastcall TStartDialog::BtnOkClick(TObject *Sender)
{
	AnsiString TimeY1_Text=TimeY1->Text,TimeH1_Text=TimeH1->Text;
	double ep[]={2000,1,1,0,0,0};
	sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
	sscanf(TimeH1_Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
	Time=epoch2time(ep);
}
//---------------------------------------------------------------------------
void __fastcall TStartDialog::TimeY1UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeY1_Text=TimeY1->Text,s;
	double ep[]={2000,1,1,0,0,0};
	int p=TimeY1->SelStart,ud=Direction==updUp?1:-1;
	sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
	if (4<p&&p<8) {
	    ep[1]+=ud;
	    if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
	    else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
	}
	else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
	time2epoch(epoch2time(ep),ep);
	TimeY1->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
	TimeY1->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TStartDialog::TimeH1UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeH1_Text=TimeH1->Text,s;
	int hms[3]={0},sec,p=TimeH1->SelStart,ud=Direction==updUp?1:-1;
	sscanf(TimeH1_Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
	if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
	sec=hms[0]*3600+hms[1]*60+hms[2];
	if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
	TimeH1->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
	TimeH1->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TStartDialog::BtnFileTimeClick(TObject *Sender)
{
	FILETIME tc,ta,tw;
	SYSTEMTIME st;
	AnsiString s;
	HANDLE h;
	
	if ((h=CreateFile(FileName,GENERIC_READ,0,NULL,OPEN_EXISTING,
					  FILE_ATTRIBUTE_NORMAL,0))==INVALID_HANDLE_VALUE) {
		return;
	}
	GetFileTime(h,&tc,&ta,&tw);
	CloseHandle(h);
	FileTimeToSystemTime(&tc,&st); // file create time
	TimeY1->Text=s.sprintf("%04d/%02d/%02d",st.wYear,st.wMonth,st.wDay);
	TimeH1->Text=s.sprintf("%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
}
//---------------------------------------------------------------------------

