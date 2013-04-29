//---------------------------------------------------------------------------
#ifndef satdlgH
#define satdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSatDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOk;
	TPanel *Panel1;
	TPanel *Panel2;
	TPanel *Panel3;
	TPanel *Panel4;
	TPanel *Panel5;
	TCheckBox *PRN01;
	TCheckBox *PRN02;
	TCheckBox *PRN03;
	TCheckBox *PRN04;
	TCheckBox *PRN05;
	TCheckBox *PRN06;
	TCheckBox *PRN07;
	TCheckBox *PRN08;
	TCheckBox *PRN09;
	TCheckBox *PRN10;
	TCheckBox *PRN11;
	TCheckBox *PRN12;
	TCheckBox *PRN13;
	TCheckBox *PRN14;
	TCheckBox *PRN15;
	TCheckBox *PRN16;
	TCheckBox *PRN17;
	TCheckBox *PRN18;
	TCheckBox *PRN19;
	TCheckBox *PRN20;
	TCheckBox *PRN21;
	TCheckBox *PRN22;
	TCheckBox *PRN23;
	TCheckBox *PRN24;
	TCheckBox *PRN25;
	TCheckBox *PRN26;
	TCheckBox *PRN27;
	TCheckBox *PRN28;
	TCheckBox *PRN29;
	TCheckBox *PRN30;
	TCheckBox *PRN31;
	TCheckBox *PRN32;
	TCheckBox *SBAS;
	TCheckBox *GLO;
	TCheckBox *GAL;
	TCheckBox *PRN33;
	TButton *BtnChkAll;
	TButton *BtnUnchkAll;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnChkAllClick(TObject *Sender);
	void __fastcall BtnUnchkAllClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:
public:
	int ValidSat[36];
	__fastcall TSatDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSatDialog *SatDialog;
//---------------------------------------------------------------------------
#endif
