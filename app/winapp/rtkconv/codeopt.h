//---------------------------------------------------------------------------
#ifndef codeoptH
#define codeoptH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TCodeOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TGroupBox *GroupBox1;
	TCheckBox *G01;
	TCheckBox *G02;
	TCheckBox *G03;
	TCheckBox *G04;
	TCheckBox *G06;
	TCheckBox *G07;
	TCheckBox *G05;
	TCheckBox *G14;
	TCheckBox *G15;
	TCheckBox *G16;
	TCheckBox *G17;
	TCheckBox *G19;
	TCheckBox *G20;
	TCheckBox *G18;
	TCheckBox *G22;
	TCheckBox *G24;
	TCheckBox *G25;
	TCheckBox *G26;
	TGroupBox *GroupBox2;
	TCheckBox *R01;
	TCheckBox *R02;
	TCheckBox *R14;
	TCheckBox *R19;
	TGroupBox *GroupBox3;
	TCheckBox *E01;
	TCheckBox *E10;
	TCheckBox *E11;
	TCheckBox *E12;
	TCheckBox *E13;
	TCheckBox *E24;
	TCheckBox *E25;
	TCheckBox *E26;
	TCheckBox *E27;
	TCheckBox *E28;
	TCheckBox *E29;
	TCheckBox *E30;
	TCheckBox *E32;
	TCheckBox *E33;
	TCheckBox *E31;
	TCheckBox *E37;
	TCheckBox *E38;
	TCheckBox *E39;
	TGroupBox *GroupBox4;
	TCheckBox *J01;
	TCheckBox *J07;
	TCheckBox *J08;
	TCheckBox *J13;
	TCheckBox *J12;
	TCheckBox *J16;
	TCheckBox *J17;
	TCheckBox *J18;
	TCheckBox *J24;
	TCheckBox *J26;
	TCheckBox *J35;
	TCheckBox *J25;
	TGroupBox *GroupBox5;
	TCheckBox *S01;
	TCheckBox *S24;
	TCheckBox *S25;
	TCheckBox *S26;
	TCheckBox *J36;
	TCheckBox *G21;
	TCheckBox *G08;
	TCheckBox *G23;
	TCheckBox *E34;
	TButton *BtnSetAll;
	TCheckBox *J33;
	TGroupBox *GroupBox6;
	TCheckBox *C40;
	TCheckBox *C42;
	TCheckBox *C27;
	TCheckBox *C28;
	TCheckBox *C29;
	TCheckBox *C41;
	TCheckBox *C18;
	TCheckBox *C43;
	TCheckBox *C33;
	TCheckBox *R44;
	TCheckBox *R46;
	TCheckBox *R45;
	TGroupBox *GroupBox7;
	TCheckBox *I49;
	TCheckBox *I54;
	TCheckBox *I26;
	TCheckBox *I52;
	TCheckBox *I53;
	TCheckBox *I50;
	TCheckBox *I51;
	TCheckBox *I55;
	TCheckBox *R66;
	TCheckBox *R67;
	TCheckBox *R68;
	TCheckBox *R30;
	TCheckBox *R31;
	TCheckBox *R33;
	TCheckBox *J57;
	TCheckBox *J58;
	TCheckBox *J59;
	TCheckBox *J60;
	TCheckBox *J34;
	TCheckBox *C56;
	TCheckBox *C02;
	TCheckBox *C12;
	TCheckBox *C10;
	TCheckBox *C06;
	TCheckBox *C57;
	TCheckBox *C58;
	TCheckBox *C26;
	TCheckBox *C61;
	TCheckBox *C62;
	TCheckBox *C63;
	TCheckBox *C64;
	TCheckBox *C65;
	TCheckBox *C39;
	TCheckBox *C30;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnSetAllClick(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	int NavSys,FreqType;
	__fastcall TCodeOptDialog(TComponent* Owner);
};
//----------------------------------------------------------------------
extern PACKAGE TCodeOptDialog *CodeOptDialog;
//----------------------------------------------------------------------
#endif
