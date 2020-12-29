//---------------------------------------------------------------------------
// rtkconv : rinex translator
//
//          Copyright (C) 2007-2011 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtkconv [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//           2010/07/18  1.1 rtklib 2.4.0
//           2011/06/10  1.2 rtklib 2.4.1
//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QTimer>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QMimeData>
#include <QDebug>
#include <QCompleter>
#include <QFileSystemModel>

#include "convmain.h"
#include "timedlg.h"
#include "aboutdlg.h"
#include "startdlg.h"
#include "keydlg.h"
#include "convopt.h"
#include "viewer.h"
#include "rtklib.h"

//---------------------------------------------------------------------------

MainWindow *mainWindow;

#define PRGNAME     "RTKCONV-QT"  // program name
#define MAXHIST     20         // max number of histories
#define TSTARTMARGIN 60.0      // time margin for file name replacement
#define TRACEFILE   "rtkconv.trace" // trace file

static int abortf=0;

// show message in message area ---------------------------------------------
extern "C" {
extern int showmsg(char *format,...)
{
    va_list arg;
    char buff[1024];
    va_start(arg,format); vsprintf(buff,format,arg); va_end(arg);
    QMetaObject::invokeMethod(mainWindow->Message,"setText",Qt::QueuedConnection,Q_ARG(QString,QString(buff)));
    return abortf;
}
}



