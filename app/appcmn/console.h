//---------------------------------------------------------------------------
#ifndef consoleH
#define consoleH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>

//---------------------------------------------------------------------------
class TConsole : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel2;
	TButton *BtnClose;
	TSpeedButton *BtnAsc;
	TSpeedButton *BtnHex;
	TSpeedButton *BtnClear;
	TPaintBox *Console;
	TScrollBar *Scroll;
	TSpeedButton *BtnDown;
	TSpeedButton *BtnStop;
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall BtnClearClick(TObject *Sender);
	void __fastcall ConsolePaint(TObject *Sender);
	void __fastcall ScrollChange(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall BtnAscClick(TObject *Sender);
	void __fastcall BtnHexClick(TObject *Sender);
	void __fastcall BtnDownClick(TObject *Sender);
	void __fastcall BtnStopClick(TObject *Sender);
private:
	TStringList *ConBuff;
	int Stop,ScrollPos;
public:
	__fastcall TConsole(TComponent* Owner);
	void __fastcall AddMsg(unsigned char *buff, int n);
};
//---------------------------------------------------------------------------
extern PACKAGE TConsole *Console;
//---------------------------------------------------------------------------
#endif
