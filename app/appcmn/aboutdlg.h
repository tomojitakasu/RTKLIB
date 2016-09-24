//---------------------------------------------------------------------------
#ifndef aboutdlgH
#define aboutdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TAboutDialog : public TForm
{
__published:
	TLabel *LabelVer;
	TLabel *LabelAbout;
	TLabel *LabelCopyright;
	TImage *Icon4;
	TImage *Icon1;
	TImage *Icon2;
	TImage *Icon3;
	TImage *Icon5;
	TImage *Icon6;
	TImage *Icon7;
	TPanel *Panel1;
	TImage *Icon8;
	TSpeedButton *BtnOk;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
private:
public:
	int IconIndex;
	AnsiString About;
	__fastcall TAboutDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutDialog *AboutDialog;
//---------------------------------------------------------------------------
#endif
