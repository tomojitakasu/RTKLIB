//---------------------------------------------------------------------------
#ifndef startdlgH
#define startdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "rtklib.h"
//---------------------------------------------------------------------------
class TStartDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TPanel *Panel1;
	TEdit *TimeY1;
	TUpDown *TimeY1UD;
	TEdit *TimeH1;
	TUpDown *TimeH1UD;
	TLabel *Label1;
	TButton *BtnFileTime;
	void __fastcall TimeY1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeH1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnFileTimeClick(TObject *Sender);
private:
public:
	const char *FileName;
	gtime_t Time;
	__fastcall TStartDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TStartDialog *StartDialog;
//---------------------------------------------------------------------------
#endif
