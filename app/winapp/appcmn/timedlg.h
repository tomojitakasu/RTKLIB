//---------------------------------------------------------------------------
#ifndef timedlgH
#define timedlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "rtklib.h"
//---------------------------------------------------------------------------
class TTimeDialog : public TForm
{
__published:
	TButton *BtnOk;
	TLabel *Message;
	void __fastcall FormShow(TObject *Sender);
private:
public:
	gtime_t Time;
	__fastcall TTimeDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TTimeDialog *TimeDialog;
//---------------------------------------------------------------------------
#endif
