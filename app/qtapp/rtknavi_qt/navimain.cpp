//---------------------------------------------------------------------------
// rtknavi : real-time positioning ap
//
//          Copyright (C) 2007-2014 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtknavi [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//           2010/07/18  1.1 rtklib 2.4.0
//           2010/08/16  1.2 fix bug on setting of satellite antenna model
//           2010/09/04  1.3 fix bug on setting of receiver antenna delta
//           2011/06/10  1.4 rtklib 2.4.1
//           2012/04/03  1.5 rtklib 2.4.2
//           2014/09/06  1.6 rtklib 2.4.3
//           2016/01/31  2.0 ported to Qt
//---------------------------------------------------------------------------
#include <stdio.h>
#include <math.h>

#include <QMessageBox>
#include <QMainWindow>
#include <QTimer>
#include <QFileInfo>
#include <QFont>
#include <QSettings>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QShowEvent>
#include <QCloseEvent>
#include <QPainter>
#include <QDebug>

#ifdef QT5
#include <QCommandLineParser>
#endif

#include "rtklib.h"
#include "instrdlg.h"
#include "outstrdlg.h"
#include "logstrdlg.h"
#include "mondlg.h"
#include "tcpoptdlg.h"
#include "aboutdlg.h"
#include "markdlg.h"
#include "viewer.h"
#include "naviopt.h"
#include "ui_navimain.h"
#include "navimain.h"
#include "graph.h"

MainWindow *mainForm;

//---------------------------------------------------------------------------

#define PRGNAME     "RTKNAVI-QT"           // program name
#define TRACEFILE   "rtknavi_%Y%m%d%h%M.trace" // debug trace file
#define STATFILE    "rtknavi_%Y%m%d%h%M.stat"  // solution status file
const QChar degreeChar(0260);           // character code of degree (UTF-8)
#define SATSIZE     20                  // satellite circle size in skyplot
#define MINSNR      10                  // minimum snr
#define MAXSNR      60                  // maximum snr
#define POSFONTNAME "Palatino Linotype"
#define POSFONTSIZE 12
#define MINBLLEN    0.01                // minimum baseline length to show

#define KACYCLE     1000                // keep alive cycle (ms)
#define TIMEOUT     10000               // inactive timeout time (ms)
#define DEFAULTPORT 52001               // default monitor port number
#define MAXPORTOFF  9                   // max port number offset
#define MAXTRKSCALE 23                  // track scale

#define SQRT(x)     ((x)<0.0?0.0:sqrt(x))
#define MIN(x,y)    ((x)<(y)?(x):(y))

//---------------------------------------------------------------------------

rtksvr_t rtksvr;                        // rtk server struct
stream_t monistr;                       // monitor stream

// show message in message area ---------------------------------------------
#if 0
extern "C" {
extern int showmsg(char *format,...) {return 0;}
}
#endif
// convert degree to deg-min-sec --------------------------------------------
static void degtodms(double deg, double *dms)
{
    double sgn=1.0;
    if (deg<0.0) {deg=-deg; sgn=-1.0;}
    dms[0]=floor(deg);
    dms[1]=floor((deg-dms[0])*60.0);
    dms[2]=(deg-dms[0]-dms[1]/60.0)*3600;
    dms[0]*=sgn;
}
// execute command ----------------------------------------------------------
int  MainWindow::ExecCmd(const QString &cmd, int show)
{
    Q_UNUSED(show);

    return QProcess::startDetached(cmd); /* FIXME: show option not yet supported */
}
// constructor --------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
{
    mainForm=this;
    setupUi(this);

    setlocale(LC_NUMERIC,"C");

    SvrCycle=SvrBuffSize=0;
    SolBuffSize=1000;

    for (int i=0;i<MAXSTRRTK;i++) {
        StreamC[i]=Stream[i]=Format[i]=0;
    }
    for (int i=0;i<3;i++) {
        CmdEna[i][0]=CmdEna[i][1]=0;
    }

    TimeSys=SolType=PlotType1=PlotType2=FreqType1=FreqType2=0;
    TrkType1=TrkType2=0;
    TrkScale1=TrkScale2=5;
    BLMode1=BLMode2=0;
    PSol=PSolS=PSolE=Nsat[0]=Nsat[1]=0;
    NMapPnt=0;
    OpenPort=0;
    Time=NULL;
    SolStat=Nvsat=NULL;
    SolCurrentStat=0;
    SolRov=SolRef=Qr=VelRov=Age=Ratio=NULL;

    for (int i=0;i<2;i++) for (int j=0;j<MAXSAT;j++) {
        Sat[i][j]=Vsat[i][j]=0;
        Az[i][j]=El[i][j]=0.0;
        for (int k=0;k<NFREQ;k++) Snr[i][j][k]=0;
    }
    PrcOpt=prcopt_default;
    SolOpt=solopt_default;
    
    rtksvrinit(&rtksvr);
    strinit(&monistr);
    
    setWindowTitle(QString(tr("%1 ver. %2")).arg(PRGNAME).arg(VER_RTKLIB));
    setWindowIcon(QIcon(":/icons/rtknavi_Icon.ico"));

    TLEData.n=TLEData.nmax=0;
    TLEData.data=NULL;
    
    PanelStack=PanelMode=0;

    for (int i=0;i<3;i++) {
        TrkOri[i]=0.0;
    }
    optDialog=new OptDialog(this);
    inputStrDialog= new InputStrDialog(this);
    outputStrDialog=new OutputStrDialog(this);
    logStrDialog = new LogStrDialog(this);
    monitor=new MonitorDialog(this);
    systemTray= new QSystemTrayIcon(this);

    SetTrayIcon(1);

    trayMenu=new QMenu(this);
}

MainWindow::~MainWindow()
{
    delete [] Time;   delete [] SolStat; delete [] Nvsat;  delete [] SolRov;
    delete [] SolRef; delete [] Qr;      delete [] VelRov; delete [] Age;
    delete [] Ratio;
}

// callback on form create --------------------------------------------------
void  MainWindow::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    trace(3,"FormCreate\n");

    UpdatePlot();

    trayMenu->addAction(tr("Main Window..."),this,SLOT(MenuExpandClick()));
    trayMenu->addAction(tr("Monitor..."),this,SLOT(MenuMonitorClick()));
    trayMenu->addAction(tr("Plot..."),this,SLOT(MenuPlotClick()));
    trayMenu->addSeparator();
    MenuStartAction=trayMenu->addAction(tr("Start"),this,SLOT(MenuStartClick()));
    MenuStopAction=trayMenu->addAction(tr("Stop"),this,SLOT(MenuStopClick()));
    trayMenu->addSeparator();
    MenuExitAction=trayMenu->addAction(tr("Exit"),this,SLOT(MenuExitClick()));

    systemTray->setContextMenu(trayMenu);

    connect(systemTray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(TrayIconClick(QSystemTrayIcon::ActivationReason)));
    
    connect(BtnExit,SIGNAL(clicked()),this,SLOT(BtnExitClick()));
    connect(BtnStart,SIGNAL(clicked()),this,SLOT(BtnStartClick()));
    connect(BtnStop,SIGNAL(clicked()),this,SLOT(BtnStopClick()));
    connect(BtnPlot,SIGNAL(clicked()),this,SLOT(BtnPlotClick()));
    connect(BtnAbout,SIGNAL(clicked(bool)),this,SLOT(BtnAboutClick()));
    connect(BtnFreqType1,SIGNAL(clicked(bool)),this,SLOT(BtnFreqType1Click()));
    connect(BtnFreqType2,SIGNAL(clicked(bool)),this,SLOT(BtnFreqType2Click()));
    connect(BtnExpand,SIGNAL(clicked(bool)),this,SLOT(BtnExpand1Click()));
    connect(BtnShrink,SIGNAL(clicked(bool)),this,SLOT(BtnShrink1Click()));
    connect(BtnInputStr,SIGNAL(clicked(bool)),this,SLOT(BtnInputStrClick()));
    connect(BtnLogStr,SIGNAL(clicked(bool)),this,SLOT(BtnLogStrClick()));
    connect(BtnMonitor,SIGNAL(clicked(bool)),this,SLOT(BtnMonitorClick()));
    connect(BtnOpt,SIGNAL(clicked(bool)),this,SLOT(BtnOptClick()));
    connect(BtnOutputStr,SIGNAL(clicked(bool)),this,SLOT(BtnOutputStrClick()));
    connect(BtnPanel,SIGNAL(clicked(bool)),this,SLOT(BtnPanelClick()));
    connect(BtnPlot,SIGNAL(clicked(bool)),this,SLOT(BtnPlotClick()));
    connect(BtnPlotType1,SIGNAL(clicked(bool)),this,SLOT(BtnPlotType1Click()));
    connect(BtnPlotType2,SIGNAL(clicked(bool)),this,SLOT(BtnPlotType2Click()));
    connect(BtnSave,SIGNAL(clicked(bool)),this,SLOT(BtnSaveClick()));
    connect(BtnSolType,SIGNAL(clicked(bool)),this,SLOT(BtnSolTypeClick()));
    connect(BtnSolType2,SIGNAL(clicked(bool)),this,SLOT(BtnSolTypeClick()));
    connect(BtnTaskTray,SIGNAL(clicked(bool)),this,SLOT(BtnTaskTrayClick()));
    connect(BtnTimeSys,SIGNAL(clicked(bool)),this,SLOT(BtnTimeSysClick()));
    connect(BtnMark,SIGNAL(clicked(bool)),this,SLOT(BtnMarkClick()));
    connect(ScbSol,SIGNAL(valueChanged(int)),this,SLOT(ScbSolChange()));
    connect(&Timer,SIGNAL(timeout()),this,SLOT(TimerTimer()));

    Timer.setInterval(100);
    Timer.setSingleShot(false);
    Timer.start();

    QString file=QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile=fi.absolutePath()+"/"+fi.baseName()+".ini";
    
    InitSolBuff();
    strinitcom();

#ifdef QT5
    QCommandLineParser parser;
    parser.setApplicationDescription("RTK Navi");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniFileOption(QStringList() << "i" << "ini-file",
            QCoreApplication::translate("main", "ini file to use"),
            QCoreApplication::translate("main", "ini file"));
    parser.addOption(iniFileOption);

    parser.process(*QApplication::instance());

    if (parser.isSet(iniFileOption))
        IniFile=parser.value(iniFileOption);
