//---------------------------------------------------------------------------
#ifndef svrmainH
#define svrmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Graphics.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include <ImgList.hpp>

#include "rtklib.h"
#include "tcpoptdlg.h"
#include <System.ImageList.hpp>

#define MAXSTR        7    // number of streams

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TPanel *Panel1;
	TLabel *LabelInput;
	TLabel *LabelOutput1;
	TComboBox *Input;
	TLabel *Label3;
	TComboBox *Output1;
	TButton *BtnInput;
	TLabel *Label4;
	TButton *BtnOutput1;
	TButton *BtnOutput2;
	TLabel *InputByte;
	TPanel *IndInput;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *LabelOutput2;
	TComboBox *Output2;
	TLabel *InputBps;
	TLabel *Output1Byte;
	TLabel *Output1Bps;
	TLabel *Output2Byte;
	TLabel *Output2Bps;
	TPanel *IndOutput1;
	TPanel *IndOutput2;
	TLabel *Label5;
	TTimer *Timer1;
	TLabel *LabelOutput5;
	TComboBox *Output5;
	TButton *BtnOutput5;
	TLabel *Output5Byte;
	TLabel *Output5Bps;
	TPanel *IndOutput5;
	TButton *BtnCmd;
	TLabel *Label1;
	TPanel *Panel4;
	TLabel *Message;
	TProgressBar *Progress;
	TSpeedButton *BtnAbout;
	TSpeedButton *BtnStrMon;
	TTimer *Timer2;
	TSpeedButton *BtnTaskIcon;
	TPopupMenu *PopupMenu;
	TMenuItem *MenuStart;
	TMenuItem *MenuStop;
	TMenuItem *N1;
	TMenuItem *N2;
	TMenuItem *MenuExpand;
	TMenuItem *MenuExit;
	TTrayIcon *TrayIcon;
	TImageList *ImageList;
	TPanel *Panel2;
	TLabel *Label8;
	TLabel *ConTime;
	TLabel *Time;
	TButton *BtnConv1;
	TButton *BtnConv2;
	TButton *BtnConv5;
	TLabel *Label2;
	TBitBtn *BtnStart;
	TBitBtn *BtnStop;
	TBitBtn *BtnOpt;
	TBitBtn *BtnExit;
	TButton *BtnCmd1;
	TButton *BtnCmd2;
	TButton *BtnCmd5;
	TPanel *Panel5;
	TPanel *Panel12;
	TPanel *Panel11;
	TPanel *Panel13;
	TPanel *Panel14;
	TPanel *Panel17;
	TPanel *Panel16;
	TLabel *LabelOutput4;
	TLabel *Output4Bps;
	TLabel *Output4Byte;
	TButton *BtnCmd4;
	TButton *BtnConv4;
	TButton *BtnOutput4;
	TPanel *IndOutput4;
	TComboBox *Output4;
	TPanel *Panel15;
	TLabel *LabelOutput3;
	TLabel *Output3Bps;
	TLabel *Output3Byte;
	TButton *BtnCmd3;
	TButton *BtnConv3;
	TButton *BtnOutput3;
	TPanel *IndOutput3;
	TComboBox *Output3;
	TLabel *Label9;
	TButton *BtnLog1;
	TButton *BtnLog2;
	TButton *BtnLog3;
	TButton *BtnLog4;
	TButton *BtnLog5;
	TPanel *Panel18;
	TLabel *LabelOutput6;
	TLabel *Output6Bps;
	TLabel *Output6Byte;
	TButton *BtnCmd6;
	TButton *BtnConv6;
	TButton *BtnOutput6;
	TPanel *IndOutput6;
	TComboBox *Output6;
	TButton *BtnLog6;
	TButton *BtnLog;
	TPanel *IndLog;
	TPanel *IndLog1;
	TPanel *IndLog2;
	TPanel *IndLog3;
	TPanel *IndLog4;
	TPanel *IndLog5;
	TPanel *IndLog6;
	void __fastcall BtnExitClick(TObject *Sender);
	void __fastcall BtnInputClick(TObject *Sender);
	void __fastcall BtnOutputClick(TObject *Sender);
	void __fastcall BtnStartClick(TObject *Sender);
	void __fastcall BtnStopClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall BtnOptClick(TObject *Sender);
	void __fastcall OutputChange(TObject *Sender);
	void __fastcall InputChange(TObject *Sender);
	void __fastcall BtnCmdClick(TObject *Sender);
	void __fastcall BtnAboutClick(TObject *Sender);
	void __fastcall BtnStrMonClick(TObject *Sender);
	void __fastcall Timer2Timer(TObject *Sender);
	void __fastcall BtnTaskIconClick(TObject *Sender);
	void __fastcall MenuExpandClick(TObject *Sender);
	void __fastcall TrayIconDblClick(TObject *Sender);
	void __fastcall MenuStartClick(TObject *Sender);
	void __fastcall MenuStopClick(TObject *Sender);
	void __fastcall MenuExitClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BtnConvClick(TObject *Sender);
	void __fastcall EnaOut1Click(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall TrayIconMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall BtnLogClick(TObject *Sender);
private:
	AnsiString IniFile;
	AnsiString Paths[MAXSTR][7],Cmds[MAXSTR][3],CmdsTcp[MAXSTR][3];
	AnsiString TcpHistory[MAXHIST],TcpMntpHist[MAXHIST];
	AnsiString StaPosFile,ExeDirectory,LocalDirectory,SwapInterval;
	AnsiString ProxyAddress,LogFile;
	AnsiString ConvMsg[MAXSTR-1],ConvOpt[MAXSTR-1],AntType,RcvType;
	AnsiString PathLog[MAXSTR];
	int ConvEna[MAXSTR-1],ConvInp[MAXSTR-1],ConvOut[MAXSTR-1],StaId,StaSel;
	int TraceLevel,SvrOpt[6],CmdEna[MAXSTR][3],CmdEnaTcp[MAXSTR][3];
	int NmeaReq,FileSwapMargin,RelayBack,ProgBarRange,PathEna[MAXSTR];
	double AntPos[3],AntOff[3];
	gtime_t StartTime,EndTime;
	
	void __fastcall SerialOpt(int index, int path);
	void __fastcall TcpCliOpt(int index, int path);
	void __fastcall TcpSvrOpt(int index, int path);
	void __fastcall NtripSvrOpt(int index, int path);
	void __fastcall NtripCliOpt(int index, int path);
	void __fastcall NtripCasOpt(int index, int path);
	void __fastcall UdpCliOpt(int index, int path);
	void __fastcall UdpSvrOpt(int index, int path);
	void __fastcall FileOpt(int index, int path);
	void __fastcall ShowMsg(AnsiString msg);
	void __fastcall SvrStart(void);
	void __fastcall SvrStop(void);
	void __fastcall UpdateEnable(void);
	void __fastcall SetTrayIcon(int index);
	void __fastcall LoadOpt(void);
	void __fastcall SaveOpt(void);
public:
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
