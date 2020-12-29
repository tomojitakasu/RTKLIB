//---------------------------------------------------------------------------
#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "browsmain.h"
#include "staoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TStaListDialog *StaListDialog;
//---------------------------------------------------------------------------
__fastcall TStaListDialog::TStaListDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TStaListDialog::FormShow(TObject *Sender)
{
    StaList->Clear();
    
    for (int i=0;i<MainForm->StaList->Count;i++) {
        StaList->Lines->Add(MainForm->StaList->Strings[i]);
    }
}
//---------------------------------------------------------------------------
void __fastcall TStaListDialog::BtnOkClick(TObject *Sender)
{
    MainForm->StaList->Clear();
    
    for (int i=0;i<StaList->Lines->Count;i++) {
        MainForm->StaList->Add(StaList->Lines->Strings[i]);
    }
}
//---------------------------------------------------------------------------
void __fastcall TStaListDialog::BtnLoadClick(TObject *Sender)
{
    AnsiString file;
    FILE *fp;
    char buff[1024],*p;
    
    if (!OpenDialog->Execute()) return;
    
    file=OpenDialog->FileName;
    
    if (!(fp=fopen(file.c_str(),"r"))) return;
    
    StaList->Clear();
    StaList->Visible=false;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if ((p=strchr(buff,'#'))) *p='\0';
        for (p=strtok(buff," ,\r\n");p;p=strtok(NULL," ,\r\n")) {
            StaList->Lines->Add(p);
        }
    }
    fclose(fp);
    StaList->Visible=true;
}
//---------------------------------------------------------------------------
void __fastcall TStaListDialog::BtnSaveClick(TObject *Sender)
{
	if (!SaveDialog->Execute()) return;
	StaList->Lines->SaveToFile(SaveDialog->FileName);
}
//---------------------------------------------------------------------------