#endif /*TODO: alternative for QT4 */
    LoadOpt();
    
    LoadNav(&rtksvr.nav);
    
    OpenMoniPort(MoniPort);

    if (TLEFileF!="") {
        tle_read(qPrintable(TLEFileF),&TLEData);
    }
    if (TLESatFileF!="") {
        tle_name_read(qPrintable(TLESatFileF),&TLEData);
    }

    UpdatePanel();
    UpdateTimeSys();
    UpdateSolType();
    UpdateFont();
    UpdatePos();
    UpdateEnable();
}
// callback on form close ---------------------------------------------------
void  MainWindow::closeEvent(QCloseEvent *event )
{
    trace(3,"FormClose\n");

    if (event->spontaneous()) return;

    if (OpenPort>0) {
        // send disconnect message
        strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));
        
        strclose(&monistr);
    }
    SaveOpt();
    SaveNav(&rtksvr.nav);

    QWidget::closeEvent(event);
}
// update panel -------------------------------------------------------------
void  MainWindow::UpdatePanel(void)
{
    if (PanelMode==0) {
        Panel21 ->setVisible(true);
        Panel5  ->setVisible(false);
        Panel221->setVisible(false);
    }
    else if (PanelMode==1) {
        Panel21 ->setVisible(true);
        Panel5  ->setVisible(false);
        Panel221->setVisible(true);
    }
    else if (PanelMode==2) {
        Panel21 ->setVisible(false);
        Panel5  ->setVisible(true);
        Panel221->setVisible(true);
    }
    else {
        Panel21 ->setVisible(false);
        Panel5  ->setVisible(true);
        Panel221->setVisible(false);
    }
}
// update enabled -----------------------------------------------------------
void MainWindow::UpdateEnable(void)
{
    BtnExpand->setVisible(PlotType2==6);
    BtnShrink->setVisible(PlotType2==6);
}
// callback on button-exit --------------------------------------------------
void  MainWindow::BtnExitClick()
{
    trace(3,"BtnExitClick\n");
    
    close();
}
// callback on button-start -------------------------------------------------
void  MainWindow::BtnStartClick()
{
    trace(3,"BtnStartClick\n");
    
    SvrStart();
}
// callback on button-stop --------------------------------------------------
void  MainWindow::BtnStopClick()
{
    trace(3,"BtnStopClick\n");
    
    SvrStop();
}
// callback on button-plot --------------------------------------------------
void  MainWindow::BtnPlotClick()
{
    QString cmd;
    
    trace(3,"BtnPlotClick\n");
    
    if (OpenPort<=0) {
        QMessageBox::critical(this,tr("Error"),tr("monitor port not open"));
        return;
    }
    cmd=QString("rtkplot_qt -p tcpcli://localhost:%1 -t \"%2 %3\"").arg(OpenPort)
                .arg(windowTitle()).arg(": RTKPLOT");
    if (!ExecCmd(cmd,1)) {
        QMessageBox::critical(this,tr("Error"),tr("error: rtkplot execution"));
    }
}
// callback on button-options -----------------------------------------------
void  MainWindow::BtnOptClick()
{
    int i,chgmoni=0;
    
    trace(3,"BtnOptClick\n");
    
    optDialog->PrcOpt     =PrcOpt;
    optDialog->SolOpt     =SolOpt;
    optDialog->DebugStatusF=DebugStatusF;
    optDialog->DebugTraceF=DebugTraceF;
    optDialog->BaselineC  =BaselineC;
    optDialog->Baseline[0]=Baseline[0];
    optDialog->Baseline[1]=Baseline[1];
    
    optDialog->RovPosTypeF=RovPosTypeF;
    optDialog->RefPosTypeF=RefPosTypeF;
    optDialog->RovAntPcvF =RovAntPcvF;
    optDialog->RefAntPcvF =RefAntPcvF;
    optDialog->RovAntF    =RovAntF;
    optDialog->RefAntF    =RefAntF;
    
    optDialog->SatPcvFileF=SatPcvFileF;
    optDialog->AntPcvFileF=AntPcvFileF;
    optDialog->StaPosFileF=StaPosFileF;
    optDialog->GeoidDataFileF=GeoidDataFileF;
    optDialog->DCBFileF   =DCBFileF;
    optDialog->EOPFileF   =EOPFileF;
    optDialog->TLEFileF   =TLEFileF;
    optDialog->TLESatFileF=TLESatFileF;
    optDialog->LocalDirectory=LocalDirectory;
    
    optDialog->SvrCycle   =SvrCycle;
    optDialog->TimeoutTime=TimeoutTime;
    optDialog->ReconTime  =ReconTime;
    optDialog->NmeaCycle  =NmeaCycle;
    optDialog->FileSwapMargin=FileSwapMargin;
    optDialog->SvrBuffSize=SvrBuffSize;
    optDialog->SolBuffSize=SolBuffSize;
    optDialog->SavedSol   =SavedSol;
    optDialog->NavSelect  =NavSelect;
    optDialog->DgpsCorr   =DgpsCorr;
    optDialog->SbasCorr   =SbasCorr;
    optDialog->ExSats     =ExSats;
    optDialog->ProxyAddr  =ProxyAddr;
    optDialog->MoniPort   =MoniPort;
    optDialog->PanelStack =PanelStack;
    PosFont=optDialog->PosFont;
    UpdateFont();
    UpdatePanel();

    for (i=0;i<3;i++) {
        optDialog->RovAntDel[i]=RovAntDel[i];
        optDialog->RefAntDel[i]=RefAntDel[i];
        optDialog->RovPos   [i]=RovPos   [i];
        optDialog->RefPos   [i]=RefPos   [i];
    }
    optDialog->exec();

    if (optDialog->result()!=QDialog::Accepted) return;
    
    PrcOpt     =optDialog->PrcOpt;
    SolOpt     =optDialog->SolOpt;
    DebugStatusF=optDialog->DebugStatusF;
    DebugTraceF=optDialog->DebugTraceF;
    BaselineC  =optDialog->BaselineC;
    Baseline[0]=optDialog->Baseline[0];
    Baseline[1]=optDialog->Baseline[1];
    
    RovPosTypeF=optDialog->RovPosTypeF;
    RefPosTypeF=optDialog->RefPosTypeF;
    RovAntPcvF =optDialog->RovAntPcvF;
    RefAntPcvF =optDialog->RefAntPcvF;
    RovAntF    =optDialog->RovAntF;
    RefAntF    =optDialog->RefAntF;
    
    SatPcvFileF=optDialog->SatPcvFileF;
    AntPcvFileF=optDialog->AntPcvFileF;
    StaPosFileF=optDialog->StaPosFileF;
    GeoidDataFileF=optDialog->GeoidDataFileF;
    DCBFileF   =optDialog->DCBFileF;
    EOPFileF   =optDialog->EOPFileF;
    TLEFileF   =optDialog->TLEFileF;
    TLESatFileF=optDialog->TLESatFileF;
    LocalDirectory=optDialog->LocalDirectory;

    SvrCycle   =optDialog->SvrCycle;
    TimeoutTime=optDialog->TimeoutTime;
    ReconTime  =optDialog->ReconTime;
    NmeaCycle  =optDialog->NmeaCycle;
    FileSwapMargin=optDialog->FileSwapMargin;
    SvrBuffSize=optDialog->SvrBuffSize;
    SavedSol   =optDialog->SavedSol;
    NavSelect  =optDialog->NavSelect;
    DgpsCorr   =optDialog->DgpsCorr;
    SbasCorr   =optDialog->SbasCorr;
    ExSats     =optDialog->ExSats;
    ProxyAddr  =optDialog->ProxyAddr;
    if (MoniPort!=optDialog->MoniPort) chgmoni=1;
    MoniPort   =optDialog->MoniPort;
    PanelStack =optDialog->PanelStack;
    
    if (SolBuffSize!=optDialog->SolBuffSize) {
        SolBuffSize=optDialog->SolBuffSize;
        InitSolBuff();
        UpdateTime();
        UpdatePos();
        UpdatePlot();
    }
    for (i=0;i<3;i++) {
        RovAntDel[i]=optDialog->RovAntDel[i];
        RefAntDel[i]=optDialog->RefAntDel[i];
        RovPos   [i]=optDialog->RovPos   [i];
        RefPos   [i]=optDialog->RefPos   [i];
    }
    PosFont=optDialog->PosFont;
    
    UpdateFont();
    UpdatePanel();
    
    if (!chgmoni) return;
    
    // send disconnect message
    if (OpenPort>0) {
        strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));
        
        strclose(&monistr);
    }
    // reopen monitor stream
    OpenMoniPort(MoniPort);
}
// callback on button-input-streams -----------------------------------------
void  MainWindow::BtnInputStrClick()
{
    int i,j;
    
    trace(3,"BtnInputStrClick\n");

    for (i=0;i<3;i++) {
        inputStrDialog->StreamC[i]=StreamC[i];
        inputStrDialog->Stream [i]=Stream [i];
        inputStrDialog->Format [i]=Format [i];
        inputStrDialog->RcvOpt [i]=RcvOpt [i];
        
        /* Paths[0]:serial,[1]:tcp,[2]:file,[3]:ftp */
        for (j=0;j<4;j++) inputStrDialog->Paths[i][j]=Paths[i][j];
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        inputStrDialog->CmdEna   [i][j]=CmdEna   [i][j];
        inputStrDialog->Cmds     [i][j]=Cmds     [i][j];
        inputStrDialog->CmdEnaTcp[i][j]=CmdEnaTcp[i][j];
        inputStrDialog->CmdsTcp  [i][j]=CmdsTcp  [i][j];
    }
    for (i=0;i<10;i++) {
        inputStrDialog->History [i]=History [i];
        inputStrDialog->MntpHist[i]=MntpHist[i];
    }
    inputStrDialog->NmeaReq   =NmeaReq;
    inputStrDialog->TimeTag   =InTimeTag;
    inputStrDialog->TimeSpeed =InTimeSpeed;
    inputStrDialog->TimeStart =InTimeStart;
    inputStrDialog->NmeaPos[0]=NmeaPos[0];
    inputStrDialog->NmeaPos[1]=NmeaPos[1];
    
    inputStrDialog->exec();

    if (inputStrDialog->result()!=QDialog::Accepted) return;
    
    for (i=0;i<3;i++) {
        StreamC[i]=inputStrDialog->StreamC[i];
        Stream [i]=inputStrDialog->Stream[i];
        Format [i]=inputStrDialog->Format[i];
        RcvOpt [i]=inputStrDialog->RcvOpt[i];
        for (j=0;j<4;j++) Paths[i][j]=inputStrDialog->Paths[i][j];
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        CmdEna   [i][j]=inputStrDialog->CmdEna   [i][j];
        Cmds     [i][j]=inputStrDialog->Cmds     [i][j];
        CmdEnaTcp[i][j]=inputStrDialog->CmdEnaTcp[i][j];
        CmdsTcp  [i][j]=inputStrDialog->CmdsTcp  [i][j];
    }
    for (i=0;i<10;i++) {
        History [i]=inputStrDialog->History [i];
        MntpHist[i]=inputStrDialog->MntpHist[i];
    }
    NmeaReq=inputStrDialog->NmeaReq;
    InTimeTag  =inputStrDialog->TimeTag;
    InTimeSpeed=inputStrDialog->TimeSpeed;
    InTimeStart=inputStrDialog->TimeStart;
    NmeaPos[0] =inputStrDialog->NmeaPos[0];
    NmeaPos[1] =inputStrDialog->NmeaPos[1];
}
// confirm overwrite --------------------------------------------------------
int  MainWindow::ConfOverwrite(const QString &path)
{
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int i;
    QString buff1, buff2;
    
    trace(3,"ConfOverwrite\n");
    
    buff1=path.mid(path.indexOf("::"));
        
    if (!QFile::exists(buff1)) return 1; // file not exists
    
    // check overwrite input files
    for (i=0;i<3;i++) {
        if (!StreamC[i]||itype[Stream[i]]!=STR_FILE) continue;
        
        buff2=Paths[i][2];
        buff2=buff2.mid(buff2.indexOf("::"));
        
        if (buff1==buff2) {
            Message->setText(QString(tr("invalid output %1")).arg(buff1));
            return 0;
        }
    }

    return QMessageBox::question(this,tr("File exists"),buff1)==QMessageBox::Yes;
}
// callback on button-output-streams ----------------------------------------
void  MainWindow::BtnOutputStrClick()
{
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,j,str,update[2]={0};
    char path[1024];
    
    trace(3,"BtnOutputStrClick\n");
    
    for (i=3;i<5;i++) {
        outputStrDialog->StreamC[i-3]=StreamC[i];
        outputStrDialog->Stream [i-3]=Stream[i];
        outputStrDialog->Format [i-3]=Format[i];
        for (j=0;j<4;j++) outputStrDialog->Paths[i-3][j]=Paths[i][j];
    }
    for (i=0;i<10;i++) {
        outputStrDialog->History [i]=History [i];
        outputStrDialog->MntpHist[i]=MntpHist[i];
    }
    outputStrDialog->OutTimeTag=OutTimeTag;
    outputStrDialog->OutAppend =OutAppend;
    outputStrDialog->SwapInterval=OutSwapInterval;
    outputStrDialog->exec();

    if (outputStrDialog->result()!=QDialog::Accepted) return;
    
    for (i=3;i<5;i++) {
        if (StreamC[i]!=outputStrDialog->StreamC[i-3]||
            Stream [i]!=outputStrDialog->Stream[i-3]||
            Format [i]!=outputStrDialog->Format[i-3]||
            Paths[i][0]!=outputStrDialog->Paths[i-3][0]||
            Paths[i][1]!=outputStrDialog->Paths[i-3][1]||
            Paths[i][2]!=outputStrDialog->Paths[i-3][2]||
            Paths[i][3]!=outputStrDialog->Paths[i-3][3]) update[i-3]=1;
        StreamC[i]=outputStrDialog->StreamC[i-3];
        Stream [i]=outputStrDialog->Stream[i-3];
        Format [i]=outputStrDialog->Format[i-3];
        for (j=0;j<4;j++) Paths[i][j]=outputStrDialog->Paths[i-3][j];
    }
    for (i=0;i<10;i++) {
        History [i]=outputStrDialog->History [i];
        MntpHist[i]=outputStrDialog->MntpHist[i];
    }
    OutTimeTag=outputStrDialog->OutTimeTag;
    OutAppend =outputStrDialog->OutAppend;
    OutSwapInterval=outputStrDialog->SwapInterval;
    
    if (BtnStart->isEnabled()) return;
    
    for (i=3;i<5;i++) {
        if (!update[i-3]) continue;
        
        rtksvrclosestr(&rtksvr,i);
        
        if (!StreamC[i]) continue;
        
        str=otype[Stream[i]];
        if      (str==STR_SERIAL)             strncpy(path,qPrintable(Paths[i][0]),1024);
        else if (str==STR_FILE  )             strncpy(path,qPrintable(Paths[i][2]),1024);
        else if (str==STR_FTP||str==STR_HTTP) strncpy(path,qPrintable(Paths[i][3]),1024);
        else                                  strncpy(path,qPrintable(Paths[i][1]),1024);
        if (str==STR_FILE&&!ConfOverwrite(path)) {
            StreamC[i]=0;
            continue;
        }
        SolOpt.posf=Format[i];
        rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
    }
}
// callback on button-log-streams -------------------------------------------
void  MainWindow::BtnLogStrClick()
{
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,j,str,update[3]={0};
    char path[1024];
    
    trace(3,"BtnLogStrClick\n");
    
    for (i=5;i<8;i++) {
        logStrDialog->StreamC[i-5]=StreamC[i];
        logStrDialog->Stream [i-5]=Stream [i];
        for (j=0;j<4;j++) logStrDialog->Paths[i-5][j]=Paths[i][j];
    }
    for (i=0;i<10;i++) {
        logStrDialog->History [i]=History [i];
        logStrDialog->MntpHist[i]=MntpHist[i];
    }
    logStrDialog->LogTimeTag=LogTimeTag;
    logStrDialog->LogAppend =LogAppend;
    logStrDialog->SwapInterval=LogSwapInterval;
    
    logStrDialog->exec();

    if (logStrDialog->result()!=QDialog::Accepted) return;
    
    for (i=5;i<8;i++) {
        if (StreamC[i]!=outputStrDialog->StreamC[(i-5)%2]||
            Stream [i]!=outputStrDialog->Stream[(i-5)%2]||
            Paths[i][0]!=outputStrDialog->Paths[(i-3)%2][0]||
            Paths[i][1]!=outputStrDialog->Paths[(i-3)%2][1]||
            Paths[i][2]!=outputStrDialog->Paths[(i-3)%2][2]||
            Paths[i][3]!=outputStrDialog->Paths[(i-3)%2][3]) update[i-5]=1;
        StreamC[i]=logStrDialog->StreamC[i-5];
        Stream [i]=logStrDialog->Stream [i-5];
        for (j=0;j<4;j++) Paths[i][j]=logStrDialog->Paths[i-5][j];
    }
    for (i=0;i<10;i++) {
        History [i]=logStrDialog->History [i];
        MntpHist[i]=logStrDialog->MntpHist[i];
    }
    LogTimeTag=logStrDialog->LogTimeTag;
    LogAppend =logStrDialog->LogAppend;
    LogSwapInterval=logStrDialog->SwapInterval;
    
    if (BtnStart->isEnabled()) return;
    
    for (i=5;i<8;i++) {
        if (!update[i-5]) continue;
        
        rtksvrclosestr(&rtksvr,i);
        
        if (!StreamC[i]) continue;
        
        str=otype[Stream[i]];
        if      (str==STR_SERIAL)             strncpy(path,qPrintable(Paths[i][0]),1024);
        else if (str==STR_FILE  )             strncpy(path,qPrintable(Paths[i][2]),1024);
        else if (str==STR_FTP||str==STR_HTTP) strncpy(path,qPrintable(Paths[i][3]),1024);
        else                                  strncpy(path,qPrintable(Paths[i][1]),1024);
        if (str==STR_FILE&&!ConfOverwrite(path)) {
            StreamC[i]=0;
            continue;
        }
        rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
    }
}
// callback on button-solution-show -----------------------------------------
void  MainWindow::BtnPanelClick()
{
    trace(3,"BtnPanelClick\n");
    
    if (++PanelMode>3) PanelMode=0;
    UpdatePanel();
}
// callback on button-plot-type-1 -------------------------------------------
void  MainWindow::BtnTimeSysClick()
{
    trace(3,"BtnTimeSysClick\n");
    
    if (++TimeSys>3) TimeSys=0;
    UpdateTimeSys();
}
// callback on button-solution-type -----------------------------------------
void  MainWindow::BtnSolTypeClick()
{
    trace(3,"BtnSolTypeClick\n");
    
    if (++SolType>4) SolType=0;
    UpdateSolType();
}
// callback on button-plottype-1 --------------------------------------------
void  MainWindow::BtnPlotType1Click()
{
    trace(3,"BtnPlotType1Click\n");
    
    if (++PlotType1>6) PlotType1=0;
    UpdatePlot();
    UpdatePos();
    UpdateEnable();
}
// callback on button-plottype-2 --------------------------------------------
void  MainWindow::BtnPlotType2Click()
{
    trace(3,"BtnPlotType2Click\n");
    
    if (++PlotType2>6) PlotType2=0;

    UpdatePlot();
    UpdatePos();
    UpdateEnable();
}
// callback on button frequency-type-1 --------------------------------------
void  MainWindow::BtnFreqType1Click()
{
    trace(3,"BtnFreqType1Click\n");
    
    if (PlotType1==6) {
        if (++TrkType1>1) TrkType1=0;
        UpdatePlot();
    }
    else if (PlotType2==5) {
        if (++BLMode1>1) BLMode1=0;
        UpdatePlot();
    }
    else {
        if (++FreqType1>NFREQ+1) FreqType1=0;
        UpdateSolType();
    }
}
// callback on button frequency-type-2 --------------------------------------
void  MainWindow::BtnFreqType2Click()
{
    trace(3,"BtnFreqType2Click\n");
    
    if (PlotType2==6) {
        if (++TrkType2>1) TrkType2=0;
        UpdatePlot();
    }
    else if (PlotType1==5) {
        if (++BLMode2>1) BLMode2=0;
        UpdatePlot();
    }
    else {
        if (++FreqType2>NFREQ+1) FreqType2=0;
        UpdateSolType();
    }
}
// callback on button expand-1 ----------------------------------------------
void MainWindow::BtnExpand1Click()
{
    if (TrkScale2<=0) return;
    TrkScale2--;
    UpdatePlot();
}
// callback on button shrink-1 ----------------------------------------------
void MainWindow::BtnShrink1Click()
{
    if (TrkScale2>=MAXTRKSCALE) return;
    TrkScale2++;
    UpdatePlot();
}
// callback on button expand-2 ----------------------------------------------
void MainWindow::BtnExpand2Click()
{
    if (TrkScale2<=0) return;
    TrkScale2--;
    UpdatePlot();
}
// callback on button shrink-2 ----------------------------------------------
void MainWindow::BtnShrink2Click()
{
    if (TrkScale2>=MAXTRKSCALE) return;
    TrkScale2++;
    UpdatePlot();
}
// callback on button-rtk-monitor -------------------------------------------
void  MainWindow::BtnMonitorClick()
{
    
    trace(3,"BtnMonitorClick\n");
    
    monitor->setWindowTitle(windowTitle()+": RTK Monitor");
    monitor->show();
}
// callback on scroll-solution change ---------------------------------------
void  MainWindow::ScbSolChange()
{
    trace(3,"ScbSolChange\n");
    
    PSol=PSolS+ScbSol->value();
    if (PSol>=SolBuffSize) PSol-=SolBuffSize;

    UpdateTime();
    UpdatePos();
    UpdatePlot();
}
// callback on button-save --------------------------------------------------
void  MainWindow::BtnSaveClick()
{
    trace(3,"BtnSaveClick\n");
    
    SaveLog();
}
// callback on button-about -------------------------------------------------
void  MainWindow::BtnAboutClick()
{
    QString prog=PRGNAME;
    
    trace(3,"BtnAboutClick\n");

    aboutDialog=new AboutDialog(this);
    aboutDialog->About=prog;
    aboutDialog->IconIndex=5;
    aboutDialog->exec();

    delete aboutDialog;
}
// callback on button-tasktray ----------------------------------------------
void  MainWindow::BtnTaskTrayClick()
{
    trace(3,"BtnTaskTrayClick\n");
    
    setVisible(false);
    systemTray->setToolTip(windowTitle());
    systemTray->setVisible(true);
}
// callback on button-tasktray ----------------------------------------------
void  MainWindow::TrayIconClick(QSystemTrayIcon::ActivationReason reason)
{
    trace(3,"TaskIconDblClick\n");
    if (reason!=QSystemTrayIcon::DoubleClick) return;

    setVisible(true);
    systemTray->setVisible(false);
}
// callback on menu-expand --------------------------------------------------
void  MainWindow::MenuExpandClick()
{
    trace(3,"MenuExpandClick\n");
    
    setVisible(true);
    systemTray->setVisible(false);
}
// callback on menu-start ---------------------------------------------------
void  MainWindow::MenuStartClick()
{
    trace(3,"MenuStartClick\n");
    
    BtnStartClick();
}
// callback on menu-stop ----------------------------------------------------
void  MainWindow::MenuStopClick()
{
    trace(3,"MenuStopClick\n");
    
    BtnStopClick();
}
// callback on menu-monitor -------------------------------------------------
void  MainWindow::MenuMonitorClick()
{
    trace(3,"MenuMonitorClick\n");
    
    BtnMonitorClick();
}
// callback on menu-plot ----------------------------------------------------
void  MainWindow::MenuPlotClick()
{
    trace(3,"MenuPlotClick\n");
    
    BtnPlotClick();
}
// callback on menu-exit ----------------------------------------------------
void  MainWindow::MenuExitClick()
{
    trace(3,"MenuExitClick\n");
    
    BtnExitClick();
}
// start rtk server ---------------------------------------------------------
void  MainWindow::SvrStart(void)
{
    solopt_t solopt[2];
    double pos[3],nmeapos[3];
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,strs[MAXSTRRTK]={0},sat,ex,stropt[8]={0};
    char *paths[8],*cmds[3]={0},*rcvopts[3]={0};
    char buff[1024],*p;
    gtime_t time=timeget();
    pcvs_t pcvr,pcvs;
    pcv_t *pcv;
    
    trace(3,"SvrStart\n");
    
    memset(&pcvr,0,sizeof(pcvs_t));
    memset(&pcvs,0,sizeof(pcvs_t));

    Message->setText("");
    
    if (RovPosTypeF<=2) { // LLH,XYZ
        PrcOpt.rovpos=0;
        PrcOpt.ru[0]=RovPos[0];
        PrcOpt.ru[1]=RovPos[1];
        PrcOpt.ru[2]=RovPos[2];
    }
    else { // RTCM position
        PrcOpt.rovpos=4;
        for (i=0;i<3;i++) PrcOpt.ru[i]=0.0;
    }
    if (RefPosTypeF<=2) { // LLH,XYZ
        PrcOpt.refpos=0;
        PrcOpt.rb[0]=RefPos[0];
        PrcOpt.rb[1]=RefPos[1];
        PrcOpt.rb[2]=RefPos[2];
    }
    else if (RefPosTypeF==3) { // RTCM position
        PrcOpt.refpos=4;
        for (i=0;i<3;i++) PrcOpt.rb[i]=0.0;
    }else { // average of single position
        PrcOpt.refpos=1;
        for (i=0;i<3;i++) PrcOpt.rb[i]=0.0;
    }

    for (i=0;i<MAXSAT;i++) {
        PrcOpt.exsats[i]=0;
    }
    if (ExSats!="") { // excluded satellites
        strcpy(buff,qPrintable(ExSats));
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if (*p=='+') {ex=2; p++;} else ex=1;
            if (!(sat=satid2no(p))) continue;
            PrcOpt.exsats[sat-1]=ex;
        }
    }
    if ((RovAntPcvF||RefAntPcvF)&&!readpcv(qPrintable(AntPcvFileF),&pcvr)) {
        Message->setText(QString(tr("rcv ant file read error %1")).arg(AntPcvFileF));
        return;
    }
    if (RovAntPcvF) {
        if ((pcv=searchpcv(0,qPrintable(RovAntF),time,&pcvr))) {
            PrcOpt.pcvr[0]=*pcv;
        }
        else {
            Message->setText(QString(tr("no antenna pcv %1")).arg(qPrintable(RovAntF)));
        }
        for (i=0;i<3;i++) PrcOpt.antdel[0][i]=RovAntDel[i];
    }
    if (RefAntPcvF) {
        if ((pcv=searchpcv(0,qPrintable(RefAntF),time,&pcvr))) {
            PrcOpt.pcvr[1]=*pcv;
        }
        else {
            Message->setText(QString(tr("no antenna pcv %1")).arg(qPrintable(RefAntF)));
        }
        for (i=0;i<3;i++) PrcOpt.antdel[1][i]=RefAntDel[i];
    }
    if (RovAntPcvF||RefAntPcvF) {
        free(pcvr.pcv);
    }
    if (PrcOpt.sateph==EPHOPT_PREC||PrcOpt.sateph==EPHOPT_SSRCOM) {
        if (!readpcv(qPrintable(SatPcvFileF),&pcvs)) {
            Message->setText(QString(tr("sat ant file read error %1")).arg(SatPcvFileF));
            return;
        }
        for (i=0;i<MAXSAT;i++) {
            if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
            rtksvr.nav.pcvs[i]=*pcv;
        }
        free(pcvs.pcv);
    }
    if (BaselineC) {
        PrcOpt.baseline[0]=Baseline[0];
        PrcOpt.baseline[1]=Baseline[1];
    }
    else {
        PrcOpt.baseline[0]=0.0;
        PrcOpt.baseline[1]=0.0;
    }
    for (i=0;i<3;i++) strs[i]=StreamC[i]?itype[Stream[i]]:STR_NONE;
    for (i=3;i<5;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
    for (i=5;i<8;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
    for (i=0;i<8;i++) {
        paths[i]= new char[1024];
        paths[i][0]='\0';
        if      (strs[i]==STR_NONE  ) strcpy(paths[i],"");
        else if (strs[i]==STR_SERIAL) strcpy(paths[i],qPrintable(Paths[i][0]));
        else if (strs[i]==STR_FILE  ) strcpy(paths[i],qPrintable(Paths[i][2]));
        else if (strs[i]==STR_FTP||strs[i]==STR_HTTP) strcpy(paths[i],qPrintable(Paths[i][3]));
        else strcpy(paths[i],qPrintable(Paths[i][1]));
    }
    for (i=0;i<3;i++) {
        cmds[i]=new char[1024];
        rcvopts[i]=new char[1024];
        cmds[i][0]=rcvopts[i][0]='\0';
        if (strs[i]==STR_SERIAL) {
            if (CmdEna[i][0]) strcpy(cmds[i],qPrintable(Cmds[i][0]));
        }
        else if (strs[i]==STR_TCPCLI||strs[i]==STR_TCPSVR||
                 strs[i]==STR_NTRIPCLI) {
            if (CmdEnaTcp[i][0]) strcpy(cmds[i],qPrintable(CmdsTcp[i][0]));
        }
        strcpy(rcvopts[i],qPrintable(RcvOpt[i]));
    }
    NmeaCycle=NmeaCycle<1000?1000:NmeaCycle;
    pos[0]=NmeaPos[0]*D2R;
    pos[1]=NmeaPos[1]*D2R;
    pos[2]=0.0;
    pos2ecef(pos,nmeapos);
    
    strsetdir(qPrintable(LocalDirectory));
    strsetproxy(qPrintable(ProxyAddr));
    
    for (i=3;i<8;i++) {
        if (strs[i]==STR_FILE&&!ConfOverwrite(paths[i])) return;
    }
    if (DebugTraceF>0) {
        traceopen(TRACEFILE);
        tracelevel(DebugTraceF);
    }
    if (DebugStatusF>0) {
        rtkopenstat(STATFILE,DebugStatusF);
    }
    if (SolOpt.geoid>0&&GeoidDataFileF!="") {
        opengeoid(SolOpt.geoid,qPrintable(GeoidDataFileF));
    }
    if (DCBFileF!="") {
        readdcb(qPrintable(DCBFileF),&rtksvr.nav,NULL);
    }
    for (i=0;i<2;i++) {
        solopt[i]=SolOpt;
        solopt[i].posf=Format[i+3];
    }
    stropt[0]=TimeoutTime;
    stropt[1]=ReconTime;
    stropt[2]=1000;
    stropt[3]=SvrBuffSize;
    stropt[4]=FileSwapMargin;
    strsetopt(stropt);
    
    // start rtk server
    if (!rtksvrstart(&rtksvr,SvrCycle,SvrBuffSize,strs,paths,Format,NavSelect,
                     cmds,rcvopts,NmeaCycle,NmeaReq,nmeapos,&PrcOpt,solopt,
                     &monistr)) {
        traceclose();
        for (i=0;i<8;i++) delete[] paths[i];
        for (i=0;i<3;i++) delete[] rcvopts[i];
        for (i=0;i<3;i++) delete[] cmds[i];
        return;
    }
    for (i=0;i<8;i++) delete[] paths[i];
    for (i=0;i<3;i++) delete[] rcvopts[i];
    for (i=0;i<3;i++) delete[] cmds[i];
    PSol=PSolS=PSolE=0;
    SolStat[0]=Nvsat[0]=0;
    for (i=0;i<3;i++) SolRov[i]=SolRef[i]=VelRov[i]=0.0;
    for (i=0;i<9;i++) Qr[i]=0.0;
    Age[0]=Ratio[0]=0.0;
    Nsat[0]=Nsat[1]=0;
    UpdatePos();
    UpdatePlot();
    BtnStart   ->setVisible(false);
    BtnOpt      ->setEnabled(false);
    BtnExit     ->setEnabled(false);
    BtnInputStr ->setEnabled(false);
    MenuStartAction   ->setEnabled(false);
    MenuExitAction    ->setEnabled(false);
    ScbSol      ->setEnabled(false);
    BtnStop     ->setVisible(true);
    MenuStopAction    ->setEnabled(true);
    Svr->setStyleSheet("QLabel {background-color: rgb(255,128,0);}");
    SetTrayIcon(0);
}
// strop rtk server ---------------------------------------------------------
void  MainWindow::SvrStop(void)
{
    char *cmds[3]={0};
    int i,n,m,str;
    
    trace(3,"SvrStop\n");
    
    for (i=0;i<3;i++) {
        cmds[i]=new char[1024];
        cmds[i][0]='\0';
        str=rtksvr.stream[i].type;
        
        if (str==STR_SERIAL) {
            if (CmdEna[i][1]) strcpy(cmds[i],qPrintable(Cmds[i][1]));
        }
        else if (str==STR_TCPCLI||str==STR_TCPSVR||str==STR_NTRIPCLI) {
            if (CmdEnaTcp[i][1]) strcpy(cmds[i],qPrintable(CmdsTcp[i][1]));
        }
    }
    rtksvrstop(&rtksvr,cmds);
    for (i=0;i<3;i++) delete[] cmds[i];
    
    BtnStart    ->setVisible(true);
    BtnOpt      ->setEnabled(true);
    BtnExit     ->setEnabled(true);
    BtnInputStr ->setEnabled(true);
    MenuStartAction   ->setEnabled(true);
    MenuExitAction    ->setEnabled(true);
    ScbSol      ->setEnabled(true);
    BtnStop     ->setVisible(false);
    MenuStopAction    ->setEnabled(false);
    Svr->setStyleSheet("QLabel {background-color: gray;}");
    SetTrayIcon(1);
    
    LabelTime->setStyleSheet("QLabel {color: gray;}");
    IndSol->setStyleSheet("QLabel {color: white; background-color: white;}");
    n=PSolE-PSolS; if (n<0) n+=SolBuffSize;
    m=PSol-PSolS;  if (m<0) m+=SolBuffSize;
    if (n>0) {
        ScbSol->setMaximum(n-1); ScbSol->setValue(m);
    }
    Message->setText("");
    
    if (DebugTraceF>0) traceclose();
    if (DebugStatusF>0) rtkclosestat();
    if (SolOpt.geoid>0&&GeoidDataFileF!="") closegeoid();
}
// callback on interval timer -----------------------------------------------
void  MainWindow::TimerTimer()
{
    static int n=0,inactive=0;
    sol_t *sol;
    int i,update=0;
    
    trace(4,"TimerTimer\n");
    
    rtksvrlock(&rtksvr);
    
    for (i=0;i<rtksvr.nsol;i++) {
        sol=rtksvr.solbuf+i;
        UpdateLog(sol->stat,sol->time,sol->rr,sol->qr,rtksvr.rtk.rb,sol->ns,
                  sol->age,sol->ratio);
        update=1;
    }
    rtksvr.nsol=0;
    SolCurrentStat=rtksvr.state?rtksvr.rtk.sol.stat:0;
    
    rtksvrunlock(&rtksvr);
    
    if (update) {
        UpdateTime();
        UpdatePos();
        inactive=0;
    }
    else {
        if (++inactive*Timer.interval()>TIMEOUT) SolCurrentStat=0;
    }
    if (SolCurrentStat) {
        Svr->setStyleSheet("QLabel {background-color: rgb(0,255,0);}");
        LabelTime->setStyleSheet("QLabel { color: black;}");
    }
    else {
        IndSol->setStyleSheet("QLabel {color: white; background-color: white;}");
        Solution->setStyleSheet("QLabel {color: gray;}");
        Svr->setStyleSheet(rtksvr.state?"QLabel {background-color: green; }":"QLabel {background-color: gray; }");
    }
    if (!(++n%5)) UpdatePlot();

    UpdateStr();
    
    // keep alive for monitor port
    if (!(++n%(KACYCLE/Timer.interval()))&&OpenPort) {
        unsigned char buf[1];
        buf[0]='\r';
        strwrite(&monistr,buf,1);
    }
}
// change plot type ---------------------------------------------------------
void  MainWindow::ChangePlot(void)
{
}
// update time-system -------------------------------------------------------
void  MainWindow::UpdateTimeSys(void)
{
    QString label[]={tr("GPST"),tr("UTC"),tr("LT"),tr("GPST")};
    
    trace(3,"UpdateTimeSys\n");
    
    BtnTimeSys->setText(label[TimeSys]);
    UpdateTime();
}
// update solution type -----------------------------------------------------
void  MainWindow::UpdateSolType(void)
{
    QString label[]={
        tr("Lat/Lon/Height"),tr("Lat/Lon/Height"),tr("X/Y/Z-ECEF"),tr("E/N/U-Baseline"),
        tr("Pitch/Yaw/Length-Baseline"),""
    };
    trace(3,"UpdateSolType\n");
    
    Plabel0->setText(label[SolType]);

    UpdatePos();
}
// update log ---------------------------------------------------------------
void  MainWindow::UpdateLog(int stat, gtime_t time, double *rr,
    float *qr, double *rb, int ns, double age, double ratio)
{
    int i;
    
    if (!stat) return;
    
    trace(4,"UpdateLog\n");
    
    SolStat[PSolE]=stat; Time[PSolE]=time; Nvsat[PSolE]=ns; Age[PSolE]=age;
    Ratio[PSolE]=ratio;
    for (i=0;i<3;i++) {
        SolRov[i+PSolE*3]=rr[i];
        SolRef[i+PSolE*3]=rb[i];
        VelRov[i+PSolE*3]=rr[i+3];
    }
    Qr[  PSolE*9]=qr[0];
    Qr[4+PSolE*9]=qr[1];
    Qr[8+PSolE*9]=qr[2];
    Qr[1+PSolE*9]=Qr[3+PSolE*9]=qr[3];
    Qr[5+PSolE*9]=Qr[7+PSolE*9]=qr[4];
    Qr[2+PSolE*9]=Qr[6+PSolE*9]=qr[5];
    
    PSol=PSolE;
    if (++PSolE>=SolBuffSize) PSolE=0;
    if (PSolE==PSolS&&++PSolS>=SolBuffSize) PSolS=0;
}
// update font --------------------------------------------------------------
void  MainWindow::UpdateFont(void)
{
    QLabel *label[]={
        PlabelA,Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3,Solution,LabelStd,LabelNSat
    };
    QString color=label[7]->styleSheet();
    int i;
    
    trace(4,"UpdateFont\n");
    
    for (i=0;i<10;i++) label[i]->setFont(PosFont);
    QFont tmp=PosFont;
    tmp.setPointSize(9);
    label[0]->setFont(tmp); label[7]->setStyleSheet(color);
    tmp.setPointSize(8);
    label[8]->setFont(tmp); label[8]->setStyleSheet("QLabel {color: gray;}");;
    label[9]->setFont(tmp); label[9]->setStyleSheet("QLabel {color: gray;}");
}
// update time --------------------------------------------------------------
void  MainWindow::UpdateTime(void)
{
    gtime_t time=Time[PSol];
    struct tm *t;
    double tow;
    int week;
    char tstr[64];
    QString str;
    
    trace(4,"UpdateTime\n");
    
    if      (TimeSys==0) time2str(time,tstr,1);
    else if (TimeSys==1) time2str(gpst2utc(time),tstr,1);
    else if (TimeSys==2) {
        time=gpst2utc(time);
        if (!(t=localtime(&time.time))) str="2000/01/01 00:00:00.0";
        else str=QString("%1/%2/%3 %4:%5:%6.%7").arg(t->tm_year+1900,4,10,QChar('0'))
                     .arg(t->tm_mon+1,2,10,QChar('0')).arg(t->tm_mday,2,10,QChar('0')).arg(t->tm_hour,2,10,QChar('0')).arg(t->tm_min,2,10,QChar('0'))
                     .arg(t->tm_sec,2,10,QChar('0')).arg(static_cast<int>(time.sec*10));
    }
    else if (TimeSys==3) {
        tow=time2gpst(time,&week);
        str=QString("week %1 %2 s").arg(week,4,10,QChar('0')).arg(tow,8,'f',1);
    }
    LabelTime->setText(str);
}
// update solution display --------------------------------------------------
void  MainWindow::UpdatePos(void)
{
    QLabel *label[]={Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3,LabelStd,LabelNSat};
    QString sol[]={tr("----"),tr("FIX"),tr("FLOAT"),tr("SBAS"),tr("DGPS"),tr("SINGLE"),tr("PPP")};
    QString s[9],ext="";
    QString color[]={"silver","green","rgb(0,170,255)","rgb(255,0,255)","blue","red","rgb(128,0,128)"};
    double *rr=SolRov+PSol*3,*rb=SolRef+PSol*3,*qr=Qr+PSol*9,pos[3]={0},Qe[9]={0};
    double dms1[3]={0},dms2[3]={0},bl[3]={0},enu[3]={0},pitch=0.0,yaw=0.0,len;
    int i,stat=SolStat[PSol];
    
    trace(4,"UpdatePos\n");

    if (rtksvr.rtk.opt.mode==PMODE_STATIC||rtksvr.rtk.opt.mode==PMODE_PPP_STATIC) {
        ext=" (S)";
    }
    else if (rtksvr.rtk.opt.mode==PMODE_FIXED||rtksvr.rtk.opt.mode==PMODE_PPP_FIXED) {
        ext=" (F)";
    }
    PlabelA->setText("Solution"+ext+":");

    Solution->setText(sol[stat]);
    Solution->setStyleSheet(QString("QLabel {color: %1;}").arg(rtksvr.state?color[stat]:"gray"));
    IndSol->setStyleSheet(QString("QLabel {color: %1; background-color: %1}").arg(rtksvr.state&&stat?color[stat]:"white"));
    if (norm(rr,3)>0.0&&norm(rb,3)>0.0) {
        for (i=0;i<3;i++) bl[i]=rr[i]-rb[i];
    }
    len=norm(bl,3);
    if (SolType==0) {
        if (norm(rr,3)>0.0) {
            ecef2pos(rr,pos); covenu(pos,qr,Qe);
            degtodms(pos[0]*R2D,dms1);
            degtodms(pos[1]*R2D,dms2);
            if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
        }
        s[0]=pos[0]<0?tr("S:"):tr("N:"); s[1]=pos[1]<0?tr("W:"):tr("E:");
        s[2]=SolOpt.height==1?"H:":"He:";
        s[3]=QString("%1%2 %3' %4\"").arg(fabs(dms1[0]),0,'f',0).arg(degreeChar).arg(dms1[1],2,'f',0,'0').arg(dms1[2],7,'f',4,'0');
        s[4]=QString("%1%2 %3' %4\"").arg(fabs(dms2[0]),0,'f',0).arg(degreeChar).arg(dms2[1],2,'f',0,'0').arg(dms2[2],7,'f',4,'0');
        s[5]=QString("%1 m").arg(pos[2],0,'f',3);
        s[6]=QString(tr("N:%1 E:%2 U:%3 m")).arg(SQRT(Qe[4]),6,'f',3).arg(SQRT(Qe[0]),6,'f',3).arg(SQRT(Qe[8]),6,'f',3);
    }
    else if (SolType==1) {
        if (norm(rr,3)>0.0) {
            ecef2pos(rr,pos); covenu(pos,qr,Qe);
            if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
        }
        s[0]=pos[0]<0?"S:":"N:"; s[1]=pos[1]<0?"W:":"E:";
        s[2]=SolOpt.height==1?"H:":"He:";
        s[3]=QString("%1 %2").arg(fabs(pos[0])*R2D,0,'f',8).arg(degreeChar);
        s[4]=QString("%1 %2").arg(fabs(pos[1])*R2D,0,'f',8).arg(degreeChar);
        s[5]=QString("%1").arg(pos[2],0,'f',3);
        s[6]=QString(tr("N:%1 E:%2 U:%3 m")).arg(SQRT(Qe[4]),6,'f',3).arg(SQRT(Qe[0]),6,'f',3).arg(SQRT(Qe[8]),6,'f',3);
    }
    else if (SolType==2) {
        s[0]="X:"; s[1]="Y:"; s[2]="Z:";
        s[3]=QString("%1 m").arg(rr[0],0,'f',3);
        s[4]=QString("%1 m").arg(rr[1],0,'f',3);
        s[5]=QString("%1 m").arg(rr[2],0,'f',3);
        s[6]=QString("X:%1 Y:%2 Z:%3 m").arg(SQRT(qr[0]),6,'f',3).arg(SQRT(qr[4]),6,'f',3).arg(SQRT(qr[8]),6,'f',3);
    }
    else if (SolType==3) {
        if (len>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
        }
        s[0]="E:"; s[1]="N:"; s[2]="U:";
        s[3]=QString("%1 m").arg(enu[0],0,'f',3);
        s[4]=QString("%1 m").arg(enu[1],0,'f',3);
        s[5]=QString("%1 m").arg(enu[2],0,'f',3);
        s[6]=QString(tr("N:%1 E:%2 U:%3 m")).arg(SQRT(Qe[4]),6,'f',3).arg(SQRT(Qe[0]),6,'f',3).arg(SQRT(Qe[8]),6,'f',3);
    }
    else {
        if (len>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
            pitch=asin(enu[2]/len);
            yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
        }
        s[0]="P:"; s[1]="Y:"; s[2]="L:";
        s[3]=QString("%1 %2").arg(pitch*R2D,0,'f',3).arg(degreeChar);
        s[4]=QString("%1 %2").arg(yaw*R2D,0,'f',3).arg(degreeChar);
        s[5]=QString("%1 m").arg(len,0,'f',3);
        s[6]=QString(tr("N:%1 E:%2 U:%3 m")).arg(SQRT(Qe[4]),6,'f',3).arg(SQRT(Qe[0]),6,'f',3).arg(SQRT(Qe[8]),6,'f',3);
    }
    s[7]=QString(tr("Age:%1 s Ratio:%2 # Sat:%3")).arg(Age[PSol],4,'f',1).arg(Ratio[PSol],4,'f',1).arg(Nvsat[PSol],2);
    if (Ratio[PSol]>0.0) s[8]=QString(" R:%1").arg(Ratio[PSol],4,'f',1);

    for (i=0;i<8;i++) label[i]->setText(s[i]);
    for (i=3;i<6;i++) {
        label[i]->setStyleSheet(QString("QLabel {color: %1;}").arg(PrcOpt.mode==PMODE_MOVEB&&SolType<=2?"grey":"black"));
    }
    IndQ->setStyleSheet(IndSol->styleSheet());
    SolS->setText(Solution->text());
    SolS->setStyleSheet(Solution->styleSheet());
    SolQ->setText(ext+" "+label[0]->text()+" "+label[3]->text()+" "+
                  label[1]->text()+" "+label[4]->text()+" "+
                  label[2]->text()+" "+label[5]->text()+s[8]);
}
// update stream status indicators ------------------------------------------
void  MainWindow::UpdateStr(void)
{
    QString color[]={"red","gray","orange","rgb(0,128,0)","rgb(0,255,0)"};
    QLabel *ind[MAXSTRRTK]={Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8};
    int i,sstat[MAXSTRRTK]={0};
    char msg[MAXSTRMSG]="";
    
    trace(4,"UpdateStr\n");
    
    rtksvrsstat(&rtksvr,sstat,msg);
    for (i=0;i<MAXSTRRTK;i++) {
        ind[i]->setStyleSheet(QString("QLabel {background-color: %1}").arg(color[sstat[i]+1]));
        if (sstat[i]) {
            Message->setText(msg);
        }
    }
}
// draw solution plot -------------------------------------------------------
void  MainWindow::DrawPlot(QLabel *plot, int type, int freq)
{
    QString s1,s2;
    gtime_t time;

    if (!plot) return;

    QPixmap buffer(plot->size());

    if (buffer.isNull()) return;

    buffer.fill(Qt::red);

    QPainter *c= new QPainter(&buffer);
    QFont font;
    font.setPixelSize(8);
    c->setFont(font);

    QString fstr[]={"","L1 ","L2 ","L5 ","L6 ","L7 ","L8 ","","","",""};
    int w=buffer.size().width()-2,h=buffer.height()-2;
    int i,j,x,sat[2][MAXSAT],ns[2],snr[2][MAXSAT][NFREQ],vsat[2][MAXSAT];
    int *snr0[MAXSAT],*snr1[MAXSAT];
    char name[16];
    double az[2][MAXSAT],el[2][MAXSAT],rr[3],rs[6],e[3],pos[3],azel[2];
    
    trace(4,"DrawPlot\n");
    
    fstr[NFREQ+1]="SYS ";
    
    for (i=0;i<MAXSAT;i++) {
        snr0[i]=snr[0][i];
        snr1[i]=snr[1][i];
    }
    ns[0]=rtksvrostat(&rtksvr,0,&time,sat[0],az[0],el[0],snr0,vsat[0]);
    ns[1]=rtksvrostat(&rtksvr,1,&time,sat[1],az[1],el[1],snr1,vsat[1]);
    
    rtksvrlock(&rtksvr);
    matcpy(rr,rtksvr.rtk.sol.rr,3,1);
    ecef2pos(rr,pos);
    rtksvrunlock(&rtksvr);
    
    for (i=0;i<2;i++) {
        for (j=0;j<ns[i];j++) {
            if (az[i][j]!=0.0||el[i][j]!=0.0) continue;
            satno2id(sat[i][j],name);
            if (!tle_pos(time,name,"","",&TLEData,NULL,rs)) continue;
            if (geodist(rs,rr,e)>0.0) {
                satazel(pos,e,azel);
                az[i][j]=azel[0];
                el[i][j]=azel[1];
            }
        }
        if (ns[i]>0) {
            Nsat[i]=ns[i];
            for (int j=0;j<ns[i];j++) {
                Sat [i][j]=sat [i][j];
                Az  [i][j]=az  [i][j];
                El  [i][j]=el  [i][j];
                for (int k=0;k<NFREQ;k++) {
                    Snr[i][j][k]=snr[i][j][k];
                }
                Vsat[i][j]=vsat[i][j];
            }
        }
        else {
            for (j=0;j<Nsat[i];j++) {
                Vsat[i][j]=0;
                for (int k=0;k<NFREQ;k++) {
                    Snr[i][j][k]=0;
                }
            }
        }
    }
    c->setBrush(Qt::white);
    c->fillRect(buffer.rect(),QBrush(Qt::white));
    x=4;
    if (type==0) { // snr plot rover+base
         if (w<=3*h) { // vertical
             DrawSnr(c,w,(h-12)/2,0,15,0,freq);
             DrawSnr(c,w,(h-12)/2,0,14+(h-12)/2,1,freq);
             s1=QString("Rover:Base %1SNR (dBHz)").arg(fstr[freq]);
             DrawText(c,x,1,s1,Qt::gray,0);
         }
         else { // horizontal
             DrawSnr(c,w/2,h-15,0  ,15,0,freq);
             DrawSnr(c,w/2,h-15,w/2,15,1,freq);
             s1=QString("Rover %1 SNR (dBHz)").arg(fstr[freq]);
             s2=QString("Base %1 SNR (dBHz)").arg(fstr[freq]);
             DrawText(c,x,1,s1,Qt::gray,0);
             DrawText(c,w/2+x,1,s2,Qt::gray,0);
        }
    }
    else if (type==1) { // snr plot rover
        DrawSnr(c,w,h-15,0,15,0,freq);
        s1=QString(tr("Rover %1 SNR (dBHz)")).arg(fstr[freq]);
        DrawText(c,x,1,s1,Qt::gray,0);
    }
    else if (type==2) { // skyplot rover
        DrawSat(c,w,h,0,0,0,freq);
        s1=QString("Rover %1").arg(fstr[!freq?1:freq]);
        DrawText(c,x,1,s1,Qt::gray,0);
    }
    else if (type==3) { // skyplot+snr plot rover
        s1=QString("Rover %1").arg(fstr[!freq?1:freq]);
        s2=QString("SNR (dBHz)");
        if (w>=h*2) { // horizontal
            DrawSat(c,h,h,0,0,0,freq);
            DrawSnr(c,w-h,h-15,h,15,0,freq);
            DrawText(c,x,1,s1,Qt::gray,0);
            DrawText(c,x+h,1,s2,Qt::gray,0);
        }
        else { // vertical
            DrawSat(c,w,h/2,0,0,0,freq);
            DrawSnr(c,w,(h-12)/2,0,14+(h-12)/2,0,freq);
            DrawText(c,x,1,s1,Qt::gray,0);
        }
    }
    else if (type==4) { // skyplot rover+base
        s1=QString("Rover %1").arg(fstr[!freq?1:freq]);
        s2=QString("Base %1").arg(fstr[!freq?1:freq]);
        if (w>=h) { // horizontal
            DrawSat(c,w/2,h,0  ,0,0,freq);
            DrawSat(c,w/2,h,w/2,0,1,freq);
            DrawText(c,x,1,s1,Qt::gray,0);
            DrawText(c,x+w/2,1,s2,Qt::gray,0);
        }
        else { // vertical
            DrawSat(c,w,h/2,0,0  ,0,freq);
            DrawSat(c,w,h/2,0,h/2,1,freq);
            DrawText(c,x,1,s1,Qt::gray,0);
            DrawText(c,x,h/2+1,s2,Qt::gray,0);
        }
    }
    else if (type==5) { // baseline plot
        DrawBL(c,plot,w,h);
        DrawText(c,x,1,"Baseline",Qt::gray,0);
    } else if (type==6) { // track plot
        DrawTrk(c,plot,buffer);
        DrawText(c,x,3,"Gnd Trk",Qt::gray,0);
    }
    plot->setPixmap(buffer);
    delete c;
}
// update solution plot ------------------------------------------------------
void  MainWindow::UpdatePlot(void)
{
    DrawPlot(Disp1,PlotType1,FreqType1);
    DrawPlot(Disp2,PlotType2,FreqType2);
}
// snr color ----------------------------------------------------------------
QColor  MainWindow::SnrColor(int snr)
{
    QColor color[]={Qt::green,QColor(255,128,0,255),QColor(0,255,128,255),Qt::blue,Qt::red,Qt::gray};
    unsigned int r1,g1,b1;
    QColor c1,c2;
    double a;
    int i;
    
    if (snr<25) return color[5];
    if (snr<27) return color[4];
    if (snr>47) return color[0];
    a=(snr-27.5)/5.0;
    i=static_cast<int>(a);
    a-=i;
    c1=color[3-i];
    c2=color[4-i];
    r1=static_cast<unsigned int>(a*c1.red()  +(1.0-a)*c2.red())&0xFF;
    g1=static_cast<unsigned int>(a*c1.green()+(1.0-a)*c2.green())&0xFF;
    b1=static_cast<unsigned int>(a*c1.blue() +(1.0-a)*c2.blue())&0xFF;
    
    return QColor(r1,g1,b1);
}
// draw snr plot ------------------------------------------------------------
void  MainWindow::DrawSnr(QPainter *c, int w, int h, int x0, int y0,
	int index, int freq)
{
    static const QColor color[]={
        QColor(0,128,0),QColor(0,128,128),QColor(0xA0,0,0xA0),
        QColor(128,0,0),QColor(0,0,128),QColor(128,128,128)
    };
    static const QColor color_sys[]={
        Qt::green,QColor(0,0xAA,0xFF),QColor(255,0,255),Qt::blue,Qt::red,Qt::gray
    };
    QString s;
    int i,j,k,l,n,x1,y1,y2,y3,hh=h-15,ww,www,snr[NFREQ+1],mask[6]={0};
    char id[16],sys[]="GREJCS",*q;
    
    trace(4,"DrawSnr: w=%d h=%d x0=%d y0=%d index=%d freq=%d\n",w,h,x0,y0,index,freq);
    for (snr[0]=MINSNR+10;snr[0]<MAXSNR;snr[0]+=10) {
        y1=y0+hh-(snr[0]-MINSNR)*hh/(MAXSNR-MINSNR);
        c->setPen(QColor(0x0c,0x0c,0x0c));
        c->drawLine(x0+3,y1,x0+w-13,y1);
        DrawText(c,x0+w-9,y1,QString::number(snr[0]),Qt::gray,1);
    }

    y1=y0+hh;
    QRect b(x0+1,y0,x0+w-2,y1);
    c->setBrush(Qt::NoBrush);
    c->setPen(Qt::gray);

    c->drawRect(b);
    
    for (i=0;i<Nsat[index]&&i<MAXSAT;i++) {
        
        ww=(w-16)/Nsat[index];
        www=ww-2<8?ww-2:8;
        x1=x0+i*(w-16)/Nsat[index]+ww/2;
        satno2id(Sat[index][i],id);
        l=(q=strchr(sys,id[0]))?(int)(q-sys):5;
        
        for (j=snr[0]=0;j<NFREQ;j++) {
            snr[j+1]=Snr[index][i][j];
            if ((freq&&freq==j+1)||((!freq||freq>NFREQ)&&snr[j+1]>snr[0])) {
                snr[0]=snr[j+1];
            }
        }
        for (j=0;j<NFREQ+2;j++) {
            k=j<NFREQ+1?j:0;
            y3=j<NFREQ+1?0:2;
            y2=y1-y3;
            if (snr[k]>0) y2-=(snr[k]-MINSNR)*hh/(MAXSNR-MINSNR)-y3;
            y2=y2<2?2:(y1<y2?y1:y2);
            
            QRect r1(x1,y1,www,y2-y1);
            if (j==0) {
                c->setBrush(QBrush(freq<NFREQ?SnrColor(snr[k]):color_sys[l],Qt::SolidPattern));
                if (!Vsat[index][i]) c->setBrush(QBrush(QColor(0x0c,0x0c,0x0c),Qt::SolidPattern));
                c->drawRect(r1);
            }
            else {
                c->setPen(j<NFREQ+1?QColor(0x0c,0x0c,0x0c):Qt::gray);
                c->setBrush(Qt::NoBrush);
                c->drawRect(r1);
            }
        }
        DrawText(c,x1+www/2,y1+6,(s=id+1),color[l],1);
        mask[l]=1;
    }
    for (i=n=0;i<6;i++) if (mask[i]) n++;
    for (i=j=0;i<6;i++) {
        if (!mask[i]) continue;
        sprintf(id,"%c",sys[i]);
        DrawText(c,x0+w-15+8*(-n+j++),y0+3,(s=id),color[i],0);
    }
}
// draw satellites in skyplot -----------------------------------------------
void  MainWindow::DrawSat(QPainter *c, int w, int h, int x0, int y0,
    int index, int freq)
{
    static const QColor color_sys[]={
        Qt::green,QColor(0x00,0xAA,0xFF),QColor(0xff,0x00,0xff),Qt::blue,Qt::red,Qt::gray
    };
    QString s;
    QPoint p(w/2,h/2);
    double r=MIN(w*0.95,h*0.95)/2,azel[MAXSAT*2],dop[4];
    int i,k,l,d,x[MAXSAT],y[MAXSAT],ns=0,f=!freq?0:freq-1;
    char id[16],sys[]="GREJCS",*q;
    
    trace(4,"DrawSat: w=%d h=%d index=%d freq=%d\n",w,h,index,freq);
    
    DrawSky(c,w,h,x0,y0);
    
    for (i=0,k=Nsat[index]-1;i<Nsat[index]&&i<MAXSAT;i++,k--) {
        if (El[index][k]<=0.0) continue;
        if (Vsat[index][k]) {
            azel[ns*2]=Az[index][k]; azel[1+ns*2]=El[index][k];
            ns++;
        }
        satno2id(Sat[index][k],id);
        l=(q=strchr(sys,id[0]))?(int)(q-sys):5;
        x[i]=static_cast<int>(p.x()+r*(90-El[index][k]*R2D)/90*sin(Az[index][k]))+x0;
        y[i]=static_cast<int>(p.y()-r*(90-El[index][k]*R2D)/90*cos(Az[index][k]))+y0;
        c->setPen(Qt::gray);
        d=SATSIZE/2;
        c->setBrush(!Vsat[index][k]?QColor(0xc0,0xc0,0xc0):
                        (freq<NFREQ?SnrColor(Snr[index][k][f]):color_sys[l]));
        c->drawEllipse(x[i]-d,y[i]-d,2*d+1,2*d+1);
        DrawText(c,x[i],y[i],s=id,Qt::white,1);
    }
    dops(ns,azel,0.0,dop);
    DrawText(c,x0+3,y0+h-15,QString("# Sat:%1").arg(Nsat[index],2),Qt::gray,0);
    DrawText(c,x0+w-3,y0+h-15,QString("GDOP:%1").arg(dop[0],0,'f',1),Qt::gray,2);
}
// draw baseline plot -------------------------------------------------------
void  MainWindow::DrawBL(QPainter *c,QLabel *disp, int w, int h)
{
    QColor color[]={QColor(0xc0,0xc0,0xc0),Qt::green,QColor(0x00,0xAA,0xFF),QColor(0xff,0x00,0xff),Qt::blue,Qt::red,QColor(0x80,0x80,0x00)};
    QString label[]={tr("N"),tr("E"),tr("S"),tr("W")};
    QPoint p(w/2,h/2),p1,p2,pp;
    double r=MIN(w*0.95,h*0.95)/2;
    double *rr=SolRov+PSol*3,*rb=SolRef+PSol*3;
    double bl[3]={0},pos[3],enu[3],len=0.0,pitch=0.0,yaw=0.0;
    double cp,q,az=0.0;
    QColor col=Qt::white;
    int i,d1=10,d2=16,d3=10,cy=0,sy=0,cya=0,sya=0,a,x1,x2,y1,y2,r1,digit,mode;
    
    trace(4,"DrawBL: w=%d h=%d\n",w,h);
    
    mode=disp==Disp1?BLMode1:BLMode2;

    if (PMODE_DGPS<=PrcOpt.mode&&PrcOpt.mode<=PMODE_FIXED) {
        col=rtksvr.state&&SolStat[PSol]&&SolCurrentStat?color[SolStat[PSol]]:Qt::white;
        
        if (norm(rr,3)>0.0&&norm(rb,3)>0.0) {
            for (i=0;i<3;i++) bl[i]=rr[i]-rb[i];
        }
        if ((len=norm(bl,3))>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu);
            pitch=asin(enu[2]/len);
            yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
            if (mode) az=yaw;
        }
    }
    if (len>=MINBLLEN) {
        cp =cos(pitch);
        cy =static_cast<int>((r-d1-d2/2)*cp*cos(yaw-az));
        sy =static_cast<int>((r-d1-d2/2)*cp*sin(yaw-az));
        cya=static_cast<int>(((r-d1-d2/2)*cp-d2/2-4)*cos(yaw-az));
        sya=static_cast<int>(((r-d1-d2/2)*cp-d2/2-4)*sin(yaw-az));
    }
    p1.setX(p.x()-sy); p1.setY(p.y()+cy); // base
    p2.setX(p.x()+sy); p2.setY(p.y()-cy); // rover
    
    c->setPen(Qt::gray);
    c->drawEllipse(p.x()-r,p.y()-r,2*r+1,2*r+1);
    r1=static_cast<int>(r-d1/2);
    c->drawEllipse(p.x()-r1,p.y()-r1,2*r1+1,2*r1+1);
    
    pp=pitch<0.0?p2:p1;
    c->setPen(QColor(0xc0,0xc0,0xc0));
    c->drawLine(p,pp);
    if (pitch<0.0) {
        c->setBrush(Qt::white);
        c->drawEllipse(pp.x()-d2/2,pp.y()-d2/2,d2+1,d2+1);
        DrawArrow(c,p.x()+sya,p.y()-cya,d3,static_cast<int>((yaw-az)*R2D),QColor(0xc0,0xc0,0xc0));
    }
    c->setBrush(col);
    c->drawEllipse(pp.x()-d2/2+2,pp.y()-d2/2+2,d2-1,d2-1);
    for (a=0;a<360;a+=5) {
        q=a%90==0?0:(a%30==0?r-d1*3:(a%10==0?r-d1*2:r-d1));
        x1=static_cast<int>(r*sin(a*D2R-az));
        y1=static_cast<int>(r*cos(a*D2R-az));
        x2=static_cast<int>(q*sin(a*D2R-az));
        y2=static_cast<int>(q*cos(a*D2R-az));
        c->setPen(QColor(0xc0,0xc0,0xc0));
        c->drawLine(p.x()+x1,p.y()-y1,p.x()+x2,p.y()-y2);
        c->setBrush(Qt::white);
        if (a%90==0) {
            DrawText(c,p.x()+x1,p.y()-y1,label[a/90],Qt::gray,1);
        }
        if (a==0) {
            x1=static_cast<int>((r-d1*3/2)*sin(a*D2R-az));
            y1=static_cast<int>((r-d1*3/2)*cos(a*D2R-az));
            DrawArrow(c,p.x()+x1,p.y()-y1,d3,-static_cast<int>(az*R2D),QColor(0xc0,0xc0,0xc0));
        }
    }
    pp=pitch>=0.0?p2:p1;
    c->setPen(Qt::gray);
    c->drawLine(p,pp);
    if (pitch>=0.0) {
        c->setBrush(Qt::white);
        c->drawEllipse(pp.x()-d2/2,pp.y()-d2/2,d2,d2);
        DrawArrow(c,p.x()+sya,p.y()-cya,d3,static_cast<int>((yaw-az)*R2D),Qt::gray);
    }
    c->setBrush(col);
    c->drawEllipse(pp.x()-d2/2+2,pp.y()-d2/2+2,d2-4,d2-4);
    c->setBrush(Qt::white);
    digit=len<10.0?3:(len<100.0?2:(len<1000.0?1:0));
    DrawText(c,p.x(),p.y() ,QString("%1 m").arg(len,0,'f',digit),Qt::gray,1);
    DrawText(c,5,  h-15,QString("Y: %1%2").arg(yaw*R2D,0,'f',1).arg(degreeChar),Qt::gray,0);
    DrawText(c,w-3,h-15,QString("P: %1%2").arg(pitch*R2D,0,'f',1).arg(degreeChar),Qt::gray,2);
}
// draw track plot ----------------------------------------------------------
void MainWindow::DrawTrk(QPainter *c, QLabel *disp, QPixmap &buff)
{
    QColor mcolor[]={QColor(0xc0,0xc0,0xc0),Qt::green,QColor(0x00,0xAA,0xFF),QColor(0xff,0x00,0xff),Qt::blue,Qt::red,QColor(0x80,0x80,0x00)};
    Graph *graph = new Graph(&buff);
    QVector<QColor> color;
    QPoint p1,p2;
    QString label;
    double scale[]={
        0.00021,0.00047,0.001,0.0021,0.0047,0.01,0.021,0.047,0.1,0.21,0.47,
        1.0,2.1,4.7,10.0,21.0,47.0,100.0,210.0,470.0,1000.0,2100.0,4700.0,
        10000.0
    };
    double *x,*y,xt,yt,sx,sy,ref[3],pos[3],dr[3],enu[3];
    int i,j,k,n=0,type,scl;

    trace(3,"DrawTrk\n");

    type=disp==Disp1?TrkType1 :TrkType2 ;
    scl =disp==Disp1?TrkScale1:TrkScale2;

    x=new double[SolBuffSize];
    y=new double[SolBuffSize];

    if (norm(TrkOri,3)<1E-6) {
        if (norm(SolRef+PSol*3,3)>1E-6) {
            matcpy(TrkOri,SolRef+PSol*3,3,1);
        }
        else {
            matcpy(TrkOri,SolRov+PSol*3,3,1);
        }
    }
    if (norm(SolRef+PSol*3,3)>1E-6) {
        matcpy(ref,SolRef+PSol*3,3,1);
    }
    else {
        matcpy(ref,TrkOri,3,1);
    }
    ecef2pos(ref,pos);
    for (i=k=PSolS;i!=PSolE;) {
        for (j=0;j<3;j++) dr[j]=SolRov[j+i*3]-ref[j];
        if (i==PSol) k=n;
        ecef2enu(pos,dr,enu);
        x[n]=enu[0];
        y[n]=enu[1];
        color.append(mcolor[SolStat[i]]);
        n++;
        if (++i>=SolBuffSize) i=0;
    }
    graph->SetSize(buff.width(),buff.height());
    graph->SetScale(scale[scl],scale[scl]);
    graph->Color[1]=QColor(0xc0,0xc0,0xc0);

    if (n>0) {
        graph->SetCent(x[k],y[k]);
    }
    if (type==1) {
        graph->XLPos=7;
        graph->YLPos=7;
        graph->DrawCircles(*c,0);
    }
    else {
        graph->XLPos=2;
        graph->YLPos=4;
        graph->DrawAxis(*c,0,0);
    }
    graph->DrawPoly(*c,x,y,n,QColor(0xc0,0xc0,0xc0),0);
    graph->DrawMarks(*c,x,y,color,n,0,3,0);
    if (n>0) {
        graph->ToPoint(x[k],y[k],p1);
        graph->DrawMark(*c,p1,0,Qt::white,18,0);
        graph->DrawMark(*c,p1,1,Qt::black,16,0);
        graph->DrawMark(*c,p1,5,Qt::black,20,0);
        graph->DrawMark(*c,p1,0,Qt::black,12,0);
        graph->DrawMark(*c,p1,0,color.at(k),10,0);
    }
    // scale
    graph->GetPos(p1,p2);
    graph->GetTick(xt,yt);
    graph->GetScale(sx,sy);
    p2.rx()=p2.x()-35;
    p2.ry()=p2.y()-12;
    graph->DrawMark(*c,p2,11,Qt::gray,static_cast<int>(xt/sx+0.5),0);
    p2.ry()=p2.y()-2;
    if      (xt<0.01  ) label.sprintf("%.0f mm",xt*1000.0);
    else if (xt<1.0   ) label.sprintf("%.0f cm",xt*100.0);
    else if (xt<1000.0) label.sprintf("%.0f m" ,xt);
    else                label.sprintf("%.0f km",xt/1000.0);
    graph->DrawText(*c,p2,label,Qt::gray,Qt::white,0,1,0);

    // ref position
    if (norm(ref,3)>1E-6) {
        p1.rx()+=2;
        p1.ry()=p2.y()+12;
        label=QString("%1 %2").arg(pos[0]*R2D,0,'f').arg(pos[1]*R2D,0,'f',9);
        graph->DrawText(*c,p1,label,Qt::gray,Qt::white,1,1,0);
    }
    delete graph;
    delete [] x;
    delete [] y;
}
// draw skyplot -------------------------------------------------------------
void  MainWindow::DrawSky(QPainter *c, int w, int h, int x0, int y0)
{
    QString label[]={tr("N"),tr("E"),tr("S"),tr("W")};
    QPoint p(x0+w/2,y0+h/2);
    double r=MIN(w*0.95,h*0.95)/2;
    int a,e,d,x,y;
    
    c->setBrush(Qt::white);
    for (e=0;e<90;e+=30) {
        d=static_cast<int>(r*(90-e)/90);
        c->setPen(e==0?Qt::gray:QColor(0xc0,0xc0,0xc0));
        c->drawEllipse(p.x()-d,p.y()-d,2*d+1,2*d+1);
    }
    for (a=0;a<360;a+=45) {
        x=static_cast<int>(r*sin(a*D2R));
        y=static_cast<int>(r*cos(a*D2R));
        c->setPen(QColor(0xc0,0xc0,0xc0));
        c->drawLine(p.x(),p.y(),p.x()+x,p.y()-y);
        if (a%90==0) DrawText(c,p.x()+x,p.y()-y,label[a/90],Qt::gray,1);
    }
}
// draw text ----------------------------------------------------------------
void  MainWindow::DrawText(QPainter *c, int x, int y, const QString &s,
    const QColor &color, int align)
{
    int flags=0;
    switch (align) {
    case 0: flags|=Qt::AlignLeft;break;
    case 1: flags|=Qt::AlignHCenter;break;
    case 2: flags|=Qt::AlignRight;break;
    }

    QRectF off=c->boundingRect(QRectF(),flags,s);

    c->setPen(color);

    c->translate(x,y);
    c->drawText(off,s);
    c->translate(-x,-y);
}
// draw arrow ---------------------------------------------------------------
void  MainWindow::DrawArrow(QPainter *c, int x, int y, int siz,
    int ang, const QColor &color)
{
    QPoint p1[4],p2[4];
    int i;
    
    p1[0].setX(0); p1[1].setX(siz/2); p1[2].setX(-siz/2); p1[3].setX(0);
    p1[0].setY(siz/2); p1[1].setY(-siz/2);p1[2].setY(-siz/2); p1[3].setY(siz/2);
    
    for (i=0;i<4;i++) {
        p2[i].setX(x+static_cast<int>(p1[i].x()*cos(-ang*D2R)-p1[i].y()*sin(-ang*D2R)+0.5));
        p2[i].setY(y-static_cast<int>(p1[i].x()*sin(-ang*D2R)+p1[i].y()*cos(-ang*D2R)+0.5));
    }
    c->setBrush(QBrush(color,Qt::SolidPattern));
    c->setPen(color);
    c->drawPolygon(p2,3);
}
// open monitor port --------------------------------------------------------
void  MainWindow::OpenMoniPort(int port)
{
    QString s;
    int i;
    char path[64];
    
    if (port<=0) return;
    
    trace(3,"OpenMoniPort: port=%d\n",port);
    
    for (i=0;i<=MAXPORTOFF;i++) {
        
        sprintf(path,":%d",port+i);
        
        if (stropen(&monistr,STR_TCPSVR,STR_MODE_RW,path)) {
            strsettimeout(&monistr,TimeoutTime,ReconTime);
            if (i>0) setWindowTitle(QString(tr("%1 ver.%2 (%3)")).arg(PRGNAME).arg(VER_RTKLIB).arg(i+1));
            OpenPort=MoniPort+i;
            return;
        }
    }
    QMessageBox::critical(this,tr("Error"),QString(tr("monitor port %1-%2 open error")).arg(port).arg(port+MAXPORTOFF));
    OpenPort=0;
}
// initialize solution buffer -----------------------------------------------
void  MainWindow::InitSolBuff(void)
{
    double ep[]={2000,1,1,0,0,0};
    int i,j;
    
    trace(3,"InitSolBuff\n");
    
    delete [] Time;   delete [] SolStat; delete [] Nvsat;  delete [] SolRov;
    delete [] SolRef; delete [] Qr;      delete [] VelRov; delete [] Age;
    delete [] Ratio;
    
    if (SolBuffSize<=0) SolBuffSize=1;
    Time   =new gtime_t[SolBuffSize];
    SolStat=new int[SolBuffSize];
    Nvsat  =new int[SolBuffSize];
    SolRov =new double[SolBuffSize*3];
    SolRef =new double[SolBuffSize*3];
    VelRov =new double[SolBuffSize*3];
    Qr     =new double[SolBuffSize*9];
    Age    =new double[SolBuffSize];
    Ratio  =new double[SolBuffSize];
    PSol=PSolS=PSolE=0;
    for (i=0;i<SolBuffSize;i++) {
        Time[i]=epoch2time(ep);
        SolStat[i]=Nvsat[i]=0;
        for (j=0;j<3;j++) SolRov[j+i*3]=SolRef[j+i*3]=VelRov[j+i*3]=0.0;
        for (j=0;j<9;j++) Qr[j+i*9]=0.0;
        Age[i]=Ratio[i]=0.0;
    }
    ScbSol->setMaximum(0); ScbSol->setValue(0);
}
// save log file ------------------------------------------------------------
void  MainWindow::SaveLog(void)
{
    QString fileName;
    int posf[]={SOLF_LLH,SOLF_LLH,SOLF_XYZ,SOLF_ENU,SOLF_ENU,SOLF_LLH};
    solopt_t opt;
    sol_t sol={0};
    double  ep[6],pos[3];
    QString fileTemplate;
    int i;
    
    trace(3,"SaveLog\n");
    
    time2epoch(timeget(),ep);
    fileTemplate=QString("rtk_%1%2%3%4%5%6.txt")
            .arg(ep[0],4,'f',0,QChar('0')).arg(ep[1],2,'f',0,QChar('0')).arg(ep[2],2,'f',0,QChar('0'))
            .arg(ep[3],2,'f',0,QChar('0')).arg(ep[4],2,'f',0,QChar('0')).arg(ep[5],2,'f',0,QChar('0'));
    fileName=QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,QString(),fileTemplate));
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this,tr("Error"),tr("log file open error"));
        return;
    }
    QTextStream str(&file);

    opt=SolOpt;
    opt.posf=posf[SolType];
    if (SolOpt.outhead) {
        QString data;

        data=QString("%% program   : %1 ver.%2\n").arg(PRGNAME).arg(VER_RTKLIB);
        str<<data;
        if (PrcOpt.mode==PMODE_DGPS||PrcOpt.mode==PMODE_KINEMA||
            PrcOpt.mode==PMODE_STATIC) {
            ecef2pos(PrcOpt.rb,pos);
            data=QString("%% ref pos   :%1 %2 %3\n").arg(pos[0]*R2D,13,'f',9)
                    .arg(pos[1]*R2D,14,'f',9).arg(pos[2],10,'f',4);
            str<<data;
        }
        file.write("%%\n");
    }
    FILE* f = fdopen(file.handle(), "w");
    outsolhead(f,&opt);

    for (i=PSolS;i!=PSolE;) {
        sol.time=Time[i];
        matcpy(sol.rr,SolRov+i*3,3,1);
        sol.stat=SolStat[i];
        sol.ns=Nvsat[i];
        sol.ratio=Ratio[i];
        sol.age=Age[i];
        outsol(f,&sol,SolRef+i*3,&opt);
        if (++i>=SolBuffSize) i=0;
    }
}
// load navigation data -----------------------------------------------------
void  MainWindow::LoadNav(nav_t *nav)
{
    QSettings settings(IniFile,QSettings::IniFormat);
    QString str;
    eph_t eph0;
    char buff[2049],*p;
    int i;
    
    trace(3,"LoadNav\n");
    
    memset(&eph0,0,sizeof(eph_t));

    for (i=0;i<MAXSAT;i++) {
        if ((str=settings.value(QString("navi/eph_%1").arg(i,2)).toString()).isEmpty()) continue;
        nav->eph[i]=eph0;
        strcpy(buff,qPrintable(str));
        if (!(p=strchr(buff,','))) continue;
        *p='\0';
        if (!(nav->eph[i].sat=satid2no(buff))) continue;
        sscanf(p+1,"%d,%d,%d,%d,%ld,%ld,%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d",
               &nav->eph[i].iode,
               &nav->eph[i].iodc,
               &nav->eph[i].sva ,
               &nav->eph[i].svh ,
               &nav->eph[i].toe.time,
               &nav->eph[i].toc.time,
               &nav->eph[i].ttr.time,
               &nav->eph[i].A   ,
               &nav->eph[i].e   ,
               &nav->eph[i].i0  ,
               &nav->eph[i].OMG0,
               &nav->eph[i].omg ,
               &nav->eph[i].M0  ,
               &nav->eph[i].deln,
               &nav->eph[i].OMGd,
               &nav->eph[i].idot,
               &nav->eph[i].crc ,
               &nav->eph[i].crs ,
               &nav->eph[i].cuc ,
               &nav->eph[i].cus ,
               &nav->eph[i].cic ,
               &nav->eph[i].cis ,
               &nav->eph[i].toes,
               &nav->eph[i].fit ,
               &nav->eph[i].f0  ,
               &nav->eph[i].f1  ,
               &nav->eph[i].f2  ,
               &nav->eph[i].tgd[0],
               &nav->eph[i].code,
               &nav->eph[i].flag);
    }
    str=settings.value("navi/ion","").toString();
    QStringList tokens=str.split(",");
    for (i=0;i<8;i++) nav->ion_gps[i]=0.0;
    for (i=0;(i<8)&&(i<tokens.size());i++) nav->ion_gps[i]=tokens.at(i).toDouble();

    str=settings.value("navi/utc","").toString();
    tokens=str.split(",");
    for (i=0;i<4;i++) nav->utc_gps[i]=0.0;
    for (i=0;(i<4)&&(i<tokens.size());i++) nav->utc_gps[i]=tokens.at(i).toDouble();
    
    nav->leaps=settings.value("navi/leaps",0).toInt();
    
}
// save navigation data -----------------------------------------------------
void  MainWindow::SaveNav(nav_t *nav)
{
    QSettings settings(IniFile,QSettings::IniFormat);
    QString str;
    char id[32];
    int i;
    
    trace(3,"SaveNav\n");
    
    for (i=0;i<MAXSAT;i++) {
        if (nav->eph[i].ttr.time==0) continue;
        str="";
        satno2id(nav->eph[i].sat,id);
        str=str+id+",";
        str=str+QString("%1,").arg(nav->eph[i].iode);
        str=str+QString("%1,").arg(nav->eph[i].iodc);
        str=str+QString("%1,").arg(nav->eph[i].sva);
        str=str+QString("%1,").arg(nav->eph[i].svh);
        str=str+QString("%1,").arg(static_cast<int>(nav->eph[i].toe.time));
        str=str+QString("%1,").arg(static_cast<int>(nav->eph[i].toc.time));
        str=str+QString("%1,").arg(static_cast<int>(nav->eph[i].ttr.time));
        str=str+QString("%1,").arg(nav->eph[i].A,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].e,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].i0,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].OMG0,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].omg,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].M0,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].deln,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].OMGd,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].idot,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].crc,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].crs,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].cuc,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].cus,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].cic,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].cis,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].toes,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].fit,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].f0,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].f1,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].f2,0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].tgd[0],0,'E',14);
        str=str+QString("%1,").arg(nav->eph[i].code);
        str=str+QString("%1,").arg(nav->eph[i].flag);
        settings.setValue(QString("navi/eph_%1").arg(i,2),str);
    }
    str="";
    for (i=0;i<8;i++) str=str+QString("%1,").arg(nav->ion_gps[i],0,'E',14);
    settings.setValue("navi/ion",str);
    
    str="";
    for (i=0;i<4;i++) str=str+QString("%1,").arg(nav->utc_gps[i],0,'E',14);
    settings.setValue("navi/utc",str);
    
    settings.setValue("navi/leaps",nav->leaps);

}
// set tray icon ------------------------------------------------------------
void  MainWindow::SetTrayIcon(int index)
{
    QPixmap pix(":/buttons/navi.bmp");
    systemTray->setIcon(QIcon(pix.copy(index*16,0,16,16)));
}
// load option from ini file ------------------------------------------------
void  MainWindow::LoadOpt(void)
{
    QSettings settings(IniFile,QSettings::IniFormat);
    QString s;
    int i,j,no,strno[]={0,1,6,2,3,4,5,7};
    
    trace(3,"LoadOpt\n");

    for (i=0;i<8;i++) {
        no=strno[i];
        StreamC[i]=settings.value(QString("stream/streamc%1").arg(no),0).toInt();
        Stream [i]=settings.value(QString("stream/stream%1").arg(no),0).toInt();
        Format [i]=settings.value(QString("stream/format%1").arg(no),0).toInt();
        for (j=0;j<4;j++) {
            Paths[i][j]=settings.value(QString("stream/path_%1_%2").arg(no).arg(j),"").toString();
        }
    }
    for (i=0;i<3;i++) {
        RcvOpt [i]=settings.value(QString("stream/rcvopt%1").arg(i+1),"").toString();
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        Cmds[i][j]=settings.value(QString("serial/cmd_%1_%2").arg(i).arg(j),"").toString();
        CmdEna[i][j]=settings.value(QString("serial/cmdena_%1_%2").arg(i).arg(j),0).toInt();
        Cmds[i][j].replace("@@","\r\n");
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        CmdsTcp[i][j]=settings.value(QString("tcpip/cmd_%1_%2").arg(i).arg(j),"").toString();
        CmdEnaTcp[i][j]=settings.value(QString("tcpip/cmdena_%1_%2").arg(i).arg(j),0).toInt();
        CmdsTcp[i][j].replace("@@","\r\n");
    }
    PrcOpt.mode     =settings.value("prcopt/mode",            0).toInt();
    PrcOpt.nf       =settings.value("prcopt/nf",              2).toInt();
    PrcOpt.elmin    =settings.value("prcopt/elmin",    15.0*D2R).toInt();
    PrcOpt.snrmask.ena[0]=settings.value("prcopt/snrmask_ena1",0).toInt();
    PrcOpt.snrmask.ena[1]=settings.value("prcopt/snrmask_ena2",0).toInt();
    for (i=0;i<NFREQ;i++) for (j=0;j<9;j++) {
        PrcOpt.snrmask.mask[i][j]=
            settings.value(QString("prcopt/snrmask_%1_%2").arg(i+1).arg(j+1),0.0).toInt();
    }
    PrcOpt.dynamics =settings.value("prcopt/dynamics",        0).toInt();
    PrcOpt.tidecorr =settings.value("prcopt/tidecorr",        0).toInt();
    PrcOpt.modear   =settings.value("prcopt/modear",          1).toInt();
    PrcOpt.glomodear=settings.value("prcopt/glomodear",       0).toInt();
    PrcOpt.bdsmodear=settings.value("prcopt/bdsmodear",       0).toInt();
    PrcOpt.maxout   =settings.value("prcopt/maxout",          5).toInt();
    PrcOpt.minlock  =settings.value("prcopt/minlock",         0).toInt();
    PrcOpt.minfix   =settings.value("prcopt/minfix",         10).toInt();
    PrcOpt.ionoopt  =settings.value("prcopt/ionoopt",IONOOPT_BRDC).toInt();
    PrcOpt.tropopt  =settings.value("prcopt/tropopt",TROPOPT_SAAS).toInt();
    PrcOpt.sateph   =settings.value("prcopt/ephopt",  EPHOPT_BRDC).toInt();
    PrcOpt.niter    =settings.value("prcopt/niter",           1).toInt();
    PrcOpt.eratio[0]=settings.value("prcopt/eratio0",     100.0).toDouble();
    PrcOpt.eratio[1]=settings.value("prcopt/eratio1",     100.0).toDouble();
    PrcOpt.err[1]   =settings.value("prcopt/err1",        0.003).toDouble();
    PrcOpt.err[2]   =settings.value("prcopt/err2",        0.003).toDouble();
    PrcOpt.err[3]   =settings.value("prcopt/err3",          0.0).toDouble();
    PrcOpt.err[4]   =settings.value("prcopt/err4",          1.0).toDouble();
    PrcOpt.prn[0]   =settings.value("prcopt/prn0",         1E-4).toDouble();
    PrcOpt.prn[1]   =settings.value("prcopt/prn1",         1E-3).toDouble();
    PrcOpt.prn[2]   =settings.value("prcopt/prn2",         1E-4).toDouble();
    PrcOpt.prn[3]   =settings.value("prcopt/prn3",         10.0).toDouble();
    PrcOpt.prn[4]   =settings.value("prcopt/prn4",         10.0).toDouble();
    PrcOpt.sclkstab =settings.value("prcopt/sclkstab",    5E-12).toDouble();
    PrcOpt.thresar[0]=settings.value("prcopt/thresar",       3.0).toDouble();
    PrcOpt.elmaskar =settings.value("prcopt/elmaskar",      0.0).toDouble();
    PrcOpt.elmaskhold=settings.value("prcopt/elmaskhold",    0.0).toDouble();
    PrcOpt.thresslip=settings.value("prcopt/thresslip",    0.05).toDouble();
    PrcOpt.maxtdiff =settings.value("prcopt/maxtdiff",     30.0).toDouble();
    PrcOpt.maxgdop  =settings.value("prcopt/maxgdop",      30.0).toDouble();
    PrcOpt.maxinno  =settings.value("prcopt/maxinno",      30.0).toDouble();
    PrcOpt.syncsol  =settings.value("prcopt/syncsol",         0).toInt();
    ExSats          =settings.value("prcopt/exsats",         "").toString();
    PrcOpt.navsys   =settings.value("prcopt/navsys",    SYS_GPS).toInt();
    PrcOpt.posopt[0]=settings.value("prcopt/posopt1",         0).toInt();
    PrcOpt.posopt[1]=settings.value("prcopt/posopt2",         0).toInt();
    PrcOpt.posopt[2]=settings.value("prcopt/posopt3",         0).toInt();
    PrcOpt.posopt[3]=settings.value("prcopt/posopt4",         0).toInt();
    PrcOpt.posopt[4]=settings.value("prcopt/posopt5",         0).toInt();
    PrcOpt.posopt[5]=settings.value("prcopt/posopt6",         0).toInt();
    PrcOpt.maxaveep =settings.value("prcopt/maxaveep",     3600).toInt();
    PrcOpt.initrst  =settings.value("prcopt/initrst",         1).toInt();

    BaselineC       =settings.value("prcopt/baselinec",       0).toInt();
    Baseline[0]     =settings.value("prcopt/baseline1",     0.0).toDouble();
    Baseline[1]     =settings.value("prcopt/baseline2",     0.0).toDouble();
    
    SolOpt.posf     =settings.value("solopt/posf",            0).toInt();
    SolOpt.times    =settings.value("solopt/times",           0).toInt();
    SolOpt.timef    =settings.value("solopt/timef",           1).toInt();
    SolOpt.timeu    =settings.value("solopt/timeu",           3).toInt();
    SolOpt.degf     =settings.value("solopt/degf",            0).toInt();
    s=settings.value("solopt/sep"," ").toString();
    strcpy(SolOpt.sep,qPrintable(s));
    SolOpt.outhead  =settings.value("solop/outhead",         0).toInt();
    SolOpt.outopt   =settings.value("solopt/outopt",          0).toInt();
    SolOpt.datum    =settings.value("solopt/datum",           0).toInt();
    SolOpt.height   =settings.value("solopt/height",          0).toInt();
    SolOpt.geoid    =settings.value("solopt/geoid",           0).toInt();
    SolOpt.nmeaintv[0]=settings.value("solopt/nmeaintv1",     0.0).toDouble();
    SolOpt.nmeaintv[1]=settings.value("solopt/nmeaintv2",     0.0).toDouble();
    DebugStatusF    =settings.value("setting/debugstatus",     0).toInt();
    DebugTraceF     =settings.value("setting/debugtrace",      0).toInt();
    
    RovPosTypeF     =settings.value("setting/rovpostype",      0).toInt();
    RefPosTypeF     =settings.value("setting/refpostype",      0).toInt();
    RovAntPcvF      =settings.value("setting/rovantpcv",       0).toInt();
    RefAntPcvF      =settings.value("setting/refantpcv",       0).toInt();
    RovAntF         =settings.value("setting/rovant",         "").toString();
    RefAntF         =settings.value("setting/refant",         "").toString();
    SatPcvFileF     =settings.value("setting/satpcvfile",     "").toString();
    AntPcvFileF     =settings.value("setting/antpcvfile",     "").toString();
    StaPosFileF     =settings.value("setting/staposfile",     "").toString();
    GeoidDataFileF  =settings.value("setting/geoiddatafile",  "").toString();
    DCBFileF        =settings.value("setting/dcbfile",        "").toString();
    EOPFileF        =settings.value("setting/eopfile",        "").toString();
    TLEFileF        =settings.value("setting/tlefile",        "").toString();
    TLESatFileF     =settings.value("setting/tlesatfile",     "").toString();
    LocalDirectory  =settings.value("setting/localdirectory","C:\\Temp").toString();
    
    SvrCycle        =settings.value("setting/svrcycle",       10).toInt();
    TimeoutTime     =settings.value("setting/timeouttime", 10000).toInt();
    ReconTime       =settings.value("setting/recontime",   10000).toInt();
    NmeaCycle       =settings.value("setting/nmeacycle",    5000).toInt();
    SvrBuffSize     =settings.value("setting/svrbuffsize", 32768).toInt();
    SolBuffSize     =settings.value("setting/solbuffsize",  1000).toInt();
    SavedSol        =settings.value("setting/savedsol",      100).toInt();
    NavSelect       =settings.value("setting/navselect",       0).toInt();
    PrcOpt.sbassatsel=settings.value("setting/sbassat",        0).toInt();
    DgpsCorr        =settings.value("setting/dgpscorr",        0).toInt();
    SbasCorr        =settings.value("setting/sbascorr",        0).toInt();
    
    NmeaReq         =settings.value("setting/nmeareq",         0).toInt();
    InTimeTag       =settings.value("setting/intimetag",       0).toInt();
    InTimeSpeed     =settings.value("setting/intimespeed",  "x1").toString();
    InTimeStart     =settings.value("setting/intimestart",   "0").toString();
    OutTimeTag      =settings.value("setting/outtimetag",      0).toInt();
    OutAppend       =settings.value("setting/outappend",       0).toInt();
    OutSwapInterval =settings.value("setting/outswapinterval","").toString();
    LogTimeTag      =settings.value("setting/logtimetag",      0).toInt();
    LogAppend       =settings.value("setting/logappend",       0).toInt();
    LogSwapInterval =settings.value("setting/logswapinterval","").toString();
    NmeaPos[0]      =settings.value("setting/nmeapos1",      0.0).toDouble();
    NmeaPos[1]      =settings.value("setting/nmeapos2",      0.0).toDouble();
    FileSwapMargin  =settings.value("setting/fswapmargin",    30).toInt();
    
    TimeSys         =settings.value("setting/timesys",         0).toInt();
    SolType         =settings.value("setting/soltype",         0).toInt();
    PlotType1       =settings.value("setting/plottype",        0).toInt();
    PlotType2       =settings.value("setting/plottype2",       0).toInt();
    PanelMode       =settings.value("setting/panelmode",       0).toInt();
    ProxyAddr       =settings.value("setting/proxyaddr",      "").toString();
    MoniPort        =settings.value("setting/moniport",DEFAULTPORT).toInt();
    PanelStack      =settings.value("setting/panelstack",      0).toInt();
    TrkType1        =settings.value("setting/trktype1",        0).toInt();
    TrkType2        =settings.value("setting/trktype2",        0).toInt();
    TrkScale1       =settings.value("setting/trkscale1",       5).toInt();
    TrkScale2       =settings.value("setting/trkscale2",       5).toInt();
    BLMode1         =settings.value("setting/blmode1",         0).toInt();
    BLMode2         =settings.value("setting/blmode2",         0).toInt();
    MarkerName      =settings.value("setting/markername",     "").toString();
    MarkerComment   =settings.value("setting/markercomment",  "").toString();

    for (i=0;i<3;i++) {
        RovAntDel[i]=settings.value(QString("setting/rovantdel_%1").arg(i),0.0).toDouble();
        RefAntDel[i]=settings.value(QString("setting/refantdel_%1").arg(i),0.0).toDouble();
        RovPos   [i]=settings.value(QString("setting/rovpos_%1").arg(i),0.0).toDouble();
        RefPos   [i]=settings.value(QString("setting/refpos_%1").arg(i),0.0).toDouble();
    }
    for (i=0;i<10;i++) {
        History[i]=settings.value(QString("tcpopt/history%1").arg(i),"").toString();
    }
    for (i=0;i<10;i++) {
        MntpHist[i]=settings.value(QString("tcpopt/mntphist%1").arg(i),"").toString();
    }
    NMapPnt        =settings.value("mapopt/nmappnt",0).toInt();
    for (i=0;i<NMapPnt;i++) {
        PntName[i]=settings.value(QString("mapopt/pntname%1").arg(i+1),"").toString();
        QString pos=settings.value(QString("mapopt/pntpos%1").arg(i+1),"0,0,0").toString();
        PntPos[i][0]=PntPos[i][1]=PntPos[i][2]=0.0;
        sscanf(qPrintable(pos),"%lf,%lf,%lf",PntPos[i],PntPos[i]+1,PntPos[i]+2);
    }
    PosFont.setFamily(settings.value("setting/posfontname",POSFONTNAME).toString());
    PosFont.setPointSize(settings.value("setting/posfontsize",POSFONTSIZE).toInt());
//    PosFont.setStyle(QColor(settings.value("setting/posfontcolor",(int)clBlack).toInt());
    if (settings.value("setting/posfontbold",  0).toInt()) PosFont.setBold(true);
    if (settings.value("setting/posfontitalic",0).toInt()) PosFont.setItalic(true);;
    
    TextViewer::Color1=QColor(static_cast<QRgb>(settings.value("viewer/color1",static_cast<int>(Qt::black)).toInt()));
    TextViewer::Color2=QColor(static_cast<QRgb>(settings.value("viewer/color2",static_cast<int>(Qt::white)).toInt()));
    TextViewer::FontD.setFamily(settings.value("viewer/fontname","Courier New").toString());
    TextViewer::FontD.setPointSize(settings.value("viewer/fontsize",9).toInt());
    
    UpdatePanel();

    Splitter1->restoreState(settings.value("window/splitpos").toByteArray());
    Splitter2->restoreState(settings.value("window/splitpos2").toByteArray());

    resize(settings.value("window/width",   388).toInt(),
           settings.value("window/height",  284).toInt());
}
// save option to ini file --------------------------------------------------
void  MainWindow::SaveOpt(void)
{
    QSettings settings(IniFile,QSettings::IniFormat);
    int i,j,no,strno[]={0,1,6,2,3,4,5,7};
    
    trace(3,"SaveOpt\n");
    
    for (i=0;i<8;i++) {
        no=strno[i];
        settings.setValue(QString("stream/streamc%1").arg(no),StreamC[i]);
        settings.setValue(QString("stream/stream%1").arg(no),Stream [i]);
        settings.setValue(QString("stream/format%1").arg(no),Format [i]);
        for (j=0;j<4;j++) {
            settings.setValue(QString("stream/path_%1_%2").arg(no).arg(j),Paths[i][j]);
        }
    }
    for (i=0;i<3;i++) {
        settings.setValue(QString("stream/rcvopt%1").arg(i+1),RcvOpt[i]);
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        Cmds[i][j].replace("\r\n","@@");
        settings.setValue(QString("serial/cmd_%1_%2").arg(i).arg(j),Cmds  [i][j]);
        settings.setValue(QString("serial/cmdena_%1_%2").arg(i).arg(j),CmdEna[i][j]);
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        CmdsTcp[i][j].replace("\r\n","@@");
        settings.setValue (QString("tcpip/cmd_%1_%2").arg(i).arg(j),CmdsTcp  [i][j]);
        settings.setValue(QString("tcpip/cmdena_%1_%2").arg(i).arg(j),CmdEnaTcp[i][j]);
    }
    settings.setValue("prcopt/mode",       PrcOpt.mode        );
    settings.setValue("prcopt/nf",         PrcOpt.nf          );
    settings.setValue("prcopt/elmin",      PrcOpt.elmin       );
    settings.setValue("prcopt/snrmask_ena1",PrcOpt.snrmask.ena[0]);
    settings.setValue("prcopt/snrmask_ena2",PrcOpt.snrmask.ena[1]);
    for (i=0;i<NFREQ;i++) for (j=0;j<9;j++) {
        settings.setValue(QString("prcopt/snrmask_%1_%2").arg(i+1).arg(j+1),
                        PrcOpt.snrmask.mask[i][j]);
    }
    settings.setValue("prcopt/dynamics",   PrcOpt.dynamics    );
    settings.setValue("prcopt/tidecorr",   PrcOpt.tidecorr    );
    settings.setValue("prcopt/modear",     PrcOpt.modear      );
    settings.setValue("prcopt/glomodear",  PrcOpt.glomodear   );
    settings.setValue("prcopt/bdsmodear",  PrcOpt.bdsmodear   );
    settings.setValue("prcopt/maxout",     PrcOpt.maxout      );
    settings.setValue("prcopt/minlock",    PrcOpt.minlock     );
    settings.setValue("prcopt/minfix",     PrcOpt.minfix      );
    settings.setValue("prcopt/ionoopt",    PrcOpt.ionoopt     );
    settings.setValue("prcopt/tropopt",    PrcOpt.tropopt     );
    settings.setValue("prcopt/ephopt",     PrcOpt.sateph      );
    settings.setValue("prcopt/niter",      PrcOpt.niter       );
    settings.setValue("prcopt/eratio0",    PrcOpt.eratio[0]   );
    settings.setValue("prcopt/eratio1",    PrcOpt.eratio[1]   );
    settings.setValue("prcopt/err1",       PrcOpt.err[1]      );
    settings.setValue("prcopt/err2",       PrcOpt.err[2]      );
    settings.setValue("prcopt/err3",       PrcOpt.err[3]      );
    settings.setValue("prcopt/err4",       PrcOpt.err[4]      );
    settings.setValue("prcopt/prn0",       PrcOpt.prn[0]      );
    settings.setValue("prcopt/prn1",       PrcOpt.prn[1]      );
    settings.setValue("prcopt/prn2",       PrcOpt.prn[2]      );
    settings.setValue("prcopt/prn3",       PrcOpt.prn[3]      );
    settings.setValue("prcopt/prn4",       PrcOpt.prn[4]      );
    settings.setValue("prcopt/sclkstab",   PrcOpt.sclkstab    );
    settings.setValue("prcopt/thresar",    PrcOpt.thresar[0]  );
    settings.setValue("prcopt/elmaskar",   PrcOpt.elmaskar    );
    settings.setValue("prcopt/elmaskhold", PrcOpt.elmaskhold  );
    settings.setValue("prcopt/thresslip",  PrcOpt.thresslip   );
    settings.setValue("prcopt/maxtdiff",   PrcOpt.maxtdiff    );
    settings.setValue("prcopt/maxgdop",    PrcOpt.maxgdop     );
    settings.setValue("prcopt/maxinno",    PrcOpt.maxinno     );
    settings.setValue("prcopt/syncsol",    PrcOpt.syncsol     );
    settings.setValue("prcopt/exsats",     ExSats             );
    settings.setValue("prcopt/navsys",     PrcOpt.navsys      );
    settings.setValue("prcopt/posopt1",    PrcOpt.posopt[0]   );
    settings.setValue("prcopt/posopt2",    PrcOpt.posopt[1]   );
    settings.setValue("prcopt/posopt3",    PrcOpt.posopt[2]   );
    settings.setValue("prcopt/posopt4",    PrcOpt.posopt[3]   );
    settings.setValue("prcopt/posopt5",    PrcOpt.posopt[4]   );
    settings.setValue("prcopt/posopt6",    PrcOpt.posopt[5]   );
    settings.setValue("prcopt/maxaveep",   PrcOpt.maxaveep    );
    settings.setValue("prcopt/initrst",    PrcOpt.initrst     );

    settings.setValue("prcopt/baselinec",  BaselineC          );
    settings.setValue("prcopt/baseline1",  Baseline[0]        );
    settings.setValue("prcopt/baseline2",  Baseline[1]        );
    
    settings.setValue("solopt/posf",       SolOpt.posf        );
    settings.setValue("solopt/times",      SolOpt.times       );
    settings.setValue("solopt/timef",      SolOpt.timef       );
    settings.setValue("solopt/timeu",      SolOpt.timeu       );
    settings.setValue("solopt/degf",       SolOpt.degf        );
    settings.setValue("solopt/sep",        SolOpt.sep         );
    settings.setValue("solopt/outhead",    SolOpt.outhead     );
    settings.setValue("solopt/outopt",     SolOpt.outopt      );
    settings.setValue("solopt/datum",      SolOpt.datum       );
    settings.setValue("solopt/height",     SolOpt.height      );
    settings.setValue("solopt/geoid",      SolOpt.geoid       );
    settings.setValue("solopt/nmeaintv1",  SolOpt.nmeaintv[0] );
    settings.setValue("solopt/nmeaintv2",  SolOpt.nmeaintv[1] );
    settings.setValue("setting/debugstatus",DebugStatusF       );
    settings.setValue("setting/debugtrace", DebugTraceF        );
    
    settings.setValue("setting/rovpostype", RovPosTypeF        );
    settings.setValue("setting/refpostype", RefPosTypeF        );
    settings.setValue("setting/rovantpcv",  RovAntPcvF         );
    settings.setValue("setting/refantpcv",  RefAntPcvF         );
    settings.setValue("setting/rovant",     RovAntF            );
    settings.setValue("setting/refant",     RefAntF            );
    settings.setValue("setting/satpcvfile", SatPcvFileF        );
    settings.setValue("setting/antpcvfile", AntPcvFileF        );
    settings.setValue("setting/staposfile", StaPosFileF        );
    settings.setValue("setting/geoiddatafile",GeoidDataFileF   );
    settings.setValue("setting/dcbfile",    DCBFileF           );
    settings.setValue("setting/eopfile",    EOPFileF           );
    settings.setValue("setting/tlefile",    TLEFileF           );
    settings.setValue("setting/tlesatfile", TLESatFileF        );
    settings.setValue("setting/localdirectory",LocalDirectory  );
    
    settings.setValue("setting/svrcycle",   SvrCycle           );
    settings.setValue("setting/timeouttime",TimeoutTime        );
    settings.setValue("setting/recontime",  ReconTime          );
    settings.setValue("setting/nmeacycle",  NmeaCycle          );
    settings.setValue("setting/svrbuffsize",SvrBuffSize        );
    settings.setValue("setting/solbuffsize",SolBuffSize        );
    settings.setValue("setting/savedsol",   SavedSol           );
    settings.setValue("setting/navselect",  NavSelect          );
    settings.setValue("setting/sbassat",    PrcOpt.sbassatsel  );
    settings.setValue("setting/dgpscorr",   DgpsCorr           );
    settings.setValue("setting/sbascorr",   SbasCorr           );
    
    settings.setValue("setting/nmeareq",    NmeaReq            );
    settings.setValue("setting/intimetag",  InTimeTag          );
    settings.setValue("setting/intimespeed",InTimeSpeed        );
    settings.setValue("setting/intimestart",InTimeStart        );
    settings.setValue("setting/outtimetag", OutTimeTag         );
    settings.setValue("setting/outappend",  OutAppend          );
    settings.setValue("setting/outswapinterval",OutSwapInterval);
    settings.setValue("setting/logtimetag", LogTimeTag         );
    settings.setValue("setting/logappend",  LogAppend          );
    settings.setValue("setting/logswapinterval",LogSwapInterval);
    settings.setValue("setting/nmeapos1",   NmeaPos[0]         );
    settings.setValue("setting/nmeapos2",   NmeaPos[1]         );
    settings.setValue("setting/fswapmargin",FileSwapMargin     );
    
    settings.setValue("setting/timesys",    TimeSys            );
    settings.setValue("setting/soltype",    SolType            );
    settings.setValue("setting/plottype",   PlotType1          );
    settings.setValue("setting/plottype2",  PlotType2          );
    settings.setValue("setting/panelmode",  PanelMode          );
    settings.setValue("setting/proxyaddr",  ProxyAddr          );
    settings.setValue("setting/moniport",   MoniPort           );
    settings.setValue("setting/panelstack", PanelStack         );
    settings.setValue("setting/trktype1",   TrkType1           );
    settings.setValue("setting/trktype2",   TrkType2           );
    settings.setValue("setting/trkscale1",  TrkScale1          );
    settings.setValue("setting/trkscale2",  TrkScale2          );
    settings.setValue("setting/blmode1",    BLMode1            );
    settings.setValue("setting/blmode2",    BLMode2            );
    settings.setValue("setting/markername", MarkerName         );
    settings.setValue("setting/markercomment",MarkerComment    );

    for (i=0;i<3;i++) {
        settings.setValue(QString("setting/rovantdel_%1").arg(i),RovAntDel[i]);
        settings.setValue(QString("setting/refantdel_%1").arg(i),RefAntDel[i]);
        settings.setValue(QString("setting/rovpos_%1").arg(i),   RovPos[i]);
        settings.setValue(QString("setting/refpos_%1").arg(i),   RefPos[i]);
    }
    for (i=0;i<10;i++) {
        settings.setValue(QString("tcpopt/history%1").arg(i),History [i]);
    }
    for (i=0;i<10;i++) {
        settings.setValue(QString("tcpopt/mntphist%1").arg(i),MntpHist[i]);
    }
    settings.setValue("mapopt/nmappnt",NMapPnt);
    for (i=0;i<NMapPnt;i++) {
        settings.setValue(QString("mapopt/pntname%1").arg(i+1),PntName[i]);
        settings.setValue(QString("mapopt/pntpos%1").arg(i+1),
            QString("%1,%2,%3").arg(PntPos[i][0],0,'f',4).arg(PntPos[i][1],0,'f',4).arg(PntPos[i][2],0,'f',4));
    }
    settings.setValue("setting/posfontname", PosFont.family()    );
    settings.setValue("setting/posfontsize", PosFont.pointSize()    );
//    settings.setValue("setting/posfontcolor",(int)PosFont->Color);
    settings.setValue("setting/posfontbold",  PosFont.bold());
    settings.setValue("setting/posfontitalic",PosFont.italic());

    settings.setValue("viewer/color1",  static_cast<int>(TextViewer::Color1.rgb()));
    settings.setValue("viewer/color2",  static_cast<int>(TextViewer::Color2.rgb()));
    settings.setValue("viewer/fontname",TextViewer::FontD.family());
    settings.setValue("viewer/fontsize",TextViewer::FontD.pointSize());
    
    settings.setValue("window/width",    size().width());
    settings.setValue("window/height",   size().height());

    settings.setValue("window/splitpos",Splitter1->saveState());
    settings.setValue("window/splitpos2",Splitter2->saveState());
}
//---------------------------------------------------------------------------
void MainWindow::BtnMarkClick()
{
    QMarkDialog *markDialog=new QMarkDialog(this);
    markDialog->PosMode=rtksvr.rtk.opt.mode;
    markDialog->Marker=MarkerName;
    markDialog->Comment=MarkerComment;

    if (markDialog->exec()!=QDialog::Accepted) return;
    rtksvr.rtk.opt.mode=markDialog->PosMode;
    MarkerName=markDialog->Marker;
    MarkerComment=markDialog->Comment;

    delete markDialog;
}
//---------------------------------------------------------------------------


