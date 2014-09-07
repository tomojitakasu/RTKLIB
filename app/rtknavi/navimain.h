//---------------------------------------------------------------------------
#ifndef navimainH
#define navimainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>

#include "rtklib.h"

#define MAXSCALE	18
#define MAXMAPPNT	10

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel2;
	TPanel *Panel3;
	TPanel *Panel11;
	
	TLabel *LabelTime;
	TLabel *Message;
	
	TPopupMenu *PopupMenu;
	TMenuItem *MenuMonitor;
	TMenuItem *MenuExpand;
	TMenuItem *N1;
	TMenuItem *MenuStart;
	TMenuItem *MenuPlot;
	TMenuItem *N2;
	TMenuItem *MenuExit;
	TMenuItem *MenuStop;
	
	TSpeedButton *BtnTimeSys;
	
	TTrayIcon *TrayIcon;
	
	TSaveDialog *SaveDialog;
	
	TImageList *ImageList;
	
	TTimer *Timer;
	TPanel *Panel4;
	TButton *BtnExit;
	TButton *BtnOpt;
	TButton *BtnPlot;
	TButton *BtnStart;
	TButton *BtnStop;
	TPanel *Panel33;
	TSpeedButton *BtnTaskTray;
	TSpeedButton *BtnAbout;
	TPanel *Panel31;
	TSpeedButton *BtnMonitor;
	TPanel *Panel32;
	TPanel *Panel12;
	TImage *Image1;
	TImage *Image2;
	TPanel *Str1;
	TPanel *Str2;
	TPanel *Str3;
	TPanel *Str4;
	TPanel *Str5;
	TPanel *Str6;
	TPanel *Str7;
	TPanel *Str8;
	TPanel *Svr;
	TPanel *Panel13;
	TPanel *Panel131;
	TSpeedButton *BtnOutputStr;
	TPanel *Panel132;
	TSpeedButton *BtnLogStr;
	TPanel *Panel121;
	TSpeedButton *BtnInputStr;
	TPanel *Panel21;
	TPanel *Panel211;
	TLabel *LabelNSat;
	TLabel *LabelStd;
	TLabel *Plabel1;
	TLabel *Plabel2;
	TLabel *Plabel3;
	TLabel *PlabelA;
	TLabel *Pos1;
	TLabel *Pos2;
	TLabel *Pos3;
	TLabel *Solution;
	TPanel *IndSol;
	TPanel *Panel213;
	TScrollBar *ScbSol;
	TButton *BtnSave;
	TPanel *Panel212;
	TLabel *Plabel0;
	TSplitter *Splitter1;
	TPanel *Panel221;
	TImage *Plot1;
	TSpeedButton *BtnFreqType1;
	TSplitter *Splitter2;
	TPanel *Panel222;
	TImage *Disp2;
	TImage *Plot2;
	TSpeedButton *BtnFreqType2;
	TImage *Disp1;
	TSpeedButton *BtnSolType;
	TSpeedButton *BtnPlotType1;
	TSpeedButton *BtnPlotType2;
	TSpeedButton *BtnPanel;
	TPanel *Pane6;
	TPanel *Panel22;
	TPanel *Panel5;
	TPanel *IndQ;
	TLabel *SolS;
	TLabel *SolQ;
	TSpeedButton *BtnSolType2;
	
	void __fastcall FormCreate        (TObject *Sender);
	void __fastcall FormShow          (TObject *Sender);
	void __fastcall FormClose         (TObject *Sender, TCloseAction &Action);
	
	void __fastcall TimerTimer        (TObject *Sender);
	
	void __fastcall BtnStartClick     (TObject *Sender);
	void __fastcall BtnStopClick      (TObject *Sender);
	void __fastcall BtnPlotClick      (TObject *Sender);
	void __fastcall BtnOptClick       (TObject *Sender);
	void __fastcall BtnExitClick      (TObject *Sender);
	
	void __fastcall BtnTimeSysClick   (TObject *Sender);
	void __fastcall BtnInputStrClick  (TObject *Sender);
	void __fastcall BtnOutputStrClick (TObject *Sender);
	void __fastcall BtnLogStrClick    (TObject *Sender);
	void __fastcall BtnSolTypeClick   (TObject *Sender);
	void __fastcall BtnPlotType1Click  (TObject *Sender);
	
	void __fastcall BtnMonitorClick   (TObject *Sender);
	void __fastcall BtnSaveClick      (TObject *Sender);
	void __fastcall BtnAboutClick     (TObject *Sender);
	void __fastcall BtnTaskTrayClick  (TObject *Sender);
	
	void __fastcall MenuExpandClick   (TObject *Sender);
	void __fastcall MenuStartClick    (TObject *Sender);
	void __fastcall MenuStopClick     (TObject *Sender);
	void __fastcall MenuPlotClick     (TObject *Sender);
	void __fastcall MenuMonitorClick  (TObject *Sender);
	void __fastcall MenuExitClick     (TObject *Sender);
	
	void __fastcall ScbSolChange      (TObject *Sender);
	
	void __fastcall TrayIconDblClick  (TObject *Sender);
	void __fastcall BtnFreqType1Click(TObject *Sender);
	void __fastcall Panel221Resize(TObject *Sender);
	void __fastcall Panel4Resize(TObject *Sender);
	void __fastcall Panel21Resize(TObject *Sender);
	void __fastcall Panel222Resize(TObject *Sender);
	void __fastcall BtnPanelClick(TObject *Sender);
	void __fastcall BtnPlotType2Click(TObject *Sender);
	void __fastcall BtnFreqType2Click(TObject *Sender);
	void __fastcall Panel211Resize(TObject *Sender);
	void __fastcall Panel5Resize(TObject *Sender);
