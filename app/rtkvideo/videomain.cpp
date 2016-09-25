//---------------------------------------------------------------------------
// rtkvideo : video capture
//
//          Copyright (C) 2016 by T.TAKASU, All rights reserved.
//
// version : $Revision:$ $Date:$
// history : 2016/09/25 1.0 new
//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "videomain.h"
#include "videoopt.h"

#define DEFAULT_TCP_PORT 10033
#define DEFAULT_CAP_WIDTH 500
#define DEFAULT_CAP_HEIGHT 500

#define STR_BUF_SIZE 4096000

#define MIN_WINDOW_WIDTH  320
#define MIN_WINDOW_HEIGHT 240

#define PRGNAME     "RTKVIDEO"
#define INI_FILE    "rtkvideo.ini"

#define MIN(x,y)    ((x)<(y)?(x):(y))

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    Device = NULL;
    Codec = NULL;
    Surf = NULL;
    StartTime.time = 0;
    CaptureTime.time = 0;
    Video_Width = Video_Height = FRM = FRM0 = FPS = 0;
    DevName = L"";
    OutFile = L"";
    Profile = CapSizeEna = Annotation = 0;
    TcpPortEna = OutFileEna = OutTimeTag = FileSwap = 0;
    CapWidth = DEFAULT_CAP_WIDTH;
    CapHeight = DEFAULT_CAP_HEIGHT;
    TcpPortNo = DEFAULT_TCP_PORT;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    const int str_opt[8] = {0, 0, 0, STR_BUF_SIZE, 0};
    
    LoadOptions();
    
    TCaptureDeviceList list = 
        TCaptureDeviceManager::Current->GetDevicesByMediaType(TMediaType::Video);
    
    if (list->Count > 0) {
        DevName = list->Items[0]->Name;
    }
    Codec = new TBitmapCodecManager();
    Surf = new TBitmapSurface;
    
    strsetopt(str_opt);
    
    for (int i = 0; i < 2; i++) {
        strinit(OutStr+i);
    }
    initlock(&DeviceLock);
    
    Timer1->Enabled = true;
    
    Caption = PRGNAME;
    Caption = Caption+" ver."+VER_RTKLIB+" "+PATCH_LEVEL;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    if (Device && Device->State == TCaptureDeviceState::Capturing) {
        Device->StopCapture();
    }
    SaveOptions();
    
    delete Codec;
    delete Surf;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStartClick(TObject *Sender)
{
    if (!CaptureStart()) return;
    
    BtnStart->Enabled = false;
    BtnStop ->Enabled = true;
    BtnOpt  ->Enabled = false;
    BtnExit ->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
    CaptureStop();
    
    BtnStart->Enabled = true;
    BtnStop ->Enabled = false;
    BtnOpt  ->Enabled = true;
    BtnExit ->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOptClick(TObject *Sender)
{
    if (VideoOptDlg->ShowModal() != mrOk) return;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject *Sender)
{
    TButton *btn[] = {
        BtnStart, BtnStop, BtnOpt, BtnExit
    };
    if (Width  < MIN_WINDOW_WIDTH ) Width  = MIN_WINDOW_WIDTH;
    if (Height < MIN_WINDOW_HEIGHT) Height = MIN_WINDOW_HEIGHT;
    
    for (int i = 0; i < 4; i++) {
        btn[i]->Width = Panel1->Width/4-4;
        btn[i]->Position->X = i*Panel1->Width/4+2;
    }
    Message1->Position->X = 23;
    Message2->Position->X = 23;
    Message3->Position->X = Panel2->Width-Message3->Width-10;
    Message4->Position->X = 10;
    Message5->Position->X = Panel2->Width-Message5->Width-10;
    Ind1    ->Position->Y = Panel2->Height-35;
    Ind2    ->Position->Y = Panel2->Height-19;
    Message1->Position->Y = Panel2->Height-38;
    Message2->Position->Y = Panel2->Height-22;
    Message3->Position->Y = Panel2->Height-22;
    Message4->Position->Y = 8;
    Message5->Position->Y = 8;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::CaptureStart(void)
{
    const char *swap[] = {
        "", "::S=0.05", "::S=0.1", "::S=0.25", "::S=0.5", "::S=1", "::S=2"
    };
    char path[1024];
    int retry;
    
    Device = dynamic_cast<TVideoCaptureDevice *>
            (TCaptureDeviceManager::Current->GetDevicesByName(DevName));
    
    if (!Device) return 0;
    
    DynamicArray<TVideoCaptureSetting> settings =
        Device->GetAvailableCaptureSettings(NULL);
    
    if (Profile < 0 || Profile >= settings.Length) {
        return 0;
    }
    if (!Device->SetCaptureSetting(settings[Profile])) {
        return 0;
    }
    if (CapSizeEna) {
        Video_Width  = CapWidth;
        Video_Height = CapHeight;
    }
    else {
        Video_Width  = settings[Profile].Width;
        Video_Height = settings[Profile].Height;
    }
    Image->Width  = Video_Width;
    Image->Height = Video_Height;
    
    if (TcpPortEna) {
        sprintf(path, ":%d", TcpPortNo);
        if (!stropen(OutStr, STR_TCPSVR, STR_MODE_RW, path)) {
            Message3->Text = "tcp port open error";
        }
    }
    if (OutFileEna && OutFile != "") {
        AnsiString outfile = OutFile;
        sprintf(path, "%s%s%s", outfile.c_str(), OutTimeTag ? "::T" : "",
                swap[FileSwap]);
        if (!stropen(OutStr+1, STR_FILE, STR_MODE_W, path)) {
            Message3->Text = "file open error";
        }
    }
    lock(&DeviceLock);
    
    Device->OnSampleBufferReady = SampleBufferReady;
    
    for (retry = 0; retry < 3; retry++) {
        Device->StartCapture();
        if (Device->State == TCaptureDeviceState::Capturing) break;
        sleepms(100);
    }
    unlock(&DeviceLock);
    
    if (retry >= 3) {
        strclose(OutStr);
        strclose(OutStr+1);
        return 0;
    }
    StartTime = utc2gpst(timeget());
    return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CaptureStop(void)
{
    if (!Device) return;
    
    lock(&DeviceLock);
    
    if (Device->State != TCaptureDeviceState::Stopped) {
        Device->StopCapture();
    }
    unlock(&DeviceLock);
    
    for (int i = 0; i < 2; i++) {
        strclose(OutStr+i);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SampleBufferReady(TObject *Sender, const TMediaTime ATime)
{
    TThread::Synchronize(TThread::CurrentThread, SampleBufferSync);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SampleBufferSync(void)
{
    UnicodeString str;
    
    if (!Device || Device->State == TCaptureDeviceState::Stopped) {
        return;
    }
    lock(&DeviceLock);
    
    CaptureTime = utc2gpst(timeget());
    FRM++;
    Device->SampleBufferToBitmap(Image->Bitmap, true);
    
    unlock(&DeviceLock);
    
    // show image in monitor
    Disp->Bitmap->Assign(Image->Bitmap);
    
    if (TcpPortEna || OutFileEna) {
        
        if (Annotation) {
            str.sprintf(L"%s FRM=%d",time_str(CaptureTime,1), FRM);
            DrawText(Image->Bitmap->Canvas, 10, 10, str, 12, claYellow, 1, 1);
        }
        TMemoryStream *buff = new TMemoryStream;
        
        // convert image to jpeg
        Surf->Assign(Image->Bitmap);
        Codec->SaveToStream(buff, Surf, L".jpg", 0);
        
        for (int i = 0; i< 2; i++) {
            strwrite(OutStr+i, (unsigned char *)buff->Memory, buff->Size);
        }
        delete buff;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DrawText(TCanvas *c, int x, int y,
    UnicodeString str, int size, TAlphaColor color, int ha, int va)
{
    TTextAlign halign = ha == 0 ? TTextAlign::Center : (ha == 1 ? TTextAlign::Leading : TTextAlign::Trailing);
    TTextAlign valign = va == 0 ? TTextAlign::Center : (va == 1 ? TTextAlign::Leading : TTextAlign::Trailing);
    TRectF rect(x, y, x+100, y+30);
    TFillTextFlags flags;
    c->Fill->Kind = TBrushKind::None;
    c->Stroke->Color = color;
    c->Font->Size = size;
    c->FillText(rect, str, true, 1.0f, flags, valign, halign);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
    static int Timer_Count = 0;
    
    TAlphaColor color[]={
        claRed, 0xFF404040, claOrange, claGreen, claLime
    };
    char msg[1024], str_msg[4096] = "", *p, *q;
    int outb = 0, outr = 0;
    
    lock(&DeviceLock);
    
    Ind1->Fill->Color = color[strstat(OutStr, str_msg)+1];
    strsum(OutStr, NULL, NULL, NULL, &outr);
    sprintf(msg, "%.1fMbps %s", outr/1e6, str_msg);
    Message1->Text = msg;
    
    Ind2->Fill->Color = color[strstat(OutStr+1, str_msg)+1];
    strsum(OutStr+1, NULL, NULL, &outb, NULL);
    sprintf(msg, "%.1fMB", outb/1e6);
    strstatx(OutStr+1, str_msg);
    if ((p = strstr(str_msg, "openpath=")) && (q = strchr(p+9, '\n'))) {
        *q = '\0';
        strcat(msg, p+9);
    }
    Message2->Text = msg;
    
    if (++Timer_Count*Timer1->Interval >= 1000) {
        FPS = FRM-FRM0;
        FRM0 = FRM;
        Timer_Count = 0;
    }
    sprintf(msg, "%d x %d FRM=%d FPS=%2d", Video_Width, Video_Height,
            FRM, FPS);
    Message3->Text = msg;
    
    time2str(utc2gpst(timeget()), msg, 1);
    Message4->Text = msg;
    
    sprintf(msg, "TIME = %.1f s",
            !StartTime.time ? 0.0 : timediff(CaptureTime, StartTime));
    Message5->Text = msg;
    
    unlock(&DeviceLock);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadOptions(void)
{
    FILE *fp;
    char buff[1080], sec[32] = "", str[1024] = "";
    int val;
    
    if (!(fp = fopen(INI_FILE, "r"))) return;
    
    while (fgets(buff, sizeof(buff), fp)) {
        if (*buff == '[') {
            sscanf(buff+1, "%31[^\]]", sec);
        }
        else if (!strcmp(sec, "videoopt")) {
            if      (sscanf(buff, "capsizeena = %d", &val)) CapSizeEna = val;
            else if (sscanf(buff, "capwidth = %d",   &val)) CapWidth   = val;
            else if (sscanf(buff, "capheight = %d",  &val)) CapHeight  = val;
            else if (sscanf(buff, "tcpportena = %d", &val)) TcpPortEna = val;
            else if (sscanf(buff, "tcpportno = %d",  &val)) TcpPortNo  = val;
            else if (sscanf(buff, "outfileena = %d", &val)) OutFileEna = val;
            else if (sscanf(buff, "outfile = %1023[^\r\n]", str)) OutFile = str;
            else if (sscanf(buff, "outtimetag = %d", &val)) OutTimeTag = val;
            else if (sscanf(buff, "fileswap = %d",   &val)) FileSwap   = val;
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
    AnsiString outfile = OutFile;
    FILE *fp;
    
    if (!(fp = fopen(INI_FILE, "w"))) return;
    
    fprintf(fp, "[videoopt]\n");
    fprintf(fp, "capsizeena = %d\n", CapSizeEna);
    fprintf(fp, "capwidth = %d\n",   CapWidth  );
    fprintf(fp, "capheight = %d\n",  CapHeight );
    fprintf(fp, "tcpportena = %d\n", TcpPortEna);
    fprintf(fp, "tcpportno = %d\n",  TcpPortNo );
    fprintf(fp, "outfileena = %d\n", OutFileEna);
    fprintf(fp, "outfile = %s\n"   , outfile.c_str());
    fprintf(fp, "outtimetag = %d\n", OutTimeTag);
    fprintf(fp, "fileswap = %d\n",   FileSwap  );
    fprintf(fp, "[window]\n");
    fprintf(fp, "win_width = %d\n",  Width     );
    fprintf(fp, "win_height = %d\n", Height    );
    fprintf(fp, "win_left = %d\n",   Left      );
    fprintf(fp, "win_top = %d\n",    Top       );
    
    fclose(fp);
}
//---------------------------------------------------------------------------



