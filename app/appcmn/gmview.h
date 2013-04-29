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
	TSpeedButton *BtnHome;
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BtnHomeClick(TObject *Sender);


private:
    void __fastcall ExecFunc(AnsiString func);

public:
	__fastcall TGoogleMapView(TComponent* Owner);
    void __fastcall ShowHome(void);
    int  __fastcall GetState(void);
    void __fastcall ClearMark(void);
    void __fastcall AddMark(double lat, double lon, AnsiString title, AnsiString msg);
    void __fastcall PosMark(double lat, double lon, AnsiString title);
    void __fastcall HighlightMark(AnsiString title);
};
//---------------------------------------------------------------------------
extern PACKAGE TGoogleMapView *GoogleMapView;
//---------------------------------------------------------------------------
#endif
