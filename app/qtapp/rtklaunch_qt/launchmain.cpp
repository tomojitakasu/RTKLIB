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
//#include "launchoptdlg.h"

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

    QString file = QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile = fi.absolutePath() + "/" + fi.baseName() + ".ini";
    Option = 0;
    Minimize = 0;

    setWindowTitle(tr("RTKLIB v.%1 %2").arg(VER_RTKLIB).arg(PATCH_LEVEL));
    setWindowIcon(QIcon(":/icons/rtk9.bmp"));

    QCoreApplication::setApplicationName("rtklaunch_qt");
    QCoreApplication::setApplicationVersion("1.0");

    QSettings settings(IniFile, QSettings::IniFormat);
    Option =  settings.value("pos/option", 0).toInt();
    Minimize =  settings.value("pos/minimize", 1).toInt();
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
        TrayIcon.setVisible(true);
    }

    if (parser.isSet(miniOption)) Minimize = 1;

    trayMenu = new QMenu(this);
    trayMenu->addAction(tr("Expand"), this, SLOT(MenuExpandClick()));
    trayMenu->addSeparator();
    trayMenu->addAction(tr("RTK&PLOT"), this, SLOT(BtnPlotClick()));
    trayMenu->addAction(tr("&RTKPOST"), this, SLOT(BtnPostClick()));
    trayMenu->addAction(tr("RTK&NAVI"), this, SLOT(BtnNaviClick()));
    trayMenu->addAction(tr("RTK&CONV"), this, SLOT(BtnConvClick()));
    trayMenu->addAction(tr("RTK&GET"), this, SLOT(BtnGetClick()));
    trayMenu->addAction(tr("RTK&STR"), this, SLOT(BtnStrClick()));
    trayMenu->addAction(tr("&NTRIP BROWSER"), this, SLOT(BtnNtripClick()));
    trayMenu->addSeparator();
    trayMenu->addAction(tr("&Exit"), this, SLOT(close()));

    TrayIcon.setContextMenu(trayMenu);
    TrayIcon.setIcon(QIcon(":/icons/rtk9.bmp"));
    TrayIcon.setToolTip(windowTitle());

    QMenu *Popup = new QMenu();
    Popup->addAction(tr("&Expand"), this, SLOT(MenuExpandClick()));
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

    connect(BtnPlot, SIGNAL(clicked(bool)), this, SLOT(BtnPlotClick()));
    connect(BtnConv, SIGNAL(clicked(bool)), this, SLOT(BtnConvClick()));
    connect(BtnStr, SIGNAL(clicked(bool)), this, SLOT(BtnStrClick()));
    connect(BtnPost, SIGNAL(clicked(bool)), this, SLOT(BtnPostClick()));
    connect(BtnNtrip, SIGNAL(clicked(bool)), this, SLOT(BtnNtripClick()));
    connect(BtnNavi, SIGNAL(clicked(bool)), this, SLOT(BtnNaviClick()));
    connect(BtnTray, SIGNAL(clicked(bool)), this, SLOT(BtnTrayClick()));
    connect(BtnGet, SIGNAL(clicked(bool)), this, SLOT(BtnGetClick()));
    connect(BtnVideo, SIGNAL(clicked(bool)), this, SLOT(BtnVideoClick()));
    connect(BtnOption, SIGNAL(clicked(bool)), this, SLOT(BtnOptionClick()));
    connect(&TrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(BtnExit, SIGNAL(clicked(bool)), this, SLOT(accept()));

    connect(actionRtkConv, SIGNAL(triggered(bool)), this, SLOT(BtnConvClick()));
    connect(actionRtkGet, SIGNAL(triggered(bool)), this, SLOT(BtnGetClick()));
    connect(actionRtkNavi, SIGNAL(triggered(bool)), this, SLOT(BtnNaviClick()));
    connect(actionRtkNtrip, SIGNAL(triggered(bool)), this, SLOT(BtnNtripClick()));
    connect(actionRtkPlot, SIGNAL(triggered(bool)), this, SLOT(BtnPlotClick()));
    connect(actionRtkPost, SIGNAL(triggered(bool)), this, SLOT(BtnPostClick()));
    connect(actionRtkStr, SIGNAL(triggered(bool)), this, SLOT(BtnStrClick()));
    connect(actionRtkVideo, SIGNAL(triggered(bool)), this, SLOT(BtnVideoClick()));

    UpdatePanel();
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QSettings settings(IniFile, QSettings::IniFormat);

    move(settings.value("pos/left", 0).toInt(),
         settings.value("pos/top", 0).toInt());

    resize(settings.value("pos/width", 310).toInt(),
           settings.value("pos/height", 79).toInt());

}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous()) return;

    QSettings settings(IniFile, QSettings::IniFormat);
    settings.setValue("pos/left", pos().x());
    settings.setValue("pos/top", pos().y());
    settings.setValue("pos/width", width());
    settings.setValue("pos/height", height());
    settings.setValue("pos/option", Option);
    settings.setValue("pos/minimize", Minimize);
}
//---------------------------------------------------------------------------
void MainForm::BtnPlotClick()
{
    QString cmd1 = "./rtkplot_qt", cmd2 = "../../../bin/rtkplot_qt";
    QStringList opts;


    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnConvClick()
{
    QString cmd1 = "./rtkconv_qt", cmd2 = "../../../bin/rtkconv_qt";
    QStringList opts;


    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnStrClick()
{
    QString cmd1 = "./strsvr_qt", cmd2 = "../../../bin/strsvr_qt";
    QStringList opts;


    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnPostClick()
{
    QString cmd1 = "./rtkpost_qt", cmd2 = "../../../bin/rtkpost_qt";
    QStringList opts;

    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnNtripClick()
{
    QString cmd1 = "./srctblbrows_qt", cmd2 = "../../../bin/srctblbrows_qt";
    QStringList opts;

    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnNaviClick()
{
    QString cmd1 = "./rtknavi_qt", cmd2 = "../../../bin/rtknavi_qt";
    QStringList opts;

    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnGetClick()
{
    QString cmd1 = "./rtkget_qt", cmd2 = "../../../bin/rtkget_qt";
    QStringList opts;

    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2, opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnVideoClick()
{
    QString cmd1 = "rtkvideo", cmd2 = "..\\..\\..\\bin\\rtkvideo";
    QStringList opts;

    if (!ExecCmd(cmd1, opts)) ExecCmd(cmd2,  opts);
}
//---------------------------------------------------------------------------
int MainForm::ExecCmd(const QString &cmd, const QStringList &opt)
{
    return QProcess::startDetached(cmd, opt);
}
//---------------------------------------------------------------------------
void MainForm::BtnTrayClick()
{
    setVisible(false);
    TrayIcon.setVisible(true);
}
//---------------------------------------------------------------------------
void MainForm::TrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::DoubleClick) return;

    setVisible(true);
    TrayIcon.setVisible(false);
}
//---------------------------------------------------------------------------
void MainForm::MenuExpandClick()
{
    setVisible(true);
    TrayIcon.setVisible(false);
    Minimize = 0;
    UpdatePanel();
}
//---------------------------------------------------------------------------
void MainForm::UpdatePanel(void)
{
    if (Minimize) {
        Panel1->setVisible(false);
        Panel2->setVisible(true);
    }
    else {
        Panel1->setVisible(true);
        Panel2->setVisible(false);
    }
}
//---------------------------------------------------------------------------
void MainForm::BtnOptionClick()
{
//    launchOptDialog->exec();
//    if (launchOptDialog->result()!=QDialog::Accepted) return;
    UpdatePanel();
}
