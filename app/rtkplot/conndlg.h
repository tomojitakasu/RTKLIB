//---------------------------------------------------------------------------
#ifndef conndlgH
#define conndlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#define MAXHIST		10

//---------------------------------------------------------------------------
class TConnectDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TComboBox *SelStream1;
	TButton *BtnOpt1;
	TLabel *Label1;
	TLabel *Label2;
	TComboBox *SolFormat1;
	TButton *BtnCmd1;
	TLabel *Label3;
	TLabel *Label4;
	TComboBox *TimeFormS;
	TComboBox *DegFormS;
	TLabel *Label5;
	TLabel *Label6;
	TEdit *FieldSepS;
	TLabel *Label7;
	TEdit *TimeOutTimeE;
	TEdit *ReConnTimeE;
	TLabel *Label8;
	TComboBox *SelStream2;
	TButton *BtnOpt2;
	TButton *BtnCmd2;
	TComboBox *SolFormat2;
	TLabel *Label9;
	TLabel *Label10;
	void __fastcall BtnOpt1Click(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnCmd1Click(TObject *Sender);
	void __fastcall SelStream1Change(TObject *Sender);
	void __fastcall SolFormat1Change(TObject *Sender);
	void __fastcall BtnOpt2Click(TObject *Sender);
	void __fastcall BtnCmd2Click(TObject *Sender);
	void __fastcall SolFormat2Change(TObject *Sender);
	void __fastcall SelStream2Change(TObject *Sender);
private:
	void __fastcall SerialOpt1(int opt);
	void __fastcall SerialOpt2(int opt);
	void __fastcall TcpOpt1(int opt);
	void __fastcall TcpOpt2(int opt);
	void __fastcall FileOpt1(int opt);
	void __fastcall FileOpt2(int opt);
	void __fastcall UpdateEnable(void);
public:
	int Stream1,Stream2,Format1,Format2,CmdEna1[2],CmdEna2[2];
	int TimeForm,DegForm,TimeOutTime,ReConnTime;
	AnsiString Path,Paths1[4],Paths2[4];
	AnsiString TcpHistory[MAXHIST],TcpMntpHist[MAXHIST];
	AnsiString Cmds1[2],Cmds2[2],FieldSep;
	__fastcall TConnectDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConnectDialog *ConnectDialog;
//---------------------------------------------------------------------------
#endif
