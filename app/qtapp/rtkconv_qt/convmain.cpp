//---------------------------------------------------------------------------
// rtkconv : rinex converter
//
//          Copyright (C) 2007-2020 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtkconv [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//			 2010/07/18  1.1 rtklib 2.4.0
//			 2011/06/10  1.2 rtklib 2.4.1
//			 2013/04/01  1.3 rtklib 2.4.2
//			 2020/11/30  1.4 support RINEX 3.04
//			                 support NavIC/IRNSS
//                           support SBF for auto format recognition
//                           support "Phase Shift" option
//                           no support "Scan Obs" option
//---------------------------------------------------------------------------
#include <clocale>

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

#define PRGNAME     "RTKCONV-QT"        // program name
#define MAXHIST     20                  // max number of histories
#define TSTARTMARGIN 60.0               // time margin for file name replacement
#define TRACEFILE   "rtkconv_qt.trace"     // trace file

static int abortf = 0;

// show message in message area ---------------------------------------------
extern "C" {
extern int showmsg(const char *format, ...)
{
    va_list arg;
    char buff[1024];

    va_start(arg, format); vsprintf(buff, format, arg); va_end(arg);
    QMetaObject::invokeMethod(mainWindow->message, "setText", Qt::QueuedConnection, Q_ARG(QString, QString(buff)));
    return abortf;
}
extern void settime(gtime_t) {}
extern void settspan(gtime_t, gtime_t) {}
}

// constructor --------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
    mainWindow = this;

    comboTimeInterval->setValidator(new QDoubleValidator());
    lETimeUnit->setValidator(new QDoubleValidator());

    gtime_t time0 = { 0, 0 };
    int i;

    setlocale(LC_NUMERIC, "C");
    setWindowIcon(QIcon(":/icons/rtkconv_Icon.ico"));
    setAcceptDrops(true);

    QString app_filename = QApplication::applicationFilePath();
    QFileInfo fi(app_filename);
    iniFile = fi.absolutePath() + "/" + fi.baseName() + ".ini";

    convOptDialog = new ConvOptDialog(this);
    timeDialog = new TimeDialog(this);
    keyDialog = new KeyDialog(this);
    aboutDialog = new AboutDialog(this);
    startDialog = new StartDialog(this);
    viewer = new TextViewer(this);

    cBFormat->clear();
    cBFormat->addItem(tr("Auto"));
    for (i = 0; i <= MAXRCVFMT; i++)
        cBFormat->addItem(formatstrs[i]);
    cBFormat->addItem(formatstrs[STRFMT_RINEX]);

    rinexTime = time0;

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    outputFile1->setCompleter(fileCompleter);
    outputFile2->setCompleter(fileCompleter);
    outputFile3->setCompleter(fileCompleter);
    outputFile4->setCompleter(fileCompleter);
    outputFile5->setCompleter(fileCompleter);
    outputFile6->setCompleter(fileCompleter);
    outputFile7->setCompleter(fileCompleter);
    outputFile8->setCompleter(fileCompleter);
    outputFile9->setCompleter(fileCompleter);
    inputFile->setCompleter(fileCompleter);

    QCompleter *dirCompleter = new QCompleter(this);
    QFileSystemModel *dirModel = new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    outputDirectory->setCompleter(dirCompleter);

    btnAbort->setVisible(false);

    connect(btnPlot, SIGNAL(clicked(bool)), this, SLOT(btnPlotClicked()));
    connect(btnConvert, SIGNAL(clicked(bool)), this, SLOT(btnConvertClicked()));
    connect(btnOptions, SIGNAL(clicked(bool)), this, SLOT(btnOptionsClicked()));
    connect(btnExit, SIGNAL(clicked(bool)), this, SLOT(btnExitClicked()));
    connect(btnAbout, SIGNAL(clicked(bool)), this, SLOT(btnAboutClicked()));
    connect(btnTimeStart, SIGNAL(clicked(bool)), this, SLOT(btnTimeStartClicked()));
    connect(btnTimeStop, SIGNAL(clicked(bool)), this, SLOT(btnTimeStopClicked()));
    connect(btnInputFile, SIGNAL(clicked(bool)), this, SLOT(btnInputFileClicked()));
    connect(btnInputFileView, SIGNAL(clicked(bool)), this, SLOT(btnInputFileViewClicked()));
    connect(btnOutputFile1, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile1Clicked()));
    connect(btnOutputFile2, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile2Clicked()));
    connect(btnOutputFile3, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile3Clicked()));
    connect(btnOutputFile4, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile4Clicked()));
    connect(btnOutputFile5, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile5Clicked()));
    connect(btnOutputFile6, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile6Clicked()));
    connect(btnOutputFile7, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile7Clicked()));
    connect(btnOutputFile8, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile8Clicked()));
    connect(btnOutputFile9, SIGNAL(clicked(bool)), this, SLOT(btnOutputFile9Clicked()));
    connect(btnOutputFileView1, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView1Clicked()));
    connect(btnOutputFileView2, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView2Clicked()));
    connect(btnOutputFileView3, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView3Clicked()));
    connect(btnOutputFileView4, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView4Clicked()));
    connect(btnOutputFileView5, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView5Clicked()));
    connect(btnOutputFileView6, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView6Clicked()));
    connect(btnOutputFileView7, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView7Clicked()));
    connect(btnOutputFileView8, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView8Clicked()));
    connect(btnOutputFileView9, SIGNAL(clicked(bool)), this, SLOT(btnOutputFileView9Clicked()));
    connect(btnAbort, SIGNAL(clicked(bool)), this, SLOT(btnAbortClicked()));
    connect(cBTimeStart, SIGNAL(clicked(bool)), this, SLOT(timeStartClicked()));
    connect(cBTimeEnd, SIGNAL(clicked(bool)), this, SLOT(timeEndClicked()));
    connect(cBTimeInterval, SIGNAL(clicked(bool)), this, SLOT(TimeIntervalClicked()));
    connect(cBTimeUnit, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputDirectoryEnable, SIGNAL(clicked(bool)), this, SLOT(outputDirrectoryEnableClicked()));
    connect(inputFile, SIGNAL(currentIndexChanged(int)), this, SLOT(inputFileChanged()));
    connect(inputFile, SIGNAL(editTextChanged(QString)), this, SLOT(inputFileChanged()));
    connect(cBFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(formatChanged()));
    connect(outputDirectory, SIGNAL(editingFinished()), this, SLOT(outputDirectoryChanged()));
    connect(btnOutputDirectory, SIGNAL(clicked(bool)), this, SLOT(btnOutputDirrectoryClicked()));
    connect(btnKey, SIGNAL(clicked(bool)), this, SLOT(btnKeyClicked()));
    connect(btnPost, SIGNAL(clicked(bool)), this, SLOT(btnPostClicked()));
    connect(outputFileEnable1, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable2, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable3, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable4, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable5, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable6, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable7, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable8, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(outputFileEnable9, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));

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
        iniFile = parser.value(iniFileOption);

    loadOptions();

    if (parser.isSet(titleOption))
        setWindowTitle(parser.value(titleOption));
}
// callback on form close ---------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *)
{
    saveOptions();
}
// set output file paths ----------------------------------------------------
void MainWindow::setOutputFiles(const QString &infile)
{
    QLineEdit *edit[] = {
        outputFile1, outputFile2, outputFile3, outputFile4, outputFile5, outputFile6, outputFile7, outputFile8, outputFile9
    };
    QString format_Text = cBFormat->currentText();
    QString outputDirectory_Text = outputDirectory->text();
    QString ofile[10];

    if (outputDirectoryEnable->isChecked()) {
        QFileInfo info(infile);

        ofile[0] = outputDirectory_Text + "/" + info.fileName();
    } else {
        ofile[0] = infile;
    }
    ofile[0].replace('*', '0');
    ofile[0].replace('?', '0');

    if (!rinexFile) {
        QFileInfo info(ofile[0]);
        ofile[0] = info.absolutePath() + "/" + info.baseName();
        ofile[1] = ofile[0] + ".obs";
        ofile[2] = ofile[0] + ".nav";
        ofile[3] = ofile[0] + ".gnav";
        ofile[4] = ofile[0] + ".hnav";
        ofile[5] = ofile[0] + ".qnav";
        ofile[6] = ofile[0] + ".lnav";
        ofile[7] = ofile[0] + ".cnav";
        ofile[8] = ofile[0] + ".inav";
        ofile[9] = ofile[0] + ".sbs";
    } else {
        QFileInfo info(ofile[0]);
        ofile[0] = info.filePath() + "/";
        ofile[1] += ofile[0] + QString("%%r%%n0.%%yO");
        if (rinexVersion >= 3 && navSys && (navSys != SYS_GPS)) /* ver.3 and mixed system */
            ofile[2] += ofile[0] + "%%r%%n0.%%yP";
        else
            ofile[2] += ofile[0] + "%%r%%n0.%%yN";
        ofile[3] += ofile[0] + "%%r%%n0.%%yG";
        ofile[4] += ofile[0] + "%%r%%n0.%%yH";
        ofile[5] += ofile[0] + "%%r%%n0.%%yQ";
        ofile[6] += ofile[0] + "%%r%%n0.%%yL";
        ofile[7] += ofile[0] + "%%r%%n0.%%yC";
        ofile[8] += ofile[0] + "%%r%%n0.%%yI";
        ofile[9] += ofile[0] + "%%r%%n0_%%y.sbs";
    }
    for (int i = 0; i < 9; i++) {
        if (ofile[i + 1] == infile) ofile[i + 1] += "_";
        ofile[i + 1] = QDir::toNativeSeparators(ofile[i + 1]);
        edit[i]->setText(ofile[i + 1]);
    }
}
// callback on file drag and drop -------------------------------------------
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    trace(3, "dragEnterEvent\n");

    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

