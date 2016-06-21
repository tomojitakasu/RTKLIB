//---------------------------------------------------------------------------

#ifndef markdlgH
#define markdlgH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TMarkDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TLabel *Label2;
	TRadioButton *RadioStop;
	TRadioButton *RadioGo;
	TLabel *LabelPosMode;
	TComboBox *MarkerName;
	TCheckBox *ChkMarkerName;
	TEdit *CommentText;
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall ChkMarkerNameClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	UnicodeString Marker,Comment;
	int PosMode;
	
	__fastcall TMarkDialog(TComponent* Owner);
	
};
//---------------------------------------------------------------------------
extern PACKAGE TMarkDialog *MarkDialog;
//---------------------------------------------------------------------------
#endif
