//---------------------------------------------------------------------------

#ifndef plotoptH
#define plotoptH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include "plotmain.h"
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TPlotOptDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOK;
	TComboBox *PlotStyle;
	TComboBox *Origin;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TComboBox *ShowArrow;
	TPanel *Panel2;
	TColorDialog *ColorDialog;
	TPanel *Color1;
	TButton *BtnColor1;
	TLabel *Label5;
	TPanel *Color2;
	TButton *BtnColor2;
	TLabel *Label6;
	TPanel *Color3;
	TButton *BtnColor3;
	TLabel *Label7;
	TPanel *Color4;
	TButton *BtnColor4;
	TLabel *Label9;
	TComboBox *ShowStats;
	TLabel *Label8;
	TLabel *Label10;
	TLabel *Label12;
	TComboBox *TimeLabel;
	TLabel *Label13;
	TComboBox *AutoScale;
	TPanel *MColor1;
	TLabel *Label14;
	TPanel *MColor2;
	TPanel *MColor3;
	TPanel *MColor4;
	TPanel *MColor5;
	TButton *BtnFont;
	TFontDialog *FontDialog;
	TLabel *Msg;
	TLabel *Label17;
	TComboBox *ShowSlip;
	TLabel *Label18;
	TLabel *Label19;
	TComboBox *ShowErr;
	TComboBox *MarkSize;
	TLabel *Label16;
	TComboBox *ShowHalfC;
	TComboBox *YRange;
	TLabel *Label20;
	TComboBox *ShowLabel;
	TLabel *Label21;
	TComboBox *ShowGLabel;
	TLabel *Label22;
	TComboBox *ShowScale;
	TLabel *Label23;
	TComboBox *ShowCompass;
	TLabel *Label24;
	TComboBox *ShowEph;
	TGroupBox *GroupBox1;
	TCheckBox *NavSys1;
	TCheckBox *NavSys2;
	TCheckBox *NavSys5;
	TCheckBox *NavSys3;
	TCheckBox *NavSys4;
	TComboBox *ElMask;
	TLabel *Label11;
	TComboBox *AnimCycle;
	TLabel *Label25;
	TComboBox *HideLowSat;
	TPanel *Panel1;
	TLabel *FontLabel;
	TLabel *LabelFont;
	TLabel *Label26;
	TComboBox *ElMaskP;
	TLabel *LabelRefPos;
	TEdit *RefPos1;
	TEdit *RefPos2;
	TEdit *RefPos3;
	TButton *BtnRefPos;
	TLabel *LabelExSats;
	TEdit *ExSats;
	TLabel *Label28;
	TComboBox *MaxDop;
	TEdit *BuffSize;
	TLabel *Label29;
	TOpenDialog *OpenDialog;
	TLabel *Label27;
	TEdit *RefCycle;
	TPanel *MColor6;
	TCheckBox *NavSys6;
	TEdit *QcCmd;
	TLabel *Label30;
	TLabel *Label31;
	TComboBox *RcvPos;
	TLabel *Label32;
	TPanel *MColor7;
	TPanel *MColor8;
	TPanel *MColor9;
	TPanel *MColor10;
	TPanel *MColor11;
	TPanel *MColor12;
	TLabel *Label15;
	TComboBox *LatLonFmt;
	TLabel *Label33;
	TEdit *RnxOpts;
	TLabel *Label34;
	TEdit *TLEFile;
	TButton *BtnTLEFile;
	TLabel *Label35;
	TEdit *TLESatFile;
	TButton *BtnTLESatFile;
	TLabel *Label36;
	TSpeedButton *BtnTLEView;
	TSpeedButton *BtnTLESatView;
	TLabel *Label37;
	TComboBox *MaxMP;
	TCheckBox *NavSys7;
	TEdit *EditTimeSync;
	TCheckBox *ChkTimeSync;
	TLabel *Label38;
	TEdit *ApiKey;
	void __fastcall BtnOKClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnColor1Click(TObject *Sender);
	void __fastcall BtnColor2Click(TObject *Sender);
	void __fastcall BtnColor3Click(TObject *Sender);
	void __fastcall BtnColor4Click(TObject *Sender);
	void __fastcall BtnRefPosClick(TObject *Sender);
	void __fastcall OriginChange(TObject *Sender);
	void __fastcall AutoScaleChange(TObject *Sender);
	void __fastcall MColorClick(TObject *Sender);
	void __fastcall BtnFontClick(TObject *Sender);
	void __fastcall RcvPosChange(TObject *Sender);
	void __fastcall BtnTLEFileClick(TObject *Sender);
	void __fastcall BtnQcCmdClick(TObject *Sender);
	void __fastcall BtnTLESatFileClick(TObject *Sender);
	void __fastcall BtnTLEViewClick(TObject *Sender);
	void __fastcall BtnTLESatViewClick(TObject *Sender);
	void __fastcall ChkTimeSyncClick(TObject *Sender);
private:
	TFont *FontOpt;
	void __fastcall UpdateFont(void);
	void __fastcall UpdateEnable(void);
public:
	TPlot *Plot;
	__fastcall TPlotOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPlotOptDialog *PlotOptDialog;
//---------------------------------------------------------------------------
#endif
