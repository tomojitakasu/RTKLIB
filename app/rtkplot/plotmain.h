//---------------------------------------------------------------------------
#ifndef plotmainH
#define plotmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include <jpeg.hpp>
#include "graph.h"
#include "console.h"
#include "rtklib.h"
#include <ExtDlgs.hpp>
#include "SHDocVw_OCX.h"
#include <Vcl.OleCtrls.hpp>
#include <Vcl.FileCtrl.hpp>

#define MAXNFILE    256                 // max number of solution files
#define MAXSTRBUFF  1024                // max length of stream buffer
#define MAXWAYPNT   99                  // max number of waypoints
#define MAXMAPPATH  4096                // max number of map paths
#define MAXMAPLAYER 12                  // max number of map layers

#define PRGNAME     "RTKPLOT"           // program name

#define CHARDEG     "\302\260"          // character code of degree (UTF-8)
#define CHARUP2     "\302\262"          // character code of ^2     (UTF-8)

#define DEFTSPAN    600.0               // default time span (s)
#define INTARROW    60.0                // direction arrow interval (s)
#define MAXTDIFF    60.0                // max differential time (s)
#define DOPLIM      30.0                // dop view limit
#define TTOL        DTTOL               // time-differnce tolerance (s)
#define TBRK        300.0               // time to recognize break (s)
#define THRESLIP    0.1                 // slip threshold of LG-jump (m)
#define SIZE_COMP   45                  // compass size (pixels)
#define SIZE_VELC   45                  // velocity circle size (pixels)
#define CLORANGE    (TColor)0x00AAFF
#define MIN_RANGE_GE 10.0               // min range for GE view

#define PLOT_TRK    0                   // plot-type: track-plot
#define PLOT_SOLP   1                   // plot-type: position-plot
#define PLOT_SOLV   2                   // plot-type: velocity-plot
#define PLOT_SOLA   3                   // plot-type: accel-plot
#define PLOT_NSAT   4                   // plot-type: number-of-satellite-plot
#define PLOT_RES    5                   // plot-type: residual-plot
#define PLOT_OBS    6                   // plot-type: observation-data-plot
#define PLOT_SKY    7                   // plot-type: sky-plot
#define PLOT_DOP    8                   // plot-type: dop-plot
#define PLOT_SNR    9                   // plot-type: snr/mp-plot
#define PLOT_SNRE   10                  // plot-type: snr/mp-el-plot
#define PLOT_MPS    11                  // plot-type: mp-skyplot

#define ORG_STARTPOS 0                  // plot-origin: start position
#define ORG_ENDPOS  1                   // plot-origin: end position
#define ORG_AVEPOS  2                   // plot-origin: average position
#define ORG_FITPOS  3                   // plot-origin: linear-fit position
#define ORG_REFPOS  4                   // plot-origin: reference position
#define ORG_LLHPOS  5                   // plot-origin: lat/lon/hgt position
#define ORG_AUTOPOS 6                   // plot-origin: auto-input position
#define ORG_IMGPOS  7                   // plot-origin: image-center position
#define ORG_MAPPOS  8                   // plot-origin: map center position
#define ORG_PNTPOS  9                   // plot-origin: way-point position

#define TRACEFILE   "rtkplot.trace"     // trace file
#define QCTMPFILE   "rtkplot_qc.temp"   // tempolary file for qc
#define QCERRFILE   "rtkplot_qc.err"    // error file for qc

#define SQR(x)      ((x)*(x))
#define SQRT(x)     ((x)<0.0||(x)!=(x)?0.0:sqrt(x))
#define MAX(x,y)    ((x)>(y)?(x):(y))
#define MIN(x,y)    ((x)<(y)?(x):(y))

extern const char *PTypes[];

// time-position class ------------------------------------------------------
class TIMEPOS
{
private:
    int nmax_;
public:
    int n;
    gtime_t *t;
    double *x,*y,*z,*xs,*ys,*zs,*xys;
    int *q;
    TIMEPOS(int nmax, int sflg);
    ~TIMEPOS();
    TIMEPOS *tdiff(void);
    TIMEPOS *diff(const TIMEPOS *pos2, int qflag);
};

