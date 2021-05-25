//---------------------------------------------------------------------------
// strsvr : stream server
//
//          Copyright (C) 2007-2012 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : strsvr [-t title][-i file][-auto][-tray]
//
//           -t title   window title
//           -i file    ini file path
//           -auto      auto start
//           -tray      start as task tray icon
//
// version : $Revision:$ $Date:$
// history : 2008/04/03  1.1 rtklib 2.3.1
//           2010/07/18  1.2 rtklib 2.4.0
//           2011/06/10  1.3 rtklib 2.4.1
//           2012/12/15  1.4 rtklib 2.4.2
//                       add stream conversion function
//                       add option -auto and -tray
//---------------------------------------------------------------------------
#include <clocale>

#include <QTimer>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>
#include <QMenu>
#include <QStringList>

#include "rtklib.h"
#include "svroptdlg.h"
#include "serioptdlg.h"
#include "fileoptdlg.h"
#include "tcpoptdlg.h"
#include "ftpoptdlg.h"
#include "cmdoptdlg.h"
#include "convdlg.h"
#include "aboutdlg.h"
#include "refdlg.h"
#include "mondlg.h"
#include "svrmain.h"
#include "mondlg.h"

//---------------------------------------------------------------------------

#define PRGNAME     "STRSVR-QT"        // program name
#define TRACEFILE   "strsvr.trace"  // debug trace file
#define CLORANGE    QColor(0x00,0xAA,0xFF)

#define MIN(x,y)    ((x)<(y)?(x):(y))
#define MAX(x,y)    ((x)>(y)?(x):(y))

strsvr_t strsvr;

extern "C" {
extern int showmsg(const char *, ...)  {return 0;}
extern void settime(gtime_t) {}
extern void settspan(gtime_t, gtime_t) {}
}

