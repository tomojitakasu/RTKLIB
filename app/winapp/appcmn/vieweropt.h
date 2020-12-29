//---------------------------------------------------------------------------

#ifndef vieweroptH
#define vieweroptH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TViewerOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TLabel *Label6;
	TPanel *Color1;
	TButton *BtnColor1;
	TLabel *Label1;
	TPanel *Color2;
	TButton *BtnColor2;
	TLabel *Label15;
	TLabel *FontLabel;
	TButton *BtnFont;
	TColorDialog *ColorDialog;
	TFontDialog *FontDialog;
	void __fastcall BtnColor1Click(TObject *Sender);
	void __fastcall BtnColor2Click(TObject *Sender);
	void __fastcall BtnFontClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:
public:
	__fastcall TViewerOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TViewerOptDialog *ViewerOptDialog;
//---------------------------------------------------------------------------
#endif
