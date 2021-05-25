//---------------------------------------------------------------------------
#ifndef navimainH
#define navimainH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QColor>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMenu>


#include "rtklib.h"
#include "ui_navimain.h"

#define MAXSCALE	18
#define MAXMAPPNT	10

class AboutDialog;
class OptDialog;
class InputStrDialog;
class OutputStrDialog;
class LogStrDialog;
class MonitorDialog;

//---------------------------------------------------------------------------
class MainWindow : public QDialog, private Ui::MainForm
{
        Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:	
    void TimerTimer();
	
    void BtnStartClick();
    void BtnStopClick();
    void BtnPlotClick();
    void BtnOptClick();
    void BtnExitClick();
	
    void BtnTimeSysClick();
    void BtnInputStrClick();
    void BtnOutputStrClick();
    void BtnLogStrClick();
    void BtnSolTypeClick();
    void BtnPlotType1Click();
	
    void BtnMonitorClick();
    void BtnSaveClick();
    void BtnAboutClick();
    void BtnTaskTrayClick();
	
    void MenuExpandClick();
    void MenuStartClick();
    void MenuStopClick();
    void MenuPlotClick();
    void MenuMonitorClick();
    void MenuExitClick();
    void ScbSolChange();
	
    void TrayIconClick(QSystemTrayIcon::ActivationReason);
    void BtnFreqType1Click();
    void BtnPanelClick();
    void BtnPlotType2Click();
    void BtnFreqType2Click();
    void BtnPlotType3Click();
    void BtnFreqType3Click();
    void BtnPlotType4Click();
    void BtnFreqType4Click();
    void BtnExpand1Click();
    void BtnShrink1Click();
    void BtnExpand2Click();
    void BtnShrink2Click();
    void BtnExpand3Click();
    void BtnShrink3Click();
    void BtnExpand4Click();
    void BtnShrink4Click();
    void BtnMarkClick();

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent *);

private:
    AboutDialog *aboutDialog;
    InputStrDialog *inputStrDialog;
    OutputStrDialog *outputStrDialog;
    LogStrDialog *logStrDialog;
    MonitorDialog *monitor;
    QSystemTrayIcon *systemTray;
    QMenu *trayMenu;
    QAction *MenuStartAction, *MenuStopAction, *MenuExitAction;

    void  UpdateLog (int stat, gtime_t time, double *rr, float *qr,
                     double *rb, int ns, double age, double ratio);
    void SvrStart(void);
    void SvrStop(void);
    void UpdatePanel(void);
    void UpdateTimeSys(void);
    void UpdateSolType(void);
    void UpdateFont(void);
    void UpdateTime(void);
    void UpdatePos(void);
    void UpdateStr(void);
    void DrawPlot(QLabel *plot, int type, int freq);
    void UpdatePlot(void);
    void UpdateEnable(void);
    void ChangePlot(void);
    int ConfOverwrite(const QString &path);
	
    void DrawSnr(QPainter *c, int w, int h, int x0, int y0, int index, int freq);
    void DrawSat(QPainter *c, int w, int h, int x0, int y0, int index, int freq);
    void DrawBL(QPainter *c, int id, int w, int h);
    void DrawTrk(QPainter *c, int id, QPixmap &plot);
    void DrawSky(QPainter *c, int w, int h, int x0, int y0);
    void DrawText(QPainter *c, int x, int y, const QString &s,
                   const QColor &color, int ha, int va);
    void DrawArrow    (QPainter *c, int x, int y, int siz,
                                  int ang, const QColor &color);
    void OpenMoniPort(int port);
    void InitSolBuff(void);
    void SaveLog(void);
    void LoadNav(nav_t *nav);
    void SaveNav(nav_t *nav);
    void LoadOpt(void);
    void SaveOpt(void);
    void SetTrayIcon(int index);
    int ExecCmd(const QString &cmd, const QStringList &opt, int show);
    void BtnFreqTypeChange(int i);
    QColor SnrColor(int snr);
public:
    OptDialog *optDialog;
    QString IniFile;
	
    int TimerCycle,TimerInact;
    int PanelStack, PanelMode;
    int SvrCycle, SvrBuffSize, Scale, SolBuffSize, NavSelect, SavedSol;

    int NmeaReq,NmeaCycle,InTimeTag,InTime64Bit;
    int OutTimeTag,OutAppend,LogTimeTag,LogAppend;
    int TimeoutTime, ReconTime, SbasCorr, DgpsCorr, TideCorr, FileSwapMargin;
    int Stream[MAXSTRRTK], StreamC[MAXSTRRTK], Format[MAXSTRRTK];
    int CmdEna[3][3], CmdEnaTcp[3][3];
    int TimeSys,SolType;
    int PlotType[4],FreqType[4];
    int TrkType[4];
    int TrkScale[4];
    int BLMode[4];   
    int MoniPort, OpenPort, AutoRun;
	
    int PSol, PSolS, PSolE, Nsat[2], SolCurrentStat;
    int Sat[2][MAXSAT], Snr[2][MAXSAT][NFREQ], Vsat[2][MAXSAT];
    double Az[2][MAXSAT], El[2][MAXSAT];
	gtime_t *Time;
    int *SolStat, *Nvsat;
    double *SolRov, *SolRef, *Qr, *VelRov, *Age, *Ratio;
    double TrkOri[3], MaxBL;

    QString Paths[MAXSTRRTK][4], Cmds[3][3], CmdsTcp[3][3];
    QString InTimeStart, InTimeSpeed, ExSats;
    QString RcvOpt[3], ProxyAddr;
    QString OutSwapInterval, LogSwapInterval, ResetCmd;

	prcopt_t PrcOpt;
	solopt_t SolOpt;

    QFont PanelFont, PosFont;

    int DebugTraceF, DebugStatusF, OutputGeoidF, BaselineC;
    int RovPosTypeF, RefPosTypeF, RovAntPcvF, RefAntPcvF;
    QString RovAntF, RefAntF, SatPcvFileF, AntPcvFileF;
    double RovAntDel[3], RefAntDel[3], RovPos[3], RefPos[3], NmeaPos[3];
	double Baseline[2];

    QString History[10];

    QTimer Timer;
	
    QString GeoidDataFileF, StaPosFileF, DCBFileF, EOPFileF;
    QString LocalDirectory, PntName[MAXMAPPNT];

	double PntPos[MAXMAPPNT][3];
	int NMapPnt;

    QString MarkerName, MarkerComment;
};

extern MainWindow *mainForm;
//---------------------------------------------------------------------------
#endif
