//---------------------------------------------------------------------------
// rtkget : gnss data downloader
//
//          Copyright (C) 2012 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// options : rtkget [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2012/12/28  1.0 new
//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QFileDialog>
#include <QProcess>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QThread>

//---------------------------------------------------------------------------
#include "rtklib.h"
#include "keydlg.h"
#include "aboutdlg.h"
#include "getmain.h"
#include "getoptdlg.h"
#include "staoptdlg.h"
#include "viewer.h"

#define PRGNAME     "RTKGET"  // program name

#define URL_FILE    "..\\data\\URL_LIST.txt"
#define TEST_FILE   "rtkget_test.txt"
#define TRACE_FILE  "rtkget.trace"

#define MAX_URL     2048
#define MAX_URL_SEL 64
#define MAX_STA     2048
#define MAX_HIST    16

#define MAX(x,y)    ((x)>(y)?(x):(y))

static int abortf=0;          // abort flag

MainForm *mainForm;

// show message in message area ---------------------------------------------
extern "C" {
extern int showmsg(char *format,...)
{
    va_list arg;
    QString str;
    char buff[1024],buff2[10224],*p,*q;
    int len;
    
    va_start(arg,format);
    vsprintf(buff,format,arg);
    va_end(arg);
    
    if ((p=strstr(buff,"STAT="))) {
        QMetaObject::invokeMethod(mainForm->MsgLabel3,"text",Qt::AutoConnection,Q_RETURN_ARG(QString,str));
        len=str.length();
        q=buff2;
        q+=sprintf(q,"%s",qPrintable(str)+MAX(len-66,0));
        if (*(q-1)=='_') q--;
        sprintf(q,"%s",p+5);
        QMetaObject::invokeMethod(mainForm->MsgLabel3,"setText",Qt::QueuedConnection,Q_ARG(QString,QString(buff2)));
    }
    else if ((p=strstr(buff,"->"))) {
        *p='\0';
        QMetaObject::invokeMethod(mainForm->MsgLabel1,"setText",Qt::QueuedConnection,Q_ARG(QString,QString(buff)));
        QMetaObject::invokeMethod(mainForm->MsgLabel2,"setText",Qt::QueuedConnection,Q_ARG(QString,QString(p+2)));
    }
    return abortf;
}
void settspan(gtime_t, gtime_t) {}
void settime(gtime_t) {}
}

class DownloadThread:public QThread
{
public:
    QString usr,pwd,proxy;
    FILE *fp=NULL;
    url_t urls[MAX_URL_SEL];
    gtime_t ts,te;
    double ti;
    char *stas[MAX_STA],dir[1024],msg[1024],path[1024];
    int nsta,nurl,seqnos,seqnoe,opts;
    bool test;

    explicit DownloadThread(const QString LogFile, bool append, bool t){
        seqnos=0;seqnoe=0;opts=0;
        test=t;

        for (int i=0;i<MAX_STA;i++) stas[i]=new char [16];

        if (LogFile!="") {
            reppath(qPrintable(LogFile),path,utc2gpst(timeget()),"","");
            fp=fopen(path,append?"a":"w");
        }
    }
    ~DownloadThread(){
        if (fp)
            fclose(fp);
        for (int i=0;i<MAX_STA;i++) delete [] stas[i];
    }

protected:
    void run(){
        if (test)
            dl_test(ts,te,ti,urls,nurl,stas,nsta,dir,35,0,fp);
        else
            dl_exec(ts,te,ti,seqnos,seqnoe,urls,nurl,stas,nsta,dir,qPrintable(usr),
                qPrintable(pwd),qPrintable(proxy),opts,msg,fp);
    }
};



