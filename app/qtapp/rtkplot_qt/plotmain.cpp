//---------------------------------------------------------------------------
// rtkplot : visualization of solution and obs data ap
//
//          Copyright (C) 2007-2020 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtkplot [-t title][-i file][-r][-p path][-x level][file ...]
//
//           -t title  window title
//           -i file   ini file path
//           -r        open file as obs and nav file
//           -p path   connect to path
//                       serial://port[:brate[:bsize[:parity[:stopb[:fctr]]]]]
//                       tcpsvr://:port
//                       tcpcli://addr[:port]
//                       ntrip://[user[:passwd]@]addr[:port][/mntpnt]
//                       file://path
//           -p1 path  connect port 1 to path
//           -p2 path  connect port 2 to path
//           -x level  debug trace level (0:off)
//           file      solution files or rinex obs/nav file
//
// version : $Revision: 1.1 $ $Date: 2008/07/17 22:15:27 $
// history : 2008/07/14  1.0 new
//           2009/11/27  1.1 rtklib 2.3.0
//           2010/07/18  1.2 rtklib 2.4.0
//           2010/06/10  1.3 rtklib 2.4.1
//           2010/06/19  1.4 rtklib 2.4.1 p1
//           2012/11/21  1.5 rtklib 2.4.2
//           2016/06/11  1.6 rtklib 2.4.3
//           2020/11/30  1.7 support NavIC/IRNSS
//                           delete functions for Google Earth View
//                           support Map API by Leaflet for Map View
//                           move Google Map API Key option to Map View
//                           modify order of ObsType selections
//                           improve slip detection performance by LG jump
//                           fix bug on MP computation for L3,L4,... freq.

//---------------------------------------------------------------------------
#include <clocale>

#include <QShowEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QDebug>
#include <QToolBar>
#include <QScreen>
#include <QtGlobal>
#include <QFileInfo>

#include <QFileInfo>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QClipboard>
#include <QSettings>
#include <QFont>
#include <QMimeData>

#include "rtklib.h"
#include "plotmain.h"
#include "plotopt.h"
#include "refdlg.h"
#include "tspandlg.h"
#include "aboutdlg.h"
#include "conndlg.h"
#include "console.h"
#include "pntdlg.h"
#include "mapoptdlg.h"
#include "skydlg.h"
#include "freqdlg.h"
#include "mapview.h"
#include "viewer.h"
#include "vmapdlg.h"
#include "fileseldlg.h"

#define YLIM_AGE    10.0        // ylimit of age of differential
#define YLIM_RATIO  20.0        // ylimit of raito factor
#define MAXSHAPEFILE 16             // max number of shape files

static int RefreshTime = 100;    // update only every 100ms

extern QString color2String(const QColor &c);

extern "C" {
extern void settime(gtime_t) {}
extern void settspan(gtime_t, gtime_t) {}
}

// instance of Plot --------------------------------------------------------
Plot *plot = NULL;

// constructor --------------------------------------------------------------
Plot::Plot(QWidget *parent) : QMainWindow(parent)
{
    setupUi(this);

    plot = this;

    setWindowIcon(QIcon(":/icons/rtk2.bmp"));

    setlocale(LC_NUMERIC, "C"); // use point as decimal separator in formatted output

    gtime_t t0 = { 0, 0 };
    nav_t nav0;
    obs_t obs0 = { 0, 0, NULL };
    sta_t sta0;
    gis_t gis0 = {};
    solstatbuf_t solstat0 = { 0, 0, 0 };
    double ep[] = { 2000, 1, 1, 0, 0, 0 }, xl[2], yl[2];
    double xs[] = { -DEFTSPAN / 2, DEFTSPAN / 2 };

    memset(&nav0, 0, sizeof(nav_t));
    memset(&sta0, 0, sizeof(sta_t));

    QString file = QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile = fi.absolutePath() + "/" + fi.baseName() + ".ini";

    toolBar->addWidget(Panel1);

    FormWidth = FormHeight = 0;
    Drag = 0; Xn = Yn = -1; NObs = 0;
    IndexObs = NULL;
    Az = NULL; El = NULL;
    Week = Flush = PlotType = 0;
    AnimCycle = 1;
    for (int i = 0; i < 2; i++) {
        initsolbuf(SolData + i, 0, 0);
        SolStat[i] = solstat0;
        SolIndex[i] = 0;
    }
    ObsIndex = 0;
    Obs = obs0;
    Nav = nav0;
    Sta = sta0;
    Gis = gis0;
    SimObs = 0;

    X0 = Y0 = Xc = Yc = Xs = Ys = Xcent = 0.0;
    MouseDownTick = 0;
    GEState = GEDataState[0] = GEDataState[1] = 0;
    GEHeading = 0.0;
    OEpoch = t0;
    for (int i = 0; i < 3; i++) OPos[i] = OVel[i] = 0.0;
    Az = El = NULL;
    for (int i = 0; i < NFREQ + NEXOBS; i++) Mp[i] = NULL;

    GraphT = new Graph(Disp);
    GraphT->Fit = 0;

    for (int i = 0; i < 3; i++) {
        GraphG[i] = new Graph(Disp);
        GraphG[i]->XLPos = 0;
        GraphG[i]->GetLim(xl, yl);
        GraphG[i]->SetLim(xs, yl);
    }
    GraphR = new Graph(Disp);
    for (int i = 0; i < 2; i++)
        GraphE[i] = new Graph(Disp);
    GraphS = new Graph(Disp);

    GraphR->GetLim(xl, yl);
    GraphR->SetLim(xs, yl);

    MapSize[0] = MapSize[1] = 0;
    MapScaleX = MapScaleY = 0.1;
    MapScaleEq = 0;
    MapLat = MapLon = 0.0;
    PointType = 0;

    NWayPnt = 0;
    SelWayPnt = -1;

    SkySize[0] = SkySize[1] = 0;
    SkyCent[0] = SkyCent[1] = 0;
    SkyScale = SkyScaleR = 240.0;
    SkyFov[0] = SkyFov[1] = SkyFov[2] = 0.0;
    SkyElMask = 1;
    SkyDestCorr = SkyRes = SkyFlip = 0;

    for (int i = 0; i < 10; i++) SkyDest[i] = 0.0;

    SkyBinarize = 0;
    SkyBinThres1 = 0.3;
    SkyBinThres2 = 0.1;

    for (int i = 0; i < 3; i++) TimeEna[i] = 0;
    TimeLabel = AutoScale = ShowStats = 0;
    ShowLabel = ShowGLabel = 1;
    ShowArrow = ShowSlip = ShowHalfC = ShowErr = ShowEph = 0;
    PlotStyle = MarkSize = Origin = RcvPos = 0;
    TimeInt = ElMask = YRange = 0.0;
    MaxDop = 30.0;
    MaxMP = 10.0;
    TimeStart = TimeEnd = epoch2time(ep);
    Console1 = new Console(this);
    Console2 = new Console(this);
    RangeList->setParent(0);
    RangeList->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);

    for (int i = 0; i < 361; i++) ElMaskData[i] = 0.0;

    Trace = 0;
    ConnectState = OpenRaw = 0;
    RtConnType = 0;
    strinitcom();
    strinit(Stream);
    strinit(Stream + 1);

    FrqType->clear();
    FrqType->addItem("L1/LC");
    for (int i=0;i<NFREQ;i++) {
        FrqType->addItem(QString("L%1").arg(i+1));
    }
    FrqType->setCurrentIndex(0);

    TLEData.n = TLEData.nmax = 0;
    TLEData.data = NULL;

    freqDialog = new FreqDialog(this);
    mapOptDialog = new MapOptDialog(this);
    mapView = new MapView(this);
    spanDialog = new SpanDialog(this);
    connectDialog = new ConnectDialog(this);
    skyImgDialog = new SkyImgDialog(this);
    plotOptDialog = new PlotOptDialog(this);
    aboutDialog = new AboutDialog(this);
    pntDialog = new PntDialog(this);
    fileSelDialog = new FileSelDialog(this);
    viewer = new TextViewer(this);

    BtnConnect->setDefaultAction(MenuConnect);
    BtnReload->setDefaultAction(MenuReload);
    BtnClear->setDefaultAction(MenuClear);
    BtnOptions->setDefaultAction(MenuOptions);
    BtnMapView->setDefaultAction(MenuMapView);
    BtnCenterOri->setDefaultAction(MenuCenterOri);
    BtnFitHoriz->setDefaultAction(MenuFitHoriz);
    BtnFitVert->setDefaultAction(MenuFitVert);
    BtnShowTrack->setDefaultAction(MenuShowTrack);
    BtnFixCent->setDefaultAction(MenuFixCent);
    BtnFixHoriz->setDefaultAction(MenuFixHoriz);
    BtnFixVert->setDefaultAction(MenuFixVert);
    BtnShowMap->setDefaultAction(MenuShowMap);
    BtnShowGrid->setDefaultAction(MenuShowGrid);
    BtnShowImg->setDefaultAction(MenuShowImg);
    BtnShowSkyplot->setDefaultAction(MenuShowSkyplot);
    MenuShowSkyplot->setChecked(true);
    MenuShowGrid->setChecked(true);

    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    DirSelector = new QTreeView(this);
    Panel2->layout()->addWidget(DirSelector);
    DirSelector->setModel(dirModel);
    DirSelector->hideColumn(1); DirSelector->hideColumn(2); DirSelector->hideColumn(3); //only show names

    fileModel = new QFileSystemModel(this);
    fileModel->setFilter((fileModel->filter() & ~QDir::Dirs & ~QDir::AllDirs));
    fileModel->setNameFilterDisables(false);
    FileList->setModel(fileModel);


    connect(BtnOn1, SIGNAL(clicked(bool)), this, SLOT(BtnOn1Click()));
    connect(BtnOn2, SIGNAL(clicked(bool)), this, SLOT(BtnOn2Click()));
    connect(BtnOn3, SIGNAL(clicked(bool)), this, SLOT(BtnOn3Click()));
    connect(BtnSol1, SIGNAL(clicked(bool)), this, SLOT(BtnSol1Click()));//FIXME Double Click
    connect(BtnSol2, SIGNAL(clicked(bool)), this, SLOT(BtnSol2Click()));
    connect(BtnSol12, SIGNAL(clicked(bool)), this, SLOT(BtnSol12Click()));
    connect(BtnRangeList, SIGNAL(clicked(bool)), this, SLOT(BtnRangeListClick()));
    connect(BtnAnimate, SIGNAL(clicked(bool)), this, SLOT(BtnAnimateClick()));
    connect(MenuAbout, SIGNAL(triggered(bool)), this, SLOT(MenuAboutClick()));
    connect(MenuAnimStart, SIGNAL(triggered(bool)), this, SLOT(MenuAnimStartClick()));
    connect(MenuAnimStop, SIGNAL(triggered(bool)), this, SLOT(MenuAnimStopClick()));
    connect(MenuBrowse, SIGNAL(triggered(bool)), this, SLOT(MenuBrowseClick()));
    connect(MenuCenterOri, SIGNAL(triggered(bool)), this, SLOT(MenuCenterOriClick()));
    connect(MenuClear, SIGNAL(triggered(bool)), this, SLOT(MenuClearClick()));
    connect(MenuConnect, SIGNAL(triggered(bool)), this, SLOT(MenuConnectClick()));
    connect(MenuDisconnect, SIGNAL(triggered(bool)), this, SLOT(MenuDisconnectClick()));
    connect(MenuFitHoriz, SIGNAL(triggered(bool)), this, SLOT(MenuFitHorizClick()));
    connect(MenuFitVert, SIGNAL(triggered(bool)), this, SLOT(MenuFitVertClick()));
    connect(MenuFixCent, SIGNAL(triggered(bool)), this, SLOT(MenuFixCentClick()));
    connect(MenuFixHoriz, SIGNAL(triggered(bool)), this, SLOT(MenuFixHorizClick()));
    connect(MenuFixVert, SIGNAL(triggered(bool)), this, SLOT(MenuFixVertClick()));
    connect(MenuMapView, SIGNAL(triggered(bool)), this, SLOT(MenuMapViewClick()));
    connect(MenuMapImg, SIGNAL(triggered(bool)), this, SLOT(MenuMapImgClick()));
    connect(MenuMax, SIGNAL(triggered(bool)), this, SLOT(MenuMaxClick()));
    connect(MenuMonitor1, SIGNAL(triggered(bool)), this, SLOT(MenuMonitor1Click()));
    connect(MenuMonitor2, SIGNAL(triggered(bool)), this, SLOT(MenuMonitor2Click()));
    connect(MenuOpenElevMask, SIGNAL(triggered(bool)), this, SLOT(MenuOpenElevMaskClick()));
    connect(MenuOpenMapImage, SIGNAL(triggered(bool)), this, SLOT(MenuOpenMapImageClick()));
    connect(MenuOpenShape, SIGNAL(triggered(bool)), this, SLOT(MenuOpenShapeClick()));
    connect(MenuOpenNav, SIGNAL(triggered(bool)), this, SLOT(MenuOpenNavClick()));
    connect(MenuOpenObs, SIGNAL(triggered(bool)), this, SLOT(MenuOpenObsClick()));
    connect(MenuOpenSkyImage, SIGNAL(triggered(bool)), this, SLOT(MenuOpenSkyImageClick()));
    connect(MenuOpenSol1, SIGNAL(triggered(bool)), this, SLOT(MenuOpenSol1Click()));
    connect(MenuOpenSol2, SIGNAL(triggered(bool)), this, SLOT(MenuOpenSol2Click()));
    connect(MenuOptions, SIGNAL(triggered(bool)), this, SLOT(MenuOptionsClick()));
    connect(MenuPlotMapView, SIGNAL(triggered(bool)), this, SLOT(MenuPlotMapViewClick()));
    connect(MenuPort, SIGNAL(triggered(bool)), this, SLOT(MenuPortClick()));
    connect(MenuShapeFile, SIGNAL(triggered(bool)), this, SLOT(MenuShapeFileClick()));
    connect(MenuQuit, SIGNAL(triggered(bool)), this, SLOT(MenuQuitClick()));
    connect(MenuReload, SIGNAL(triggered(bool)), this, SLOT(MenuReloadClick()));
    connect(MenuSaveDop, SIGNAL(triggered(bool)), this, SLOT(MenuSaveDopClick()));
    connect(MenuSaveElMask, SIGNAL(triggered(bool)), this, SLOT(MenuSaveElMaskClick()));
    connect(MenuSaveImage, SIGNAL(triggered(bool)), this, SLOT(MenuSaveImageClick()));
    connect(MenuSaveSnrMp, SIGNAL(triggered(bool)), this, SLOT(MenuSaveSnrMpClick()));
    connect(MenuShowMap, SIGNAL(triggered(bool)), this, SLOT(MenuShowMapClick()));
    connect(MenuShowImg, SIGNAL(triggered(bool)), this, SLOT(MenuShowImgClick()));
    connect(MenuShowGrid, SIGNAL(triggered(bool)), this, SLOT(MenuShowGridClick()));
    connect(MenuShowSkyplot, SIGNAL(triggered(bool)), this, SLOT(MenuShowSkyplotClick()));
    connect(MenuShowTrack, SIGNAL(triggered(bool)), this, SLOT(MenuShowTrackClick()));
    connect(MenuSkyImg, SIGNAL(triggered(bool)), this, SLOT(MenuSkyImgClick()));
    connect(MenuSrcObs, SIGNAL(triggered(bool)), this, SLOT(MenuSrcObsClick()));
    connect(MenuSrcSol, SIGNAL(triggered(bool)), this, SLOT(MenuSrcSolClick()));
    connect(MenuStatusBar, SIGNAL(triggered(bool)), this, SLOT(MenuStatusBarClick()));
    connect(MenuTime, SIGNAL(triggered(bool)), this, SLOT(MenuTimeClick()));
    connect(MenuToolBar, SIGNAL(triggered(bool)), this, SLOT(MenuToolBarClick()));
    connect(MenuVisAna, SIGNAL(triggered(bool)), this, SLOT(MenuVisAnaClick()));
    connect(MenuWaypnt, SIGNAL(triggered(bool)), this, SLOT(MenuWaypointClick()));
    connect(MenuMapLayer, SIGNAL(triggered(bool)), this, SLOT(MenuMapLayerClick()));
    connect(MenuOpenWaypoint, SIGNAL(triggered(bool)), this, SLOT(MenuOpenWaypointClick()));
    connect(MenuSaveWaypoint, SIGNAL(triggered(bool)), this, SLOT(MenuSaveWaypointClick()));
    connect(BtnMessage2, SIGNAL(clicked(bool)), this, SLOT(BtnMessage2Click()));
    connect(RangeList, SIGNAL(clicked(QModelIndex)), this, SLOT(RangeListClick()));
    connect(&Timer, SIGNAL(timeout()), this, SLOT(TimerTimer()));
    connect(PlotTypeS, SIGNAL(currentIndexChanged(int)), this, SLOT(PlotTypeSChange()));
    connect(QFlag, SIGNAL(currentIndexChanged(int)), this, SLOT(QFlagChange()));
    connect(TimeScroll, SIGNAL(valueChanged(int)), this, SLOT(TimeScrollChange()));
    connect(SatList, SIGNAL(currentIndexChanged(int)), this, SLOT(SatListChange()));
    connect(DopType, SIGNAL(currentIndexChanged(int)), this, SLOT(DopTypeChange()));
    connect(ObsType, SIGNAL(currentIndexChanged(int)), this, SLOT(ObsTypeChange()));
    connect(ObsType2, SIGNAL(currentIndexChanged(int)), this, SLOT(ObsTypeChange()));
    connect(FrqType, SIGNAL(currentIndexChanged(int)), this, SLOT(ObsTypeChange()));
    connect(DriveSel, SIGNAL(currentIndexChanged(QString)), this, SLOT(DriveSelChanged()));
    connect(DirSelector, SIGNAL(clicked(QModelIndex)), this, SLOT(DirSelChange(QModelIndex)));
    connect(DirSelector, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(DirSelSelected(QModelIndex)));
    connect(BtnDirSel, SIGNAL(clicked(bool)), this, SLOT(BtnDirSelClick()));
    connect(FileList, SIGNAL(clicked(QModelIndex)), this, SLOT(FileListClick(QModelIndex)));
    connect(Filter, SIGNAL(currentIndexChanged(QString)), this, SLOT(FilterClick()));


    bool state = false;
