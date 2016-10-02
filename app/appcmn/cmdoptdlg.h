//---------------------------------------------------------------------------
#ifndef cmdoptdlgH
#define cmdoptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TCmdOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TMemo *OpenCmd;
	TMemo *CloseCmd;
	TCheckBox *ChkOpenCmd;
	TCheckBox *ChkCloseCmd;
	TButton *BtnLoad;
	TButton *BtnSave;
	TSaveDialog *SaveDialog;
	TOpenDialog *OpenDialog;
	TCheckBox *ChkPeriodicCmd;
	TMemo *PeriodicCmd;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall ChkCloseCmdClick(TObject *Sender);
	void __fastcall ChkOpenCmdClick(TObject *Sender);
	void __fastcall UpdateEnable(void);
	void __fastcall BtnLoadClick(TObject *Sender);
	void __fastcall BtnSaveClick(TObject *Sender);
private:
public:
	AnsiString Cmds[3];
	int CmdEna[3];
	__fastcall TCmdOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCmdOptDialog *CmdOptDialog;
//---------------------------------------------------------------------------
#endif