QString color2String(const QColor &c){
    return QString("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}

// number to comma-separated number -----------------------------------------
static void num2cnum(int num, char *str)
{
    char buff[256],*p=buff,*q=str;
    int i,n;
    n=sprintf(buff,"%u",(uint32_t)num);
    for (i=0;i<n;i++) {
        *q++=*p++;
        if ((n-i-1)%3==0&&i<n-1) *q++=',';
    }
    *q='\0';
}
// constructor --------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QCoreApplication::setApplicationName(PRGNAME);
    QCoreApplication::setApplicationVersion(QString(VER_RTKLIB) + " " + PATCH_LEVEL);

    setWindowIcon(QIcon(":/icons/rtk6.bmp"));

    setlocale(LC_NUMERIC, "C"); // use point as decimal separator in formatted output

    QString file = QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile = fi.absolutePath() + "/" + fi.baseName() + ".ini";

    TrayIcon = new QSystemTrayIcon(QIcon(":/icons/strsvr_Icon"));

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction(MenuExpand);
    trayMenu->addSeparator();
    trayMenu->addAction(MenuStart);
    trayMenu->addAction(MenuStop);
    trayMenu->addSeparator();
    trayMenu->addAction(MenuExit);

    TrayIcon->setContextMenu(trayMenu);

    svrOptDialog = new SvrOptDialog(this);
    tcpOptDialog = new TcpOptDialog(this);
    serialOptDialog = new SerialOptDialog(this);
    fileOptDialog = new FileOptDialog(this);
    ftpOptDialog = new FtpOptDialog(this);
    strMonDialog = new StrMonDialog(this);

    BtnStop->setVisible(false);

    StartTime.sec = StartTime.time = EndTime.sec = EndTime.time = 0;

    connect(BtnExit, SIGNAL(clicked(bool)), this, SLOT(BtnExitClick()));
    connect(BtnInput, SIGNAL(clicked(bool)), this, SLOT(BtnInputClick()));
    connect(BtnStart, SIGNAL(clicked(bool)), this, SLOT(BtnStartClick()));
    connect(BtnStop, SIGNAL(clicked(bool)), this, SLOT(BtnStopClick()));
    connect(BtnOpt, SIGNAL(clicked(bool)), this, SLOT(BtnOptClick()));
    connect(BtnCmd, SIGNAL(clicked(bool)), this, SLOT(BtnCmdClick()));
    connect(BtnCmd1, SIGNAL(clicked(bool)), this, SLOT(BtnCmdClick()));
    connect(BtnCmd2, SIGNAL(clicked(bool)), this, SLOT(BtnCmdClick()));
    connect(BtnCmd3, SIGNAL(clicked(bool)), this, SLOT(BtnCmdClick()));
    connect(BtnCmd4, SIGNAL(clicked(bool)), this, SLOT(BtnCmdClick()));
    connect(BtnCmd5, SIGNAL(clicked(bool)), this, SLOT(BtnCmdClick()));
    connect(BtnCmd6, SIGNAL(clicked(bool)), this, SLOT(BtnCmdClick()));
    connect(BtnAbout, SIGNAL(clicked(bool)), this, SLOT(BtnAboutClick()));
    connect(BtnStrMon, SIGNAL(clicked(bool)), this, SLOT(BtnStrMonClick()));
    connect(MenuStart, SIGNAL(triggered(bool)), this, SLOT(MenuStartClick()));
    connect(MenuStop, SIGNAL(triggered(bool)), this, SLOT(MenuStopClick()));
    connect(MenuExit, SIGNAL(triggered(bool)), this, SLOT(MenuExitClick()));
    connect(MenuExpand, SIGNAL(triggered(bool)), this, SLOT(MenuExpandClick()));
    connect(BtnOutput1, SIGNAL(clicked(bool)), this, SLOT(BtnOutputClick()));
    connect(BtnOutput2, SIGNAL(clicked(bool)), this, SLOT(BtnOutputClick()));
    connect(BtnOutput3, SIGNAL(clicked(bool)), this, SLOT(BtnOutputClick()));
    connect(BtnOutput4, SIGNAL(clicked(bool)), this, SLOT(BtnOutputClick()));
    connect(BtnOutput5, SIGNAL(clicked(bool)), this, SLOT(BtnOutputClick()));
    connect(BtnOutput6, SIGNAL(clicked(bool)), this, SLOT(BtnOutputClick()));
    connect(BtnTaskIcon, SIGNAL(clicked(bool)), this, SLOT(BtnTaskIconClick()));
    connect(Output1, SIGNAL(currentIndexChanged(int)), this, SLOT(OutputChange()));
    connect(Output2, SIGNAL(currentIndexChanged(int)), this, SLOT(OutputChange()));
    connect(Output3, SIGNAL(currentIndexChanged(int)), this, SLOT(OutputChange()));
    connect(Output4, SIGNAL(currentIndexChanged(int)), this, SLOT(OutputChange()));
    connect(Output5, SIGNAL(currentIndexChanged(int)), this, SLOT(OutputChange()));
    connect(Output6, SIGNAL(currentIndexChanged(int)), this, SLOT(OutputChange()));
    connect(BtnConv1, SIGNAL(clicked(bool)), this, SLOT(BtnConvClick()));
    connect(BtnConv2, SIGNAL(clicked(bool)), this, SLOT(BtnConvClick()));
    connect(BtnConv3, SIGNAL(clicked(bool)), this, SLOT(BtnConvClick()));
    connect(BtnConv4, SIGNAL(clicked(bool)), this, SLOT(BtnConvClick()));
    connect(BtnConv5, SIGNAL(clicked(bool)), this, SLOT(BtnConvClick()));
    connect(BtnConv6, SIGNAL(clicked(bool)), this, SLOT(BtnConvClick()));
    connect(BtnLog1, SIGNAL(clicked(bool)), this, SLOT(BtnLogClick()));
    connect(BtnLog2, SIGNAL(clicked(bool)), this, SLOT(BtnLogClick()));
    connect(BtnLog3, SIGNAL(clicked(bool)), this, SLOT(BtnLogClick()));
    connect(BtnLog4, SIGNAL(clicked(bool)), this, SLOT(BtnLogClick()));
    connect(BtnLog5, SIGNAL(clicked(bool)), this, SLOT(BtnLogClick()));
    connect(BtnLog6, SIGNAL(clicked(bool)), this, SLOT(BtnLogClick()));
    connect(&Timer1, SIGNAL(timeout()), this, SLOT(Timer1Timer()));
    connect(&Timer2, SIGNAL(timeout()), this, SLOT(Timer2Timer()));
    connect(TrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(Input, SIGNAL(currentIndexChanged(int)), this, SLOT(InputChange()));

    Timer1.setInterval(50);
    Timer2.setInterval(100);

    QTimer::singleShot(100, this, SLOT(FormCreate()));
}
// callback on form create --------------------------------------------------
void MainForm::FormCreate()
{
    int autorun = 0, tasktray = 0;

    strsvrinit(&strsvr, MAXSTR-1);

    setWindowTitle(QString("%1 ver.%2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));


    QCommandLineParser parser;
    parser.setApplicationDescription(tr("stream server qt"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    // A boolean option with a single name (-p)
    QCommandLineOption showProgressOption("p", QCoreApplication::translate("main", "Show progress during copy"));
    parser.addOption(showProgressOption);

    QCommandLineOption trayOption(QStringList() << "tray",
                      QCoreApplication::translate("main", "start as task tray icon."));
    parser.addOption(trayOption);

    QCommandLineOption autoOption(QStringList() << "auto",
                      QCoreApplication::translate("main", "auto start."));
    parser.addOption(autoOption);

    QCommandLineOption windowTitleOption(QStringList() << "t",
                         QCoreApplication::translate("main", "window title."),
                         QCoreApplication::translate("main", "title"));
    parser.addOption(windowTitleOption);

    QCommandLineOption iniFileOption(QStringList() << "i",
                     QCoreApplication::translate("main", "ini file path."),
                     QCoreApplication::translate("main", "file"));
    parser.addOption(iniFileOption);

    parser.process(*QApplication::instance());

    if (parser.isSet(iniFileOption))
        IniFile = parser.value(iniFileOption);

    LoadOpt();

    if (parser.isSet(windowTitleOption))
        setWindowTitle(parser.value(windowTitleOption));
    if (parser.isSet(autoOption)) autorun = 1;
    if (parser.isSet(trayOption)) tasktray = 1;

    SetTrayIcon(0);

    if (tasktray) {
        setVisible(false);
        TrayIcon->setVisible(true);
    }
    if (autorun)
        SvrStart();
    Timer1.start();
    Timer2.start();
}
// callback on form close ---------------------------------------------------
void MainForm::closeEvent(QCloseEvent *)
{
    SaveOpt();
}
// callback on button-exit --------------------------------------------------
void MainForm::BtnExitClick()
{
    close();
}
// callback on button-start -------------------------------------------------
void MainForm::BtnStartClick()
{
    SvrStart();
}
// callback on button-stop --------------------------------------------------
void MainForm::BtnStopClick()
{
    SvrStop();
}
// callback on button-options -----------------------------------------------
void MainForm::BtnOptClick()
{
    for (int i = 0; i < 6; i++) svrOptDialog->SvrOpt[i] = SvrOpt[i];
    for (int i = 0; i < 3; i++) svrOptDialog->AntPos[i] = AntPos[i];
    for (int i = 0; i < 3; i++) svrOptDialog->AntOff[i] = AntOff[i];
    svrOptDialog->TraceLevel = TraceLevel;
    svrOptDialog->NmeaReq = NmeaReq;
    svrOptDialog->FileSwapMargin = FileSwapMargin;
    svrOptDialog->RelayBack = RelayBack;
    svrOptDialog->ProgBarRange = ProgBarRange;
    svrOptDialog->StaPosFile = StaPosFile;
    svrOptDialog->ExeDirectory = ExeDirectory;
    svrOptDialog->LocalDirectory = LocalDirectory;
    svrOptDialog->ProxyAddress = ProxyAddress;
    svrOptDialog->StaId = StaId;
    svrOptDialog->StaSel = StaSel;
    svrOptDialog->AntType = AntType;
    svrOptDialog->RcvType = RcvType;
    svrOptDialog->LogFile = LogFile;

    svrOptDialog->exec();
    if (svrOptDialog->result() != QDialog::Accepted) return;

    for (int i = 0; i < 6; i++) SvrOpt[i] = svrOptDialog->SvrOpt[i];
    for (int i = 0; i < 3; i++) AntPos[i] = svrOptDialog->AntPos[i];
    for (int i = 0; i < 3; i++) AntOff[i] = svrOptDialog->AntOff[i];
    TraceLevel = svrOptDialog->TraceLevel;
    NmeaReq = svrOptDialog->NmeaReq;
    FileSwapMargin = svrOptDialog->FileSwapMargin;
    RelayBack = svrOptDialog->RelayBack;
    ProgBarRange = svrOptDialog->ProgBarRange;
    StaPosFile = svrOptDialog->StaPosFile;
    ExeDirectory = svrOptDialog->ExeDirectory;
    LocalDirectory = svrOptDialog->LocalDirectory;
    ProxyAddress = svrOptDialog->ProxyAddress;
    StaId = svrOptDialog->StaId;
    StaSel = svrOptDialog->StaSel;
    AntType = svrOptDialog->AntType;
    RcvType = svrOptDialog->RcvType;
    LogFile = svrOptDialog->LogFile;
}
// callback on button-input-opt ---------------------------------------------
void MainForm::BtnInputClick()
{
    switch (Input->currentIndex()) {
        case 0: SerialOpt(0, 0); break;
        case 1: TcpCliOpt(0, 1); break; // TCP Client
        case 2: TcpSvrOpt(0, 2); break; // TCP Server
        case 3: NtripCliOpt(0, 3); break; // Ntrip Client
        case 4: UdpSvrOpt(0, 6); break;  // UDP Server
        case 5: FileOpt(0, 0); break;
    }
}
// callback on button-input-cmd ---------------------------------------------
void MainForm::BtnCmdClick()
{
    CmdOptDialog *cmdOptDialog = new CmdOptDialog(this);


    QPushButton *btn[] = { BtnCmd, BtnCmd1, BtnCmd2, BtnCmd3, BtnCmd4, BtnCmd5, BtnCmd6 };
    QComboBox *type[] = { Input, Output1, Output2, Output3, Output4, Output5, Output6 };
    int i, j;

    for (i = 0; i < MAXSTR; i++)
        if (btn[i] == qobject_cast<QPushButton *>(sender())) break;
    if (i >= MAXSTR) return;

    for (j = 0; j < 3; j++) {
        if (type[i]->currentText() == tr("Serial")) {
            cmdOptDialog->Cmds[j] = Cmds[i][j];
            cmdOptDialog->CmdEna[j] = CmdEna[i][j];
        }
        else {
            cmdOptDialog->Cmds[j] = CmdsTcp[i][j];
            cmdOptDialog->CmdEna[j] = CmdEnaTcp[i][j];
        }
    }
	if (i==0) cmdOptDialog->setWindowTitle("Input Serial/TCP Commands");
	else cmdOptDialog->setWindowTitle(QString("Output%1 Serial/TCP Commands").arg(i));

    cmdOptDialog->exec();
    if (cmdOptDialog->result() != QDialog::Accepted) return;

    for (j=0;j<3;j++) {
        if (type[i]->currentText() == tr("Serial")) {
            Cmds[i][j] = cmdOptDialog->Cmds[j];
            CmdEna[i][j] = cmdOptDialog->CmdEna[j];
        }
        else {
            CmdsTcp[i][j] = cmdOptDialog->Cmds[j];
            CmdEnaTcp[i][j] = cmdOptDialog->CmdEna[j];
        }
    }

    delete cmdOptDialog;
}
// callback on button-output1-opt -------------------------------------------
void MainForm::BtnOutputClick()
{
    QPushButton *btn[]={BtnOutput1,BtnOutput2,BtnOutput3,BtnOutput4,BtnOutput5,BtnOutput6};
	QComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};
	int i;

	for (i=0;i<MAXSTR-1;i++) {
		if ((QPushButton *)sender()==btn[i]) break;
	}
	if (i>=MAXSTR-1) return;

	switch (type[i]->currentIndex()) {
		case 1: SerialOpt  (i+1,0); break;
		case 2: TcpCliOpt  (i+1,1); break;
		case 3: TcpSvrOpt  (i+1,2); break;
		case 4: NtripSvrOpt(i+1,3); break;
		case 5: NtripCasOpt(i+1,4); break;
		case 6: UdpCliOpt  (i+1,5); break;
		case 7: FileOpt    (i+1,6); break;
	}
}
// callback on button-output-conv ------------------------------------------
void MainForm::BtnConvClick()
{
    QPushButton *btn[]={BtnConv1,BtnConv2,BtnConv3,BtnConv4,BtnConv5,BtnConv6};
    int i;

    for (i=0;i<MAXSTR-1;i++) {
        if ((QPushButton *)sender()==btn[i]) break;
    }
    if (i>=MAXSTR-1) return;

    ConvDialog *convDialog = new ConvDialog(this);

    convDialog->ConvEna=ConvEna[i];
    convDialog->ConvInp=ConvInp[i];
    convDialog->ConvOut=ConvOut[i];
    convDialog->ConvMsg=ConvMsg[i];
    convDialog->ConvOpt=ConvOpt[i];
	convDialog->setWindowTitle(QString("Output%1 Conversion Options").arg(i+1));

    convDialog->exec();
    if (convDialog->result() != QDialog::Accepted) return;

    ConvEna[i]=convDialog->ConvEna;
    ConvInp[i]=convDialog->ConvInp;
    ConvOut[i]=convDialog->ConvOut;
    ConvMsg[i]=convDialog->ConvMsg;
    ConvOpt[i]=convDialog->ConvOpt;

    delete convDialog;
}
// callback on buttn-about --------------------------------------------------
void MainForm::BtnAboutClick()
{
    AboutDialog *aboutDialog = new AboutDialog(this);

    aboutDialog->About = PRGNAME;
    aboutDialog->IconIndex = 6;
    aboutDialog->exec();

    delete aboutDialog;
}
// callback on task-icon ----------------------------------------------------
void MainForm::BtnTaskIconClick()
{
    setVisible(false);
    TrayIcon->setVisible(true);
}
// callback on task-icon double-click ---------------------------------------
void MainForm::TrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::DoubleClick) return;

    setVisible(true);
    TrayIcon->setVisible(false);
}
// callback on menu-expand --------------------------------------------------
void MainForm::MenuExpandClick()
{
    setVisible(true);
    TrayIcon->setVisible(false);
}
// callback on menu-start ---------------------------------------------------
void MainForm::MenuStartClick()
{
    SvrStart();
}
// callback on menu-stop ----------------------------------------------------
void MainForm::MenuStopClick()
{
    SvrStop();
}
// callback on menu-exit ----------------------------------------------------
void MainForm::MenuExitClick()
{
    close();
}
// callback on stream-monitor -----------------------------------------------
void MainForm::BtnStrMonClick()
{
    strMonDialog->show();
}
// callback on input type change --------------------------------------------
void MainForm::InputChange()
{
    UpdateEnable();
}
// callback on output type change ------------------------------------------
void MainForm::OutputChange()
{
    UpdateEnable();
}
// callback on interval timer -----------------------------------------------
void MainForm::Timer1Timer()
{
    QColor color[] = { Qt::red, Qt::white, CLORANGE, Qt::green, QColor(0x00, 0xff, 0x00), QColor(0xff, 0xff, 0x00) };
    QLabel *e0[] = { IndInput, IndOutput1, IndOutput2, IndOutput3, IndOutput4, IndOutput5, IndOutput6 };
    QLabel *e1[] = { InputByte, Output1Byte, Output2Byte, Output3Byte, Output4Byte,Output5Byte, Output6Byte };
    QLabel *e2[] = { InputBps, Output1Bps, Output2Bps, Output3Bps, Output4Bps, Output5Bps, Output6Bps};
    QLabel *e3[] = {IndLog,IndLog1,IndLog2,IndLog3,IndLog4,IndLog5,IndLog6};
    gtime_t time = utc2gpst(timeget());
    int stat[MAXSTR] = { 0 }, byte[MAXSTR] = { 0 }, bps[MAXSTR] = { 0 }, log_stat[MAXSTR]={0};
    char msg[MAXSTRMSG * MAXSTR] = "", s1[256], s2[256];
    double ctime, t[4], pos;

    strsvrstat(&strsvr, stat, log_stat, byte, bps, msg);
    for (int i = 0; i < MAXSTR; i++) {
        num2cnum(byte[i], s1);
        num2cnum(bps[i], s2);
        e0[i]->setStyleSheet(QString("background-color: %1").arg(color2String(color[stat[i] + 1])));
        e1[i]->setText(s1);
        e2[i]->setText(s2);
        e3[i]->setStyleSheet(QString("color :%1").arg(color2String(color[log_stat[i]+1])));
    }
    pos = fmod(byte[0] / 1e3 / MAX(ProgBarRange, 1), 1.0) * 110.0;
    Progress->setValue(!stat[0] ? 0 : MIN((int)pos, 100));

    time2str(time, s1, 0);
    Time->setText(QString(tr("%1 GPST")).arg(s1));

    if (Panel1->isEnabled())
        ctime = timediff(EndTime, StartTime);
    else
        ctime = timediff(time, StartTime);
    ctime = floor(ctime);
    t[0] = floor(ctime / 86400.0); ctime -= t[0] * 86400.0;
    t[1] = floor(ctime / 3600.0); ctime -= t[1] * 3600.0;
    t[2] = floor(ctime / 60.0); ctime -= t[2] * 60.0;
    t[3] = ctime;
    ConTime->setText(QString("%1d %2:%3:%4").arg(t[0], 0, 'f', 0).arg(t[1], 2, 'f', 0, QChar('0')).arg(t[2], 2, 'f', 0, QChar('0')).arg(t[3], 2, 'f', 2, QChar('0')));

    num2cnum(byte[0], s1); num2cnum(bps[0], s2);
    TrayIcon->setToolTip(QString(tr("%1 bytes %2 bps")).arg(s1).arg(s2));
    SetTrayIcon(stat[0] <= 0 ? 0 : (stat[0] == 3 ? 2 : 1));

    Message->setText(msg);
    Message->setToolTip(msg);
}
// start stream server ------------------------------------------------------
void MainForm::SvrStart(void)
{
    QComboBox *type[]={Input,Output1,Output2,Output3,Output4,Output5,Output6};
    strconv_t *conv[MAXSTR-1]={0};
    static char str1[MAXSTR][1024], str2[MAXSTR][1024];
    int itype[]={
        STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_UDPSVR,
        STR_FILE,STR_FTP,STR_HTTP
    };
    int otype[]={
        STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPCAS,
        STR_UDPCLI,STR_FILE
    };
    int strs[MAXSTR]={0},opt[8]={0};
    char *paths[MAXSTR],*logs[MAXSTR],*cmds[MAXSTR]={0},*cmds_periodic[MAXSTR]={0};
    char filepath[1024];
    char *p;

    if (TraceLevel>0) {
        traceopen(!LogFile.isEmpty()?qPrintable(LogFile):TRACEFILE);
        tracelevel(TraceLevel);
    }
    for (int i = 0; i < MAXSTR; i++) {
        paths[i] = str1[i];
        logs[i] = str2[i];
    }

    strs[0] = itype[type[0]->currentIndex()];
    strcpy(paths[0], qPrintable(Paths[0][type[0]->currentIndex()]));
    strcpy(logs[0],type[0]->currentIndex()>5||!PathEna[0]?"":qPrintable(PathLog[0]));

    for (int i=1;i<MAXSTR;i++) {
        strs[i]=otype[type[i]->currentIndex()];
        strcpy(paths[1], !type[i]->currentIndex() ? "" : qPrintable(Paths[i][type[i]->currentIndex() - 1]));
        strcpy(logs[i],!PathEna[i]?"":qPrintable(PathLog[i]));
    }

    for (int i=0;i<MAXSTR;i++) {
        cmds[i] = cmds_periodic[i] = NULL;
        if (strs[i]==STR_SERIAL) {
            cmds[i] = new char[1024];
            cmds_periodic[i] = new char[1024];
            if (CmdEna[i][0]) strncpy(cmds[i],qPrintable(Cmds[i][0]), 1024);
            if (CmdEna[i][2]) strncpy(cmds_periodic[i], qPrintable(Cmds[i][2]), 1024);
        }
        else if (strs[i]==STR_TCPCLI||strs[i]==STR_NTRIPCLI) {
            cmds[i] = new char[1024];
            cmds_periodic[i] = new char[1024];
            if (CmdEnaTcp[i][0]) strncpy(cmds[i], qPrintable(CmdsTcp[i][0]), 1024);
            if (CmdEnaTcp[i][2]) strncpy(cmds_periodic[i], qPrintable(CmdsTcp[i][2]), 1024);
        }
    }
    for (int i=0;i<5;i++) {
        opt[i]=SvrOpt[i];
    }
    opt[5]=NmeaReq?SvrOpt[5]:0;
    opt[6]=FileSwapMargin;
    opt[7]=RelayBack;

    for (int i=1;i<MAXSTR;i++) { // for each out stream
        if (strs[i]!=STR_FILE) continue;
        strcpy(filepath,paths[i]);
        if (strstr(filepath,"::A")) continue;
        if ((p=strstr(filepath,"::"))) *p='\0';
        if (!QFile::exists(filepath)) continue;
        if (QMessageBox::question(this, tr("Overwrite"), tr("File %1 exists. \nDo you want to overwrite?").arg(filepath)) != QMessageBox::Yes) return;
    }
    for (int i=0;i<MAXSTR;i++) { // for each log stream
        if (!*logs[i]) continue;
        strcpy(filepath,logs[i]);
        if (strstr(filepath,"::A")) continue;
        if ((p=strstr(filepath,"::"))) *p='\0';
        if (!QFile::exists(filepath)) continue;
        if (QMessageBox::question(this, tr("Overwrite"), tr("File %1 exists. \nDo you want to overwrite?").arg(filepath)) != QMessageBox::Yes) return;
    }

    strsetdir(qPrintable(LocalDirectory));
    strsetproxy(qPrintable(ProxyAddress));

    for (int i=0;i<MAXSTR-1;i++) { // for each out stream
        if (Input->currentIndex() == 2||Input->currentIndex() == 4) continue;
        if (!ConvEna[i]) continue;
        if (!(conv[i] = strconvnew(ConvInp[i], ConvOut[i], qPrintable(ConvMsg[i]),
                       StaId, StaSel, qPrintable(ConvOpt[i])))) continue;
        QStringList tokens = AntType.split(',');
        if (tokens.size() >= 3)
        {
            strcpy(conv[i]->out.sta.antdes, qPrintable(tokens.at(0)));
            strcpy(conv[i]->out.sta.antsno, qPrintable(tokens.at(1)));
            conv[i]->out.sta.antsetup = atoi(qPrintable(tokens.at(2)));
        }
        tokens = RcvType.split(',');
        if (tokens.size() >= 3)
        {
            strcpy(conv[i]->out.sta.rectype, qPrintable(tokens.at(0)));
            strcpy(conv[i]->out.sta.recver, qPrintable(tokens.at(1)));
            strcpy(conv[i]->out.sta.recsno, qPrintable(tokens.at(2)));
        }
        matcpy(conv[i]->out.sta.pos,AntPos,3,1);
        matcpy(conv[i]->out.sta.del,AntOff,3,1);
    }
    // stream server start
    if (!strsvrstart(&strsvr, opt, strs, paths, logs, conv, cmds, cmds_periodic, AntPos)) {
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (cmds[i]) delete[] cmds[i];
        if (cmds_periodic[i]) delete[] cmds_periodic[i];
    };

    StartTime = utc2gpst(timeget());
    Panel1->setEnabled(false);
    BtnStart->setVisible(false);
    BtnStop->setVisible(true);
    BtnOpt->setEnabled(false);
    BtnExit->setEnabled(false);
    MenuStart->setEnabled(false);
    MenuStop->setEnabled(true);
    MenuExit->setEnabled(false);
    SetTrayIcon(1);

}
// stop stream server -------------------------------------------------------
void MainForm::SvrStop(void)
{
    char *cmds[MAXSTR];
    QComboBox *type[]={Input,Output1,Output2,Output3,Output4,Output5,Output6};
    const int itype[] = {
        STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_UDPSVR,STR_FILE,
        STR_FTP,STR_HTTP    };
    const int otype[] = {
        STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPCAS,
        STR_FILE
    };
    int strs[MAXSTR];

    strs[0] = itype[Input->currentIndex()];
    for (int i=1;i<MAXSTR;i++) {
        strs[1] = otype[type[i]->currentIndex()];
    }

    for (int i = 0; i < MAXSTR; i++) {
        cmds[i] = NULL;
        if (strs[i] == STR_SERIAL) {
            cmds[i] = new char[1024];
            if (CmdEna[i][1]) strncpy(cmds[i], qPrintable(Cmds[i][1]), 1024);
        } else if (strs[i] == STR_TCPCLI || strs[i] == STR_NTRIPCLI) {
            cmds[i] = new char[1024];
            if (CmdEnaTcp[i][1]) strncpy(cmds[i], qPrintable(CmdsTcp[i][1]), 1024);
        }
    }
    strsvrstop(&strsvr, cmds);

    EndTime = utc2gpst(timeget());
    Panel1->setEnabled(true);
    BtnStart->setVisible(true);
    BtnStop->setVisible(false);
    BtnOpt->setEnabled(true);
    BtnExit->setEnabled(true);
    MenuStart->setEnabled(true);
    MenuStop->setEnabled(false);
    MenuExit->setEnabled(true);
    SetTrayIcon(0);

    for (int i = 0; i < MAXSTR - 1; i++) {
        if (cmds[i]) delete[] cmds[i];
        if (ConvEna[i]) strconvfree(strsvr.conv[i]);
    }
    if (TraceLevel > 0) traceclose();
}
// callback on interval timer for stream monitor ----------------------------
void MainForm::Timer2Timer()
{
    const QString types[]={
        tr("None"),tr("Serial"),tr("File"),tr("TCP Server"),tr("TCP Client"),tr("UDP"),tr("Ntrip Sever"),
        tr("Ntrip Client"),tr("FTP"),tr("HTTP"),tr("Ntrip Cast"),tr("UDP Server"),
        tr("UDP Client")
    };
    const QString modes[]={tr("-"),tr("R"),tr("W"),tr("R/W")};
    const QString states[]={tr("ERR"),tr("-"),tr("WAIT"),tr("CONN")};
    unsigned char *msg = 0;
    char *p;
    int i,len,inb,inr,outb,outr;

    if (strMonDialog->StrFmt) {
        lock(&strsvr.lock);
        len = strsvr.npb;
        if (len > 0 && (msg = (unsigned char *)malloc(len))) {
            memcpy(msg, strsvr.pbuf, len);
            strsvr.npb = 0;
        }
        unlock(&strsvr.lock);
        if (len <= 0 || !msg) return;
        strMonDialog->AddMsg(msg, len);
        free(msg);
    }
    else {
        if (!(msg = (unsigned char *)malloc(16000))) return;

        for (i = 0, p = (char*)msg; i < MAXSTR; i++) {
            p += sprintf(p, "[STREAM %d]\n", i);
            strsum(strsvr.stream + i, &inb, &inr, &outb, &outr);
            strstatx(strsvr.stream + i, p);
            p += strlen(p);
            if (inb > 0) {
                p += sprintf(p,"  inb     = %d\n", inb);
                p += sprintf(p,"  inr     = %d\n", inr);
            }
            if (outb > 0) {
                p += sprintf(p,"  outb    = %d\n", outb);
                p += sprintf(p,"  outr    = %d\n", outr);
            }
        }
        strMonDialog->AddMsg(msg,strlen((char*)msg));

        free(msg);
    }
}
// set serial options -------------------------------------------------------
void MainForm::SerialOpt(int index, int path)
{
    serialOptDialog->Path = Paths[index][path];
    serialOptDialog->Opt = (index==0)?0:1;

    serialOptDialog->exec();
    if (serialOptDialog->result() != QDialog::Accepted) return;
    Paths[index][path] = serialOptDialog->Path;
}
// set tcp server options -------------------------------------------------------
void MainForm::TcpSvrOpt(int index, int path)
{
    tcpOptDialog->Path = Paths[index][path];
    tcpOptDialog->Opt = 0;
    tcpOptDialog->exec();
    if (tcpOptDialog->result()!=QDialog::Accepted) return;
	Paths[index][path]=tcpOptDialog->Path;
}
// set tcp client options ---------------------------------------------------
void MainForm::TcpCliOpt(int index, int path)
{
	tcpOptDialog->Path=Paths[index][path];
	tcpOptDialog->Opt=1;
    for (int i = 0; i < MAXHIST; i++) tcpOptDialog->History[i] = TcpHistory[i];

    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][path] = tcpOptDialog->Path;
    for (int i = 0; i < MAXHIST; i++) TcpHistory[i] = tcpOptDialog->History[i];
}
// set ntrip server options ---------------------------------------------------------
void MainForm::NtripSvrOpt(int index, int path)
{
    tcpOptDialog->Path = Paths[index][path];
    tcpOptDialog->Opt = 2;
    for (int i = 0; i < MAXHIST; i++) tcpOptDialog->History[i] = TcpHistory[i];
    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][path] = tcpOptDialog->Path;
    for (int i = 0; i < MAXHIST; i++) TcpHistory[i] = tcpOptDialog->History[i];
}
// set ntrip client options ---------------------------------------------------------
void MainForm::NtripCliOpt(int index, int path)
{
    tcpOptDialog->Path = Paths[index][path];
    tcpOptDialog->Opt = 3;
    for (int i = 0; i < MAXHIST; i++) tcpOptDialog->History[i] = TcpHistory[i];
    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][path] = tcpOptDialog->Path;
    for (int i = 0; i < MAXHIST; i++) TcpHistory[i] = tcpOptDialog->History[i];
}
// set ntrip caster options ---------------------------------------------------------
void MainForm::NtripCasOpt(int index, int path)
{
    tcpOptDialog->Path = Paths[index][path];
    tcpOptDialog->Opt = 4;
    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][path] = tcpOptDialog->Path;
}
// set udp server options ---------------------------------------------------------
void MainForm::UdpSvrOpt(int index, int path)
{
    tcpOptDialog->Path = Paths[index][path];
    tcpOptDialog->Opt = 6;
    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][path] = tcpOptDialog->Path;
}
// set udp client options ---------------------------------------------------------
void MainForm::UdpCliOpt(int index, int path)
{
    tcpOptDialog->Path = Paths[index][path];
    tcpOptDialog->Opt = 7;
    tcpOptDialog->exec();
    if (tcpOptDialog->result() != QDialog::Accepted) return;

    Paths[index][path] = tcpOptDialog->Path;
}
// set file options ---------------------------------------------------------
void MainForm::FileOpt(int index, int path)
{
    fileOptDialog->Path = Paths[index][path];
    fileOptDialog->setWindowTitle("File Options");
    fileOptDialog->Opt = (index==0)?0:1;
    fileOptDialog->exec();
    if (fileOptDialog->result() != QDialog::Accepted) return;
    Paths[index][path] = fileOptDialog->Path;
}
// undate enable of widgets -------------------------------------------------
void MainForm::UpdateEnable(void)
{
    QComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};
	QLabel *label1[]={LabelOutput1,LabelOutput2,LabelOutput3,LabelOutput4,LabelOutput5,LabelOutput6};
	QLabel *label2[]={Output1Byte,Output2Byte,Output3Byte,Output4Byte,Output5Byte,Output6Byte};
	QLabel *label3[]={Output1Bps,Output2Bps,Output3Bps,Output4Bps,Output5Bps,Output6Bps};
	QPushButton *btn1[]={BtnOutput1,BtnOutput2,BtnOutput3,BtnOutput4,BtnOutput5,BtnOutput6};
	QPushButton *btn2[]={BtnCmd1,BtnCmd2,BtnCmd3,BtnCmd4,BtnCmd5,BtnCmd6};
	QPushButton *btn3[]={BtnConv1,BtnConv2,BtnConv3,BtnConv4,BtnConv5,BtnConv6};
	QPushButton *btn4[]={BtnLog1,BtnLog2,BtnLog3,BtnLog4,BtnLog5,BtnLog6};

    BtnCmd->setEnabled(Input->currentIndex() < 2 || Input->currentIndex() == 3);
    for (int i=0;i<MAXSTR-1;i++) {
        label1[i]->setStyleSheet(QString("color :%1").arg(color2String(type[i]->currentIndex() > 0 ? Qt::black : Qt::gray)));
        label2[i]->setStyleSheet(QString("color :%1").arg(color2String(type[i]->currentIndex() > 0 ? Qt::black : Qt::gray)));
        label3[i]->setStyleSheet(QString("color :%1").arg(color2String(type[i]->currentIndex() > 0 ? Qt::black : Qt::gray)));
        btn1[i]->setEnabled(type[i]->currentIndex() > 0);
        btn2[i]->setEnabled(btn1[i]->isEnabled() && (type[i]->currentIndex() == 1 || type[i]->currentIndex() == 2));
        btn3[i]->setEnabled(btn1[i]->isEnabled() && Input->currentIndex() != 2 && Input->currentIndex() != 4);
        btn4[i]->setEnabled(btn1[i]->isEnabled()&&(type[i]->currentIndex()==1||type[i]->currentIndex()==2));
    }
}
// set task-tray icon -------------------------------------------------------
void MainForm::SetTrayIcon(int index)
{
    QString icon[] = { ":/icons/tray0.bmp", ":/icons/tray1.bmp", ":/icons/tray2.bmp" };

    TrayIcon->setIcon(QIcon(icon[index]));
}
// load options -------------------------------------------------------------
void MainForm::LoadOpt(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);
    QComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};
    int optdef[] = { 10000, 10000, 1000, 32768, 10, 0 };

    Input->setCurrentIndex(settings.value("set/input", 0).toInt());
    for (int i=0;i<MAXSTR-1;i++) {
        type[i]->setCurrentIndex(settings.value(QString("set/output%1").arg(i), 0).toInt());
    }
    TraceLevel = settings.value("set/tracelevel", 0).toInt();
    NmeaReq = settings.value("set/nmeareq", 0).toInt();
    FileSwapMargin = settings.value("set/fswapmargin", 30).toInt();
    RelayBack = settings.value("set/relayback", 30).toInt();
    ProgBarRange = settings.value("set/progbarrange", 30).toInt();
    StaId = settings.value("set/staid", 0).toInt();
    StaSel = settings.value("set/stasel", 0).toInt();
    AntType = settings.value("set/anttype", "").toString();
    RcvType = settings.value("set/rcvtype", "").toString();

    for (int i = 0; i < 6; i++)
        SvrOpt[i] = settings.value(QString("set/svropt_%1").arg(i), optdef[i]).toInt();
    for (int i = 0; i < 3; i++) {
        AntPos[i] = settings.value(QString("set/antpos_%1").arg(i), 0.0).toDouble();
        AntOff[i] = settings.value(QString("set/antoff_%1").arg(i), 0.0).toDouble();
    }
    for (int i = 0; i < MAXSTR - 1; i++) {
        ConvEna[i] = settings.value(QString("conv/ena_%1").arg(i), 0).toInt();
        ConvInp[i] = settings.value(QString("conv/inp_%1").arg(i), 0).toInt();
        ConvOut[i] = settings.value(QString("conv/out_%1").arg(i), 0).toInt();
        ConvMsg[i] = settings.value(QString("conv/msg_%1").arg(i), "").toString();
        ConvOpt[i] = settings.value(QString("conv/opt_%1").arg(i), "").toString();
    }
    for (int i = 0; i < MAXSTR; i++)
        for (int j = 0; j < 2; j++) {
            CmdEna   [i][j] = settings.value(QString("serial/cmdena_%1_%2").arg(i).arg(j), 1).toInt();
            CmdEnaTcp[i][j] = settings.value(QString("tcpip/cmdena_%1_%2").arg(i).arg(j), 1).toInt();
        }
    for (int i = 0; i < MAXSTR; i++) for (int j = 0; j < 4; j++)
            Paths[i][j] = settings.value(QString("path/path_%1_%2").arg(i).arg(j), "").toString();
	for (int i=0;i<MAXSTR;i++) {
		PathLog[i]=settings.value(QString("path/path_log_%1").arg(i),"").toString();
		PathEna[i]=settings.value(QString("path/path_ena_%1").arg(i),0).toInt();
	}
    for (int i = 0; i < MAXSTR; i++)
        for (int j = 0; j < 2; j++) {
            Cmds[i][j] = settings.value(QString("serial/cmd_%1_%2").arg(i).arg(j), "").toString();
            Cmds[i][j] = Cmds[i][j].replace("@@", "\n");
        }
    for (int i = 0; i < MAXSTR; i++)
        for (int j = 0; j < 2; j++) {
            CmdsTcp[i][j] = settings.value(QString("tcpip/cmd_%1_%2").arg(i).arg(j), "").toString();
            CmdsTcp[i][j] = CmdsTcp[i][j].replace("@@", "\n");
        }
    for (int i = 0; i < MAXHIST; i++)
        TcpHistory[i] = settings.value(QString("tcpopt/history%1").arg(i), "").toString();
    for (int i = 0; i < MAXHIST; i++)
        TcpMntpHist[i] = settings.value(QString("tcpopt/mntphist%1").arg(i), "").toString();
    StaPosFile = settings.value("stapos/staposfile", "").toString();
    ExeDirectory = settings.value("dirs/exedirectory", "").toString();
    LocalDirectory = settings.value("dirs/localdirectory", "").toString();
    ProxyAddress = settings.value("dirs/proxyaddress", "").toString();
    LogFile = settings.value("file/logfile",       "").toString();

    UpdateEnable();
}
// save options--------------------------------------------------------------
void MainForm::SaveOpt(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);
    QComboBox *type[]={Output1,Output2,Output3,Output4,Output5,Output6};

    settings.setValue("set/input", Input->currentIndex());
    for (int i=0;i<MAXSTR-1;i++) {
        settings.setValue(QString("set/output%1").arg(i), type[i]->currentIndex());
    }
    settings.setValue("set/tracelevel", TraceLevel);
    settings.setValue("set/nmeareq", NmeaReq);
    settings.setValue("set/fswapmargin", FileSwapMargin);
    settings.setValue("set/relayback", RelayBack);
    settings.setValue("set/progbarrange", ProgBarRange);
    settings.setValue("set/staid", StaId);
    settings.setValue("set/stasel", StaSel);
    settings.setValue("set/anttype", AntType);
    settings.setValue("set/rcvtype", RcvType);

    for (int i = 0; i < 6; i++)
        settings.setValue(QString("set/svropt_%1").arg(i), SvrOpt[i]);
    for (int i = 0; i < 3; i++) {
        settings.setValue(QString("set/antpos_%1").arg(i), AntPos[i]);
        settings.setValue(QString("set/antoff_%1").arg(i), AntOff[i]);
    }
    for (int i = 0; i < MAXSTR - 1; i++) {
        settings.setValue(QString("conv/ena_%1").arg(i), ConvEna[i]);
        settings.setValue(QString("conv/inp_%1").arg(i), ConvInp[i]);
        settings.setValue(QString("conv/out_%1").arg(i), ConvOut[i]);
        settings.setValue(QString("conv/msg_%1").arg(i), ConvMsg[i]);
        settings.setValue(QString("conv/opt_%1").arg(i), ConvOpt[i]);
    }
    for (int i = 0; i < MAXSTR; i++)
        for (int j = 0; j < 2; j++) {
            settings.setValue(QString("serial/cmdena_%1_%2").arg(i).arg(j), CmdEna  [i][j]);
            settings.setValue(QString("tcpip/cmdena_%1_%2").arg(i).arg(j), CmdEnaTcp[i][j]);
        }
    for (int i = 0; i < MAXSTR; i++) for (int j = 0; j < 4; j++)
            settings.setValue(QString("path/path_%1_%2").arg(i).arg(j), Paths[i][j]);

    for (int i=0;i<MAXSTR;i++) {
        settings.setValue(QString("path/path_log_%1").arg(i), PathLog[i]);
        settings.setValue(QString("path/path_ena_%1").arg(i), PathEna[i]);
    }

    for (int i = 0; i < MAXSTR; i++)
        for (int j = 0; j < 2; j++) {
            Cmds[j][i] = Cmds[j][i].replace("\n", "@@");
            settings.setValue(QString("serial/cmd_%1_%2").arg(i).arg(j), Cmds[i][j]);
        }
    for (int i = 0; i < MAXSTR; i++)
        for (int j = 0; j < 2; j++) {
            CmdsTcp[j][i] = CmdsTcp[j][i].replace("\n", "@@");
            settings.setValue(QString("tcpip/cmd_%1_%2").arg(i).arg(j), CmdsTcp[i][j]);
        }
    for (int i = 0; i < MAXHIST; i++)
        settings.setValue(QString("tcpopt/history%1").arg(i), tcpOptDialog->History[i]);

    settings.setValue("stapos/staposfile", StaPosFile);
    settings.setValue("dirs/exedirectory", ExeDirectory);
    settings.setValue("dirs/localdirectory", LocalDirectory);
    settings.setValue("dirs/proxyaddress", ProxyAddress);
    settings.setValue("file/logfile",LogFile);
}
//---------------------------------------------------------------------------
void MainForm::BtnLogClick()
{
	QPushButton *btn[]={BtnLog,BtnLog1,BtnLog2,BtnLog3,BtnLog4,BtnLog5,BtnLog6};
	int i;

	for (i=0;i<MAXSTR;i++) {
		if ((QPushButton *)sender()==btn[i]) break;
	}
	if (i>=MAXSTR) return;

	fileOptDialog->Path=PathLog[i];
	fileOptDialog->PathEna=PathEna[i];
	fileOptDialog->setWindowTitle((i==0)?tr("Input Log Options"):tr("Return Log Options"));
	fileOptDialog->Opt=2;
    fileOptDialog->exec();

	if (fileOptDialog->result()!=QDialog::Accepted) return;
	PathLog[i]=fileOptDialog->Path;
	PathEna[i]=fileOptDialog->PathEna;
}
//---------------------------------------------------------------------------
