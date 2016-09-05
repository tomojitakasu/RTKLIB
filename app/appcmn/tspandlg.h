//---------------------------------------------------------------------------
#ifndef tspandlgH
#define tspandlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TSpanDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOk;
	TCheckBox *TimeStartF;
	TCheckBox *TimeEndF;
	TCheckBox *TimeIntF;
	TEdit *TimeY2;
	TEdit *TimeY1;
	TUpDown *TimeY1UD;
	TUpDown *TimeY2UD;
	TEdit *TimeH2;
	TEdit *TimeH1;
	TUpDown *TimeH1UD;
	TUpDown *TimeH2UD;
	TSpeedButton *BtnTime2;
	TSpeedButton *BtnTime1;
	TComboBox *EditTimeInt;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall TimeStartFClick(TObject *Sender);
	void __fastcall TimeEndFClick(TObject *Sender);
	void __fastcall TimeIntFClick(TObject *Sender);
	void __fastcall TimeY1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeH1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeY2UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeH2UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnTime1Click(TObject *Sender);
	void __fastcall BtnTime2Click(TObject *Sender);
	void __fastcall TimeY1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall TimeH2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall TimeH1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall TimeY2KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
private:
	void __fastcall UpdateEnable(void);
public:
	int TimeEna[3],TimeVal[3];
	gtime_t TimeStart,TimeEnd;
	double TimeInt;
	__fastcall TSpanDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSpanDialog *SpanDialog;
//---------------------------------------------------------------------------
#endif
