//---------------------------------------------------------------------------
#include <fmx.h>
#pragma hdrstop

#include "videomain.h"
#include "videoopt.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"

TVideoOptDlg *VideoOptDlg;

//---------------------------------------------------------------------------
__fastcall TVideoOptDlg::TVideoOptDlg(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::FormShow(TObject *Sender)
{
    TCaptureDeviceList list = 
        TCaptureDeviceManager::Current->GetDevicesByMediaType(TMediaType::Video);
    
    SelDev->Clear();
    
    for (int i = 0; i < list->Count; i++) {
        SelDev->Items->Add(list->Items[i]->Name);
        if (i == 0) {
            SelDev->ItemIndex = 0;
        }
        else if (list->Items[i]->Name == MainForm->DevName) {
            SelDev->ItemIndex = i;
        }
    }
    UnicodeString str;
    SelProf      ->ItemIndex = MainForm->Profile;
    ChkCapSize   ->IsChecked = MainForm->CapSizeEna;
    EditCapWidth ->Text      = str.sprintf(L"%d", MainForm->CapWidth);
    EditCapHeight->Text      = str.sprintf(L"%d", MainForm->CapHeight);
    ChkTcpPort   ->IsChecked = MainForm->TcpPortEna;
    EditTcpPort  ->Text      = str.sprintf(L"%d", MainForm->TcpPortNo);
    ChkOutFile   ->IsChecked = MainForm->OutFileEna;
    EditFile     ->Text      = MainForm->OutFile;
    ChkTimeTag   ->IsChecked = MainForm->OutTimeTag;
    SelFileSwap  ->ItemIndex = MainForm->FileSwap;
    UpdateProf();
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::BtnOkClick(TObject *Sender)
{
    if (SelDev->Selected) {
        MainForm->DevName = SelDev->Selected->Text;
    }
    else {
        MainForm->DevName = L"";
    }
    MainForm->Profile    = SelProf      ->ItemIndex;
    MainForm->CapSizeEna = ChkCapSize   ->IsChecked;
    MainForm->CapWidth   = EditCapWidth ->Text.ToInt();
    MainForm->CapHeight  = EditCapHeight->Text.ToInt();
    MainForm->TcpPortEna = ChkTcpPort   ->IsChecked;
    MainForm->TcpPortNo  = EditTcpPort  ->Text.ToInt();
    MainForm->OutFileEna = ChkOutFile   ->IsChecked;
    MainForm->OutFile    = EditFile     ->Text;
    MainForm->OutTimeTag = ChkTimeTag   ->IsChecked;
    MainForm->FileSwap   = SelFileSwap  ->ItemIndex;
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::BtnFileClick(TObject *Sender)
{
    SaveDialog->FileName = EditFile->Text;
    if (!SaveDialog->Execute()) return;
    EditFile->Text = SaveDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::SelDevChange(TObject *Sender)
{
    UpdateProf();
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::ChkTcpPortChange(TObject *Sender)
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::UpdateProf(void)
{
    if (!SelDev->Selected) {
        return;
    }
    TVideoCaptureDevice *device = dynamic_cast<TVideoCaptureDevice*>
        (TCaptureDeviceManager::Current->GetDevicesByName(SelDev->Selected->Text));
    
    if (!device) return;
    
    DynamicArray<TVideoCaptureSetting> settings =
        device->GetAvailableCaptureSettings(NULL);
    
    SelProf->Clear();
    
    for (int i = 0; i < settings.Length; i++) {
        UnicodeString str;
        str.sprintf(L"%d x %d, FPS=%.0f (%.0f-%.0f)", settings[i].Width,
                    settings[i].Height, settings[i].FrameRate,
                    settings[i].MinFrameRate, settings[i].MaxFrameRate);
        SelProf->Items->Add(str);
    }
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::UpdateEnable(void)
{
    EditCapWidth ->Enabled = ChkCapSize->IsChecked;
    EditCapHeight->Enabled = ChkCapSize->IsChecked;
    Label6       ->Enabled = ChkCapSize->IsChecked;
    EditTcpPort  ->Enabled = ChkTcpPort->IsChecked;
    EditFile     ->Enabled = ChkOutFile->IsChecked;
    BtnFile      ->Enabled = ChkOutFile->IsChecked;
    ChkTimeTag   ->Enabled = ChkOutFile->IsChecked;
    SelFileSwap  ->Enabled = ChkOutFile->IsChecked;
    Label4       ->Enabled = ChkOutFile->IsChecked;
    Label5       ->Enabled = ChkOutFile->IsChecked;
}
//---------------------------------------------------------------------------

