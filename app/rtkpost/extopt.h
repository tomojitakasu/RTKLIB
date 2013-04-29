//---------------------------------------------------------------------------
#ifndef extoptH
#define extoptH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TExtOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TPanel *Panel6;
	TLabel *Label23;
	TLabel *Label48;
	TLabel *Label49;
	TLabel *Label59;
	TLabel *Label61;
	TLabel *Label84;
	TLabel *Label85;
	TLabel *Label88;
	TLabel *Label89;
	TCheckBox *ExtEna0;
	TPanel *Panel2;
	TLabel *Label69;
	TLabel *Label50;
	TLabel *Label51;
	TEdit *CodeErr00;
	TEdit *CodeErr01;
	TEdit *CodeErr02;
	TEdit *CodeErr03;
	TEdit *CodeErr04;
	TEdit *CodeErr05;
	TPanel *Panel3;
	TLabel *Label52;
	TLabel *Label70;
	TEdit *CodeErr10;
	TEdit *CodeErr11;
	TEdit *CodeErr12;
	TEdit *CodeErr13;
	TEdit *CodeErr14;
	TEdit *CodeErr15;
	TPanel *Panel4;
	TLabel *Label71;
	TLabel *Label72;
	TLabel *Label73;
	TEdit *CodeErr20;
	TEdit *CodeErr21;
	TEdit *CodeErr22;
	TEdit *CodeErr23;
	TEdit *CodeErr24;
	TEdit *CodeErr25;
	TPanel *Panel7;
	TLabel *Label12;
	TLabel *Label53;
	TLabel *Label58;
	TLabel *Label90;
	TLabel *Label91;
	TLabel *Label92;
	TLabel *Label93;
	TLabel *Label94;
	TLabel *Label95;
	TCheckBox *ExtEna1;
	TPanel *Panel8;
	TLabel *Label74;
	TLabel *Label77;
	TLabel *Label80;
	TEdit *PhaseErr00;
	TEdit *PhaseErr01;
	TEdit *PhaseErr02;
	TEdit *PhaseErr03;
	TEdit *PhaseErr04;
	TEdit *PhaseErr05;
	TPanel *Panel9;
	TLabel *Label75;
	TLabel *Label78;
	TEdit *PhaseErr10;
	TEdit *PhaseErr11;
	TEdit *PhaseErr12;
	TEdit *PhaseErr13;
	TEdit *PhaseErr14;
	TEdit *PhaseErr15;
	TPanel *Panel10;
	TLabel *Label76;
	TLabel *Label79;
	TLabel *Label81;
	TEdit *PhaseErr20;
	TEdit *PhaseErr21;
	TEdit *PhaseErr22;
	TEdit *PhaseErr23;
	TEdit *PhaseErr24;
	TEdit *PhaseErr25;
	TPanel *Panel5;
	TLabel *Label54;
	TLabel *Label56;
	TLabel *Label57;
	TLabel *Label55;
	TLabel *Label67;
	TLabel *Label68;
	TCheckBox *ExtEna2;
	TEdit *GloICB0;
	TEdit *GloICB1;
	TCheckBox *ExtEna3;
	TEdit *GpsGloB0;
	TEdit *GpsGloB1;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall ExtEna0Click(TObject *Sender);
	void __fastcall ExtEna1Click(TObject *Sender);
	void __fastcall ExtEna3Click(TObject *Sender);
	void __fastcall ExtEna2Click(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:
	void __fastcall GetExtErrOpt(void);
	void __fastcall SetExtErrOpt(void);
	void __fastcall UpdateEnable(void);
public:
	__fastcall TExtOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TExtOptDialog *ExtOptDialog;
//---------------------------------------------------------------------------
#endif
