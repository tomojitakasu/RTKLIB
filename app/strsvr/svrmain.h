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

#define MAXSTR        4    // number of streams

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
	TPanel *Panel3;
	TLabel *LabelOutput3;
	TComboBox *Output3;
	TButton *BtnOutput3;
	TLabel *Output3Byte;
	TLabel *Output3Bps;
	TPanel *IndOutput3;
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
	TButton *BtnConv3;
	TLabel *Label2;
	TBitBtn *BtnStart;
	TBitBtn *BtnStop;
	TBitBtn *BtnOpt;
	TBitBtn *BtnExit;
	TButton *BtnCmd1;
	TButton *BtnCmd2;
	TButton *BtnCmd3;
	void __fastcall BtnExitClick(TObject *Sender);
	void __fastcall BtnInputClick(TObject *Sender);
	void __fastcall BtnOutput1Click(TObject *Sender);
	void __fastcall BtnOutput2Click(TObject *Sender);
	void __fastcall BtnStartClick(TObject *Sender);
	void __fastcall BtnStopClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall BtnOptClick(TObject *Sender);
	void __fastcall Output1Change(TObject *Sender);
	void __fastcall Output2Change(TObject *Sender);
	void __fastcall InputChange(TObject *Sender);
	void __fastcall BtnOutput3Click(TObject *Sender);
	void __fastcall Output3Change(TObject *Sender);
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
	void __fastcall BtnConv1Click(TObject *Sender);
	void __fastcall BtnConv2Click(TObject *Sender);
	void __fastcall BtnConv3Click(TObject *Sender);
	void __fastcall EnaOut1Click(TObject *Sender);
	void __fastcall EnaOut2Click(TObject *Sender);
	void __fastcall EnaOut3Click(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall TrayIconMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
private:
	AnsiString IniFile;
	AnsiString Paths[MAXSTR][4],Cmds[MAXSTR][3],CmdsTcp[MAXSTR][3];
	AnsiString TcpHistory[MAXHIST],TcpMntpHist[MAXHIST];
	AnsiString StaPosFile,ExeDirectory,LocalDirectory,SwapInterval;
	AnsiString ProxyAddress,SrcTblFile,LogFile;
	AnsiString ConvMsg[MAXSTR-1],ConvOpt[MAXSTR-1],AntType,RcvType;
	int ConvEna[MAXSTR-1],ConvInp[MAXSTR-1],ConvOut[MAXSTR-1],StaId,StaSel;
	int TraceLevel,SvrOpt[6],CmdEna[MAXSTR][3],CmdEnaTcp[MAXSTR][3];
	int NmeaReq,FileSwapMargin,RelayBack,ProgBarRange;
	double AntPos[3],AntOff[3];
	gtime_t StartTime,EndTime;
	
	void __fastcall SerialOpt(int index, int opt);
	void __fastcall TcpOpt(int index, int opt);
	void __fastcall FileOpt(int index, int opt);
	void __fastcall FtpOpt(int index, int opt);
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
