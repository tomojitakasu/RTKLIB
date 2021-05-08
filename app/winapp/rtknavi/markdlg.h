//---------------------------------------------------------------------------

#ifndef markdlgH
#define markdlgH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TMarkDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TLabel *Label2;
	TRadioButton *RadioStop;
	TRadioButton *RadioGo;
	TLabel *LabelPosMode;
	TComboBox *MarkerName;
	TCheckBox *ChkMarkerName;
	TEdit *MarkerComment;
	TLabel *Label1;
	TSpeedButton *BtnRepDlg;
	TRadioButton *RadioFix;
	TEdit *EditLat;
	TEdit *EditLon;
	TEdit *EditHgt;
	TLabel *LabelPos;
	TButton *BtnPos;
	void __fastcall BtnCancelClick(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall ChkMarkerNameClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnRepDlgClick(TObject *Sender);
	void __fastcall RadioGoClick(TObject *Sender);
	void __fastcall BtnPosClick(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	AnsiString Marker,Comment;
	double FixPos[3];
	int PosMode,NMark;
	
	__fastcall TMarkDialog(TComponent* Owner);
	
};
//---------------------------------------------------------------------------
extern PACKAGE TMarkDialog *MarkDialog;
//---------------------------------------------------------------------------
#endif
