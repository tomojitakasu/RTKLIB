//---------------------------------------------------------------------------
#ifndef vmainH
#define vmainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Media.hpp>
#include <FMX.Layouts.hpp>
#include "rtklib.h"

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
    TButton *BtnStart;
    TButton *BtnStop;
    TButton *BtnOpt;
    TButton *BtnExit;
    TImage *Disp;
    TLabel *Message1;
    TLabel *Message2;
    TImage *Image;
    TRectangle *Ind1;
    TTimer *Timer1;
    TLabel *Message3;
    TLayout *Panel1;
    TRectangle *Panel2;
    TLabel *Message4;
    TRectangle *Ind2;
    TLabel *Message5;
    void __fastcall BtnStopClick(TObject *Sender);
    void __fastcall BtnOptClick(TObject *Sender);
    void __fastcall BtnExitClick(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall BtnStartClick(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormShow(TObject *Sender);
private:
    TVideoCaptureDevice *Device;
    TBitmapCodecManager *Codec;
    TBitmapSurface *Surf;
    stream_t OutStr[2];
    gtime_t StartTime;
    gtime_t CaptureTime;
    lock_t DeviceLock;
    int Video_Width, Video_Height, FRM, FRM0, FPS;
    
    int  __fastcall CaptureStart(void);
    void __fastcall CaptureStop(void);
    void __fastcall LoadOptions(void);
    void __fastcall SaveOptions(void);
    void __fastcall DrawText(TCanvas *c, int x, int y,
        UnicodeString str, int size, TAlphaColor color, int ha, int va);
    void __fastcall SampleBufferReady(TObject *Sender, const TMediaTime ATime);
    void __fastcall SampleBufferSync(void);
public:
    UnicodeString DevName, OutFile;
    int Profile, CapSizeEna, CapWidth, CapHeight, Annotation;
    int TcpPortEna, TcpPortNo, OutFileEna, OutTimeTag, FileSwap;
    
    __fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
