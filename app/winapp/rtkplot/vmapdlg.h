//---------------------------------------------------------------------------

#ifndef vmapdlgH
#define vmapdlgH
//---------------------------------------------------------------------------
#include "rtklib.h"

#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TVecMapDialog : public TForm
{
__published:
	TButton *BtnApply;
	TButton *BtnClose;
	TCheckBox *Vis1;
	TLabel *Label1;
	TPanel *Color1;
	TLabel *Label2;
	TPanel *Panel11;
	TRadioButton *Layer1;
	TLabel *Label3;
	TPanel *Panel12;
	TPanel *Color2;
	TCheckBox *Vis2;
	TRadioButton *Layer2;
	TPanel *Panel13;
	TPanel *Color3;
	TCheckBox *Vis3;
	TRadioButton *Layer3;
	TPanel *Panel14;
	TPanel *Color4;
	TCheckBox *Vis4;
	TRadioButton *Layer4;
	TPanel *Panel15;
	TPanel *Color5;
	TCheckBox *Vis5;
	TRadioButton *Layer5;
	TPanel *Panel16;
	TPanel *Color6;
	TCheckBox *Vis6;
	TRadioButton *Layer6;
	TPanel *Panel17;
	TPanel *Color7;
	TCheckBox *Vis7;
	TRadioButton *Layer7;
	TPanel *Panel18;
	TPanel *Color8;
	TCheckBox *Vis8;
	TRadioButton *Layer8;
	TPanel *Panel19;
	TPanel *Color9;
	TCheckBox *Vis9;
	TRadioButton *Layer9;
	TPanel *Panel1A;
	TPanel *Color10;
	TCheckBox *Vis10;
	TRadioButton *Layer10;
	TBitBtn *BtnUp;
	TBitBtn *BtnDown;
	TPanel *Panel1;
	TColorDialog *ColorDialog;
	TPanel *Panel21;
	TPanel *Panel1B;
	TPanel *Color11;
	TCheckBox *Vis11;
	TRadioButton *Layer11;
	TPanel *Panel22;
	TPanel *Color12;
	TCheckBox *Vis12;
	TRadioButton *Layer12;
	TLabel *Label4;
	TPanel *Color1F;
	TPanel *Color2F;
	TPanel *Color3F;
	TPanel *Color4F;
	TPanel *Color5F;
	TPanel *Color6F;
	TPanel *Color7F;
	TPanel *Color8F;
	TPanel *Color9F;
	TPanel *Color10F;
	TPanel *Color11F;
	TPanel *Color12F;
	void __fastcall Layer1Click(TObject *Sender);
	void __fastcall BtnUpClick(TObject *Sender);
	void __fastcall BtnDownClick(TObject *Sender);
	void __fastcall BtnDeleteClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnApplyClick(TObject *Sender);
	void __fastcall Vis1Click(TObject *Sender);
	void __fastcall ColorClick(TObject *Sender);
	void __fastcall BtnCloseClick(TObject *Sender);
private:
	gis_t Gis;
	void __fastcall UpdateMap(void);
public:
	__fastcall TVecMapDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TVecMapDialog *VecMapDialog;
//---------------------------------------------------------------------------
#endif