// rtkplot class ------------------------------------------------------------
class TPlot : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel10;
	TPanel *Panel11;
	TPanel *Panel12;
	TPanel *Panel2;
	TPanel *Panel21;
	TPanel *Panel22;
	TPanel *StrStatus1;
	TPanel *StrStatus2;
	TPaintBox *Disp;
	
	TComboBox *PlotTypeS;
	TComboBox *QFlag;
	TComboBox *ObsType;
	TComboBox *DopType;
	TComboBox *SatList;
	TListBox *RangeList;
	TSpeedButton *BtnConnect;
	TSpeedButton *BtnSol1;
	TSpeedButton *BtnSol2;
	TSpeedButton *BtnSol12;
	TSpeedButton *BtnOn1;
	TSpeedButton *BtnOn2;
	TSpeedButton *BtnOn3;
	TSpeedButton *BtnRangeList;
	TSpeedButton *BtnFitHoriz;
	TSpeedButton *BtnFitVert;
	TSpeedButton *BtnCenterOri;
	TSpeedButton *BtnFixHoriz;
	TSpeedButton *BtnFixVert;
	TSpeedButton *BtnShowTrack;
	TSpeedButton *BtnShowSkyplot;
	TSpeedButton *BtnShowImg;
	TSpeedButton *BtnShowMap;
	TSpeedButton *BtnAnimate;
	TSpeedButton *BtnClear;
	TSpeedButton *BtnReload;
	TScrollBar *TimeScroll;
	
	TLabel *ConnectMsg;
	TLabel *Message1;
	TLabel *Message2;
	TLabel *QL1;
	TLabel *QL2;
	TLabel *QL3;
	TLabel *QL4;
	TLabel *QL5;
	TLabel *QL6;
	TLabel *QL7;
	TMainMenu *hh;
	
	TMenuItem *MenuFile;
	TMenuItem *MenuOpenSol1;
	TMenuItem *MenuOpenSol2;
	TMenuItem *MenuOpenMapImage;
	TMenuItem *MenuOpenShape;
	TMenuItem *MenuOpenObs;
	TMenuItem *MenuOpenNav;
	TMenuItem *MenuOpenElevMask;
	TMenuItem *MenuConnect;
	TMenuItem *MenuDisconnect;
	TMenuItem *MenuPort;
	TMenuItem *MenuReload;
	TMenuItem *MenuClear;
	TMenuItem *MenuQuit;
	
	TMenuItem *MenuEdit;
	TMenuItem *MenuTime;
	TMenuItem *MenuMapImg;
	TMenuItem *MenuWaypnt;
	TMenuItem *MenuSrcSol;
	TMenuItem *MenuSrcObs;
	TMenuItem *MenuQcObs;
	TMenuItem *MenyCopy;
	TMenuItem *MenuOptions;
	
	TMenuItem *MenuView;
	TMenuItem *MenuToolBar;
	TMenuItem *MenuStatusBar;
	TMenuItem *MenuMonitor1;
	TMenuItem *MenuMonitor2;
	TMenuItem *MenuCenterOri;
	TMenuItem *MenuFitHoriz;
	TMenuItem *MenuFitVert;
	TMenuItem *MenuShowTrack;
	TMenuItem *MenuFixHoriz;
	TMenuItem *MenuFixVert;
	TMenuItem *MenuShowImg;
	TMenuItem *MenuShowMap;
	TMenuItem *MenuAnimStart;
	TMenuItem *MenuAnimStop;
	
	TMenuItem *MenuHelp;
	TMenuItem *MenuAbout;
	
	TMenuItem *N1;
	TMenuItem *N2;
	TMenuItem *N3;
	TMenuItem *N5;
	TMenuItem *N6;
	TMenuItem *N7;
	TMenuItem *N8;
	TMenuItem *N9;
	TMenuItem *N10;
	TMenuItem *N11;
	TMenuItem *N12;
	TMenuItem *N13;
	
	TTimer *Timer;
	
	TOpenDialog *OpenSolDialog;
	TOpenDialog *OpenObsDialog;
	TOpenDialog *OpenElMaskDialog;
	TOpenDialog *OpenMapPathDialog;
	TOpenDialog *OpenMapDialog;
	TMenuItem *N14;
	TMenuItem *MenuSaveDop;
	TSaveDialog *SaveDialog;
	TMenuItem *MenuSaveImage;
	TSavePictureDialog *SaveImageDialog;
	TMenuItem *N15;
	TMenuItem *MenuGE;
	TSpeedButton *BtnGE;
	TComboBox *FrqType;
	TComboBox *ObsType2;
	TMenuItem *MenuVisAna;
	TMenuItem *N16;
	TSpeedButton *BtnOptions;
	TSpeedButton *BtnFixCent;
	TMenuItem *MenuFixCent;
	TSpeedButton *BtnGM;
	TMenuItem *MenuSaveSnrMp;
	TMenuItem *MenuGM;
	TMenuItem *MenuOpenSkyImage;
	TMenuItem *MenuSkyImg;
	TMenuItem *MenuShowSkyplot;
	TMenuItem *Windows1;
	TMenuItem *MenuMax;
	TMenuItem *MenuPlotGE;
	TMenuItem *MenuPlotGM;
	TMenuItem *MenuPlotGEGM;
	TMenuItem *N17;
	TPanel *Panel101;
	TPanel *Panel102;
	TPanel *Panel103;
	TPanel *Panel104;
	TMenuItem *MenuSaveElMask;
	TMenuItem *MenuMapLayer;
	TSpeedButton *BtnMessage2;
	TMenuItem *MenuOpenWaypoint;
	TMenuItem *MenuSaveWaypoint;
	TOpenDialog *OpenWaypointDialog;
	TSaveDialog *SaveWaypointDialog;
	TPanel *Panel3;
	TPanel *PanelBrowse;
	TSplitter *Splitter1;
	TDriveComboBox *DriveSel;
	TDirectoryListBox *DirSel;
	TSplitter *Splitter2;
	TFileListBox *FileList;
	TComboBox *FileMask;
	TMenuItem *MenuBrowse;
	TPanel *StrStatus;
	TSpeedButton *BtnShowGrid;
	TMenuItem *MenuShowGrid;
	TPanel *Panel4;
	TSpeedButton *BtnUdList;
	
	void __fastcall FormCreate			(TObject *Sender);
	void __fastcall FormShow			(TObject *Sender);
	void __fastcall FormResize			(TObject *Sender);
	void __fastcall FormActivate		(TObject *Sender);
	void __fastcall FormKeyDown			(TObject *Sender, WORD &Key, TShiftState Shift);
	
	void __fastcall MenuOpenSol1Click	(TObject *Sender);
	void __fastcall MenuOpenSol2Click	(TObject *Sender);
	void __fastcall MenuOpenMapImageClick(TObject *Sender);
	void __fastcall MenuOpenShapeClick (TObject *Sender);
	void __fastcall MenuOpenObsClick	(TObject *Sender);
	void __fastcall MenuOpenNavClick	(TObject *Sender);
	void __fastcall MenuOpenElevMaskClick(TObject *Sender);
	void __fastcall MenuConnectClick	(TObject *Sender);
	void __fastcall MenuDisconnectClick (TObject *Sender);
	void __fastcall MenuPortClick		(TObject *Sender);
	void __fastcall MenuReloadClick		(TObject *Sender);
	void __fastcall MenuClearClick		(TObject *Sender);
	void __fastcall MenuQuitClick		(TObject *Sender);
	
	void __fastcall MenuTimeClick		(TObject *Sender);
	void __fastcall MenuMapImgClick		(TObject *Sender);
	void __fastcall MenuWaypointClick	(TObject *Sender);
	void __fastcall MenuSrcSolClick		(TObject *Sender);
	void __fastcall MenuSrcObsClick		(TObject *Sender);
	void __fastcall MenuQcObsClick		(TObject *Sender);
	void __fastcall MenyCopyClick		(TObject *Sender);
	void __fastcall MenuOptionsClick	(TObject *Sender);
	
	void __fastcall MenuToolBarClick	(TObject *Sender);
	void __fastcall MenuStatusBarClick	(TObject *Sender);
	void __fastcall MenuMonitor1Click	(TObject *Sender);
	void __fastcall MenuMonitor2Click	(TObject *Sender);
	void __fastcall MenuCenterOriClick	(TObject *Sender);
	void __fastcall MenuFitHorizClick	(TObject *Sender);
	void __fastcall MenuFitVertClick	(TObject *Sender);
	void __fastcall MenuShowTrackClick	(TObject *Sender);
	void __fastcall MenuFixHorizClick	(TObject *Sender);
	void __fastcall MenuFixVertClick	(TObject *Sender);
	void __fastcall MenuShowImgClick	(TObject *Sender);
	void __fastcall MenuShowMapClick	(TObject *Sender);
	void __fastcall MenuAnimStartClick	(TObject *Sender);
	void __fastcall MenuAnimStopClick	(TObject *Sender);
	void __fastcall MenuAboutClick		(TObject *Sender);
	
	void __fastcall BtnConnectClick		(TObject *Sender);
	void __fastcall BtnSol1Click		(TObject *Sender);
	void __fastcall BtnSol2Click		(TObject *Sender);
	void __fastcall BtnSol12Click		(TObject *Sender);
	void __fastcall BtnSol1DblClick		(TObject *Sender);
	void __fastcall BtnSol2DblClick		(TObject *Sender);
	void __fastcall BtnShowImgClick		(TObject *Sender);
	void __fastcall BtnOn1Click			(TObject *Sender);
	void __fastcall BtnOn2Click			(TObject *Sender);
	void __fastcall BtnOn3Click			(TObject *Sender);
	void __fastcall BtnRangeListClick	(TObject *Sender);
	void __fastcall RangeListClick		(TObject *Sender);
	void __fastcall BtnCenterOriClick	(TObject *Sender);
	void __fastcall BtnFitHorizClick	(TObject *Sender);
	void __fastcall BtnFitVertClick		(TObject *Sender);
	void __fastcall BtnShowTrackClick	(TObject *Sender);
	void __fastcall BtnFixHorizClick	(TObject *Sender);
	void __fastcall BtnFixVertClick		(TObject *Sender);
	void __fastcall BtnShowMapClick	(TObject *Sender);
	void __fastcall BtnAnimateClick		(TObject *Sender);
	void __fastcall BtnClearClick		(TObject *Sender);
	void __fastcall BtnReloadClick		(TObject *Sender);
	
	void __fastcall PlotTypeSChange		(TObject *Sender);
	void __fastcall QFlagChange			(TObject *Sender);
	void __fastcall ObsTypeChange		(TObject *Sender);
	void __fastcall DopTypeChange		(TObject *Sender);
	void __fastcall SatListChange		(TObject *Sender);
	void __fastcall TimeScrollChange	(TObject *Sender);
	
	void __fastcall TimerTimer			(TObject *Sender);
	
	void __fastcall DispPaint			(TObject *Sender);
	void __fastcall DispMouseLeave		(TObject *Sender);
	void __fastcall DispMouseMove		(TObject *Sender, TShiftState Shift,
										 int X, int Y);
	void __fastcall DispMouseDown		(TObject *Sender, TMouseButton Button,
										 TShiftState Shift, int X, int Y);
	void __fastcall DispMouseUp			(TObject *Sender, TMouseButton Button,
										 TShiftState Shift, int X, int Y);
	void __fastcall MouseWheel			(TObject *Sender, TShiftState Shift,
										 int WheelDelta, TPoint &MousePos, bool &Handled);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall MenuSaveDopClick(TObject *Sender);
	void __fastcall MenuSaveImageClick(TObject *Sender);
	void __fastcall MenuGEClick(TObject *Sender);
	void __fastcall BtnGEClick(TObject *Sender);
	void __fastcall MenuVisAnaClick(TObject *Sender);
	void __fastcall BtnOptionsClick(TObject *Sender);
	void __fastcall BtnFixCentClick(TObject *Sender);
	void __fastcall MenuFixCentClick(TObject *Sender);
	void __fastcall BtnGMClick(TObject *Sender);
	void __fastcall MenuGMClick(TObject *Sender);
	void __fastcall MenuSaveSnrMpClick(TObject *Sender);
	void __fastcall MenuOpenSkyImageClick(TObject *Sender);
	void __fastcall MenuSkyImgClick(TObject *Sender);
	void __fastcall MenuShowSkyplotClick(TObject *Sender);
	void __fastcall BtnShowSkyplotClick(TObject *Sender);
	void __fastcall MenuPlotGEClick(TObject *Sender);
	void __fastcall MenuPlotGMClick(TObject *Sender);
	void __fastcall MenuPlotGEGMClick(TObject *Sender);
	void __fastcall MenuMaxClick(TObject *Sender);
	void __fastcall DispGesture(TObject *Sender, const TGestureEventInfo &EventInfo,
          bool &Handled);
	void __fastcall MenuSaveElMaskClick(TObject *Sender);
	void __fastcall MenuMapLayerClick(TObject *Sender);
	void __fastcall BtnMessage2Click(TObject *Sender);
	void __fastcall MenuOpenWaypointClick(TObject *Sender);
	void __fastcall MenuSaveWaypointClick(TObject *Sender);
	void __fastcall DispDblClick(TObject *Sender);
	void __fastcall FileMaskChange(TObject *Sender);
	void __fastcall FileListClick(TObject *Sender);
	void __fastcall Splitter1Moved(TObject *Sender);
	void __fastcall MenuBrowseClick(TObject *Sender);
	void __fastcall MenuShowGridClick(TObject *Sender);
	void __fastcall BtnShowGridClick(TObject *Sender);
	void __fastcall BtnUdListClick(TObject *Sender);


