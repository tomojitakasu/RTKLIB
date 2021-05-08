//---------------------------------------------------------------------------
#ifndef mapviewH
#define mapviewH
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
class TMapView : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel2;
	TCppWebBrowser *Browser1;
	TPanel *Panel5;
	TButton *BtnClose;
	TTimer *Timer1;
	TSpeedButton *BtnSync;
	TSpeedButton *BtnZoomIn;
	TSpeedButton *BtnZoomOut;
	TButton *BtnOpt;
	TCppWebBrowser *Browser2;
	TRadioButton *MapSel1;
	TRadioButton *MapSel2;
	TPanel *Panel3;
	TTimer *Timer2;
	TPanel *Panel21;
	TPanel *Panel22;
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall BtnZoomOutClick(TObject *Sender);
	void __fastcall BtnZoomInClick(TObject *Sender);
	void __fastcall BtnSyncClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOptClick(TObject *Sender);
	void __fastcall MapSel1Click(TObject *Sender);
	void __fastcall MapSel2Click(TObject *Sender);
	void __fastcall Timer2Timer(TObject *Sender);


private:
	int MapState[2],MarkState[2];
	double Lat,Lon;
	double MarkPos[2][2];
	
    void __fastcall ShowMapLL(void);
    void __fastcall ShowMapGM(void);
    void __fastcall ShowMap(int map);
    void __fastcall SetView(int map, double lat, double lon, int zoom);
    void __fastcall AddMark(int map, int index, double lat, double lon,
	    int state);
    void __fastcall UpdateMap(void);
    void __fastcall SelectMap(int map);
    int  __fastcall GetState(int map);
    int __fastcall ExecFunc(int map, UTF8String func);

public:
	int MapSel;
    
	__fastcall TMapView(TComponent* Owner);
    void __fastcall SetCent(double lat, double lon);
    void __fastcall SetMark(int index, double lat, double lon);
    void __fastcall ShowMark(int index);
    void __fastcall HideMark(int index);
};
//---------------------------------------------------------------------------
extern PACKAGE TMapView *MapView;
//---------------------------------------------------------------------------
#endif
