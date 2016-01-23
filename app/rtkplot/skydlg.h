//---------------------------------------------------------------------------
#ifndef skydlgH
#define skydlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TSkyImgDialog : public TForm
{
__published:
	TButton *BtnClose;
	TButton *BtnSave;
	TButton *BtnUpdate;
	TPanel *Panel1;
	TLabel *Label1;
	TEdit *SkySize1;
	TEdit *SkySize2;
	TPanel *Panel2;
	TEdit *SkyFov1;
	TUpDown *SkyFov1UpDown;
	TEdit *SkyFov2;
	TUpDown *SkyFov2UpDown;
	TLabel *Label6;
	TLabel *Label2;
	TCheckBox *SkyElMask;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *SkyFov3;
	TUpDown *SkyFov3UpDown;
	TLabel *Label7;
	TEdit *SkyCent1;
	TEdit *SkyCent2;
	TLabel *Label5;
	TLabel *Label9;
	TLabel *Label10;
	TEdit *SkyScale;
	TUpDown *SkyCent1UpDown;
	TUpDown *SkyScaleUpDown;
	TUpDown *SkyCent2UpDown;
	TCheckBox *SkyDestCorr;
	TEdit *SkyDest1;
	TEdit *SkyDest2;
	TEdit *SkyDest3;
	TEdit *SkyDest4;
	TEdit *SkyDest5;
	TEdit *SkyDest6;
	TEdit *SkyDest7;
	TEdit *SkyDest8;
	TLabel *Label11;
	TLabel *Label12;
	TLabel *Label13;
	TLabel *Label14;
	TLabel *Label15;
	TLabel *Label16;
	TLabel *Label17;
	TLabel *Label18;
	TLabel *Label8;
	TComboBox *SkyRes;
	TCheckBox *SkyFlip;
	TButton *BtnLoad;
	TOpenDialog *OpenTagDialog;
	TEdit *SkyDest9;
	TLabel *Label19;
	TLabel *Label20;
	TButton *BtnGenMask;
	TCheckBox *SkyBinarize;
	TEdit *SkyBinThres1;
	TUpDown *SkyBinThres1UpDown;
	TEdit *SkyBinThres2;
	TUpDown *SkyBinThres2UpDown;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnCloseClick(TObject *Sender);
	void __fastcall BtnUpdateClick(TObject *Sender);
	void __fastcall BtnSaveClick(TObject *Sender);
	void __fastcall SkyFov2UpDownChangingEx(TObject *Sender, bool &AllowChange, short NewValue,
          TUpDownDirection Direction);
	void __fastcall SkyFov1UpDownChangingEx(TObject *Sender, bool &AllowChange, short NewValue,
          TUpDownDirection Direction);
	void __fastcall SkyFov3UpDownChangingEx(TObject *Sender, bool &AllowChange, short NewValue,
          TUpDownDirection Direction);
	void __fastcall SkyCent1UpDownChangingEx(TObject *Sender, bool &AllowChange, short NewValue,
          TUpDownDirection Direction);
	void __fastcall SkyCent2UpDownChangingEx(TObject *Sender, bool &AllowChange, short NewValue,
          TUpDownDirection Direction);
	void __fastcall SkyScaleUpDownChangingEx(TObject *Sender, bool &AllowChange, short NewValue,
          TUpDownDirection Direction);
	void __fastcall SkyElMaskMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall SkyDestCorrMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall SkyFlipMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall SkyResChange(TObject *Sender);
	void __fastcall BtnLoadClick(TObject *Sender);
	void __fastcall BtnGenMaskClick(TObject *Sender);
	void __fastcall SkyBinThres1UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall SkyBinThres2UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall SkyBinarizeMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
private:
	void __fastcall UpdateSky(void);
	void __fastcall UpdateEnable(void);
	
public:
	__fastcall TSkyImgDialog(TComponent* Owner);
	void __fastcall UpdateField(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TSkyImgDialog *SkyImgDialog;
//---------------------------------------------------------------------------
#endif
