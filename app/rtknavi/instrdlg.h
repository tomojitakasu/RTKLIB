//---------------------------------------------------------------------------
#ifndef instrdlgH
#define instrdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TInputStrDialog : public TForm
{
__published:
	TButton *BtnCancel;
	TButton *BtnOk;
	TComboBox *Stream1;
	TLabel *Label5;
	TButton *BtnStr1;
	TLabel *Label6;
	TComboBox *Stream2;
	TButton *BtnStr2;
	TLabel *Label10;
	TComboBox *Format1;
	TLabel *Label7;
	TComboBox *Format2;
	TButton *BtnCmd1;
	TButton *BtnCmd2;
	TLabel *Label11;
	TLabel *LabelF1;
	TEdit *FilePath1;
	TEdit *FilePath2;
	TCheckBox *TimeTagC;
	TEdit *TimeStartE;
	TButton *BtnFile1;
	TButton *BtnFile2;
	TComboBox *NmeaReqL;
	TOpenDialog *OpenDialog;
	TComboBox *TimeSpeedL;
	TLabel *LabelF2;
	TLabel *LabelF3;
	TEdit *NmeaPos1;
	TEdit *NmeaPos2;
	TButton *BtnPos;
	TLabel *LabelNmea;
	TCheckBox *StreamC1;
	TCheckBox *StreamC2;
	TEdit *FilePath3;
	TButton *BtnFile3;
	TCheckBox *StreamC3;
	TComboBox *Stream3;
	TButton *BtnStr3;
	TComboBox *Format3;
	TButton *BtnCmd3;
	TButton *BtnRcvOpt1;
	TButton *BtnRcvOpt2;
	TButton *BtnRcvOpt3;
	TLabel *Label1;
	TEdit *NmeaPos3;
	TEdit *EditResetCmd;
	TLabel *LabelResetCmd;
	TEdit *EditMaxBL;
	TLabel *LabelMaxBL;
	TLabel *LabelKm;
	TCheckBox *Chk64Bit;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnStr1Click(TObject *Sender);
	void __fastcall BtnStr2Click(TObject *Sender);
	void __fastcall Stream1Change(TObject *Sender);
	void __fastcall Stream2Change(TObject *Sender);
	void __fastcall BtnCmd1Click(TObject *Sender);
	void __fastcall BtnCmd2Click(TObject *Sender);
	void __fastcall TimeTagCClick(TObject *Sender);
	void __fastcall NmeaReqCClick(TObject *Sender);
	void __fastcall BtnFile1Click(TObject *Sender);
	void __fastcall BtnFile2Click(TObject *Sender);
	void __fastcall NmeaReqLChange(TObject *Sender);
	void __fastcall BtnPosClick(TObject *Sender);
	void __fastcall StreamC1Click(TObject *Sender);
	void __fastcall StreamC2Click(TObject *Sender);
	void __fastcall Stream3Change(TObject *Sender);
	void __fastcall BtnStr3Click(TObject *Sender);
	void __fastcall BtnFile3Click(TObject *Sender);
	void __fastcall BtnCmd3Click(TObject *Sender);
	void __fastcall BtnRcvOpt1Click(TObject *Sender);
	void __fastcall BtnRcvOpt2Click(TObject *Sender);
	void __fastcall BtnRcvOpt3Click(TObject *Sender);
private:
	AnsiString __fastcall GetFilePath(AnsiString path);
	AnsiString __fastcall SetFilePath(AnsiString path);
	void __fastcall SerialOpt(int index, int opt);
	void __fastcall TcpOpt(int index, int opt);
	void __fastcall FtpOpt(int index, int opt);
	void __fastcall UpdateEnable(void);
public:
	int StreamC[3],Stream[3],Format[3],CmdEna[3][3],CmdEnaTcp[3][3];
	int NmeaReq,TimeTag,Time64Bit,NRcv;
	double NmeaPos[3],MaxBL;
	AnsiString Paths[3][4],Cmds[3][3],CmdsTcp[3][3],TimeStart,TimeSpeed;
	AnsiString RcvOpt[3],ResetCmd;
	AnsiString History[10],MntpHist[10];
	__fastcall TInputStrDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TInputStrDialog *InputStrDialog;
//---------------------------------------------------------------------------
#endif
