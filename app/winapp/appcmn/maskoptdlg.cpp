//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "maskoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMaskOptDialog *MaskOptDialog;
//---------------------------------------------------------------------------
__fastcall TMaskOptDialog::TMaskOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	Mask.ena[0]=0;
	Mask.ena[1]=0;
	for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
		Mask.mask[i][j]=0.0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMaskOptDialog::FormShow(TObject *Sender)
{
	TEdit *mask[][9]={
		{Mask_1_1,Mask_1_2,Mask_1_3,Mask_1_4,Mask_1_5,Mask_1_6,Mask_1_7,Mask_1_8,Mask_1_9},
		{Mask_2_1,Mask_2_2,Mask_2_3,Mask_2_4,Mask_2_5,Mask_2_6,Mask_2_7,Mask_2_8,Mask_2_9},
		{Mask_3_1,Mask_3_2,Mask_3_3,Mask_3_4,Mask_3_5,Mask_3_6,Mask_3_7,Mask_3_8,Mask_3_9}
	};
	AnsiString s;
	MaskEna1->Checked=Mask.ena[0];
	MaskEna2->Checked=Mask.ena[1];
	for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
		mask[i][j]->Text=s.sprintf("%.0f",Mask.mask[i][j]);
	}
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMaskOptDialog::BtnOkClick(TObject *Sender)
{
	TEdit *mask[][9]={
		{Mask_1_1,Mask_1_2,Mask_1_3,Mask_1_4,Mask_1_5,Mask_1_6,Mask_1_7,Mask_1_8,Mask_1_9},
		{Mask_2_1,Mask_2_2,Mask_2_3,Mask_2_4,Mask_2_5,Mask_2_6,Mask_2_7,Mask_2_8,Mask_2_9},
		{Mask_3_1,Mask_3_2,Mask_3_3,Mask_3_4,Mask_3_5,Mask_3_6,Mask_3_7,Mask_3_8,Mask_3_9}
	};
	Mask.ena[0]=MaskEna1->Checked;
	Mask.ena[1]=MaskEna2->Checked;
	for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
		Mask.mask[i][j]=mask[i][j]->Text.ToDouble();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMaskOptDialog::MaskEna1Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMaskOptDialog::UpdateEnable(void)
{
	TEdit *mask[][9]={
		{Mask_1_1,Mask_1_2,Mask_1_3,Mask_1_4,Mask_1_5,Mask_1_6,Mask_1_7,Mask_1_8,Mask_1_9},
		{Mask_2_1,Mask_2_2,Mask_2_3,Mask_2_4,Mask_2_5,Mask_2_6,Mask_2_7,Mask_2_8,Mask_2_9},
		{Mask_3_1,Mask_3_2,Mask_3_3,Mask_3_4,Mask_3_5,Mask_3_6,Mask_3_7,Mask_3_8,Mask_3_9}
	};
	for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
		mask[i][j]->Enabled=MaskEna1->Checked||MaskEna2->Checked;
	}
}
//---------------------------------------------------------------------------

