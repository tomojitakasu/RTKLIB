//---------------------------------------------------------------------------
#ifndef getoptdlgH
#define getoptdlgH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TDownOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TPanel *Panel1;
	TEdit *Proxy;
	TEdit *UrlFile;
	TButton *BtnUrlFile;
	TLabel *Label1;
	TOpenDialog *OpenDialog;
	TLabel *Label2;
	TComboBox *TraceLevel;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *LogFile;
	TButton *BtnLogFile;
	TCheckBox *LogAppend;
	TSaveDialog *SaveDialog;
	TCheckBox *HoldErr;
	TCheckBox *HoldList;
	TLabel *Label5;
	TComboBox *DateFormat;
	TLabel *Label6;
	TEdit *NCol;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnUrlFileClick(TObject *Sender);
	void __fastcall BtnLogFileClick(TObject *Sender);
private:
public:
	__fastcall TDownOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDownOptDialog *DownOptDialog;
//---------------------------------------------------------------------------
#endif