// constructor --------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    mainWindow=this;

    TimeInt->setValidator(new QDoubleValidator());
    TimeUnit->setValidator(new QDoubleValidator());

    gtime_t time0={0,0};
    int i;

    setlocale(LC_NUMERIC,"C");
    setWindowIcon(QIcon(":/icons/rtkconv_Icon.ico"));
    setAcceptDrops(true);

    QString file=QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile=fi.absolutePath()+"/"+fi.baseName()+".ini";

    convOptDialog = new ConvOptDialog(this);
    timeDialog = new TimeDialog(this);
    keyDialog = new KeyDialog(this);
    aboutDialog = new AboutDialog(this);
    startDialog = new StartDialog(this);
    viewer=new TextViewer(this);

    Format->clear();
    Format->addItem(tr("Auto"));
    for (i=0;i<=MAXRCVFMT;i++) {
        Format->addItem(formatstrs[i]);
    }
    Format->addItem(formatstrs[STRFMT_RINEX]);
    RnxTime=time0;
    EventEna=0;

    QCompleter *fileCompleter=new QCompleter(this);
    QFileSystemModel *fileModel=new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    OutFile1->setCompleter(fileCompleter);
    OutFile2->setCompleter(fileCompleter);
    OutFile3->setCompleter(fileCompleter);
    OutFile4->setCompleter(fileCompleter);
    OutFile5->setCompleter(fileCompleter);
    OutFile6->setCompleter(fileCompleter);
    OutFile7->setCompleter(fileCompleter);
    InFile->setCompleter(fileCompleter);

    QCompleter *dirCompleter=new QCompleter(this);
    QFileSystemModel *dirModel=new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs|QDir::Drives|QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    OutDir->setCompleter(dirCompleter);

    BtnAbort->setVisible(false);

    connect(BtnPlot,SIGNAL(clicked(bool)),this,SLOT(BtnPlotClick()));
    connect(BtnConvert,SIGNAL(clicked(bool)),this,SLOT(BtnConvertClick()));
    connect(BtnOptions,SIGNAL(clicked(bool)),this,SLOT(BtnOptionsClick()));
    connect(BtnExit,SIGNAL(clicked(bool)),this,SLOT(BtnExitClick()));
    connect(BtnAbout,SIGNAL(clicked(bool)),this,SLOT(BtnAboutClick()));
    connect(BtnTime1,SIGNAL(clicked(bool)),this,SLOT(BtnTime1Click()));
    connect(BtnTime2,SIGNAL(clicked(bool)),this,SLOT(BtnTime2Click()));
    connect(BtnInFile,SIGNAL(clicked(bool)),this,SLOT(BtnInFileClick()));
    connect(BtnOutFile1,SIGNAL(clicked(bool)),this,SLOT(BtnOutFile1Click()));
    connect(BtnOutFile2,SIGNAL(clicked(bool)),this,SLOT(BtnOutFile2Click()));
    connect(BtnOutFile3,SIGNAL(clicked(bool)),this,SLOT(BtnOutFile3Click()));
    connect(BtnOutFile4,SIGNAL(clicked(bool)),this,SLOT(BtnOutFile4Click()));
    connect(BtnOutFile5,SIGNAL(clicked(bool)),this,SLOT(BtnOutFile5Click()));
    connect(BtnOutFile6,SIGNAL(clicked(bool)),this,SLOT(BtnOutFile6Click()));
    connect(BtnOutFile7,SIGNAL(clicked(bool)),this,SLOT(BtnOutFile7Click()));
    connect(BtnOutFileView1,SIGNAL(clicked(bool)),this,SLOT(BtnOutFileView1Click()));
    connect(BtnOutFileView2,SIGNAL(clicked(bool)),this,SLOT(BtnOutFileView2Click()));
    connect(BtnOutFileView3,SIGNAL(clicked(bool)),this,SLOT(BtnOutFileView3Click()));
    connect(BtnOutFileView4,SIGNAL(clicked(bool)),this,SLOT(BtnOutFileView4Click()));
    connect(BtnOutFileView5,SIGNAL(clicked(bool)),this,SLOT(BtnOutFileView5Click()));
    connect(BtnOutFileView6,SIGNAL(clicked(bool)),this,SLOT(BtnOutFileView6Click()));
    connect(BtnOutFileView7,SIGNAL(clicked(bool)),this,SLOT(BtnOutFileView7Click()));
    connect(BtnAbort,SIGNAL(clicked(bool)),this,SLOT(BtnAbortClick()));
    connect(TimeStartF,SIGNAL(clicked(bool)),this,SLOT(TimeStartFClick()));
    connect(TimeEndF,SIGNAL(clicked(bool)),this,SLOT(TimeEndFClick()));
    connect(TimeIntF,SIGNAL(clicked(bool)),this,SLOT(TimeIntFClick()));
    connect(TimeUnitF,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));
    connect(OutDirEna,SIGNAL(clicked(bool)),this,SLOT(OutDirEnaClick()));
    connect(InFile,SIGNAL(currentIndexChanged(int)),this,SLOT(InFileChange()));
    connect(InFile,SIGNAL(editTextChanged(QString)),this,SLOT(InFileChange()));
    connect(Format,SIGNAL(currentIndexChanged(int)),this,SLOT(FormatChange()));
    connect(OutDir,SIGNAL(editingFinished()),this,SLOT(OutDirChange()));
    connect(BtnOutDir,SIGNAL(clicked(bool)),this,SLOT(BtnOutDirClick()));
    connect(BtnKey,SIGNAL(clicked(bool)),this,SLOT(BtnKeyClick()));
    connect(BtnPost,SIGNAL(clicked(bool)),this,SLOT(BtnPostClick()));
    connect(BtnInFileView,SIGNAL(clicked(bool)),this,SLOT(BtnInFileViewClick()));
    connect(OutFileEna1,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));
    connect(OutFileEna2,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));
    connect(OutFileEna3,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));
    connect(OutFileEna4,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));
    connect(OutFileEna5,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));
    connect(OutFileEna6,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));
    connect(OutFileEna7,SIGNAL(clicked(bool)),this,SLOT(UpdateEnable()));

    QTimer::singleShot(100,this,SLOT(FormCreate()));
}
// callback on form create --------------------------------------------------
void MainWindow::FormCreate()
{
    setWindowTitle(QString(tr("%1 ver.%2 %3")).arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
}
// callback on form show ----------------------------------------------------
void MainWindow::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QCommandLineParser parser;
    parser.setApplicationDescription("RTK Navi");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniFileOption(QStringList() << "i",
            QCoreApplication::translate("main", "ini file to use"),
            QCoreApplication::translate("main", "ini file"));
    parser.addOption(iniFileOption);

    QCommandLineOption titleOption(QStringList() << "t",
            QCoreApplication::translate("main", "window title"),
            QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    parser.process(*QApplication::instance());

    if (parser.isSet(iniFileOption))

        IniFile=parser.value(iniFileOption);
    LoadOpt();

    if (parser.isSet(titleOption))
        setWindowTitle(parser.value(titleOption));

    EventEna=1;
}
// callback on form close ---------------------------------------------------
void MainWindow::closeEvent(QCloseEvent*)
{
    SaveOpt();
}
// set output file paths ----------------------------------------------------
void MainWindow::SetOutFiles(const QString &infile)
{
    QLineEdit *edit[]={
        OutFile1,OutFile2,OutFile3,OutFile4,OutFile5,OutFile6,OutFile7
    };
    QString Format_Text=Format->currentText();
    QString OutDir_Text=OutDir->text();
    QString ofile[8];
    int i,lex=Format_Text.contains("LEX");
    
    if (!EventEna) return;
    
    if (OutDirEna->isChecked()) {
        QFileInfo info(infile);

        ofile[0]=OutDir_Text+"/"+info.fileName();
    }
    else {
        ofile[0]=infile;
    }
    ofile[0].replace('*','0');
    ofile[0].replace('?','0');

    if (!RnxFile) {
        QFileInfo info(ofile[0]);
        ofile[0]=info.absolutePath()+"/"+info.baseName();
        ofile[1]=ofile[0]+".obs";
        ofile[2]=ofile[0]+".nav";
        ofile[3]=ofile[0]+".gnav";
        ofile[4]=ofile[0]+".hnav";
        ofile[5]=ofile[0]+".qnav";
        ofile[6]=ofile[0]+".lnav";
        ofile[7]=ofile[0]+(lex?".lex":".sbs");
    }
    else {
        QFileInfo info(ofile[0]);
        ofile[0]=info.filePath()+"/";
        ofile[1]+=ofile[0]+QString("%%r%%n0.%%yO");
        if (RnxVer>=3&&NavSys&&(NavSys!=SYS_GPS)) { /* ver.3 and mixed system */
            ofile[2]+=ofile[0]+"%%r%%n0.%%yP";
        }
        else {
            ofile[2]+=ofile[0]+"%%r%%n0.%%yN";
        }
        ofile[3]+=ofile[0]+"%%r%%n0.%%yG";
        ofile[4]+=ofile[0]+"%%r%%n0.%%yH";
        ofile[5]+=ofile[0]+"%%r%%n0.%%yQ";
        ofile[6]+=ofile[0]+"%%r%%n0.%%yL";
        ofile[7]+=ofile[0]+(lex?"%%r%%n0_%%y.lex":"%%r%%n0_%%y.sbs");
    }
    for (i=0;i<7;i++) {
        if (ofile[i+1]==infile) ofile[i+1]+="_";
        edit[i]->setText(ofile[i+1]);
    }
}
// callback on file drag and drop -------------------------------------------
void  MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
void  MainWindow::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/uri-list")) return;

    QString file=QUrl(event->mimeData()->text()).toLocalFile();

    InFile->setCurrentText(file);
    SetOutFiles(InFile->currentText());
}
// add history --------------------------------------------------------------
void MainWindow::AddHist(QComboBox *combo)
{
    QString hist=combo->currentText();
    if (hist=="") return;
    int i=combo->currentIndex();
    if (i>=0) combo->removeItem(i);
    combo->insertItem(0,hist);
    for (int i=combo->count()-1;i>=MAXHIST;i--) combo->removeItem(i);
    combo->setCurrentIndex(0);
}
// read history -------------------------------------------------------------
void MainWindow::ReadList(QComboBox* combo, QSettings *ini, const QString &key)
{
    QString item;
    int i;

    for (i=0;i<100;i++) {
        item=ini->value(QString("%1_%2").arg(key).arg(i,3),"").toString();
        if (item!="") combo->addItem(item); else break;
    }
}
// write history ------------------------------------------------------------
void MainWindow::WriteList(QSettings *ini, const QString &key, const QComboBox *combo)
{
    int i;

    for (i=0;i<combo->count();i++) {
        ini->setValue(QString("%1_%2").arg(key).arg(i,3),combo->itemText(i));
    }
}
// callback on button-plot --------------------------------------------------
void MainWindow::BtnPlotClick()
{
    QString file1=OutFile1->text();
    QString file2=OutFile2->text();
    QString file3=OutFile3->text();
    QString file4=OutFile4->text();
    QString file5=OutFile5->text();
    QString file6=OutFile6->text();
    QString file[]={file1,file2,file3,file4,file5,file6};
    QString cmd1="rtkplot_Qt",cmd2="..\\..\\..\\bin\\rtkplot_qt",opts=" -r";
    QCheckBox *cb[]={
        OutFileEna1,OutFileEna2,OutFileEna3,OutFileEna4,OutFileEna5,OutFileEna6
    };
    int i,ena[6];
    
    for (i=0;i<6;i++) ena[i]=cb[i]->isEnabled()&&cb[i]->isChecked();
    
    for (i=0;i<6;i++) {
        if (ena[i]) opts=opts+" \""+RepPath(file[i])+"\"";
    }
    if (opts==" -r") return;
    
    if (!ExecCmd(cmd1+opts)&&!ExecCmd(cmd2+opts)) {
        Message->setText(tr("error : rtkplot execution"));
    }
}
// callback on button-post-proc ---------------------------------------------
void MainWindow::BtnPostClick()
{
    QString path2="..\\..\\..\\bin\\";
    QString cmd1=CmdPostExe,cmd2=path2+CmdPostExe,opts=" ";
    
    if (!OutFileEna1->isChecked()) return;
    
    opts=opts+" -r \""+OutFile1->text()+"\"";
    opts=opts+" -n \"\" -n \"\"";
    
    if (OutFileEna7->isChecked()) {
        opts=opts+" -n \""+OutFile7->text()+"\"";
    }
    if (TimeStartF->isChecked()) opts=opts+" -ts "+TimeY1->text()+" "+TimeH1->text();
    if (TimeEndF  ->isChecked()) opts=opts+" -te "+TimeY2->text()+" "+TimeH2->text();
    if (TimeIntF  ->isChecked()) opts=opts+" -ti "+TimeInt->currentText();
    if (TimeUnitF ->isChecked()) opts=opts+" -tu "+TimeUnit->text();
    
    if (!ExecCmd(cmd1+opts)&&!ExecCmd(cmd2+opts)) {
        Message->setText(tr("error : rtkpost execution"));
    }
}
// callback on button-options -----------------------------------------------
void MainWindow::BtnOptionsClick()
{
    int rnxfile=RnxFile;
    convOptDialog->exec();
    if (convOptDialog->result()!=QDialog::Accepted) return;
    if (RnxFile!=rnxfile) {
        SetOutFiles(InFile->currentText());
    }
    UpdateEnable();
}
//---------------------------------------------------------------------------
void MainWindow::BtnAbortClick()
{
    abortf=1;
}
// callback on button-convert -----------------------------------------------
void MainWindow::BtnConvertClick()
{
    ConvertFile();
}
// callback on button-exit --------------------------------------------------
void MainWindow::BtnExitClick()
{
    close();
}
// callbck on button-time-1 -------------------------------------------------
void MainWindow::BtnTime1Click()
{
    gtime_t ts={0,0},te={0,0};
    double tint=0.0,tunit=0.0;
    GetTime(&ts,&te,&tint,&tunit);
    timeDialog->Time=ts;
    timeDialog->exec();
}
// callbck on button-time-2 -------------------------------------------------
void MainWindow::BtnTime2Click()
{
    gtime_t ts={0,0},te={0,0};
    double tint=0.0,tunit=0.0;
    GetTime(&ts,&te,&tint,&tunit);
    timeDialog->Time=te;
    timeDialog->exec();
}
// callback on button-input-file --------------------------------------------
void MainWindow::BtnInFileClick()
{    
    InFile->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Input RTCM, RCV RAW or RINEX OBS File"),QString(),
         tr("All (*.*);;RTCM 2 (*.rtcm2);;RTCM 3 (*.rtcm3);;NovtAtel (*.gps);;ublox (*.ubx);;SuperStart II (*.log);;"
            "Hemisphere (*.bin);;Javad (*.jps);;RINEX OBS (*.obs *.*O);Septentrio (*.sbf)"))));
    SetOutFiles(InFile->currentText());
}
// callback on output-directory change --------------------------------------
void MainWindow::OutDirChange()
{
    SetOutFiles(InFile->currentText());
}
// callback on button-output-directory --------------------------------------
void MainWindow::BtnOutDirClick()
{
    OutDir->setText(QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,tr("Output Directory"),OutDir->text())));
    SetOutFiles(InFile->currentText());
}
// callback on button-keyword -----------------------------------------------
void MainWindow::BtnKeyClick()
{
    keyDialog->Flag=1;
    keyDialog->show();
}
// callback on button-output-file-1 -----------------------------------------
void MainWindow::BtnOutFile1Click()
{
    QString selectedFilter="RINEX OBS (*.obs *.*O";
    OutFile1->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Output RINEX OBS File"),QString(),
              tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                 "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;SBAS Log (*.sbs);;LEX Log (*.lex)"),&selectedFilter)));
}
// callback on button-output-file-2 -----------------------------------------
void MainWindow::BtnOutFile2Click()
{
    QString selectedFilter="RINEX NAV (*.nav *.*N *.*P)";
    OutFile2->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Output RINEX NAV File"),QString(),
              tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                 "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;SBAS Log (*.sbs);;LEX Log (*.lex)"),&selectedFilter)));
}
// callback on button-output-file-3 -----------------------------------------
void MainWindow::BtnOutFile3Click()
{
    QString selectedFilter="RINEX GNAV (*.gnav *.*G)";
    OutFile3->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Output RINEX GNAV File"),QString(),
              tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                 "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;SBAS Log (*.sbs);;LEX Log (*.lex)"),&selectedFilter)));
}
// callback on button-output-file-4 -----------------------------------------
void MainWindow::BtnOutFile4Click()
{
    QString selectedFilter="RINEX HNAV (*.hnav *.*H)";
    OutFile4->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Output RINEX HNAV File"),QString(),
              tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                 "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;SBAS Log (*.sbs);;LEX Log (*.lex)"),&selectedFilter)));
}
// callback on button-output-file-5 -----------------------------------------
void MainWindow::BtnOutFile5Click()
{
    QString selectedFilter="RINEX QNAV (*.qnav *.*Q)";
    OutFile5->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Output RINEX QNAV File"),QString(),
              tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                 "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;SBAS Log (*.sbs);;LEX Log (*.lex)"),&selectedFilter)));
}
// callback on button-output-file-6 -----------------------------------------
void MainWindow::BtnOutFile6Click()
{
    QString selectedFilter="RINEX LNAV (*.lnav *.*L)";
    OutFile6->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Output RINEX LNAV File"),QString(),
              tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                 "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;SBAS Log (*.sbs);;LEX Log (*.lex)"),&selectedFilter)));
}
// callback on button-output-file-7 -----------------------------------------
void MainWindow::BtnOutFile7Click()
{
    QString selectedFilter="SBAS Log (*.sbs)";
    OutFile7->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Output SBAS/LEX Log File"),QString(),
              tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                 "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;SBAS Log (*.sbs);;LEX Log (*.lex)"),&selectedFilter)));
}
// callback on button-view-input-file ----------------------------------------
void MainWindow::BtnInFileViewClick()
{
    QString InFile_Text=InFile->currentText();
    QString ext=QFileInfo(InFile_Text).suffix();
    if (ext.length()<4) return;

    if ((ext=="obs" )||(ext=="OBS" )||(ext=="nav")||
        (ext=="NAV" )||(ext.mid(1)=="nav")||(ext.mid(1)=="NAV" )||
        (ext.at(2)=='o'  )||(ext.at(2)=='O'  )||(ext.at(2)=='n' )||
        (ext.at(2)=='N'  )||(ext.at(2)=='p'  )||(ext.at(2)=='P' )||
        (ext.at(2)=='g'  )||(ext.at(2)=='G'  )||(ext.at(2)=='h' )||
        (ext.at(2)=='H'  )||(ext.at(2)=='q'  )||(ext.at(2)=='Q' )||
        (ext.at(2)=='l'  )||(ext.at(2)=='L'  )) {
        viewer->show();
        viewer->Read(RepPath(InFile_Text));
    }
}
// callback on button-view-file-1 -------------------------------------------
void MainWindow::BtnOutFileView1Click()
{
    TextViewer *viewer=new TextViewer(this);
    QString OutFile1_Text=OutFile1->text();
    viewer->show();
    viewer->Read(RepPath(OutFile1_Text));
}
// callback on button-view-file-2 -------------------------------------------
void MainWindow::BtnOutFileView2Click()
{
    TextViewer *viewer=new TextViewer(this);
    QString OutFile2_Text=OutFile2->text();
    viewer->show();
    viewer->Read(RepPath(OutFile2_Text));
}
// callback on button-view-file-3 -------------------------------------------
void MainWindow::BtnOutFileView3Click()
{
    TextViewer *viewer=new TextViewer(this);
    QString OutFile3_Text=OutFile3->text();
    viewer->show();
    viewer->Read(RepPath(OutFile3_Text));
}
// callback on button-view-file-4 -------------------------------------------
void MainWindow::BtnOutFileView4Click()
{
    TextViewer *viewer=new TextViewer(this);
    QString OutFile4_Text=OutFile4->text();
    viewer->show();
    viewer->Read(RepPath(OutFile4_Text));
}
// callback on button-view-file-5 -------------------------------------------
void MainWindow::BtnOutFileView5Click()
{
    TextViewer *viewer=new TextViewer(this);
    QString OutFile5_Text=OutFile5->text();
    viewer->show();
    viewer->Read(RepPath(OutFile5_Text));
}
// callback on button-view-file-6 -------------------------------------------
void MainWindow::BtnOutFileView6Click()
{
    TextViewer *viewer=new TextViewer(this);
    QString OutFile6_Text=OutFile6->text();
    viewer->show();
    viewer->Read(RepPath(OutFile6_Text));
}
// callback on button-view-file-7 -------------------------------------------
void MainWindow::BtnOutFileView7Click()
{
    TextViewer *viewer=new TextViewer(this);
    QString OutFile7_Text=OutFile7->text();
    viewer->show();
    viewer->Read(RepPath(OutFile7_Text));
}
// callback on button-about -------------------------------------------------
void MainWindow::BtnAboutClick()
{
    aboutDialog->About=PRGNAME;
    aboutDialog->IconIndex=3;
    aboutDialog->exec();
}
// callback on button-time-start --------------------------------------------
void MainWindow::TimeStartFClick()
{
    UpdateEnable();
}
// callback on button-time-end ----------------------------------------------
void MainWindow::TimeEndFClick()
{
    UpdateEnable();
}
// callback on button-time-interval -----------------------------------------
void MainWindow::TimeIntFClick()
{
    UpdateEnable();
}
// callback on output-file check/uncheck ------------------------------------
void MainWindow::OutDirEnaClick()
{
    SetOutFiles(InFile->currentText());
    UpdateEnable();
}
// callback on input-file-change --------------------------------------------
void MainWindow::InFileChange()
{
    SetOutFiles(InFile->currentText());
}
// callback on format change ------------------------------------------------
void MainWindow::FormatChange()
{
    UpdateEnable();
}
// get time -----------------------------------------------------------------
void MainWindow::GetTime(gtime_t *ts, gtime_t *te, double *tint,
        double *tunit)
{
    if (TimeStartF->isChecked()) {
        QDateTime start(TimeY1->date(),TimeH1->time());
        ts->time=start.toTime_t();ts->sec=start.time().msec()/1000;
    } else ts->time=ts->sec=0;
    if (TimeEndF->isChecked()) {
        QDateTime end(TimeY2->date(),TimeH2->time());
        te->time=end.toTime_t();te->sec=end.time().msec()/1000;
    } else te->time=te->sec=0;
    if (TimeIntF->isChecked()) {
        *tint=TimeInt->currentText().toDouble();
    } else *tint=0;
    if (TimeUnitF->isChecked()) {
        *tunit=TimeUnit->text().toDouble()*3600;
    } else *tunit=0;
}
// replace keywords in file path --------------------------------------------
QString MainWindow::RepPath(const QString &File)
{
    QString Path;
    char path[1024];
    reppath(qPrintable(File),path,timeadd(RnxTime,TSTARTMARGIN),qPrintable(RnxCode),"");
    return Path=path;
}
// execute command ----------------------------------------------------------
int MainWindow::ExecCmd(const QString &cmd)
{
    return QProcess::startDetached(cmd);  /* FIXME: show option not yet supported */
}
// undate enable/disable of widgets -----------------------------------------
void MainWindow::UpdateEnable(void)
{
    QString FormatText=Format->currentText();
    int rnx=FormatText=="RINEX";
    TimeY1         ->setEnabled(TimeStartF ->isChecked());
    TimeH1         ->setEnabled(TimeStartF ->isChecked());
    BtnTime1       ->setEnabled(TimeStartF ->isChecked());
    TimeY2         ->setEnabled(TimeEndF   ->isChecked());
    TimeH2         ->setEnabled(TimeEndF   ->isChecked());
    BtnTime2       ->setEnabled(TimeEndF   ->isChecked());
    TimeInt        ->setEnabled(TimeIntF   ->isChecked());
    LabelTimeInt   ->setEnabled(TimeInt    ->isEnabled());
    TimeUnitF      ->setEnabled(TimeStartF->isChecked()&&TimeEndF->isChecked());
    TimeUnit       ->setEnabled(TimeStartF->isChecked()&&TimeEndF->isChecked()&&TimeUnitF->isChecked());
    LabelTimeUnit  ->setEnabled(TimeUnit  ->isEnabled());
    OutFileEna3    ->setEnabled(RnxVer<3&&(NavSys&SYS_GLO));
    OutFileEna4    ->setEnabled(RnxVer<3&&(NavSys&SYS_SBS));
    OutFileEna5    ->setEnabled(RnxVer<3&&(NavSys&SYS_QZS));
    OutFileEna6    ->setEnabled(RnxVer<3&&(NavSys&SYS_GAL));
    OutFileEna7    ->setEnabled(!rnx);
    OutDir         ->setEnabled(OutDirEna  ->isChecked());
    LabelOutDir    ->setEnabled(OutDirEna  ->isChecked());
    OutFile1       ->setEnabled(OutFileEna1->isChecked());
    OutFile2       ->setEnabled(OutFileEna2->isChecked());
    OutFile3       ->setEnabled(OutFileEna3->isChecked()&&RnxVer<3&&(NavSys&SYS_GLO));
    OutFile4       ->setEnabled(OutFileEna4->isChecked()&&RnxVer<3&&(NavSys&SYS_SBS));
    OutFile5       ->setEnabled(OutFileEna5->isChecked()&&RnxVer<3&&(NavSys&SYS_QZS));
    OutFile6       ->setEnabled(OutFileEna6->isChecked()&&RnxVer<3&&(NavSys&SYS_GAL));
    OutFile7       ->setEnabled(OutFileEna7->isChecked()&&!rnx);
    BtnOutDir      ->setEnabled(OutDirEna  ->isChecked());
    BtnOutFile1    ->setEnabled(OutFile1->isEnabled());
    BtnOutFile2    ->setEnabled(OutFile2->isEnabled());
    BtnOutFile3    ->setEnabled(OutFile3->isEnabled());
    BtnOutFile4    ->setEnabled(OutFile4->isEnabled());
    BtnOutFile5    ->setEnabled(OutFile5->isEnabled());
    BtnOutFile6    ->setEnabled(OutFile6->isEnabled());
    BtnOutFile7    ->setEnabled(OutFile7->isEnabled());
    BtnOutFileView1->setEnabled(OutFile1->isEnabled());
    BtnOutFileView2->setEnabled(OutFile2->isEnabled());
    BtnOutFileView3->setEnabled(OutFile3->isEnabled());
    BtnOutFileView4->setEnabled(OutFile4->isEnabled());
    BtnOutFileView5->setEnabled(OutFile5->isEnabled());
    BtnOutFileView6->setEnabled(OutFile6->isEnabled());
    BtnOutFileView7->setEnabled(OutFile7->isEnabled());
}
// convert file -------------------------------------------------------------
void MainWindow::ConvertFile(void)
{
    QString InFile_Text=InFile->currentText();
    QString OutFile1_Text=OutFile1->text(),OutFile2_Text=OutFile2->text();
    QString OutFile3_Text=OutFile3->text(),OutFile4_Text=OutFile4->text();
    QString OutFile5_Text=OutFile5->text(),OutFile6_Text=OutFile6->text();
    QString OutFile7_Text=OutFile7->text();
    int i;
    char *p;
    double RNXVER[]={2.10,2.11,2.12,3.00,3.01,3.02,3.03};
    
    conversionThread= new ConversionThread(this);

    // recognize input file format
    strcpy(conversionThread->ifile,qPrintable(InFile_Text));
    QFileInfo fi(InFile_Text);
    if (Format->currentIndex()==0) { // auto
        if      (fi.completeSuffix()=="rtcm2") conversionThread->format=STRFMT_RTCM2;
        else if (fi.completeSuffix()=="rtcm3") conversionThread->format=STRFMT_RTCM3;
        else if (fi.completeSuffix()=="gps"  ) conversionThread->format=STRFMT_OEM4;
        else if (fi.completeSuffix()=="ubx"  ) conversionThread->format=STRFMT_UBX;
        else if (fi.completeSuffix()=="log"  ) conversionThread->format=STRFMT_SS2;
        else if (fi.completeSuffix()=="bin"  ) conversionThread->format=STRFMT_CRES;
        else if (fi.completeSuffix()=="jps"  ) conversionThread->format=STRFMT_JAVAD;
        else if (fi.completeSuffix()=="bnx"  ) conversionThread->format=STRFMT_BINEX;
        else if (fi.completeSuffix()=="binex") conversionThread->format=STRFMT_BINEX;
        else if (fi.completeSuffix()=="rt17" ) conversionThread->format=STRFMT_RT17;
        else if (fi.completeSuffix()=="cmr"  ) conversionThread->format=STRFMT_CMR;
        else if (fi.completeSuffix().toLower()=="obs"  ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().toLower().contains( "nav" )) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='o'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='O'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='n'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='N'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='p'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='P'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='g'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='G'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='h'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='H'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='q'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='Q'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='l'   ) conversionThread->format=STRFMT_RINEX;
        else if (fi.completeSuffix().at(2)=='L'   ) conversionThread->format=STRFMT_RINEX;
        else {
            showmsg("file format can not be recognized");
            return;
        }
    }
    else {
        for (i=0;formatstrs[i];i++) {
            if (Format->currentText()==formatstrs[i]) break;
        }
        if (formatstrs[i]) conversionThread->format=i; else return;
    }
    conversionThread->rnxopt.rnxver=RNXVER[RnxVer];
    
    if (conversionThread->format==STRFMT_RTCM2||conversionThread->format==STRFMT_RTCM3||conversionThread->format==STRFMT_RT17
            ||conversionThread->format==STRFMT_CMR) {
        
        // input start date/time for rtcm 2, rtcm 3, RT17 or CMR
        startDialog->exec();
        if (startDialog->result()!=QDialog::Accepted) return;
        conversionThread->rnxopt.trtcm=startDialog->Time;
    }
    if (OutFile1->isEnabled()&&OutFileEna1->isChecked()) strcpy(conversionThread->ofile[0],qPrintable(OutFile1_Text));
    if (OutFile2->isEnabled()&&OutFileEna2->isChecked()) strcpy(conversionThread->ofile[1],qPrintable(OutFile2_Text));
    if (OutFile3->isEnabled()&&OutFileEna3->isChecked()) strcpy(conversionThread->ofile[2],qPrintable(OutFile3_Text));
    if (OutFile4->isEnabled()&&OutFileEna4->isChecked()) strcpy(conversionThread->ofile[3],qPrintable(OutFile4_Text));
    if (OutFile5->isEnabled()&&OutFileEna5->isChecked()) strcpy(conversionThread->ofile[4],qPrintable(OutFile5_Text));
    if (OutFile6->isEnabled()&&OutFileEna6->isChecked()) strcpy(conversionThread->ofile[5],qPrintable(OutFile6_Text));
    if (OutFile7->isEnabled()&&OutFileEna7->isChecked()) strcpy(conversionThread->ofile[6],qPrintable(OutFile7_Text));
    
    // check overwrite output file
    for (i=0;i<6;i++) {
        if (!QFile(conversionThread->ofile[i]).exists()) continue;
        if (QMessageBox::question(this,tr("Overwrite"),QString(tr("%1 exists. Do you want to overwrite?")).arg(conversionThread->ofile[i]))!=QMessageBox::Yes) return;
    }
    GetTime(&conversionThread->rnxopt.ts,&conversionThread->rnxopt.te,&conversionThread->rnxopt.tint,&conversionThread->rnxopt.tunit);
    strncpy(conversionThread->rnxopt.staid,qPrintable(RnxCode),31);
    sprintf(conversionThread->rnxopt.prog,"%s %s %s",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
    strncpy(conversionThread->rnxopt.runby,qPrintable(RunBy),31);
    strncpy(conversionThread->rnxopt.marker,qPrintable(Marker),63);
    strncpy(conversionThread->rnxopt.markerno,qPrintable(MarkerNo),31);
    strncpy(conversionThread->rnxopt.markertype,qPrintable(MarkerType),31);
    for (i=0;i<2;i++) strncpy(conversionThread->rnxopt.name[i],qPrintable(Name[i]),31);
    for (i=0;i<3;i++) strncpy(conversionThread->rnxopt.rec [i],qPrintable(Rec [i]),31);
    for (i=0;i<3;i++) strncpy(conversionThread->rnxopt.ant [i],qPrintable(Ant [i]),31);
    if (AutoPos) {
        for (i=0;i<3;i++) conversionThread->rnxopt.apppos[i]=AppPos[i];
    }
    for (i=0;i<3;i++) conversionThread->rnxopt.antdel[i]=AntDel[i];
    strncpy(conversionThread->rnxopt.rcvopt,qPrintable(RcvOption),255);
    conversionThread->rnxopt.navsys=NavSys;
    conversionThread->rnxopt.obstype=ObsType;
    conversionThread->rnxopt.freqtype=FreqType;
    p=conversionThread->rnxopt.comment[0];
    sprintf(p,"log: %-53.53s",conversionThread->ifile);
    p=conversionThread->rnxopt.comment[1];
    p+=sprintf(p,"format: %s",formatstrs[conversionThread->format]);
    if (*conversionThread->rnxopt.rcvopt) sprintf(p,", option: %s",conversionThread->rnxopt.rcvopt);
    for (i=0;i<2;i++) strncpy(conversionThread->rnxopt.comment[i+2],qPrintable(Comment[i]),63);
    for (i=0;i<6;i++) strcpy(conversionThread->rnxopt.mask[i],qPrintable(CodeMask[i]));
    conversionThread->rnxopt.autopos=AutoPos;
    conversionThread->rnxopt.scanobs=ScanObs;
    conversionThread->rnxopt.outiono=OutIono;
    conversionThread->rnxopt.outtime=OutTime;
    conversionThread->rnxopt.outleaps=OutLeaps;
    
    QStringList exsatsLst=ExSats.split(" ");
    foreach (const QString & sat,exsatsLst)
    {
        int satid;
        if (!(satid=satid2no(qPrintable(sat)))) continue;
        conversionThread->rnxopt.exsats[satid-1]=1;
    }

    abortf=0;
    BtnConvert  ->setVisible(false);
    BtnAbort    ->setVisible(true);
    Panel1      ->setEnabled(false);
    Panel2      ->setEnabled(false);
    BtnPlot     ->setEnabled(false);
    BtnPost     ->setEnabled(false);
    BtnOptions  ->setEnabled(false);
    BtnExit     ->setEnabled(false);
    Format      ->setEnabled(false);
    BtnKey      ->setEnabled(false);
    LabelInFile ->setEnabled(false);
    LabelOutDir ->setEnabled(false);
    LabelOutFile->setEnabled(false);
    LabelFormat ->setEnabled(false);
    Message     ->setText("");
    
    if (TraceLevel>0) {
        traceopen(TRACEFILE);
        tracelevel(TraceLevel);
    }
    setCursor(Qt::WaitCursor);

    // post processing positioning
    connect(conversionThread,SIGNAL(finished()),this,SLOT(ConversionFinished()));

    conversionThread->start();
}
// conversion done -------------------------------------------------------------
void MainWindow::ConversionFinished()
{
    setCursor(Qt::ArrowCursor);

    if (TraceLevel>0) {
        traceclose();
    }
    BtnConvert  ->setVisible(true);
    BtnAbort    ->setVisible(false);
    Panel1      ->setEnabled(true);
    Panel2      ->setEnabled(true);
    BtnPlot     ->setEnabled(true);
    BtnPost     ->setEnabled(true);
    BtnOptions  ->setEnabled(true);
    BtnExit     ->setEnabled(true);
    Format      ->setEnabled(true);
    BtnKey      ->setEnabled(true);
    LabelInFile ->setEnabled(true);
    LabelOutDir ->setEnabled(true);
    LabelOutFile->setEnabled(true);
    LabelFormat ->setEnabled(true);
    
#if 0
    // set time-start/end if time not specified
    if (!TimeStartF->Checked&&rnxopt.tstart.time!=0) {
        time2str(rnxopt.tstart,tstr,0);
        tstr[10]='\0';
        TimeY1->Text=tstr;
        TimeH1->Text=tstr+11;
    }
    if (!TimeEndF->Checked&&rnxopt.tend.time!=0) {
        time2str(rnxopt.tend,tstr,0);
        tstr[10]='\0';
        TimeY2->Text=tstr;
        TimeH2->Text=tstr+11;
    }
#endif
    RnxTime=conversionThread->rnxopt.tstart;

    conversionThread->deleteLater();

    AddHist(InFile);
}
// load options -------------------------------------------------------------
void MainWindow::LoadOpt(void)
{
    QSettings ini(IniFile,QSettings::IniFormat);
    QString mask="1111111111111111111111111111111111111111111";
    
    RnxVer              =ini.value ("opt/rnxver",      0).toInt();
    RnxFile             =ini.value ("opt/rnxfile",     0).toInt();
    RnxCode             =ini.value ("opt/rnxcode","0000").toString();
    RunBy               =ini.value ("opt/runby",      "").toString();
    Marker              =ini.value ("opt/marker",     "").toString();
    MarkerNo            =ini.value ("opt/markerno",   "").toString();
    MarkerType          =ini.value ("opt/markertype", "").toString();
    Name[0]             =ini.value ("opt/name0",      "").toString();
    Name[1]             =ini.value ("opt/name1",      "").toString();
    Rec[0]              =ini.value ("opt/rec0",       "").toString();
    Rec[1]              =ini.value ("opt/rec1",       "").toString();
    Rec[2]              =ini.value ("opt/rec2",       "").toString();
    Ant[0]              =ini.value ("opt/ant0",       "").toString();
    Ant[1]              =ini.value ("opt/ant1",       "").toString();
    Ant[2]              =ini.value ("opt/ant2",       "").toString();
    AppPos[0]           =ini.value ("opt/apppos0",   0.0).toDouble();
    AppPos[1]           =ini.value ("opt/apppos1",   0.0).toDouble();
    AppPos[2]           =ini.value ("opt/apppos2",   0.0).toDouble();
    AntDel[0]           =ini.value ("opt/antdel0",   0.0).toDouble();
    AntDel[1]           =ini.value ("opt/antdel1",   0.0).toDouble();
    AntDel[2]           =ini.value ("opt/antdel2",   0.0).toDouble();
    Comment[0]          =ini.value ("opt/comment0",   "").toString();
    Comment[1]          =ini.value ("opt/comment1",   "").toString();
    RcvOption           =ini.value ("opt/rcvoption",  "").toString();
    NavSys              =ini.value ("opt/navsys",    0x3).toInt();
    ObsType             =ini.value ("opt/obstype",   0xF).toInt();
    FreqType            =ini.value ("opt/freqtype",  0x3).toInt();
    ExSats              =ini.value ("opt/exsats",     "").toString();
    TraceLevel          =ini.value ("opt/tracelevel",  0).toInt();
    RnxTime.time        =ini.value ("opt/rnxtime",     0).toInt();
    CodeMask[0]         =ini.value ("opt/codemask_1",mask).toString();
    CodeMask[1]         =ini.value ("opt/codemask_2",mask).toString();
    CodeMask[2]         =ini.value ("opt/codemask_3",mask).toString();
    CodeMask[3]         =ini.value ("opt/codemask_4",mask).toString();
    CodeMask[4]         =ini.value ("opt/codemask_5",mask).toString();
    CodeMask[5]         =ini.value ("opt/codemask_6",mask).toString();
    AutoPos             =ini.value ("opt/autopos",     0).toInt();
    ScanObs             =ini.value ("opt/scanobs",     0).toInt();
    OutIono             =ini.value ("opt/outiono",     0).toInt();
    OutTime             =ini.value ("opt/outtime",     0).toInt();
    OutLeaps            =ini.value ("opt/outleaps",    0).toInt();
    
    TimeStartF ->setChecked(ini.value("set/timestartf",  0).toBool());
    TimeEndF   ->setChecked(ini.value("set/timeendf",    0).toBool());
    TimeIntF   ->setChecked(ini.value("set/timeintf",    0).toBool());
    TimeY1     ->setDate(ini.value ("set/timey1",     "2000/01/01").value<QDate>());
    TimeH1     ->setTime(ini.value ("set/timeh1",     "00:00:00"  ).value<QTime>());
    TimeY2     ->setDate(ini.value ("set/timey2",     "2000/01/01").value<QDate>());
    TimeH2     ->setTime(ini.value ("set/timeh2",     "00:00:00"  ).value<QTime>());
    TimeInt    ->setCurrentText(ini.value ("set/timeint",   "1").toString());
    TimeUnitF  ->setChecked(ini.value("set/timeunitf",   0).toBool());
    TimeUnit   ->setText(ini.value ("set/timeunit", "24").toString());
    InFile     ->setCurrentText(ini.value ("set/infile",     "").toString());
    OutDir     ->setText(ini.value ("set/outdir",     "").toString());
    OutFile1   ->setText(ini.value ("set/outfile1",   "").toString());
    OutFile2   ->setText(ini.value ("set/outfile2",   "").toString());
    OutFile3   ->setText(ini.value ("set/outfile3",   "").toString());
    OutFile4   ->setText(ini.value ("set/outfile4",   "").toString());
    OutFile5   ->setText(ini.value ("set/outfile5",   "").toString());
    OutFile6   ->setText(ini.value ("set/outfile6",   "").toString());
    OutFile7   ->setText(ini.value ("set/outfile7",   "").toString());
    OutDirEna  ->setChecked(ini.value("set/outdirena",   false).toBool());
    OutFileEna1->setChecked(ini.value("set/outfileena1", true).toBool());
    OutFileEna2->setChecked(ini.value("set/outfileena2", true).toBool());
    OutFileEna3->setChecked(ini.value("set/outfileena3", true).toBool());
    OutFileEna4->setChecked(ini.value("set/outfileena4", true).toBool());
    OutFileEna5->setChecked(ini.value("set/outfileena5", true).toBool());
    OutFileEna6->setChecked(ini.value("set/outfileena6", true).toBool());
    OutFileEna7->setChecked(ini.value("set/outfileena7", true).toBool());
    Format   ->setCurrentIndex(ini.value("set/format",      0).toInt());
    
    ReadList(InFile,&ini,"hist/inputfile");
    
    TextViewer::Color1=ini.value("viewer/color1",QColor(Qt::black)).value<QColor>();
    TextViewer::Color2=ini.value("viewer/color2",QColor(Qt::white)).value<QColor>();
    TextViewer::FontD.setFamily(ini.value ("viewer/fontname","Courier New").toString());
    TextViewer::FontD.setPointSize(ini.value("viewer/fontsize",9).toInt());

    CmdPostExe         =ini.value  ("set/cmdpostexe","rtkpost_qt").toString();
    
    UpdateEnable();
}
// save options -------------------------------------------------------------
void MainWindow::SaveOpt(void)
{
    QSettings ini(IniFile,QSettings::IniFormat);

    ini.setValue ("opt/rnxver",     RnxVer);
    ini.setValue ("opt/rnxfile",    RnxFile);
    ini.setValue ("opt/rnxcode",    RnxCode);
    ini.setValue ("opt/runby",      RunBy);
    ini.setValue ("opt/marker",     Marker);
    ini.setValue ("opt/markerno",   MarkerNo);
    ini.setValue ("opt/markertype", MarkerType);
    ini.setValue ("opt/name0",      Name[0]);
    ini.setValue ("opt/name1",      Name[1]);
    ini.setValue ("opt/rec0",       Rec[0]);
    ini.setValue ("opt/rec1",       Rec[1]);
    ini.setValue ("opt/rec2",       Rec[2]);
    ini.setValue ("opt/ant0",       Ant[0]);
    ini.setValue ("opt/ant1",       Ant[1]);
    ini.setValue ("opt/ant2",       Ant[2]);
    ini.setValue ("opt/apppos0",    AppPos[0]);
    ini.setValue ("opt/apppos1",    AppPos[1]);
    ini.setValue ("opt/apppos2",    AppPos[2]);
    ini.setValue ("opt/antdel0",    AntDel[0]);
    ini.setValue ("opt/antdel1",    AntDel[1]);
    ini.setValue ("opt/antdel2",    AntDel[2]);
    ini.setValue ("opt/comment0",   Comment[0]);
    ini.setValue ("opt/comment1",   Comment[1]);
    ini.setValue ("opt/rcvoption",  RcvOption);
    ini.setValue ("opt/navsys",     NavSys);
    ini.setValue ("opt/obstype",    ObsType);
    ini.setValue ("opt/freqtype",   FreqType);
    ini.setValue ("opt/exsats",     ExSats);
    ini.setValue ("opt/tracelevel", TraceLevel);
    ini.setValue ("opt/rnxtime",(int)RnxTime.time);
    ini.setValue ("opt/codemask_1", CodeMask[0]);
    ini.setValue ("opt/codemask_2", CodeMask[1]);
    ini.setValue ("opt/codemask_3", CodeMask[2]);
    ini.setValue ("opt/codemask_4", CodeMask[3]);
    ini.setValue ("opt/codemask_5", CodeMask[4]);
    ini.setValue ("opt/codemask_6", CodeMask[5]);
    ini.setValue ("opt/autopos",    AutoPos);
    ini.setValue ("opt/scanobs",    ScanObs);
    ini.setValue ("opt/outiono",    OutIono);
    ini.setValue ("opt/outtime",    OutTime);
    ini.setValue ("opt/outleaps",   OutLeaps);
    
    ini.setValue ("set/timestartf", TimeStartF ->isChecked());
    ini.setValue ("set/timeendf",   TimeEndF   ->isChecked());
    ini.setValue ("set/timeintf",   TimeIntF   ->isChecked());
    ini.setValue ("set/timey1",     TimeY1     ->date());
    ini.setValue ("set/timeh1",     TimeH1     ->time());
    ini.setValue ("set/timey2",     TimeY2     ->date());
    ini.setValue ("set/timeh2",     TimeH2     ->time());
    ini.setValue ("set/timeint",    TimeInt    ->currentText());
    ini.setValue("set/timeunitf",  TimeUnitF  ->isChecked());
    ini.setValue ("set/timeunit",   TimeUnit   ->text());
    ini.setValue ("set/infile",     InFile     ->currentText());
    ini.setValue ("set/outdir",     OutDir     ->text());
    ini.setValue ("set/outfile1",   OutFile1   ->text());
    ini.setValue ("set/outfile2",   OutFile2   ->text());
    ini.setValue ("set/outfile3",   OutFile3   ->text());
    ini.setValue ("set/outfile4",   OutFile4   ->text());
    ini.setValue ("set/outfile5",   OutFile5   ->text());
    ini.setValue ("set/outfile6",   OutFile6   ->text());
    ini.setValue ("set/outfile7",   OutFile7   ->text());
    ini.setValue("set/outdirena",  OutDirEna  ->isChecked());
    ini.setValue("set/outfileena1",OutFileEna1->isChecked());
    ini.setValue("set/outfileena2",OutFileEna2->isChecked());
    ini.setValue("set/outfileena3",OutFileEna3->isChecked());
    ini.setValue("set/outfileena4",OutFileEna4->isChecked());
    ini.setValue("set/outfileena5",OutFileEna5->isChecked());
    ini.setValue("set/outfileena6",OutFileEna6->isChecked());
    ini.setValue("set/outfileena7",OutFileEna7->isChecked());
    ini.setValue("set/format",     Format     ->currentIndex());
    
    WriteList(&ini,"hist/inputfile",InFile);
    
    ini.setValue("viewer/color1",  TextViewer::Color1  );
    ini.setValue("viewer/color2",  TextViewer::Color2  );
    ini.setValue("viewer/fontname",TextViewer::FontD.family());
    ini.setValue("viewer/fontsize",TextViewer::FontD.pointSize());
}
//---------------------------------------------------------------------------

