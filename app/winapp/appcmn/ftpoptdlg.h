//---------------------------------------------------------------------------
#ifndef ftpoptdlgH
#define ftpoptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>

#define MAXHIST		10

//---------------------------------------------------------------------------
class TFtpOptDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOk;
	TEdit *User;
	TEdit *Passwd;
	TLabel *LabelUser;
	TLabel *LabelPasswd;
	TComboBox *Addr;
	TLabel *Label1;
	TComboBox *Interval;
	TLabel *Label2;
	TLabel *Label3;
	TComboBox *Offset;
	TLabel *Label4;
	TEdit *RetryInterval;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Label7;
	TSpeedButton *BtnKey;
	TLabel *Label8;
	TComboBox *PathOffset;
	TLabel *Label9;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnKeyClick(TObject *Sender);
private:
	void __fastcall AddHist(TComboBox *list, AnsiString *hist);
	void __fastcall UpdateEnable(void);
public:
	int Opt;
	AnsiString Path,History[MAXHIST],MntpHist[MAXHIST];
	__fastcall TFtpOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFtpOptDialog *FtpOptDialog;
//---------------------------------------------------------------------------
#endif
