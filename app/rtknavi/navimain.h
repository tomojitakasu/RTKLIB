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
#include <System.ImageList.hpp>

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
	TSpeedButton *BtnTaskTray;
	TSpeedButton *BtnAbout;
	TSpeedButton *BtnMonitor;
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
	TPanel *Panel122;
	TSpeedButton *BtnOutputStr;
	TPanel *Panel123;
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
	TPanel *Panel22;
	TImage *Plot1;
	TSpeedButton *BtnFreqType1;
	TSplitter *Splitter2;
	TPanel *Panel23;
	TImage *Disp2;
	TImage *Plot2;
	TSpeedButton *BtnFreqType2;
	TImage *Disp1;
	TSpeedButton *BtnSolType;
	TSpeedButton *BtnPlotType1;
	TSpeedButton *BtnPlotType2;
	TSpeedButton *BtnPanel;
	TPanel *Pane6;
	TPanel *Panel5;
	TPanel *IndQ;
	TLabel *SolS;
	TLabel *SolQ;
	TSpeedButton *BtnSolType2;
	TBitBtn *BtnStart;
	TBitBtn *BtnMark;
	TBitBtn *BtnPlot;
	TBitBtn *BtnOpt;
	TBitBtn *BtnExit;
	TBitBtn *BtnStop;
	TSpeedButton *BtnExpand1;
	TSpeedButton *BtnShrink1;
	TSpeedButton *BtnExpand2;
	TSpeedButton *BtnShrink2;
	TPanel *Panel24;
	TImage *Disp3;
	TImage *Plot3;
	TSpeedButton *BtnFreqType3;
	TSpeedButton *BtnPlotType3;
	TSpeedButton *BtnExpand3;
	TSpeedButton *BtnShrink3;
	TPanel *Panel25;
	TImage *Disp4;
	TImage *Plot4;
	TSpeedButton *BtnFreqType4;
	TSpeedButton *BtnPlotType4;
	TSpeedButton *BtnExpand4;
	TSpeedButton *BtnShrink4;
	TSplitter *Splitter3;
	TSplitter *Splitter4;
	
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
	void __fastcall Panel22Resize(TObject *Sender);
	void __fastcall Panel4Resize(TObject *Sender);
	void __fastcall Panel21Resize(TObject *Sender);
	void __fastcall Panel23Resize(TObject *Sender);
	void __fastcall BtnPanelClick(TObject *Sender);
	void __fastcall BtnPlotType2Click(TObject *Sender);
	void __fastcall BtnFreqType2Click(TObject *Sender);
	void __fastcall Panel211Resize(TObject *Sender);
	void __fastcall Panel5Resize(TObject *Sender);
	void __fastcall BtnExpand1Click(TObject *Sender);
	void __fastcall BtnShrink1Click(TObject *Sender);
	void __fastcall BtnExpand2Click(TObject *Sender);
	void __fastcall BtnShrink2Click(TObject *Sender);
	void __fastcall BtnMarkClick(TObject *Sender);
	void __fastcall Panel24Resize(TObject *Sender);
	void __fastcall Panel25Resize(TObject *Sender);
	void __fastcall BtnPlotType3Click(TObject *Sender);
	void __fastcall BtnFreqType3Click(TObject *Sender);
	void __fastcall BtnPlotType4Click(TObject *Sender);
	void __fastcall BtnFreqType4Click(TObject *Sender);
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
	void __fastcall UpdateEnable (void);
	void __fastcall ChangePlot   (void);
	int  __fastcall ConfOverwrite(const char *path);
	
	void __fastcall DrawSnr      (TCanvas *c, int w, int h, int x0, int y0, int index, int freq);
	void __fastcall DrawSat      (TCanvas *c, int w, int h, int x0, int y0, int index, int freq);
	void __fastcall DrawBL       (TImage *plot, int w, int h);
	void __fastcall DrawTrk      (TImage *plot);
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
	int NmeaReq,NmeaCycle,InTimeTag,InTime64Bit;
	int OutTimeTag,OutAppend,LogTimeTag,LogAppend;
	int TimeoutTime,ReconTime,SbasCorr,DgpsCorr,TideCorr,FileSwapMargin;
	int Stream[MAXSTRRTK],StreamC[MAXSTRRTK],Format[MAXSTRRTK];
	int CmdEna[3][3],CmdEnaTcp[3][3];
	int TimeSys,SolType;
	int PlotType1,FreqType1,PlotType2,FreqType2;
	int PlotType3,FreqType3,PlotType4,FreqType4;
	int TrkType1,TrkType2,TrkType3,TrkType4;
	int TrkScale1,TrkScale2,TrkScale3,TrkScale4;
	int BLMode1,BLMode2,BLMode3,BLMode4;
	int MoniPort,OpenPort;
	
	int PSol,PSolS,PSolE,Nsat[2],SolCurrentStat;
	int Sat[2][MAXSAT],Snr[2][MAXSAT][NFREQ],Vsat[2][MAXSAT];
	double Az[2][MAXSAT],El[2][MAXSAT];
	gtime_t *Time;
	int *SolStat,*Nvsat;
	double *SolRov,*SolRef,*Qr,*VelRov,*Age,*Ratio;
	double TrkOri[3],MaxBL;
	AnsiString Paths[MAXSTRRTK][4],Cmds[3][3],CmdsTcp[3][3];
	AnsiString InTimeStart,InTimeSpeed,ExSats;
	AnsiString RcvOpt[3],ProxyAddr;
	AnsiString OutSwapInterval,LogSwapInterval,ResetCmd;
	prcopt_t PrcOpt;
	solopt_t SolOpt;
	TFont *PosFont;
	int DebugTraceF,DebugStatusF,OutputGeoidF,BaselineC;
	int RovPosTypeF,RefPosTypeF,RovAntPcvF,RefAntPcvF;
	AnsiString RovAntF,RefAntF,SatPcvFileF,AntPcvFileF;
	double RovAntDel[3],RefAntDel[3],RovPos[3],RefPos[3],NmeaPos[3];
	double Baseline[2];
	AnsiString History[10],MntpHist[10];
	
	AnsiString GeoidDataFileF,StaPosFileF,DCBFileF,EOPFileF,TLEFileF;
	AnsiString TLESatFileF,LocalDirectory,PntName[MAXMAPPNT];
	double PntPos[MAXMAPPNT][3];
	int NMapPnt;
	
	AnsiString MarkerName,MarkerComment;
	
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
