//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "ftpoptdlg.h"
#include "keydlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFtpOptDialog *FtpOptDialog;

//---------------------------------------------------------------------------
__fastcall TFtpOptDialog::TFtpOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFtpOptDialog::FormShow(TObject *Sender)
{
	AnsiString s,cap[]={"FTP Option","HTTP Option"};
	char buff[2048],*p,*q;
	char *addr,*file=(char *)"",*user=(char *)"",*passwd=(char *)"";
	int topts[4]={0,3600,0,0};
	
	Caption=cap[Opt];
	
	strcpy(buff,Path.c_str());
    
    if ((p=strchr(buff,'/'))) {
        if ((q=strstr(p+1,"::"))) {
            *q='\0';
            sscanf(q+2,"T=%d,%d,%d,%d",topts,topts+1,topts+2,topts+3);
        }
        file=p+1;
        *p='\0';
    }
    if ((p=strrchr(buff,'@'))) {
        *p++='\0';
        if ((q=strchr(buff,':'))) {
             *q='\0'; passwd=q+1;
        }
        *q='\0'; user=buff;
    }
    else p=buff;
    addr=p;	
	
	Addr->Text=s.sprintf("%s/%s",addr,file);
	User->Text=user;
	Passwd->Text=passwd;
	PathOffset   ->Text=s.sprintf("%.2g",topts[0]/3600.0);
	Interval     ->Text=s.sprintf("%.2g",topts[1]/3600.0);
	Offset       ->Text=s.sprintf("%.2g",topts[2]/3600.0);
	RetryInterval->Text=s.sprintf("%d",topts[3]);
	Addr->Items->Clear();
	for (int i=0;i<MAXHIST;i++) {
		if (History[i]!="") Addr->Items->Add(History[i]);
	}
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TFtpOptDialog::BtnOkClick(TObject *Sender)
{
	AnsiString PathOffset_Text=PathOffset->Text;
	AnsiString Interval_Text=Interval->Text;
	AnsiString Offset_Text=Offset->Text;
	AnsiString RetryInterval_Text=RetryInterval->Text;
	AnsiString User_Text=User->Text,Passwd_Text=Passwd->Text;
	AnsiString Addr_Text=Addr->Text,s;
	int topts[4];
	
	topts[0]=(int)(atof(PathOffset_Text.c_str())*3600.0);
	topts[1]=(int)(atof(Interval_Text.c_str())*3600.0);
	topts[2]=(int)(atof(Offset_Text.c_str())*3600.0);
	topts[3]=atoi(RetryInterval_Text.c_str());
	
	Path=s.sprintf("%s:%s@%s::T=%d,%d,%d,%d",User_Text.c_str(),
				   Passwd_Text.c_str(),Addr_Text.c_str(),
				   topts[0],topts[1],topts[2],topts[3]);
	
	AddHist(Addr,History);
}
//---------------------------------------------------------------------------
void __fastcall TFtpOptDialog::BtnKeyClick(TObject *Sender)
{
	KeyDialog->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFtpOptDialog::AddHist(TComboBox *list, AnsiString *hist)
{
	for (int i=0;i<MAXHIST;i++) {
		if (list->Text!=hist[i]) continue;
		for (int j=i+1;j<MAXHIST;j++) hist[j-1]=hist[j];
		hist[MAXHIST-1]="";
	}
	for (int i=MAXHIST-1;i>0;i--) hist[i]=hist[i-1];
	hist[0]=list->Text;
	
	list->Clear();
	for (int i=0;i<MAXHIST;i++) {
		if (hist[i]!="") list->Items->Add(hist[i]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFtpOptDialog::UpdateEnable(void)
{
	User       ->Enabled=Opt==0;
	Passwd     ->Enabled=Opt==0;
	LabelUser  ->Enabled=Opt==0;
	LabelPasswd->Enabled=Opt==0;
}
//---------------------------------------------------------------------------

