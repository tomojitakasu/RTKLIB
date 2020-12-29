//---------------------------------------------------------------------------
#ifndef confdlgH
#define confdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TConfDialog : public TForm
{
__published:
	TButton *BtnOverwrite;
	TButton *BtnCancel;
	TLabel *Label1;
	TLabel *Label2;
private:
public:
	__fastcall TConfDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConfDialog *ConfDialog;
//---------------------------------------------------------------------------
#endif
