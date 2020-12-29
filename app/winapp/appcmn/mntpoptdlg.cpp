//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "mntpoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMntpOptDialog *MntpOptDialog;

//---------------------------------------------------------------------------
__fastcall TMntpOptDialog::TMntpOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMntpOptDialog::FormShow(TObject *Sender)
{
	TEdit *edit[]={
        SrcTbl1,SrcTbl2,SrcTbl3,NULL,SrcTbl5,SrcTbl6,SrcTbl7,SrcTbl8,SrcTbl9,
        NULL,NULL,SrcTbl12,SrcTbl13,NULL,NULL,SrcTbl16
	};
	TComboBox *box[]={
        NULL,NULL,NULL,SrcTbl4,NULL,NULL,NULL,NULL,NULL,SrcTbl10,SrcTbl11,NULL,
		NULL,SrcTbl14,SrcTbl15,NULL
	};
	AnsiString str=MntpStr;
	char buff[2048],*p=buff,*q;
	int i;
    
	MntPntE->Text=MntPnt;

	sprintf(buff,"%.2047s",str.c_str());
	
	for (int i=0;i<16;i++) {
		if (edit[i]) edit[i]->Text=""; else box[i]->ItemIndex=0;
	}
	for (int i=0;*p&&i<16;i++,p=q+1) {
		if ((q=strchr(p,';'))) *q='\0';
		if (edit[i]) edit[i]->Text=p; else box[i]->Text=p;
		if (!q) break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMntpOptDialog::BtnOkClick(TObject *Sender)
{
	MntPnt=MntPntE->Text;
	MntpStr=SrcTbl1->Text+";"+SrcTbl2->Text+";"+SrcTbl3->Text+";"+SrcTbl4->Text+";"+
	        SrcTbl5->Text+";"+SrcTbl6->Text+";"+SrcTbl7->Text+";"+SrcTbl8->Text+";"+
	        SrcTbl9->Text+";"+SrcTbl10->Text+";"+SrcTbl11->Text+";"+SrcTbl12->Text+";"+
	        SrcTbl13->Text+";"+SrcTbl14->Text+";"+SrcTbl15->Text+";"+SrcTbl16->Text;
}
//---------------------------------------------------------------------------