// callback on file drag and drop -------------------------------------------
void MainWindow::dropEvent(QDropEvent *event)
{
    trace(3, "dropEvent\n");

    if (!event->mimeData()->hasFormat("text/uri-list")) return;

    QString file = QDir::toNativeSeparators(QUrl(event->mimeData()->text()).toLocalFile());

    inputFile->setCurrentText(file);
    setOutputFiles(inputFile->currentText());
}
// add history --------------------------------------------------------------
void MainWindow::addHistory(QComboBox *combo)
{
    QString history = combo->currentText();

    if (history == "") return;

    int i = combo->currentIndex();
    if (i >= 0) combo->removeItem(i);

    combo->insertItem(0, history);
    for (int i = combo->count() - 1; i >= MAXHIST; i--) combo->removeItem(i);

    combo->setCurrentIndex(0);
}
// read history -------------------------------------------------------------
void MainWindow::readList(QComboBox *combo, QSettings *ini, const QString &key)
{
    QString item;
    int i;

    for (i = 0; i < 100; i++) {
        item = ini->value(QString("%1_%2").arg(key).arg(i, 3), "").toString();
        if (item != "" && combo->findText(item)==-1) combo->addItem(item);
    }
}
// write history ------------------------------------------------------------
void MainWindow::writeList(QSettings *ini, const QString &key, const QComboBox *combo)
{
    int i;

    for (i = 0; i < combo->count(); i++)
        ini->setValue(QString("%1_%2").arg(key).arg(i, 3), combo->itemText(i));
}
// callback on button-plot --------------------------------------------------
void MainWindow::btnPlotClicked()
{
    QString file1 = outputFile1->text();
    QString file2 = outputFile2->text();
    QString file3 = outputFile3->text();
    QString file4 = outputFile4->text();
    QString file5 = outputFile5->text();
    QString file6 = outputFile6->text();
    QString file7 = outputFile7->text();
    QString file8 = outputFile8->text();
    QString file[] = { file1, file2, file3, file4, file5, file6, file7, file8};
    QString cmd1 = "rtkplot_qt", cmd2 = "..\\..\\..\\bin\\rtkplot_qt", cmd3 = "..\\rtkplot_qt\\rtkplot_qt";
    QStringList opts;

    opts << " -r";
    QCheckBox *cb[] = {
        outputFileEnable1, outputFileEnable2, outputFileEnable3, outputFileEnable4, outputFileEnable5, outputFileEnable6, outputFileEnable7, outputFileEnable8
    };
    int i, ena[8];

    for (i = 0; i < 8; i++) ena[i] = cb[i]->isEnabled() && cb[i]->isChecked();

    for (i = 0; i < 8; i++)
        if (ena[i]) opts << " \"" + repPath(file[i]) + "\"";
    if (opts.size() == 1) return;

    if (!execCommand(cmd1, opts) && !execCommand(cmd2, opts) && !execCommand(cmd3, opts))
        message->setText(tr("error : rtkplot_qt execution"));
}
// callback on button-post-proc ---------------------------------------------
void MainWindow::btnPostClicked()
{
    QString cmd1 = commandPostExe, cmd2 = QString("..\\..\\..\\bin\\") + commandPostExe, cmd3 = QString("..\\rtkpost_qt\\") + commandPostExe;
    QStringList opts;

    if (!outputFileEnable1->isChecked()) return;

    opts << + " -r \"" + outputFile1->text() + "\"";
    opts << " -n \"\" -n \"\"";

    if (outputFileEnable9->isChecked())
        opts << " -n \"" + outputFile9->text() + "\"";

    if (cBTimeStart->isChecked()) opts << + " -ts " + dateTime1->dateTime().toString("yyyy/MM/dd hh:mm:ss");
    if (cBTimeEnd->isChecked()) opts << " -te " + dateTime2->dateTime().toString("yyyy/MM/dd hh:mm:ss");
    if (cBTimeInterval->isChecked()) opts << " -ti " + comboTimeInterval->currentText();
    if (cBTimeUnit->isChecked()) opts << " -tu " + cBTimeUnit->text();

    if (!execCommand(cmd1, opts) && !execCommand(cmd2, opts) && !execCommand(cmd3, opts))
        message->setText(tr("error : rtkpost_qt execution"));
}
// callback on button-options -----------------------------------------------
void MainWindow::btnOptionsClicked()
{
    int rnxfile = rinexFile;

    convOptDialog->exec();
    if (convOptDialog->result() != QDialog::Accepted) return;
    if (rinexFile != rnxfile)
        setOutputFiles(inputFile->currentText());
    updateEnable();
}
//---------------------------------------------------------------------------
void MainWindow::btnAbortClicked()
{
    abortf = 1;
}
// callback on button-convert -----------------------------------------------
void MainWindow::btnConvertClicked()
{
    convertFile();
}
// callback on button-exit --------------------------------------------------
void MainWindow::btnExitClicked()
{
    close();
}
// callbck on button-time-1 -------------------------------------------------
void MainWindow::btnTimeStartClicked()
{
    gtime_t ts = { 0, 0 }, te = { 0, 0 };
    double tint = 0.0, tunit = 0.0;

    getTime(&ts, &te, &tint, &tunit);
    timeDialog->time = ts;
    timeDialog->exec();
}
// callbck on button-time-2 -------------------------------------------------
void MainWindow::btnTimeStopClicked()
{
    gtime_t ts = { 0, 0 }, te = { 0, 0 };
    double tint = 0.0, tunit = 0.0;

    getTime(&ts, &te, &tint, &tunit);
    timeDialog->time = te;
    timeDialog->exec();
}
// callback on button-input-file --------------------------------------------
void MainWindow::btnInputFileClicked()
{
    inputFile->setCurrentText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Input RTCM, RCV RAW or RINEX File"), QString(),
                                             tr("All (*.*);;RTCM 2 (*.rtcm2);;RTCM 3 (*.rtcm3);;NovtAtel (*.gps);;ublox (*.ubx);;SuperStart II (*.log);;"
                                            "Hemisphere (*.bin);;Javad (*.jps);;RINEX OBS (*.obs *.*O);Septentrio (*.sbf)"))));
    setOutputFiles(inputFile->currentText());
}
// callback on output-directory change --------------------------------------
void MainWindow::outputDirectoryChanged()
{
    setOutputFiles(inputFile->currentText());
}
// callback on button-output-directory --------------------------------------
void MainWindow::btnOutputDirrectoryClicked()
{
    outputDirectory->setText(QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Output Directory"), outputDirectory->text())));
    setOutputFiles(inputFile->currentText());
}
// callback on button-keyword -----------------------------------------------
void MainWindow::btnKeyClicked()
{
    keyDialog->Flag = 1;
    keyDialog->show();
}
// callback on button-output-file-1 -----------------------------------------
void MainWindow::btnOutputFile1Clicked()
{
    QString selectedFilter = tr("RINEX OBS (*.obs *.*O)");

    outputFile1->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX OBS File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-2 -----------------------------------------
void MainWindow::btnOutputFile2Clicked()
{
    QString selectedFilter = tr("RINEX NAV (*.nav *.*N *.*P)");

    outputFile2->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX NAV File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-3 -----------------------------------------
void MainWindow::btnOutputFile3Clicked()
{
    QString selectedFilter = tr("RINEX GNAV (*.gnav *.*G)");

    outputFile3->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX GNAV File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-4 -----------------------------------------
void MainWindow::btnOutputFile4Clicked()
{
    QString selectedFilter = tr("RINEX HNAV (*.hnav *.*H)");

    outputFile4->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX HNAV File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-5 -----------------------------------------
void MainWindow::btnOutputFile5Clicked()
{
    QString selectedFilter = tr("RINEX QNAV (*.qnav *.*Q)");

    outputFile5->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX QNAV File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-6 -----------------------------------------
void MainWindow::btnOutputFile6Clicked()
{
    QString selectedFilter = tr("RINEX LNAV (*.lnav *.*L)");

    outputFile6->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output RINEX LNAV File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-7 -----------------------------------------
void MainWindow::btnOutputFile7Clicked()
{
    QString selectedFilter = tr("RINEX CNAV (*.cnav *.*C)");

    outputFile7->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output SRINEX CNAVFile"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-8 -----------------------------------------
void MainWindow::btnOutputFile8Clicked()
{
    QString selectedFilter = tr("RINEX INAV (*.inav *.*I)");

    outputFile7->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output SBAS/LEX Log File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-output-file-9 -----------------------------------------
void MainWindow::btnOutputFile9Clicked()
{
    QString selectedFilter = tr("SBAS Log (*.sbs)");

    outputFile7->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Output SBAS/LEX Log File"), QString(),
                                        tr("All (*.*);;RINEX OBS (*.obs *.*O);;RINEX NAV (*.nav *.*N *.*P);;RINEX GNAV (*.gnav *.*G);;RINEX HNAV (*.hnav *.*H);;"
                                           "RINEX QNAV (*.qnav *.*Q);;RINEX LNAV (*.lnav *.*L);;RINEX CNAV (*.cnav *.*C);;RINEX INAV (*.inav *.*I);;"
                                           "SBAS Log (*.sbs);;LEX Log (*.lex)"), &selectedFilter)));
}
// callback on button-view-input-file ----------------------------------------
void MainWindow::btnInputFileViewClicked()
{
    QString InFile_Text = inputFile->currentText();
    QString ext = QFileInfo(InFile_Text).suffix();

    if (ext.length() < 3) return;

    if ((ext.toLower() == "obs") || (ext.toLower() == "nav") ||
        (ext.mid(1).toLower() == "nav") ||
        (ext.at(2) == 'o') || (ext.at(2) == 'O') || (ext.at(2) == 'n') ||
        (ext.at(2) == 'N') || (ext.at(2) == 'p') || (ext.at(2) == 'P') ||
        (ext.at(2) == 'g') || (ext.at(2) == 'G') || (ext.at(2) == 'h') ||
        (ext.at(2) == 'H') || (ext.at(2) == 'q') || (ext.at(2) == 'Q') ||
        (ext.at(2) == 'l') || (ext.at(2) == 'L') || (ext.at(2) == 'c') ||
        (ext.at(2) == 'C') || (ext.at(2) == 'I') || (ext.at(2) == 'i') ||
            (ext == "rnx") || (ext == "RNX")) {
        viewer->show();
        viewer->read(repPath(InFile_Text));
    }
}
// callback on button-view-file-1 -------------------------------------------
void MainWindow::btnOutputFileView1Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile1_Text = outputFile1->text();

    viewer->show();
    viewer->read(repPath(outputFile1_Text));
}
// callback on button-view-file-2 -------------------------------------------
void MainWindow::btnOutputFileView2Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile2_Text = outputFile2->text();

    viewer->show();
    viewer->read(repPath(outputFile2_Text));
}
// callback on button-view-file-3 -------------------------------------------
void MainWindow::btnOutputFileView3Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile3_Text = outputFile3->text();

    viewer->show();
    viewer->read(repPath(outputFile3_Text));
}
// callback on button-view-file-4 -------------------------------------------
void MainWindow::btnOutputFileView4Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile4_Text = outputFile4->text();

    viewer->show();
    viewer->read(repPath(outputFile4_Text));
}
// callback on button-view-file-5 -------------------------------------------
void MainWindow::btnOutputFileView5Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile5_Text = outputFile5->text();

    viewer->show();
    viewer->read(repPath(outputFile5_Text));
}
// callback on button-view-file-6 -------------------------------------------
void MainWindow::btnOutputFileView6Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile6_Text = outputFile6->text();

    viewer->show();
    viewer->read(repPath(outputFile6_Text));
}
// callback on button-view-file-7 -------------------------------------------
void MainWindow::btnOutputFileView7Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile7_Text = outputFile7->text();

    viewer->show();
    viewer->read(repPath(outputFile7_Text));
}
// callback on button-view-file-8 -------------------------------------------
void MainWindow::btnOutputFileView8Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile8_Text = outputFile8->text();

    viewer->show();
    viewer->read(repPath(outputFile8_Text));
}
// callback on button-view-file-9 -------------------------------------------
void MainWindow::btnOutputFileView9Clicked()
{
    TextViewer *viewer = new TextViewer(this);
    QString outputFile9_Text = outputFile9->text();

    viewer->show();
    viewer->read(repPath(outputFile9_Text));
}
// callback on button-about -------------------------------------------------
void MainWindow::btnAboutClicked()
{
    aboutDialog->aboutString = PRGNAME;
    aboutDialog->iconIndex = 3;
    aboutDialog->exec();
}
// callback on button-time-start --------------------------------------------
void MainWindow::timeStartClicked()
{
    updateEnable();
}
// callback on button-time-end ----------------------------------------------
void MainWindow::timeEndClicked()
{
    updateEnable();
}
// callback on button-time-interval -----------------------------------------
void MainWindow::TimeIntervalClicked()
{
    updateEnable();
}
// callback on output-file check/uncheck ------------------------------------
void MainWindow::outputDirrectoryEnableClicked()
{
    setOutputFiles(inputFile->currentText());
    updateEnable();
}
// callback on input-file-change --------------------------------------------
void MainWindow::inputFileChanged()
{
    setOutputFiles(inputFile->currentText());
}
// callback on format change ------------------------------------------------
void MainWindow::formatChanged()
{
    updateEnable();
}
// get time -----------------------------------------------------------------
void MainWindow::getTime(gtime_t *ts, gtime_t *te, double *tint,
             double *tunit)
{
    if (cBTimeStart->isChecked()) {
        QDateTime start(dateTime1->dateTime());
        ts->time = start.toSecsSinceEpoch(); ts->sec = start.time().msec() / 1000;
    } else {
        ts->time = 0; ts->sec = 0;
    }

    if (cBTimeEnd->isChecked()) {
        QDateTime end(dateTime2->dateTime());
        te->time = end.toSecsSinceEpoch(); te->sec = end.time().msec() / 1000;
    } else {
        te->time = 0; te->sec = 0;
    }

    if (cBTimeInterval->isChecked())
        *tint = comboTimeInterval->currentText().toDouble();
    else *tint = 0;

    if (cBTimeUnit->isChecked())
        *tunit = cBTimeUnit->text().toDouble() * 3600;
    else *tunit = 0;
}
// replace keywords in file path --------------------------------------------
QString MainWindow::repPath(const QString &File)
{
    QString Path;
    char path[1024];

    reppath(qPrintable(File), path, timeadd(rinexTime, TSTARTMARGIN), qPrintable(rinexStationCode), "");
    return Path = path;
}
// execute command ----------------------------------------------------------
int MainWindow::execCommand(const QString &cmd, QStringList &opt)
{
    return QProcess::startDetached(cmd, opt);
}
// update enable/disable of widgets -----------------------------------------
void MainWindow::updateEnable(void)
{
    QString FormatText = cBFormat->currentText();
    int rnx = FormatText == "RINEX";
    int sep_nav=(rinexVersion<3||separateNavigation);

    dateTime1->setEnabled(cBTimeStart->isChecked());
    btnTimeStart->setEnabled(cBTimeStart->isChecked());
    dateTime2->setEnabled(cBTimeEnd->isChecked());
    btnTimeStop->setEnabled(cBTimeEnd->isChecked());
    comboTimeInterval->setEnabled(cBTimeInterval->isChecked());
    lbTimeInterval->setEnabled(cBTimeInterval->isEnabled());
    cBTimeUnit->setEnabled(cBTimeStart->isChecked() && cBTimeEnd->isChecked());
    lETimeUnit->setEnabled(cBTimeStart->isChecked() && cBTimeEnd->isChecked() && cBTimeUnit->isChecked());
    lbTimeUnit->setEnabled(cBTimeUnit->isEnabled());
    outputFileEnable3->setEnabled(sep_nav && (navSys & SYS_GLO));
    outputFileEnable4->setEnabled(sep_nav && (navSys & SYS_SBS));
    outputFileEnable5->setEnabled(sep_nav && (navSys & SYS_QZS) && rinexVersion>=5);
    outputFileEnable6->setEnabled(sep_nav && (navSys & SYS_GAL) && rinexVersion>=2);
    outputFileEnable7->setEnabled(sep_nav && (navSys & SYS_CMP) && rinexVersion>=4);
    outputFileEnable8->setEnabled(sep_nav && (navSys & SYS_IRN) && rinexVersion>=6);
    outputFileEnable9->setEnabled(!rnx);
    outputDirectory->setEnabled(outputDirectoryEnable->isChecked());
    lbOutputDirectory->setEnabled(outputDirectoryEnable->isChecked());
    outputFile1->setEnabled(outputFileEnable1->isChecked());
    outputFile2->setEnabled(outputFileEnable2->isChecked());
    outputFile3->setEnabled(outputFileEnable3->isChecked() && outputFileEnable3->isEnabled());
    outputFile4->setEnabled(outputFileEnable4->isChecked() && outputFileEnable4->isEnabled());
    outputFile5->setEnabled(outputFileEnable5->isChecked() && outputFileEnable5->isEnabled());
    outputFile6->setEnabled(outputFileEnable6->isChecked() && outputFileEnable6->isEnabled());
    outputFile7->setEnabled(outputFileEnable7->isChecked() && outputFileEnable7->isEnabled());
    outputFile8->setEnabled(outputFileEnable8->isChecked() && outputFileEnable8->isEnabled());
    outputFile9->setEnabled(outputFileEnable9->isChecked() && !rnx);
    btnOutputDirectory->setEnabled(outputDirectoryEnable->isChecked());
    btnOutputFile1->setEnabled(outputFile1->isEnabled());
    btnOutputFile2->setEnabled(outputFile2->isEnabled());
    btnOutputFile3->setEnabled(outputFile3->isEnabled());
    btnOutputFile4->setEnabled(outputFile4->isEnabled());
    btnOutputFile5->setEnabled(outputFile5->isEnabled());
    btnOutputFile6->setEnabled(outputFile6->isEnabled());
    btnOutputFile7->setEnabled(outputFile7->isEnabled());
    btnOutputFile8->setEnabled(outputFile8->isEnabled());
    btnOutputFile9->setEnabled(outputFile9->isEnabled());
    btnOutputFileView1->setEnabled(outputFile1->isEnabled());
    btnOutputFileView2->setEnabled(outputFile2->isEnabled());
    btnOutputFileView3->setEnabled(outputFile3->isEnabled());
    btnOutputFileView4->setEnabled(outputFile4->isEnabled());
    btnOutputFileView5->setEnabled(outputFile5->isEnabled());
    btnOutputFileView6->setEnabled(outputFile6->isEnabled());
    btnOutputFileView7->setEnabled(outputFile7->isEnabled());
    btnOutputFileView8->setEnabled(outputFile8->isEnabled());
    btnOutputFileView9->setEnabled(outputFile9->isEnabled());
}
// convert file -------------------------------------------------------------
void MainWindow::convertFile(void)
{
    QString inputFile_Text = inputFile->currentText();
    QString outputFile1_Text = outputFile1->text(), outputFile2_Text = outputFile2->text();
    QString outputFile3_Text = outputFile3->text(), outputFile4_Text = outputFile4->text();
    QString outputFile5_Text = outputFile5->text(), outputFile6_Text = outputFile6->text();
    QString outputFile7_Text = outputFile7->text(), outputFile8_Text = outputFile8->text();
    QString outputFile9_Text = outputFile9->text();
    int i;
    double RNXVER[] = { 210, 211, 212, 300, 301, 302, 303, 304 };

    conversionThread = new ConversionThread(this);

    // recognize input file format
    strcpy(conversionThread->ifile, qPrintable(inputFile_Text));
    QFileInfo fi(inputFile_Text);
    if (cBFormat->currentIndex() == 0) { // auto
        if (fi.completeSuffix() == "rtcm2") {
            conversionThread->format = STRFMT_RTCM2;
        } else if (fi.completeSuffix() == "rtcm3") {
            conversionThread->format = STRFMT_RTCM3;
        } else if (fi.completeSuffix() == "gps") {
            conversionThread->format = STRFMT_OEM4;
        } else if (fi.completeSuffix() == "ubx") {
            conversionThread->format = STRFMT_UBX;
        } else if (fi.completeSuffix() == "log") {
            conversionThread->format = STRFMT_SS2;
        } else if (fi.completeSuffix() == "bin") {
            conversionThread->format = STRFMT_CRES;
        } else if (fi.completeSuffix() == "jps") {
            conversionThread->format = STRFMT_JAVAD;
        } else if (fi.completeSuffix() == "bnx") {
            conversionThread->format = STRFMT_BINEX;
        } else if (fi.completeSuffix() == "binex") {
            conversionThread->format = STRFMT_BINEX;
        } else if (fi.completeSuffix() == "rt17") {
            conversionThread->format = STRFMT_RT17;
        } else if (fi.completeSuffix() == "sfb") {
            conversionThread->format = STRFMT_SEPT;
        } else if (fi.completeSuffix().toLower() == "obs") {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().toLower().contains("nav")) {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'o') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'O') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'n') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'N') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'p') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'P') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'g') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'G') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'h') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'H') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'q') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'Q') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'l') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'L') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'c') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'C') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'i') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix().at(2) == 'I') {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix() == "rnx") {
            conversionThread->format = STRFMT_RINEX;
        } else if (fi.completeSuffix() == "RNX") {
            conversionThread->format = STRFMT_RINEX;
        } else {
            showmsg("file format can not be recognized");
            return;
        }
    } else {
        for (i = 0; formatstrs[i]; i++)
            if (cBFormat->currentText() == formatstrs[i]) break;
        if (formatstrs[i]) conversionThread->format = i;
        else return;
    }
    conversionThread->rnxopt.rnxver = RNXVER[rinexVersion];

    if (conversionThread->format == STRFMT_RTCM2 || conversionThread->format == STRFMT_RTCM3 || conversionThread->format == STRFMT_RT17) {
        // input start date/time for rtcm 2, rtcm 3, RT17 or CMR
        startDialog->exec();
        if (startDialog->result() != QDialog::Accepted) return;
        conversionThread->rnxopt.trtcm = startDialog->time;
    }
    if (outputFile1->isEnabled() && outputFileEnable1->isChecked()) strcpy(conversionThread->ofile[0], qPrintable(outputFile1_Text));
    if (outputFile2->isEnabled() && outputFileEnable2->isChecked()) strcpy(conversionThread->ofile[1], qPrintable(outputFile2_Text));
    if (outputFile3->isEnabled() && outputFileEnable3->isChecked()) strcpy(conversionThread->ofile[2], qPrintable(outputFile3_Text));
    if (outputFile4->isEnabled() && outputFileEnable4->isChecked()) strcpy(conversionThread->ofile[3], qPrintable(outputFile4_Text));
    if (outputFile5->isEnabled() && outputFileEnable5->isChecked()) strcpy(conversionThread->ofile[4], qPrintable(outputFile5_Text));
    if (outputFile6->isEnabled() && outputFileEnable6->isChecked()) strcpy(conversionThread->ofile[5], qPrintable(outputFile6_Text));
    if (outputFile7->isEnabled() && outputFileEnable7->isChecked()) strcpy(conversionThread->ofile[6], qPrintable(outputFile7_Text));
    if (outputFile8->isEnabled() && outputFileEnable8->isChecked()) strcpy(conversionThread->ofile[7], qPrintable(outputFile8_Text));
    if (outputFile9->isEnabled() && outputFileEnable9->isChecked()) strcpy(conversionThread->ofile[8], qPrintable(outputFile9_Text));

    // check overwrite output file
    for (i = 0; i < 9; i++) {
        if (!QFile(conversionThread->ofile[i]).exists()) continue;
        if (QMessageBox::question(this, tr("Overwrite"), QString(tr("%1 exists. Do you want to overwrite?")).arg(conversionThread->ofile[i])) != QMessageBox::Yes) return;
    }
    getTime(&conversionThread->rnxopt.ts, &conversionThread->rnxopt.te, &conversionThread->rnxopt.tint, &conversionThread->rnxopt.tunit);
    strncpy(conversionThread->rnxopt.staid, qPrintable(rinexStationCode), 31);
    sprintf(conversionThread->rnxopt.prog, "%s %s %s", PRGNAME, VER_RTKLIB, PATCH_LEVEL);
    strncpy(conversionThread->rnxopt.runby, qPrintable(runBy), 31);
    strncpy(conversionThread->rnxopt.marker, qPrintable(marker), 63);
    strncpy(conversionThread->rnxopt.markerno, qPrintable(markerNo), 31);
    strncpy(conversionThread->rnxopt.markertype, qPrintable(markerType), 31);
    for (i = 0; i < 2; i++) strncpy(conversionThread->rnxopt.name[i], qPrintable(name[i]), 31);
    for (i = 0; i < 3; i++) strncpy(conversionThread->rnxopt.rec [i], qPrintable(receiver [i]), 31);
    for (i = 0; i < 3; i++) strncpy(conversionThread->rnxopt.ant [i], qPrintable(antenna [i]), 31);
    if (autoPosition)
        for (i = 0; i < 3; i++) conversionThread->rnxopt.apppos[i] = approxPosition[i];
    for (i = 0; i < 3; i++) conversionThread->rnxopt.antdel[i] = antennaDelta[i];
    strncpy(conversionThread->rnxopt.rcvopt, qPrintable(receiverOptions), 255);
    conversionThread->rnxopt.navsys = navSys;
    conversionThread->rnxopt.obstype = observationType;
    conversionThread->rnxopt.freqtype = frequencyType;
    for (i=0;i<2;i++) sprintf(conversionThread->rnxopt.comment[i],"%.63s",qPrintable(comment[i]));
    for (i = 0; i < 7; i++) strcpy(conversionThread->rnxopt.mask[i], qPrintable(modeMask[i]));
    conversionThread->rnxopt.autopos = autoPosition;
    conversionThread->rnxopt.phshift = phaseShift;
    conversionThread->rnxopt.halfcyc = halfCycle;
    conversionThread->rnxopt.outiono = outputIonoCorr;
    conversionThread->rnxopt.outtime = outputTimeCorr;
    conversionThread->rnxopt.outleaps = outputLeapSeconds;
    conversionThread->rnxopt.sep_nav = separateNavigation;
    conversionThread->rnxopt.ttol = timeTolerance;
    if (enableGlonassFrequency) {
        for (i=0;i<MAXPRNGLO;i++) conversionThread->rnxopt.glofcn[i]=glonassFrequency[i];
    }

    QStringList exsatsLst = excludedSatellites.split(" ");
    foreach(const QString &sat, exsatsLst){
        int satid;

        if (!(satid = satid2no(qPrintable(sat)))) continue;
        conversionThread->rnxopt.exsats[satid - 1] = 1;
    }

    abortf = 0;
    btnConvert->setVisible(false);
    btnAbort->setVisible(true);
    panel1->setEnabled(false);
    panel2->setEnabled(false);
    btnPlot->setEnabled(false);
    btnPost->setEnabled(false);
    btnOptions->setEnabled(false);
    btnExit->setEnabled(false);
    cBFormat->setEnabled(false);
    btnKey->setEnabled(false);
    lbInputFile->setEnabled(false);
    lbOutputDirectory->setEnabled(false);
    lbOutputFile->setEnabled(false);
    lbFormat->setEnabled(false);
    message->setText("");

    if (traceLevel > 0) {
        traceopen(TRACEFILE);
        tracelevel(traceLevel);
    }
    setCursor(Qt::WaitCursor);

    // post processing positioning
    connect(conversionThread, SIGNAL(finished()), this, SLOT(conversionFinished()));

    conversionThread->start();
}
// conversion done -------------------------------------------------------------
void MainWindow::conversionFinished()
{
    setCursor(Qt::ArrowCursor);

    if (traceLevel > 0)
        traceclose();
    btnConvert->setVisible(true);
    btnAbort->setVisible(false);
    panel1->setEnabled(true);
    panel2->setEnabled(true);
    btnPlot->setEnabled(true);
    btnPost->setEnabled(true);
    btnOptions->setEnabled(true);
    btnExit->setEnabled(true);
    cBFormat->setEnabled(true);
    btnKey->setEnabled(true);
    lbInputFile->setEnabled(true);
    lbOutputDirectory->setEnabled(true);
    lbOutputFile->setEnabled(true);
    lbFormat->setEnabled(true);

#if 0
    // set time-start/end if time not specified
    if (!TimeStartF->Checked && rnxopt.tstart.time != 0) {
        time2str(rnxopt.tstart, tstr, 0);
        tstr[10] = '\0';
        TimeY1->Text = tstr;
        TimeH1->Text = tstr + 11;
    }
    if (!TimeEndF->Checked && rnxopt.tend.time != 0) {
        time2str(rnxopt.tend, tstr, 0);
        tstr[10] = '\0';
        TimeY2->Text = tstr;
        TimeH2->Text = tstr + 11;
    }
#endif
    rinexTime = conversionThread->rnxopt.tstart;

    conversionThread->deleteLater();

    addHistory(inputFile);
}
// load options -------------------------------------------------------------
void MainWindow::loadOptions(void)
{
    QSettings ini(iniFile, QSettings::IniFormat);
    QString opt, mask = "11111111111111111111111111111111111111111111111111111111111111111111";

    rinexVersion = ini.value("opt/rnxver", 6).toInt();
    rinexFile = ini.value("opt/rnxfile", 0).toInt();
    rinexStationCode = ini.value("opt/rnxcode", "0000").toString();
    runBy = ini.value("opt/runby", "").toString();
    marker = ini.value("opt/marker", "").toString();
    markerNo = ini.value("opt/markerno", "").toString();
    markerType = ini.value("opt/markertype", "").toString();
    name[0] = ini.value("opt/name0", "").toString();
    name[1] = ini.value("opt/name1", "").toString();
    receiver[0] = ini.value("opt/rec0", "").toString();
    receiver[1] = ini.value("opt/rec1", "").toString();
    receiver[2] = ini.value("opt/rec2", "").toString();
    antenna[0] = ini.value("opt/ant0", "").toString();
    antenna[1] = ini.value("opt/ant1", "").toString();
    antenna[2] = ini.value("opt/ant2", "").toString();
    approxPosition[0] = ini.value("opt/apppos0", 0.0).toDouble();
    approxPosition[1] = ini.value("opt/apppos1", 0.0).toDouble();
    approxPosition[2] = ini.value("opt/apppos2", 0.0).toDouble();
    antennaDelta[0] = ini.value("opt/antdel0", 0.0).toDouble();
    antennaDelta[1] = ini.value("opt/antdel1", 0.0).toDouble();
    antennaDelta[2] = ini.value("opt/antdel2", 0.0).toDouble();
    comment[0] = ini.value("opt/comment0", "").toString();
    comment[1] = ini.value("opt/comment1", "").toString();
    receiverOptions = ini.value("opt/rcvoption", "").toString();
    navSys = ini.value("opt/navsys", 0xff).toInt();
    observationType = ini.value("opt/obstype", 0xF).toInt();
    frequencyType = ini.value("opt/freqtype", 0x7).toInt();
    excludedSatellites = ini.value("opt/exsats", "").toString();
    traceLevel = ini.value("opt/tracelevel", 0).toInt();
    rinexTime.time = ini.value("opt/rnxtime", 0).toInt();
    modeMask[0] = ini.value("opt/codemask_1", mask).toString();
    modeMask[1] = ini.value("opt/codemask_2", mask).toString();
    modeMask[2] = ini.value("opt/codemask_3", mask).toString();
    modeMask[3] = ini.value("opt/codemask_4", mask).toString();
    modeMask[4] = ini.value("opt/codemask_5", mask).toString();
    modeMask[5] = ini.value("opt/codemask_6", mask).toString();
    modeMask[6] = ini.value("opt/codemask_7", mask).toString();
    autoPosition = ini.value("opt/autopos", 0).toInt();
    phaseShift = ini.value("opt/phaseShift", 0).toInt();
    halfCycle = ini.value("opt/halfcyc", 0).toInt();
    outputIonoCorr = ini.value("opt/outiono", 0).toInt();
    outputTimeCorr = ini.value("opt/outtime", 0).toInt();
    outputLeapSeconds = ini.value("opt/outleaps", 0).toInt();
    separateNavigation = ini.value("opt/sepnav", 0).toInt();
    timeTolerance	= ini.value("opt/timetol", 0.005).toInt();
    enableGlonassFrequency = ini.value("opt/glofcnena", 0).toInt();
    for (int i=0;i<27;i++)
        glonassFrequency[i]=ini.value(QString("opt/glofcn%1").arg(i+1,2, 10, QLatin1Char('0')),0).toInt();

    cBTimeStart->setChecked(ini.value("set/timestartf", 0).toBool());
    cBTimeEnd->setChecked(ini.value("set/timeendf", 0).toBool());
    cBTimeInterval->setChecked(ini.value("set/timeintf", 0).toBool());
    dateTime1->setDate(ini.value("set/timey1", "2020/01/01").value<QDate>());
    dateTime1->setTime(ini.value("set/timeh1", "00:00:00").value<QTime>());
    dateTime2->setDate(ini.value("set/timey2", "2020/01/01").value<QDate>());
    dateTime2->setTime(ini.value("set/timeh2", "00:00:00").value<QTime>());
    comboTimeInterval->setCurrentText(ini.value("set/timeint", "1").toString());
    cBTimeUnit->setChecked(ini.value("set/timeunitf", 0).toBool());
    lETimeUnit->setText(ini.value("set/timeunit", "24").toString());
    inputFile->setCurrentText(ini.value("set/infile", "").toString());
    outputDirectory->setText(ini.value("set/outdir", "").toString());
    outputFile1->setText(ini.value("set/outfile1", "").toString());
    outputFile2->setText(ini.value("set/outfile2", "").toString());
    outputFile3->setText(ini.value("set/outfile3", "").toString());
    outputFile4->setText(ini.value("set/outfile4", "").toString());
    outputFile5->setText(ini.value("set/outfile5", "").toString());
    outputFile6->setText(ini.value("set/outfile6", "").toString());
    outputFile7->setText(ini.value("set/outfile7", "").toString());
    outputFile8->setText(ini.value("set/outfile8", "").toString());
    outputFile9->setText(ini.value("set/outfile9", "").toString());
    outputDirectoryEnable->setChecked(ini.value("set/outdirena", false).toBool());
    outputFileEnable1->setChecked(ini.value("set/outfileena1", true).toBool());
    outputFileEnable2->setChecked(ini.value("set/outfileena2", true).toBool());
    outputFileEnable3->setChecked(ini.value("set/outfileena3", true).toBool());
    outputFileEnable4->setChecked(ini.value("set/outfileena4", true).toBool());
    outputFileEnable5->setChecked(ini.value("set/outfileena5", true).toBool());
    outputFileEnable6->setChecked(ini.value("set/outfileena6", true).toBool());
    outputFileEnable7->setChecked(ini.value("set/outfileena7", true).toBool());
    outputFileEnable8->setChecked(ini.value("set/outfileena8", true).toBool());
    outputFileEnable9->setChecked(ini.value("set/outfileena9", true).toBool());
    cBFormat->setCurrentIndex(ini.value("set/format", 0).toInt());

    readList(inputFile, &ini, "hist/inputfile");

    TextViewer::colorText = ini.value("viewer/color1", QColor(Qt::black)).value<QColor>();
    TextViewer::colorBackground = ini.value("viewer/color2", QColor(Qt::white)).value<QColor>();
    TextViewer::font.setFamily(ini.value("viewer/fontname", "Courier New").toString());
    TextViewer::font.setPointSize(ini.value("viewer/fontsize", 9).toInt());

    commandPostExe = ini.value("set/cmdpostexe", "rtkpost_qt").toString();

    updateEnable();
}
// save options -------------------------------------------------------------
void MainWindow::saveOptions(void)
{
    QSettings ini(iniFile, QSettings::IniFormat);

    ini.setValue("opt/rnxver", rinexVersion);
    ini.setValue("opt/rnxfile", rinexFile);
    ini.setValue("opt/rnxcode", rinexStationCode);
    ini.setValue("opt/runby", runBy);
    ini.setValue("opt/marker", marker);
    ini.setValue("opt/markerno", markerNo);
    ini.setValue("opt/markertype", markerType);
    ini.setValue("opt/name0", name[0]);
    ini.setValue("opt/name1", name[1]);
    ini.setValue("opt/rec0", receiver[0]);
    ini.setValue("opt/rec1", receiver[1]);
    ini.setValue("opt/rec2", receiver[2]);
    ini.setValue("opt/ant0", antenna[0]);
    ini.setValue("opt/ant1", antenna[1]);
    ini.setValue("opt/ant2", antenna[2]);
    ini.setValue("opt/apppos0", approxPosition[0]);
    ini.setValue("opt/apppos1", approxPosition[1]);
    ini.setValue("opt/apppos2", approxPosition[2]);
    ini.setValue("opt/antdel0", antennaDelta[0]);
    ini.setValue("opt/antdel1", antennaDelta[1]);
    ini.setValue("opt/antdel2", antennaDelta[2]);
    ini.setValue("opt/comment0", comment[0]);
    ini.setValue("opt/comment1", comment[1]);
    ini.setValue("opt/rcvoption", receiverOptions);
    ini.setValue("opt/navsys", navSys);
    ini.setValue("opt/obstype", observationType);
    ini.setValue("opt/freqtype", frequencyType);
    ini.setValue("opt/exsats", excludedSatellites);
    ini.setValue("opt/tracelevel", traceLevel);
    ini.setValue("opt/rnxtime", (int)(rinexTime.time));
    ini.setValue("opt/codemask_1", modeMask[0]);
    ini.setValue("opt/codemask_2", modeMask[1]);
    ini.setValue("opt/codemask_3", modeMask[2]);
    ini.setValue("opt/codemask_4", modeMask[3]);
    ini.setValue("opt/codemask_5", modeMask[4]);
    ini.setValue("opt/codemask_6", modeMask[5]);
    ini.setValue("opt/codemask_7", modeMask[6]);
    ini.setValue("opt/autopos", autoPosition);
    ini.setValue("opt/phasewhift", phaseShift);
    ini.setValue("opt/halfcyc", halfCycle);
    ini.setValue("opt/outiono", outputIonoCorr);
    ini.setValue("opt/outtime", outputTimeCorr);
    ini.setValue("opt/outleaps", outputLeapSeconds);
    ini.setValue("opt/sepnav", separateNavigation);
    ini.setValue("opt/timetol", timeTolerance);
    ini.setValue("opt/glofcnena",  enableGlonassFrequency);
    for (int i=0;i<27;i++) {
        ini.setValue(QString("opt/glofcn%1").arg(i+1,2,10,QLatin1Char('0')),glonassFrequency[i]);
    }

    ini.setValue("set/timestartf", cBTimeStart->isChecked());
    ini.setValue("set/timeendf", cBTimeEnd->isChecked());
    ini.setValue("set/timeintf", cBTimeInterval->isChecked());
    ini.setValue("set/timey1", dateTime1->date());
    ini.setValue("set/timeh1", dateTime1->time());
    ini.setValue("set/timey2", dateTime2->date());
    ini.setValue("set/timeh2", dateTime2->time());
    ini.setValue("set/timeint", comboTimeInterval->currentText());
    ini.setValue("set/timeunitf", cBTimeUnit->isChecked());
    ini.setValue("set/timeunit", lETimeUnit->text());
    ini.setValue("set/infile", inputFile->currentText());
    ini.setValue("set/outdir", outputDirectory->text());
    ini.setValue("set/outfile1", outputFile1->text());
    ini.setValue("set/outfile2", outputFile2->text());
    ini.setValue("set/outfile3", outputFile3->text());
    ini.setValue("set/outfile4", outputFile4->text());
    ini.setValue("set/outfile5", outputFile5->text());
    ini.setValue("set/outfile6", outputFile6->text());
    ini.setValue("set/outfile7", outputFile7->text());
    ini.setValue("set/outfile8", outputFile8->text());
    ini.setValue("set/outfile9", outputFile9->text());
    ini.setValue("set/outdirena", outputDirectoryEnable->isChecked());
    ini.setValue("set/outfileena1", outputFileEnable1->isChecked());
    ini.setValue("set/outfileena2", outputFileEnable2->isChecked());
    ini.setValue("set/outfileena3", outputFileEnable3->isChecked());
    ini.setValue("set/outfileena4", outputFileEnable4->isChecked());
    ini.setValue("set/outfileena5", outputFileEnable5->isChecked());
    ini.setValue("set/outfileena6", outputFileEnable6->isChecked());
    ini.setValue("set/outfileena7", outputFileEnable7->isChecked());
    ini.setValue("set/outfileena8", outputFileEnable8->isChecked());
    ini.setValue("set/outfileena9", outputFileEnable9->isChecked());
    ini.setValue("set/format", cBFormat->currentIndex());

    writeList(&ini, "hist/inputfile", inputFile);

    ini.setValue("viewer/color1", TextViewer::colorText);
    ini.setValue("viewer/color2", TextViewer::colorText);
    ini.setValue("viewer/fontname", TextViewer::font.family());
    ini.setValue("viewer/fontsize", TextViewer::font.pointSize());
}
//---------------------------------------------------------------------------
