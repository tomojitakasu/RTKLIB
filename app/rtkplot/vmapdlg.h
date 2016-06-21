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
	TButton *BtnOk;
	TButton *BtnCancel;
	TCheckBox *Vis1;
	TLabel *Label1;
	TPanel *Color1;
	TLabel *Label2;
	TButton *BtnColor1;
	TPanel *Panel11;
	TRadioButton *Layer1;
	TLabel *Label3;
	TPanel *Panel12;
	TPanel *Color2;
	TButton *BtnColor2;
	TCheckBox *Vis2;
	TRadioButton *Layer2;
	TPanel *Panel13;
	TPanel *Color3;
	TButton *BtnColor3;
	TCheckBox *Vis3;
	TRadioButton *Layer3;
	TPanel *Panel14;
	TPanel *Color4;
	TButton *BtnColor4;
	TCheckBox *Vis4;
	TRadioButton *Layer4;
	TPanel *Panel15;
	TPanel *Color5;
	TButton *BtnColor5;
	TCheckBox *Vis5;
	TRadioButton *Layer5;
	TPanel *Panel16;
	TPanel *Color6;
	TButton *BtnColor6;
	TCheckBox *Vis6;
	TRadioButton *Layer6;
	TPanel *Panel17;
	TPanel *Color7;
	TButton *BtnColor7;
	TCheckBox *Vis7;
	TRadioButton *Layer7;
	TPanel *Panel18;
	TPanel *Color8;
	TButton *BtnColor8;
	TCheckBox *Vis8;
	TRadioButton *Layer8;
	TPanel *Panel19;
	TPanel *Color9;
	TButton *BtnColor9;
	TCheckBox *Vis9;
	TRadioButton *Layer9;
	TPanel *Panel1A;
	TPanel *Color10;
	TButton *BtnColor10;
	TCheckBox *Vis10;
	TRadioButton *Layer10;
	TBitBtn *BtnUp;
	TBitBtn *BtnDown;
	TPanel *Panel1;
	TColorDialog *ColorDialog;
	TPanel *Panel2;
	TPanel *Panel3;
	TPanel *Panel4;
	TPanel *Panel5;
	TPanel *Panel6;
	TPanel *Panel7;
	TPanel *Panel8;
	TPanel *Panel9;
	TPanel *Panel10;
	TPanel *Panel21;
	TPanel *Panel20;
	TPanel *Panel1B;
	TPanel *Color11;
	TButton *BtnColor11;
	TCheckBox *Vis11;
	TRadioButton *Layer11;
	TPanel *Panel24;
	TPanel *Panel22;
	TPanel *Color12;
	TButton *BtnColor12;
	TCheckBox *Vis12;
	TRadioButton *Layer12;
	void __fastcall BtnColor1Click(TObject *Sender);
	void __fastcall Layer1Click(TObject *Sender);
	void __fastcall BtnUpClick(TObject *Sender);
	void __fastcall BtnDownClick(TObject *Sender);
	void __fastcall BtnDeleteClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnColor2Click(TObject *Sender);
	void __fastcall BtnColor3Click(TObject *Sender);
	void __fastcall BtnColor4Click(TObject *Sender);
	void __fastcall BtnColor5Click(TObject *Sender);
	void __fastcall BtnColor6Click(TObject *Sender);
	void __fastcall BtnColor7Click(TObject *Sender);
	void __fastcall BtnColor8Click(TObject *Sender);
	void __fastcall BtnColor9Click(TObject *Sender);
	void __fastcall BtnColor10Click(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall Vis1Click(TObject *Sender);
	void __fastcall BtnColor11Click(TObject *Sender);
	void __fastcall BtnColor12Click(TObject *Sender);
private:
	gis_t Gis;
	void __fastcall UpdateLayer(void);
public:
	__fastcall TVecMapDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TVecMapDialog *VecMapDialog;
//---------------------------------------------------------------------------
#endif
