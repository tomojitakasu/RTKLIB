//---------------------------------------------------------------------------
#ifndef convdlgH
#define convdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Vcl.Graphics.hpp>
//---------------------------------------------------------------------------
class TConvDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TCheckBox *Conversion;
	TComboBox *InFormat;
	TComboBox *OutFormat;
	TLabel *Label2;
	TLabel *Label10;
	TEdit *Options;
	TLabel *Label4;
	TEdit *OutMsgs;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall ConversionClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	AnsiString ConvMsg,ConvOpt,AntType,RcvType;
	int ConvEna,ConvInp,ConvOut,StaId;
	double AntPos[3],AntOff[3];
	
	__fastcall TConvDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConvDialog *ConvDialog;
//---------------------------------------------------------------------------
#endif
