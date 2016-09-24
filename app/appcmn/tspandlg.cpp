//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "plotmain.h"
#include "timedlg.h"
#include "tspandlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSpanDialog *SpanDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TSpanDialog::TSpanDialog(TComponent* Owner)
	: TForm(Owner)
{
	for (int i=0;i<3;i++) {
		TimeEna[i]=true;
		TimeVal[i]=true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::FormShow(TObject *Sender)
{
	char ts[64],te[64];
	AnsiString s;
	TimeStartF->Checked=TimeEna[0];
	TimeEndF  ->Checked=TimeEna[1];
	TimeIntF  ->Checked=TimeEna[2];
	time2str(TimeStart,ts,0); ts[10]='\0';
	time2str(TimeEnd,  te,0); te[10]='\0';
	TimeY1->Text=ts;
	TimeH1->Text=ts+11;
	TimeY2->Text=te;
	TimeH2->Text=te+11;
	EditTimeInt->Text=s.sprintf("%g",TimeInt);
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::BtnOkClick(TObject *Sender)
{
	AnsiString TimeY1_Text=TimeY1->Text,TimeH1_Text=TimeH1->Text;
	AnsiString TimeY2_Text=TimeY2->Text,TimeH2_Text=TimeH2->Text;
	AnsiString EditTimeInt_Text=EditTimeInt->Text;
	double eps[]={2000,1,1,0,0,0},epe[]={2000,1,1,0,0,0};
	
	TimeEna[0]=TimeStartF->Checked;
	TimeEna[1]=TimeEndF  ->Checked;
	TimeEna[2]=TimeIntF  ->Checked;
	sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",eps,eps+1,eps+2);
	sscanf(TimeH1_Text.c_str(),"%lf:%lf:%lf",eps+3,eps+4,eps+5);
	sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",epe,epe+1,epe+2);
	sscanf(TimeH2_Text.c_str(),"%lf:%lf:%lf",epe+3,epe+4,epe+5);
	TimeStart=epoch2time(eps);
	TimeEnd=epoch2time(epe);
	TimeInt=str2dbl(EditTimeInt_Text);
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeStartFClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeEndFClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeIntFClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeY1UDChangingEx(TObject *Sender,
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
void __fastcall TSpanDialog::TimeH1UDChangingEx(TObject *Sender,
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
void __fastcall TSpanDialog::TimeY2UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeY2_Text=TimeY2->Text,s;
	double ep[]={2000,1,1,0,0,0};
	int p=TimeY2->SelStart,ud=Direction==updUp?1:-1;
	sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
	if (4<p&&p<8) {
	    ep[1]+=ud;
	    if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
	    else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
	}
	else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
	time2epoch(epoch2time(ep),ep);
	TimeY2->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
	TimeY2->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeH2UDChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString TimeH2_Text=TimeH2->Text,s;
	int hms[3]={0},sec,p=TimeH2->SelStart,ud=Direction==updUp?1:-1;
	sscanf(TimeH2_Text.c_str(),"%d:%d:%d",hms,hms+1,hms+2);
	if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
	sec=hms[0]*3600+hms[1]*60+hms[2];
	if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
	TimeH2->Text=s.sprintf("%02d:%02d:%02d",sec/3600,(sec%3600)/60,sec%60);
	TimeH2->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeY1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeY1UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeH1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeH1UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeY2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeY2UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::TimeH2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN) {
        TimeH2UDChangingEx(Sender,allowchange,0,Key==VK_UP?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::UpdateEnable(void)
{
	TimeY1  ->Enabled   =TimeStartF->Checked&&TimeVal[0];
	TimeH1  ->Enabled   =TimeStartF->Checked&&TimeVal[0];
	TimeY2  ->Enabled   =TimeEndF  ->Checked&&TimeVal[1];
	TimeH2  ->Enabled   =TimeEndF  ->Checked&&TimeVal[1];
	TimeY1UD->Enabled   =TimeStartF->Checked&&TimeVal[0];
	TimeH1UD->Enabled   =TimeStartF->Checked&&TimeVal[0];
	TimeY2UD->Enabled   =TimeEndF  ->Checked&&TimeVal[1];
	TimeH2UD->Enabled   =TimeEndF  ->Checked&&TimeVal[1];
	EditTimeInt->Enabled=TimeIntF  ->Checked&&TimeVal[2];
	BtnTime1->Enabled   =TimeStartF->Checked&&TimeVal[0];
	BtnTime2->Enabled   =TimeEndF  ->Checked&&TimeVal[1];
	TimeStartF->Enabled =TimeVal[0]==1;
	TimeEndF  ->Enabled =TimeVal[1]==1;
	TimeIntF  ->Enabled =TimeVal[2]==1;
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::BtnTime1Click(TObject *Sender)
{
	AnsiString TimeY1_Text=TimeY1->Text,TimeH1_Text=TimeH1->Text;
	double ep[]={2000,1,1,0,0,0};
	sscanf(TimeY1_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
	sscanf(TimeH1_Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
	TimeDialog->Time=epoch2time(ep);
	TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TSpanDialog::BtnTime2Click(TObject *Sender)
{
	AnsiString TimeY2_Text=TimeY2->Text,TimeH2_Text=TimeH2->Text;
	double ep[]={2000,1,1,0,0,0};
	sscanf(TimeY2_Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
	sscanf(TimeH2_Text.c_str(),"%lf:%lf:%lf",ep+3,ep+4,ep+5);
	TimeDialog->Time=epoch2time(ep);
	TimeDialog->ShowModal();
}
//---------------------------------------------------------------------------

