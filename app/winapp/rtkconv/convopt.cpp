//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "convmain.h"
#include "convopt.h"
#include "codeopt.h"
#include "freqdlg.h"
#include "glofcndlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConvOptDialog *ConvOptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TConvOptDialog::TConvOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	AnsiString s;
	int glo=MAXPRNGLO,gal=MAXPRNGAL,qzs=MAXPRNQZS,cmp=MAXPRNCMP,irn=MAXPRNIRN;
	if (glo<=0) Nav2->Enabled=false;
	if (gal<=0) Nav3->Enabled=false;
	if (qzs<=0) Nav4->Enabled=false;
	if (cmp<=0) Nav6->Enabled=false;
	if (irn<=0) Nav7->Enabled=false;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	RnxVer->ItemIndex=MainWindow->RnxVer;
	RnxFile->Checked=MainWindow->RnxFile;
	RnxCode->Text=MainWindow->RnxCode;
	RunBy->Text=MainWindow->RunBy;
	Marker->Text=MainWindow->Marker;
	MarkerNo->Text=MainWindow->MarkerNo;
	MarkerType->Text=MainWindow->MarkerType;
	Name0->Text=MainWindow->Name[0];
	Name1->Text=MainWindow->Name[1];
	Rec0->Text=MainWindow->Rec[0];
	Rec1->Text=MainWindow->Rec[1];
	Rec2->Text=MainWindow->Rec[2];
	Ant0->Text=MainWindow->Ant[0];
	Ant1->Text=MainWindow->Ant[1];
	AppPos0->Text=s.sprintf("%.4f",MainWindow->AppPos[0]);
	AppPos1->Text=s.sprintf("%.4f",MainWindow->AppPos[1]);
	AppPos2->Text=s.sprintf("%.4f",MainWindow->AppPos[2]);
	AntDel0->Text=s.sprintf("%.4f",MainWindow->AntDel[0]);
	AntDel1->Text=s.sprintf("%.4f",MainWindow->AntDel[1]);
	AntDel2->Text=s.sprintf("%.4f",MainWindow->AntDel[2]);
	Comment0->Text=MainWindow->Comment[0];
	Comment1->Text=MainWindow->Comment[1];
	RcvOption->Text=MainWindow->RcvOption;
	for (int i=0;i<7;i++) CodeMask[i]=MainWindow->CodeMask[i];
	AutoPos->Checked=MainWindow->AutoPos;
	PhaseShift->Checked=MainWindow->PhaseShift;
	HalfCyc->Checked=MainWindow->HalfCyc;
	OutIono->Checked=MainWindow->OutIono;
	OutTime->Checked=MainWindow->OutTime;
	OutLeaps->Checked=MainWindow->OutLeaps;
	GloFcnDialog->EnaGloFcn=MainWindow->EnaGloFcn;
	for (int i=0;i<27;i++) {
		GloFcnDialog->GloFcn[i]=MainWindow->GloFcn[i];
	}
	Nav1->Checked=MainWindow->NavSys&SYS_GPS;
	Nav2->Checked=MainWindow->NavSys&SYS_GLO;
	Nav3->Checked=MainWindow->NavSys&SYS_GAL;
	Nav4->Checked=MainWindow->NavSys&SYS_QZS;
	Nav5->Checked=MainWindow->NavSys&SYS_SBS;
	Nav6->Checked=MainWindow->NavSys&SYS_CMP;
	Nav7->Checked=MainWindow->NavSys&SYS_IRN;
	Obs1->Checked=MainWindow->ObsType&OBSTYPE_PR;
	Obs2->Checked=MainWindow->ObsType&OBSTYPE_CP;
	Obs3->Checked=MainWindow->ObsType&OBSTYPE_DOP;
	Obs4->Checked=MainWindow->ObsType&OBSTYPE_SNR;
	Freq1->Checked=MainWindow->FreqType&FREQTYPE_L1;
	Freq2->Checked=MainWindow->FreqType&FREQTYPE_L2;
	Freq3->Checked=MainWindow->FreqType&FREQTYPE_L3;
	Freq4->Checked=MainWindow->FreqType&FREQTYPE_L4;
	Freq5->Checked=MainWindow->FreqType&FREQTYPE_L5;
	ExSats->Text=MainWindow->ExSats;
	TraceLevel->ItemIndex=MainWindow->TraceLevel;
	ChkSepNav->Checked=MainWindow->SepNav;
	TimeTol->Text=s.sprintf("%.4g",MainWindow->TimeTol);
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::BtnOkClick(TObject *Sender)
{
	MainWindow->RnxVer=RnxVer->ItemIndex;
	MainWindow->RnxFile=RnxFile->Checked;
	MainWindow->RnxCode=RnxCode->Text;
	MainWindow->RunBy=RunBy->Text;
	MainWindow->Marker=Marker->Text;
	MainWindow->MarkerNo=MarkerNo->Text;
	MainWindow->MarkerType=MarkerType->Text;
	MainWindow->Name[0]=Name0->Text;
	MainWindow->Name[1]=Name1->Text;
	MainWindow->Rec[0]=Rec0->Text;
	MainWindow->Rec[1]=Rec1->Text;
	MainWindow->Rec[2]=Rec2->Text;
	MainWindow->Ant[0]=Ant0->Text;
	MainWindow->Ant[1]=Ant1->Text;
	MainWindow->AppPos[0]=str2dbl(AppPos0->Text);
	MainWindow->AppPos[1]=str2dbl(AppPos1->Text);
	MainWindow->AppPos[2]=str2dbl(AppPos2->Text);
	MainWindow->AntDel[0]=str2dbl(AntDel0->Text);
	MainWindow->AntDel[1]=str2dbl(AntDel1->Text);
	MainWindow->AntDel[2]=str2dbl(AntDel2->Text);
	MainWindow->Comment[0]=Comment0->Text;
	MainWindow->Comment[1]=Comment1->Text;
	MainWindow->RcvOption=RcvOption->Text;
	for (int i=0;i<7;i++) MainWindow->CodeMask[i]=CodeMask[i];
	MainWindow->AutoPos=AutoPos->Checked;
	MainWindow->PhaseShift=PhaseShift->Checked;
	MainWindow->HalfCyc=HalfCyc->Checked;
	MainWindow->OutIono=OutIono->Checked;
	MainWindow->OutTime=OutTime->Checked;
	MainWindow->OutLeaps=OutLeaps->Checked;
	MainWindow->EnaGloFcn=GloFcnDialog->EnaGloFcn;
	for (int i=0;i<27;i++) {
		MainWindow->GloFcn[i]=GloFcnDialog->GloFcn[i];
	}
	int navsys=0,obstype=0,freqtype=0;
	if (Nav1->Checked) navsys|=SYS_GPS;
	if (Nav2->Checked) navsys|=SYS_GLO;
	if (Nav3->Checked) navsys|=SYS_GAL;
	if (Nav4->Checked) navsys|=SYS_QZS;
	if (Nav5->Checked) navsys|=SYS_SBS;
	if (Nav6->Checked) navsys|=SYS_CMP;
	if (Nav7->Checked) navsys|=SYS_IRN;
	if (Obs1->Checked) obstype|=OBSTYPE_PR;
	if (Obs2->Checked) obstype|=OBSTYPE_CP;
	if (Obs3->Checked) obstype|=OBSTYPE_DOP;
	if (Obs4->Checked) obstype|=OBSTYPE_SNR;
	if (Freq1->Checked) freqtype|=FREQTYPE_L1;
	if (Freq2->Checked) freqtype|=FREQTYPE_L2;
	if (Freq3->Checked) freqtype|=FREQTYPE_L3;
	if (Freq4->Checked) freqtype|=FREQTYPE_L4;
	if (Freq5->Checked) freqtype|=FREQTYPE_L5;
	MainWindow->NavSys=navsys;
	MainWindow->ObsType=obstype;
	MainWindow->FreqType=freqtype;
	MainWindow->ExSats=ExSats->Text;
	MainWindow->TraceLevel=TraceLevel->ItemIndex;
	MainWindow->SepNav=ChkSepNav->Checked;
	MainWindow->TimeTol=str2dbl(TimeTol->Text);
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::RnxFileClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::RnxVerChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::AutoPosClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::BtnFreqClick(TObject *Sender)
{
    FreqDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::BtnMaskClick(TObject *Sender)
{
	CodeOptDialog->NavSys=0;
	CodeOptDialog->FreqType=0;
	if (Nav1->Checked) CodeOptDialog->NavSys|=SYS_GPS;
	if (Nav2->Checked) CodeOptDialog->NavSys|=SYS_GLO;
	if (Nav3->Checked) CodeOptDialog->NavSys|=SYS_GAL;
	if (Nav4->Checked) CodeOptDialog->NavSys|=SYS_QZS;
	if (Nav5->Checked) CodeOptDialog->NavSys|=SYS_SBS;
	if (Nav6->Checked) CodeOptDialog->NavSys|=SYS_CMP;
	if (Nav7->Checked) CodeOptDialog->NavSys|=SYS_IRN;
	if (Freq1->Checked) CodeOptDialog->FreqType|=FREQTYPE_L1;
	if (Freq2->Checked) CodeOptDialog->FreqType|=FREQTYPE_L2;
	if (Freq3->Checked) CodeOptDialog->FreqType|=FREQTYPE_L3;
	if (Freq4->Checked) CodeOptDialog->FreqType|=FREQTYPE_L4;
	if (Freq5->Checked) CodeOptDialog->FreqType|=FREQTYPE_L5;
	CodeOptDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::UpdateEnable(void)
{
	AppPos0->Enabled=AutoPos->Checked;
	AppPos1->Enabled=AutoPos->Checked;
	AppPos2->Enabled=AutoPos->Checked;
	ChkSepNav->Enabled=RnxVer->ItemIndex>=3;
	Label13->Enabled=MainWindow->TimeIntF->Checked;
	TimeTol->Enabled=MainWindow->TimeIntF->Checked;
	Nav3->Enabled=RnxVer->ItemIndex>=1;
	Nav4->Enabled=RnxVer->ItemIndex>=5;
	Nav6->Enabled=RnxVer->ItemIndex==2||RnxVer->ItemIndex>=4;
	Nav7->Enabled=RnxVer->ItemIndex>=6;
	Freq3->Enabled=RnxVer->ItemIndex>=1;
	Freq4->Enabled=RnxVer->ItemIndex>=1;
	Freq5->Enabled=RnxVer->ItemIndex>=1;
	PhaseShift->Enabled=RnxVer->ItemIndex>=4;
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::BtnFcnClick(TObject *Sender)
{
	if (GloFcnDialog->ShowModal()!=mrOk) return;
}
//---------------------------------------------------------------------------