private:
	tle_t TLEData;

	void __fastcall UpdateLog    (int stat, gtime_t time, double *rr, float *qr,
								  double *rb, int ns, double age, double ratio);
	void __fastcall SvrStart     (void);
	void __fastcall SvrStop      (void);
	void __fastcall UpdatePanel  (void);
	void __fastcall UpdateTimeSys(void);
	void __fastcall UpdateSolType(void);
	void __fastcall UpdateFont   (void);
	void __fastcall UpdateTime   (void);
	void __fastcall UpdatePos    (void);
	void __fastcall UpdateStr    (void);
	void __fastcall DrawPlot     (TImage *plot, int type, int freq);
	void __fastcall UpdatePlot   (void);
	void __fastcall ChangePlot   (void);
	int  __fastcall ConfOverwrite(const char *path);
	
	void __fastcall DrawSnr      (TCanvas *c, int w, int h, int top, int index, int freq);
	void __fastcall DrawSat      (TCanvas *c, int w, int h, int x0, int y0, int index, int freq);
	void __fastcall DrawBL       (TCanvas *c, int w, int h);
	void __fastcall DrawSky      (TCanvas *c, int w, int h, int x0, int y0);
	void __fastcall DrawText     (TCanvas *c, int x, int y, UnicodeString s,
								  TColor color, int align);
	void __fastcall DrawArrow    (TCanvas *c, int x, int y, int siz,
								  int ang, TColor color);
	void __fastcall OpenMoniPort (int port);
	void __fastcall InitSolBuff  (void);
	void __fastcall SaveLog      (void);
	void __fastcall LoadNav      (nav_t *nav);
	void __fastcall SaveNav      (nav_t *nav);
	void __fastcall LoadOpt      (void);
	void __fastcall SaveOpt      (void);
	void __fastcall SetTrayIcon  (int index);
	int  __fastcall ExecCmd      (AnsiString cmd, int show);
	TColor __fastcall SnrColor   (int snr);
public:
	AnsiString IniFile;
	
	int PanelStack,PanelMode;
	int SvrCycle,SvrBuffSize,Scale,SolBuffSize,NavSelect,SavedSol;
	int NmeaReq,NmeaCycle,InTimeTag,OutTimeTag,OutAppend,LogTimeTag,LogAppend;
	int TimeoutTime,ReconTime,SbasCorr,DgpsCorr,TideCorr,FileSwapMargin;
	int Stream[MAXSTRRTK],StreamC[MAXSTRRTK],Format[MAXSTRRTK];
	int CmdEna[3][2],CmdEnaTcp[3][2];
	int TimeSys,SolType,PlotType1,FreqType1,PlotType2,FreqType2;
	int MoniPort,OpenPort;
	
	int PSol,PSolS,PSolE,Nsat[2],SolCurrentStat;
	int Sat[2][MAXSAT],Snr[2][MAXSAT][NFREQ],Vsat[2][MAXSAT];
	double Az[2][MAXSAT],El[2][MAXSAT];
	gtime_t *Time;
	int *SolStat,*Nvsat;
	double *SolRov,*SolRef,*Qr,*VelRov,*Age,*Ratio;
	AnsiString Paths[MAXSTRRTK][4],Cmds[3][2],CmdsTcp[3][2];
	AnsiString InTimeStart,InTimeSpeed,ExSats;
	AnsiString RcvOpt[3],ProxyAddr;
	AnsiString OutSwapInterval,LogSwapInterval;
	prcopt_t PrcOpt;
	solopt_t SolOpt;
	TFont *PosFont;
	int DebugTraceF,DebugStatusF,OutputGeoidF,BaselineC;
	int RovPosTypeF,RefPosTypeF,RovAntPcvF,RefAntPcvF;
	AnsiString RovAntF,RefAntF,SatPcvFileF,AntPcvFileF;
	double RovAntDel[3],RefAntDel[3],RovPos[3],RefPos[3],NmeaPos[2];
	double Baseline[2];
	AnsiString History[10],MntpHist[10];
	
	AnsiString GeoidDataFileF,StaPosFileF,DCBFileF,EOPFileF,TLEFileF;
	AnsiString TLESatFileF,LocalDirectory,PntName[MAXMAPPNT];
	double PntPos[MAXMAPPNT][3];
	int NMapPnt;
	
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
