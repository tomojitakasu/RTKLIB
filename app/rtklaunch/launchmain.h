//---------------------------------------------------------------------------
#ifndef launchmainH
#define launchmainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Menus.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TSpeedButton *BtnPlot;
	TSpeedButton *BtnNavi;
	TSpeedButton *BtnNtrip;
	TSpeedButton *BtnPost;
	TSpeedButton *BtnStr;
	TSpeedButton *BtnConv;
	TSpeedButton *BtnGet;
	TTrayIcon *TrayIcon;
	TPopupMenu *PopupMenu;
	TMenuItem *MenuPlot;
	TMenuItem *MenuConv;
	TMenuItem *MenuStr;
	TMenuItem *MenuPost;
	TMenuItem *MenuNtrip;
	TMenuItem *MenuNavi;
	TMenuItem *MenuGet;
	TMenuItem *N1;
	TMenuItem *MenuExit;
	TPanel *BtnTray;
	TMenuItem *MenuExpand;
	TMenuItem *N2;
	TPanel *Panel1;
	TPanel *BtnOption;
	TSpeedButton *BtnRtklib;
	TPanel *Panel2;
	TPanel *BtnExit;
	TSpeedButton *BtnVideo;
	TMenuItem *MenuVideo;
	
	void __fastcall BtnPlotClick(TObject *Sender);
	void __fastcall BtnConvClick(TObject *Sender);
	void __fastcall BtnStrClick(TObject *Sender);
	void __fastcall BtnPostClick(TObject *Sender);
	void __fastcall BtnNtripClick(TObject *Sender);
	void __fastcall BtnNaviClick(TObject *Sender);
	void __fastcall BtnGetClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall BtnTrayClick(TObject *Sender);
	void __fastcall TrayIconDblClick(TObject *Sender);
	void __fastcall MenuPlotClick(TObject *Sender);
	void __fastcall MenuConvClick(TObject *Sender);
	void __fastcall MenuStrClick(TObject *Sender);
	void __fastcall MenuPostClick(TObject *Sender);
	void __fastcall MenuNtripClick(TObject *Sender);
	void __fastcall MenuNaviClick(TObject *Sender);
	void __fastcall MenuGetClick(TObject *Sender);
	void __fastcall MenuExitClick(TObject *Sender);
	void __fastcall MenuExpandClick(TObject *Sender);
	void __fastcall Panel1Resize(TObject *Sender);
	void __fastcall BtnOptionClick(TObject *Sender);
	void __fastcall BtnRtklibMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnExitClick(TObject *Sender);
	void __fastcall BtnVideoClick(TObject *Sender);
	void __fastcall MenuVideoClick(TObject *Sender);

private:
	AnsiString IniFile;
	int Tray;
	
	void __fastcall UpdatePanel(void);
	int __fastcall ExecCmd(AnsiString cmd);
public:
	int Option,Minimize;
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
