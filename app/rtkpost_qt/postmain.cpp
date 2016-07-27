//---------------------------------------------------------------------------
// rtkpost_qt : post-processing analysis
//
//          Copyright (C) 2007-2012 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtkpost [-t title][-i file][-r file][-b file][-n file ...]
//                   [-d dir][-o file]
//                   [-ts y/m/d h:m:s][-te y/m/d h:m:s][-ti tint][-tu tunit]
//
//           -t title   window title
//           -i file    ini file path
//           -r file    rinex obs rover file
//           -b file    rinex obs base station file
//           -n file    rinex nav/clk, sp3, ionex or sp3 file
//           -d dir     output directory
//           -o file    output file
//           -ts y/m/d h:m:s time start
//           -te y/m/d h:m:s time end
//           -ti tint   time interval (s)
//           -tu tunit  time unit (hr)
//
// version : $Revision: 1.1 $ $Date: 2008/07/17 22:14:45 $
// history : 2008/07/14  1.0 new
//           2008/11/17  1.1 rtklib 2.1.1
//           2008/04/03  1.2 rtklib 2.3.1
//           2010/07/18  1.3 rtklib 2.4.0
//           2010/09/07  1.3 rtklib 2.4.1
//           2011/04/03  1.4 rtklib 2.4.2
//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <QShowEvent>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QSettings>
#include <QFileDialog>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QtGlobal>
#include <QThread>
#include <QMimeData>
#include <QFileSystemModel>
#include <QCompleter>

#include "rtklib.h"
#include "postmain.h"
#include "postopt.h"
#include "kmzconv.h"
#include "refdlg.h"
#include "timedlg.h"
#include "keydlg.h"
#include "aboutdlg.h"
#include "viewer.h"

#define PRGNAME     "RTKPOST-QT"
#define MAXHIST     20
#ifdef Q_OS_WIN
#define GOOGLE_EARTH "C:\\Program Files\\Google\\Google Earth\\googleearth.exe"
#endif
#ifdef Q_OS_LINUX
#define GOOGLE_EARTH "google-earth"
#endif
#ifndef GOOGLE_EARTH
#define GOOGLE_EARTH ""
#endif
static const char version[]="$Revision: 1.1 $ $Date: 2008/07/17 22:14:45 $";

// global variables ---------------------------------------------------------
static gtime_t tstart_={0,0};         // time start for progress-bar
static gtime_t tend_  ={0,0};         // time end for progress-bar

MainForm *mainForm;

extern "C" {

// show message in message area ---------------------------------------------
extern int showmsg(char *format, ...)
{
    va_list arg;
    char buff[1024];
    if (*format) {
        va_start(arg,format);
        vsprintf(buff,format,arg);
        va_end(arg);
        QMetaObject::invokeMethod(mainForm, "ShowMsg",Qt::QueuedConnection,
                                  Q_ARG(QString,QString(buff)));
    }    
    return !mainForm->AbortFlag;
}
// set time span of progress bar --------------------------------------------
extern void settspan(gtime_t ts, gtime_t te)
{
    tstart_=ts;
    tend_  =te;
}
// set current time to show progress ----------------------------------------
extern void settime(gtime_t time)
{
    double tt;
    if (tend_.time!=0&&tstart_.time!=0&&(tt=timediff(tend_,tstart_))>0.0) {
        QMetaObject::invokeMethod(mainForm->Progress,"setValue",
                  Qt::QueuedConnection,
                  Q_ARG(int,(int)(timediff(time,tstart_)/tt*100.0+0.5)));
    }
}

} // extern "C"


