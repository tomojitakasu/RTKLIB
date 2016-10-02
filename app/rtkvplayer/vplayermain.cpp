//---------------------------------------------------------------------------
// vplayermain.c : simple video player
//
//          Copyright (C) 2016 by T.TAKASU, All rights reserved.
//
// version : $Revision:$ $Date:$
// history : 2016/09/25 1.0 new
//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "vplayermain.h"
#include "rtklib.h"

#define VIDEO_TYPE_NONE  0
#define VIDEO_TYPE_MEDIA 1
#define VIDEO_TYPE_MJPEG 2

#define MIN_WINDOW_WIDTH  320
#define MIN_WINDOW_HEIGHT 240

#define PRGNAME     "RTKVPLAYER"
#define INI_FILE    "rtkvplayer.ini"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    VideoType = VIDEO_TYPE_NONE;
    FileName = "";
    Track = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    LoadOptions();
    
    Fmx::Platform::Win::TWinWindowHandle *handle =
        Fmx::Platform::Win::WindowHandleToPlatform(Handle);
    ::DragAcceptFiles(handle->Wnd, true);
    
    Caption = PRGNAME;
    Caption = Caption+" ver."+VER_RTKLIB+" "+PATCH_LEVEL;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    StopVideo();
    ClearVideo();
    
    SaveOptions();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOpenClick(TObject *Sender)
{
    if (OpenDialog->Execute()) OpenVideo(OpenDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnPlayClick(TObject *Sender)
{
    if (!PlayVideo()) return;
    BtnOpen ->Enabled = false;
    BtnPlay ->Enabled = false;
    BtnStop ->Enabled = true;
    BtnClear->Enabled = false;
    BtnExit ->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
    StopVideo();
    BtnOpen ->Enabled = true;
    BtnPlay ->Enabled = true;
    BtnStop ->Enabled = false;
    BtnClear->Enabled = true;
    BtnExit ->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnClearClick(TObject *Sender)
{
    ClearVideo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DropFiles(TWMDropFiles msg)
{
    wchar_t str[1024];
    
    if (DragQueryFile((HDROP)msg.Drop, 0xFFFFFFFF, NULL, 0) <= 0) return;
    DragQueryFile((HDROP)msg.Drop, 0, str, sizeof(file));
    UnicodeString file = str;
    OpenVideo(file);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
    UnicodeString str;
    double time, period;
    int width, height;
    
    GetVideoTime(time, period);
    GetVideoSize(width, height);
    if (FileName != "") {
        str.sprintf(L"%.1f / %.1f s (%d x %d)", time, period, width, height);
    }
    ProgressBar->Value = (int)(GetVideoPos()*1000);
    Message1->Text = str;
    Message2->Text = FileName;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProgressBarMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, float X, float Y)
{
    SetVideoPos(X/ProgressBar->Width);
    Track = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProgressBarMouseMove(TObject *Sender, TShiftState Shift,
          float X, float Y)
{
    if (Track) SetVideoPos(X/ProgressBar->Width);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProgressBarMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, float X, float Y)
{
    if (Track) SetVideoPos(X/ProgressBar->Width);
    Track = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject *Sender)
{
    if (Width  < MIN_WINDOW_WIDTH ) Width  = MIN_WINDOW_WIDTH;
    if (Height < MIN_WINDOW_HEIGHT) Height = MIN_WINDOW_HEIGHT;
    Panel3->ItemWidth = (Panel3->Width-2)/5;
    UpdateVideo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnPosStartClick(TObject *Sender)
{
    SetVideoPos(0.0f);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OpenVideo(UnicodeString file)
{
    if (MjpgPlayer->OpenVideo(file)) {
        FileName = file;
        VideoType = VIDEO_TYPE_MJPEG;
    }
    else {
        try {
            MediaPlayer->FileName = file;
        }
        catch (Exception &ex) {
            return;
        }
        VideoType = VIDEO_TYPE_MEDIA;
    }
    FileName = file;
    SetVideoPos(0.0f);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ClearVideo(void)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        MediaPlayer->Clear();
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        MjpgPlayer->ClearVideo();
    }
    FileName = "";
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::PlayVideo(void)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        MediaPlayer->Play();
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        MjpgPlayer->PlayVideo();
    }
    else {
        return 0;
    }
    return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::StopVideo(void)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        MediaPlayer->Stop();
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        MjpgPlayer->StopVideo();
    }
}
//---------------------------------------------------------------------------
float __fastcall TMainForm::GetVideoPos(void)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        return MediaPlayer->Duration == 0 ? 0.0f :
               (float)MediaPlayer->CurrentTime/MediaPlayer->Duration;
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        return MjpgPlayer->GetVideoPos();
    }
    return 0.0f;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SetVideoPos(float pos)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        MediaPlayer->CurrentTime = (TMediaTime)(MediaPlayer->Duration*pos);
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        MjpgPlayer->SetVideoPos(pos);
    }
    UpdateVideo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::GetVideoTime(double &time, double &period)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        time = MediaPlayer->CurrentTime*1e-7;
        period = MediaPlayer->Duration*1e-7;
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        MjpgPlayer->GetVideoTime(time, period);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::GetVideoSize(int &width, int &height)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        width  = (int)MediaPlayer->VideoSize.X;
        height = (int)MediaPlayer->VideoSize.Y;
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        MjpgPlayer->GetVideoSize(width, height);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateVideo(void)
{
    if (VideoType == VIDEO_TYPE_MEDIA) {
        TMediaState state = MediaPlayer->State;
        
        if (MediaPlayer->CurrentTime == MediaPlayer->Duration) {
            MediaPlayer->CurrentTime = MediaPlayer->Duration-1000000;
            MediaPlayer->Play();
            MediaPlayer->Stop();
        }
        else if (state == TMediaState::Stopped) {
            MediaPlayer->Play();
            MediaPlayer->Stop();
        }
    }
    else if (VideoType == VIDEO_TYPE_MJPEG) {
        MjpgPlayer->UpdateVideo();
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadOptions(void)
{
    FILE *fp;
    char buff[1080], sec[32] = "";
    int val;
    
    if (!(fp = fopen(INI_FILE, "r"))) return;
    
    while (fgets(buff, sizeof(buff), fp)) {
        if (*buff == '[') {
            sscanf(buff+1, "%31[^\]]", sec);
        }
        else if (!strcmp(sec, "window")) {
            if      (sscanf(buff, "win_width = %d",  &val)) Width      = val;
            else if (sscanf(buff, "win_height = %d", &val)) Height     = val;
            else if (sscanf(buff, "win_left = %d",   &val)) Left       = val;
            else if (sscanf(buff, "win_top = %d",    &val)) Top        = val;
        }
    }
    fclose(fp);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SaveOptions(void)
{
    FILE *fp;
    
    if (!(fp = fopen(INI_FILE, "w"))) return;
    
    fprintf(fp, "[window]\n");
    fprintf(fp, "win_width = %d\n",  Width     );
    fprintf(fp, "win_height = %d\n", Height    );
    fprintf(fp, "win_left = %d\n",   Left      );
    fprintf(fp, "win_top = %d\n",    Top       );
    
    fclose(fp);
}
//---------------------------------------------------------------------------




