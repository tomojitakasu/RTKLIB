//---------------------------------------------------------------------------
// rtkvideo : video capture
//
//			Copyright (C) 2016 by T.TAKASU, All rights reserved.
//
// version : $Revision:$ $Date:$
// history : 2016/09/25  1.0  new
//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "videomain.h"
#include "videoopt.h"

#define PRGNAME		"RTKVIDEO"
#define INI_FILE	"rtkvideo.ini"
#define TRACE_FILE	"rtkvideo.trace"

#define MIN_WINDOW_WIDTH  320
#define MIN_WINDOW_HEIGHT 240
#define STR_BUF_SIZE 4096000
#define DEFAULT_TCP_PORT 10033
#define DEFAULT_CAP_WIDTH 500
#define DEFAULT_CAP_HEIGHT 500
#define DEFAULT_CODEC_QUALITY 90

#define MIN(x,y)	((x)<(y)?(x):(y))

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TMainForm *MainForm;

//---------------------------------------------------------------------------
__fastcall TStreamWriterThread::TStreamWriterThread(TMainForm *parent, TBitmap *bitmap)
{
	Parent = parent;
	Bitmap = bitmap;
}
//---------------------------------------------------------------------------
__fastcall TStreamWriterThread::~TStreamWriterThread()
{
}
//---------------------------------------------------------------------------
void __fastcall TStreamWriterThread::Execute(void)
{
	TBitmapCodecManager *codec = new TBitmapCodecManager();
	TBitmapSurface *surf = new TBitmapSurface;
	TMemoryStream *buff = new TMemoryStream;
	TBitmapCodecSaveParams *params = new TBitmapCodecSaveParams;
	
	params->Quality = Parent->CodecQuality;
	
	while (!Terminated) {
		
		lock(&Parent->Lock); ////
		
		if (Parent->FrameCount > Parent->SentFrameCount) {
			surf->Assign(Bitmap);
			Parent->SentFrameCount = Parent->FrameCount;
			
			unlock(&Parent->Lock); ////
			
			buff->Clear();
			codec->SaveToStream(buff, surf, L".jpg", params);
			
			for (int i = 0; i < 2; i++) {
				strwrite(Parent->OutputStream+i, (unsigned char *)buff->Memory,
						 buff->Size);
			}
		}
		else {
			unlock(&Parent->Lock); ////
		}
		sleepms(15);
	}
	delete codec;
	delete surf;
	delete buff;
	delete params;
}
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
	Device = NULL;
	StartTime.time = 0;
	CaptureTime.time = 0;
	StartTick = CaptureTick = 0;
	FrameWidth = FrameHeight = 0;
	FrameCount = FrameCount0 = SentFrameCount = DispFrameCount = TimerCount = 0;
	FrameRate = 0;
	CaptionPos = 2;
	CaptionSize = 24;
	CaptionColor = claRed;
	DevName = L"";
	OutFile = L"";
	Profile = CapSizeEna = 0;
	TcpPortEna = OutFileEna = OutTimeTag = FileSwap = 0;
	CapWidth = DEFAULT_CAP_WIDTH;
	CapHeight = DEFAULT_CAP_HEIGHT;
	TcpPortNo = DEFAULT_TCP_PORT;
	CodecQuality = DEFAULT_CODEC_QUALITY;
	initlock(&Lock);
#if 0
	traceopen(TRACE_FILE);
	tracelevel(2);
