//---------------------------------------------------------------------------

#ifndef vpoptdlgH
#define vpoptdlgH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Edit.hpp>
//---------------------------------------------------------------------------
class TVideoPlayerOptDialog : public TForm
{
__published:	// IDE で管理されるコンポーネント
	TButton *BtnOk;
	TButton *BtnCancel;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TEdit *EditMjpgRate;
	TEdit *EditSyncAddr;
	TEdit *EditSyncPort;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TVideoPlayerOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TVideoPlayerOptDialog *VideoPlayerOptDialog;
//---------------------------------------------------------------------------
#endif
