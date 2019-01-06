//---------------------------------------------------------------------------
#ifndef gmviewH
#define gmviewH
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
class TGoogleMapView : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel2;
	TCppWebBrowser *WebBrowser;
	TPanel *Panel5;
	TButton *BtnClose;
	TTimer *Timer1;
	TSpeedButton *BtnFixCent;
	TSpeedButton *BtnExpand;
	TSpeedButton *BtnShrink;
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall BtnShrinkClick(TObject *Sender);
	void __fastcall BtnExpandClick(TObject *Sender);
	void __fastcall BtnFixCentClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);


private:
	int State,WebCreate;
	double Lat,Lon,Zoom;
	double MarkPos[2][2];
	
    void __fastcall ExecFunc(AnsiString func);

public:
	int FixCent;
	
	__fastcall TGoogleMapView(TComponent* Owner);
    int  __fastcall GetState(void);
    void __fastcall SetView(double lat, double lon, int zoom);
    void __fastcall SetCent(double lat, double lon);
    void __fastcall SetZoom(int zoom);
    void __fastcall ClearMark(void);
    void __fastcall AddMark(double lat, double lon, AnsiString title, AnsiString msg);
    void __fastcall SetMark(int index, const double *pos);
    void __fastcall ShowMark(int index);
    void __fastcall HideMark(int index);
};
//---------------------------------------------------------------------------
extern PACKAGE TGoogleMapView *GoogleMapView;
//---------------------------------------------------------------------------
#endif
