//---------------------------------------------------------------------------
#ifndef logstrdlgH
#define logstrdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TLogStrDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOk;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Label10;
	TComboBox *Stream1;
	TComboBox *Stream2;
	TButton *BtnStr1;
	TButton *BtnStr2;
	TEdit *FilePath1;
	TEdit *FilePath2;
	TButton *BtnFile1;
	TButton *BtnFile2;
	TLabel *LabelF1;
	TSaveDialog *SaveDialog;
	TCheckBox *TimeTagC;
	TCheckBox *Stream1C;
	TCheckBox *Stream2C;
	TSpeedButton *BtnKey;
	TCheckBox *Stream3C;
	TComboBox *Stream3;
	TButton *BtnStr3;
	TEdit *FilePath3;
	TButton *BtnFile3;
	TComboBox *SwapIntv;
	TLabel *Label1;
	TLabel *Label2;
	TCheckBox *OutEventC;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Stream1Change(TObject *Sender);
	void __fastcall Stream2Change(TObject *Sender);
	void __fastcall BtnStr1Click(TObject *Sender);
	void __fastcall BtnStr2Click(TObject *Sender);
	void __fastcall BtnFile1Click(TObject *Sender);
	void __fastcall BtnFile2Click(TObject *Sender);
	void __fastcall Stream1CClick(TObject *Sender);
	void __fastcall Stream2CClick(TObject *Sender);
	void __fastcall BtnKeyClick(TObject *Sender);
	void __fastcall BtnStr3Click(TObject *Sender);
	void __fastcall BtnFile3Click(TObject *Sender);
	void __fastcall Stream3CClick(TObject *Sender);
	void __fastcall Stream3Change(TObject *Sender);
private:
	AnsiString __fastcall GetFilePath(AnsiString path);
	AnsiString __fastcall SetFilePath(AnsiString path);
	void __fastcall SerialOpt(int index, int opt);
	void __fastcall TcpOpt(int index, int opt);
	void __fastcall UpdateEnable(void);
public:
	int StreamC[3],Stream[3],LogTimeTag,LogAppend;
	AnsiString Paths[3][4],SwapInterval;
	AnsiString History[10],MntpHist[10];
	__fastcall TLogStrDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLogStrDialog *LogStrDialog;
//---------------------------------------------------------------------------
#endif