#ifdef QWEBKIT
    state = true;
#endif
#ifdef QWEBENGINE
    state = true;
#endif
    MenuMapView->setEnabled(state);
    MenuPlotMapView->setEnabled(state);

    Disp->setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);

    LoadOpt();

    updateTime.start();
    qApp->installEventFilter(this);
}
// destructor ---------------------------------------------------------------
Plot::~Plot()
{
    delete [] IndexObs;
    delete [] Az;
    delete [] El;

    for (int i = 0; i < NFREQ + NEXOBS; i++) delete Mp[i];

    delete GraphT;
    delete GraphR;
    delete GraphS;
    for (int i = 0; i < 2; i++)
        delete GraphE[i];
    for (int i = 0; i < 3; i++)
        delete GraphG[i];
    delete RangeList;

    delete DirSelector;
}
// callback on all events ----------------------------------------------------
bool Plot::eventFilter(QObject *obj, QEvent *event)
{
    if ((obj == Disp) && (event->type() == QEvent::MouseMove))
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (childAt(mouseEvent->pos()) == Disp)
            mouseMove(mouseEvent);
    }
    if ((obj == Disp) && (event->type() == QEvent::Resize))
    {
        UpdateSize();
        Refresh();
    }

    return false;
}
// callback on form-show ----------------------------------------------------
void Plot::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) {
        Refresh();
        return;
    }
    QString path1 = "", path2 = "";
    char str_path[256];

    trace(3, "FormShow\n");

    QCommandLineParser parser;
    parser.setApplicationDescription("RTK plot");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption rawOption(QStringList() << "r",
                     QCoreApplication::translate("main", "open raw"));
    parser.addOption(rawOption);

    QCommandLineOption titleOption(QStringList() << "t" << "title",
                       QCoreApplication::translate("main", "use window tile <title>."),
                       QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    QCommandLineOption pathOption(QStringList() << "p" << "path",
                      QCoreApplication::translate("main", "open <path>."),
                      QCoreApplication::translate("main", "path"));
    parser.addOption(pathOption);

    QCommandLineOption path1Option(QStringList() << "1",
                       QCoreApplication::translate("main", "open <path> as solution 1."),
                       QCoreApplication::translate("main", "path"));
    parser.addOption(path1Option);

    QCommandLineOption path2Option(QStringList() << "2",
                       QCoreApplication::translate("main", "open <path> as solution 2."),
                       QCoreApplication::translate("main", "path"));
    parser.addOption(path2Option);

    QCommandLineOption traceOption(QStringList() << "x" << "tracelevel",
                       QCoreApplication::translate("main", "set trace lavel to <tracelavel>."),
                       QCoreApplication::translate("main", "tracelevel"));
    parser.addOption(traceOption);

    parser.addPositionalArgument("file", QCoreApplication::translate("main", "Read files."));

    parser.process(*QApplication::instance());

    if (parser.isSet(rawOption))
        OpenRaw = 1;
    if (parser.isSet(titleOption))
        Title = parser.value(titleOption);
    if (parser.isSet(pathOption))
        path1 = parser.value(pathOption);
    if (parser.isSet(path1Option))
        path1 = parser.value(path1Option);
    if (parser.isSet(path2Option))
        path2 = parser.value(path2Option);
    if (parser.isSet(traceOption))
        Trace = parser.value(traceOption).toInt();

    const QStringList args = parser.positionalArguments();
    QString file;
    foreach(file, args)
    OpenFiles.append(file);

    if (Trace>0) {
        traceopen(TRACEFILE);
        tracelevel(Trace);
    }
    LoadOpt();

    UpdateType(PlotType >= PLOT_OBS ? PLOT_TRK : PlotType);

    UpdateColor();
    UpdateSatMask();
    UpdateOrigin();
    UpdateSize();

    if (path1 != "" || path2 != "") {
        ConnectPath(path1, 0);
        ConnectPath(path2, 1);
        Connect();
    } else if (OpenFiles.count() <= 0) {
        setWindowTitle(Title != "" ? Title : QString(tr("%1 ver. %2 %3")).arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    }
    if (ShapeFile!="") {
        QStringList files;
        char *paths[MAXSHAPEFILE];
        for (int i=0;i<MAXSHAPEFILE;i++) {
            paths[i]=new char [1024];
        }
        int n=expath(qPrintable(ShapeFile),paths,MAXSHAPEFILE);

        for (int i=0;i<n;i++) {
            files.append(paths[i]);
        }
        ReadShapeFile(files);

        for (int i=0;i<MAXSHAPEFILE;i++) {
            delete [] paths[i];
        }
    }
    if (TLEFile != "")
        tle_read(qPrintable(TLEFile), &TLEData);
    if (TLESatFile != "")
        tle_name_read(qPrintable(TLESatFile), &TLEData);

    QFileInfoList drives = QDir::drives();
    if (drives.size() > 1 && drives.at(0).filePath() != "/") {
        Panel1->setVisible(true);
        DriveSel->clear();

        foreach(const QFileInfo &drive, drives) {
            DriveSel->addItem(drive.filePath());
        }
    } else {
        Panel1->setVisible(false); // do not show drive selection on unix
    }
    if (Dir == "") Dir = drives.at(0).filePath();

    DriveSel->setCurrentText(Dir.mid(0, Dir.indexOf(":") + 2));
    dirModel->setRootPath(Dir);
    DirSelector->setVisible(false);
    DirSelected->setText(Dir);
    fileModel->setRootPath(Dir);
    FileList->setRootIndex(fileModel->index(Dir));
    FilterClick();

    if (MenuBrowse->isChecked()) {
        PanelBrowse->setVisible(true);
    }
    else {
        PanelBrowse->setVisible(false);
    }

    strinit(&StrTimeSync);
    NStrBuff=0;

    if (TimeSyncOut) {
        sprintf(str_path,":%d",TimeSyncPort);
        stropen(&StrTimeSync,STR_TCPSVR,STR_MODE_RW,str_path);
    }

    Timer.start(RefCycle);
    UpdatePlot();
    UpdateEnable();

    if (OpenFiles.count() > 0) {
        if (CheckObs(OpenFiles.at(0)) || OpenRaw) ReadObs(OpenFiles);
        else ReadSol(OpenFiles, 0);
    }
}
// callback on form-close ---------------------------------------------------
void Plot::closeEvent(QCloseEvent *)
{
    trace(3, "FormClose\n");

    RangeList->setVisible(false);

    SaveOpt();

    if (Trace > 0) traceclose();
}

// callback for painting   --------------------------------------------------
void Plot::paintEvent(QPaintEvent *)
{
    UpdateDisp();
}
// callback on form-resize --------------------------------------------------
void Plot::resizeEvent(QResizeEvent *)
{
    trace(3, "FormResize\n");

    // suppress repeated resize callback
    if (FormWidth == width() && FormHeight == height()) return;

    UpdateSize();
    Refresh();

    FormWidth = width();
    FormHeight = height();
}
// callback on drag-and-drop files ------------------------------------------
void Plot::dragEnterEvent(QDragEnterEvent *event)
{
    trace(3, "dragEnterEvent\n");

    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

// callback on drag-and-drop files ------------------------------------------
void Plot::dropEvent(QDropEvent *event)
{
    QStringList files;
    int n;

    trace(3, "dropEvent\n");

    if (ConnectState || !event->mimeData()->hasUrls())
        return;
    foreach(QUrl url, event->mimeData()->urls()) {
        files.append(QDir::toNativeSeparators(url.toString()));
    }

    QString file=files.at(0);

    if (files.size() == 1 && (n = files.at(0).lastIndexOf('.')) != -1) {
        QString ext = files.at(0).mid(n).toLower();
        if ((ext == "jpg") || (ext == "jpeg")) {
            if (PlotType == PLOT_TRK)
                ReadMapData(file);
            else if (PlotType == PLOT_SKY || PlotType == PLOT_MPS)
                ReadSkyData(file);
        }
        ;
    } else if (CheckObs(files.at(0))) {
        ReadObs(files);
    } else if (!BtnSol1->isChecked() && BtnSol2->isChecked()) {
        ReadSol(files, 1);
    } else {
        ReadSol(files, 0);
    }
}
// callback on menu-open-solution-1 -----------------------------------------
void Plot::MenuOpenSol1Click()
{
    trace(3, "MenuOpenSol1Click\n");

    ReadSol(QStringList(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open Solution 1"), QString(), tr("Solution File (*.pos *.stat *.nmea *.txt *.ubx);;All (*.*)")))), 0);
}
// callback on menu-open-solution-2 -----------------------------------------
void Plot::MenuOpenSol2Click()
{
    trace(3, "MenuOpenSol2Click\n");

    ReadSol(QStringList(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open Solution 2"), QString(), tr("Solution File (*.pos *.stat *.nmea *.txt *.ubx);;All (*.*)")))), 1);
}
// callback on menu-open-map-image ------------------------------------------
void Plot::MenuOpenMapImageClick()
{
    trace(3, "MenuOpenMapImage\n");

    ReadMapData(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open Map Image"), MapImageFile, tr("JPEG File (*.jpg *.jpeg);;All (*.*)"))));
}
// callback on menu-open-track-points ---------------------------------------
void Plot::MenuOpenShapeClick()
{
    trace(3, "MenuOpenShapePath\n");

    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open Shape File"), QString(), tr("Shape File (*.shp);;All (*.*)"));
    for (int i = 0; i < files.size(); i++)
        files[i] = QDir::toNativeSeparators(files.at(i));

    ReadShapeFile(files);
}
// callback on menu-open-sky-image ------------------------------------------
void Plot::MenuOpenSkyImageClick()
{
    trace(3, "MenuOpenSkyImage\n");

    ReadSkyData(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open Sky Image"), SkyImageFile, tr("JPEG File (*.jpg *.jpeg);;All (*.*)"))));
}
// callback on menu-oepn-waypoint -------------------------------------------
void Plot::MenuOpenWaypointClick()
{
    trace(3, "MenuOpenWaypointClick\n");

    ReadWaypoint(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Open Waypoint"), SkyImageFile, tr("Waypoint File (*.gpx, *.pos);;All (*.*)"))));
}
// callback on menu-open-obs-data -------------------------------------------
void Plot::MenuOpenObsClick()
{
    trace(3, "MenuOpenObsClick\n");
    ReadObs(QFileDialog::getOpenFileNames(this, tr("Open Obs/Nav Data"), QString(), tr("RINEX OBS (*.obs *.*o *.*d *.O.rnx *.*o.gz *.*o.Z *.d.gz *.d.Z);;All (*.*)")));
}
// callback on menu-open-nav-data -------------------------------------------
void Plot::MenuOpenNavClick()
{
    trace(3, "MenuOpenNavClick\n");

    ReadNav(QFileDialog::getOpenFileNames(this, tr("Open Raw Obs/Nav Messages"), QString(), tr("RINEX NAV (*.nav *.gnav *.hnav *.qnav *.*n *.*g *.*h *.*q *.*p *N.rnx);;All (*.*)")));
}
// callback on menu-open-elev-mask ------------------------------------------
void Plot::MenuOpenElevMaskClick()
{
    trace(3, "MenuOpenElevMaskClick\n");

    ReadElMaskData(QDir::toNativeSeparators(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Opene Elevation Mask"), QString(), tr("Text File (*.txt);;All (*.*)")))));
}
// callback on menu-vis-analysis --------------------------------------------
void Plot::MenuVisAnaClick()
{
    if (RcvPos != 1) { // lat/lon/height
        ShowMsg(tr("specify Receiver Position as Lat/Lon/Hgt"));
        return;
    }
    if (spanDialog->TimeStart.time == 0) {
        int week;
        double tow = time2gpst(utc2gpst(timeget()), &week);
        spanDialog->TimeStart = gpst2time(week, floor(tow / 3600.0) * 3600.0);
        spanDialog->TimeEnd = timeadd(spanDialog->TimeStart, 86400.0);
        spanDialog->TimeInt = 30.0;
    }
    spanDialog->TimeEna[0] = spanDialog->TimeEna[1] = spanDialog->TimeEna[2] = 1;
    spanDialog->TimeVal[0] = spanDialog->TimeVal[1] = spanDialog->TimeVal[2] = 2;

    spanDialog->exec();

    if (spanDialog->result() == QDialog::Accepted) {
        TimeStart = spanDialog->TimeStart;
        TimeEnd = spanDialog->TimeEnd;
        TimeInt = spanDialog->TimeInt;
        GenVisData();
    }
    spanDialog->TimeVal[0] = spanDialog->TimeVal[1] = spanDialog->TimeVal[2] = 1;
}
// callback on menu-save image ----------------------------------------------
void Plot::MenuSaveImageClick()
{
    Buff.save(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Image"), QString(), tr("JPEG  (*.jpg);;Windows Bitmap (*.bmp)"))));
}
// callback on menu-save-waypoint -------------------------------------------
void Plot::MenuSaveWaypointClick()
{
    trace(3, "MenuSaveWaypointClick\n");

    SaveWaypoint(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Waypoint"), QString(), tr("GPX File (*.gpx, *.pos);;All (*.*)"))));
}
// callback on menu-save-# of sats/dop --------------------------------------
void Plot::MenuSaveDopClick()
{
    trace(3, "MenuSaveDopClick\n");

    SaveDop(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Data"), QString(), tr("All (*.*);;Text File (*.txt)"))));
}
// callback on menu-save-snr,azel -------------------------------------------
void Plot::MenuSaveSnrMpClick()
{
    trace(3, "MenuSaveSnrMpClick\n");

    SaveSnrMp(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Data"), QString(), tr("All (*.*);;Text File (*.txt)"))));
}
// callback on menu-save-elmask ---------------------------------------------
void Plot::MenuSaveElMaskClick()
{
    trace(3, "MenuSaveElMaskClick\n");

    SaveElMask(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Data"), QString(), tr("All (*.*);;Text File (*.txt)"))));
}
// callback on menu-connect -------------------------------------------------
void Plot::MenuConnectClick()
{
    trace(3, "MenuConnectClick\n");

    Connect();
}
// callback on menu-disconnect ----------------------------------------------
void Plot::MenuDisconnectClick()
{
    trace(3, "MenuDisconnectClick\n");

    Disconnect();
}
// callback on menu-connection-settings -------------------------------------
void Plot::MenuPortClick()
{
    int i;

    trace(3, "MenuPortClick\n");

    connectDialog->Stream1 = RtStream[0];
    connectDialog->Stream2 = RtStream[1];
    connectDialog->Format1 = RtFormat[0];
    connectDialog->Format2 = RtFormat[1];
    connectDialog->TimeForm = RtTimeForm;
    connectDialog->DegForm = RtDegForm;
    connectDialog->FieldSep = RtFieldSep;
    connectDialog->TimeOutTime = RtTimeOutTime;
    connectDialog->ReConnTime = RtReConnTime;
    for (i = 0; i < 3; i++) {
        connectDialog->Paths1[i] = StrPaths[0][i];
        connectDialog->Paths2[i] = StrPaths[1][i];
    }
    for (i = 0; i < 2; i++) {
        connectDialog->Cmds1  [i] = StrCmds[0][i];
        connectDialog->Cmds2  [i] = StrCmds[1][i];
        connectDialog->CmdEna1[i] = StrCmdEna[0][i];
        connectDialog->CmdEna2[i] = StrCmdEna[1][i];
    }
    for (i = 0; i < 10; i++) connectDialog->TcpHistory [i] = StrHistory [i];

    connectDialog->exec();

    if (connectDialog->result() != QDialog::Accepted) return;

    RtStream[0] = connectDialog->Stream1;
    RtStream[1] = connectDialog->Stream2;
    RtFormat[0] = connectDialog->Format1;
    RtFormat[1] = connectDialog->Format2;
    RtTimeForm = connectDialog->TimeForm;
    RtDegForm = connectDialog->DegForm;
    RtFieldSep = connectDialog->FieldSep;
    RtTimeOutTime = connectDialog->TimeOutTime;
    RtReConnTime = connectDialog->ReConnTime;
    for (i = 0; i < 3; i++) {
        StrPaths[0][i] = connectDialog->Paths1[i];
        StrPaths[1][i] = connectDialog->Paths2[i];
    }
    for (i = 0; i < 2; i++) {
        StrCmds  [0][i] = connectDialog->Cmds1  [i];
        StrCmds  [1][i] = connectDialog->Cmds2  [i];
        StrCmdEna[0][i] = connectDialog->CmdEna1[i];
        StrCmdEna[1][i] = connectDialog->CmdEna2[i];
    }
    for (i = 0; i < 10; i++) StrHistory [i] = connectDialog->TcpHistory [i];
}
// callback on menu-reload --------------------------------------------------
void Plot::MenuReloadClick()
{
    trace(3, "MenuReloadClick\n");

    Reload();
}
// callback on menu-clear ---------------------------------------------------
void Plot::MenuClearClick()
{
    trace(3, "MenuClearClick\n");

    Clear();
}
// callback on menu-exit-----------------------------------------------------
void Plot::MenuQuitClick()
{
    trace(3, "MenuQuitClick\n");

    close();
}
// callback on menu-time-span/interval --------------------------------------
void Plot::MenuTimeClick()
{
    sol_t *sols, *sole;
    int i;

    trace(3, "MenuTimeClick\n");

    if (!TimeEna[0]) {
        if (Obs.n > 0) {
            TimeStart = Obs.data[0].time;
        } else if (BtnSol2->isChecked() && SolData[1].n > 0) {
            sols = getsol(SolData + 1, 0);
            TimeStart = sols->time;
        } else if (SolData[0].n > 0) {
            sols = getsol(SolData, 0);
            TimeStart = sols->time;
        }
    }
    if (!TimeEna[1]) {
        if (Obs.n > 0) {
            TimeEnd = Obs.data[Obs.n - 1].time;
        } else if (BtnSol2->isChecked() && SolData[1].n > 0) {
            sole = getsol(SolData + 1, SolData[1].n - 1);
            TimeEnd = sole->time;
        } else if (SolData[0].n > 0) {
            sole = getsol(SolData, SolData[0].n - 1);
            TimeEnd = sole->time;
        }
    }
    for (i = 0; i < 3; i++)
        spanDialog->TimeEna[i] = TimeEna[i];

    spanDialog->TimeStart = TimeStart;
    spanDialog->TimeEnd = TimeEnd;
    spanDialog->TimeInt = TimeInt;
    spanDialog->TimeVal[0] = !ConnectState;
    spanDialog->TimeVal[1] = !ConnectState;

    spanDialog->exec();

    if (spanDialog->result() != QDialog::Accepted) return;

    if (TimeEna[0] != spanDialog->TimeEna[0] ||
        TimeEna[1] != spanDialog->TimeEna[1] ||
        TimeEna[2] != spanDialog->TimeEna[2] ||
        timediff(TimeStart, spanDialog->TimeStart) != 0.0 ||
        timediff(TimeEnd, spanDialog->TimeEnd) != 0.0 ||
        !qFuzzyCompare(TimeInt, spanDialog->TimeInt)) {
        for (i = 0; i < 3; i++) TimeEna[i] = spanDialog->TimeEna[i];

        TimeStart = spanDialog->TimeStart;
        TimeEnd = spanDialog->TimeEnd;
        TimeInt = spanDialog->TimeInt;

        Reload();
    }
}
// callback on menu-map-image -----------------------------------------------
void Plot::MenuMapImgClick()
{
    trace(3, "MenuMapImgClick\n");

    mapOptDialog->show();
}
// callback on menu-sky image -----------------------------------------------
void Plot::MenuSkyImgClick()
{
    trace(3, "MenuSkyImgClick\n");

    skyImgDialog->show();
}
// callback on menu-vec map -------------------------------------------------
void Plot::MenuMapLayerClick()
{
    trace(3, "MenuMapLayerClick\n");

    vecMapDialog = new VecMapDialog(this);

    vecMapDialog->exec();

    delete vecMapDialog;

}
// callback on menu-solution-source -----------------------------------------
void Plot::MenuSrcSolClick()
{
    int sel = !BtnSol1->isChecked() && BtnSol2->isChecked();

    trace(3, "MenuSrcSolClick\n");

    if (SolFiles[sel].count() <= 0) return;

    viewer->setWindowTitle(SolFiles[sel].at(0));
    viewer->option = 0;
    viewer->show();
    viewer->read(SolFiles[sel].at(0));
}
// callback on menu-obs-data-source -----------------------------------------
void Plot::MenuSrcObsClick()
{
    TextViewer *viewer;
    char file[1024], tmpfile[1024];
    int cstat;

    trace(3, "MenuSrcObsClick\n");

    if (ObsFiles.count() <= 0) return;

    strcpy(file, qPrintable(ObsFiles.at(0)));
    cstat = rtk_uncompress(file, tmpfile);

    viewer = new TextViewer(this);
    viewer->setWindowTitle(ObsFiles.at(0));
    viewer->option = 0;
    viewer->show();
    viewer->read(!cstat ? file : tmpfile);
    if (cstat) remove(tmpfile);
}
// callback on menu-copy-to-clipboard ---------------------------------------
void Plot::MenuCopyClick()
{
    trace(3, "MenuCopyClick\n");

    QClipboard *clipboard = QApplication::clipboard();

    clipboard->setPixmap(Buff);
}
// callback on menu-options -------------------------------------------------
void Plot::MenuOptionsClick()
{
    QString tlefile = TLEFile, tlesatfile = TLESatFile;
    double oopos[3];
    char str_path[256];
    int timesyncout=TimeSyncOut, rcvpos=RcvPos;

    trace(3, "MenuOptionsClick\n");

    matcpy(oopos,OOPos,3,1);

    plotOptDialog->move(pos().x() + width() / 2 - plotOptDialog->width() / 2,
                pos().y() + height() / 2 - plotOptDialog->height() / 2);
    plotOptDialog->plot = this;

    plotOptDialog->exec();

    if (plotOptDialog->result() != QDialog::Accepted) return;

    SaveOpt();

    for (int i = 0; i < 3; i++) oopos[i] -= OOPos[i];

    if (TLEFile != tlefile) {
        free(TLEData.data);
        TLEData.data = NULL;
        TLEData.n = TLEData.nmax = 0;
        tle_read(qPrintable(TLEFile), &TLEData);
    }
    if (TLEFile != tlefile || TLESatFile != tlesatfile)
        tle_name_read(qPrintable(TLESatFile), &TLEData);
    if (rcvpos != RcvPos || norm(oopos, 3) > 1E-3 || TLEFile != tlefile) {
        if (SimObs) GenVisData(); else UpdateObs(NObs);
    }
    UpdateColor();
    UpdateSize();
    UpdateOrigin();
    UpdateInfo();
    UpdateSatMask();
    UpdateSatList();

    Refresh();
    Timer.start(RefCycle);

    for (int i = 0; i < RangeList->count(); i++) {
        QString str=RangeList->item(i)->text();
        double range;
        QString unit;

        QStringList tokens = str.split(' ');
        if (tokens.length()==2) {
            bool ok;
            range = tokens.at(0).toInt(&ok);
            unit = tokens.at(1);
            if (ok) {
                if (unit == "cm") range*=0.01;
                else if (unit == "km") range*=1000.0;
                if (range==YRange) {
                    RangeList->item(i)->setSelected(true);
                    break;
                }

            }
        }
    }
    if (!timesyncout && TimeSyncOut) {
        sprintf(str_path,":%d",TimeSyncPort);
        stropen(&StrTimeSync,STR_TCPSVR,STR_MODE_RW,str_path);
    }
    else if (timesyncout && !TimeSyncOut) {
        strclose(&StrTimeSync);
    }
}
// callback on menu-show-tool-bar -------------------------------------------
void Plot::MenuToolBarClick()
{
    trace(3, "MenuToolBarClick\n");

    toolBar->setVisible(MenuToolBar->isChecked());

    UpdateSize();
    Refresh();
}
// callback on menu-show-status-bar -----------------------------------------
void Plot::MenuStatusBarClick()
{
    trace(3, "MenuStatusBarClick\n");

    statusbar->setVisible(MenuStatusBar->isChecked());
    UpdateSize();
    Refresh();
}
// callback on menu-show-browse-panel ---------------------------------------
void Plot::MenuBrowseClick()
{
    trace(3,"MenuBrowseClick\n");

    if (MenuBrowse->isChecked()) {
        PanelBrowse->setVisible(true);
    }
    else {
        PanelBrowse->setVisible(false);
    }

    Disp->updateGeometry();

    UpdateSize();
    Refresh();
}
// callback on menu-waypoints -----------------------------------------------
void Plot::MenuWaypointClick()
{
    trace(3, "MenuWaypointClick\n");

    pntDialog->show();
}
// callback on menu-input-monitor-1 -----------------------------------------
void Plot::MenuMonitor1Click()
{
    trace(3, "MenuMonitor1Click\n");

    Console1->setWindowTitle(tr("Monitor RT Input 1"));
    Console1->show();
}
// callback on menu-input-monitor-2 -----------------------------------------
void Plot::MenuMonitor2Click()
{
    trace(3, "MenuMonitor2Click\n");

    Console2->setWindowTitle(tr("Monitor RT Input 2"));
    Console2->show();
}
// callback on menu-map-view ---------------------------------------
void Plot::MenuMapViewClick()
{
    trace(3, "MenuGEClick\n");

    mapView->setWindowTitle(
        QString(tr("%1 ver.%2 %3: Google Earth View")).arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    mapView->show();
}
// callback on menu-center-origin -------------------------------------------
void Plot::MenuCenterOriClick()
{
    trace(3, "MenuCenterOriClick\n");

    SetRange(0, YRange);
    Refresh();
}
// callback on menu-fit-horizontal ------------------------------------------
void Plot::MenuFitHorizClick()
{
    trace(3, "MenuFitHorizClick\n");

    if (PlotType == PLOT_TRK) FitRange(0); else FitTime();
    Refresh();
}
// callback on menu-fit-vertical --------------------------------------------
void Plot::MenuFitVertClick()
{
    trace(3, "MenuFitVertClick\n");

    FitRange(0);
    Refresh();
}
// callback on menu-show-skyplot --------------------------------------------
void Plot::MenuShowSkyplotClick()
{
    trace(3, "MenuShowSkyplotClick\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on menu-show-map-image ------------------------------------------
void Plot::MenuShowImgClick()
{
    trace(3, "MenuShowImgClick\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on menu-show-grid -----------------------------------------------
void Plot::MenuShowGridClick()
{
    trace(3,"MenuShowGrid\n");

    UpdatePlot();
    UpdateEnable();
    Refresh();
}
// callback on menu-show-track-points ---------------------------------------
void Plot::MenuShowTrackClick()
{
    trace(3, "MenuShowTrackClick\n");

    if (!MenuShowTrack->isChecked()) {
        MenuFixHoriz->setChecked(false);
        MenuFixVert->setChecked(false);
    }
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-fix-center ----------------------------------------------
void Plot::MenuFixCentClick()
{
    trace(3, "MenuFixCentClick\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on menu-fix-horizontal ------------------------------------------
void Plot::MenuFixHorizClick()
{
    trace(3, "MenuFixHorizClick\n");

    Xcent = 0.0;
    UpdatePlot();
    UpdateEnable();
}
// callback on menu-fix-vertical --------------------------------------------
void Plot::MenuFixVertClick()
{
    trace(3, "MenuFixVertClick\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on menu-show-map -------------------------------------------------
void Plot::MenuShowMapClick()
{
    trace(3, "MenuShowMapClick\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on menu-windows-maximize ----------------------------------------
void Plot::MenuMaxClick()
{
    QScreen *scr = QApplication::screens().at(0);
    QRect rect = scr->availableGeometry();
    QSize thisDecoration = this->frameSize() - this->size();

    this->move(rect.x(), rect.y());
    this->resize(rect.width() - thisDecoration.width(), rect.height() - thisDecoration.height());

    mapView->hide();
}
// callback on menu-windows-mapview -----------------------------------------
void Plot::MenuPlotMapViewClick()
{
    QScreen *scr = QApplication::screens().at(0);
    QRect rect = scr->availableGeometry();
    QSize thisDecoration = this->frameSize() - this->size();

    this->move(rect.x(), rect.y());
    this->resize(rect.width() / 2 - thisDecoration.width(), rect.height() - thisDecoration.height());

    QSize GEDecoration = mapView->frameSize() - mapView->size();
    mapView->resize(rect.width()/2 - GEDecoration.width(), rect.height() - GEDecoration.height());
    mapView->move(rect.x() + rect.width() / 2, rect.y());

    mapView->setVisible(true);
}
//---------------------------------------------------------------------------
#if 0
void Plot::DispGesture()
{
    AnsiString s;
    int b, e;

    b = EventInfo.Flags.Contains(gfBegin);
    e = EventInfo.Flags.Contains(gfEnd);

    if (EventInfo.GestureID == Controls::igiZoom) {
        s.sprintf("zoom: Location=%d,%d,Flag=%d,%d,Angle=%.1f,Disnance=%d",
              EventInfo.Location.X, EventInfo.Location.Y, b, e,
              EventInfo.Angle, EventInfo.Distance);
        Message1->Caption = s;
    } else if (EventInfo.GestureID == Controls::igiPan) {
        s.sprintf("pan: Location=%d,%d,Flag=%d,%d,Angle=%.1f,Disnance=%d",
              EventInfo.Location.X, EventInfo.Location.Y, b, e,
              EventInfo.Angle, EventInfo.Distance);
        Message1->Caption = s;
    } else if (EventInfo.GestureID == Controls::igiRotate) {
        s.sprintf("rotate: Location=%d,%d,Flag=%d,%d,Angle=%.1f,Disnance=%d",
              EventInfo.Location.X, EventInfo.Location.Y, b, e,
              EventInfo.Angle, EventInfo.Distance);
        Message1->Caption = s;
    }
}
#endif
// callback on menu-animation-start -----------------------------------------
void Plot::MenuAnimStartClick()
{
    trace(3, "MenuAnimStartClick\n");
}
// callback on menu-animation-stop ------------------------------------------
void Plot::MenuAnimStopClick()
{
    trace(3, "MenuAnimStopClick\n");
}
// callback on menu-about ---------------------------------------------------
void Plot::MenuAboutClick()
{
    trace(3, "MenuAboutClick\n");

    aboutDialog->aboutString = PRGNAME;
    aboutDialog->iconIndex = 2;
    aboutDialog->exec();
}
// callback on button-connect/disconnect ------------------------------------
void Plot::BtnConnectClick()
{
    trace(3, "BtnConnectClick\n");

    if (!ConnectState) MenuConnectClick();
    else MenuDisconnectClick();
}
// callback on button-solution-1 --------------------------------------------
void Plot::BtnSol1Click()
{
    trace(3, "BtnSol1Click\n");

    BtnSol12->setChecked(false);
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// callback on button-solution-2 --------------------------------------------
void Plot::BtnSol2Click()
{
    trace(3, "BtnSol2Click\n");

    BtnSol12->setChecked(false);
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// callback on button-solution-1-2 ------------------------------------------
void Plot::BtnSol12Click()
{
    trace(3, "BtnSol12Click\n");

    BtnSol1->setChecked(false);
    BtnSol2->setChecked(false);
    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// callback on button-solution-1 double-click -------------------------------
void Plot::BtnSol1DblClick()
{
    trace(3, "BtnSol1DblClick\n");

    MenuOpenSol1Click();
}
// callback on button-solution-2 double-click -------------------------------
void Plot::BtnSol2DblClick()
{
    trace(3, "BtnSol2DblClick\n");

    MenuOpenSol2Click();
}

// callback on button-plot-1-onoff ------------------------------------------
void Plot::BtnOn1Click()
{
    trace(3, "BtnOn1Click\n");

    UpdateSize();
    Refresh();
}
// callback on button-plot-2-onoff-------------------------------------------
void Plot::BtnOn2Click()
{
    trace(3, "BtnOn2Click\n");

    UpdateSize();
    Refresh();
}
// callback on button-plot-3-onoff ------------------------------------------
void Plot::BtnOn3Click()
{
    trace(3, "BtnOn3Click\n");

    UpdateSize();
    Refresh();
}
// callback on button-range-list --------------------------------------------
void Plot::BtnRangeListClick()
{
    QToolButton *btn=(QToolButton *)sender();
    trace(3, "BtnRangeListClick\n");

    QPoint pos = btn->mapToGlobal(btn->pos());
    pos.rx() -= btn->width();
    pos.ry() += btn->height();

    RangeList->move(pos);
    RangeList->setVisible(!RangeList->isVisible());
}
// callback on button-range-list --------------------------------------------
void Plot::RangeListClick()
{
    bool okay;
    QString unit;
    QListWidgetItem *i;

    trace(3, "RangeListClick\n");

    RangeList->setVisible(false);
    if ((i = RangeList->currentItem()) == NULL) return;

    QStringList tokens = i->text().split(" ");

    if (tokens.length()!=2) return;

    YRange = tokens.at(0).toDouble(&okay);
    if (!okay) return;

    unit = tokens.at(1);

    if (unit == "cm") YRange*=0.01;
    else if (unit == "km") YRange*=1000.0;
    SetRange(0, YRange);
    UpdatePlot();
    UpdateEnable();
}

// callback on button-animation ---------------------------------------------
void Plot::BtnAnimateClick()
{
    trace(3, "BtnAnimateClick\n");

    UpdateEnable();
}
// callback on button-message 2 ---------------------------------------------
void Plot::BtnMessage2Click()
{
    if (++PointType > 2) PointType = 0;
}
// callback on plot-type selection change -----------------------------------
void Plot::PlotTypeSChange()
{
    int i;

    trace(3, "PlotTypeSChnage\n");

    for (i = 0; !PTypes[i].isNull(); i++)
        if (PlotTypeS->currentText() == PTypes[i]) UpdateType(i);

    UpdateTime();
    UpdatePlot();
    UpdateEnable();
}
// callback on quality-flag selection change --------------------------------
void Plot::QFlagChange()
{
    trace(3, "QFlagChange\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on obs-type selection change ------------------------------------
void Plot::ObsTypeChange()
{
    trace(3, "ObsTypeChange\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on dop-type selection change ------------------------------------
void Plot::DopTypeChange()
{
    trace(3, "DopTypeChange\n");

    UpdatePlot();
    UpdateEnable();
}
// callback on satellite-list selection change ------------------------------
void Plot::SatListChange()
{
    trace(3, "SatListChange\n");

    UpdateSatSel();
    UpdatePlot();
    UpdateEnable();
}
// callback on time scroll-bar change ---------------------------------------
void Plot::TimeScrollChange()
{
    int sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;

    trace(3, "TimeScrollChange\n");

    if (PlotType <= PLOT_NSAT || PlotType == PLOT_RES)
        SolIndex[sel] = TimeScroll->value();
    else
        ObsIndex = TimeScroll->value();
    UpdatePlot();
}
// callback on mouse-down event ---------------------------------------------
void Plot::mousePressEvent(QMouseEvent *event)
{
    X0 = Disp->mapFromGlobal(event->globalPosition()).x();
    Y0 = Disp->mapFromGlobal(event->globalPosition()).y();
    Xcent0 = Xcent;

    trace(3, "DispMouseDown: X=%d Y=%d\n", X0, Y0);

    Drag = event->buttons().testFlag(Qt::LeftButton) ? 1 : (event->buttons().testFlag(Qt::RightButton) ? 11 : 0);

    if (PlotType == PLOT_TRK)
        MouseDownTrk(X0, Y0);
    else if (PlotType <= PLOT_NSAT || PlotType == PLOT_RES || PlotType == PLOT_SNR)
        MouseDownSol(X0, Y0);
    else if (PlotType == PLOT_OBS || PlotType == PLOT_DOP)
        MouseDownObs(X0, Y0);
    else Drag = 0;

    RangeList->setVisible(false);
}
// callback on mouse-move event ---------------------------------------------
void Plot::mouseMove(QMouseEvent *event)
{
    double dx, dy, dxs, dys;

    if ((abs(Disp->mapFromGlobal(event->globalPosition()).x() - Xn) < 1) &&
        (abs(Disp->mapFromGlobal(event->globalPosition()).y() - Yn) < 1)) return;

    Xn = Disp->mapFromGlobal(event->globalPosition()).x();
    Yn = Disp->mapFromGlobal(event->globalPosition()).y();

    trace(4, "DispMouseMove: X=%d Y=%d\n", Xn, Yn);

    if (Drag == 0) {
        UpdatePoint(Xn, Yn);
        return;
    }

    dx = (X0 - Xn) * Xs;
    dy = (Yn - Y0) * Ys;
    dxs = pow(2.0, (X0 - Xn) / 100.0);
    dys = pow(2.0, (Yn - Y0) / 100.0);

    if (PlotType == PLOT_TRK)
        MouseMoveTrk(Xn, Yn, dx, dy, dxs, dys);
    else if (PlotType <= PLOT_NSAT || PlotType == PLOT_RES || PlotType == PLOT_SNR)
        MouseMoveSol(Xn, Yn, dx, dy, dxs, dys);
    else if (PlotType == PLOT_OBS || PlotType == PLOT_DOP)
        MouseMoveObs(Xn, Yn, dx, dy, dxs, dys);
}
// callback on mouse-up event -----------------------------------------------
void Plot::mouseReleaseEvent(QMouseEvent *event)
{
    trace(3, "DispMouseUp: X=%d Y=%d\n", Disp->mapFromGlobal(event->globalPosition()).x(), Disp->mapFromGlobal(event->globalPosition()).y());

    Drag = 0;
    setCursor(Qt::ArrowCursor);
    Refresh();
    Refresh_MapView();
}
// callback on mouse-double-click -------------------------------------------
void Plot::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint p(static_cast<int>(X0), static_cast<int>(Y0));
    double x, y;

    if (event->button() != Qt::LeftButton) return;

    trace(3, "DispDblClick X=%d Y=%d\n", p.x(), p.y());

    if (BtnFixHoriz->isChecked()) return;

    if (PlotType == PLOT_TRK) {
        GraphT->ToPos(p, x, y);
        GraphT->SetCent(x, y);
        Refresh();
        Refresh_MapView();
    } else if (PlotType <= PLOT_NSAT || PlotType == PLOT_RES || PlotType == PLOT_SNR) {
        GraphG[0]->ToPos(p, x, y);
        SetCentX(x);
        Refresh();
    } else if (PlotType == PLOT_OBS || PlotType == PLOT_DOP) {
        GraphR->ToPos(p, x, y);
        SetCentX(x);
        Refresh();
    }
}
// callback on mouse-leave event --------------------------------------------
void Plot::leaveEvent(QEvent *)
{
    trace(3, "DispMouseLeave\n");

    Xn = Yn = -1;
    Message2->setVisible(false);
    Message2->setText("");
}
// callback on mouse-down event on track-plot -------------------------------
void Plot::MouseDownTrk(int X, int Y)
{
    int i, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;

    trace(3, "MouseDownTrk: X=%d Y=%d\n", X, Y);

    if (Drag == 1 && (i = SearchPos(X, Y)) >= 0) {
        SolIndex[sel] = i;
        UpdateTime();
        UpdateInfo();
        Drag = 0;
        Refresh();
    } else {
        GraphT->GetCent(Xc, Yc);
        GraphT->GetScale(Xs, Ys);
        setCursor(Drag == 1 ? Qt::SizeAllCursor : Qt::SplitVCursor);
    }
}
// callback on mouse-down event on solution-plot ----------------------------
void Plot::MouseDownSol(int X, int Y)
{
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    QPoint pnt, p(X, Y);
    gtime_t time = { 0, 0 };
    sol_t *data;
    double x, xl[2], yl[2];
    int i, area = -1, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;

    trace(3, "MouseDownSol: X=%d Y=%d\n", X, Y);

    if (PlotType == PLOT_SNR) {
        if (0 <= ObsIndex && ObsIndex < NObs) time = Obs.data[IndexObs[ObsIndex]].time;
    } else {
        if ((data = getsol(SolData + sel, SolIndex[sel]))) time = data->time;
    }
    if (time.time && !MenuFixHoriz->isChecked()) {
        x = TimePos(time);

        GraphG[0]->GetLim(xl, yl);
        GraphG[0]->ToPoint(x, yl[1], pnt);

        if ((X - pnt.x()) * (X - pnt.x()) + (Y - pnt.y()) * (Y - pnt.y()) < 25) {
            setCursor(Qt::SizeHorCursor);
            Drag = 20;
            Refresh();
            return;
        }
    }
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked() || (i != 1 && PlotType == PLOT_SNR)) continue;

        GraphG[i]->GetCent(Xc, Yc);
        GraphG[i]->GetScale(Xs, Ys);
        area = GraphG[i]->OnAxis(Disp->mapFromGlobal(p));

        if (Drag == 1 && area == 0) {
            setCursor(Qt::SizeAllCursor);
            Drag += i;
            return;
        } else if (area == 1) {
            setCursor(Drag == 1 ? Qt::SizeVerCursor : Qt::SplitVCursor);
            Drag += i + 4;
            return;
        } else if (area == 0) {
            break;
        }
    }
    if (area == 0 || area == 8) {
        setCursor(Drag == 1 ? Qt::SizeHorCursor : Qt::SplitHCursor);
        Drag += 3;
    } else {
        Drag = 0;
    }
}
// callback on mouse-down event on observation-data-plot --------------------
void Plot::MouseDownObs(int X, int Y)
{
    QPoint pnt, p(X, Y);
    double x, xl[2], yl[2];
    int area;

    trace(3, "MouseDownObs: X=%d Y=%d\n", X, Y);

    if (0 <= ObsIndex && ObsIndex < NObs && !MenuFixHoriz->isChecked()) {
        x = TimePos(Obs.data[IndexObs[ObsIndex]].time);

        GraphR->GetLim(xl, yl);
        GraphR->ToPoint(x, yl[1], pnt);

        if ((X - pnt.x()) * (X - pnt.x()) + (Y - pnt.y()) * (Y - pnt.y()) < 25) {
            setCursor(Qt::SizeHorCursor);
            Drag = 20;
            Refresh();
            return;
        }
    }
    GraphR->GetCent(Xc, Yc);
    GraphR->GetScale(Xs, Ys);
    area = GraphR->OnAxis(Disp->mapFromGlobal(p));

    if (area == 0 || area == 8) {
        setCursor(Drag == 1 ? Qt::SizeHorCursor : Qt::SplitHCursor);
        Drag += 3;
    } else {
        Drag = 0;
    }
}
// callback on mouse-move event on track-plot -------------------------------
void Plot::MouseMoveTrk(int X, int Y, double dx, double dy,
            double dxs, double dys)
{
    trace(4, "MouseMoveTrk: X=%d Y=%d\n", X, Y);

    Q_UNUSED(dxs);

    if (Drag == 1 && !MenuFixHoriz->isChecked())
        GraphT->SetCent(Xc + dx, Yc + dy);
    else if (Drag > 1)
        GraphT->SetScale(Xs * dys, Ys * dys);
    MenuCenterOri->setChecked(false);

    if (updateTime.elapsed() < RefreshTime) return;
    updateTime.restart();

    Refresh();
}
// callback on mouse-move event on solution-plot ----------------------------
void Plot::MouseMoveSol(int X, int Y, double dx, double dy,
            double dxs, double dys)
{
    QPoint p1, p2, p(X, Y);
    double x, y, xs, ys;
    int i, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;

    trace(4, "MouseMoveSol: X=%d Y=%d\n", X, Y);

    if (Drag <= 4) {
        for (i = 0; i < 3; i++) {
            GraphG[i]->GetCent(x, y);
            if (!MenuFixHoriz->isChecked())
                x = Xc + dx;
            if (!MenuFixVert->isChecked() || !MenuFixVert->isEnabled())
                y = i == Drag - 1 ? Yc + dy : y;
            GraphG[i]->SetCent(x, y);
            SetCentX(x);
        }
        if (MenuFixHoriz->isChecked()) {
            GraphG[0]->GetPos(p1, p2);
            Xcent = Xcent0 + 2.0 * (X - X0) / (p2.x() - p1.x());
            if (Xcent > 1.0) Xcent = 1.0;
            if (Xcent < -1.0) Xcent = -1.0;
        }
    } else if (Drag <= 7) {
        GraphG[Drag - 5]->GetCent(x, y);
        if (!MenuFixVert->isChecked() || !MenuFixVert->isEnabled())
            y = Yc + dy;
        GraphG[Drag - 5]->SetCent(x, y);
    } else if (Drag <= 14) {
        for (i = 0; i < 3; i++) {
            GraphG[i]->GetScale(xs, ys);
            GraphG[i]->SetScale(Xs * dxs, ys);
        }
        SetScaleX(Xs * dxs);
    } else if (Drag <= 17) {
        GraphG[Drag - 15]->GetScale(xs, ys);
        GraphG[Drag - 15]->SetScale(xs, Ys * dys);
    } else if (Drag == 20) {
        GraphG[0]->ToPos(p, x, y);
        if (PlotType == PLOT_SNR) {
            for (i = 0; i < NObs; i++)
                if (TimePos(Obs.data[IndexObs[i]].time) >= x) break;
            ObsIndex = i < NObs ? i : NObs - 1;
        } else {
            for (i = 0; i < SolData[sel].n; i++)
                if (TimePos(SolData[sel].data[i].time) >= x) break;
            SolIndex[sel] = i < SolData[sel].n ? i : SolData[sel].n - 1;
        }
        UpdateTime();
    }
    MenuCenterOri->setChecked(false);

    if (updateTime.elapsed() < RefreshTime) return;
    updateTime.restart();

    Refresh();
}
// callback on mouse-move events on observataion-data-plot ------------------
void Plot::MouseMoveObs(int X, int Y, double dx, double dy,
            double dxs, double dys)
{
    QPoint p1, p2, p(X, Y);
    double x, y, xs, ys;
    int i;

    Q_UNUSED(dys);

    trace(4, "MouseMoveObs: X=%d Y=%d\n", X, Y);

    if (Drag <= 4) {
        GraphR->GetCent(x, y);
        if (!MenuFixHoriz->isChecked()) x = Xc + dx;
        if (!MenuFixVert->isChecked()) y = Yc + dy;
        GraphR->SetCent(x, y);
        SetCentX(x);

        if (MenuFixHoriz->isChecked()) {
            GraphR->GetPos(p1, p2);
            Xcent = Xcent0 + 2.0 * (X - X0) / (p2.x() - p1.x());
            if (Xcent > 1.0) Xcent = 1.0;
            if (Xcent < -1.0) Xcent = -1.0;
        }
    } else if (Drag <= 14) {
        GraphR->GetScale(xs, ys);
        GraphR->SetScale(Xs * dxs, ys);
        SetScaleX(Xs * dxs);
    } else if (Drag == 20) {
        GraphR->ToPos(p, x, y);
        for (i = 0; i < NObs; i++)
            if (TimePos(Obs.data[IndexObs[i]].time) >= x) break;
        ObsIndex = i < NObs ? i : NObs - 1;
        UpdateTime();
    }
    MenuCenterOri->setChecked(false);

    if (updateTime.elapsed() < RefreshTime) return;
    updateTime.restart();

    Refresh();
}
// callback on mouse-wheel events -------------------------------------------
void Plot::wheelEvent(QWheelEvent *event)
{
    QPoint p(Xn, Yn);
    double xs, ys, ds = pow(2.0, -event->angleDelta().y() / 1200.0);
    int i, area = -1;

    event->accept();

    trace(4, "MouseWheel: WheelDelta=%d\n", event->angleDelta().y());

    if (Xn < 0 || Yn < 0) return;

    if (PlotType == PLOT_TRK) { // track-plot
        GraphT->GetScale(xs, ys);
        GraphT->SetScale(xs * ds, ys * ds);
    } else if (PlotType <= PLOT_NSAT || PlotType == PLOT_RES || PlotType == PLOT_SNR) {
        for (i = 0; i < 3; i++) {
            if (PlotType == PLOT_SNR && i != 1) continue;
            area = GraphG[i]->OnAxis(p);
            if (area == 0 || area == 1 || area == 2) {
                GraphG[i]->GetScale(xs, ys);
                GraphG[i]->SetScale(xs, ys * ds);
            } else if (area == 0) {
                break;
            }
        }
        if (area == 8) {
            for (i = 0; i < 3; i++) {
                GraphG[i]->GetScale(xs, ys);
                GraphG[i]->SetScale(xs * ds, ys);
                SetScaleX(xs * ds);
            }
        }
    } else if (PlotType == PLOT_OBS || PlotType == PLOT_DOP) {
        area = GraphR->OnAxis(p);
        if (area == 0 || area == 8) {
            GraphR->GetScale(xs, ys);
            GraphR->SetScale(xs * ds, ys);
            SetScaleX(xs * ds);
        }
    } else {
        return;
    }

    Refresh();
}
// callback on key-down events ----------------------------------------------
void Plot::keyPressEvent(QKeyEvent *event)
{
    double sfact = 1.05, fact = event->modifiers().testFlag(Qt::ShiftModifier) ? 1.0 : 10.0;
    double xc, yc, yc1, yc2, yc3, xs, ys, ys1, ys2, ys3;
    int key = event->modifiers().testFlag(Qt::ControlModifier) ? 10 : 0;

    trace(3, "FormKeyDown:\n");

    switch (event->key()) {
    case Qt::Key_Up: key += 1; break;
    case Qt::Key_Down: key += 2; break;
    case Qt::Key_Left: key += 3; break;
    case Qt::Key_Right: key += 4; break;
    default: return;
    }
    if (event->modifiers().testFlag(Qt::AltModifier)) return;

    if (PlotType == PLOT_TRK) {
        GraphT->GetCent(xc, yc);
        GraphT->GetScale(xs, ys);
        if (key == 1) {
            if (!MenuFixHoriz->isChecked()) yc += fact * ys;
        }
        if (key == 2) {
            if (!MenuFixHoriz->isChecked()) yc -= fact * ys;
        }
        if (key == 3) {
            if (!MenuFixHoriz->isChecked()) xc -= fact * xs;
        }
        if (key == 4) {
            if (!MenuFixHoriz->isChecked()) xc += fact * xs;
        }
        if (key == 11) {
            xs /= sfact; ys /= sfact;
        }
        if (key == 12) {
            xs *= sfact; ys *= sfact;
        }
        GraphT->SetCent(xc, yc);
        GraphT->SetScale(xs, ys);
    } else if (PlotType <= PLOT_NSAT || PlotType == PLOT_RES) {
        GraphG[0]->GetCent(xc, yc1);
        GraphG[1]->GetCent(xc, yc2);
        GraphG[2]->GetCent(xc, yc3);
        GraphG[0]->GetScale(xs, ys1);
        GraphG[1]->GetScale(xs, ys2);
        GraphG[2]->GetScale(xs, ys3);
        if (key == 1) {
            if (!MenuFixVert->isChecked()) yc1 += fact * ys1;
            yc2 += fact * ys2;
            yc3 += fact * ys3;
        }
        if (key == 2) {
            if (!MenuFixVert->isChecked()) yc1 -= fact * ys1;
            yc2 -= fact * ys2;
            yc3 -= fact * ys3;
        }
        if (key == 3) {
            if (!MenuFixHoriz->isChecked()) xc -= fact * xs;
        }
        if (key == 4) {
            if (!MenuFixHoriz->isChecked()) xc += fact * xs;
        }
        if (key == 11) {
            ys1 /= sfact;
            ys2 /= sfact;
            ys3 /= sfact;
        }
        if (key == 12) {
            ys1 *= sfact;
            ys2 *= sfact;
            ys3 *= sfact;
        }
        if (key == 13) xs *= sfact;
        if (key == 14) xs /= sfact;
        GraphG[0]->SetCent(xc, yc1);
        GraphG[1]->SetCent(xc, yc2);
        GraphG[2]->SetCent(xc, yc3);
        GraphG[0]->SetScale(xs, ys1);
        GraphG[1]->SetScale(xs, ys2);
        GraphG[2]->SetScale(xs, ys3);
    } else if (PlotType == PLOT_OBS || PlotType == PLOT_DOP || PlotType == PLOT_SNR) {
        GraphR->GetCent(xc, yc);
        GraphR->GetScale(xs, ys);
        if (key == 1) {
            if (!MenuFixVert->isChecked()) yc += fact * ys;
        }
        if (key == 2) {
            if (!MenuFixVert->isChecked()) yc -= fact * ys;
        }
        if (key == 3) {
            if (!MenuFixHoriz->isChecked()) xc -= fact * xs;
        }
        if (key == 4) {
            if (!MenuFixHoriz->isChecked()) xc += fact * xs;
        }
        if (key == 11) ys /= sfact;
        if (key == 12) xs *= sfact;
        if (key == 13) xs *= sfact;
        if (key == 14) xs /= sfact;
        GraphR->SetCent(xc, yc);
        GraphR->SetScale(xs, ys);
    }
    Refresh();
}
// callback on interval-timer -----------------------------------------------
void Plot::TimerTimer()
{
    const QColor color[] = { Qt::red, Qt::gray, QColor(0x00, 0xAA, 0xFF), Qt::green, QColor(0x00, 0xff, 0x00) };
    QLabel *strstatus[] = { StrStatus1, StrStatus2 };
    Console *console[] = { Console1, Console2 };
    QString connectmsg = "";
    static uint8_t buff[16384];
    solopt_t opt = solopt_default;
    sol_t *sol;
    const gtime_t ts = { 0, 0 };
    gtime_t time = {0, 0};
    double tint = TimeEna[2]?TimeInt:0.0, pos[3], ep[6];
    int i, j, n, inb, inr, cycle, nmsg[2] = { 0 }, stat, istat;
    int sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;
    char msg[MAXSTRMSG] = "";

    trace(4, "TimeTimer\n");

    if (ConnectState) { // real-time input mode
        for (i = 0; i < 2; i++) {
            opt.posf = RtFormat[i];
            opt.times = RtTimeForm == 0 ? 0 : RtTimeForm - 1;
            opt.timef = RtTimeForm >= 1;
            opt.degf = RtDegForm;
            strcpy(opt.sep, qPrintable(RtFieldSep));
            strsum(Stream + i, &inb, &inr, NULL, NULL);
            stat = strstat(Stream + i, msg);
            strstatus[i]->setStyleSheet(QStringLiteral("QLabel {color %1;}").arg(color2String(color[stat < 3 ? stat + 1 : 3])));
            if (*msg && strcmp(msg, "localhost"))
                connectmsg += QStringLiteral("(%1) %2 ").arg(i + 1).arg(msg);
            while ((n = strread(Stream + i, buff, sizeof(buff))) > 0) {
                for (j = 0; j < n; j++) {
                    istat = inputsol(buff[j], ts, ts, tint, 0, &opt, SolData + i);
                    if (istat == 0) continue;
                    if (istat < 0) { // disconnect received
                        Disconnect();
                        return;
                    }
                    if (Week == 0 && SolData[i].n == 1) { // first data
                        if (PlotType > PLOT_NSAT)
                            UpdateType(PLOT_TRK);
                        time2gpst(SolData[i].time, &Week);
                        UpdateOrigin();
                        ecef2pos(SolData[i].data[0].rr, pos);
                        mapView->SetCent(pos[0]*R2D,pos[1]*R2D);
                    }
                    nmsg[i]++;
                }
                console[i]->addMessage(buff, n);
            }
            if (nmsg[i] > 0) {
                strstatus[i]->setStyleSheet(QStringLiteral("QLabel {color %1;}").arg(color2String(color[4])));
                SolIndex[i] = SolData[i].n - 1;
            }
        }
        ConnectMsg->setText(connectmsg);

        if (nmsg[0] <= 0 && nmsg[1] <= 0) return;
    } else if (BtnAnimate->isEnabled() && BtnAnimate->isChecked()) { // animation mode
        cycle = AnimCycle <= 0 ? 1 : AnimCycle;

        if (PlotType <= PLOT_NSAT || PlotType == PLOT_RES) {
            SolIndex[sel] += cycle;
            if (SolIndex[sel] >= SolData[sel].n - 1) {
                SolIndex[sel] = SolData[sel].n - 1;
                BtnAnimate->setChecked(false);
            }
        } else {
            ObsIndex += cycle;
            if (ObsIndex >= NObs - 1) {
                ObsIndex = NObs - 1;
                BtnAnimate->setChecked(false);
            }
        }
    } else if (TimeSyncOut) { // time sync
        time.time = 0;
        while (strread(&StrTimeSync, (uint8_t *)StrBuff + NStrBuff, 1)) {
            if (++NStrBuff >= 1023) {
                NStrBuff = 0;
                continue;
            }
            if (StrBuff[NStrBuff-1] == '\n') {
                StrBuff[NStrBuff-1] = '\0';
                if (sscanf(StrBuff,"%lf/%lf/%lf %lf:%lf:%lf",ep,ep+1,ep+2,
                           ep+3,ep+4,ep+5)>=6) {
                    time=epoch2time(ep);
                }
                NStrBuff = 0;
            }
        }
        if (time.time && (PlotType <= PLOT_NSAT || PlotType <= PLOT_RES)) {
           i = SolIndex[sel];
           if (!(sol = getsol(SolData + sel, i))) return;
           double tt = timediff(sol->time, time);
           if (tt < -DTTOL) {
               for (;i < SolData[sel].n; i++) {
                   if (!(sol = getsol(SolData + sel,i))) continue;
                   if (timediff(sol->time, time) >= -DTTOL) {
                       i--;
                       break;
                   }
               }
           } else if (tt > DTTOL) {
               for (;i >= 0; i--) {
                   if (!(sol = getsol(SolData + sel,i))) continue;
                   if (timediff(sol->time, time) <= DTTOL) break;
               }
           }
           SolIndex[sel] = MAX(0, MIN(SolData[sel].n - 1, i));
        }
        else return;
    }
    else {
        return;
    }

    UpdateTime();

    if (updateTime.elapsed() < RefreshTime) return;
    updateTime.restart();

    UpdatePlot();
}
// set center of x-axis -----------------------------------------------------
void Plot::SetCentX(double c)
{
    double x, y;
    int i;

    trace(3, "SetCentX: c=%.3f:\n", c);

    GraphR->GetCent(x, y);
    GraphR->SetCent(c, y);
    for (i = 0; i < 3; i++) {
        GraphG[i]->GetCent(x, y);
        GraphG[i]->SetCent(c, y);
    }
}
// set scale of x-axis ------------------------------------------------------
void Plot::SetScaleX(double s)
{
    double xs, ys;
    int i;

    trace(3, "SetScaleX: s=%.3f:\n", s);

    GraphR->GetScale(xs, ys);
    GraphR->SetScale(s, ys);
    for (i = 0; i < 3; i++) {
        GraphG[i]->GetScale(xs, ys);
        GraphG[i]->SetScale(s, ys);
    }
}
// update plot-type with fit-range ------------------------------------------
void Plot::UpdateType(int type)
{
    trace(3, "UpdateType: type=%d\n", type);

    PlotType = type;

    if (AutoScale && PlotType <= PLOT_SOLA && (SolData[0].n > 0 || SolData[1].n > 0))
        FitRange(0);
    else
        SetRange(0, YRange);
    UpdatePlotType();
}
// update size of plot ------------------------------------------------------
void Plot::UpdateSize(void)
{
    QPushButton *btn[] = { BtnOn1, BtnOn2, BtnOn3 };
    QPoint p1(0, 0), p2(Disp->width(), Disp->height());
    double xs, ys,font_px=Disp->font().pixelSize()*1.33;
    int i, n, h, tmargin, bmargin, rmargin, lmargin;

    trace(3, "UpdateSize\n");

    tmargin=(int)(font_px*0.9); // top margin (px)
    bmargin=(int)(font_px*1.8); // bottom
    rmargin=(int)(font_px*1.2); // right
    lmargin=(int)(font_px*3.6); // left

    GraphT->resize();
    GraphS->resize();
    GraphR->resize();
    for (int i = 0; i < 3; i++)
        GraphG[i]->resize();
    for (int i = 0; i < 2; i++)
        GraphE[i]->resize();

    GraphT->SetPos(p1, p2);
    GraphS->SetPos(p1, p2);
    GraphS->GetScale(xs, ys);
    xs = MAX(xs, ys);
    GraphS->SetScale(xs, xs);
    p1.rx() += lmargin; p1.ry() += tmargin;
    p2.rx() -= rmargin; p2.setY(p2.y() - bmargin);
    GraphR->SetPos(p1, p2);

    p1.setX(p1.x()+(int)(font_px*1.2));
    p1.setY(tmargin); p2.setY(p1.y());
    for (i = n = 0; i < 3; i++) if (btn[i]->isChecked()) n++;
    for (i = 0; i < 3; i++) {
        if (!btn[i]->isChecked() || (n <= 0)) continue;
        if (n == 0) break;
        h = (Disp->height() - tmargin - bmargin) / n;
        p2.ry() += h;
        GraphG[i]->SetPos(p1, p2);
        p1.ry() += h;
    }
    p1.setY(tmargin); p2.setY(p1.y());
    for (i = n = 0; i < 2; i++) if (btn[i]->isChecked()) n++;
    for (i = 0; i < 2; i++) {
        if (!btn[i]->isChecked() || (n <= 0)) continue;
        if (n == 0) break;
        h = (Disp->height() - tmargin - bmargin) / n;
        p2.ry() += h;
        GraphE[i]->SetPos(p1, p2);
        p1.ry() += h;
    }
}
// update colors on plot ----------------------------------------------------
void Plot::UpdateColor(void)
{
    int i;

    trace(3, "UpdateColor\n");

    for (i = 0; i < 3; i++) {
        GraphT->Color[i] = CColor[i];
        GraphR->Color[i] = CColor[i];
        GraphS->Color[i] = CColor[i];
        GraphG[0]->Color[i] = CColor[i];
        GraphG[1]->Color[i] = CColor[i];
        GraphG[2]->Color[i] = CColor[i];
    }
    Disp->setFont(Font);
}
// update time-cursor -------------------------------------------------------
void Plot::UpdateTime(void)
{
    gtime_t time;
    sol_t *sol;
    double tt;
    int i, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;

    trace(3, "UpdateTime\n");

    // time-cursor change on solution-plot
    if (PlotType <= PLOT_NSAT || PlotType <= PLOT_RES) {
        TimeScroll->setMaximum(MAX(1, SolData[sel].n - 1));
        TimeScroll->setValue(SolIndex[sel]);
        if (!(sol = getsol(SolData + sel, SolIndex[sel]))) return;
        time = sol->time;
    } else if (NObs > 0) { // time-cursor change on observation-data-plot
        TimeScroll->setMaximum(MAX(1, NObs - 1));
        TimeScroll->setValue(ObsIndex);
        time = Obs.data[IndexObs[ObsIndex]].time;
    } else {
        return;
    }

    // time-synchronization between solutions and observation-data
    for (sel = 0; sel < 2; sel++) {
        i = SolIndex[sel];
        if (!(sol = getsol(SolData + sel, i))) continue;
        tt = timediff(sol->time, time);
        if (tt < -DTTOL) {
            for (; i < SolData[sel].n; i++) {
                if (!(sol = getsol(SolData + sel, i))) continue;
                if (timediff(sol->time, time) >= -DTTOL) break;
            }
        } else if (tt > DTTOL) {
            for (; i >= 0; i--) {
                if (!(sol = getsol(SolData + sel, i))) continue;
                if (timediff(sol->time, time) <= DTTOL) break;
            }
        }
        SolIndex[sel] = MAX(0, MIN(SolData[sel].n - 1, i));
    }
    i = ObsIndex;
    if (i <= NObs - 1) {
        tt = timediff(Obs.data[IndexObs[i]].time, time);
        if (tt < -DTTOL) {
            for (; i < NObs; i++)
                if (timediff(Obs.data[IndexObs[i]].time, time) >= -DTTOL) break;
        } else if (tt > DTTOL) {
            for (; i >= 0; i--)
                if (timediff(Obs.data[IndexObs[i]].time, time) <= DTTOL) break;
        }
        ObsIndex = MAX(0, MIN(NObs - 1, i));
    }
}
// update origin of plot ----------------------------------------------------
void Plot::UpdateOrigin(void)
{
    gtime_t time = { 0, 0 };
    sol_t *sol;
    double opos[3] = { 0 }, pos[3], ovel[3] = { 0 };
    int i, j, n = 0, sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;
    QString sta;

    trace(3, "UpdateOrigin\n");

    if (Origin == ORG_STARTPOS) {
        if (!(sol = getsol(SolData, 0)) || sol->type != 0) return;
        for (i = 0; i < 3; i++) opos[i] = sol->rr[i];
    } else if (Origin == ORG_ENDPOS) {
        if (!(sol = getsol(SolData, SolData[0].n - 1)) || sol->type != 0) return;
        for (i = 0; i < 3; i++) opos[i] = sol->rr[i];
    } else if (Origin == ORG_AVEPOS) {
        for (i = 0; (sol = getsol(SolData, i)) != NULL; i++) {
            if (sol->type != 0) continue;
            for (j = 0; j < 3; j++) opos[j] += sol->rr[j];
            n++;
        }
        if (n > 0) for (i = 0; i < 3; i++) opos[i] /= n;
    } else if (Origin == ORG_FITPOS) {
        if (!FitPos(&time, opos, ovel)) return;
    } else if (Origin == ORG_REFPOS) {
        if (norm(SolData[0].rb, 3) > 0.0) {
            for (i = 0; i < 3; i++) opos[i] = SolData[0].rb[i];
        } else {
            if (!(sol = getsol(SolData, 0)) || sol->type != 0) return;
            for (i = 0; i < 3; i++) opos[i] = sol->rr[i];
        }
    } else if (Origin == ORG_LLHPOS) {
        pos2ecef(OOPos, opos);
    } else if (Origin == ORG_AUTOPOS) {
        if (SolFiles[sel].count() > 0) {
            QFileInfo fi(SolFiles[sel].at(0));

            ReadStaPos(fi.baseName().left(4).toUpper(), sta, opos);
        }
    } else if (Origin == ORG_IMGPOS) {
        pos[0] = MapLat * D2R;
        pos[1] = MapLon * D2R;
        pos[2] = 0.0;
        pos2ecef(pos, opos);
    } else if (Origin == ORG_MAPPOS) {
        pos[0] = (Gis.bound[0] + Gis.bound[1]) / 2.0;
        pos[1] = (Gis.bound[2] + Gis.bound[3]) / 2.0;
        pos[2] = 0.0;
        pos2ecef(pos, opos);
    } else if (Origin - ORG_PNTPOS < MAXWAYPNT) {
        for (i = 0; i < 3; i++) opos[i] = PntPos[Origin - ORG_PNTPOS][i];
    }
    if (norm(opos, 3) <= 0.0) {
        // default start position
        if (!(sol = getsol(SolData, 0)) || sol->type != 0) return;
        for (i = 0; i < 3; i++) opos[i] = sol->rr[i];
    }
    OEpoch = time;
    for (i = 0; i < 3; i++) {
        OPos[i] = opos[i];
        OVel[i] = ovel[i];
    }
    Refresh_MapView();
}
// update satellite mask ----------------------------------------------------
void Plot::UpdateSatMask(void)
{
    int sat, prn;
    char buff[256], *p;

    trace(3, "UpdateSatMask\n");

    for (sat = 1; sat <= MAXSAT; sat++) SatMask[sat - 1] = 0;
    for (sat = 1; sat <= MAXSAT; sat++)
        if (!(satsys(sat, &prn) & NavSys)) SatMask[sat - 1] = 1;
    if (ExSats != "") {
        strcpy(buff, qPrintable(ExSats));

        for (p = strtok(buff, " "); p; p = strtok(NULL, " ")) {
            if (*p == '+' && (sat = satid2no(p + 1))) SatMask[sat - 1] = 0; // included
            else if ((sat = satid2no(p))) SatMask[sat - 1] = 1;             // excluded
        }
    }
}
// update satellite select ---------------------------------------------------
void Plot::UpdateSatSel(void)
{
    QString SatListText = SatList->currentText();
    char id[16];
    int i, sys = 0;

    if (SatListText == "G") sys = SYS_GPS;
    else if (SatListText == "R") sys = SYS_GLO;
    else if (SatListText == "E") sys = SYS_GAL;
    else if (SatListText == "J") sys = SYS_QZS;
    else if (SatListText == "C") sys = SYS_CMP;
    else if (SatListText == "I") sys = SYS_IRN;
    else if (SatListText == "S") sys = SYS_SBS;

    for (i = 0; i < MAXSAT; i++) {
        satno2id(i + 1, id);
        SatSel[i] = SatListText == "ALL" || SatListText == id || satsys(i + 1, NULL) == sys;
    }
}
// update enable/disable of widgets -----------------------------------------
void Plot::UpdateEnable(void)
{
    bool data = BtnSol1->isChecked() || BtnSol2->isChecked() || BtnSol12->isChecked();
    bool plot = (PLOT_SOLP <= PlotType) && (PlotType <= PLOT_NSAT);
    bool sel = (!BtnSol1->isChecked()) && (BtnSol2->isChecked()) ? 1 : 0;

    trace(3, "UpdateEnable\n");

    Panel1->setVisible(MenuToolBar->isChecked());
    Panel21->setVisible(MenuStatusBar->isChecked());

    MenuConnect->setChecked(ConnectState);
    BtnSol1->setEnabled(true);
    BtnSol2->setEnabled(PlotType<=PLOT_NSAT||PlotType==PLOT_RES||PlotType==PLOT_RESE);
    BtnSol12->setEnabled(!ConnectState && PlotType <= PLOT_SOLA && SolData[0].n > 0 && SolData[1].n > 0);

    QFlag->setVisible(PlotType == PLOT_TRK || PlotType == PLOT_SOLP ||
              PlotType == PLOT_SOLV || PlotType == PLOT_SOLA ||
              PlotType == PLOT_NSAT);
    ObsType->setVisible(PlotType == PLOT_OBS || PlotType <= PLOT_SKY);
    ObsType2->setVisible(PlotType == PLOT_SNR || PlotType == PLOT_SNRE || PlotType == PLOT_MPS);

    FrqType->setVisible(PlotType == PLOT_RES||PlotType==PLOT_RESE);
    DopType->setVisible(PlotType == PLOT_DOP);
    SatList->setVisible(PlotType == PLOT_RES ||PlotType==PLOT_RESE||PlotType>=PLOT_OBS||
                PlotType == PLOT_SKY || PlotType == PLOT_DOP ||
                PlotType == PLOT_SNR || PlotType == PLOT_SNRE ||
                PlotType == PLOT_MPS);
    QFlag->setEnabled(data);
    ObsType->setEnabled(data && !SimObs);
    ObsType2->setEnabled(data && !SimObs);

    Panel102->setVisible(PlotType==PLOT_SOLP||PlotType==PLOT_SOLV||
                         PlotType==PLOT_SOLA||PlotType==PLOT_NSAT||
                         PlotType==PLOT_RES ||PlotType==PLOT_RESE||
                         PlotType==PLOT_SNR ||PlotType==PLOT_SNRE);
    BtnOn1->setEnabled(plot||PlotType==PLOT_SNR||PlotType==PLOT_RES||
                             PlotType==PLOT_RESE||PlotType==PLOT_SNRE);
    BtnOn2->setEnabled(plot||PlotType==PLOT_SNR||PlotType==PLOT_RES||
                             PlotType==PLOT_RESE||PlotType==PLOT_SNRE);
    BtnOn3->setEnabled(plot || PlotType == PLOT_SNR || PlotType == PLOT_RES);

    BtnCenterOri->setVisible(PlotType == PLOT_TRK || PlotType == PLOT_SOLP ||
                 PlotType == PLOT_SOLV || PlotType == PLOT_SOLA ||
                 PlotType == PLOT_NSAT);
    BtnCenterOri->setEnabled(PlotType != PLOT_NSAT);
    BtnRangeList->setVisible(PlotType == PLOT_TRK || PlotType == PLOT_SOLP ||
                 PlotType == PLOT_SOLV || PlotType == PLOT_SOLA ||
                 PlotType == PLOT_NSAT);
    BtnRangeList->setEnabled(PlotType != PLOT_NSAT);


    BtnFitHoriz->setVisible(PlotType == PLOT_SOLP || PlotType == PLOT_SOLV ||
                PlotType == PLOT_SOLA || PlotType == PLOT_NSAT ||
                PlotType == PLOT_RES || PlotType == PLOT_OBS ||
                PlotType == PLOT_DOP || PlotType == PLOT_SNR ||
                PlotType == PLOT_SNRE);
    BtnFitVert->setVisible(PlotType == PLOT_TRK || PlotType == PLOT_SOLP ||
                   PlotType == PLOT_SOLV || PlotType == PLOT_SOLA);
    BtnFitHoriz->setEnabled(data);
    BtnFitVert->setEnabled(data);

    MenuShowTrack->setEnabled(data);
    BtnFixCent->setVisible(PlotType == PLOT_TRK);
    BtnFixHoriz->setVisible(PlotType == PLOT_SOLP || PlotType == PLOT_SOLV ||
                PlotType == PLOT_SOLA || PlotType == PLOT_NSAT ||
                PlotType == PLOT_RES || PlotType == PLOT_OBS ||
                PlotType == PLOT_DOP || PlotType == PLOT_SNR);
    BtnFixVert->setVisible(PlotType == PLOT_SOLP || PlotType == PLOT_SOLV ||
                   PlotType == PLOT_SOLA);
    BtnFixCent->setEnabled(data);
    BtnFixHoriz->setEnabled(data);
    BtnFixVert->setEnabled(data);
    BtnShowMap->setVisible(PlotType == PLOT_TRK);
    BtnShowMap->setEnabled(!BtnSol12->isChecked());
    MenuMapView->setVisible(PlotType==PLOT_TRK||PlotType==PLOT_SOLP);
    Panel12->setVisible(!ConnectState);
    BtnAnimate->setVisible(data && MenuShowTrack->isChecked());
    TimeScroll->setVisible(data && MenuShowTrack->isChecked());
    TimeScroll->setEnabled(data && MenuShowTrack->isChecked());

    if (!MenuShowTrack->isChecked()) {
        MenuFixHoriz->setEnabled(false);
        MenuFixVert->setEnabled(false);
        MenuFixCent->setEnabled(false);
        BtnAnimate->setChecked(false);
    }
    MenuMapImg->setEnabled(MapImage.height() > 0);
    MenuSkyImg->setEnabled(SkyImageI.height() > 0);
    MenuSrcSol->setEnabled(SolFiles[sel].count() > 0);
    MenuSrcObs->setEnabled(ObsFiles.count() > 0);
    MenuMapLayer->setEnabled(true);

    MenuShowSkyplot->setEnabled(MenuShowSkyplot->isVisible());

    BtnShowImg->setVisible(PlotType == PLOT_TRK || PlotType == PLOT_SKY ||
                   PlotType == PLOT_MPS);
    BtnShowGrid->setVisible(PlotType==PLOT_TRK);
    MenuAnimStart->setEnabled(!ConnectState && BtnAnimate->isEnabled() && !BtnAnimate->isChecked());
    MenuAnimStop->setEnabled(!ConnectState && BtnAnimate->isEnabled() && BtnAnimate->isChecked());

    MenuOpenSol1->setEnabled(!ConnectState);
    MenuOpenSol2->setEnabled(!ConnectState);
    MenuConnect->setEnabled(!ConnectState);
    MenuDisconnect->setEnabled(ConnectState);
    MenuPort->setEnabled(!ConnectState);
    MenuOpenObs->setEnabled(!ConnectState);
    MenuOpenNav->setEnabled(!ConnectState);
    MenuOpenElevMask->setEnabled(!ConnectState);
    MenuReload->setEnabled(!ConnectState);

    MenuReload->setEnabled(!ConnectState);
    StrStatus->setEnabled(ConnectState);
    BtnFreq->setVisible(FrqType->isVisible()||ObsType->isVisible()||ObsType2->isVisible());
}
// linear-fitting of positions ----------------------------------------------
int Plot::FitPos(gtime_t *time, double *opos, double *ovel)
{
    sol_t *sol;
    int i, j;
    double t, x[2], Ay[3][2] = { { 0 } }, AA[3][4] = { { 0 } };

    trace(3, "FitPos\n");

    if (SolData[0].n <= 0) return 0;

    for (i = 0; (sol = getsol(SolData, i)) != NULL; i++) {
        if (sol->type != 0) continue;
        if (time->time == 0) *time = sol->time;
        t = timediff(sol->time, *time);

        for (j = 0; j < 3; j++) {
            Ay[j][0] += sol->rr[j];
            Ay[j][1] += sol->rr[j] * t;
            AA[j][0] += 1.0;
            AA[j][1] += t;
            AA[j][2] += t;
            AA[j][3] += t * t;
        }
    }
    for (i = 0; i < 3; i++) {
        if (solve("N", AA[i], Ay[i], 2, 1, x)) return 0;
        opos[i] = x[0];
        ovel[i] = x[1];
    }
    return 1;
}
// fit time-range of plot ---------------------------------------------------
void Plot::FitTime(void)
{
    sol_t *sols, *sole;
    double tl[2] = { 86400.0 * 7, 0.0 }, tp[2], xl[2], yl[2], zl[2];
    int sel = !BtnSol1->isChecked() && BtnSol2->isChecked() ? 1 : 0;

    trace(3, "FitTime\n");

    sols = getsol(SolData + sel, 0);
    sole = getsol(SolData + sel, SolData[sel].n - 1);
    if (sols && sole) {
        tl[0] = MIN(tl[0], TimePos(sols->time));
        tl[1] = MAX(tl[1], TimePos(sole->time));
    }
    if (Obs.n > 0) {
        tl[0] = MIN(tl[0], TimePos(Obs.data[0].time));
        tl[1] = MAX(tl[1], TimePos(Obs.data[Obs.n - 1].time));
    }
    if (TimeEna[0]) tl[0] = TimePos(TimeStart);
    if (TimeEna[1]) tl[1] = TimePos(TimeEnd);

    if (qFuzzyCompare(tl[0], tl[1])) {
        tl[0] = tl[0] - DEFTSPAN / 2.0;
        tl[1] = tl[0] + DEFTSPAN / 2.0;
    } else if (tl[0] > tl[1]) {
        tl[0] = -DEFTSPAN / 2.0;
        tl[1] = DEFTSPAN / 2.0;
    }
    GraphG[0]->GetLim(tp, xl);
    GraphG[1]->GetLim(tp, yl);
    GraphG[2]->GetLim(tp, zl);
    GraphG[0]->SetLim(tl, xl);
    GraphG[1]->SetLim(tl, yl);
    GraphG[2]->SetLim(tl, zl);
    GraphR->GetLim(tp, xl);
    GraphR->SetLim(tl, xl);
}
// set x/y-range of plot ----------------------------------------------------
void Plot::SetRange(int all, double range)
{
    double xl[] = { -range, range };
    double yl[] = { -range, range };
    double zl[] = { -range, range };
    double xs, ys, tl[2], xp[2], pos[3];

    trace(3, "SetRange: all=%d range=%.3f\n", all, range);

    if (all || PlotType == PLOT_TRK) {
        GraphT->SetLim(xl, yl);
        GraphT->GetScale(xs, ys);
        GraphT->SetScale(MAX(xs, ys), MAX(xs, ys));
        if (norm(OPos, 3) > 0.0) {
            ecef2pos(OPos, pos);
            mapView->SetCent(pos[0] * R2D, pos[1] * R2D);
        }
    }
    if (PLOT_SOLP <= PlotType && PlotType <= PLOT_SOLA) {
        GraphG[0]->GetLim(tl, xp);
        GraphG[0]->SetLim(tl, xl);
        GraphG[1]->SetLim(tl, yl);
        GraphG[2]->SetLim(tl, zl);
    } else if (PlotType == PLOT_NSAT) {
        GraphG[0]->GetLim(tl, xp);
        xl[0] = yl[0] = zl[0] = 0.0;
        xl[1] = MaxDop;
        yl[1] = YLIM_AGE;
        zl[1] = YLIM_RATIO;
        GraphG[0]->SetLim(tl, xl);
        GraphG[1]->SetLim(tl, yl);
        GraphG[2]->SetLim(tl, zl);
    } else if (PlotType < PLOT_SNR) {
        GraphG[0]->GetLim(tl, xp);
        xl[0] = -MaxMP; xl[1] = MaxMP;
        yl[0] = -MaxMP/100.0; yl[1] = MaxMP/100.0;
        zl[0] = 0.0; zl[1] = 90.0;
        GraphG[0]->SetLim(tl, xl);
        GraphG[1]->SetLim(tl, yl);
        GraphG[2]->SetLim(tl, zl);
    } else {
        GraphG[0]->GetLim(tl, xp);
        xl[0] = 10.0; xl[1] = 60.0;
        yl[0] = -MaxMP; yl[1] = MaxMP;
        zl[0] = 0.0; zl[1] = 90.0;
        GraphG[0]->SetLim(tl, xl);
        GraphG[1]->SetLim(tl, yl);
        GraphG[2]->SetLim(tl, zl);
    }
}
// fit x/y-range of plot ----------------------------------------------------
void Plot::FitRange(int all)
{
    TIMEPOS *pos, *pos1, *pos2;
    sol_t *data;
    double xs, ys, xp[2], tl[2], xl[] = { 1E8, -1E8 }, yl[2] = { 1E8, -1E8 }, zl[2] = { 1E8, -1E8 };
    double lat, lon, lats[2] = { 90, -90 }, lons[2] = { 180, -180 }, llh[3];
    int i, type = PlotType - PLOT_SOLP;

    trace(3, "FitRange: all=%d\n", all);

    MenuFixHoriz->setChecked(false);

    if (BtnSol1->isChecked()) {
        pos = SolToPos(SolData, -1, QFlag->currentIndex(), type);

        for (i = 0; i < pos->n; i++) {
            xl[0] = MIN(xl[0], pos->x[i]);
            yl[0] = MIN(yl[0], pos->y[i]);
            zl[0] = MIN(zl[0], pos->z[i]);
            xl[1] = MAX(xl[1], pos->x[i]);
            yl[1] = MAX(yl[1], pos->y[i]);
            zl[1] = MAX(zl[1], pos->z[i]);
        }
        delete pos;
    }
    if (BtnSol2->isChecked()) {
        pos = SolToPos(SolData + 1, -1, QFlag->currentIndex(), type);

        for (i = 0; i < pos->n; i++) {
            xl[0] = MIN(xl[0], pos->x[i]);
            yl[0] = MIN(yl[0], pos->y[i]);
            zl[0] = MIN(zl[0], pos->z[i]);
            xl[1] = MAX(xl[1], pos->x[i]);
            yl[1] = MAX(yl[1], pos->y[i]);
            zl[1] = MAX(zl[1], pos->z[i]);
        }
        delete pos;
    }
    if (BtnSol12->isChecked()) {
        pos1 = SolToPos(SolData, -1, 0, type);
        pos2 = SolToPos(SolData + 1, -1, 0, type);
        pos = pos1->diff(pos2, QFlag->currentIndex());

        for (i = 0; i < pos->n; i++) {
            xl[0] = MIN(xl[0], pos->x[i]);
            yl[0] = MIN(yl[0], pos->y[i]);
            zl[0] = MIN(zl[0], pos->z[i]);
            xl[1] = MAX(xl[1], pos->x[i]);
            yl[1] = MAX(yl[1], pos->y[i]);
            zl[1] = MAX(zl[1], pos->z[i]);
        }
        delete pos1;
        delete pos2;
        delete pos;
    }
    xl[0] -= 0.05;
    xl[1] += 0.05;
    yl[0] -= 0.05;
    yl[1] += 0.05;
    zl[0] -= 0.05;
    zl[1] += 0.05;

    if (all || PlotType == PLOT_TRK) {
        GraphT->SetLim(xl, yl);
        GraphT->GetScale(xs, ys);
        GraphT->SetScale(MAX(xs, ys), MAX(xs, ys));
    }
    if (all || PlotType <= PLOT_SOLA || PlotType == PLOT_RES) {
        GraphG[0]->GetLim(tl, xp);
        GraphG[0]->SetLim(tl, xl);
        GraphG[1]->SetLim(tl, yl);
        GraphG[2]->SetLim(tl, zl);
    }
    if (all) {
        if (BtnSol1->isChecked()) {
            for (i = 0; (data = getsol(SolData, i)) != NULL; i++) {
                ecef2pos(data->rr, llh);
                lats[0] = MIN(lats[0], llh[0] * R2D);
                lons[0] = MIN(lons[0], llh[1] * R2D);
                lats[1] = MAX(lats[1], llh[0] * R2D);
                lons[1] = MAX(lons[1], llh[1] * R2D);
            }
        }
        if (BtnSol2->isChecked()) {
            for (i = 0; (data = getsol(SolData + 1, i)) != NULL; i++) {
                ecef2pos(data->rr, llh);
                lats[0] = MIN(lats[0], llh[0] * R2D);
                lons[0] = MIN(lons[0], llh[1] * R2D);
                lats[1] = MAX(lats[1], llh[0] * R2D);
                lons[1] = MAX(lons[1], llh[1] * R2D);
            }
        }
        if (lats[0] <= lats[1] && lons[0] <= lons[1]) {
            lat = (lats[0] + lats[1]) / 2.0;
            lon = (lons[0] + lons[1]) / 2.0;
        }
    }
}
// set center of track plot -------------------------------------------------
void Plot::SetTrkCent(double lat, double lon)
{
    gtime_t time = { 0, 0 };
    double pos[3] = { 0 }, rr[3], xyz[3];

    if (PlotType != PLOT_TRK) return;
    pos[0] = lat * D2R;
    pos[1] = lon * D2R;
    pos2ecef(pos, rr);
    PosToXyz(time, rr, 0, xyz);
    GraphT->SetCent(xyz[0], xyz[1]);
    UpdatePlot();
}
// load options from ini-file -----------------------------------------------
void Plot::LoadOpt(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    trace(3, "LoadOpt\n");

//    PlotType = settings.value("plot/plottype", 0).toInt();
    TimeLabel = settings.value("plot/timelabel", 1).toInt();
    LatLonFmt = settings.value("plot/latlonfmt", 0).toInt();
    AutoScale = settings.value("plot/autoscale", 1).toInt();
    ShowStats = settings.value("plot/showstats", 0).toInt();
    ShowLabel = settings.value("plot/showlabel", 1).toInt();
    ShowGLabel = settings.value("plot/showglabel", 1).toInt();
    ShowCompass = settings.value("plot/showcompass", 0).toInt();
    ShowScale = settings.value("plot/showscale", 1).toInt();
    ShowArrow = settings.value("plot/showarrow", 0).toInt();
    ShowSlip = settings.value("plot/showslip", 0).toInt();
    ShowHalfC = settings.value("plot/showhalfc", 0).toInt();
    ShowErr = settings.value("plot/showerr", 0).toInt();
    ShowEph = settings.value("plot/showeph", 0).toInt();
    PlotStyle = settings.value("plot/plotstyle", 0).toInt();
    MarkSize = settings.value("plot/marksize", 2).toInt();
    NavSys = settings.value("plot/navsys", SYS_ALL).toInt();
    AnimCycle = settings.value("plot/animcycle", 10).toInt();
    RefCycle = settings.value("plot/refcycle", 100).toInt();
    HideLowSat = settings.value("plot/hidelowsat", 0).toInt();
    ElMaskP = settings.value("plot/elmaskp", 0).toInt();
    ExSats = settings.value("plot/exsats", "").toString();
    RtBuffSize = settings.value("plot/rtbuffsize", 10800).toInt();
    TimeSyncOut = settings.value("plot/timesyncout", 0).toInt();
    TimeSyncPort = settings.value("plot/timesyncport", 10071).toInt();
    RtStream[0] = settings.value("plot/rtstream1", 0).toInt();
    RtStream[1] = settings.value("plot/rtstream2", 0).toInt();
    RtFormat[0] = settings.value("plot/rtformat1", 0).toInt();
    RtFormat[1] = settings.value("plot/rtformat2", 0).toInt();
    RtTimeForm = settings.value("plot/rttimeform", 0).toInt();
    RtDegForm = settings.value("plot/rtdegform", 0).toInt();
    RtFieldSep = settings.value("plot/rtfieldsep", "").toString();
    RtTimeOutTime = settings.value("plot/rttimeouttime", 0).toInt();
    RtReConnTime = settings.value("plot/rtreconntime", 10000).toInt();

    MColor[0][0] = settings.value("plot/mcolor0", QColor(0xc0, 0xc0, 0xc0)).value<QColor>();
    MColor[0][1] = settings.value("plot/mcolor1", QColor(Qt::green)).value<QColor>();
    MColor[0][2] = settings.value("plot/mcolor2", QColor(0x00, 0xAA, 0xFF)).value<QColor>();
    MColor[0][3] = settings.value("plot/mcolor3", QColor(0xff, 0x00, 0xff)).value<QColor>();
    MColor[0][4] = settings.value("plot/mcolor4", QColor(Qt::blue)).value<QColor>();
    MColor[0][5] = settings.value("plot/mcolor5", QColor(Qt::red)).value<QColor>();
    MColor[0][6] = settings.value("plot/mcolor6", QColor(0x80, 0x80, 0x00)).value<QColor>();
    MColor[0][7] = settings.value("plot/mcolor7", QColor(Qt::gray)).value<QColor>();
    MColor[1][0] = settings.value("plot/mcolor8", QColor(0xc0, 0xc0, 0xc0)).value<QColor>();
    MColor[1][1] = settings.value("plot/mcolor9", QColor(0x80, 0x40, 0x00)).value<QColor>();
    MColor[1][2] = settings.value("plot/mcolor10", QColor(0x00, 0x80, 0x80)).value<QColor>();
    MColor[1][3] = settings.value("plot/mcolor11", QColor(0xFF, 0x00, 0x80)).value<QColor>();
    MColor[1][4] = settings.value("plot/mcolor12", QColor(0xFF, 0x80, 0x00)).value<QColor>();
    MColor[1][5] = settings.value("plot/mcolor13", QColor(0x80, 0x80, 0xFF)).value<QColor>();
    MColor[1][6] = settings.value("plot/mcolor14", QColor(0xFF, 0x80, 0x80)).value<QColor>();
    MColor[1][7] = settings.value("plot/mcolor15", QColor(Qt::gray)).value<QColor>();
    MapColor[0] = settings.value("plot/mapcolor1", QColor(0xc0, 0xc0, 0xc0)).value<QColor>();
    MapColor[1] = settings.value("plot/mapcolor2", QColor(0xc0, 0xc0, 0xc0)).value<QColor>();
    MapColor[2] = settings.value("plot/mapcolor3", QColor(0xf0, 0xd0, 0xd0)).value<QColor>();
    MapColor[3] = settings.value("plot/mapcolor4", QColor(0xd0, 0xf0, 0xd0)).value<QColor>();
    MapColor[4] = settings.value("plot/mapcolor5", QColor(0xd0, 0xd0, 0xf0)).value<QColor>();
    MapColor[5] = settings.value("plot/mapcolor6", QColor(0xd0, 0xf0, 0xf0)).value<QColor>();
    MapColor[6] = settings.value("plot/mapcolor7", QColor(0xf8, 0xf8, 0xd0)).value<QColor>();
    MapColor[7] = settings.value("plot/mapcolor8", QColor(0xf0, 0xf0, 0xf0)).value<QColor>();
    MapColor[8] = settings.value("plot/mapcolor9", QColor(0xf0, 0xf0, 0xf0)).value<QColor>();
    MapColor[9] = settings.value("plot/mapcolor10", QColor(0xf0, 0xf0, 0xf0)).value<QColor>();
    MapColor[10] = settings.value("plot/mapcolor11", QColor(Qt::white)).value<QColor>();
    MapColor[11] = settings.value("plot/mapcolor12", QColor(Qt::white)).value<QColor>();
    MapColorF[0] = settings.value("plot/mapcolorf1", QColor(Qt::white)).value<QColor>();
    MapColorF[1] = settings.value("plot/mapcolorf2", QColor(Qt::white)).value<QColor>();
    MapColorF[2] = settings.value("plot/mapcolorf3", QColor(Qt::white)).value<QColor>();
    MapColorF[3] = settings.value("plot/mapcolorf4", QColor(Qt::white)).value<QColor>();
    MapColorF[4] = settings.value("plot/mapcolorf5", QColor(Qt::white)).value<QColor>();
    MapColorF[5] = settings.value("plot/mapcolorf6", QColor(Qt::white)).value<QColor>();
    MapColorF[6] = settings.value("plot/mapcolorf7", QColor(Qt::white)).value<QColor>();
    MapColorF[7] = settings.value("plot/mapcolorf8", QColor(Qt::white)).value<QColor>();
    MapColorF[8] = settings.value("plot/mapcolorf9", QColor(Qt::white)).value<QColor>();
    MapColorF[9] = settings.value("plot/mapcolorf10", QColor(Qt::white)).value<QColor>();
    MapColorF[10] = settings.value("plot/mapcolorf11", QColor(Qt::white)).value<QColor>();
    MapColorF[11] = settings.value("plot/mapcolorf12", QColor(Qt::white)).value<QColor>();
    CColor[0] = settings.value("plot/color1", QColor(Qt::white)).value<QColor>();
    CColor[1] = settings.value("plot/color2", QColor(0xc0, 0xc0, 0xc0)).value<QColor>();
    CColor[2] = settings.value("plot/color3", QColor(Qt::black)).value<QColor>();
    CColor[3] = settings.value("plot/color4", QColor(0xc0, 0xc0, 0xc0)).value<QColor>();

    plotOptDialog->refDialog->stationPositionFile = settings.value("plot/staposfile", "").toString();

    ElMask = settings.value("plot/elmask", 0.0).toDouble();
    MaxDop = settings.value("plot/maxdop", 30.0).toDouble();
    MaxMP = settings.value("plot/maxmp", 10.0).toDouble();
    YRange = settings.value("plot/yrange", 5.0).toDouble();
    Origin = settings.value("plot/orgin", 2).toInt();
    RcvPos = settings.value("plot/rcvpos", 0).toInt();
    OOPos[0] = settings.value("plot/oopos1", 0).toDouble();
    OOPos[1] = settings.value("plot/oopos2", 0).toDouble();
    OOPos[2] = settings.value("plot/oopos3", 0).toDouble();
    ShapeFile = settings.value("plot/shapefile", "").toString();
    TLEFile = settings.value("plot/tlefile", "").toString();
    TLESatFile = settings.value("plot/tlesatfile", "").toString();

    FontName = settings.value("plot/fontname","Tahoma").toString();
    FontSize = settings.value("plot/fontsize",8).toInt();
    Font.setFamily(FontName);
    Font.setPointSize(FontSize);

    RnxOpts = settings.value("plot/rnxopts", "").toString();

    MapApi = settings.value("mapview/mapapi" , 0).toInt();
    ApiKey = settings.value("mapview/apikey" ,"").toString();
    MapStrs[0][0] = settings.value("mapview/mapstrs_0_0","OpenStreetMap").toString();
    MapStrs[0][1] = settings.value("mapview/mapstrs_0_1","https://tile.openstreetmap.org/{z}/{x}/{y}.png").toString();
    MapStrs[0][2] = settings.value("mapview/mapstrs_0_2","https://osm.org/copyright").toString();
    for (int i=1;i<6;i++) for (int j=0;j<3;j++) {
        MapStrs[i][j] = settings.value(QString("mapview/mapstrs_%1_%2").arg(i).arg(j),"").toString();
    }
    for (int i=0;i<2;i++) {
        StrCmds  [0][i] = settings.value(QString("str/strcmd1_%1").arg(i), "").toString();
        StrCmds  [1][i] = settings.value(QString("str/strcmd2_%1").arg(i), "").toString();
        StrCmdEna[0][i] = settings.value(QString("str/strcmdena1_%1").arg(i), 0).toInt();
        StrCmdEna[1][i] = settings.value(QString("str/strcmdena2_%1").arg(i), 0).toInt();
    }
    for (int i = 0; i < 3; i++) {
        StrPaths[0][i] = settings.value(QString("str/strpath1_%1").arg(i), "").toString();
        StrPaths[1][i] = settings.value(QString("str/strpath2_%1").arg(i), "").toString();
    }
    for (int i = 0; i < 10; i++) {
        StrHistory [i] = settings.value(QString("str/strhistry_%1").arg(i), "").toString();
    }

    TextViewer::colorText = settings.value("viewer/color1", QColor(Qt::black)).value<QColor>();
    TextViewer::colorBackground = settings.value("viewer/color2", QColor(Qt::white)).value<QColor>();
    TextViewer::font.setFamily(settings.value("viewer/fontname", "Consolas").toString());
    TextViewer::font.setPointSize(settings.value("viewer/fontsize", 9).toInt());

    MenuBrowse->setChecked(settings.value("solbrows/show",       0).toBool());
    BrowseSplitter->restoreState(settings.value("solbrows/split1",   100).toByteArray());
    Dir  =settings.value("solbrows/dir",  "C:\\").toString();

    for (int i = 0; i < RangeList->count(); i++) {
        QString s=RangeList->item(i)->text();
        double range;
        QString unit;
        bool ok;

        QStringList tokens = s.split(' ');
        if (tokens.size()==2) {
            range = tokens.at(0).toInt(&ok);
            unit = tokens.at(1);
            if (ok) {
                if (unit == "cm") range*=0.01;
                else if (unit == "km") range*=1000.0;
                if (range==YRange) {
                    RangeList->item(i)->setSelected(true);
                    break;
                }

            }
        }

    }
}
// save options to ini-file -------------------------------------------------
void Plot::SaveOpt(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    trace(3, "SaveOpt\n");

    //settings.setValue("plot/plottype", PlotType);
    settings.setValue("plot/timelabel", TimeLabel);
    settings.setValue("plot/latlonfmt", LatLonFmt);
    settings.setValue("plot/autoscale", AutoScale);
    settings.setValue("plot/showstats", ShowStats);
    settings.setValue("plot/showlabel", ShowLabel);
    settings.setValue("plot/showglabel", ShowGLabel);
    settings.setValue("plot/showcompass", ShowCompass);
    settings.setValue("plot/showscale", ShowScale);
    settings.setValue("plot/showarrow", ShowArrow);
    settings.setValue("plot/showslip", ShowSlip);
    settings.setValue("plot/showhalfc", ShowHalfC);
    settings.setValue("plot/showerr", ShowErr);
    settings.setValue("plot/showeph", ShowEph);
    settings.setValue("plot/plotstyle", PlotStyle);
    settings.setValue("plot/marksize", MarkSize);
    settings.setValue("plot/navsys", NavSys);
    settings.setValue("plot/animcycle", AnimCycle);
    settings.setValue("plot/refcycle", RefCycle);
    settings.setValue("plot/hidelowsat", HideLowSat);
    settings.setValue("plot/elmaskp", ElMaskP);
    settings.setValue("plot/exsats", ExSats);
    settings.setValue("plot/rtbuffsize", RtBuffSize);
    settings.setValue("plot/timesyncout", TimeSyncOut);
    settings.setValue("plot/timesyncport", TimeSyncPort);
    settings.setValue("plot/rtstream1", RtStream[0]);
    settings.setValue("plot/rtstream2", RtStream[1]);
    settings.setValue("plot/rtformat1", RtFormat[0]);
    settings.setValue("plot/rtformat2", RtFormat[1]);
    settings.setValue("plot/rttimeform", RtTimeForm);
    settings.setValue("plot/rtdegform", RtDegForm);
    settings.setValue("plot/rtfieldsep", RtFieldSep);
    settings.setValue("plot/rttimeouttime", RtTimeOutTime);
    settings.setValue("plot/rtreconntime", RtReConnTime);

    settings.setValue("plot/mcolor0", MColor[0][0]);
    settings.setValue("plot/mcolor1", MColor[0][1]);
    settings.setValue("plot/mcolor2", MColor[0][2]);
    settings.setValue("plot/mcolor3", MColor[0][3]);
    settings.setValue("plot/mcolor4", MColor[0][4]);
    settings.setValue("plot/mcolor5", MColor[0][5]);
    settings.setValue("plot/mcolor6", MColor[0][6]);
    settings.setValue("plot/mcolor7", MColor[0][7]);
    settings.setValue("plot/mcolor8", MColor[0][0]);
    settings.setValue("plot/mcolor9", MColor[1][1]);
    settings.setValue("plot/mcolor10", MColor[1][2]);
    settings.setValue("plot/mcolor11", MColor[1][3]);
    settings.setValue("plot/mcolor12", MColor[1][4]);
    settings.setValue("plot/mcolor13", MColor[1][5]);
    settings.setValue("plot/mcolor14", MColor[1][6]);
    settings.setValue("plot/mcolor15", MColor[1][7]);
    settings.setValue("plot/mapcolor1", MapColor[0]);
    settings.setValue("plot/mapcolor2", MapColor[1]);
    settings.setValue("plot/mapcolor3", MapColor[2]);
    settings.setValue("plot/mapcolor4", MapColor[3]);
    settings.setValue("plot/mapcolor5", MapColor[4]);
    settings.setValue("plot/mapcolor6", MapColor[5]);
    settings.setValue("plot/mapcolor7", MapColor[6]);
    settings.setValue("plot/mapcolor8", MapColor[7]);
    settings.setValue("plot/mapcolor9", MapColor[8]);
    settings.setValue("plot/mapcolor10", MapColor[9]);
    settings.setValue("plot/mapcolor11", MapColor[10]);
    settings.setValue("plot/mapcolor12", MapColor[11]);
    settings.setValue("plot/mapcolorf1", MapColorF[0]);
    settings.setValue("plot/mapcolorf2", MapColorF[1]);
    settings.setValue("plot/mapcolorf3", MapColorF[2]);
    settings.setValue("plot/mapcolorf4", MapColorF[3]);
    settings.setValue("plot/mapcolorf5", MapColorF[4]);
    settings.setValue("plot/mapcolorf6", MapColorF[5]);
    settings.setValue("plot/mapcolorf7", MapColorF[6]);
    settings.setValue("plot/mapcolorf8", MapColorF[7]);
    settings.setValue("plot/mapcolorf9", MapColorF[8]);
    settings.setValue("plot/mapcolorf10", MapColorF[9]);
    settings.setValue("plot/mapcolorf11", MapColorF[10]);
    settings.setValue("plot/mapcolorf12", MapColorF[11]);
    settings.setValue("plot/color1", CColor[0]);
    settings.setValue("plot/color2", CColor[1]);
    settings.setValue("plot/color3", CColor[2]);
    settings.setValue("plot/color4", CColor[3]);

    settings.setValue("plot/staposfile", plotOptDialog->refDialog->stationPositionFile);

    settings.setValue("plot/elmask", ElMask);
    settings.setValue("plot/maxdop", MaxDop);
    settings.setValue("plot/maxmp", MaxMP);
    settings.setValue("plot/yrange", YRange);
    settings.setValue("plot/orgin", Origin);
    settings.setValue("plot/rcvpos", RcvPos);
    settings.setValue("plot/oopos1", OOPos[0]);
    settings.setValue("plot/oopos2", OOPos[1]);
    settings.setValue("plot/oopos3", OOPos[2]);
    settings.setValue("plot/shapefile", ShapeFile);
    settings.setValue("plot/tlefile", TLEFile);
    settings.setValue("plot/tlesatfile", TLESatFile);

    settings.setValue("plot/fontname", Font.family());
    settings.setValue("plot/fontsize", Font.pointSize());

    settings.setValue("plot/rnxopts", RnxOpts);
    settings.setValue("mapview/apikey", ApiKey);
    settings.setValue("mapview/mapapi", MapApi);
     for (int i=0;i<6;i++) for (int j=0;j<3;j++) {
         settings.setValue(QString("mapview/mapstrs_%1_%2").arg(i).arg(j),MapStrs[i][j]);
     }

    for (int i = 0; i < 2; i++) {
        settings.setValue(QString("str/strcmd1_%1").arg(i), StrCmds  [0][i]);
        settings.setValue(QString("str/strcmd2_%1").arg(i), StrCmds  [1][i]);
        settings.setValue(QString("str/strcmdena1_%1").arg(i), StrCmdEna[0][i]);
        settings.setValue(QString("str/strcmdena2_%1").arg(i), StrCmdEna[1][i]);
    }
    for (int i = 0; i < 3; i++) {
        settings.setValue(QString("str/strpath1_%1").arg(i), StrPaths[0][i]);
        settings.setValue(QString("str/strpath2_%1").arg(i), StrPaths[1][i]);
    }
    for (int i = 0; i < 10; i++) {
        settings.setValue(QString("str/strhistry_%1").arg(i), StrHistory [i]);
    }

    settings.setValue("viewer/color1", TextViewer::colorText);
    settings.setValue("viewer/color2", TextViewer::colorBackground);
    settings.setValue("viewer/fontname", TextViewer::font.family());
    settings.setValue("viewer/fontsize", TextViewer::font.pointSize());

    settings.setValue("solbrows/dir", fileSelDialog->Dir);
    settings.setValue("solbrows/split1", BrowseSplitter->saveState());
}

//---------------------------------------------------------------------------
void Plot::DriveSelChanged()
{
    DirSelector->setVisible(false);

    DirSelector->setRootIndex(dirModel->index(DriveSel->currentText()));
    DirSelected->setText(DriveSel->currentText());
}
//---------------------------------------------------------------------------
void Plot::BtnDirSelClick()
{
#ifdef FLOATING_DIRSELECTOR
    QPoint pos = Panel5->mapToGlobal(BtnDirSel->pos());
    pos.rx() += BtnDirSel->width() - DirSelector->width();
    pos.ry() += BtnDirSel->height();

    DirSelector->move(pos);
#endif
    DirSelector->setVisible(!DirSelector->isVisible());
}
//---------------------------------------------------------------------------
void Plot::DirSelChange(QModelIndex index)
{
    DirSelector->expand(index);

    Dir = dirModel->filePath(index);
    DirSelected->setText(Dir);
    fileModel->setRootPath(Dir);
    FileList->setRootIndex(fileModel->index(Dir));
}
//---------------------------------------------------------------------------
void Plot::DirSelSelected(QModelIndex)
{
    DirSelector->setVisible(false);
}
//---------------------------------------------------------------------------
void Plot::FileListClick(QModelIndex index)
{
    QStringList file;

    file.append(fileModel->filePath(index));
    plot->ReadSol(file, 0);

    DirSelector->setVisible(false);
}
//---------------------------------------------------------------------------
void Plot::FilterClick()
{
    QString filter = Filter->currentText();

    // only keep data between brackets
    filter = filter.mid(filter.indexOf("(") + 1);
    filter.remove(")");

    fileModel->setNameFilters(filter.split(" "));
    DirSelector->setVisible(false);
}
//---------------------------------------------------------------------------
void Plot::BtnFreqClick()
{
    freqDialog->exec();
}
//---------------------------------------------------------------------------
