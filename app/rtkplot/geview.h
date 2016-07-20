//---------------------------------------------------------------------------
#ifndef geviewH
#define geviewH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "SHDocVw_OCX.h"
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.OleCtrls.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TGoogleEarthView : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel2;
	TCppWebBrowser *WebBrowser;
	TSpeedButton *BtnGENorm;
	TSpeedButton *BtnGETilt;
	TSpeedButton *BtnEnaAlt;
	TSpeedButton *BtnHeading;
	TPanel *Panel3;
	TPanel *Panel5;
	TButton *BtnClose;
	TSpeedButton *BtnFixCent;
	TPanel *Panel6;
	TLabel *Debug;
	TTimer *Timer1;
	TSpeedButton *BtnShrink;
	TSpeedButton *BtnExpand;
	TTimer *Timer2;
	TSpeedButton *BtnRotL;
	TSpeedButton *BtnRotR;
	TPanel *Panel7;
	TPanel *Panel8;
	TSpeedButton *BtnOpt1;
	TSpeedButton *BtnOpt4;
	TSpeedButton *BtnOpt2;
	TSpeedButton *BtnOpt3;
	TSpeedButton *BtnOpt5;
	TSpeedButton *BtnOpt6;
	TSpeedButton *BtnOpt7;
	TSpeedButton *BtnOpt8;
	TSpeedButton *BtnOpt9;
	TSpeedButton *BtnOpt;
	void __fastcall BtnGENormClick(TObject *Sender);
	void __fastcall BtnGETiltClick(TObject *Sender);
	void __fastcall BtnOpt1Click(TObject *Sender);
	void __fastcall BtnHeadingClick(TObject *Sender);
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall BtnFixCentClick(TObject *Sender);
	void __fastcall BtnEnaAltClick(TObject *Sender);
	void __fastcall BtnShrinkMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnShrinkMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnExpandMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnExpandMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall Timer2Timer(TObject *Sender);
	void __fastcall BtnRotLMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnRotLMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnRotRMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnRotRMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall Panel2Gesture(TObject *Sender, const TGestureEventInfo &EventInfo,
          bool &Handled);
	void __fastcall BtnOptClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);


private:
    int State,Expand,Rotate,MarkVis[2],TrackVis[2];
    double Lat,Lon,Range,Heading;
    double MarkPos[2][2];
    
    void __fastcall UpdateOpts (void);
    void __fastcall UpdateEnable(void);
    void __fastcall ExecFunc   (AnsiString func);

public:
	int FixCent;
	
	__fastcall TGoogleEarthView(TComponent* Owner);
    int __fastcall  GetState   (void);
    void __fastcall Init       (void);
    void __fastcall Clear      (void);
	void __fastcall SetView    (double lat, double lon, double range, double heading);
	void __fastcall SetCent    (double lat, double lon);
	void __fastcall SetRange   (double range);
    void __fastcall SetHeading (double angle);
	void __fastcall SetMark    (int index, const double *pos);
	void __fastcall ShowMark   (int index);
	void __fastcall HideMark   (int index);
	void __fastcall ClearTrack (int index);
	int  __fastcall UpdateTrack(int index, solbuf_t *sol);
	void __fastcall ShowTrack  (int index);
	void __fastcall HideTrack  (int index);
	void __fastcall UpdatePoint(void);
	void __fastcall ShowPoint  (void);
	void __fastcall HidePoint  (void);
    void __fastcall SetOpts    (const int *opts);
    void __fastcall GetOpts    (int *opts);
};
//---------------------------------------------------------------------------
extern PACKAGE TGoogleEarthView *GoogleEarthView;
//---------------------------------------------------------------------------
#endif
