//---------------------------------------------------------------------------
#ifndef fileoptdlgH
#define fileoptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TFileOptDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOk;
	TSaveDialog *SaveDialog;
	TPanel *Panel1;
	TButton *BtnFilePath;
	TEdit *FilePath;
	TLabel *Label1;
	TCheckBox *ChkTimeTag;
	TComboBox *TimeSpeed;
	TEdit *TimeStart;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TComboBox *SwapIntv;
	TLabel *Label5;
	TSpeedButton *BtnKey;
	TOpenDialog *OpenDialog;
	TCheckBox *Chk64Bit;
	void __fastcall BtnFilePathClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ChkTimeTagClick(TObject *Sender);
	void __fastcall BtnKeyClick(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	int Opt;
	AnsiString Path;
	__fastcall TFileOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFileOptDialog *FileOptDialog;
//---------------------------------------------------------------------------
#endif
