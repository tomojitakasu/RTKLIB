//---------------------------------------------------------------------------
#ifndef optdlgH
#define optdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TConvOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TGroupBox *GroupBox1;
	TLabel *Label1;
	TEdit *Marker;
	TLabel *Label2;
	TEdit *RunBy;
	TEdit *Name0;
	TEdit *Name1;
	TEdit *Rec2;
	TEdit *Rec1;
	TEdit *Rec0;
	TLabel *Label4;
	TLabel *Label5;
	TEdit *Ant0;
	TEdit *Ant1;
	TEdit *Ant2;
	TEdit *AppPos2;
	TEdit *AntDel2;
	TEdit *AppPos1;
	TEdit *AntDel1;
	TEdit *AppPos0;
	TEdit *AntDel0;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *Label10;
	TEdit *Comment0;
	TEdit *Comment1;
	TGroupBox *GroupBox2;
	TCheckBox *Nav1;
	TCheckBox *Nav2;
	TCheckBox *Nav3;
	TCheckBox *Nav4;
	TGroupBox *GroupBox3;
	TCheckBox *Obs2;
	TCheckBox *Obs3;
	TCheckBox *Obs4;
	TGroupBox *GroupBox4;
	TCheckBox *Freq1;
	TCheckBox *Freq2;
	TCheckBox *Freq3;
	TCheckBox *Freq4;
	TCheckBox *Obs1;
	TCheckBox *Nav5;
	TComboBox *TraceLevel;
	TLabel *Label3;
	TCheckBox *Freq5;
	TLabel *Label8;
	TEdit *RcvOption;
	TLabel *Label9;
	TComboBox *RnxVer;
	TCheckBox *Nav6;
	TEdit *MarkerType;
	TEdit *MarkerNo;
	TEdit *ExSats;
	TLabel *Label11;
	TCheckBox *Freq6;
	TCheckBox *RnxFile;
	TLabel *Label12;
	TEdit *RnxCode;
	TCheckBox *OutIono;
	TCheckBox *OutTime;
	TCheckBox *OutLeaps;
	TCheckBox *AutoPos;
	TButton *BtnMask;
	TCheckBox *Nav7;
	TCheckBox *Freq7;
	TCheckBox *ScanObs;
	TCheckBox *HalfCyc;
	TCheckBox *ChkSepNav;
	TLabel *Label13;
	TEdit *TimeTol;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall RnxFileClick(TObject *Sender);
	void __fastcall BtnMaskClick(TObject *Sender);
	void __fastcall RnxVerChange(TObject *Sender);
	void __fastcall AutoPosClick(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	AnsiString CodeMask[7];
	
	__fastcall TConvOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConvOptDialog *ConvOptDialog;
//---------------------------------------------------------------------------
#endif