//---------------------------------------------------------------------------
MainForm::MainForm(QWidget * parent)
    : QWidget(parent)
{
    mainForm=this;
    setupUi(this);

    setlocale(LC_NUMERIC,"C");

    viewer=new TextViewer(this);

    connect(BtnAll,SIGNAL(clicked(bool)),this,SLOT(BtnAllClick()));
    connect(BtnDir,SIGNAL(clicked(bool)),this,SLOT(BtnDirClick()));
    connect(BtnDownload,SIGNAL(clicked(bool)),this,SLOT(BtnDownloadClick()));
    connect(BtnExit,SIGNAL(clicked(bool)),this,SLOT(BtnExitClick()));
    connect(BtnFile,SIGNAL(clicked(bool)),this,SLOT(BtnFileClick()));
    connect(BtnHelp,SIGNAL(clicked(bool)),this,SLOT(BtnHelpClick()));
    connect(BtnKeyword,SIGNAL(clicked(bool)),this,SLOT(BtnKeywordClick()));
    connect(BtnLog,SIGNAL(clicked(bool)),this,SLOT(BtnLogClick()));
    connect(BtnOpts,SIGNAL(clicked(bool)),this,SLOT(BtnOptsClick()));
    connect(BtnStas,SIGNAL(clicked(bool)),this,SLOT(BtnStasClick()));
    connect(BtnTest,SIGNAL(clicked(bool)),this,SLOT(BtnTestClick()));
    connect(BtnTray,SIGNAL(clicked(bool)),this,SLOT(BtnTrayClick()));
    connect(DataType,SIGNAL(currentIndexChanged(int)),this,SLOT(DataTypeChange()));
    connect(SubType,SIGNAL(currentIndexChanged(int)),this,SLOT(DataTypeChange()));
    connect(Dir,SIGNAL(currentIndexChanged(int)),this,SLOT(DirChange()));
    connect(LocalDir,SIGNAL(clicked(bool)),this,SLOT(LocalDirClick()));
    connect(HidePasswd,SIGNAL(clicked(bool)),this,SLOT(HidePasswdClick()));
    connect(DataList,SIGNAL(clicked(QModelIndex)),this,SLOT(DataListClick()));
    connect(StaList,SIGNAL(clicked(QModelIndex)),this,SLOT(StaListClick()));
    connect(&TrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(TrayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(&Timer,SIGNAL(timeout()),this,SLOT(TimerTimer()));

    for (int i=0;i<8;i++)
        Images[i].load(QString(":/buttons/wait%1.bmp").arg(i+1));

    TimerCnt=0;

    TrayIcon.setIcon(QPixmap(":/icons/rtk8.bmp"));
    setWindowIcon(QIcon(":/icons/rtk8.bmp"));

    FormCreate();
    setAcceptDrops(true);
}
//---------------------------------------------------------------------------
void MainForm::FormCreate()
{
    QString str;
    
    QString file=QApplication::applicationFilePath();

    IniFile=QFileInfo(file).absoluteFilePath()+".ini";
    
    setWindowTitle(QString("%1 v.%2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    
    QCommandLineParser parser;
    parser.setApplicationDescription("RTK Get");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption iniFileOption(QStringList() << "i" << "ini-file",
            QCoreApplication::translate("main", "ini file to use"),
            QCoreApplication::translate("main", "ini file"));
    parser.addOption(iniFileOption);

    QCommandLineOption titleOption(QStringList() << "t" << "title",
            QCoreApplication::translate("main", "window title"),
            QCoreApplication::translate("main", "title"));
    parser.addOption(titleOption);

    parser.process(*QApplication::instance());

    if (parser.isSet(iniFileOption))
        IniFile=parser.value(iniFileOption);

    if (parser.isSet(titleOption))
        setWindowTitle(parser.value(titleOption));


    LoadOpt();
    LoadUrl(UrlFile);
    UpdateType();
    UpdateEnable();
    
    if (TraceLevel>0) {
        traceopen(TRACE_FILE);
        tracelevel(TraceLevel);
    }
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *event)
{
    if (event->spontaneous()) return;

    traceclose();
    SaveOpt();
}
//---------------------------------------------------------------------------
void MainForm::BtnFileClick()
{
    QString str;
    gtime_t ts,te;
    double ti;
    char path[1024]=".";

    str=LocalDir->isChecked()?Dir->currentText():MsgLabel2->text();
    GetTime(&ts,&te,&ti);
    if (str!="") reppath(qPrintable(str),path,ts,"","");

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
//---------------------------------------------------------------------------
void MainForm::BtnLogClick()
{
    if (LogFile=="") return;

    viewer->setWindowTitle(LogFile);
    viewer->exec();
    viewer->Read(LogFile);
}
//---------------------------------------------------------------------------
void MainForm::BtnTestClick()
{
    url_t urls[MAX_URL_SEL];
    
    if (BtnTest->text()==tr("&Abort")) {
        BtnTest->setEnabled(false);
        abortf=1;
        return;
    }

    thread=new DownloadThread(TEST_FILE,false,true);
    GetTime(&thread->ts,&thread->te,&thread->ti);
    thread->nurl=SelectUrl(urls);
    if (timediff(thread->ts,thread->te)>0.0||thread->nurl<=0) {
        MsgLabel3->setText(tr("no local data"));
        return;
    }
        
    thread->nsta=SelectSta(thread->stas);
    
    if (LocalDir->isChecked()) {
        strcpy(thread->dir,qPrintable(Dir->currentText()));
    }
    PanelEnable(0);
    BtnTest->setEnabled(true);
    BtnTest->setText(tr("&Abort"));
    MsgLabel1->setStyleSheet("QLabel { color: gray;}");
    MsgLabel3->setText("");
    abortf=0;

    connect(thread,SIGNAL(finished()),this,SLOT(DownloadFinished()));

    thread->start();
    
}
//---------------------------------------------------------------------------
void MainForm::BtnOptsClick()
{
    QString urlfile=UrlFile;
    
    DownOptDialog downOptDialog(this);

    downOptDialog.exec();

    if (downOptDialog.result()!=QDialog::Accepted) return;
    
    if (UrlFile==urlfile) return;
    
    LoadUrl(UrlFile);
    UpdateType();
}
//---------------------------------------------------------------------------
void MainForm::BtnDownloadClick()
{
    QString str;
    int i;

    if (BtnDownload->text()==tr("&Abort")) {
        BtnDownload->setEnabled(false);
        abortf=1;
        return;
    }

    thread=new DownloadThread(LogFile,LogAppend, false);
    GetTime(&thread->ts,&thread->te,&thread->ti);
    
    str=Number->text();
    if (sscanf(qPrintable(str),"%d-%d",&thread->seqnos,&thread->seqnoe)==1) thread->seqnoe=thread->seqnos;
    
    thread->nurl=SelectUrl(thread->urls);
    if (timediff(thread->ts,thread->te)>0.0||thread->nurl<=0) {
        MsgLabel3->setText(tr("no download data"));
        return;
    }
    for (i=0;i<MAX_STA;i++) thread->stas[i]=new char [16];
    
    thread->nsta=SelectSta(thread->stas);
    thread->usr=FtpLogin->text();
    thread->pwd=FtpPasswd->text();
    thread->proxy=ProxyAddr;
    
    if (!SkipExist->isChecked()) thread->opts|=DLOPT_FORCE;
    if (!UnZip    ->isChecked()) thread->opts|=DLOPT_KEEPCMP;
    if (HoldErr )                thread->opts|=DLOPT_HOLDERR;
    if (HoldList)                thread->opts|=DLOPT_HOLDLST;
    
    if (LocalDir->isChecked()) {
        strcpy(thread->dir,qPrintable(Dir->currentText()));
    }
    abortf=0;
    PanelEnable(0);
    BtnDownload->setEnabled(true);
    BtnDownload->setText(tr("&Abort"));
    MsgLabel3->setText("");

    connect(thread,SIGNAL(finished()),this,SLOT(DownloadFinished()));

    Timer.start(200);

    thread->start();
}
//---------------------------------------------------------------------------
void MainForm::DownloadFinished(){

    PanelEnable(1);
    UpdateEnable();
    Timer.stop();

    if (Dir->isEnabled()) AddHist(Dir);

    if (thread->test)
    {
        BtnTest->setText(tr("&Test..."));
        MsgLabel1->setStyleSheet("QLabel { color: bloack;}");
        MsgLabel3->setText("");

        TextViewer *viewer;

        viewer=new TextViewer(this);
        viewer->Option=2;
        viewer->Read(TEST_FILE);
        viewer->setWindowTitle(tr("Local File Test"));
        viewer->exec();

        remove(TEST_FILE);
    } else
    {
        BtnDownload->setText(tr("&Download"));
        MsgLabel3->setText(thread->msg);
    }

    UpdateMsg();
    UpdateEnable();

    thread->deleteLater();

}
//---------------------------------------------------------------------------
void MainForm::BtnExitClick()
{
    close();
}
//---------------------------------------------------------------------------
void MainForm::BtnStasClick()
{
    StaListDialog staListDialog(this);
    staListDialog.exec();

    if (staListDialog.result()!=QDialog::Accepted) return;
    UpdateStaList();
    BtnAll->setText("A");
}
//---------------------------------------------------------------------------
void MainForm::BtnDirClick()
{
    QString dir=Dir->currentText();
    dir=QFileDialog::getExistingDirectory(this,tr("Output Directory"),dir);
    Dir->insertItem(0,dir);
    Dir->setCurrentIndex(0);
}
//---------------------------------------------------------------------------
void MainForm::DirChange()
{
    UpdateMsg();
}
//---------------------------------------------------------------------------
void MainForm::BtnAllClick()
{
    int i,n=0;
    
    StaList->setVisible(false);
    for (i=StaList->count()-1;i>=0;i--) {
        StaList->item(i)->setSelected(BtnAll->text()=="A");
        if (StaList->item(i)->isSelected()) n++;
    }
    StaList->setVisible(true);
    BtnAll->setText(BtnAll->text()=="A"?"C":"A");
    LabelSta->setText(QString(tr("Stations (%1)")).arg(n));
}
//---------------------------------------------------------------------------
void MainForm::BtnKeywordClick()
{
    KeyDialog keyDialog(this);
    keyDialog.Flag=3;
    keyDialog.exec();
}
//---------------------------------------------------------------------------
void MainForm::BtnHelpClick()
{
    AboutDialog aboutDialog(this);
    QString prog=PRGNAME;
    
    aboutDialog.About=prog;
    aboutDialog.IconIndex=8;
    aboutDialog.exec();
}
//---------------------------------------------------------------------------
void MainForm::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
//---------------------------------------------------------------------------
void MainForm::dropEvent(QDropEvent * event)
{
    if (StaList==childAt(event->pos())) {
        LoadSta(event->mimeData()->text());
    }
    event->acceptProposedAction();
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
void MainForm::HidePasswdClick()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void MainForm::LocalDirClick()
{
    UpdateMsg();
    UpdateEnable();
}
//---------------------------------------------------------------------------
void MainForm::DataTypeChange()
{
    UpdateType();
}
//---------------------------------------------------------------------------
void MainForm::DataListClick()
{
    UpdateMsg();
    MsgLabel3->setText("");
}
//---------------------------------------------------------------------------
void MainForm::StaListClick()
{
    UpdateStaList();
}
//---------------------------------------------------------------------------
void MainForm::TimerTimer()
{
    lbImage->setPixmap(Images[TimerCnt%8]);
    qApp->processEvents();
    TimerCnt++;
}
//---------------------------------------------------------------------------
void MainForm::LoadOpt(void)
{
    QSettings setting(IniFile,QSettings::IniFormat);
    QStringList stas;
    
    TimeY1->setDate       (setting.value ("opt/startd","2011/01/01").toDate());
    TimeH1->setTime       (setting.value ("opt/starth",     "00:00").toTime());
    TimeY2->setDate       (setting.value ("opt/endd",  "2011/01/01").toDate());
    TimeH2->setTime       (setting.value ("opt/endh",       "00:00").toTime());
    TimeInt->insertItem (0,setting.value ("opt/timeint",      "24 H").toString());TimeInt->setCurrentIndex(0);
    Number->setText       (setting.value ("opt/number",         "0").toString());
    UrlFile               =setting.value ("opt/urlfile",         "").toString();
    LogFile               =setting.value ("opt/logfile",         "").toString();
    Stations              =setting.value ("opt/stations",        "").toString();
    ProxyAddr             =setting.value ("opt/proxyaddr",       "").toString();
    FtpLogin  ->setText   (setting.value ("opt/login",  "anonymous").toString());
    FtpPasswd ->setText   (setting.value ("opt/passwd",     "user@").toString());
    UnZip     ->setChecked(setting.value ("opt/unzip",            1).toBool());
    SkipExist ->setChecked(setting.value ("opt/skipexist",        1).toBool());
    HidePasswd->setChecked(setting.value ("opt/hidepasswd",       0).toBool());
    HoldErr               =setting.value ("opt/holderr",          0).toInt();
    HoldList              =setting.value ("opt/holdlist",         0).toInt();
    NCol                  =setting.value ("opt/ncol",            35).toInt();
    LogAppend             =setting.value ("opt/logappend",        0).toInt();
    DateFormat            =setting.value ("opt/dateformat",       0).toInt();
    TraceLevel            =setting.value ("opt/tracelevel",       0).toInt();
    LocalDir  ->setChecked(setting.value ("opt/localdirena",      0).toBool());
    Dir       ->insertItem   (0,setting.value ("opt/localdir",        "").toString());Dir->setCurrentIndex(0);
    DataType  ->insertItem   (0,setting.value ("opt/datatype",        "").toString());DataType->setCurrentIndex(0);

    StaList->clear();

    for (int i=0;i<10;i++) {
        stas=setting.value(QString("sta/station%1").arg(i),"").toString().split(",");
        QString s;
        foreach (s,stas) {
            StaList->addItem(s);
        }
    }
    ReadHist(setting,"dir",Dir);
    TextViewer::Color1=setting.value("viewer/color1",QColor(Qt::black)).value<QColor>();
    TextViewer::Color2=setting.value("viewer/color2",QColor(Qt::white)).value<QColor>();
    TextViewer::FontD.setFamily(setting.value ("viewer/fontname","Courier New").toString());
    TextViewer::FontD.setPixelSize(setting.value("viewer/fontsize",9).toInt());
}
//---------------------------------------------------------------------------
void MainForm::SaveOpt(void)
{
    QSettings setting(IniFile,QSettings::IniFormat);
    QString sta;

    
    setting.setValue ("opt/startd",     TimeY1->date()       );
    setting.setValue ("opt/starth",     TimeH1->time()       );
    setting.setValue ("opt/endd",       TimeY2->date()       );
    setting.setValue ("opt/endh",       TimeH2->time()       );
    setting.setValue ("opt/timeint",    TimeInt->currentText());
    setting.setValue ("opt/number",     Number->text()       );
    setting.setValue ("opt/urlfile",    UrlFile            );
    setting.setValue ("opt/logfile",    LogFile            );
    setting.setValue ("opt/stations",   Stations           );
    setting.setValue ("opt/proxyaddr",  ProxyAddr          );
    setting.setValue ("opt/login",      FtpLogin  ->text() );
    setting.setValue ("opt/passwd",     FtpPasswd ->text() );
    setting.setValue ("opt/unzip",      UnZip     ->isChecked());
    setting.setValue ("opt/skipexist",  SkipExist ->isChecked());
    setting.setValue ("opt/hidepasswd", HidePasswd->isChecked());
    setting.setValue ("opt/holderr",    HoldErr            );
    setting.setValue ("opt/holdlist",   HoldList           );
    setting.setValue ("opt/ncol",       NCol               );
    setting.setValue ("opt/logappend",  LogAppend          );
    setting.setValue ("opt/dateformat", DateFormat         );
    setting.setValue ("opt/tracelevel", TraceLevel         );
    setting.setValue ("opt/localdirena",LocalDir ->isChecked());
    setting.setValue ("opt/localdir",   Dir       ->currentText());
    setting.setValue ("opt/datatype",   DataType  ->currentText());

    for (int i=0,j=0;i<10;i++) {
        for (int k=0;k<256&&j<StaList->count();k++) {
            sta.append(k==0?QString(""):QString(",")+StaList->item(i)->text());
        }
        setting.setValue (QString("sta/station%1").arg(i),sta);
    }
    WriteHist(setting,"dir",Dir);
    setting.setValue("viewer/color1",  TextViewer::Color1);
    setting.setValue("viewer/color2",  TextViewer::Color2);
    setting.setValue ("viewer/fontname",TextViewer::FontD.family());
    setting.setValue("viewer/fontsize",TextViewer::FontD.pixelSize());
}
//---------------------------------------------------------------------------
void MainForm::LoadUrl(QString file)
{
    url_t *urls;
    char *p,*subtype;
    char *sel[]={"*"};
    int i,n;
    
    urls=new url_t [MAX_URL];
    
    Types.clear();
    Urls.clear();
    Locals.clear();
    DataType->clear();
    SubType ->clear();
    DataList->clear();
    DataType->addItem(tr("ALL"));
    SubType ->addItem("");
    
    if (file=="") file=URL_FILE; // default url
    
    n=dl_readurls(qPrintable(file),sel,1,urls,MAX_URL);
    
    for (i=0;i<n;i++) {
        Types.append(urls[i].type);
        Urls.append(urls[i].path);
        Locals.append(urls[i].dir );

        if (!(p=strchr(urls[i].type,'_'))) continue;
        *p='\0';
        if (DataType->findText(urls[i].type)==-1)
            DataType->addItem(urls[i].type);

        subtype=p+1;
        if ((p=strchr(subtype,'_'))) *p='\0';
        if (SubType->findText(subtype)==-1)
            SubType->addItem(subtype);
    }
    DataType->setCurrentIndex(0);
    SubType ->setCurrentIndex(0);
    
    delete [] urls;
}
//---------------------------------------------------------------------------
void MainForm::LoadSta(QString file)
{
    QFile f(file);
    QByteArray buff;

    if (!f.open(QIODevice::ReadOnly)) return;
    
    StaList->clear();
    
    while (!f.atEnd())
    {
        buff=f.readLine();
        buff=buff.mid(buff.indexOf('#'));
        StaList->addItem(buff);
    }

    UpdateStaList();
    BtnAll->setText("A");
}
//---------------------------------------------------------------------------
void MainForm::GetTime(gtime_t *ts, gtime_t *te, double *ti)
{
    QString str;
    double eps[6]={2010,1,1},epe[6]={2010,1,1},val;
    
    eps[0]=TimeY1->date().year();eps[1]=TimeY1->date().month();eps[2]=TimeY1->date().day();
    eps[3]=TimeH1->time().hour();eps[4]=TimeH1->time().minute();
    epe[0]=TimeY2->date().year();epe[1]=TimeY2->date().month();epe[2]=TimeY2->date().day();
    epe[3]=TimeH2->time().hour();epe[4]=TimeH2->time().minute();

    *ts=epoch2time(eps);
    *te=epoch2time(epe);
    *ti=86400.0;
    
    str=TimeInt->currentText();
    QStringList tokens=str.split(" ");
    val=tokens.at(0).toDouble();
    if (tokens.size()!=2) *ti=val*3600.0;
    else {
        if (tokens.at(1)==tr("day")) *ti=val*86400.0;
        else if (tokens.at(1)==tr("min")) *ti=val*60.0;
        else *ti=val*3600.0;
    }
}
//---------------------------------------------------------------------------
int MainForm::SelectUrl(url_t *urls)
{
    QString str,file=UrlFile;
    char *types[MAX_URL_SEL];
    int i,nurl=0;
    
    for (i=0;i<MAX_URL_SEL;i++) types[i]=new char [64];
    
    for (i=0;i<DataList->count()&&nurl<MAX_URL_SEL;i++) {
        if (!DataList->item(i)->isSelected()) continue;
        str=DataList->item(i)->text();
        strcpy(types[nurl++],qPrintable(str));
    }
    if (UrlFile=="") file=URL_FILE;
    
    nurl=dl_readurls(qPrintable(file),types,nurl,urls,MAX_URL_SEL);
    
    for (i=0;i<MAX_URL_SEL;i++) delete [] types[i];
    
    return nurl;
}
//---------------------------------------------------------------------------
int MainForm::SelectSta(char **stas)
{
    QString str;
    int i,nsta=0,len;
    
    for (i=0;i<StaList->count()&&nsta<MAX_STA;i++) {
        if (!StaList->item(i)->isSelected()) continue;
        str=StaList->item(i)->text();
        len=str.length();
        if (str.indexOf(' ')!=-1) len=str.indexOf(' ');
        if (len>15) len=15;
        strncpy(stas[nsta],qPrintable(str),len);
        stas[nsta++][len]='\0';
    }
    return nsta;
}
//---------------------------------------------------------------------------
void MainForm::UpdateType(void)
{
    QString str;
    QString type,subtype;
    int i;
    
    DataList->clear();
    
    for (i=0;i<Types.size();i++) {
        str=Types.at(i);
        QStringList tokens=str.split('_');

        type=subtype="";
        if (tokens.size()>1)
        {
            type=tokens.at(0);
            subtype=tokens.at(1);
        } else type=tokens.at(0);

        if (DataType->currentText()!=tr("ALL")&&DataType->currentText()!=type) continue;
        if (SubType ->currentText()!=""&&SubType->currentText()!=subtype) continue;
        DataList->addItem(Types.at(i));
    }
    MsgLabel1->setText("");
    MsgLabel2->setText("");
}
//---------------------------------------------------------------------------
void MainForm::UpdateMsg(void)
{
    int i,j,n=0;
    
    for (i=0;i<DataList->count();i++) {
        if (!DataList->item(i)->isSelected()) continue;
        for (j=0;j<Types.count();j++) {
            if (DataList->item(i)->text()!=Types.at(j)) continue;
            MsgLabel1->setText(Urls.at(j));
            MsgLabel2->setText(LocalDir->isChecked()?Dir->currentText():Locals.at(j));
            Msg1->setToolTip(MsgLabel1->text());
            Msg2->setToolTip(MsgLabel2->text());
            n++;
            break;
        }
    }
    if (n>=2) {
        MsgLabel1->setText(MsgLabel1->text()+" ...");
        MsgLabel2->setText(MsgLabel2->text()+" ...");
    }
}
//---------------------------------------------------------------------------
void MainForm::UpdateStaList(void)
{
    int i,n=0;
    
    for (i=0;i<StaList->count();i++) {
        if (StaList->item(i)->isSelected()) n++;
    }
    LabelSta->setText(QString(tr("Stations (%1)")).arg(n));
}
//---------------------------------------------------------------------------
void MainForm::UpdateEnable(void)
{
    Dir   ->setEnabled(LocalDir->isChecked());
    BtnDir->setEnabled(LocalDir->isChecked());
    if (HidePasswd->isChecked())
        FtpPasswd->setEchoMode(QLineEdit::Password);
    else
        FtpPasswd->setEchoMode(QLineEdit::Normal);
}
//---------------------------------------------------------------------------
void MainForm::PanelEnable(int ena)
{
    Panel1     ->setEnabled(ena);
    Panel2     ->setEnabled(ena);
    BtnFile    ->setEnabled(ena);
    BtnLog     ->setEnabled(ena);
    BtnOpts    ->setEnabled(ena);
    BtnTest    ->setEnabled(ena);
    BtnDownload->setEnabled(ena);
    BtnExit    ->setEnabled(ena);
    BtnAll     ->setEnabled(ena);
    BtnStas    ->setEnabled(ena);
    DataType   ->setEnabled(ena);
    SubType    ->setEnabled(ena);
    DataList   ->setEnabled(ena);
    TimeY1     ->setEnabled(ena);
    TimeH1     ->setEnabled(ena);
    TimeY2     ->setEnabled(ena);
    TimeH2     ->setEnabled(ena);
    TimeInt    ->setEnabled(ena);
    Number     ->setEnabled(ena);
    StaList    ->setEnabled(ena);
    FtpLogin   ->setEnabled(ena);
    FtpPasswd  ->setEnabled(ena);
    SkipExist  ->setEnabled(ena);
    UnZip      ->setEnabled(ena);
    LocalDir   ->setEnabled(ena);
    Dir        ->setEnabled(ena);
    BtnDir     ->setEnabled(ena);
}
// --------------------------------------------------------------------------
void MainForm::ReadHist(QSettings &setting, QString key, QComboBox *combo)
{
    QString s,item;
    int i;
    
    combo->clear();
    
    for (i=0;i<MAX_HIST;i++) {
        item=setting.value(QString("history/%1_%2").arg(key).arg(i,3),"").toString();
        if (item!="") combo->addItem(item);
    }
}
// --------------------------------------------------------------------------
void MainForm::WriteHist(QSettings &setting, QString key, QComboBox * combo)
{
    QString s;
    int i;
    
    for (i=0;i<combo->count();i++) {
        setting.setValue(QString("history/%1_%2").arg(key).arg(i),combo->itemText(i));
    }
}
// --------------------------------------------------------------------------
void MainForm::AddHist(QComboBox *combo)
{
    QString hist=combo->currentText();
    if (hist=="") return;
    int i=combo->currentIndex();
    combo->removeItem(i);
    combo->insertItem(0,hist);
    for (int i=combo->count()-1;i>=MAX_HIST;i--) combo->removeItem(i);
    combo->setCurrentIndex(0);
}
//---------------------------------------------------------------------------
int MainForm::ExecCmd(QString cmd)
{
    return QProcess::startDetached(cmd);
}
//---------------------------------------------------------------------------