ProcessingThread::ProcessingThread(QObject *parent):QThread(parent)
{
    n=stat=0;
    prcopt=prcopt_default;
    solopt=solopt_default;
    ts.time=ts.sec=0;
    te.time=te.sec=0;
    ti=tu=0;
    rov=base=NULL;
    for (int i=0;i<6;i++) {infile[i]=new char[1024];infile[i][0]='\0';};
    outfile[0]='\0';

    memset(&prcopt,0,sizeof(prcopt_t));
    memset(&solopt,0,sizeof(solopt_t));
    memset(&filopt,0,sizeof(filopt_t));
}
ProcessingThread::~ProcessingThread()
{
    for (int i=0;i<6;i++) delete infile[i];
    if (rov) delete rov;
    if (base) delete base;
    rov=base=NULL;
}
void ProcessingThread::addInput(const QString & file) {
    if (file!="") {
        strcpy(infile[n++],qPrintable(file));
    }
}
void ProcessingThread::addList(char * &sta, const QString & list) {
    char *r;
    sta =new char [list.length()+1 +1];

    QStringList lines=list.split("\n");

    r=sta;
    foreach(QString line,lines)
    {
        int index=line.indexOf("#");
        if (index==-1) index=line.length();
        strcpy(r,qPrintable(line.mid(0,index)));
        r+=index;
        strcpy(r++," ");
    }
}
void ProcessingThread::run()
{
    if ((stat=postpos(ts,te,ti,tu,&prcopt,&solopt,&filopt,infile,n,outfile,
                      rov,base))==1) {
        showmsg("aborted");
    };
    emit done(stat);
}
// constructor --------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    mainForm=this;

    setWindowIcon(QIcon(":/icons/rktpost_Icon.ico"));
    setlocale(LC_NUMERIC,"C");
    int i;
    
    QString file=QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile=fi.absolutePath()+"/"+fi.baseName()+".ini";
    
    DynamicModel=IonoOpt=TropOpt=RovAntPcv=RefAntPcv=AmbRes=0;
    RovPosType=RefPosType=0;
    OutCntResetAmb=5; LockCntFixAmb=5; FixCntHoldAmb=10;
    MaxAgeDiff=30.0; RejectThres=30.0; RejectGdop=30.0;
    MeasErrR1=MeasErrR2=100.0; MeasErr2=0.004; MeasErr3=0.003; MeasErr4=1.0;
    SatClkStab=1E-11; ValidThresAR=3.0;
    RovAntE=RovAntN=RovAntU=RefAntE=RefAntN=RefAntU=0.0;

    for (i=0;i<3;i++) RovPos[i]=0.0;
    for (i=0;i<3;i++) RefPos[i]=0.0;

    Progress->setVisible(false);
    setAcceptDrops(true);

    optDialog= new OptDialog(this);
    convDialog=new ConvDialog(this);
    textViewer=new TextViewer(this);

    QCompleter *fileCompleter=new QCompleter(this);
    QFileSystemModel *fileModel=new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    InputFile1->setCompleter(fileCompleter);
    InputFile2->setCompleter(fileCompleter);
    InputFile3->setCompleter(fileCompleter);
    InputFile4->setCompleter(fileCompleter);
    InputFile5->setCompleter(fileCompleter);
    InputFile6->setCompleter(fileCompleter);
    OutputFile->setCompleter(fileCompleter);

    QCompleter *dirCompleter=new QCompleter(this);
    QFileSystemModel *dirModel=new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs|QDir::Drives|QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    OutDir->setCompleter(dirCompleter);


    connect(BtnPlot,SIGNAL(clicked(bool)),this,SLOT(BtnPlotClick()));
    connect(BtnView,SIGNAL(clicked(bool)),this,SLOT(BtnViewClick()));
    connect(BtnToKML,SIGNAL(clicked(bool)),this,SLOT(BtnToKMLClick()));
    connect(BtnOption,SIGNAL(clicked(bool)),this,SLOT(BtnOptionClick()));
    connect(BtnExec,SIGNAL(clicked(bool)),this,SLOT(BtnExecClick()));
    connect(BtnAbort,SIGNAL(clicked(bool)),this,SLOT(BtnAbortClick()));
    connect(BtnExit,SIGNAL(clicked(bool)),this,SLOT(BtnExitClick()));
    connect(BtnAbout,SIGNAL(clicked(bool)),this,SLOT(BtnAboutClick()));
    connect(BtnTime1,SIGNAL(clicked(bool)),this,SLOT(BtnTime1Click()));
    connect(BtnTime2,SIGNAL(clicked(bool)),this,SLOT(BtnTime2Click()));
    connect(BtnInputFile1,SIGNAL(clicked(bool)),this,SLOT(BtnInputFile1Click()));
    connect(BtnInputFile2,SIGNAL(clicked(bool)),this,SLOT(BtnInputFile2Click()));
    connect(BtnInputFile3,SIGNAL(clicked(bool)),this,SLOT(BtnInputFile3Click()));
    connect(BtnInputFile4,SIGNAL(clicked(bool)),this,SLOT(BtnInputFile4Click()));
    connect(BtnInputFile5,SIGNAL(clicked(bool)),this,SLOT(BtnInputFile5Click()));
    connect(BtnInputFile6,SIGNAL(clicked(bool)),this,SLOT(BtnInputFile6Click()));
    connect(BtnOutputFile,SIGNAL(clicked(bool)),this,SLOT(BtnOutputFileClick()));
    connect(BtnInputView1,SIGNAL(clicked(bool)),this,SLOT(BtnInputView1Click()));
    connect(BtnInputView2,SIGNAL(clicked(bool)),this,SLOT(BtnInputView2Click()));
    connect(BtnInputView3,SIGNAL(clicked(bool)),this,SLOT(BtnInputView3Click()));
    connect(BtnInputView4,SIGNAL(clicked(bool)),this,SLOT(BtnInputView4Click()));
    connect(BtnInputView5,SIGNAL(clicked(bool)),this,SLOT(BtnInputView5Click()));
    connect(BtnInputView6,SIGNAL(clicked(bool)),this,SLOT(BtnInputView6Click()));
    connect(BtnOutputView1,SIGNAL(clicked(bool)),this,SLOT(BtnOutputView1Click()));
    connect(BtnOutputView2,SIGNAL(clicked(bool)),this,SLOT(BtnOutputView2Click()));
    connect(BtnInputPlot1,SIGNAL(clicked(bool)),this,SLOT(BtnInputPlot1Click()));
    connect(BtnInputPlot2,SIGNAL(clicked(bool)),this,SLOT(BtnInputPlot2Click()));
    connect(BtnKeyword,SIGNAL(clicked(bool)),this,SLOT(BtnKeywordClick()));
    connect(TimeStart,SIGNAL(clicked(bool)),this,SLOT(TimeStartClick()));
    connect(TimeEnd,SIGNAL(clicked(bool)),this,SLOT(TimeEndClick()));
    connect(TimeIntF,SIGNAL(clicked(bool)),this,SLOT(TimeIntFClick()));
    connect(TimeUnitF,SIGNAL(clicked(bool)),this,SLOT(TimeUnitFClick()));
    connect(InputFile1,SIGNAL(currentIndexChanged(int)),this,SLOT(InputFile1Change()));
    connect(OutDirEna,SIGNAL(clicked(bool)),this,SLOT(OutDirEnaClick()));
    connect(OutDir,SIGNAL(editingFinished()),this,SLOT(OutDirChange()));
    connect(BtnOutDir,SIGNAL(clicked(bool)),this,SLOT(BtnOutDirClick()));

    QTimer::singleShot(0,this,SLOT(FormCreate()));
}
// callback on form create --------------------------------------------------
void MainForm::FormCreate()
{
    setWindowTitle(QString("%1 ver.%2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    
}
// callback on form show ----------------------------------------------------
void MainForm::showEvent(QShowEvent* event)
{
    if (event->spontaneous()) return;

    QComboBox *ifile[]={InputFile3,InputFile4,InputFile5,InputFile6};
    int inputflag=0;

#ifdef QT5
    QCommandLineParser parser;
    parser.setApplicationDescription("RTK post");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption iniFileOption(QStringList() << "i" ,
            QCoreApplication::translate("main", "use init file <file>"));
    parser.addOption(iniFileOption);

    QCommandLineOption titleOption(QStringList() << "t",
            QCoreApplication::translate("main", "use window tile <title>"),
            QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    QCommandLineOption roverOption(QStringList() << "r",
            QCoreApplication::translate("main", "rinex obs rover <file>"),
            QCoreApplication::translate("main", "file"));
    parser.addOption(roverOption);

    QCommandLineOption baseStationOption(QStringList() << "b",
            QCoreApplication::translate("main", "rinex obs base station <path>"),
            QCoreApplication::translate("main", "file"));
    parser.addOption(baseStationOption);

    QCommandLineOption navFileOption(QStringList() << "n" << "file",
            QCoreApplication::translate("main", "rinex nav/clk, sp3, ionex or sp3 <file>"),
            QCoreApplication::translate("main", "file"));
    parser.addOption(navFileOption);

    QCommandLineOption outputOption(QStringList() << "o",
            QCoreApplication::translate("main", "output file <file>"),
            QCoreApplication::translate("main", "file"));
    parser.addOption(outputOption);

    QCommandLineOption outputDirOption(QStringList() << "d",
            QCoreApplication::translate("main", "output directory <dir>"),
            QCoreApplication::translate("main", "dir"));
    parser.addOption(outputDirOption);

    QCommandLineOption timeStartOption(QStringList() << "ts",
            QCoreApplication::translate("main", "time start"),
            QCoreApplication::translate("main", "yyyy/mm/dd hh:mm:ss"));
    parser.addOption(timeStartOption);

    QCommandLineOption timeEndOption(QStringList() << "ts",
            QCoreApplication::translate("main", "time end"),
            QCoreApplication::translate("main", "yyyy/mm/dd hh:mm:ss"));
    parser.addOption(timeEndOption);

    QCommandLineOption timeIntervalOption(QStringList() << "ti",
            QCoreApplication::translate("main", "time interval (s)"),
            QCoreApplication::translate("main", "time"));
    parser.addOption(timeIntervalOption);

    QCommandLineOption timeUnitOption(QStringList() << "tu",
            QCoreApplication::translate("main", "time unit (hr)"),
            QCoreApplication::translate("main", "unit"));
    parser.addOption(timeUnitOption);

    parser.process(*QApplication::instance());

    if (parser.isSet(iniFileOption)) {
        IniFile=parser.value(iniFileOption);
    }
    LoadOpt();

    if (parser.isSet(titleOption)) {
        setWindowTitle(parser.value(titleOption));
    }
    if (parser.isSet(roverOption)) {
        InputFile1->setCurrentText(parser.value(roverOption));
        inputflag=1;
    };
    if (parser.isSet(baseStationOption)) {
        InputFile2->setCurrentText(parser.value(baseStationOption));
    }
    if (parser.isSet(navFileOption)) {
        QStringList files=parser.values(navFileOption);
        for (int n=0;n<files.size()&&n<4;n++)
            ifile[n]->setCurrentText(files.at(n));
    }
    if (parser.isSet(outputOption)) {
        OutputFile->setCurrentText(parser.value(outputOption));
    }
    if (parser.isSet(outputDirOption)) {
        OutDirEna->setChecked(true);
        OutDir->setText(parser.value(outputDirOption));
    }
    if (parser.isSet(timeStartOption)) {
        TimeStart->setChecked(true);
        QStringList dateTime=parser.value(timeStartOption).split(" ");
        if (dateTime.size()!=2) {
            TimeY1->setDate(QDate::fromString(dateTime.at(0),"yyyy/MM/dd"));
            TimeH1->setTime(QTime::fromString(dateTime.at(0),"hh:mm:ss"));
        };
    }
    if (parser.isSet(timeEndOption)) {
        TimeEnd->setChecked(true);
        QStringList dateTime=parser.value(timeEndOption).split(" ");
        if (dateTime.size()!=2) {
            TimeY2->setDate(QDate::fromString(dateTime.at(0),"yyyy/MM/dd"));
            TimeH2->setTime(QTime::fromString(dateTime.at(0),"hh:mm:ss"));
        };
    }
    if (parser.isSet(timeIntervalOption)) {
        TimeIntF->setChecked(true);
        TimeInt->setCurrentText(parser.value(timeIntervalOption));
    }
    if (parser.isSet(timeUnitOption)) {
        TimeUnitF->setChecked(true);
        TimeUnit->setText(parser.value(timeUnitOption));
    }
#endif /*TODO: alternative for QT4 */

    LoadOpt();

    if (inputflag) SetOutFile();
    
    UpdateEnable();
}
// callback on form close ---------------------------------------------------
void MainForm::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous()) return;

    SaveOpt();
}
// callback on drop files ---------------------------------------------------
void  MainForm::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
void  MainForm::dropEvent(QDropEvent *event)
{
    QPoint point=event->pos();
    int top;
    
    if (!event->mimeData()->hasFormat("text/uri-list")) return;

    QString file=event->mimeData()->text();

    top=Panel1->pos().y()+Panel4->pos().y();
    if (point.y()<=top+InputFile1->pos().y()+InputFile1->height()) {
        InputFile1->setCurrentText(file);
        SetOutFile();
    }
    else if (point.y()<=top+InputFile2->pos().y()+InputFile2->height()) {
        InputFile2->setCurrentText(file);
    }
    else if (point.y()<=top+InputFile3->pos().y()+InputFile3->height()) {
        InputFile3->setCurrentText(file);
    }
    else if (point.y()<=top+InputFile4->pos().y()+InputFile4->height()) {
        InputFile4->setCurrentText(file);
    }
    else if (point.y()<=top+InputFile5->pos().y()+InputFile5->height()) {
        InputFile5->setCurrentText(file);
    }
    else if (point.y()<=top+InputFile6->pos().y()+InputFile6->height()) {
        InputFile6->setCurrentText(file);
    }
}
// callback on button-plot --------------------------------------------------
void MainForm::BtnPlotClick()
{
    QString OutputFile_Text=OutputFile->currentText();
    QString file=FilePath(OutputFile_Text);
    QString cmd1="rtkplot_qt",cmd2="../../../bin/rtkplot_qt",opts="";
    
    opts+=" \""+file+"\"";
    
    if (!ExecCmd(cmd1+opts,1)&&!ExecCmd(cmd2+opts,1)) {
        ShowMsg("error : rtkplot_qt execution");
    }
}
// callback on button-view --------------------------------------------------
void MainForm::BtnViewClick()
{
    QString OutputFile_Text=OutputFile->currentText();
    ViewFile(FilePath(OutputFile_Text));
}
// callback on button-to-kml ------------------------------------------------
void MainForm::BtnToKMLClick()
{
    QString OutputFile_Text=OutputFile->currentText();
    convDialog->SetInput(FilePath(OutputFile_Text));
    convDialog->exec();

}
// callback on button-options -----------------------------------------------
void MainForm::BtnOptionClick()
{
    int format=SolFormat;
    optDialog->exec();
    if (optDialog->result()!=QDialog::Accepted) return;
    if ((format==SOLF_NMEA)!=(SolFormat==SOLF_NMEA)) {
        SetOutFile();
    }
    UpdateEnable();
}
// callback on button-execute -----------------------------------------------
void MainForm::BtnExecClick()
{
    QString OutputFile_Text=OutputFile->currentText();
    
    if (InputFile1->currentText()=="") {
        showmsg("error : no rinex obs file (rover)");
        return;
    }
    if (InputFile2->currentText()==""&&PMODE_DGPS<=PosMode&&PosMode<=PMODE_FIXED) {
        showmsg("error : no rinex obs file (base station)");
        return;
    }
    if (OutputFile->currentText()=="") {
        showmsg("error : no output file");
        return;
    }
    if (OutputFile_Text.contains(".obs",Qt::CaseInsensitive)||
        OutputFile_Text.contains(".nav",Qt::CaseInsensitive)||
        OutputFile_Text.contains(".gnav",Qt::CaseInsensitive)||
        OutputFile_Text.contains(".gz",Qt::CaseInsensitive)||
        OutputFile_Text.contains(".Z",Qt::CaseInsensitive)||
        OutputFile_Text.contains(QRegExp(".??o",Qt::CaseInsensitive))||
        OutputFile_Text.contains(QRegExp(".??d",Qt::CaseInsensitive))||
        OutputFile_Text.contains(QRegExp(".??n",Qt::CaseInsensitive))||
        OutputFile_Text.contains(QRegExp(".??g",Qt::CaseInsensitive))){
        showmsg("error : invalid extension of output file (%s)",qPrintable(OutputFile_Text));
        return;
    }
    showmsg("");
    BtnAbort ->setVisible(true);
    BtnExec  ->setVisible(false);
    BtnExit  ->setEnabled(false);
    BtnView  ->setEnabled(false);
    BtnToKML ->setEnabled(false);
    BtnPlot  ->setEnabled(false);
    BtnOption->setEnabled(false);
    Panel1   ->setEnabled(false);
    
    ExecProc();
}
// callback on prcoessing finished-------------------------------------------
void MainForm::ProcessingFinished(int stat)
{
    setCursor(Qt::ArrowCursor);
    Progress->setVisible(false);

    if (stat>=0) {
        AddHist(InputFile1);
        AddHist(InputFile2);
        AddHist(InputFile3);
        AddHist(InputFile4);
        AddHist(InputFile5);
        AddHist(InputFile6);
        AddHist(OutputFile);
    }

    if (Message->text().contains("processing")) {
        showmsg("done");
    }
    BtnAbort ->setVisible(false);
    BtnExec  ->setVisible(true);
    BtnExec  ->setEnabled(true);
    BtnExit  ->setEnabled(true);
    BtnView  ->setEnabled(true);
    BtnToKML ->setEnabled(true);
    BtnPlot  ->setEnabled(true);
    BtnOption->setEnabled(true);
    Panel1   ->setEnabled(true);
}
// callback on button-abort -------------------------------------------------
void MainForm::BtnAbortClick()
{
    AbortFlag=1;
    showmsg("aborted");
}
// callback on button-exit --------------------------------------------------
void MainForm::BtnExitClick()
{
    close();
}
// callback on button-about -------------------------------------------------
void MainForm::BtnAboutClick()
{
    QString prog=PRGNAME;
    AboutDialog *aboutDialog=new AboutDialog(this);

    aboutDialog->About=prog;
    aboutDialog->IconIndex=1;
    aboutDialog->exec();

    delete aboutDialog;
}
// callback on button-time-1 ------------------------------------------------
void MainForm::BtnTime1Click()
{
    TimeDialog *timeDialog=new TimeDialog(this);
    timeDialog->Time=GetTime1();
    timeDialog->exec();

    delete timeDialog;
}
// callback on button-time-2 ------------------------------------------------
void MainForm::BtnTime2Click()
{
    TimeDialog *timeDialog=new TimeDialog(this);

    timeDialog->Time=GetTime2();
    timeDialog->exec();

    delete timeDialog;
}
// callback on button-inputfile-1 -------------------------------------------
void MainForm::BtnInputFile1Click()
{
    InputFile1->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("RINEX OBS (Rover) File"),InputFile1->currentText(),tr("All (*.*);;RINEX OBS (*.obs *.*O *.*D)"))));
    SetOutFile();
}
// callback on button-inputfile-2 -------------------------------------------
void MainForm::BtnInputFile2Click()
{
    InputFile2->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("RINEX OBS (Base Station) File"),InputFile2->currentText(),tr("All (*.*);;RINEX OBS (*.obs *.*O *.*D)"))));
}
// callback on button-inputfile-3 -------------------------------------------
void MainForm::BtnInputFile3Click()
{
    InputFile3->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File"),InputFile3->currentText(),tr("All (*.*);;RINEX NAV (*.*nav *.*N *.*P *.*G *.*H *.*Q)"))));
}
// callback on button-inputfile-4 -------------------------------------------
void MainForm::BtnInputFile4Click()
{
    InputFile4->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File"),InputFile4->currentText(),tr("All (*.*);;Precise Ephemeris/Clock (*.sp3 *.eph* *.clk*)"))));
}
// callback on button-inputfile-5 -------------------------------------------
void MainForm::BtnInputFile5Click()
{
    InputFile5->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File"),InputFile5->currentText(),tr("All (*.*);;Precise Ephemeris/Clock (*.sp3 *.eph* *.clk*)"))));
}
// callback on button-inputfile-6 -------------------------------------------
void MainForm::BtnInputFile6Click()
{
    InputFile6->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("RINEX NAV/CLK,SP3,FCB,IONEX or SBAS/EMS File"),InputFile6->currentText(),tr("All (*.*);;FCB (*.fcb),IONEX (*.*i *.ionex),SBAS (*.sbs *.ems)"))));
}
// callback on button-outputfile --------------------------------------------
void MainForm::BtnOutputFileClick()
{
    OutputFile->setCurrentText(QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Output File"),OutputFile->currentText(),tr("All (*.*);;Position Files (*.pos)"))));
}
// callback on button-inputview-1 -------------------------------------------
void MainForm::BtnInputView1Click()
{
    QString InputFile1_Text=InputFile1->currentText();
    ViewFile(FilePath(InputFile1_Text));
}
// callback on button-inputview-2 -------------------------------------------
void MainForm::BtnInputView2Click()
{
    QString InputFile2_Text=InputFile2->currentText();
    ViewFile(FilePath(InputFile2_Text));
}
// callback on button-inputview-3 -------------------------------------------
void MainForm::BtnInputView3Click()
{
    QString InputFile1_Text=InputFile1->currentText();
    QString InputFile3_Text=InputFile3->currentText();
    QString file=FilePath(InputFile3_Text);
    QString f;
    
    if (file=="") {
        file=FilePath(InputFile1_Text);
        if (!ObsToNav(file,f)) return;
        file=f;
    }
    ViewFile(file);
}
// callback on button-inputview-4 -------------------------------------------
void MainForm::BtnInputView4Click()
{
    QString InputFile4_Text=InputFile4->currentText();
    ViewFile(FilePath(InputFile4_Text));
}
// callback on button-inputview-5 -------------------------------------------
void MainForm::BtnInputView5Click()
{
    QString InputFile5_Text=InputFile5->currentText();
    ViewFile(FilePath(InputFile5_Text));
}
// callback on button-inputview-6 -------------------------------------------
void MainForm::BtnInputView6Click()
{
    QString InputFile6_Text=InputFile6->currentText();
    ViewFile(FilePath(InputFile6_Text));
}
// callback on button-outputview-1 ------------------------------------------
void MainForm::BtnOutputView1Click()
{
    QString OutputFile_Text=OutputFile->currentText();
    QString file=FilePath(OutputFile_Text)+".stat";
    if (!QFile::exists(file)) return;
    ViewFile(file);
}
// callback on button-outputview-2 ------------------------------------------
void MainForm::BtnOutputView2Click()
{
    QString OutputFile_Text=OutputFile->currentText();
    QString file=FilePath(OutputFile_Text)+".trace";
    if (!QFile::exists(file)) return;
    ViewFile(file);
}
// callback on button-inputplot-1 -------------------------------------------
void MainForm::BtnInputPlot1Click()
{
    QString InputFile1_Text=InputFile1->currentText();
    QString InputFile2_Text=InputFile2->currentText();
    QString InputFile3_Text=InputFile3->currentText();
    QString InputFile4_Text=InputFile4->currentText();
    QString InputFile5_Text=InputFile5->currentText();
    QString InputFile6_Text=InputFile6->currentText();
    QString files[6];
    QString cmd1="rtkplot_qt",cmd2="../../../bin/rtkplot_qt",opts="";
    QString navfile;
    
    files[0]=FilePath(InputFile1_Text); /* obs rover */
    files[1]=FilePath(InputFile2_Text); /* obs base */
    files[2]=FilePath(InputFile3_Text);
    files[3]=FilePath(InputFile4_Text);
    files[4]=FilePath(InputFile5_Text);
    files[5]=FilePath(InputFile6_Text);
    
    if (files[2]=="") {
        if (ObsToNav(files[0],navfile)) files[2]=navfile;
    }
    opts=" -r \""+files[0]+"\" \""+files[2]+"\" \""+files[3]+"\" \""+
        files[4]+"\" \""+files[5]+"\"";
    
    if (!ExecCmd(cmd1+opts,1)&&!ExecCmd(cmd2+opts,1)) {
        ShowMsg("error : rtkplot_qt execution");
    }
}
// callback on button-inputplot-2 -------------------------------------------
void MainForm::BtnInputPlot2Click()
{
    QString InputFile1_Text=InputFile1->currentText();
    QString InputFile2_Text=InputFile2->currentText();
    QString InputFile3_Text=InputFile3->currentText();
    QString InputFile4_Text=InputFile4->currentText();
    QString InputFile5_Text=InputFile5->currentText();
    QString InputFile6_Text=InputFile6->currentText();
    QString files[6];
    QString cmd1="rtkplot_qt",cmd2="../../../bin/rtkplot_qt",opts="";
    QString navfile;
    
    files[0]=FilePath(InputFile1_Text); /* obs rover */
    files[1]=FilePath(InputFile2_Text); /* obs base */
    files[2]=FilePath(InputFile3_Text);
    files[3]=FilePath(InputFile4_Text);
    files[4]=FilePath(InputFile5_Text);
    files[5]=FilePath(InputFile6_Text);
    
    if (files[2]=="") {
        if (ObsToNav(files[0],navfile)) files[2]=navfile;
    }
    opts=" -r \""+files[1]+"\" \""+files[2]+"\" \""+files[3]+"\" \""+
         files[4]+"\" \""+files[5]+"\"";
    
    if (!ExecCmd(cmd1+opts,1)&&!ExecCmd(cmd2+opts,1)) {
        ShowMsg("error : rtkplot_qt execution");
    }
}
// callback on button-output-directory --------------------------------------
void MainForm::BtnOutDirClick()
{
#ifdef TCPP
    QString dir=OutDir->Text;
    if (!SelectDirectory("Output Directory","",dir)) return;
    OutDir->Text=dir;
#else
    OutDir->setText(QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,tr("Output Directory"),OutDir->text())));
