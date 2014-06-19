//---------------------------------------------------------------------------
#ifndef cmdoptdlgH
#define cmdoptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <Vcl.Grids.hpp>
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
	TCheckBox *ChkPerCmd;
	TStringGrid *PerCmdTable;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall ChkCloseCmdClick(TObject *Sender);
	void __fastcall ChkOpenCmdClick(TObject *Sender);
	void __fastcall UpdateEnable(void);
	void __fastcall BtnLoadClick(TObject *Sender);
	void __fastcall BtnSaveClick(TObject *Sender);
	void __fastcall ChkPerCmdClick(TObject *Sender);
	void __fastcall PerCmdTableDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect,
          TGridDrawState State);
private:
public:
	AnsiString Cmds[2];
	int CmdEna[2];
	int PerCmdsEna;   					/* periodic commands enabled*/
#if MAXPERCMD > 0
	AnsiString PerCmds[MAXPERCMD];		/* periodic commands */
	int PerCmdsPeriods[MAXPERCMD]; 	/* periods of periodic commands */
#endif
	__fastcall TCmdOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TCmdOptDialog *CmdOptDialog;
//---------------------------------------------------------------------------
#endif
