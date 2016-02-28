//---------------------------------------------------------------------------

#include "plotmain.h"
#include "fileseldlg.h"


//---------------------------------------------------------------------------
 FileSelDialog::FileSelDialog(QWidget *parent)
    : QDialog(parent)
{
     setupUi(this);
}
//---------------------------------------------------------------------------
void  FileSelDialog::FormShow()
{
	DirSel->Directory=Dir;
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void  FileSelDialog::FormResize()
{
	Panel5->Width=Width-16;
}
//---------------------------------------------------------------------------
void  FileSelDialog::DriveSelClick()
{
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void  FileSelDialog::DirLabelClick()
{
	Panel5->Visible=!Panel5->Visible;
}
//---------------------------------------------------------------------------
void  FileSelDialog::Panel4Click()
{
	Panel5->Visible=!Panel5->Visible;
}
//---------------------------------------------------------------------------
void  FileSelDialog::BtnDirSelClick()
{
	Panel5->Visible=!Panel5->Visible;
}
//---------------------------------------------------------------------------
void  FileSelDialog::DirSelChange()
{
	Dir=DirSel->Directory;
	Panel5->Height=DirSel->Count*DirSel->ItemHeight+8;
	if (Panel5->Height>312) Panel5->Height=312;
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void  FileSelDialog::FileListClick()
{
	TStringList *file=new TStringList;
	file->Add(FileList->FileName);
	Plot->ReadSol(file,0);
	delete file;
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void  FileSelDialog::FileListMouseDown(,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void  FileSelDialog::FilterClick()
{
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void  FileSelDialog::BtnUpdateClick()
{
	FileList->Update();
}
//---------------------------------------------------------------------------

