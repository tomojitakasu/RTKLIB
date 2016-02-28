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
    void FileListClick();
    void FormShow();
    void DirSelChange();
    void BtnDirSelClick();
    void FormResize();
    void DriveSelClick();
    void DirLabelClick();
    void FilterClick();
    void Panel4Click();
    void FileListMouseDown(, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void BtnUpdateClick();
private:
public:
	AnsiString Dir;
    TFileSelDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFileSelDialog *FileSelDialog;
//---------------------------------------------------------------------------
#endif
