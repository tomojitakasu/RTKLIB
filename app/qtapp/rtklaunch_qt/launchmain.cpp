//---------------------------------------------------------------------------
// rtklaunch_qt : rtklib app launcher
//
//          Copyright (C) 2013-2016 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtklib launcher [-t title][-tray]
//
//           -t title   window title
//           -tray      start as task tray icon
//
// version : $Revision:$ $Date:$
// history : 2013/01/10  1.1 new
//---------------------------------------------------------------------------

#include <QSettings>
#include <QFileInfo>
#include <QShowEvent>
#include <QCommandLineParser>
#include <QProcess>
#include <QMenu>

#include "rtklib.h"
#include "launchmain.h"
#include "launchoptdlg.h"

//---------------------------------------------------------------------------

#define BTN_SIZE        42
#define BTN_COUNT       8
#define MAX(x, y)        ((x) > (y) ? (x) : (y))

MainForm *mainForm;

//---------------------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    mainForm = this;

    QString prgFilename = QApplication::applicationFilePath();
    QFileInfo prgFileInfo(prgFilename);
    iniFile = prgFileInfo.absolutePath() + "/" + prgFileInfo.baseName() + ".ini";
    option = 0;
    minimize = 0;

    launchOptDlg = new LaunchOptDialog(this);

    setWindowTitle(tr("RTKLIB v.%1 %2").arg(VER_RTKLIB).arg(PATCH_LEVEL));
    setWindowIcon(QIcon(":/icons/rtk9.bmp"));

    QCoreApplication::setApplicationName("rtklaunch_qt");
    QCoreApplication::setApplicationVersion("1.0");

    QSettings settings(iniFile, QSettings::IniFormat);
    option =  settings.value("pos/option", 0).toInt();
    minimize =  settings.value("pos/minimize", 1).toInt();
    BtnRtklib->setStatusTip("RTKLIB v." VER_RTKLIB " " PATCH_LEVEL);

    QCommandLineParser parser;
    parser.setApplicationDescription("rtklib application launcher Qt");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption titleOption("t",
                       QCoreApplication::translate("main", "Set Window Title"),
                       QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    QCommandLineOption trayOption(QStringList() << "tray",
                      QCoreApplication::translate("main", "only show tray icon"));
    parser.addOption(trayOption);

    QCommandLineOption miniOption(QStringList() << "min",
                      QCoreApplication::translate("main", "start minimized"));
    parser.addOption(miniOption);


    parser.process(*QApplication::instance());

    bool tray = parser.isSet(trayOption);

    if (parser.isSet(titleOption))
        setWindowTitle(parser.value(titleOption));

    if (tray) {
        setVisible(false);
        trayIcon.setVisible(true);
    }

    if (parser.isSet(miniOption)) minimize = 1;

    BtnOption->setVisible(false);

    trayMenu = new QMenu(this);
    trayMenu->addAction(tr("Expand"), this, SLOT(menuExpandClicked()));
    trayMenu->addSeparator();
    trayMenu->addAction(tr("RTK&PLOT"), this, SLOT(btnPlotClicked()));
    trayMenu->addAction(tr("&RTKPOST"), this, SLOT(btnPostClicked()));
    trayMenu->addAction(tr("RTK&NAVI"), this, SLOT(btnNaviClicked()));
    trayMenu->addAction(tr("RTK&CONV"), this, SLOT(btnConvClicked()));
    trayMenu->addAction(tr("RTK&GET"), this, SLOT(btnGetClicked()));
    trayMenu->addAction(tr("RTK&STR"), this, SLOT(btnStrClicked()));
    trayMenu->addAction(tr("&NTRIP BROWSER"), this, SLOT(btnNtripClicked()));
    trayMenu->addSeparator();
    trayMenu->addAction(tr("&Exit"), this, SLOT(close()));

    trayIcon.setContextMenu(trayMenu);
    trayIcon.setIcon(QIcon(":/icons/rtk9.bmp"));
    trayIcon.setToolTip(windowTitle());

    QMenu *Popup = new QMenu();
    Popup->addAction(tr("&Expand"), this, SLOT(menuExpandClicked()));
    Popup->addSeparator();
    Popup->addAction(actionRtkConv);
    Popup->addAction(actionRtkGet);
    Popup->addAction(actionRtkNavi);
    Popup->addAction(actionRtkNtrip);
    Popup->addAction(actionRtkPlot);
    Popup->addAction(actionRtkPost);
    Popup->addAction(actionRtkStr);
    Popup->addAction(actionRtkVideo);
    Popup->addSeparator();
    Popup->addAction(tr("E&xit"),this,SLOT(accept()));
    BtnRtklib->setMenu(Popup);

    connect(BtnPlot, SIGNAL(clicked(bool)), this, SLOT(btnPlotClicked()));
    connect(BtnConv, SIGNAL(clicked(bool)), this, SLOT(btnConvClicked()));
    connect(BtnStr, SIGNAL(clicked(bool)), this, SLOT(btnStrClicked()));
    connect(BtnPost, SIGNAL(clicked(bool)), this, SLOT(btnPostClicked()));
    connect(BtnNtrip, SIGNAL(clicked(bool)), this, SLOT(btnNtripClicked()));
    connect(BtnNavi, SIGNAL(clicked(bool)), this, SLOT(btnNaviClicked()));
    connect(BtnTray, SIGNAL(clicked(bool)), this, SLOT(btnTrayClicked()));
    connect(BtnGet, SIGNAL(clicked(bool)), this, SLOT(btnGetClicked()));
    connect(BtnVideo, SIGNAL(clicked(bool)), this, SLOT(btnVideoClicked()));
    connect(BtnOption, SIGNAL(clicked(bool)), this, SLOT(btnOptionClicked()));
    connect(BtnExit, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(&trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    connect(actionRtkConv, SIGNAL(triggered(bool)), this, SLOT(btnConvClicked()));
    connect(actionRtkGet, SIGNAL(triggered(bool)), this, SLOT(btnGetClicked()));
    connect(actionRtkNavi, SIGNAL(triggered(bool)), this, SLOT(btnNaviClicked()));
    connect(actionRtkNtrip, SIGNAL(triggered(bool)), this, SLOT(btnNtripClicked()));
    connect(actionRtkPlot, SIGNAL(triggered(bool)), this, SLOT(btnPlotClicked()));
    connect(actionRtkPost, SIGNAL(triggered(bool)), this, SLOT(btnPostClicked()));
    connect(actionRtkStr, SIGNAL(triggered(bool)), this, SLOT(btnStrClicked()));
    connect(actionRtkVideo, SIGNAL(triggered(bool)), this, SLOT(btnVideoClicked()));

    updatePanel();
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QSettings settings(iniFile, QSettings::IniFormat);

    move(settings.value("pos/left", 0).toInt(),
         settings.value("pos/top", 0).toInt());

    resize(settings.value("pos/width", 310).toInt(),
           settings.value("pos/height", 79).toInt());

}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous()) return;

    QSettings settings(iniFile, QSettings::IniFormat);
    settings.setValue("pos/left", pos().x());
    settings.setValue("pos/top", pos().y());
    settings.setValue("pos/width", width());
    settings.setValue("pos/height", height());
    settings.setValue("pos/option", option);
    settings.setValue("pos/minimize", minimize);
}
//---------------------------------------------------------------------------
void MainForm::btnPlotClicked()
{
    QString cmd1 = "./rtkplot_qt", cmd2 = "../../../bin/rtkplot_qt";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::btnConvClicked()
{
    QString cmd1 = "./rtkconv_qt", cmd2 = "../../../bin/rtkconv_qt";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::btnStrClicked()
{
    QString cmd1 = "./strsvr_qt", cmd2 = "../../../bin/strsvr_qt";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::btnPostClicked()
{
    QString cmd1 = "./rtkpost_qt", cmd2 = "../../../bin/rtkpost_qt";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::btnNtripClicked()
{
    QString cmd1 = "./srctblbrows_qt", cmd2 = "../../../bin/srctblbrows_qt";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::btnNaviClicked()
{
    QString cmd1 = "./rtknavi_qt", cmd2 = "../../../bin/rtknavi_qt";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::btnGetClicked()
{
    QString cmd1 = "./rtkget_qt", cmd2 = "../../../bin/rtkget_qt";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::btnVideoClicked()
{
    QString cmd1 = "rtkvideo", cmd2 = "..\\..\\..\\bin\\rtkvideo";
    QStringList opts;

    if (!execCmd(cmd1, opts)) execCmd(cmd2,  opts);
}
//---------------------------------------------------------------------------
int MainForm::execCmd(const QString &cmd, const QStringList &opt)
{
    return QProcess::startDetached(cmd, opt);
}
//---------------------------------------------------------------------------
void MainForm::btnTrayClicked()
{
    setVisible(false);
    trayIcon.setVisible(true);
}
//---------------------------------------------------------------------------
void MainForm::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::DoubleClick) return;

    setVisible(true);
    trayIcon.setVisible(false);
}
//---------------------------------------------------------------------------
void MainForm::menuExpandClicked()
{
    setVisible(true);
    trayIcon.setVisible(false);
    minimize = 0;
    updatePanel();
}
//---------------------------------------------------------------------------
void MainForm::updatePanel(void)
{
    if (minimize) {
        PanelTop->setVisible(false);
        PanelBottom->setVisible(true);
    }
    else {
        PanelTop->setVisible(true);
        PanelBottom->setVisible(false);
    }
}
//---------------------------------------------------------------------------
void MainForm::btnOptionClicked()
{
    launchOptDlg->exec();
    if (launchOptDlg->result()!=QDialog::Accepted) return;
    updatePanel();
}
