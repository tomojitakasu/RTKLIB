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
class TMainForm;
class TStreamWriterThread : public TThread
{
private:
	TMainForm *Parent;
	TBitmap *Bitmap;
	
	void __fastcall Execute(void);
public:
	__fastcall TStreamWriterThread(TMainForm *parent, TBitmap *bitmap);
	__fastcall ~TStreamWriterThread();
};
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TButton *BtnStart;
	TButton *BtnStop;
	TButton *BtnOpt;
	TButton *BtnExit;
	TLabel *Message1;
	TLabel *Message2;
	TImage *DispImage;
	TRectangle *Ind1;
	TTimer *Timer1;
	TLabel *Message3;
	TLayout *Panel1;
	TRectangle *Panel2;
	TLabel *Message4;
	TRectangle *Ind2;
	TLabel *Message5;
	TImage *CapImage;
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
	TStreamWriterThread *StreamWriter;
	int FrameWidth, FrameHeight, FrameCount0, FrameRate, TimerCount;
	
	int  __fastcall CaptureStart(void);
	void __fastcall CaptureStop(void);
	void __fastcall LoadOptions(void);
	void __fastcall SaveOptions(void);
	void __fastcall DrawCaption(TBitmap *bitmap);
	void __fastcall SampleBufferReady(TObject *Sender, const TMediaTime ATime);
	void __fastcall SampleBufferSync(void);
public:
	stream_t OutputStream[2];
	gtime_t StartTime, CaptureTime;
	unsigned int StartTick, CaptureTick;
	int FrameCount, SentFrameCount, DispFrameCount;
	lock_t Lock;
	UnicodeString DevName, OutFile;
	TAlphaColor CaptionColor;
	int Profile, CapSizeEna, CapWidth, CapHeight, CaptionPos, CaptionSize;
	int TcpPortEna, TcpPortNo, OutFileEna, OutTimeTag, FileSwap, CodecQuality;
	
	__fastcall TMainForm(TComponent* Owner);
	__fastcall ~TMainForm();
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
