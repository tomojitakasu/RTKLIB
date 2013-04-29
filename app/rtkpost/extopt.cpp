//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "extopt.h"
#include "postopt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TExtOptDialog *ExtOptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TExtOptDialog::TExtOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::FormShow(TObject *Sender)
{
	GetExtErrOpt();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::BtnOkClick(TObject *Sender)
{
	SetExtErrOpt();
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::ExtEna0Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::ExtEna1Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::ExtEna3Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::ExtEna2Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::GetExtErrOpt(void)
{
	TEdit *editc[][6]={
		{CodeErr00,CodeErr01,CodeErr02,CodeErr03,CodeErr04,CodeErr05},
		{CodeErr10,CodeErr11,CodeErr12,CodeErr13,CodeErr14,CodeErr15},
		{CodeErr20,CodeErr21,CodeErr22,CodeErr23,CodeErr24,CodeErr25}
	};
	TEdit *editp[][6]={
		{PhaseErr00,PhaseErr01,PhaseErr02,PhaseErr03,PhaseErr04,PhaseErr05},
		{PhaseErr10,PhaseErr11,PhaseErr12,PhaseErr13,PhaseErr14,PhaseErr15},
		{PhaseErr20,PhaseErr21,PhaseErr22,PhaseErr23,PhaseErr24,PhaseErr25}
	};
	AnsiString s;
	
	ExtEna0->Checked=OptDialog->ExtErr.ena[0];
	ExtEna1->Checked=OptDialog->ExtErr.ena[1];
	ExtEna2->Checked=OptDialog->ExtErr.ena[2];
	ExtEna3->Checked=OptDialog->ExtErr.ena[3];
	
	for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
		editc[i][j]->Text=s.sprintf("%.3f",OptDialog->ExtErr.cerr[i][j]);
		editp[i][j]->Text=s.sprintf("%.3f",OptDialog->ExtErr.perr[i][j]);
	}
	GpsGloB0->Text=s.sprintf("%.3f",OptDialog->ExtErr.gpsglob[0]);
	GpsGloB1->Text=s.sprintf("%.3f",OptDialog->ExtErr.gpsglob[1]);
	GloICB0->Text=s.sprintf("%.3f",OptDialog->ExtErr.gloicb[0]);
	GloICB1->Text=s.sprintf("%.3f",OptDialog->ExtErr.gloicb[1]);
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::SetExtErrOpt(void)
{
	TEdit *editc[][6]={
		{CodeErr00,CodeErr01,CodeErr02,CodeErr03,CodeErr04,CodeErr05},
		{CodeErr10,CodeErr11,CodeErr12,CodeErr13,CodeErr14,CodeErr15},
		{CodeErr20,CodeErr21,CodeErr22,CodeErr23,CodeErr24,CodeErr25}
	};
	TEdit *editp[][6]={
		{PhaseErr00,PhaseErr01,PhaseErr02,PhaseErr03,PhaseErr04,PhaseErr05},
		{PhaseErr10,PhaseErr11,PhaseErr12,PhaseErr13,PhaseErr14,PhaseErr15},
		{PhaseErr20,PhaseErr21,PhaseErr22,PhaseErr23,PhaseErr24,PhaseErr25}
	};
	OptDialog->ExtErr.ena[0]=ExtEna0->Checked;
	OptDialog->ExtErr.ena[1]=ExtEna1->Checked;
	OptDialog->ExtErr.ena[2]=ExtEna2->Checked;
	OptDialog->ExtErr.ena[3]=ExtEna3->Checked;
	
	for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
		OptDialog->ExtErr.cerr[i][j]=str2dbl(editc[i][j]->Text);
		OptDialog->ExtErr.perr[i][j]=str2dbl(editp[i][j]->Text);
	}
	OptDialog->ExtErr.gloicb[0]=str2dbl(GloICB0->Text);
	OptDialog->ExtErr.gloicb[1]=str2dbl(GloICB1->Text);
	OptDialog->ExtErr.gpsglob[0]=str2dbl(GpsGloB0->Text);
	OptDialog->ExtErr.gpsglob[1]=str2dbl(GpsGloB1->Text);
}
//---------------------------------------------------------------------------
void __fastcall TExtOptDialog::UpdateEnable(void)
{
	TEdit *editc[][6]={
		{CodeErr00,CodeErr01,CodeErr02,CodeErr03,CodeErr04,CodeErr05},
		{CodeErr10,CodeErr11,CodeErr12,CodeErr13,CodeErr14,CodeErr15},
		{CodeErr20,CodeErr21,CodeErr22,CodeErr23,CodeErr24,CodeErr25}
	};
	TEdit *editp[][6]={
		{PhaseErr00,PhaseErr01,PhaseErr02,PhaseErr03,PhaseErr04,PhaseErr05},
		{PhaseErr10,PhaseErr11,PhaseErr12,PhaseErr13,PhaseErr14,PhaseErr15},
		{PhaseErr20,PhaseErr21,PhaseErr22,PhaseErr23,PhaseErr24,PhaseErr25}
	};
	for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
		editc[i][j]->Enabled=ExtEna0->Checked;
		editp[i][j]->Enabled=ExtEna1->Checked;
	}
	GloICB0->Enabled=ExtEna2->Checked;
	GloICB1->Enabled=ExtEna2->Checked;
	GpsGloB0->Enabled=ExtEna3->Checked;
	GpsGloB1->Enabled=ExtEna3->Checked;
}
//---------------------------------------------------------------------------

