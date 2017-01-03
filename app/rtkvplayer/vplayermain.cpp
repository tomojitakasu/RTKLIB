//---------------------------------------------------------------------------
// vplayermain.c : simple video player
//
//			Copyright (C) 2016 by T.TAKASU, All rights reserved.
//
// version : $Revision:$ $Date:$
// history : 2016/09/25 1.0 new
//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "vplayermain.h"
#include "vpoptdlg.h"
#include "rtklib.h"

#define VIDEO_TYPE_NONE  0
#define VIDEO_TYPE_MEDIA 1
#define VIDEO_TYPE_MJPEG 2

#define MIN_WINDOW_WIDTH  320
#define MIN_WINDOW_HEIGHT 240

#define PRGNAME		"RTKVPLAYER"
#define INI_FILE	"rtkvplayer.ini"
#define PATH_TIME_SYNC "localhost:10071"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
	double ep[]={2000,1,1,0,0,0};
	VideoType = VIDEO_TYPE_NONE;
	Files = new TStringList;
	FileIndex = -1;
	Track = NStrBuff = 0;
	TimeStart = epoch2time(ep);
	MjpgRate = 10.0;
	SyncPort = 10071;
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
	
	strinitcom();
	strinit(&StrTimeSync);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	StopVideo();
	ClearVideo();
	strclose(&StrTimeSync);
	SaveOptions();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnOpenClick(TObject *Sender)
{
	if (!OpenDialog->Execute()) return;
	Files = OpenDialog->Files;
	FileIndex = 0;
	OpenVideo(Files->Strings[FileIndex]);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ReadNmea(UnicodeString file)
{
	AnsiString ansi_file=file;
	FILE *fp;
	double time,date,ep[6];
	char path[1024],buff[256],*p;
	
	strcpy(path,ansi_file.c_str());
	
	if (!(p=strrchr(path,'.'))) p=path+strlen(path);
	strcpy(p,".nmea");
	
	if (!(fp=fopen(path,"r"))) return;
	
	while (fgets(buff,sizeof(buff),fp)) {
		if (sscanf(buff,"$GPRMC,%lf,A,%*lf,%*c,%*lf,%*c,%*lf,%*lf,%lf",
				   &time,&date)>=2) {
			ep[2]=floor(date/10000.0); date-=ep[2]*10000.0;
			ep[1]=floor(date/100.0  ); date-=ep[1]*100.0;
			ep[0]=date+2000.0;
			ep[3]=floor(time/10000.0); time-=ep[3]*10000.0;
			ep[4]=floor(time/100.0  ); time-=ep[4]*100.0;
			ep[5]=time;
			TimeStart=timeadd(utc2gpst(epoch2time(ep)),-0.5);
			break;
		}
	}
	fclose(fp);
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
void __fastcall TMainForm::BtnNextClick(TObject *Sender)
{
	NextVideo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnPrevClick(TObject *Sender)
{
	PrevVideo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnPosStartClick(TObject *Sender)
{
	SetVideoPos(0.0f);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DropFiles(TWMDropFiles msg)
{
	char str[1024];
	
	if (DragQueryFile((HDROP)msg.Drop, 0xFFFFFFFF, NULL, 0) <= 0) return;
	DragQueryFile((HDROP)msg.Drop, 0, str, sizeof(file));
	UnicodeString file = str;
	OpenVideo(file);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
	AnsiString str;
	gtime_t time;
	double t, period;
	char msg[256];
	int width, height;
	
	GetVideoTime(t, period);
	GetVideoSize(width, height);
	time = timeadd(TimeStart, t);
	if (FileIndex >= 0) {
		str.sprintf("%s (%d x %d)", time_str(time, 2), width, height);
	}
	if (BtnSync->IsPressed) {
		sprintf(msg, "%s\r\n", time_str(time, 2));
		strwrite(&StrTimeSync, (unsigned char *)msg, (int)strlen(msg));
	}
	ProgressBar->Value = (int)(GetVideoPos()*1000);
	Message1->Text = str;
	Message2->Text = MediaPlayer->FileName;
	
	if (ProgressBar->Value >= 1000 && !BtnPlay->Enabled) {
		NextVideo();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnSyncClick(TObject *Sender)
{
	if (BtnSync->IsPressed) {
		stropen(&StrTimeSync, STR_TCPCLI, STR_MODE_RW, PATH_TIME_SYNC);
	}
	else {
		strclose(&StrTimeSync);
	}
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
	Panel3->ItemWidth = (Panel3->Width-2)/6;
	UpdateVideo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OpenVideo(UnicodeString file)
{
	if (MjpgPlayer->OpenVideo(file)) {
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
	ReadNmea(file);
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
	Files->Clear();
	FileIndex = -1;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::NextVideo(void)
{
	if (FileIndex >= Files->Count-1) return;
	OpenVideo(Files->Strings[++FileIndex]);
	if (!BtnPlay->Enabled) PlayVideo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::PrevVideo(void)
{
	if (FileIndex <= 0) return;
	OpenVideo(Files->Strings[--FileIndex]);
	if (!BtnPlay->Enabled) PlayVideo();
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
	double time, period;
	
	GetVideoTime(time, period);
	
	return time/period;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SetVideoPos(float pos)
{
	if (VideoType == VIDEO_TYPE_MEDIA) {
		if (MediaPlayer->Duration > 0) {
			MediaPlayer->CurrentTime = (TMediaTime)(MediaPlayer->Duration*pos);
		}
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
		if (MediaPlayer->Duration > 0) {
			period = MediaPlayer->Duration*1e-7;
		}
		else {
			period = 137.0;
		}
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
	char buff[1080], sec[32] = "", str[32] = "";
	double dbl;
	int val;
	
	if (!(fp = fopen(INI_FILE, "r"))) return;
	
	while (fgets(buff, sizeof(buff), fp)) {
		if (*buff == '[') {
			sscanf(buff+1, "%31[^\]]", sec);
		}
		else if (!strcmp(sec, "window")) {
			if		(sscanf(buff, "win_width = %d",  &val)) Width	   = val;
			else if (sscanf(buff, "win_height = %d", &val)) Height	   = val;
			else if (sscanf(buff, "win_left = %d",	 &val)) Left	   = val;
			else if (sscanf(buff, "win_top = %d",	 &val)) Top		   = val;
		}
		else if (!strcmp(sec, "option")) {
			if		(sscanf(buff, "mjpg_rate = %lf", &dbl)) MjpgRate   = dbl;
			else if (sscanf(buff, "sync_addr = %31s", str)) SyncAddr   = str;
			else if (sscanf(buff, "sync_port = %d",  &val)) SyncPort   = val;
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
	fprintf(fp, "win_width = %d\n",  Width	   );
	fprintf(fp, "win_height = %d\n", Height    );
	fprintf(fp, "win_left = %d\n",	 Left	   );
	fprintf(fp, "win_top = %d\n",	 Top	   );
	fprintf(fp, "[option]\n");
	fprintf(fp, "mjpg_rate = %.0f\n", MjpgRate );
	AnsiString sync_addr = SyncAddr;
	fprintf(fp, "sync_addr = %s\n",  sync_addr.c_str());
	fprintf(fp, "sync_port = %d\n",	 SyncPort  );
	
	fclose(fp);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::BtnOptionClick(TObject *Sender)
{
	if (VideoPlayerOptDialog->ShowModal() != mrOk) return;
}
//---------------------------------------------------------------------------

