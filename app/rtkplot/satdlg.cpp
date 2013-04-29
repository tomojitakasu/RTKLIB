//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "satdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSatDialog *SatDialog;
//---------------------------------------------------------------------------
__fastcall TSatDialog::TSatDialog(TComponent* Owner)
	: TForm(Owner)
{
	for (int i=0;i<36;i++) ValidSat[i]=1;
}
//---------------------------------------------------------------------------
void __fastcall TSatDialog::FormShow(TObject *Sender)
{
	TCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};
	for (int i=0;i<36;i++) sat[i]->Checked=ValidSat[i];
}
//---------------------------------------------------------------------------
void __fastcall TSatDialog::BtnChkAllClick(TObject *Sender)
{
	TCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};
	for (int i=0;i<36;i++) sat[i]->Checked=true;
}
//---------------------------------------------------------------------------
void __fastcall TSatDialog::BtnUnchkAllClick(TObject *Sender)
{
	TCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};
	for (int i=0;i<36;i++) sat[i]->Checked=false;
}
//---------------------------------------------------------------------------
void __fastcall TSatDialog::BtnOkClick(TObject *Sender)
{
	TCheckBox *sat[]={
		PRN01,PRN02,PRN03,PRN04,PRN05,PRN06,PRN07,PRN08,PRN09,PRN10,
		PRN11,PRN12,PRN13,PRN14,PRN15,PRN16,PRN17,PRN18,PRN19,PRN20,
		PRN21,PRN22,PRN23,PRN24,PRN25,PRN26,PRN27,PRN28,PRN29,PRN30,
		PRN31,PRN32,SBAS,GLO,GAL,PRN33
	};
	for (int i=0;i<36;i++) ValidSat[i]=sat[i]->Checked;
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TSatDialog::BtnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

