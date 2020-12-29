//---------------------------------------------------------------------------
#ifndef plotmainH
#define plotmainH
//---------------------------------------------------------------------------
#include <QTimer>
#include <QTime>
#include <QMainWindow>

#include "graph.h"
#include "console.h"
#include "rtklib.h"

#include "ui_plotmain.h"

#define MAXNFILE    256                 // max number of solution files
#define MAXSTRBUFF  1024                // max length of stream buffer
#define MAXWAYPNT   99                  // max number of waypoints
#define MAXMAPPATH  4096                // max number of map paths
#define MAXMAPLAYER 12                  // max number of map layers

#define PRGNAME     "RTKPLOT-QT"           // program name

const QChar degreeChar(0260);           // character code of degree (UTF-8)
const QChar up2Char(0262);              // character code of ^2     (UTF-8)

#define DEFTSPAN    600.0               // default time span (s)
#define INTARROW    60.0                // direction arrow interval (s)
#define MAXTDIFF    60.0                // max differential time (s)
#define DOPLIM      30.0                // dop view limit
#define TTOL        DTTOL               // time-differnce tolerance (s)
#define TBRK        300.0               // time to recognize break (s)
#define THRESLIP    0.1                 // slip threshold of LG-jump (m)
#define SIZE_COMP   45                  // compass size (pixels)
#define SIZE_VELC   45                  // velocity circle size (pixels)
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

extern const QString PTypes[];

class MapAreaDialog;
class GoogleEarthView;
class GoogleMapView;
class SpanDialog;
class ConnectDialog;
class SkyImgDialog;
class PlotOptDialog;
class AboutDialog;
class PntDialog;
class FileSelDialog;
class TextViewer;
class VecMapDialog;

class QEvent;
class QMouseEvent;
class QFocusEvent;
class QKeyEvent;
class QResizeEvent;
class QShowEvent;
class QWheelEvent;
class QPaintEVent;

// time-position class ------------------------------------------------------
class TIMEPOS
{
private:
    int nmax_;
    TIMEPOS(TIMEPOS &){}
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
class Plot : public QMainWindow, public Ui::Plot
{
    Q_OBJECT

protected:
        void  mouseMoveEvent		(QMouseEvent*);
        void  mousePressEvent		(QMouseEvent*);
        void  mouseReleaseEvent		(QMouseEvent*);
        void  mouseDoubleClickEvent (QMouseEvent*);
        void  wheelEvent			(QWheelEvent*);
        void  closeEvent            (QCloseEvent*);
        void  keyPressEvent			(QKeyEvent *);
        void  showEvent			    (QShowEvent*);
        void  resizeEvent			(QResizeEvent *);
        void  leaveEvent    		(QEvent*);
        void  dragEnterEvent        (QDragEnterEvent *event);
        void  dropEvent             (QDropEvent *event);
        void  paintEvent            (QPaintEvent *event);

public slots:
        void  MenuOpenSol1Click	();
        void  MenuOpenSol2Click	();
        void  MenuOpenMapImageClick();
        void  MenuOpenShapeClick ();
        void  MenuOpenObsClick	();
        void  MenuOpenNavClick	();
        void  MenuOpenElevMaskClick();
        void  MenuConnectClick	();
        void  MenuDisconnectClick ();
        void  MenuPortClick		();
        void  MenuReloadClick		();
        void  MenuClearClick		();
        void  MenuQuitClick		();
	
        void  MenuTimeClick		();
        void  MenuMapImgClick		();
        void  MenuWaypointClick	();
        void  MenuSrcSolClick		();
        void  MenuSrcObsClick		();
        void  MenuQcObsClick		();
        void  MenuCopyClick		();
        void  MenuOptionsClick	();
	
        void  MenuToolBarClick	();
        void  MenuStatusBarClick	();
        void  MenuMonitor1Click	();
        void  MenuMonitor2Click	();
        void  MenuCenterOriClick	();
        void  MenuFitHorizClick	();
        void  MenuFitVertClick	();
        void  MenuShowTrackClick	();
        void  MenuFixHorizClick	();
        void  MenuFixVertClick	();
        void  MenuShowMapClick	();
        void  MenuShowImgClick	();
        void  MenuAnimStartClick	();
        void  MenuAnimStopClick	();
        void  MenuAboutClick		();
	
        void  BtnConnectClick		();
        void  BtnSol1Click		();
        void  BtnSol2Click		();
        void  BtnSol12Click		();
        void  BtnSol1DblClick		();
        void  BtnSol2DblClick		();
        void  BtnOn1Click			();
        void  BtnOn2Click			();
        void  BtnOn3Click			();
        void  BtnRangeListClick	();
        void  BtnAnimateClick		();

        void  PlotTypeSChange		();
        void  QFlagChange			();
        void  ObsTypeChange		();
        void  DopTypeChange		();
        void  SatListChange		();
        void  TimeScrollChange	();
        void  RangeListClick		();

        void  TimerTimer			();
	
