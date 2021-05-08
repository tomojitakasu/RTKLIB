//---------------------------------------------------------------------------
#ifndef fileseldlgH
#define fileseldlgH

//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <FileCtrl.hpp>
#include <ComCtrls.hpp>
#include <Buttons.hpp>

//---------------------------------------------------------------------------
class TFileSelDialog : public TForm
{
__published:
	TPanel *Panel1;
	TDriveComboBox *DriveSel;
	TPanel *Panel2;
	TDirectoryListBox *DirSel;
	TFileListBox *FileList;
	TPanel *Panel3;
	TFilterComboBox *Filter;
	TLabel *DirLabel;
	TBitBtn *BtnDirSel;
	TPanel *Panel4;
	TPanel *Panel5;
	TSpeedButton *BtnUpdate;
	void __fastcall FileListClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall DirSelChange(TObject *Sender);
	void __fastcall BtnDirSelClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall DriveSelClick(TObject *Sender);
	void __fastcall DirLabelClick(TObject *Sender);
	void __fastcall FilterClick(TObject *Sender);
	void __fastcall Panel4Click(TObject *Sender);
	void __fastcall FileListMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall BtnUpdateClick(TObject *Sender);
private:
public:
	AnsiString Dir;
	__fastcall TFileSelDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFileSelDialog *FileSelDialog;
//---------------------------------------------------------------------------
#endif
