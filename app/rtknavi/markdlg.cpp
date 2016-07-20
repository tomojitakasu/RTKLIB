//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "markdlg.h"

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
	AnsiString comment=CommentText->Text;
	char str1[32],str2[1024];
	
	if (RadioGo->Checked) {
		if (PosMode==PMODE_STATIC) {
			PosMode=PMODE_KINEMA;
		}
		else if (PosMode==PMODE_PPP_STATIC) {
			PosMode=PMODE_PPP_KINEMA;
		}
	}
	else if (RadioStop->Checked) {
		if (PosMode==PMODE_KINEMA) {
			PosMode=PMODE_STATIC;
		}
		else if (PosMode==PMODE_PPP_KINEMA) {
			PosMode=PMODE_PPP_STATIC;
		}
	}
	if (ChkMarkerName->Checked) {
		sprintf(str1,"%03d",NMark);
		reppath(marker.c_str(),str2,utc2gpst(timeget()),str1,"");
		rtksvrmark(&rtksvr,str2,comment.c_str());
		NMark++;
		Label1->Caption=s.sprintf("%%r=%03d",NMark);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::ChkMarkerNameClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMarkDialog::FormShow(TObject *Sender)
{
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
			 PosMode==PMODE_KINEMA||PosMode==PMODE_PPP_KINEMA;
	RadioStop->Enabled=ena;
	RadioGo  ->Enabled=ena;
	LabelPosMode->Enabled=ena;
	MarkerName->Enabled=ChkMarkerName->Checked;
}
