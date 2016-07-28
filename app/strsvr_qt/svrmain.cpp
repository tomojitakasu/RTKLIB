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
#include <QTimer>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QSettings>
#include <QMenu>

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
#include "console.h"
#include "svrmain.h"

//---------------------------------------------------------------------------

#define PRGNAME     "STRSVR-QT"        // program name
#define TRACEFILE   "strsvr.trace"  // debug trace file
#define CLORANGE    QColor(0x00,0xAA,0xFF)

#define MIN(x,y)    ((x)<(y)?(x):(y))

strsvr_t strsvr;


QString color2String(const QColor &c){
    return QString("rgb(%1,%2,%3)").arg(c.red()).arg(c.green()).arg(c.blue());
}

// number to comma-separated number -----------------------------------------
static void num2cnum(int num, char *str)
{
    char buff[256],*p=buff,*q=str;
    int i,n;
    n=sprintf(buff,"%u",(unsigned int)num);
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
    QCoreApplication::setApplicationVersion(QString(VER_RTKLIB)+ " "+ PATCH_LEVEL);

    setWindowIcon(QIcon(":/icons/rtk6.bmp"));

    setlocale(LC_NUMERIC,"C"); // use point as decimal separator in formated output

    QString file=QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile=fi.absolutePath()+"/"+fi.baseName()+".ini";

    TrayIcon = new QSystemTrayIcon(QIcon(":/icons/strsvr_Icon"));

    QMenu *trayMenu= new QMenu(this);
    trayMenu->addAction(MenuExpand);
    trayMenu->addSeparator();
    trayMenu->addAction(MenuStart);
    trayMenu->addAction(MenuStop);
    trayMenu->addSeparator();
    trayMenu->addAction(MenuExit);

    TrayIcon->setContextMenu(trayMenu);

    svrOptDialog = new SvrOptDialog(this);
    console = new Console(this);
    tcpOptDialog= new TcpOptDialog(this);
    serialOptDialog= new SerialOptDialog(this);
    fileOptDialog= new FileOptDialog(this);
    ftpOptDialog= new FtpOptDialog(this);

    StartTime.sec=StartTime.time=EndTime.sec=EndTime.time=0;

    connect(BtnExit,SIGNAL(clicked(bool)),this,SLOT(BtnExitClick()));
    connect(BtnInput,SIGNAL(clicked(bool)),this,SLOT(BtnInputClick()));
    connect(BtnStart,SIGNAL(clicked(bool)),this,SLOT(BtnStartClick()));
    connect(BtnStop,SIGNAL(clicked(bool)),this,SLOT(BtnStopClick()));
    connect(BtnOpt,SIGNAL(clicked(bool)),this,SLOT(BtnOptClick()));
    connect(BtnCmd,SIGNAL(clicked(bool)),this,SLOT(BtnCmdClick()));
    connect(BtnAbout,SIGNAL(clicked(bool)),this,SLOT(BtnAboutClick()));
    connect(BtnStrMon,SIGNAL(clicked(bool)),this,SLOT(BtnStrMonClick()));
    connect(BtnOutput1,SIGNAL(clicked(bool)),this,SLOT(BtnOutput1Click()));
    connect(BtnOutput2,SIGNAL(clicked(bool)),this,SLOT(BtnOutput2Click()));
    connect(BtnOutput3,SIGNAL(clicked(bool)),this,SLOT(BtnOutput3Click()));
    connect(BtnTaskIcon,SIGNAL(clicked(bool)),this,SLOT(BtnTaskIconClick()));
    connect(MenuStart,SIGNAL(triggered(bool)),this,SLOT(MenuStartClick()));
    connect(MenuStop,SIGNAL(triggered(bool)),this,SLOT(MenuStopClick()));
    connect(MenuExit,SIGNAL(triggered(bool)),this,SLOT(MenuExitClick()));
    connect(MenuExpand,SIGNAL(triggered(bool)),this,SLOT(MenuExpandClick()));
    connect(Output1,SIGNAL(currentIndexChanged(int)),this,SLOT(Output1Change()));
    connect(Output2,SIGNAL(currentIndexChanged(int)),this,SLOT(Output2Change()));
    connect(Output3,SIGNAL(currentIndexChanged(int)),this,SLOT(Output3Change()));
    connect(BtnConv1,SIGNAL(clicked(bool)),this,SLOT(BtnConv1Click()));
    connect(BtnConv2,SIGNAL(clicked(bool)),this,SLOT(BtnConv2Click()));
    connect(BtnConv3,SIGNAL(clicked(bool)),this,SLOT(BtnConv3Click()));
    connect(&Timer1,SIGNAL(timeout()),this,SLOT(Timer1Timer()));
    connect(&Timer2,SIGNAL(timeout()),this,SLOT(Timer2Timer()));
    connect(TrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(Input,SIGNAL(currentIndexChanged(int)),this,SLOT(InputChange()));

    Timer1.setInterval(50);
    Timer2.setInterval(100);

    QTimer::singleShot(100,this,SLOT(FormCreate()));
}
// callback on form create --------------------------------------------------
void MainForm::FormCreate()
{
    int autorun=0,tasktray=0;
    
    strsvrinit(&strsvr,3);
    
    setWindowTitle(QString("%1 ver.%2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    

    QCommandLineParser parser;
    parser.setApplicationDescription("stream server");
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
        IniFile=parser.value(iniFileOption);

    LoadOpt();
    
    if (parser.isSet(windowTitleOption))
        setWindowTitle(parser.value(windowTitleOption));
    if (parser.isSet(autoOption)) autorun=1;
    if (parser.isSet(trayOption)) tasktray=1;

    SetTrayIcon(0);
    
    if (tasktray) {
        setVisible(false);
        TrayIcon->setVisible(true);
    }
    if (autorun) {
        SvrStart();
    }
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

    for (int i=0;i<6;i++) svrOptDialog->SvrOpt[i]=SvrOpt[i];
    for (int i=0;i<3;i++) svrOptDialog->AntPos[i]=AntPos[i];
    for (int i=0;i<3;i++) svrOptDialog->AntOff[i]=AntOff[i];
    svrOptDialog->TraceLevel=TraceLevel;
    svrOptDialog->NmeaReq=NmeaReq;
    svrOptDialog->FileSwapMargin=FileSwapMargin;
    svrOptDialog->StaPosFile=StaPosFile;
    svrOptDialog->ExeDirectory=ExeDirectory;
    svrOptDialog->LocalDirectory=LocalDirectory;
    svrOptDialog->ProxyAddress=ProxyAddress;
    svrOptDialog->StaId=StaId;
    svrOptDialog->StaSel=StaSel;
    svrOptDialog->AntType=AntType;
    svrOptDialog->RcvType=RcvType;
    
    svrOptDialog->exec();
    if (svrOptDialog->result()!=QDialog::Accepted) return;
    
    for (int i=0;i<6;i++) SvrOpt[i]=svrOptDialog->SvrOpt[i];
    for (int i=0;i<3;i++) AntPos[i]=svrOptDialog->AntPos[i];
    for (int i=0;i<3;i++) AntOff[i]=svrOptDialog->AntOff[i];
    TraceLevel=svrOptDialog->TraceLevel;
    NmeaReq=svrOptDialog->NmeaReq;
    FileSwapMargin=svrOptDialog->FileSwapMargin;
    StaPosFile=svrOptDialog->StaPosFile;
    ExeDirectory=svrOptDialog->ExeDirectory;
    LocalDirectory=svrOptDialog->LocalDirectory;
    ProxyAddress=svrOptDialog->ProxyAddress;
    StaId=svrOptDialog->StaId;
    StaSel=svrOptDialog->StaSel;
    AntType=svrOptDialog->AntType;
    RcvType=svrOptDialog->RcvType;
}
// callback on button-input-opt ---------------------------------------------
void MainForm::BtnInputClick()
{
    switch (Input->currentIndex()) {
        case 0: SerialOpt(0,0); break;
        case 1: TcpOpt(0,1); break;
        case 2: TcpOpt(0,0); break;
        case 3: TcpOpt(0,3); break;
        case 4: FileOpt(0,0); break;
        case 5: FtpOpt(0,0); break;
        case 6: FtpOpt(0,1); break;
    }
}
// callback on button-input-cmd ---------------------------------------------
void MainForm::BtnCmdClick()
{
    CmdOptDialog *cmdOptDialog= new CmdOptDialog(this);

    if (Input->currentIndex()==0) {
        cmdOptDialog->Cmds[0]=Cmds[0];
        cmdOptDialog->Cmds[1]=Cmds[1];
        cmdOptDialog->CmdEna[0]=CmdEna[0];
        cmdOptDialog->CmdEna[1]=CmdEna[1];
    }
    else {
        cmdOptDialog->Cmds[0]=CmdsTcp[0];
        cmdOptDialog->Cmds[1]=CmdsTcp[1];
        cmdOptDialog->CmdEna[0]=CmdEnaTcp[0];
        cmdOptDialog->CmdEna[1]=CmdEnaTcp[1];
    }

    cmdOptDialog->exec();

    if (cmdOptDialog->result()!=QDialog::Accepted) return;
    if (Input->currentIndex()==0) {
        Cmds[0]  =cmdOptDialog->Cmds[0];
        Cmds[1]  =cmdOptDialog->Cmds[1];
        CmdEna[0]=cmdOptDialog->CmdEna[0];
        CmdEna[1]=cmdOptDialog->CmdEna[1];
    }
    else {
        CmdsTcp[0]  =cmdOptDialog->Cmds[0];
        CmdsTcp[1]  =cmdOptDialog->Cmds[1];
        CmdEnaTcp[0]=cmdOptDialog->CmdEna[0];
        CmdEnaTcp[1]=cmdOptDialog->CmdEna[1];
    }

    delete cmdOptDialog;
}
// callback on button-output1-opt -------------------------------------------
void MainForm::BtnOutput1Click()
{
    switch (Output1->currentIndex()) {
        case 1: SerialOpt(1,0); break;
        case 2: TcpOpt(1,1); break;
        case 3: TcpOpt(1,0); break;
        case 4: TcpOpt(1,2); break;
        case 5: FileOpt(1,1); break;
    }
}
// callback on button-output2-opt -------------------------------------------
void MainForm::BtnOutput2Click()
{
    switch (Output2->currentIndex()) {
        case 1: SerialOpt(2,0); break;
        case 2: TcpOpt(2,1); break;
        case 3: TcpOpt(2,0); break;
        case 4: TcpOpt(2,2); break;
        case 5: FileOpt(2,1); break;
    }
}
// callback on button-output3-opt -------------------------------------------
void MainForm::BtnOutput3Click()
{
    switch (Output3->currentIndex()) {
        case 1: SerialOpt(3,0); break;
        case 2: TcpOpt(3,1); break;
        case 3: TcpOpt(3,0); break;
        case 4: TcpOpt(3,2); break;
        case 5: FileOpt(3,1); break; 
    }
}
// callback on button-output1-conv ------------------------------------------
void MainForm::BtnConv1Click()
{
    ConvDialog *convDialog=new ConvDialog(this);

    convDialog->ConvEna=ConvEna[0];
    convDialog->ConvInp=ConvInp[0];
    convDialog->ConvOut=ConvOut[0];
    convDialog->ConvMsg=ConvMsg[0];
    convDialog->ConvOpt=ConvOpt[0];

    convDialog->exec();
    if (convDialog->result()!=QDialog::Accepted) return;

    ConvEna[0]=convDialog->ConvEna;
    ConvInp[0]=convDialog->ConvInp;
    ConvOut[0]=convDialog->ConvOut;
    ConvMsg[0]=convDialog->ConvMsg;
    ConvOpt[0]=convDialog->ConvOpt;

    delete convDialog;
}
// callback on button-output2-conv ------------------------------------------
void MainForm::BtnConv2Click()
{
    ConvDialog *convDialog=new ConvDialog(this);

    convDialog->ConvEna=ConvEna[1];
    convDialog->ConvInp=ConvInp[1];
    convDialog->ConvOut=ConvOut[1];
    convDialog->ConvMsg=ConvMsg[1];
    convDialog->ConvOpt=ConvOpt[1];

    convDialog->exec();
    if (convDialog->result()!=QDialog::Accepted) return;

    ConvEna[1]=convDialog->ConvEna;
    ConvInp[1]=convDialog->ConvInp;
    ConvOut[1]=convDialog->ConvOut;
    ConvMsg[1]=convDialog->ConvMsg;
    ConvOpt[1]=convDialog->ConvOpt;

    delete convDialog;
}
// callback on button-output3-conv ------------------------------------------
void MainForm::BtnConv3Click()
{
    ConvDialog *convDialog=new ConvDialog(this);

    convDialog->ConvEna=ConvEna[2];
    convDialog->ConvInp=ConvInp[2];
    convDialog->ConvOut=ConvOut[2];
    convDialog->ConvMsg=ConvMsg[2];
    convDialog->ConvOpt=ConvOpt[2];

    convDialog->exec();
    if (convDialog->result()!=QDialog::Accepted) return;

    ConvEna[2]=convDialog->ConvEna;
    ConvInp[2]=convDialog->ConvInp;
    ConvOut[2]=convDialog->ConvOut;
    ConvMsg[2]=convDialog->ConvMsg;
    ConvOpt[2]=convDialog->ConvOpt;

    delete convDialog;
}
// callback on buttn-about --------------------------------------------------
void MainForm::BtnAboutClick()
{
    AboutDialog *aboutDialog=new AboutDialog(this);

    aboutDialog->About=PRGNAME;
    aboutDialog->IconIndex=6;
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
    if (reason!=QSystemTrayIcon::DoubleClick) return;

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
    console->setWindowTitle("Input Monitor");
    console->show();
}
// callback on input type change --------------------------------------------
void MainForm::InputChange()
{
    UpdateEnable();
}
// callback on output1 type change ------------------------------------------
void MainForm::Output1Change()
{
    UpdateEnable(); 
}
// callback on output2 type change ------------------------------------------
void MainForm::Output2Change()
{
    UpdateEnable();
}
// callback on output3 type change ------------------------------------------
void MainForm::Output3Change()
{
    UpdateEnable();
}
// callback on interval timer -----------------------------------------------
void MainForm::Timer1Timer()
{
    QColor color[]={Qt::red,Qt::gray,CLORANGE,Qt::green,QColor(0x00,0xff,0x00)};
    QLabel *e0[]={IndInput,IndOutput1,IndOutput2,IndOutput3};
    QLabel *e1[]={InputByte,Output1Byte,Output2Byte,Output3Byte};
    QLabel *e2[]={InputBps,Output1Bps,Output2Bps,Output3Bps};
    gtime_t time=utc2gpst(timeget());
    int stat[4]={0},byte[4]={0},bps[4]={0};
    char msg[MAXSTRMSG*4]="",s1[256],s2[256];
    double ctime,t[4];
    
    strsvrstat(&strsvr,stat,byte,bps,msg);
    for (int i=0;i<4;i++) {
        num2cnum(byte[i],s1);
        num2cnum(bps[i],s2);
        e0[i]->setStyleSheet(QString("background-color: %1").arg(color2String(color[stat[i]+1])));
        e1[i]->setText(s1);
        e2[i]->setText(s2);
    }
    Progress->setValue(!stat[0]?0:MIN(100,(int)(fmod(byte[0]/500.0,110.0))));
    
    time2str(time,s1,0);
    Time->setText(QString(tr("%1 GPST")).arg(s1));
    
    if (Panel1->isEnabled()) {
        ctime=timediff(EndTime,StartTime);
    }
    else {
        ctime=timediff(time,StartTime);
    }
    ctime=floor(ctime);
    t[0]=floor(ctime/86400.0); ctime-=t[0]*86400.0;
    t[1]=floor(ctime/3600.0 ); ctime-=t[1]*3600.0;
    t[2]=floor(ctime/60.0   ); ctime-=t[2]*60.0;
    t[3]=ctime;
    ConTime->setText(QString("%1d %2:%3:%4").arg(t[0],0,'f',0).arg(t[1],2,'f',0,QChar('0')).arg(t[2],2,'f',0,QChar('0')).arg(t[3],2,'f',2,QChar('0')));
    
    num2cnum(byte[0],s1); num2cnum(bps[0],s2);
    TrayIcon->setToolTip(QString(tr("%1 bytes %2 bps")).arg(s1).arg(s2));
    SetTrayIcon(stat[0]<=0?0:(stat[0]==3?2:1));
    
    Message->setText(msg);
}
// start stream server ------------------------------------------------------
void MainForm::SvrStart(void)
{
    strconv_t *conv[3]={0};
    static char str[4][1024];
    int itype[]={
        STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP
    };
    int otype[]={
        STR_NONE,STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE
    };
    int ip[]={0,1,1,1,2,3,3},strs[4]={0},opt[7]={0},n;
    char *paths[4],filepath[1024],buff[1024];
    char cmd[1024];
    char *ant[3]={0},*rcv[3]={0},*p;
    FILE *fp;
    
    if (TraceLevel>0) {
        traceopen(TRACEFILE);
        tracelevel(TraceLevel);
    }
    for (int i=0;i<4;i++) paths[i]=str[i];
    
    strs[0]=itype[Input->currentIndex()];
    strs[1]=otype[Output1->currentIndex()];
    strs[2]=otype[Output2->currentIndex()];
    strs[3]=otype[Output3->currentIndex()];
    
    strcpy(paths[0],qPrintable(Paths[0][ip[Input->currentIndex()]]));
    strcpy(paths[1],!Output1->currentIndex()?"":qPrintable(Paths[1][ip[Output1->currentIndex()-1]]));
    strcpy(paths[2],!Output2->currentIndex()?"":qPrintable(Paths[2][ip[Output2->currentIndex()-1]]));
    strcpy(paths[3],!Output3->currentIndex()?"":qPrintable(Paths[3][ip[Output3->currentIndex()-1]]));
    
    if (Input->currentIndex()==0) {
        if (CmdEna[0]) strncpy(cmd,qPrintable(Cmds[0]),1024);
    }
    else if (Input->currentIndex()==1||Input->currentIndex()==3) {
        if (CmdEnaTcp[0]) strncpy(cmd,qPrintable(CmdsTcp[0]),1024);
    }
    for (int i=0;i<5;i++) {
        opt[i]=SvrOpt[i];
    }
    opt[5]=NmeaReq?SvrOpt[5]:0;
    opt[6]=FileSwapMargin;
    
    for (int i=1;i<4;i++) {
        if (strs[i]!=STR_FILE) continue;
        strcpy(filepath,paths[i]);
        if (strstr(filepath,"::A")) continue;
        if ((p=strstr(filepath,"::"))) *p='\0';
        if (!(fp=fopen(filepath,"r"))) continue;
        fclose(fp);
        if (QMessageBox::question(this,tr("Overwrite"),tr("File %1 exists. \nDo you want to overwrite?").arg(filepath))!=QMessageBox::Yes) return;
    }
    strsetdir(qPrintable(LocalDirectory));
    strsetproxy(qPrintable(ProxyAddress));
    
    for (int i=0;i<3;i++) {
        if (!ConvEna[i]) continue;
        if (!(conv[i]=strconvnew(ConvInp[i],ConvOut[i],qPrintable(ConvMsg[i]),
                                 StaId,StaSel,qPrintable(ConvOpt[i])))) continue;
        strcpy(buff,qPrintable(AntType));
        for (p=strtok(buff,","),n=0;p&&n<3;p=strtok(NULL,",")) ant[n++]=p;
        strcpy(conv[i]->out.sta.antdes,ant[0]);
        strcpy(conv[i]->out.sta.antsno,ant[1]);
        conv[i]->out.sta.antsetup=atoi(ant[2]);
        strcpy(buff,qPrintable(RcvType));
        for (p=strtok(buff,","),n=0;p&&n<3;p=strtok(NULL,",")) rcv[n++]=p;
        strcpy(conv[i]->out.sta.rectype,rcv[0]);
        strcpy(conv[i]->out.sta.recver ,rcv[1]);
        strcpy(conv[i]->out.sta.recsno ,rcv[2]);
        matcpy(conv[i]->out.sta.pos,AntPos,3,1);
        matcpy(conv[i]->out.sta.del,AntOff,3,1);
    }
    // stream server start
    if (!strsvrstart(&strsvr,opt,strs,paths,conv,cmd,AntPos)) return;
    
    StartTime=utc2gpst(timeget());
    Panel1    ->setEnabled(false);
    BtnStart  ->setVisible(false);
    BtnStop   ->setVisible(true);
    BtnOpt    ->setEnabled(false);
    BtnExit   ->setEnabled(false);
    MenuStart ->setEnabled(false);
    MenuStop  ->setEnabled(true);
    MenuExit  ->setEnabled(false);
    SetTrayIcon(1);
}
// stop stream server -------------------------------------------------------
void MainForm::SvrStop(void)
{
    char cmd[1024];
    
    if (Input->currentIndex()==0) {
        if (CmdEna[1]) strncpy(cmd,qPrintable(Cmds[1]),1024);
    }
    else if (Input->currentIndex()==1||Input->currentIndex()==3) {
        if (CmdEnaTcp[1]) strncpy(cmd,qPrintable(CmdsTcp[1]),1024);
    }
    strsvrstop(&strsvr,cmd);
    
    EndTime=utc2gpst(timeget());
    Panel1    ->setEnabled(true);
    BtnStart  ->setVisible(true);
    BtnStop   ->setVisible(false);
    BtnOpt    ->setEnabled(true);
    BtnExit   ->setEnabled(true);
    MenuStart ->setEnabled(true);
    MenuStop  ->setEnabled(false);
    MenuExit  ->setEnabled(true);
    SetTrayIcon(0);
    
    for (int i=0;i<3;i++) {
        if (ConvEna[i]) strconvfree(strsvr.conv[i]);
    }
    if (TraceLevel>0) traceclose();
}
// callback on interval timer for stream monitor ----------------------------
void MainForm::Timer2Timer()
{
    unsigned char *msg=0;
    int len;
    
    lock(&strsvr.lock);
    
    len=strsvr.npb;
    if (len>0&&(msg=(unsigned char *)malloc(len))) {
        memcpy(msg,strsvr.pbuf,len);
        strsvr.npb=0;
    }
    unlock(&strsvr.lock);
    
    if (len<=0||!msg) return;
    
    console->AddMsg(msg,len);
    
    free(msg);
}
// set serial options -------------------------------------------------------
void MainForm::SerialOpt(int index, int opt)
{
    serialOptDialog->Path=Paths[index][0];
    serialOptDialog->Opt=opt;

    serialOptDialog->exec();
    if (serialOptDialog->result()!=QDialog::Accepted) return;
    Paths[index][0]=serialOptDialog->Path;
}
// set tcp/ip options -------------------------------------------------------
void MainForm::TcpOpt(int index, int opt)
{
    tcpOptDialog->Path=Paths[index][1];
    tcpOptDialog->Opt=opt;
    for (int i=0;i<MAXHIST;i++) tcpOptDialog->History[i]=TcpHistory[i];
    for (int i=0;i<MAXHIST;i++) tcpOptDialog->MntpHist[i]=TcpMntpHist[i];

    tcpOptDialog->exec();
    if (tcpOptDialog->result()!=QDialog::Accepted) return;

    Paths[index][1]=tcpOptDialog->Path;
    for (int i=0;i<MAXHIST;i++) TcpHistory[i]=tcpOptDialog->History[i];
    for (int i=0;i<MAXHIST;i++) TcpMntpHist[i]=tcpOptDialog->MntpHist[i];

}
// set file options ---------------------------------------------------------
void MainForm::FileOpt(int index, int opt)
{
    fileOptDialog->Path=Paths[index][2];
    fileOptDialog->Opt=opt;

    fileOptDialog->exec();
    if (fileOptDialog->result()!=QDialog::Accepted) return;
    Paths[index][2]=fileOptDialog->Path;
}
// set ftp/http options -----------------------------------------------------
void MainForm::FtpOpt(int index, int opt)
{
    ftpOptDialog->Path=Paths[index][3];
    ftpOptDialog->Opt=opt;

    ftpOptDialog->exec();
    if (ftpOptDialog->result()!=QDialog::Accepted) return;

    Paths[index][3]=ftpOptDialog->Path;

}
// undate enable of widgets -------------------------------------------------
void MainForm::UpdateEnable(void)
{
    BtnCmd->setEnabled(Input->currentIndex()<2||Input->currentIndex()==3);
    LabelOutput1->setStyleSheet(QString("color :%1").arg(color2String(Output1->currentIndex()>0?Qt::black:Qt::gray)));
    LabelOutput2->setStyleSheet(QString("color :%1").arg(color2String(Output2->currentIndex()>0?Qt::black:Qt::gray)));
    LabelOutput3->setStyleSheet(QString("color :%1").arg(color2String(Output3->currentIndex()>0?Qt::black:Qt::gray)));
    Output1Byte ->setStyleSheet(QString("color :%1").arg(color2String(Output1->currentIndex()>0?Qt::black:Qt::gray)));
    Output2Byte ->setStyleSheet(QString("color :%1").arg(color2String(Output2->currentIndex()>0?Qt::black:Qt::gray)));
    Output3Byte ->setStyleSheet(QString("color :%1").arg(color2String(Output3->currentIndex()>0?Qt::black:Qt::gray)));
    Output1Bps  ->setStyleSheet(QString("color :%1").arg(color2String(Output1->currentIndex()>0?Qt::black:Qt::gray)));
    Output2Bps  ->setStyleSheet(QString("color :%1").arg(color2String(Output2->currentIndex()>0?Qt::black:Qt::gray)));
    Output3Bps  ->setStyleSheet(QString("color :%1").arg(color2String(Output3->currentIndex()>0?Qt::black:Qt::gray)));
    BtnOutput1->setEnabled(Output1->currentIndex()>0);
    BtnOutput2->setEnabled(Output2->currentIndex()>0);
    BtnOutput3->setEnabled(Output3->currentIndex()>0);
    BtnConv1  ->setEnabled(BtnOutput1->isEnabled());
    BtnConv2  ->setEnabled(BtnOutput2->isEnabled());
    BtnConv3  ->setEnabled(BtnOutput3->isEnabled());
}
// set task-tray icon -------------------------------------------------------
void MainForm::SetTrayIcon(int index)
{
    QString icon[]={":/icons/tray0.bmp",":/icons/tray1.bmp",":/icons/tray2.bmp"};
    TrayIcon->setIcon(QIcon(icon[index]));
}
// load options -------------------------------------------------------------
void MainForm::LoadOpt(void)
{
    QSettings settings(IniFile,QSettings::IniFormat);
    int optdef[]={10000,10000,1000,32768,10,0};
    
    Input  ->setCurrentIndex(settings.value("set/input",       0).toInt());
    Output1->setCurrentIndex(settings.value("set/output1",     0).toInt());
    Output2->setCurrentIndex(settings.value("set/output2",     0).toInt());
    Output3->setCurrentIndex(settings.value("set/output3",     0).toInt());
    TraceLevel        =settings.value("set/tracelevel",  0).toInt();
    NmeaReq           =settings.value("set/nmeareq",     0).toInt();
    FileSwapMargin    =settings.value("set/fswapmargin",30).toInt();
    StaId             =settings.value("set/staid"       ,0).toInt();
    StaSel            =settings.value("set/stasel"      ,0).toInt();
    AntType           =settings.value("set/anttype",    "").toString();
    RcvType           =settings.value("set/rcvtype",    "").toString();
    
    for (int i=0;i<6;i++) {
        SvrOpt[i]=settings.value(QString("set/svropt_%1").arg(i),optdef[i]).toInt();
    }
    for (int i=0;i<3;i++) {
        AntPos[i]=settings.value(QString("set/antpos_%1").arg(i),0.0).toDouble();
        AntOff[i]=settings.value(QString("set/antoff_%1").arg(i),0.0).toDouble();
    }
    for (int i=0;i<3;i++) {
        ConvEna[i]=settings.value(QString("conv/ena_%1").arg(i), 0).toInt();
        ConvInp[i]=settings.value(QString("conv/inp_%1").arg(i), 0).toInt();
        ConvOut[i]=settings.value(QString("conv/out_%1").arg(i), 0).toInt();
        ConvMsg[i]=settings.value (QString("conv/msg_%1").arg(i),"").toString();
        ConvOpt[i]=settings.value (QString("conv/opt_%1").arg(i),"").toString();
    }
    for (int i=0;i<2;i++) {
        CmdEna   [i]=settings.value(QString("serial/cmdena_%1").arg(i),1).toInt();
        CmdEnaTcp[i]=settings.value(QString("tcpip/cmdena_%1").arg(i),1).toInt();
    }
    for (int i=0;i<4;i++) for (int j=0;j<4;j++) {
        Paths[i][j]=settings.value(QString("path/path_%1_%2").arg(i).arg(j),"").toString();
    }
    for (int i=0;i<2;i++) {
        Cmds[i]=settings.value(QString("serial/cmd_%1").arg(i),"").toString();
        Cmds[i]=Cmds[i].replace("@@","\n");
    }
    for (int i=0;i<2;i++) {
        CmdsTcp[i]=settings.value(QString("tcpip/cmd_%1").arg(i),"").toString();
        CmdsTcp[i]=CmdsTcp[i].replace("@@","\n");
    }
    for (int i=0;i<MAXHIST;i++) {
        TcpHistory[i]=settings.value(QString("tcpopt/history%1").arg(i),"").toString();
    }
    for (int i=0;i<MAXHIST;i++) {
        TcpMntpHist[i]=settings.value(QString("tcpopt/mntphist%1").arg(i),"").toString();
    }
    StaPosFile    =settings.value("stapos/staposfile",    "").toString();
    ExeDirectory  =settings.value("dirs/exedirectory",  "").toString();
    LocalDirectory=settings.value("dirs/localdirectory","").toString();
    ProxyAddress  =settings.value("dirs/proxyaddress",  "").toString();
    
    UpdateEnable();
}
// save options--------------------------------------------------------------
void MainForm::SaveOpt(void)
{
    QSettings settings(IniFile,QSettings::IniFormat);
    
    settings.setValue("set/input",      Input  ->currentIndex());
    settings.setValue("set/output1",    Output1->currentIndex());
    settings.setValue("set/output2",    Output2->currentIndex());
    settings.setValue("set/output3",    Output3->currentIndex());
    settings.setValue("set/tracelevel", TraceLevel);
    settings.setValue("set/nmeareq",    NmeaReq);
    settings.setValue("set/fswapmargin",FileSwapMargin);
    settings.setValue("set/staid",      StaId);
    settings.setValue("set/stasel",     StaSel);
    settings.setValue("set/anttype",    AntType);
    settings.setValue("set/rcvtype",    RcvType);
    
    for (int i=0;i<6;i++) {
        settings.setValue(QString("set/svropt_%1").arg(i),SvrOpt[i]);
    }
    for (int i=0;i<3;i++) {
        settings.setValue(QString("set/antpos_%1").arg(i),AntPos[i]);
        settings.setValue(QString("set/antoff_%1").arg(i),AntOff[i]);
    }
    for (int i=0;i<3;i++) {
        settings.setValue(QString("conv/ena_%1").arg(i),ConvEna[i]);
        settings.setValue(QString("conv/inp_%1").arg(i),ConvInp[i]);
        settings.setValue(QString("conv/out_%1").arg(i),ConvOut[i]);
        settings.setValue(QString("conv/msg_%1").arg(i),ConvMsg[i]);
        settings.setValue(QString("conv/opt_%1").arg(i),ConvOpt[i]);
    }
    for (int i=0;i<2;i++) {
        settings.setValue(QString("serial/cmdena_%1").arg(i),CmdEna   [i]);
        settings.setValue(QString("tcpip/cmdena_%1").arg(i),CmdEnaTcp[i]);
    }
    for (int i=0;i<4;i++) for (int j=0;j<4;j++) {
        settings.setValue(QString("path/path_%1_%2").arg(i).arg(j),Paths[i][j]);
    }
    for (int i=0;i<2;i++) {
        Cmds[i]=Cmds[i].replace("\n","@@");
        settings.setValue(QString("serial/cmd_%1").arg(i),Cmds[i]);
    }
    for (int i=0;i<2;i++) {
        CmdsTcp[i]=CmdsTcp[i].replace("\n","@@");
        settings.setValue(QString("tcpip/cmd_%1").arg(i),CmdsTcp[i]);
    }
    for (int i=0;i<MAXHIST;i++) {
        settings.setValue(QString("tcpopt/history%1").arg(i),tcpOptDialog->History[i]);
    }
    for (int i=0;i<MAXHIST;i++) {
        settings.setValue(QString("tcpopt/mntphist%1").arg(i),tcpOptDialog->MntpHist[i]);
    }
    settings.setValue("stapos/staposfile"    ,StaPosFile    );
    settings.setValue("dirs/exedirectory"  ,ExeDirectory  );
    settings.setValue("dirs/localdirectory",LocalDirectory);
    settings.setValue("dirs/proxyaddress"  ,ProxyAddress  );
}
//---------------------------------------------------------------------------