        void  MenuFileSelClick();
        void  MenuSaveDopClick();
        void  MenuSaveImageClick();
        void  MenuGEClick();
        void  MenuVisAnaClick();
        void  MenuFixCentClick();
        void  MenuGMClick();
        void  MenuSaveSnrMpClick();
        void  MenuOpenSkyImageClick();
        void  MenuSkyImgClick();
        void  MenuShowSkyplotClick();
        void  MenuPlotGEClick();
        void  MenuPlotGMClick();
        void  MenuPlotGEGMClick();
        void  MenuMaxClick();
        void  MenuSaveElMaskClick();
        void  MenuMapLayerClick();
        void  BtnMessage2Click();
        void  MenuOpenWaypointClick();
        void  MenuSaveWaypointClick();

private:
    QPixmap Buff;
    QImage MapImage;
    QImage SkyImageI;
    Graph *GraphT;
    Graph *GraphG[3];
    Graph *GraphR;
    Graph *GraphS;
    Graph *GraphE[2];
    Console *Console1,*Console2;
    QStringList OpenFiles;
    QStringList SolFiles[2];
    QStringList ObsFiles;
    QStringList NavFiles;

    stream_t Stream[2];
    solbuf_t SolData[2];
    solstatbuf_t SolStat[2];
    int SolIndex[2];
    obs_t Obs;
    nav_t Nav;
    sta_t Sta;
    double *Az,*El,*Mp[NFREQ+NEXOBS];
    QTimer Timer;
    QTime updateTime;
    
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
    
    void  ReadSolStat  (const QStringList &files, int sel);
    int   ReadObsRnx   (const QStringList &files, obs_t *obs, nav_t *nav, sta_t *sta);
    void  ReadMapTag   (const QString &file);
    void  ReadShapeFile(const QStringList &file);
    void  GenVisData   (void);
    void  SaveDop      (const QString &file);
    void  SaveSnrMp    (const QString &file);
    void  SaveElMask   (const QString &file);
    void  Connect      (void);
    void  Disconnect   (void);
    void  ConnectPath  (const QString &path, int ch);
    int   CheckObs     (const QString &file);
    void  UpdateObs    (int nobs);
    void  UpdateMp     (void);
    void  ClearObs     (void);
    void  ClearSol     (void);
    void  Clear        (void);
    void  Refresh      (void);
    void  Reload       (void);
    void  ReadWaitStart(void);
    void  ReadWaitEnd  (void);
    
    void  UpdateDisp   (void);
    void  UpdateType   (int type);
    void  UpdatePlotType(void);
    void  UpdateSatList(void);
    void  UpdateObsType(void);
    void  UpdateSize   (void);
    void  UpdateColor  (void);
    void  UpdateTime   (void);
    void  UpdateOrigin (void);
    void  UpdateSatMask(void);
    void  UpdateSatSel (void);
    void  UpdateInfo   (void);
    void  UpdateTimeSol(void);
    void  UpdateTimeObs(void);
    void  UpdateInfoSol(void);
    void  UpdateInfoObs(void);
    void  UpdatePoint  (int x, int y);
    void  UpdateEnable (void);
    void  FitTime      (void);
    void  SetRange     (int all, double range);
    void  FitRange     (int all);
    
    void  SetCentX     (double c);
    void  SetScaleX    (double s);
    void  MouseDownTrk (int X, int Y);
    void  MouseDownSol (int X, int Y);
    void  MouseDownObs (int X, int Y);
    void  MouseMoveTrk (int X, int Y, double dx, double dy, double dxs, double dys);
    void  MouseMoveSol (int X, int Y, double dx, double dy, double dxs, double dys);
    void  MouseMoveObs (int X, int Y, double dx, double dy, double dxs, double dys);

    void  DrawTrk      (QPainter &g,int level);
    void  DrawTrkImage (QPainter &g,int level);
    void  DrawTrkMap   (QPainter &g,int level);
    void  DrawTrkPath  (QPainter &g,int level);
    void  DrawTrkPnt   (QPainter &g,const TIMEPOS *pos, int level, int style);
    void  DrawTrkPos   (QPainter &g,const double *rr, int type, int siz, QColor color, const QString &label);
    void  DrawTrkStat  (QPainter &g,const TIMEPOS *pos, const QString &header, int p);
    void  DrawTrkError (QPainter &g,const TIMEPOS *pos, int style);
    void  DrawTrkArrow (QPainter &g,const TIMEPOS *pos);
    void  DrawTrkVel   (QPainter &g,const TIMEPOS *vel);
    void  DrawLabel    (Graph *,QPainter &g, const QPoint &p, const QString &label, int ha, int va);
    void  DrawMark     (Graph *,QPainter &g, const QPoint &p, int mark, const QColor &color, int size, int rot);
    void  DrawSol      (QPainter &g,int level, int type);
    void  DrawSolPnt   (QPainter &g,const TIMEPOS *pos, int level, int style);
    void  DrawSolStat  (QPainter &g,const TIMEPOS *pos, const QString &unit, int p);
    void  DrawNsat     (QPainter &g,int level);
    void  DrawRes      (QPainter &g,int level);
    void  DrawPolyS    (Graph *,QPainter &c, double *x, double *y, int n,
                                  const QColor &color, int style);
    
