//---------------------------------------------------------------------------
#ifndef rcvoptdlgH
#define rcvoptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TRcvOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BnCancel;
	TEdit *OptionE;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
private:
public:
	AnsiString Option;
	__fastcall TRcvOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TRcvOptDialog *RcvOptDialog;
//---------------------------------------------------------------------------
#endif
