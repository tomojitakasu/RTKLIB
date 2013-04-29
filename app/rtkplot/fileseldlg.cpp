//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "plotmain.h"
#include "fileseldlg.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TFileSelDialog *FileSelDialog;

//---------------------------------------------------------------------------
__fastcall TFileSelDialog::TFileSelDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::FormShow(TObject *Sender)
{
	DirSel->Directory=Dir;
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::FormResize(TObject *Sender)
{
	Panel5->Width=Width-16;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::DriveSelClick(TObject *Sender)
{
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::DirLabelClick(TObject *Sender)
{
	Panel5->Visible=!Panel5->Visible;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::Panel4Click(TObject *Sender)
{
	Panel5->Visible=!Panel5->Visible;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::BtnDirSelClick(TObject *Sender)
{
	Panel5->Visible=!Panel5->Visible;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::DirSelChange(TObject *Sender)
{
	Dir=DirSel->Directory;
	Panel5->Height=DirSel->Count*DirSel->ItemHeight+8;
	if (Panel5->Height>312) Panel5->Height=312;
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::FileListClick(TObject *Sender)
{
	TStringList *file=new TStringList;
	file->Add(FileList->FileName);
	Plot->ReadSol(file,0);
	delete file;
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::FileListMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::FilterClick(TObject *Sender)
{
	Panel5->Visible=false;
}
//---------------------------------------------------------------------------
void __fastcall TFileSelDialog::BtnUpdateClick(TObject *Sender)
{
	FileList->Update();
}
//---------------------------------------------------------------------------

