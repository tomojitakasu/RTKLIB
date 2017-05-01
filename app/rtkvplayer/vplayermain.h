//---------------------------------------------------------------------------

#ifndef vplayerH
#define vplayerH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Media.hpp>
#include <FMX.Types.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Dialogs.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Platform.Win.hpp>

#include "rtklib.h"
#include "mjpgplayer.h"
#include <FMX.ImgList.hpp>
#include <System.ImageList.hpp>

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TMediaPlayer *MediaPlayer;
	TMediaPlayerControl *MediaPlayerControl;
	TButton *BtnOpen;
	TButton *BtnPlay;
	TButton *BtnStop;
	TButton *BtnClear;
	TButton *BtnOption;
	TOpenDialog *OpenDialog;
	TGridLayout *Panel3;
	TProgressBar *ProgressBar;
	TTimer *Timer1;
	TLayout *Panel2;
	TButton *BtnPosStart;
	TRectangle *Panel1;
	TLabel *Message1;
	TLabel *Message2;
	TRectangle *Panel4;
	TSpeedButton *BtnSync;
	TImageList *ImageList1;
	TButton *BtnNext;
	TButton *BtnPrev;
	TButton *BtnExit;
	void __fastcall BtnOpenClick(TObject *Sender);
	void __fastcall BtnPlayClick(TObject *Sender);
	void __fastcall BtnStopClick(TObject *Sender);
	void __fastcall BtnClearClick(TObject *Sender);
	void __fastcall BtnExitClick(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall ProgressBarMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  float X, float Y);
	void __fastcall ProgressBarMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
		  float X, float Y);
	void __fastcall ProgressBarMouseMove(TObject *Sender, TShiftState Shift, float X,
		  float Y);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall BtnPosStartClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall BtnSyncClick(TObject *Sender);
	void __fastcall BtnNextClick(TObject *Sender);
	void __fastcall BtnPrevClick(TObject *Sender);
	void __fastcall BtnOptionClick(TObject *Sender);

private:
	gtime_t TimeStart;
	stream_t StrTimeSync;
	TMjpgPlayer *MjpgPlayer;
	TStrings *Files;
	int VideoType, Track, NStrBuff, FileIndex;
	char StrBuff[1024];
	
	void __fastcall DropFiles(TWMDropFiles msg);
	void __fastcall OpenVideo(UnicodeString file);
	void __fastcall ReadNmea(UnicodeString file);
	void __fastcall ClearVideo(void);
	void __fastcall NextVideo(void);
	void __fastcall PrevVideo(void);
	int  __fastcall PlayVideo(void);
	void __fastcall StopVideo(void);
	void __fastcall SyncVideo(void);
	void __fastcall SetVideoPos(float pos);
	float __fastcall GetVideoPos(void);
	void __fastcall GetVideoTime(double &time, double &period);
	void __fastcall GetVideoSize(int &width, int &height);
	void __fastcall UpdateVideo(void);
	void __fastcall LoadOptions(void);
	void __fastcall SaveOptions(void);
	
	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_DROPFILES, TWMDropFiles, DropFiles);
	END_MESSAGE_MAP(TForm);
public:
	double MjpgRate;
	UnicodeString SyncAddr;
	int SyncPort;
	
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