#endif
}
// callback on button keyword -----------------------------------------------
void MainForm::BtnKeywordClick()
{
    KeyDialog *keyDialog=new KeyDialog(this);
    keyDialog->Flag=2;
    keyDialog->exec();

    delete keyDialog;
}
// callback on time-start check -----------------------------------------
void MainForm::TimeStartClick()
{
    UpdateEnable();
}
// callback on time-end check -----------------------------------------
void MainForm::TimeEndClick()
{
    UpdateEnable();
}
// callback on time-interval check ------------------------------------------
void MainForm::TimeIntFClick()
{
    UpdateEnable();
}
// callback on time-unit check ----------------------------------------------
void MainForm::TimeUnitFClick()
{
    UpdateEnable();
}
// callback on inputfile-1 change -------------------------------------------
void MainForm::InputFile1Change()
{
    SetOutFile();
}
// callback on output-directory checked -------------------------------------
void MainForm::OutDirEnaClick()
{
	UpdateEnable();
    SetOutFile();
}
// callback on output-directory change --------------------------------------
void MainForm::OutDirChange()
{
    SetOutFile();
}
// set output file path -----------------------------------------------------
void MainForm::SetOutFile(void)
{
    QString InputFile1_Text=InputFile1->currentText();
    QString OutDir_Text=OutDir->text();
    QString ofile,ifile;
    
    if (InputFile1->currentText()=="") return;
    
    ifile=InputFile1_Text;
    
    if (OutDirEna->isChecked()) {
        QFileInfo f(ifile);
        ofile=OutDir_Text+"/"+f.baseName();
    }
    else {
        QFileInfo f(ifile);
        ofile=f.absolutePath()+"/"+f.baseName();
    }
    ofile+=SolFormat==SOLF_NMEA?".nmea":".pos";
    ofile.replace('*','0');

    OutputFile->setCurrentText(QDir::toNativeSeparators(ofile));
}
// execute post-processing --------------------------------------------------
void MainForm::ExecProc(void)
{
    QString InputFile1_Text=InputFile1->currentText(),InputFile2_Text=InputFile2->currentText();
    QString InputFile3_Text=InputFile3->currentText(),InputFile4_Text=InputFile4->currentText();
    QString InputFile5_Text=InputFile5->currentText(),InputFile6_Text=InputFile6->currentText();
    QString OutputFile_Text=OutputFile->currentText();
    QString temp;
    
    ProcessingThread *thread= new ProcessingThread(this);

    // get processing options
    if (TimeStart->isChecked()) thread->ts=GetTime1();
    if (TimeEnd  ->isChecked()) thread->te=GetTime2();
    if (TimeIntF ->isChecked()) thread->ti=TimeInt ->currentText().toDouble();
    if (TimeUnitF->isChecked()) thread->tu=TimeUnit->text().toDouble()*3600.0;

    thread->prcopt=prcopt_default;
    if (!GetOption(thread->prcopt,thread->solopt,thread->filopt)) {ProcessingFinished(0);return;}

    // set input/output files
    
    thread->addInput(InputFile1_Text);
    
    if (PMODE_DGPS<=thread->prcopt.mode&&thread->prcopt.mode<=PMODE_FIXED) {
        thread->addInput(InputFile2_Text);
    }
    if (InputFile3_Text!="") {
        thread->addInput(InputFile3_Text);
    }
    else if (!ObsToNav(InputFile1_Text,temp)) {
        showmsg("error: no navigation data");
        ProcessingFinished(0);
        return;
    } else thread->addInput(temp);

    if (InputFile4_Text!="") {
        thread->addInput(InputFile4_Text);
    }
    if (InputFile5_Text!="") {
        thread->addInput(InputFile5_Text);
    }
    if (InputFile6_Text!="") {
        thread->addInput(InputFile6_Text);
    }
    strcpy(thread->outfile,qPrintable(OutputFile_Text));
    
    // confirm overwrite
    if (!TimeStart->isChecked()||!TimeEnd->isChecked()) {
        if (QFileInfo::exists(thread->outfile)) {
            if (QMessageBox::question(this,tr("Overwrite"),QString(tr("Overwrite existing file %1.")).arg(thread->outfile))!=QMessageBox::Yes) {ProcessingFinished(0);return;}
        }
    }    
    // set rover and base station list
    thread->addList(thread->rov,RovList);
    thread->addList(thread->base,BaseList);

    Progress->setValue(0);
    Progress->setVisible(true);
    showmsg("reading...");

    setCursor(Qt::WaitCursor);
    
    // post processing positioning
    connect(thread,SIGNAL(done(int)),this,SLOT(ProcessingFinished(int)));

    thread->start();
    
    return;
}
// get processing and solution options --------------------------------------
int MainForm::GetOption(prcopt_t &prcopt, solopt_t &solopt,
                                    filopt_t &filopt)
{
    char buff[1024],*p;
    int sat,ex;
    
    // processing options
    prcopt.mode     =PosMode;
    prcopt.soltype  =Solution;
    prcopt.nf       =Freq+1;
    prcopt.navsys   =NavSys;
    prcopt.elmin    =ElMask*D2R;
    prcopt.snrmask  =SnrMask;
    prcopt.sateph   =SatEphem;
    prcopt.modear   =AmbRes;
    prcopt.glomodear=GloAmbRes;
    prcopt.bdsmodear=BdsAmbRes;
    prcopt.maxout   =OutCntResetAmb;
    prcopt.minfix   =FixCntHoldAmb;
    prcopt.minlock  =LockCntFixAmb;
    prcopt.ionoopt  =IonoOpt;
    prcopt.tropopt  =TropOpt;
    prcopt.posopt[0]=PosOpt[0];
    prcopt.posopt[1]=PosOpt[1];
    prcopt.posopt[2]=PosOpt[2];
    prcopt.posopt[3]=PosOpt[3];
    prcopt.posopt[4]=PosOpt[4];
    prcopt.posopt[5]=PosOpt[5];
    prcopt.dynamics =DynamicModel;
    prcopt.tidecorr =TideCorr;
    prcopt.armaxiter=ARIter;
    prcopt.niter    =NumIter;
    prcopt.intpref  =IntpRefObs;
    prcopt.sbassatsel=SbasSat;
    prcopt.eratio[0]=MeasErrR1;
    prcopt.eratio[1]=MeasErrR2;
    prcopt.err[1]   =MeasErr2;
    prcopt.err[2]   =MeasErr3;
    prcopt.err[3]   =MeasErr4;
    prcopt.err[4]   =MeasErr5;
    prcopt.prn[0]   =PrNoise1;
    prcopt.prn[1]   =PrNoise2;
    prcopt.prn[2]   =PrNoise3;
    prcopt.prn[3]   =PrNoise4;
    prcopt.prn[4]   =PrNoise5;
    prcopt.sclkstab =SatClkStab;
    prcopt.thresar[0]=ValidThresAR;
    prcopt.thresar[1]=ThresAR2;
    prcopt.thresar[2]=ThresAR3;
    prcopt.elmaskar =ElMaskAR*D2R;
    prcopt.elmaskhold=ElMaskHold*D2R;
    prcopt.thresslip=SlipThres;
    prcopt.maxtdiff =MaxAgeDiff;
    prcopt.maxgdop  =RejectGdop;
    prcopt.maxinno  =RejectThres;
    if (BaseLineConst) {
        prcopt.baseline[0]=BaseLine[0];
        prcopt.baseline[1]=BaseLine[1];
    }
    else {
        prcopt.baseline[0]=0.0;
        prcopt.baseline[1]=0.0;
    }
    if (PosMode!=PMODE_FIXED&&PosMode!=PMODE_PPP_FIXED) {
        for (int i=0;i<3;i++) prcopt.ru[i]=0.0;
    }
    else if (RovPosType<=2) {
        for (int i=0;i<3;i++) prcopt.ru[i]=RovPos[i];
    }
    else prcopt.rovpos=RovPosType-2; /* 1:single,2:posfile,3:rinex */
    
    if (PosMode==PMODE_SINGLE||PosMode==PMODE_MOVEB) {
        for (int i=0;i<3;i++) prcopt.rb[i]=0.0;
    }
    else if (RefPosType<=2) {
        for (int i=0;i<3;i++) prcopt.rb[i]=RefPos[i];
    }
    else prcopt.refpos=RefPosType-2;
    
    if (RovAntPcv) {
        strcpy(prcopt.anttype[0],qPrintable(RovAnt));
        prcopt.antdel[0][0]=RovAntE;
        prcopt.antdel[0][1]=RovAntN;
        prcopt.antdel[0][2]=RovAntU;
    }
    if (RefAntPcv) {
        strcpy(prcopt.anttype[1],qPrintable(RefAnt));
        prcopt.antdel[1][0]=RefAntE;
        prcopt.antdel[1][1]=RefAntN;
        prcopt.antdel[1][2]=RefAntU;
    }
    if (ExSats!="") { // excluded satellites
        strcpy(buff,qPrintable(ExSats));
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if (*p=='+') {ex=2; p++;} else ex=1;
            if (!(sat=satid2no(p))) continue;
            prcopt.exsats[sat-1]=ex;
        }
    }
    // extended receiver error model option
    prcopt.exterr=ExtErr;
    
    strcpy(prcopt.rnxopt[0],qPrintable(RnxOpts1));
    strcpy(prcopt.rnxopt[1],qPrintable(RnxOpts2));
    strcpy(prcopt.pppopt,qPrintable(PPPOpts));
    
    // solution options
    solopt.posf     =SolFormat;
    solopt.times    =TimeFormat==0?0:TimeFormat-1;
    solopt.timef    =TimeFormat==0?0:1;
    solopt.timeu    =TimeDecimal<=0?0:TimeDecimal;
    solopt.degf     =LatLonFormat;
    solopt.outhead  =OutputHead;
    solopt.outopt   =OutputOpt;
    solopt.datum    =OutputDatum;
    solopt.height   =OutputHeight;
    solopt.geoid    =OutputGeoid;
    solopt.solstatic=SolStatic;
    solopt.sstat    =DebugStatus;
    solopt.trace    =DebugTrace;
    strcpy(solopt.sep,FieldSep!=""?qPrintable(FieldSep):" ");
    strcpy(solopt.prog,qPrintable(QString("%1 ver.%2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL)));
    
    // file options
    strcpy(filopt.satantp,qPrintable(SatPcvFile));
    strcpy(filopt.rcvantp,qPrintable(AntPcvFile));
    strcpy(filopt.stapos, qPrintable(StaPosFile));
    strcpy(filopt.geoid,  qPrintable(GeoidDataFile));
    strcpy(filopt.iono,   qPrintable(IonoFile));
    strcpy(filopt.eop,    qPrintable(EOPFile));
    strcpy(filopt.dcb,    qPrintable(DCBFile));
    strcpy(filopt.blq,    qPrintable(BLQFile));
    
    return 1;
}
// observation file to nav file ---------------------------------------------
int MainForm::ObsToNav(const QString &obsfile, QString &navfile)
{
    int p;
    QFileInfo f(obsfile);
    navfile=f.canonicalPath()+f.completeBaseName();
    QString suffix=f.suffix();

    if (suffix=="") return 0;
    if ((suffix.length()==3)&&(suffix.at(2).toLower()=='o')) suffix[2]='*';
    else if ((suffix.length()==3)&&(suffix.at(2).toLower()=='d')) suffix[2]='*';
    else if (suffix.toLower()=="obs") suffix="*nav";
    else if (((p=suffix.indexOf("gz"))!=-1)||((p=suffix.indexOf('Z'))!=-1)) {
        if (p<1) return 0;
        if (suffix.at(p-1).toLower()=='o') suffix[p-1]='*';
        else if (suffix.at(p-1).toLower()=='d') suffix[p-1]='*';
        else return 0;
    }
    else return 0;
    return 1;
}
// replace file path with keywords ------------------------------------------
QString MainForm::FilePath(const QString &file)
{
    gtime_t ts={0,0};
    int p;
    char rov[256]="",base[256]="",path[1024];
    
    if (TimeStart->isChecked()) ts=GetTime1();
    
    p=0;
    while ((p=RovList.indexOf("\n",p))!=-1){
        if ((p<RovList.count())&&(RovList.at(p)!='#')) break;
    }
    if (p!=-1) strcpy(rov,qPrintable(RovList.mid(p))); else strcpy(rov,qPrintable(RovList));

    p=0;
    while ((p=BaseList.indexOf("\n",p))!=-1){
        if ((p<BaseList.count())&&(BaseList.at(p)!='#')) break;
    }
    if (p!=-1) strcpy(base,qPrintable(BaseList.mid(p))); else strcpy(base,qPrintable(RovList));

    reppath(qPrintable(file),path,ts,rov,base);
    
    return QString(path);
}
// read history -------------------------------------------------------------
void MainForm::ReadList(QComboBox* combo, QSettings *ini, const QString &key)
{
    QString item;
    int i;
    
    for (i=0;i<100;i++) {
        item=ini->value(QString("%1_%2").arg(key).arg(i,3),"").toString();
        if (item!="") combo->addItem(item); else break;
    }
}
// write history ------------------------------------------------------------
void MainForm::WriteList(QSettings *ini, const QString &key, const QComboBox *combo)
{
    int i;
    
    for (i=0;i<combo->count();i++) {
        ini->setValue(QString("%1_%2").arg(key).arg(i,3),combo->itemText(i));
    }
}
// add history --------------------------------------------------------------
void MainForm::AddHist(QComboBox *combo)
{
    QString hist=combo->currentText();
    if (hist=="") return;
    int i=combo->currentIndex();
    if (i>=0) combo->removeItem(i);
    combo->insertItem(0,hist);
    for (int i=combo->count()-1;i>=MAXHIST;i--) combo->removeItem(i);
    combo->setCurrentIndex(0);
}
// execute command ----------------------------------------------------------
int MainForm::ExecCmd(const QString &cmd, int show)
{
    Q_UNUSED(show);
    return QProcess::startDetached(cmd);  /* FIXME: show option not yet supported */
}
// view file ----------------------------------------------------------------
void MainForm::ViewFile(const QString &file)
{
    QString f;
    char tmpfile[1024];
    int cstat;
    
    if (file=="") return;
    cstat=rtk_uncompress(qPrintable(file),tmpfile);
    f=!cstat?file:tmpfile;
    
    textViewer->setWindowTitle(file);
    textViewer->show();
    if (!textViewer->Read(f)) textViewer->close();
    if (cstat==1) remove(tmpfile);
}
// show message in message area ---------------------------------------------
void MainForm::ShowMsg(const QString &msg)
{
    Message->setText(msg);
}
// get time from time-1 -----------------------------------------------------
gtime_t MainForm::GetTime1(void)
{
    QDateTime time(TimeY1->date(),TimeH1->time());

    gtime_t t;
    t.time=time.toTime_t();t.sec=time.time().msec()/1000;

    return t;
}
// get time from time-2 -----------------------------------------------------
gtime_t MainForm::GetTime2(void)
{
    QDateTime time(TimeY2->date(),TimeH2->time());

    gtime_t t;
    t.time=time.toTime_t();t.sec=time.time().msec()/1000;

    return t;
}
// set time to time-1 -------------------------------------------------------
void MainForm::SetTime1(gtime_t time)
{
    QDateTime t=QDateTime::fromTime_t(time.time); t=t.addSecs(time.sec);
    TimeY1->setTime(t.time());
    TimeH1->setDate(t.date());
}
// set time to time-2 -------------------------------------------------------
void MainForm::SetTime2(gtime_t time)
{
    QDateTime t=QDateTime::fromTime_t(time.time); t=t.addSecs(time.sec);
    TimeY2->setTime(t.time());
    TimeH2->setDate(t.date());
}
// update enable/disable of widgets -----------------------------------------
void MainForm::UpdateEnable(void)
{
    bool moder=PMODE_DGPS<=PosMode&&PosMode<=PMODE_FIXED;
    
    LabelInputFile1->setText(moder?tr("RINEX OBS: Rover"):tr("RINEX OBS"));
    InputFile2     ->setEnabled(moder);
    BtnInputFile2  ->setEnabled(moder);
    BtnInputPlot2  ->setEnabled(moder);
    BtnInputView2  ->setEnabled(moder);
    BtnOutputView1 ->setEnabled(DebugStatus>0);
    BtnOutputView2 ->setEnabled(DebugTrace >0);
    LabelInputFile3->setEnabled(moder);
    TimeY1         ->setEnabled(TimeStart->isChecked());
    TimeH1         ->setEnabled(TimeStart->isChecked());
    BtnTime1       ->setEnabled(TimeStart->isChecked());
    TimeY2         ->setEnabled(TimeEnd  ->isChecked());
    TimeH2         ->setEnabled(TimeEnd  ->isChecked());
    BtnTime2       ->setEnabled(TimeEnd  ->isChecked());
    TimeInt        ->setEnabled(TimeIntF ->isChecked());
    LabelTimeInt   ->setEnabled(TimeIntF ->isChecked());
    TimeUnitF      ->setEnabled(TimeStart->isChecked()&&TimeEnd  ->isChecked());
    TimeUnit       ->setEnabled(TimeUnitF->isEnabled()&&TimeUnitF->isChecked());
    LabelTimeUnit  ->setEnabled(TimeUnitF->isEnabled()&&TimeUnitF->isChecked());
    OutDir         ->setEnabled(OutDirEna->isChecked());
    BtnOutDir      ->setEnabled(OutDirEna->isChecked());
    LabelOutDir    ->setEnabled(OutDirEna->isChecked());
}
// load options from ini file -----------------------------------------------
void MainForm::LoadOpt(void)
{
    QSettings ini(IniFile,QSettings::IniFormat);
    
    TimeStart->setChecked(ini.value("set/timestart",   0).toBool());
    TimeEnd->setChecked(ini.value("set/timeend",     0).toBool());
    TimeY1->setDate(ini.value ("set/timey1",      "2000/01/01").toDate());
    TimeH1->setTime(ini.value ("set/timeh1",      "00:00:00").toTime());
    TimeY2->setDate(ini.value ("set/timey2",      "2000/01/01").toDate());
    TimeH2->setTime(ini.value ("set/timeh2",      "00:00:00").toTime());
    TimeIntF ->setChecked(ini.value("set/timeintf",    0).toBool());
    TimeInt->setCurrentText(ini.value ("set/timeint",     "0").toString());
    TimeUnitF->setChecked(ini.value("set/timeunitf",   0).toBool());
    TimeUnit->setText(ini.value ("set/timeunit",    "24").toString());
    InputFile1->setCurrentText(ini.value ("set/inputfile1",  "").toString());
    InputFile2->setCurrentText(ini.value ("set/inputfile2",  "").toString());
    InputFile3->setCurrentText(ini.value ("set/inputfile3",  "").toString());
    InputFile4->setCurrentText(ini.value ("set/inputfile4",  "").toString());
    InputFile5->setCurrentText(ini.value ("set/inputfile5",  "").toString());
    InputFile6->setCurrentText(ini.value ("set/inputfile6",  "").toString());
    OutDirEna->setChecked(ini.value("set/outputdirena", 0).toBool());
    OutDir->setText(ini.value ("set/outputdir",   "").toString());
    OutputFile->setCurrentText(ini.value ("set/outputfile",  "").toString());
    
    ReadList(InputFile1,&ini,"hist/inputfile1");
    ReadList(InputFile2,&ini,"hist/inputfile2");
    ReadList(InputFile3,&ini,"hist/inputfile3");
    ReadList(InputFile4,&ini,"hist/inputfile4");
    ReadList(InputFile5,&ini,"hist/inputfile5");
    ReadList(InputFile6,&ini,"hist/inputfile6");
    ReadList(OutputFile,&ini,"hist/outputfile");
    
    PosMode            =ini.value("opt/posmode",        0).toInt();
    Freq               =ini.value("opt/freq",           1).toInt();
    Solution           =ini.value("opt/solution",       0).toInt();
    ElMask             =ini.value  ("opt/elmask",      15.0).toDouble();
    SnrMask.ena[0]     =ini.value("opt/snrmask_ena1",   0).toInt();
    SnrMask.ena[1]     =ini.value("opt/snrmask_ena2",   0).toInt();
    for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
        SnrMask.mask[i][j]=
            ini.value(QString("opt/snrmask_%1_%2").arg(i+1).arg(j+1),0.0).toDouble();
    }
    IonoOpt            =ini.value("opt/ionoopt",     IONOOPT_BRDC).toInt();
    TropOpt            =ini.value("opt/tropopt",     TROPOPT_SAAS).toInt();
    RcvBiasEst         =ini.value("opt/rcvbiasest",     0).toInt();
    DynamicModel       =ini.value("opt/dynamicmodel",   0).toInt();
    TideCorr           =ini.value("opt/tidecorr",       0).toInt();
    SatEphem           =ini.value("opt/satephem",       0).toInt();
    ExSats             =ini.value ("opt/exsats",        "").toString();
    NavSys             =ini.value("opt/navsys",   SYS_GPS).toInt();
    PosOpt[0]          =ini.value("opt/posopt1",        0).toInt();
    PosOpt[1]          =ini.value("opt/posopt2",        0).toInt();
    PosOpt[2]          =ini.value("opt/posopt3",        0).toInt();
    PosOpt[3]          =ini.value("opt/posopt4",        0).toInt();
    PosOpt[4]          =ini.value("opt/posopt5",        0).toInt();
    PosOpt[5]          =ini.value("opt/posopt6",        0).toInt();
    MapFunc            =ini.value("opt/mapfunc",        0).toInt();
    
    AmbRes             =ini.value("opt/ambres",         1).toInt();
    GloAmbRes          =ini.value("opt/gloambres",      1).toInt();
    BdsAmbRes          =ini.value("opt/bdsambres",      1).toInt();
    ValidThresAR       =ini.value  ("opt/validthresar", 3.0).toDouble();
    ThresAR2           =ini.value  ("opt/thresar2",  0.9999).toDouble();
    ThresAR3           =ini.value  ("opt/thresar3",    0.25).toDouble();
    LockCntFixAmb      =ini.value("opt/lockcntfixamb",  0).toInt();
    FixCntHoldAmb      =ini.value("opt/fixcntholdamb", 10).toInt();
    ElMaskAR           =ini.value  ("opt/elmaskar",     0.0).toDouble();
    ElMaskHold         =ini.value  ("opt/elmaskhold",   0.0).toDouble();
    OutCntResetAmb     =ini.value("opt/outcntresetbias",5).toInt();
    SlipThres          =ini.value  ("opt/slipthres",   0.05).toDouble();
    MaxAgeDiff         =ini.value  ("opt/maxagediff",  30.0).toDouble();
    RejectThres        =ini.value  ("opt/rejectthres", 30.0).toDouble();
    RejectGdop         =ini.value  ("opt/rejectgdop",  30.0).toDouble();
    ARIter             =ini.value("opt/ariter",         1).toInt();
    NumIter            =ini.value("opt/numiter",        1).toInt();
    CodeSmooth         =ini.value("opt/codesmooth",     0).toInt();
    BaseLine[0]        =ini.value  ("opt/baselinelen",  0.0).toDouble();
    BaseLine[1]        =ini.value  ("opt/baselinesig",  0.0).toDouble();
    BaseLineConst      =ini.value("opt/baselineconst",  0).toInt();
    
    SolFormat          =ini.value("opt/solformat",      0).toInt();
    TimeFormat         =ini.value("opt/timeformat",     1).toInt();
    TimeDecimal        =ini.value("opt/timedecimal",    3).toInt();
    LatLonFormat       =ini.value("opt/latlonformat",   0).toInt();
    FieldSep           =ini.value ("opt/fieldsep",      "").toString();
    OutputHead         =ini.value("opt/outputhead",     1).toInt();
    OutputOpt          =ini.value("opt/outputopt",      1).toInt();
    OutputDatum        =ini.value("opt/outputdatum",    0).toInt();
    OutputHeight       =ini.value("opt/outputheight",   0).toInt();
    OutputGeoid        =ini.value("opt/outputgeoid",    0).toInt();
    SolStatic          =ini.value("opt/solstatic",      0).toInt();
    DebugTrace         =ini.value("opt/debugtrace",     0).toInt();
    DebugStatus        =ini.value("opt/debugstatus",    0).toInt();
    
    MeasErrR1          =ini.value  ("opt/measeratio1",100.0).toDouble();
    MeasErrR2          =ini.value  ("opt/measeratio2",100.0).toDouble();
    MeasErr2           =ini.value  ("opt/measerr2",   0.003).toDouble();
    MeasErr3           =ini.value  ("opt/measerr3",   0.003).toDouble();
    MeasErr4           =ini.value  ("opt/measerr4",   0.000).toDouble();
    MeasErr5           =ini.value  ("opt/measerr5",  10.000).toDouble();
    SatClkStab         =ini.value  ("opt/satclkstab", 5E-12).toDouble();
    PrNoise1           =ini.value  ("opt/prnoise1",    1E-4).toDouble();
    PrNoise2           =ini.value  ("opt/prnoise2",    1E-3).toDouble();
    PrNoise3           =ini.value  ("opt/prnoise3",    1E-4).toDouble();
    PrNoise4           =ini.value  ("opt/prnoise4",    1E+1).toDouble();
    PrNoise5           =ini.value  ("opt/prnoise5",    1E+1).toDouble();
    
    RovPosType         =ini.value("opt/rovpostype",     0).toInt();
    RefPosType         =ini.value("opt/refpostype",     0).toInt();
    RovPos[0]          =ini.value  ("opt/rovpos1",      0.0).toDouble();
    RovPos[1]          =ini.value  ("opt/rovpos2",      0.0).toDouble();
    RovPos[2]          =ini.value  ("opt/rovpos3",      0.0).toDouble();
    RefPos[0]          =ini.value  ("opt/refpos1",      0.0).toDouble();
    RefPos[1]          =ini.value  ("opt/refpos2",      0.0).toDouble();
    RefPos[2]          =ini.value  ("opt/refpos3",      0.0).toDouble();
    RovAntPcv          =ini.value("opt/rovantpcv",      0).toInt();
    RefAntPcv          =ini.value("opt/refantpcv",      0).toInt();
    RovAnt             =ini.value ("opt/rovant",        "").toString();
    RefAnt             =ini.value ("opt/refant",        "").toString();
    RovAntE            =ini.value  ("opt/rovante",      0.0).toDouble();
    RovAntN            =ini.value  ("opt/rovantn",      0.0).toDouble();
    RovAntU            =ini.value  ("opt/rovantu",      0.0).toDouble();
    RefAntE            =ini.value  ("opt/refante",      0.0).toDouble();
    RefAntN            =ini.value  ("opt/refantn",      0.0).toDouble();
    RefAntU            =ini.value  ("opt/refantu",      0.0).toDouble();
    
    RnxOpts1           =ini.value ("opt/rnxopts1",      "").toString();
    RnxOpts2           =ini.value ("opt/rnxopts2",      "").toString();
    PPPOpts            =ini.value ("opt/pppopts",       "").toString();
    
    AntPcvFile         =ini.value ("opt/antpcvfile",    "").toString();
    IntpRefObs         =ini.value("opt/intprefobs",     0).toInt();
    SbasSat            =ini.value("opt/sbassat",        0).toInt();
    NetRSCorr          =ini.value("opt/netrscorr",      0).toInt();
    SatClkCorr         =ini.value("opt/satclkcorr",     0).toInt();
    SbasCorr           =ini.value("opt/sbascorr",       0).toInt();
    SbasCorr1          =ini.value("opt/sbascorr1",      0).toInt();
    SbasCorr2          =ini.value("opt/sbascorr2",      0).toInt();
    SbasCorr3          =ini.value("opt/sbascorr3",      0).toInt();
    SbasCorr4          =ini.value("opt/sbascorr4",      0).toInt();
    SbasCorrFile       =ini.value ("opt/sbascorrfile",  "").toString();
    PrecEphFile        =ini.value ("opt/precephfile",   "").toString();
    SatPcvFile         =ini.value ("opt/satpcvfile",    "").toString();
    StaPosFile         =ini.value ("opt/staposfile",    "").toString();
    GeoidDataFile      =ini.value ("opt/geoiddatafile", "").toString();
    IonoFile           =ini.value ("opt/ionofile",      "").toString();
    EOPFile            =ini.value ("opt/eopfile",       "").toString();
    DCBFile            =ini.value ("opt/dcbfile",       "").toString();
    BLQFile            =ini.value ("opt/blqfile",       "").toString();
    GoogleEarthFile    =ini.value ("opt/googleearthfile",GOOGLE_EARTH).toString();
    
    RovList="";
    for (int i=0;i<10;i++) {
        RovList +=ini.value(QString("opt/rovlist%1").arg(i+1),"").toString();
    }
    BaseList="";
    for (int i=0;i<10;i++) {
        BaseList+=ini.value(QString("opt/baselist%1").arg(i+1),"").toString();
    }
    RovList.replace("@@","\n");
    BaseList.replace("@@","\n");

    ExtErr.ena[0]      =ini.value("opt/exterr_ena0",    0).toInt();
    ExtErr.ena[1]      =ini.value("opt/exterr_ena1",    0).toInt();
    ExtErr.ena[2]      =ini.value("opt/exterr_ena2",    0).toInt();
    ExtErr.ena[3]      =ini.value("opt/exterr_ena3",    0).toInt();
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ExtErr.cerr[i][j]=ini.value(QString("opt/exterr_cerr%1%2").arg(i).arg(j),0.3).toDouble();
    }
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ExtErr.perr[i][j]=ini.value(QString("exterr_perr%1%2").arg(i).arg(j),0.003).toDouble();
    }
    ExtErr.gloicb[0]   =ini.value  ("opt/exterr_gloicb0",0.0).toDouble();
    ExtErr.gloicb[1]   =ini.value  ("opt/exterr_gloicb1",0.0).toDouble();
    ExtErr.gpsglob[0]  =ini.value  ("opt/exterr_gpsglob0",0.0).toDouble();
    ExtErr.gpsglob[1]  =ini.value  ("opt/exterr_gpsglob1",0.0).toDouble();
    
    convDialog->TimeSpan  ->setChecked(ini.value("conv/timespan",  0).toInt());
    convDialog->TimeIntF  ->setChecked(ini.value("conv/timeintf",  0).toInt());
    convDialog->TimeY1    ->setDate(ini.value ("conv/timey1","2000/01/01").toDate());
    convDialog->TimeH1    ->setTime(ini.value ("conv/timeh1","00:00:00"  ).toTime());
    convDialog->TimeY2    ->setDate(ini.value ("conv/timey2","2000/01/01").toDate());
    convDialog->TimeH2    ->setTime(ini.value ("conv/timeh2","00:00:00"  ).toTime());
    convDialog->TimeInt   ->setText(ini.value ("conv/timeint", "0").toString());
    convDialog->TrackColor->setCurrentIndex(ini.value("conv/trackcolor",5).toInt());
    convDialog->PointColor->setCurrentIndex(ini.value("conv/pointcolor",5).toInt());
    convDialog->OutputAlt ->setCurrentIndex(ini.value("conv/outputalt", 0).toInt());
    convDialog->OutputTime->setCurrentIndex(ini.value("conv/outputtime",0).toInt());
    convDialog->AddOffset ->setChecked(ini.value("conv/addoffset", 0).toInt());
    convDialog->Offset1   ->setText(ini.value ("conv/offset1", "0").toString());
    convDialog->Offset2   ->setText(ini.value ("conv/offset2", "0").toString());
    convDialog->Offset3   ->setText(ini.value ("conv/offset3", "0").toString());
    convDialog->Compress  ->setChecked(ini.value("conv/compress",  0).toInt());
    convDialog->FormatKML ->setChecked(ini.value("conv/format",    0).toInt());

    textViewer->Color1=ini.value("viewer/color1",QColor(Qt::black)).value<QColor>();
    textViewer->Color2=ini.value("viewer/color2",QColor(Qt::white)).value<QColor>();
    textViewer->FontD.setFamily(ini.value ("viewer/fontname","Courier New").toString());
    textViewer->FontD.setPointSize(ini.value("viewer/fontsize",9).toInt());
}
// save options to ini file -------------------------------------------------
void MainForm::SaveOpt(void)
{
    QSettings ini(IniFile,QSettings::IniFormat);
    
    ini.setValue("set/timestart",   TimeStart ->isChecked()?1:0);
    ini.setValue("set/timeend",     TimeEnd   ->isChecked()?1:0);
    ini.setValue ("set/timey1",      TimeY1    ->text());
    ini.setValue ("set/timeh1",      TimeH1    ->text());
    ini.setValue ("set/timey2",      TimeY2    ->text());
    ini.setValue ("set/timeh2",      TimeH2    ->text());
    ini.setValue("set/timeintf",    TimeIntF  ->isChecked()?1:0);
    ini.setValue ("set/timeint",     TimeInt   ->currentText());
    ini.setValue("set/timeunitf",   TimeUnitF ->isChecked()?1:0);
    ini.setValue ("set/timeunit",    TimeUnit  ->text());
    ini.setValue ("set/inputfile1",  InputFile1->currentText());
    ini.setValue ("set/inputfile2",  InputFile2->currentText());
    ini.setValue ("set/inputfile3",  InputFile3->currentText());
    ini.setValue ("set/inputfile4",  InputFile4->currentText());
    ini.setValue ("set/inputfile5",  InputFile5->currentText());
    ini.setValue ("set/inputfile6",  InputFile6->currentText());
    ini.setValue("set/outputdirena",OutDirEna ->isChecked());
    ini.setValue ("set/outputdir",   OutDir    ->text());
    ini.setValue ("set/outputfile",  OutputFile->currentText());
    
    WriteList(&ini,"hist/inputfile1",     InputFile1);
    WriteList(&ini,"hist/inputfile2",     InputFile2);
    WriteList(&ini,"hist/inputfile3",     InputFile3);
    WriteList(&ini,"hist/inputfile4",     InputFile4);
    WriteList(&ini,"hist/inputfile5",     InputFile5);
    WriteList(&ini,"hist/inputfile6",     InputFile6);
    WriteList(&ini,"hist/outputfile",     OutputFile);
    
    ini.setValue("opt/posmode",     PosMode     );
    ini.setValue("opt/freq",        Freq        );
    ini.setValue("opt/solution",    Solution    );
    ini.setValue  ("opt/elmask",      ElMask      );
    ini.setValue("opt/snrmask_ena1",SnrMask.ena[0]);
    ini.setValue("opt/snrmask_ena2",SnrMask.ena[1]);
    for (int i=0;i<3;i++) for (int j=0;j<9;j++) {
        ini.setValue(QString("opt/snrmask_%1_%2").arg(i+1).arg(j+1),
                        SnrMask.mask[i][j]);
    }
    ini.setValue("opt/ionoopt",     IonoOpt     );
    ini.setValue("opt/tropopt",     TropOpt     );
    ini.setValue("opt/rcvbiasest",  RcvBiasEst  );
    ini.setValue("opt/dynamicmodel",DynamicModel);
    ini.setValue("opt/tidecorr",    TideCorr    );
    ini.setValue("opt/satephem",    SatEphem    );
    ini.setValue ("opt/exsats",      ExSats      );
    ini.setValue("opt/navsys",      NavSys      );
    ini.setValue("opt/posopt1",     PosOpt[0]   );
    ini.setValue("opt/posopt2",     PosOpt[1]   );
    ini.setValue("opt/posopt3",     PosOpt[2]   );
    ini.setValue("opt/posopt4",     PosOpt[3]   );
    ini.setValue("opt/posopt5",     PosOpt[4]   );
    ini.setValue("opt/posopt6",     PosOpt[5]   );
    ini.setValue("opt/mapfunc",     MapFunc     );
    
    ini.setValue("opt/ambres",      AmbRes      );
    ini.setValue("opt/gloambres",   GloAmbRes   );
    ini.setValue("opt/bdsambres",   BdsAmbRes   );
    ini.setValue  ("opt/validthresar",ValidThresAR);
    ini.setValue  ("opt/thresar2",    ThresAR2    );
    ini.setValue  ("opt/thresar3",    ThresAR3    );
    ini.setValue("opt/lockcntfixamb",LockCntFixAmb);
    ini.setValue("opt/fixcntholdamb",FixCntHoldAmb);
    ini.setValue  ("opt/elmaskar",    ElMaskAR    );
    ini.setValue  ("opt/elmaskhold",  ElMaskHold  );
    ini.setValue("opt/outcntresetbias",OutCntResetAmb);
    ini.setValue  ("opt/slipthres",   SlipThres   );
    ini.setValue  ("opt/maxagediff",  MaxAgeDiff  );
    ini.setValue  ("opt/rejectgdop",  RejectGdop  );
    ini.setValue  ("opt/rejectthres", RejectThres );
    ini.setValue("opt/ariter",      ARIter      );
    ini.setValue("opt/numiter",     NumIter     );
    ini.setValue("opt/codesmooth",  CodeSmooth  );
    ini.setValue  ("opt/baselinelen", BaseLine[0] );
    ini.setValue  ("opt/baselinesig", BaseLine[1] );
    ini.setValue("opt/baselineconst",BaseLineConst);
    
    ini.setValue("opt/solformat",   SolFormat   );
    ini.setValue("opt/timeformat",  TimeFormat  );
    ini.setValue("opt/timedecimal", TimeDecimal );
    ini.setValue("opt/latlonformat",LatLonFormat);
    ini.setValue ("opt/fieldsep",    FieldSep    );
    ini.setValue("opt/outputhead",  OutputHead  );
    ini.setValue("opt/outputopt",   OutputOpt   );
    ini.setValue("opt/outputdatum", OutputDatum );
    ini.setValue("opt/outputheight",OutputHeight);
    ini.setValue("opt/outputgeoid", OutputGeoid );
    ini.setValue("opt/solstatic",   SolStatic   );
    ini.setValue("opt/debugtrace",  DebugTrace  );
    ini.setValue("opt/debugstatus", DebugStatus );
    
    ini.setValue  ("opt/measeratio1", MeasErrR1   );
    ini.setValue  ("opt/measeratio2", MeasErrR2   );
    ini.setValue  ("opt/measerr2",    MeasErr2    );
    ini.setValue  ("opt/measerr3",    MeasErr3    );
    ini.setValue  ("opt/measerr4",    MeasErr4    );
    ini.setValue  ("opt/measerr5",    MeasErr5    );
    ini.setValue  ("opt/satclkstab",  SatClkStab  );
    ini.setValue  ("opt/prnoise1",    PrNoise1    );
    ini.setValue  ("opt/prnoise2",    PrNoise2    );
    ini.setValue  ("opt/prnoise3",    PrNoise3    );
    ini.setValue  ("opt/prnoise4",    PrNoise4    );
    ini.setValue  ("opt/prnoise5",    PrNoise5    );
    
    ini.setValue("opt/rovpostype",  RovPosType  );
    ini.setValue("opt/refpostype",  RefPosType  );
    ini.setValue  ("opt/rovpos1",     RovPos[0]   );
    ini.setValue  ("opt/rovpos2",     RovPos[1]   );
    ini.setValue  ("opt/rovpos3",     RovPos[2]   );
    ini.setValue  ("opt/refpos1",     RefPos[0]   );
    ini.setValue  ("opt/refpos2",     RefPos[1]   );
    ini.setValue  ("opt/refpos3",     RefPos[2]   );
    ini.setValue("opt/rovantpcv",   RovAntPcv   );
    ini.setValue("opt/refantpcv",   RefAntPcv   );
    ini.setValue ("opt/rovant",      RovAnt      );
    ini.setValue ("opt/refant",      RefAnt      );
    ini.setValue  ("opt/rovante",     RovAntE     );
    ini.setValue  ("opt/rovantn",     RovAntN     );
    ini.setValue  ("opt/rovantu",     RovAntU     );
    ini.setValue  ("opt/refante",     RefAntE     );
    ini.setValue  ("opt/refantn",     RefAntN     );
    ini.setValue  ("opt/refantu",     RefAntU     );
    
    ini.setValue ("opt/rnxopts1",    RnxOpts1    );
    ini.setValue ("opt/rnxopts2",    RnxOpts2    );
    ini.setValue ("opt/pppopts",     PPPOpts     );
    
    ini.setValue ("opt/antpcvfile",  AntPcvFile  );
    ini.setValue("opt/intprefobs",  IntpRefObs  );
    ini.setValue("opt/sbassat",     SbasSat     );
    ini.setValue("opt/netrscorr",   NetRSCorr   );
    ini.setValue("opt/satclkcorr",  SatClkCorr  );
    ini.setValue("opt/sbascorr",    SbasCorr    );
    ini.setValue("opt/sbascorr1",   SbasCorr1   );
    ini.setValue("opt/sbascorr2",   SbasCorr2   );
    ini.setValue("opt/sbascorr3",   SbasCorr3   );
    ini.setValue("opt/sbascorr4",   SbasCorr4   );
    ini.setValue ("opt/sbascorrfile",SbasCorrFile);
    ini.setValue ("opt/precephfile", PrecEphFile );
    ini.setValue ("opt/satpcvfile",  SatPcvFile  );
    ini.setValue ("opt/staposfile",  StaPosFile  );
    ini.setValue ("opt/geoiddatafile",GeoidDataFile);
    ini.setValue ("opt/ionofile",    IonoFile    );
    ini.setValue ("opt/eopfile",     EOPFile     );
    ini.setValue ("opt/dcbfile",     DCBFile     );
    ini.setValue ("opt/blqfile",     BLQFile     );
    ini.setValue ("opt/googleearthfile",GoogleEarthFile);
    
    RovList.replace("\n","@@");
    for (int i=0;i<10;i++) {
        ini.setValue(QString("opt/rovlist%1").arg(i+1),RovList.mid(i*2000,2000));
    }

    BaseList.replace("\n","@@");
    for (int i=0;i<10;i++) {
        ini.setValue(QString("opt/baselist%1").arg(i+1),BaseList.mid(i*2000,2000));
    }
    ini.setValue("opt/exterr_ena0", ExtErr.ena[0]);
    ini.setValue("opt/exterr_ena1", ExtErr.ena[1]);
    ini.setValue("opt/exterr_ena2", ExtErr.ena[2]);
    ini.setValue("opt/exterr_ena3", ExtErr.ena[3]);
    
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ini.setValue(QString("opt/exterr_cerr%1%2").arg(i).arg(j),ExtErr.cerr[i][j]);
    }
    for (int i=0;i<3;i++) for (int j=0;j<6;j++) {
        ini.setValue(QString("exterr_perr%1%2").arg(i).arg(j),ExtErr.perr[i][j]);
    }
    ini.setValue  ("opt/exterr_gloicb0",ExtErr.gloicb[0]);
    ini.setValue  ("opt/exterr_gloicb1",ExtErr.gloicb[1]);
    ini.setValue  ("opt/exterr_gpsglob0",ExtErr.gpsglob[0]);
    ini.setValue  ("opt/exterr_gpsglob1",ExtErr.gpsglob[1]);
    
    ini.setValue("conv/timespan",   convDialog->TimeSpan  ->isChecked()  );
    ini.setValue ("conv/timey1",     convDialog->TimeY1    ->text()     );
    ini.setValue ("conv/timeh1",     convDialog->TimeH1    ->text()     );
    ini.setValue ("conv/timey2",     convDialog->TimeY2    ->text()     );
    ini.setValue ("conv/timeh2",     convDialog->TimeH2    ->text()     );
    ini.setValue("conv/timeintf",   convDialog->TimeIntF  ->isChecked()  );
    ini.setValue ("conv/timeint",    convDialog->TimeInt   ->text()     );
    ini.setValue("conv/trackcolor", convDialog->TrackColor->currentIndex());
    ini.setValue("conv/pointcolor", convDialog->PointColor->currentIndex());
    ini.setValue("conv/outputalt",  convDialog->OutputAlt ->currentIndex());
    ini.setValue("conv/outputtime", convDialog->OutputTime->currentIndex());
    ini.setValue("conv/addoffset",  convDialog->AddOffset ->isChecked()  );
    ini.setValue ("conv/offset1",    convDialog->Offset1   ->text()     );
    ini.setValue ("conv/offset2",    convDialog->Offset2   ->text()     );
    ini.setValue ("conv/offset3",    convDialog->Offset3   ->text()     );
    ini.setValue("conv/compress",   convDialog->Compress  ->isChecked()  );
    ini.setValue("conv/format",     convDialog->FormatKML ->isChecked()  );

    ini.setValue("viewer/color1",textViewer->Color1  );
    ini.setValue("viewer/color2",textViewer->Color2  );
    ini.setValue("viewer/fontname",textViewer->FontD.family());
    ini.setValue("viewer/fontsize",textViewer->FontD.pointSize());
}
//---------------------------------------------------------------------------