#endif
}
//---------------------------------------------------------------------------
__fastcall TMainForm::~TMainForm()
{
	traceclose();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
	static const int str_opt[8] = {0, 0, 0, STR_BUF_SIZE, 0};
	
	LoadOptions();
	
	TCaptureDeviceList list = 
		TCaptureDeviceManager::Current->GetDevicesByMediaType(TMediaType::Video);
	
	if (list->Count > 0) {
		DevName = list->Items[0]->Name;
	}
	strsetopt(str_opt);
	for (int i = 0; i < 2; i++) {
		strinit(OutputStream+i);
	}
	Caption = PRGNAME;
	Caption = Caption+" ver."+VER_RTKLIB+" "+PATCH_LEVEL;
	
	Timer1->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (Device && Device->State == TCaptureDeviceState::Capturing) {
		Device->StopCapture();
	}
	SaveOptions();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStartClick(TObject *Sender)
{
	if (!CaptureStart()) return;
	
	BtnStart->Enabled = false;
	BtnStop ->Enabled = true;
	BtnOpt	->Enabled = false;
	BtnExit ->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
	CaptureStop();
	
	BtnStart->Enabled = true;
	BtnStop ->Enabled = false;
	BtnOpt	->Enabled = true;
	BtnExit ->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOptClick(TObject *Sender)
{
	(void)VideoOptDlg->ShowModal();
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
	Ind1	->Position->Y = Panel2->Height-35;
	Ind2	->Position->Y = Panel2->Height-19;
	Message1->Position->Y = Panel2->Height-38;
	Message2->Position->Y = Panel2->Height-22;
	Message3->Position->Y = Panel2->Height-22;
	Message4->Position->Y = 8;
	Message5->Position->Y = 8;
}
//---------------------------------------------------------------------------
int __fastcall TMainForm::CaptureStart(void)
{
	static const char *path_swap[] = {
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
	FrameWidth	= settings[Profile].Width;
	FrameHeight = settings[Profile].Height;
	CapImage->Bitmap->SetSize(FrameWidth, FrameHeight);
	
	if (TcpPortEna) {
		sprintf(path, ":%d", TcpPortNo);
		if (!stropen(OutputStream, STR_TCPSVR, STR_MODE_RW, path)) {
			Message3->Text = "tcp port open error";
		}
	}
	if (OutFileEna && OutFile != "") {
		AnsiString outfile = OutFile;
		sprintf(path, "%s%s%s", outfile.c_str(), OutTimeTag ? "::T" : "",
				path_swap[FileSwap]);
		if (!stropen(OutputStream+1, STR_FILE, STR_MODE_W, path)) {
			Message3->Text = "file open error";
		}
	}
	StreamWriter = new TStreamWriterThread(this, CapImage->Bitmap);
	
	Device->OnSampleBufferReady = SampleBufferReady;
	Device->StartCapture();
	StartTime = utc2gpst(timeget());
	StartTick = tickget();
	
	return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CaptureStop(void)
{
	if (!Device) return;
	
	Device->StopCapture();
	
	sleepms(50);
	
	StreamWriter->Terminate();
	StreamWriter->WaitFor();
	
	for (int i = 0; i < 2; i++) {
		strclose(OutputStream+i);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SampleBufferReady(TObject *Sender, const TMediaTime ATime)
{
	lock(&Lock); ////
	
	CaptureTime = utc2gpst(timeget());
	CaptureTick = tickget();
	FrameCount++;
	Device->SampleBufferToBitmap(CapImage->Bitmap, true);
	if (CaptionPos) {
		DrawCaption(CapImage->Bitmap);
	}
	unlock(&Lock); ////
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DrawCaption(TBitmap *bitmap)
{
	static const TTextAlign aligns[] = {
		TTextAlign::Leading, TTextAlign::Center, TTextAlign::Trailing
	};
	TCanvas *c = bitmap->Canvas;
	AnsiString str;
	
	str.sprintf("%s T=%.3f s TICK=%u FRM=%d", time_str(CaptureTime,3),
				(CaptureTick-StartTick)*1e-3, CaptureTick, FrameCount);
	
	c->BeginScene();
	c->Fill->Kind = TBrushKind::Solid;
	c->Fill->Color = CaptionColor;
	c->Font->Size = CaptionSize;
	c->FillText(TRect(8, 0, bitmap->Width-8, bitmap->Height-4), str, true,
		1.0f, TFillTextFlags(), aligns[CaptionPos-1], TTextAlign::Trailing);
	c->EndScene();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
	static const TAlphaColor color[] = {
		claRed, 0xFF404040, claOrange, claGreen, claLime
	};
	gtime_t capture_time;
	unsigned int capture_tick;
	char msg[1024], str_msg[4096] = "", *p, *q;
	int outb = 0, outr = 0, frame_count;
	
	lock(&Lock); ////
	
	frame_count = SentFrameCount;
	capture_time = CaptureTime;
	capture_tick = CaptureTick;
	if (SentFrameCount > DispFrameCount) {
		DispImage->Bitmap->Assign(CapImage->Bitmap);
		DispFrameCount = SentFrameCount;
	}
	unlock(&Lock); ////
	
	if (++TimerCount*Timer1->Interval >= 1000) {
		FrameRate = frame_count-FrameCount0;
		FrameCount0 = frame_count;
		TimerCount = 0;
	}
	Ind1->Fill->Color = color[strstat(OutputStream, str_msg)+1];
	strsum(OutputStream, NULL, NULL, NULL, &outr);
	sprintf(msg, "%4.1fMbps %s", outr/1e6, str_msg);
	Message1->Text = msg;
	
	Ind2->Fill->Color = color[strstat(OutputStream+1, str_msg)+1];
	strsum(OutputStream+1, NULL, NULL, &outb, NULL);
	sprintf(msg, "%4.1fMB", outb/1e6);
	strstatx(OutputStream+1, str_msg);
	if ((p = strstr(str_msg, "openpath=")) && (q = strchr(p+9, '\n'))) {
		*q = '\0';
		strcat(msg, p+9);
	}
	Message2->Text = msg;
	
	sprintf(msg, "%d x %d FRM=%d FPS=%2d", FrameWidth, FrameHeight,
		   frame_count, FrameRate);
	Message3->Text = msg;
	
	time2str(utc2gpst(timeget()), msg, 0);
	Message4->Text = msg;
	
	sprintf(msg, "T=%.3f s TICK=%u", (capture_tick-StartTick)*1e-3,
            capture_tick);
	Message5->Text = msg;
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
			if		(sscanf(buff, "captionpos = %d",   &val)) CaptionPos  = val;
			else if (sscanf(buff, "captionsize = %d",  &val)) CaptionSize = val;
			else if (sscanf(buff, "captioncolor = %d", &val)) CaptionColor = val;
			else if (sscanf(buff, "tcpportena = %d",   &val)) TcpPortEna  = val;
			else if (sscanf(buff, "tcpportno = %d",    &val)) TcpPortNo   = val;
			else if (sscanf(buff, "outfileena = %d",   &val)) OutFileEna  = val;
			else if (sscanf(buff, "outfile = %1023[^\r\n]", str)) OutFile = str;
			else if (sscanf(buff, "outtimetag = %d",   &val)) OutTimeTag  = val;
			else if (sscanf(buff, "fileswap = %d",	   &val)) FileSwap    = val;
			else if (sscanf(buff, "codecquality = %d\n", &val)) CodecQuality = val;
		}
		else if (!strcmp(sec, "window")) {
			if		(sscanf(buff, "win_width = %d",    &val)) Width	     = val;
			else if (sscanf(buff, "win_height = %d",   &val)) Height     = val;
			else if (sscanf(buff, "win_left = %d",	   &val)) Left	     = val;
			else if (sscanf(buff, "win_top = %d",	   &val)) Top	     = val;
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
	fprintf(fp, "captionpos = %d\n",   CaptionPos );
	fprintf(fp, "captionsize = %d\n",  CaptionSize);
	fprintf(fp, "captioncolor = %d\n", CaptionColor);
	fprintf(fp, "capheight = %d\n",    CapHeight  );
	fprintf(fp, "tcpportena = %d\n",   TcpPortEna );
	fprintf(fp, "tcpportno = %d\n",    TcpPortNo  );
	fprintf(fp, "outfileena = %d\n",   OutFileEna );
	fprintf(fp, "outfile = %s\n",  outfile.c_str());
	fprintf(fp, "outtimetag = %d\n",   OutTimeTag );
	fprintf(fp, "fileswap = %d\n",	   FileSwap   );
	fprintf(fp, "codecquality = %d\n", CodecQuality);
	fprintf(fp, "[window]\n");
	fprintf(fp, "win_width = %d\n",    Width      );
	fprintf(fp, "win_height = %d\n",   Height     );
	fprintf(fp, "win_left = %d\n",	   Left	      );
	fprintf(fp, "win_top = %d\n",	   Top	      );
	
	fclose(fp);
}
//---------------------------------------------------------------------------