protected:
	void __fastcall CMDialogKey(Messages::TWMKey &Message);

private:
	Graphics::TBitmap *Buff;
    Graphics::TBitmap *MapImage;
    Graphics::TBitmap *SkyImageI;
    TGraph *GraphT;
    TGraph *GraphG[3];
    TGraph *GraphR;
    TGraph *GraphS;
    TGraph *GraphE[2];
    TStrings *OpenFiles;
    TStrings *SolFiles[2];
    TStrings *ObsFiles;
    TStrings *NavFiles;
    TConsole *Console1,*Console2;
    
    stream_t Stream[2];
    stream_t StrTimeSync;
    solbuf_t SolData[2];
    solstatbuf_t SolStat[2];
    int SolIndex[2];
    obs_t Obs;
    nav_t Nav;
    sta_t Sta;
    double *Az,*El,*Mp[NFREQ+NEXOBS];
    char StrBuff[1024];
    int NStrBuff;
    
    gtime_t OEpoch;
    int FormWidth,FormHeight;
    int ConnectState,OpenRaw;
    int NObs,*IndexObs,ObsIndex;
    int Week;
    int Flush,PlotType;
    int NSolF1,NSolF2,NObsF,NNavF;
    int SatMask[MAXSAT],SatSel[MAXSAT];
    int SimObs;
    
    int Drag,Xn,Yn;
    double X0,Y0,Xc,Yc,Xs,Ys,Xcent,Xcent0;
    unsigned int MouseDownTick;
    
    int GEState,GEDataState[2];
    double GEHeading;
    
    void __fastcall DropFiles    (TWMDropFiles msg);
    
    void __fastcall ReadSolStat  (TStrings *files, int sel);
    int  __fastcall ReadObsRnx   (TStrings *files, obs_t *obs, nav_t *nav, sta_t *sta);
    void __fastcall ReadMapTag   (AnsiString file);
    void __fastcall ReadShapeFile(TStrings *files);
    void __fastcall GenVisData   (void);
    void __fastcall SaveDop      (AnsiString file);
    void __fastcall SaveSnrMp    (AnsiString file);
    void __fastcall SaveElMask   (AnsiString file);
    void __fastcall Connect      (void);
    void __fastcall Disconnect   (void);
    void __fastcall ConnectPath  (const char *path, int ch);
    int  __fastcall CheckObs     (AnsiString file);
    void __fastcall UpdateObs    (int nobs);
    void __fastcall UpdateMp     (void);
    void __fastcall ClearObs     (void);
    void __fastcall ClearSol     (void);
    void __fastcall Clear        (void);
    void __fastcall Refresh      (void);
    void __fastcall Reload       (void);
    void __fastcall ReadWaitStart(void);
    void __fastcall ReadWaitEnd  (void);
    
    void __fastcall UpdateDisp   (void);
    void __fastcall UpdateType   (int type);
    void __fastcall UpdatePlotType(void);
    void __fastcall UpdateSatList(void);
    void __fastcall UpdateObsType(void);
    void __fastcall UpdateSize   (void);
    void __fastcall UpdateColor  (void);
    void __fastcall UpdateTime   (void);
    void __fastcall UpdateOrigin (void);
    void __fastcall UpdateSatMask(void);
    void __fastcall UpdateSatSel (void);
    void __fastcall UpdateInfo   (void);
    void __fastcall UpdateTimeSol(void);
    void __fastcall UpdateTimeObs(void);
    void __fastcall UpdateInfoSol(void);
    void __fastcall UpdateInfoObs(void);
    void __fastcall UpdatePoint  (int x, int y);
    void __fastcall UpdateEnable (void);
    void __fastcall FitTime      (void);
    void __fastcall SetRange     (int all, double range);
    void __fastcall FitRange     (int all);
    
    void __fastcall SetCentX     (double c);
    void __fastcall SetScaleX    (double s);
    void __fastcall MouseDownTrk (int X, int Y);
    void __fastcall MouseDownSol (int X, int Y);
    void __fastcall MouseDownObs (int X, int Y);
    void __fastcall MouseMoveTrk (int X, int Y, double dx, double dy, double dxs, double dys);
    void __fastcall MouseMoveSol (int X, int Y, double dx, double dy, double dxs, double dys);
    void __fastcall MouseMoveObs (int X, int Y, double dx, double dy, double dxs, double dys);

    void __fastcall DrawTrk      (int level);
    void __fastcall DrawTrkImage (int level);
    void __fastcall DrawTrkMap   (int level);
    void __fastcall DrawTrkPnt   (const TIMEPOS *pos, int level, int style);
    void __fastcall DrawTrkPos   (const double *rr, int type, int siz, TColor color, AnsiString label);
    void __fastcall DrawTrkStat  (const TIMEPOS *pos, AnsiString header, int p);
    void __fastcall DrawTrkError (const TIMEPOS *pos, int style);
    void __fastcall DrawTrkArrow (const TIMEPOS *pos);
    void __fastcall DrawTrkVel   (const TIMEPOS *vel);
    void __fastcall DrawLabel    (TGraph *g, TPoint p, AnsiString label, int ha, int va);
    void __fastcall DrawMark     (TGraph *g, TPoint p, int mark, TColor color, int size,
                                  int rot);
    void __fastcall DrawSol      (int level, int type);
    void __fastcall DrawSolPnt   (const TIMEPOS *pos, int level, int style);
    void __fastcall DrawSolStat  (const TIMEPOS *pos, AnsiString unit, int p);
    void __fastcall DrawNsat     (int level);
    void __fastcall DrawRes      (int level);
    void __fastcall DrawPolyS    (TGraph *graph, double *x, double *y, int n,
                                  TColor color, int style);
    
    void __fastcall DrawObs      (int level);
    void __fastcall DrawObsSlip  (double *yp);
    void __fastcall DrawObsEphem (double *yp);
    void __fastcall DrawSkyImage (int level);
    void __fastcall DrawSky      (int level);
    void __fastcall DrawDop      (int level);
    void __fastcall DrawDopStat  (double *dop, int *ns, int n);
    void __fastcall DrawSnr      (int level);
    void __fastcall DrawSnrE     (int level);
    void __fastcall DrawMpS      (int level);
    
    AnsiString __fastcall U2A    (UnicodeString str);
    UnicodeString __fastcall A2U (AnsiString str);
    
    TIMEPOS * __fastcall SolToPos (solbuf_t *sol, int index, int qflag, int type);
    TIMEPOS * __fastcall SolToNsat(solbuf_t *sol, int index, int qflag);
    
    void __fastcall PosToXyz     (gtime_t time, const double *rr, int type, double *xyz);
    void __fastcall CovToXyz     (const double *rr, const float *qr, int type,
                                  double *xyzs);
    void __fastcall CalcStats    (const double *x, int n, double ref, double &ave,
                                  double &std, double &rms);
    int __fastcall  FitPos       (gtime_t *time, double *opos, double *ovel);
    
    AnsiString __fastcall LatLonStr(const double *pos, int ndec);
    TColor __fastcall ObsColor   (const obsd_t *obs, double az, double el);
    TColor __fastcall SysColor   (int sat);
    TColor __fastcall SnrColor   (double snr);
    TColor __fastcall MpColor    (double mp);
    void __fastcall ReadStaPos   (const char *file, const char *sta, double *rr);
    int  __fastcall SearchPos    (int x, int y);
    void __fastcall TimeSpan     (gtime_t *ts, gtime_t *te, double *tint);
    double __fastcall TimePos    (gtime_t time);
    void __fastcall TimeStr(gtime_t time, int n, int tsys, char *str);
    int  __fastcall ExecCmd      (AnsiString cmd);
    void __fastcall ShowMsg      (AnsiString msg);
    void __fastcall ShowLegend   (AnsiString *msgs);
    void __fastcall LoadOpt      (void);
    void __fastcall SaveOpt      (void);
    
    BEGIN_MESSAGE_MAP // for files drop, cursor key down event
    MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,DropFiles);
    MESSAGE_HANDLER(CM_DIALOGKEY,TWMKey,CMDialogKey);
    END_MESSAGE_MAP(TForm);
    
