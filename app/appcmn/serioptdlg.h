//---------------------------------------------------------------------------
#ifndef serioptdlgH
#define serioptdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TSerialOptDialog : public TForm
{
__published:
	TButton *BtnOk;
	TButton *BtnCancel;
	TComboBox *BitRate;
	TLabel *Label1;
	TLabel *Label3;
	TComboBox *Port;
	TLabel *Label2;
	TComboBox *ByteSize;
	TLabel *Label4;
	TComboBox *Parity;
	TLabel *Label5;
	TComboBox *StopBits;
	TLabel *Label8;
	TComboBox *FlowCtr;
	TButton *BtnCmd;
	TCheckBox *OutTcpPort;
	TEdit *TcpPort;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall BtnCmdClick(TObject *Sender);
	void __fastcall OutTcpPortClick(TObject *Sender);
private:
	void __fastcall UpdatePortList(void);
	void __fastcall UpdateEnable(void);
public:
	AnsiString Path,Cmds[2];
	int Opt,CmdEna[2];
	__fastcall TSerialOptDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSerialOptDialog *SerialOptDialog;
//---------------------------------------------------------------------------
#endif
