//---------------------------------------------------------------------------

#ifndef glofcndlgH
#define glofcndlgH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TGloFcnDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TPanel *Panel1;
	TLabel *Label1;
	TEdit *Fcn10;
	TPanel *Panel2;
	TPanel *Panel3;
	TPanel *Panel4;
	TLabel *Label2;
	TEdit *Fcn09;
	TPanel *Panel5;
	TLabel *Label3;
	TEdit *Fcn08;
	TPanel *Panel6;
	TLabel *Label4;
	TEdit *Fcn07;
	TPanel *Panel7;
	TLabel *Label5;
	TEdit *Fcn06;
	TPanel *Panel8;
	TLabel *Label6;
	TEdit *Fcn05;
	TPanel *Panel9;
	TLabel *Label7;
	TEdit *Fcn04;
	TPanel *Panel10;
	TLabel *Label8;
	TEdit *Fcn03;
	TPanel *Panel11;
	TLabel *Label9;
	TEdit *Fcn02;
	TPanel *Panel12;
	TLabel *Label10;
	TEdit *Fcn01;
	TPanel *Panel13;
	TPanel *Panel14;
	TLabel *Label11;
	TEdit *Fcn20;
	TPanel *Panel15;
	TPanel *Panel16;
	TLabel *Label12;
	TEdit *Fcn19;
	TPanel *Panel17;
	TLabel *Label13;
	TEdit *Fcn18;
	TPanel *Panel18;
	TLabel *Label14;
	TEdit *Fcn17;
	TPanel *Panel19;
	TLabel *Label15;
	TEdit *Fcn16;
	TPanel *Panel20;
	TLabel *Label16;
	TEdit *Fcn15;
	TPanel *Panel21;
	TLabel *Label17;
	TEdit *Fcn14;
	TPanel *Panel22;
	TLabel *Label18;
	TEdit *Fcn13;
	TPanel *Panel23;
	TLabel *Label19;
	TEdit *Fcn12;
	TPanel *Panel24;
	TLabel *Label20;
	TEdit *Fcn11;
	TPanel *Panel25;
	TPanel *Panel27;
	TPanel *Panel30;
	TLabel *Label24;
	TEdit *Fcn27;
	TPanel *Panel31;
	TLabel *Label25;
	TEdit *Fcn26;
	TPanel *Panel32;
	TLabel *Label26;
	TEdit *Fcn25;
	TPanel *Panel33;
	TLabel *Label27;
	TEdit *Fcn24;
	TPanel *Panel34;
	TLabel *Label28;
	TEdit *Fcn23;
	TPanel *Panel35;
	TLabel *Label29;
	TEdit *Fcn22;
	TPanel *Panel36;
	TLabel *Label30;
	TEdit *Fcn21;
	TButton *BtnRead;
	TOpenDialog *OpenDialog;
	TButton *BtnClear;
	TLabel *Label21;
	TLabel *Label22;
	TCheckBox *EnaFcn;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnReadClick(TObject *Sender);
	void __fastcall BtnClearClick(TObject *Sender);
	void __fastcall EnaFcnClick(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
	TEdit * __fastcall GetFcn(int prn);
public:	
	__fastcall TGloFcnDialog(TComponent* Owner);

	int EnaGloFcn,GloFcn[27];
};
//---------------------------------------------------------------------------
extern PACKAGE TGloFcnDialog *GloFcnDialog;
//---------------------------------------------------------------------------
#endif
