//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "keydlg.h"
#include "markdlg.h"
#include "refdlg.h"
#include "naviopt.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMarkDialog *MarkDialog;

extern rtksvr_t rtksvr;

//---------------------------------------------------------------------------
__fastcall TMarkDialog::TMarkDialog(TComponent* Owner)
	: TForm(Owner)
{
	AnsiString s;
	NMark=1;
	FixPos[0]=FixPos[0]=FixPos[0]=0.0;
	Label1->Caption=s.sprintf("%%r=%03d",NMark);
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::BtnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::BtnOkClick(TObject *Sender)
{
	AnsiString s;
	AnsiString marker=MarkerName->Text;
	AnsiString comment=MarkerComment->Text;
	char str1[32],str2[1024];
	
	if (RadioGo->Checked) {
		if (PosMode==PMODE_STATIC||PosMode==PMODE_FIXED) {
			PosMode=PMODE_KINEMA;
		}
		else if (PosMode==PMODE_PPP_STATIC||PosMode==PMODE_PPP_FIXED) {
			PosMode=PMODE_PPP_KINEMA;
		}
	}
	else if (RadioStop->Checked) {
		if (PosMode==PMODE_KINEMA||PosMode==PMODE_FIXED) {
			PosMode=PMODE_STATIC;
		}
		else if (PosMode==PMODE_PPP_KINEMA||PosMode==PMODE_PPP_FIXED) {
			PosMode=PMODE_PPP_STATIC;
		}
	}
	else if (RadioFix->Checked) {
		if (PosMode==PMODE_KINEMA||PosMode==PMODE_STATIC) {
			PosMode=PMODE_FIXED;
		}
		else if (PosMode==PMODE_PPP_KINEMA||PosMode==PMODE_PPP_STATIC) {
			PosMode=PMODE_PPP_FIXED;
		}
	}
	if (ChkMarkerName->Checked) {
		sprintf(str1,"%03d",NMark);
		reppath(marker.c_str(),str2,utc2gpst(timeget()),str1,"");
		rtksvrmark(&rtksvr,str2,comment.c_str());
		NMark++;
		Label1->Caption=s.sprintf("%%r=%03d",NMark);
	}
	Marker=marker;
	Comment=comment;
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::ChkMarkerNameClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::FormShow(TObject *Sender)
{
	MarkerName->Text=Marker;
	MarkerComment->Text=Comment;
	
	if (PosMode==PMODE_STATIC||PosMode==PMODE_PPP_STATIC) {
		RadioStop->Checked=true;
	}
	else if (PosMode==PMODE_KINEMA||PosMode==PMODE_PPP_KINEMA) {
		RadioGo->Checked=true;
	}
	else {
		RadioStop->Checked=false;
		RadioGo  ->Checked=false;
	}
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::UpdateEnable(void)
{
	bool ena=PosMode==PMODE_STATIC||PosMode==PMODE_PPP_STATIC||
			 PosMode==PMODE_KINEMA||PosMode==PMODE_PPP_KINEMA||
			 PosMode==PMODE_FIXED ||PosMode==PMODE_PPP_FIXED;
	RadioStop->Enabled=ena;
	RadioGo  ->Enabled=ena;
	RadioFix ->Enabled=ena;
	LabelPosMode->Enabled=ena;
	EditLat->Enabled=RadioFix->Checked;
	EditLon->Enabled=RadioFix->Checked;
	EditHgt->Enabled=RadioFix->Checked;
	BtnPos->Enabled=RadioFix->Checked;
	LabelPos->Enabled=RadioFix->Checked;
	MarkerName->Enabled=ChkMarkerName->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::BtnRepDlgClick(TObject *Sender)
{
	KeyDialog->Caption="Keyword Replacement in Marker Name";
	KeyDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::RadioGoClick(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::BtnPosClick(TObject *Sender)
{
	AnsiString s;
	RefDialog->Pos[0]=EditLat->Text.ToDouble();
	RefDialog->Pos[1]=EditLon->Text.ToDouble();
	RefDialog->Pos[2]=EditHgt->Text.ToDouble();
	RefDialog->StaPosFile=OptDialog->StaPosFileF;
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	EditLat->Text=s.sprintf("%.9f",RefDialog->Pos[0]);
	EditLon->Text=s.sprintf("%.9f",RefDialog->Pos[1]);
	EditHgt->Text=s.sprintf("%.4f",RefDialog->Pos[2]);
}
//---------------------------------------------------------------------------

