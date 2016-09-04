//---------------------------------------------------------------------------
#ifndef mondlgH
#define mondlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include "rtklib.h"

#define MAX_MSG_BUFF	4096

//---------------------------------------------------------------------------
class TStrMonDialog : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel2;
	TButton *BtnClose;
	TSpeedButton *BtnClear;
	TPaintBox *Console;
	TScrollBar *Scroll;
	TSpeedButton *BtnDown;
	TSpeedButton *BtnStop;
	TComboBox *SelFmt;
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall BtnClearClick(TObject *Sender);
	void __fastcall ConsolePaint(TObject *Sender);
	void __fastcall ScrollChange(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall BtnDownClick(TObject *Sender);
	void __fastcall BtnStopClick(TObject *Sender);
	void __fastcall SelFmtChange(TObject *Sender);
private:
	TStringList *ConBuff;
	int Stop,ScrollPos;
	rtcm_t rtcm;
	raw_t raw;
	
	void __fastcall AddConsole(unsigned char *msg, int len, int mode);
public:
	int StrFmt;
	__fastcall TStrMonDialog(TComponent* Owner);
	void __fastcall AddMsg(unsigned char *buff, int n);
};
//---------------------------------------------------------------------------
extern PACKAGE TStrMonDialog *StrMonDialog;
//---------------------------------------------------------------------------
#endif