public:
    Graphics::TBitmap *SkyImageR;
    AnsiString IniFile;
    AnsiString MapImageFile;
    AnsiString SkyImageFile;
    AnsiString RnxOpts;
    AnsiString ApiKey;
    tle_t TLEData;
    gis_t Gis;
    
    // connection settings
    int RtStream[2];
    AnsiString RtPath1,RtPath2;
    AnsiString StrPaths[2][3];
    AnsiString StrCmds[2][2];
    int StrCmdEna[2][2];
    int RtFormat[2];
    int RtConnType;
    int RtTimeForm;
    int RtDegForm;
    AnsiString RtFieldSep;
    int RtTimeOutTime;
    int RtReConnTime;
    double ElMaskData[361];
    
    // time options 
    int TimeEna[3];
    gtime_t TimeStart;
    gtime_t TimeEnd;
    double TimeInt;
    
    // map options 
    int MapSize[2],MapScaleEq;
    double MapScaleX,MapScaleY;
    double MapLat,MapLon;
    int PointType;
    
    // sky image options 
    int SkySize[2],SkyDestCorr,SkyElMask,SkyRes,SkyFlip,SkyBinarize;
    double SkyCent[2],SkyScale,SkyScaleR,SkyFov[3],SkyDest[10];
    double SkyBinThres1,SkyBinThres2;
    
    // plot options 
    int TimeLabel;
    int LatLonFmt;
    int ShowStats;
    int ShowSlip;
    int ShowHalfC;
    int ShowEph;
    double ElMask;
    int ElMaskP;
    int HideLowSat;
    double MaxDop,MaxMP;
    int NavSys;
    AnsiString ExSats;
    int ShowErr;
    int ShowArrow;
    int ShowGLabel;
    int ShowLabel;
    int ShowCompass;
    int ShowScale;
    int AutoScale;
    double YRange;
    int RtBuffSize;
    int TimeSyncOut;
    int TimeSyncPort;
    int Origin;
    int RcvPos;
    double OOPos[3];
    TColor MColor[2][8]; // {{mark1 0-7},{mark2 0-7}}
    TColor CColor[4];    // {background,grid,text,line}
    TColor MapColor[MAXMAPLAYER]; // mapcolors
    int PlotStyle;
    int MarkSize;
    int AnimCycle;
    int RefCycle;
    int Trace;
    AnsiString QcCmd,QcOpt;
    AnsiString TLEFile;
    AnsiString TLESatFile;
    
    AnsiString Title;
    AnsiString PntName[MAXWAYPNT];
    double PntPos[MAXWAYPNT][3];
    int NWayPnt,SelWayPnt;
    int OPosType;
    double OPos[3],OVel[3];
    
    AnsiString StrHistory [10];
    AnsiString StrMntpHist[10];
    
    __fastcall TPlot(TComponent* Owner);
    void __fastcall ReadSol    (TStrings *files, int sel);
    void __fastcall ReadObs    (TStrings *files);
    void __fastcall ReadNav    (TStrings *files);
    void __fastcall ReadMapData(AnsiString file);
    void __fastcall ReadSkyData(AnsiString file);
    void __fastcall ReadSkyTag (AnsiString file);
    void __fastcall ReadWaypoint(AnsiString file);
    void __fastcall SaveWaypoint(AnsiString file);
    void __fastcall UpdateSky  (void);
    void __fastcall ReadElMaskData(AnsiString file);
    int __fastcall GetCurrentPos(double *rr);
    int __fastcall GetCenterPos(double *rr);
    void __fastcall SetTrkCent(double lat, double lon);
    void __fastcall UpdatePlot(void);
    void __fastcall Refresh_GEView(void);
	void __fastcall Refresh_GMView(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TPlot *Plot;

//---------------------------------------------------------------------------
#endif
