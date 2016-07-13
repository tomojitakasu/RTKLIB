//---------------------------------------------------------------------------
// rtklaunch_qt : rtklib app launcher
//
//          Copyright (C) 2013 by T.TAKASU, All rights reserved.
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
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QProcess>

#include "rtklib.h"
#include "launchmain.h"

//---------------------------------------------------------------------------

#define BTN_SIZE        42
#define BTN_COUNT       7
#define MAX(x,y)        ((x)>(y)?(x):(y))

//---------------------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QString file=QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile=fi.absolutePath()+"/"+fi.baseName()+".ini";

    setWindowTitle(tr("RTKLIB v.%1 %2").arg(VER_RTKLIB).arg(PATCH_LEVEL));
    setWindowIcon(QIcon(":/icons/rtk9.bmp"));
    
    QCoreApplication::setApplicationName("rtklaunch_qt");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("rtklib application launcher");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption titleOption("t",
                                   QCoreApplication::translate("main", "Set Window Title"),
                                   QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    QCommandLineOption trayOption(QStringList() << "tray",
            QCoreApplication::translate("main", "only show tray icon"));
    parser.addOption(trayOption);

    parser.process(*QApplication::instance());

    bool tray=parser.isSet(trayOption);

    if (parser.isSet(titleOption))
        setWindowTitle(parser.value(titleOption));

    if (tray) {
        setVisible(false);
        TrayIcon.setVisible(true);
    }

    trayMenu=new QMenu(this);
    trayMenu->addAction(tr("Expand"),this,SLOT(MenuExpandClick()));
    trayMenu->addSeparator();
    trayMenu->addAction(tr("RTK&PLOT"),this,SLOT(BtnPlotClick()));
    trayMenu->addAction(tr("&RTKPOST"),this,SLOT(BtnPostClick()));
    trayMenu->addAction(tr("RTK&NAVI"),this,SLOT(BtnNaviClick()));
    trayMenu->addAction(tr("RTK&CONV"),this,SLOT(BtnConvClick()));
    trayMenu->addAction(tr("RTK&GET"),this,SLOT(BtnGetClick()));
    trayMenu->addAction(tr("RTK&STR"),this,SLOT(BtnStrClick()));
    trayMenu->addAction(tr("&NTRIP BROWSER"),this,SLOT(BtnNtripClick()));
    trayMenu->addSeparator();
    trayMenu->addAction(tr("&Exit"),this,SLOT(close()));

    TrayIcon.setContextMenu(trayMenu);
    TrayIcon.setIcon(QIcon(":/icons/rtk9.bmp"));
    TrayIcon.setToolTip(windowTitle());

    connect(BtnPlot,SIGNAL(clicked(bool)),this,SLOT(BtnPlotClick()));
    connect(BtnConv,SIGNAL(clicked(bool)),this,SLOT(BtnConvClick()));
    connect(BtnStr,SIGNAL(clicked(bool)),this,SLOT(BtnStrClick()));
    connect(BtnPost,SIGNAL(clicked(bool)),this,SLOT(BtnPostClick()));
    connect(BtnNtrip,SIGNAL(clicked(bool)),this,SLOT(BtnNtripClick()));
    connect(BtnNavi,SIGNAL(clicked(bool)),this,SLOT(BtnNaviClick()));
    connect(BtnTray,SIGNAL(clicked(bool)),this,SLOT(BtnTrayClick()));
    connect(BtnGet,SIGNAL(clicked(bool)),this,SLOT(BtnGetClick()));
    connect(&TrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));

}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QSettings settings(IniFile,QSettings::IniFormat);

    move(settings.value("pos/left",    0).toInt(),
         settings.value("pos/top",     0).toInt());

    resize(settings.value("pos/width", 310).toInt(),
           settings.value("pos/height", 79).toInt());
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *event )
{
    if (event->spontaneous()) return;

    QSettings settings(IniFile,QSettings::IniFormat);
    settings.setValue("pos/left",    pos().x());
    settings.setValue("pos/top",     pos().y());
    settings.setValue("pos/width",   width());
    settings.setValue("pos/height",  height());
}
//---------------------------------------------------------------------------
void MainForm::BtnPlotClick()
{
    QString cmd1="./rtkplot_qt",cmd2="../../../bin/rtkplot_qt",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnConvClick()
{
    QString cmd1="./rtkconv_qt",cmd2="../../../bin/rtkconv_qt",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnStrClick()
{
    QString cmd1="./strsvr_qt",cmd2="../../../bin/strsvr_qt",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnPostClick()
{
    QString cmd1="./rtkpost_qt",cmd2="../../../bin/rtkpost_qt",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnNtripClick()
{
    QString cmd1="./srctblbrows_qt",cmd2="../../../bin/srctblbrows_qt",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnNaviClick()
{
    QString cmd1="./rtknavi_qt",cmd2="../../../bin/rtknavi_qt",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
void MainForm::BtnGetClick()
{
    QString cmd1="./rtkget_qt",cmd2="../../../bin/rtkget_qt",opts="";
    
    if (!ExecCmd(cmd1+opts)) ExecCmd(cmd2+opts);
}
//---------------------------------------------------------------------------
int MainForm::ExecCmd(const QString &cmd)
{
     return QProcess::startDetached(cmd); /* FIXME: show option not yet supported */
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
    if (reason!=QSystemTrayIcon::DoubleClick) return;

    setVisible(true);
    TrayIcon.setVisible(false);
}
//---------------------------------------------------------------------------
void MainForm::MenuExpandClick()
{
    setVisible(true);
    TrayIcon.setVisible(false);
}
//---------------------------------------------------------------------------

