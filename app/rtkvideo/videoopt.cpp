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
    UpdateProf();
    
    UnicodeString str;
    SelProf      ->ItemIndex = MainForm->Profile;
    SelCapPos    ->ItemIndex = MainForm->CaptionPos;
    EditCapSize  ->Text      = str.sprintf(L"%d", MainForm->CaptionSize);
    SelCapColor  ->Color     = MainForm->CaptionColor;
    EditCodecQuality->Text   = str.sprintf(L"%d", MainForm->CodecQuality);
    ChkTcpPort   ->IsChecked = MainForm->TcpPortEna;
    EditTcpPort  ->Text      = str.sprintf(L"%d", MainForm->TcpPortNo);
    ChkOutFile   ->IsChecked = MainForm->OutFileEna;
    EditFile     ->Text      = MainForm->OutFile;
    ChkTimeTag   ->IsChecked = MainForm->OutTimeTag;
    SelFileSwap  ->ItemIndex = MainForm->FileSwap;
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
    MainForm->Profile      = SelProf    ->ItemIndex;
    MainForm->CaptionPos   = SelCapPos  ->ItemIndex;
    MainForm->CaptionSize  = EditCapSize->Text.ToInt();
    MainForm->CaptionColor = SelCapColor->Color;
    MainForm->CodecQuality = EditCodecQuality->Text.ToInt();
    MainForm->TcpPortEna   = ChkTcpPort ->IsChecked;
    MainForm->TcpPortNo    = EditTcpPort->Text.ToInt();
    MainForm->OutFileEna   = ChkOutFile ->IsChecked;
    MainForm->OutFile      = EditFile   ->Text;
    MainForm->OutTimeTag   = ChkTimeTag ->IsChecked;
    MainForm->FileSwap     = SelFileSwap->ItemIndex;
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
void __fastcall TVideoOptDlg::SelCapPosChange(TObject *Sender)
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
        str.sprintf(L"%d x %d, %2.0f FPS (%.0f-%.0f FPS)", settings[i].Width,
                    settings[i].Height, settings[i].FrameRate,
                    settings[i].MinFrameRate, settings[i].MaxFrameRate);
        SelProf->Items->Add(str);
    }
    if (settings.Length > 0) {
        SelProf->ItemIndex = 0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TVideoOptDlg::UpdateEnable(void)
{
    EditTcpPort  ->Enabled = ChkTcpPort->IsChecked;
    EditFile     ->Enabled = ChkOutFile->IsChecked;
    Label8       ->Enabled = SelCapPos->ItemIndex != 0;
    EditCapSize  ->Enabled = SelCapPos->ItemIndex != 0;
    SelCapColor  ->Enabled = SelCapPos->ItemIndex != 0;
    BtnFile      ->Enabled = ChkOutFile->IsChecked;
    ChkTimeTag   ->Enabled = ChkOutFile->IsChecked;
    SelFileSwap  ->Enabled = ChkOutFile->IsChecked;
    Label4       ->Enabled = ChkOutFile->IsChecked;
    Label5       ->Enabled = ChkOutFile->IsChecked;
}
//---------------------------------------------------------------------------

