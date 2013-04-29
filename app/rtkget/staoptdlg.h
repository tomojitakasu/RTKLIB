//---------------------------------------------------------------------------
#ifndef staoptdlgH
#define staoptdlgH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TStaListDialog : public TForm
{
__published:
	TPanel *Panel1;
	TButton *BtnLoad;
	TButton *BtnSave;
	TButton *BtnOk;
	TButton *BtnCancel;
	TMemo *StaList;
	TOpenDialog *OpenDialog;
	TSaveDialog *SaveDialog;
	void __fastcall BtnLoadClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnSaveClick(TObject *Sender);
private:
public:
	__fastcall TStaListDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TStaListDialog *StaListDialog;
//---------------------------------------------------------------------------
#endif