    void  DrawObs      (QPainter &g,int level);
    void  DrawObsSlip  (QPainter &g,double *yp);
    void  DrawObsEphem (QPainter &g,double *yp);
    void  DrawSkyImage (QPainter &g,int level);
    void  DrawSky      (QPainter &g,int level);
    void  DrawDop      (QPainter &g,int level);
    void  DrawDopStat  (QPainter &g,double *dop, int *ns, int n);
    void  DrawSnr      (QPainter &g,int level);
    void  DrawSnrE     (QPainter &g,int level);
    void  DrawMpS      (QPainter &g,int level);
    
    TIMEPOS *  SolToPos (solbuf_t *sol, int index, int qflag, int type);
    TIMEPOS *  SolToNsat(solbuf_t *sol, int index, int qflag);
    
    void  PosToXyz     (gtime_t time, const double *rr, int type, double *xyz);
    void  CovToXyz     (const double *rr, const float *qr, int type,
                                  double *xyzs);
    void  CalcStats    (const double *x, int n, double ref, double &ave,
                                  double &std, double &rms);
    int   FitPos       (gtime_t *time, double *opos, double *ovel);
    
    QString  LatLonStr(const double *pos, int ndec);
    QColor  ObsColor   (const obsd_t *obs, double az, double el);
    QColor  SysColor   (int sat);
    QColor  SnrColor   (double snr);
    QColor  MpColor    (double mp);
    void  ReadStaPos   (const QString &file, const QString &sta, double *rr);
    int   SearchPos    (int x, int y);
    void  TimeSpan     (gtime_t *ts, gtime_t *te, double *tint);
    double  TimePos    (gtime_t time);
    void  TimeStr(gtime_t time, int n, int tsys, QString &str);
    int   ExecCmd      (const QString &cmd);
    void  ShowMsg      (const QString &msg);
    void  ShowLegend   (QString *msgs);
    void  LoadOpt      (void);
    void  SaveOpt      (void);
    
    MapAreaDialog *mapAreaDialog;
    GoogleEarthView *googleEarthView;
    GoogleMapView *googleMapView;
    SpanDialog *spanDialog;
    ConnectDialog *connectDialog;
    SkyImgDialog *skyImgDialog;
    PlotOptDialog *plotOptDialog;
    AboutDialog *aboutDialog;
    PntDialog *pntDialog;
    FileSelDialog *fileSelDialog;
    TextViewer *viewer;
    VecMapDialog *vecMapDialog;
public:
    QImage SkyImageR;
    QString IniFile;
    QString MapImageFile;
    QString SkyImageFile;
    QString RnxOpts;
    tle_t TLEData;
    QFont Font;
    gis_t Gis;

    // connection settings
    int RtStream[2];
    QString RtPath1,RtPath2;
    QString StrPaths[2][3];
    QString StrCmds[2][2];
    int StrCmdEna[2][2];
    int RtFormat[2];
    int RtConnType;
    int RtTimeForm;
    int RtDegForm;
    QString RtFieldSep;
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
    QString ExSats;
    int ShowErr;
    int ShowArrow;
    int ShowGLabel;
    int ShowLabel;
    int ShowCompass;
    int ShowScale;
    int AutoScale;
    double YRange;
    int RtBuffSize;
    int Origin;
    int RcvPos;
    double OOPos[3];
    QColor MColor[2][8]; // {{mark1 0-7},{mark2 0-7}}
    QColor CColor[4];    // {background,grid,text,line}
    QColor MapColor[MAXMAPLAYER]; // mapcolors
    int PlotStyle;
    int MarkSize;
    int AnimCycle;
    int RefCycle;
    int Trace;
    QString QcCmd,QcOpt;
    QString TLEFile;
    QString TLESatFile;
    
    QString Title;
    QString PntName[MAXWAYPNT];
    double PntPos[MAXWAYPNT][3];
    int NWayPnt,SelWayPnt;
    int OPosType;
    double OPos[3],OVel[3];
    
    QString StrHistory [10];
    QString StrMntpHist[10];
    
    void  ReadSol     (const QStringList &files, int sel);
    void  ReadObs     (const QStringList &files);
    void  ReadNav     (const QStringList &files);
    void  ReadMapData (const QString &file);
    void  ReadSkyData(const QString &file);
    void  ReadWaypoint(const QString &file);
    void  SaveWaypoint(const QString &file);
    void  ReadSkyTag  (const QString &file);
    void  UpdateSky   (void);
    void  ReadElMaskData(const QString &file);
    int  GetCurrentPos(double *rr);
    int  GetCenterPos(double *rr);
    void SetTrkCent(double lat, double lon);
    void  UpdatePlot(void);
    void  Refresh_GEView(void);
    void  Refresh_GMView(void);

    explicit Plot(QWidget* parent=NULL);
    ~Plot();
};

//---------------------------------------------------------------------------
#endif
