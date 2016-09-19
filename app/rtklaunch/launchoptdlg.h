//---------------------------------------------------------------------------

#ifndef launchoptdlgH
#define launchoptdlgH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TLaunchOptDialog : public TForm
{
__published:	// IDE で管理されるコンポーネント
	TRadioButton *OptMkl;
	TRadioButton *OptWin64;
	TButton *BtnCancel;
	TButton *BtnOk;
	TRadioButton *OptNormal;
	TCheckBox *Minimize;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TLaunchOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLaunchOptDialog *LaunchOptDialog;
//---------------------------------------------------------------------------
#endif
