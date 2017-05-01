//---------------------------------------------------------------------------
// vplayermain.c : simple video player
//
//          Copyright (C) 2016 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// version : $Revision:$ $Date:$
// history : 2016/09/25 1.0 new
//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QMediaMetaData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include "vplayermain.h"
//#include "vpoptdlg.h"
#include "rtklib.h"

#define VIDEO_TYPE_NONE  0
#define VIDEO_TYPE_MEDIA 1
#define VIDEO_TYPE_MJPEG 2

#define MIN_WINDOW_WIDTH  320
#define MIN_WINDOW_HEIGHT 240

#define PRGNAME     "RTKVPLAYER_QT"
#define PATH_TIME_SYNC "localhost:10071"

extern "C" {
extern int showmsg(const char *, ...)  {return 0;}
extern void settime(gtime_t) {}
extern void settspan(gtime_t, gtime_t) {}
}

//---------------------------------------------------------------------------
MainForm::MainForm(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setAcceptDrops(true);
    double ep[]={2000,1,1,0,0,0};
    VideoType = VIDEO_TYPE_NONE;
    FileIndex = -1;
    Track = NStrBuff = 0;
    TimeStart = epoch2time(ep);
    MjpgRate = 10.0;
    SyncPort = 10071;

    QString file = QApplication::applicationFilePath();
    QFileInfo fi(file);
    IniFile = fi.absolutePath() + "/" + fi.baseName() + ".ini";

    MjpgPlayer = new QMediaPlayer();
    MjpgPlayer->setVideoOutput(VideoWidget);

    Timer1.setInterval(100);
    Timer1.start();

    connect(BtnOpen, SIGNAL(clicked(bool)), this, SLOT(BtnOpenClick()));
    connect(BtnPlay, SIGNAL(clicked(bool)), this, SLOT(BtnPlayClick()));
    connect(BtnStop, SIGNAL(clicked(bool)), this, SLOT(BtnStopClick()));
    connect(BtnClear, SIGNAL(clicked(bool)), this, SLOT(BtnClearClick()));
    connect(BtnExit, SIGNAL(clicked(bool)), this, SLOT(BtnExitClick()));
    connect(BtnPosStart, SIGNAL(clicked(bool)), this, SLOT(BtnPosStartClick()));
    connect(&Timer1, SIGNAL(timeout()),this ,SLOT(Timer1Timer()));
    connect(BtnOption, SIGNAL(clicked(bool)), this, SLOT(BtnOptionsClick()));
    connect(BtnSync, SIGNAL(clicked(bool)),this,SLOT(BtnSyncClick()));
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    LoadOptions();
        
    setWindowTitle(QString("%1 ver. %2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));

    strinitcom();
    strinit(&StrTimeSync);
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *)
{
    StopVideo();
    ClearVideo();
    strclose(&StrTimeSync);
    SaveOptions();
}
//---------------------------------------------------------------------------
void MainForm::BtnOpenClick()
{
    OpenVideo(QFileDialog::getOpenFileNames(this, tr("Open Video"), QString(), tr("Video (*.avi *.mp4 *.mjpg);;All (*.*)")));

    SetVideoPos(0.0f);
}
//---------------------------------------------------------------------------
void MainForm::ReadNmea(QString file)
{
    double time,date,ep[6];

    QFileInfo info(file);
    QString strNewName = info.path() + "/" + info.completeBaseName() + ".nmea";

    QFile fp(strNewName);

    if (!fp.open(QIODevice::ReadOnly)) return;

    QByteArray line;
    while (!fp.atEnd())
    {
        line=fp.readLine();
        if (sscanf(qPrintable(line),"$GPRMC,%lf,A,%*lf,%*c,%*lf,%*c,%*lf,%*lf,%lf",
                   &time,&date)>=2) { //FIXME: make qt-stylish
            ep[2]=floor(date/10000.0); date-=ep[2]*10000.0;
            ep[1]=floor(date/100.0  ); date-=ep[1]*100.0;
            ep[0]=date+2000.0;
            ep[3]=floor(time/10000.0); time-=ep[3]*10000.0;
            ep[4]=floor(time/100.0  ); time-=ep[4]*100.0;
            ep[5]=time;
            TimeStart=timeadd(utc2gpst(epoch2time(ep)),-0.5);
            break;
        }
    }
}
//---------------------------------------------------------------------------
void MainForm::BtnPlayClick()
{
    if (!PlayVideo()) return;
    BtnOpen ->setEnabled(false);
    BtnPlay ->setEnabled(false);
    BtnStop ->setEnabled(true);
    BtnClear->setEnabled(false);
    BtnExit ->setEnabled(false);
}
//---------------------------------------------------------------------------
void MainForm::BtnStopClick()
{
    StopVideo();
    BtnOpen ->setEnabled(true);
    BtnPlay ->setEnabled(true);
    BtnStop ->setEnabled(false);
    BtnClear->setEnabled(true);
    BtnExit ->setEnabled(true);
}
//---------------------------------------------------------------------------
void MainForm::BtnClearClick()
{
    ClearVideo();
}
//---------------------------------------------------------------------------
void MainForm::BtnExitClick()
{
    accept();
}
//---------------------------------------------------------------------------
void MainForm::BtnNextClick()
{
//    NextVideo();
}
//---------------------------------------------------------------------------
void MainForm::BtnPrevClick()
{
//    PrevVideo();
}
//---------------------------------------------------------------------------
void MainForm::BtnPosStartClick()
{
    SetVideoPos(0.0f);
}
//---------------------------------------------------------------------------
void MainForm::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
//---------------------------------------------------------------------------
void MainForm::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/uri-list")) return;

    QString file = QDir::toNativeSeparators(QUrl(event->mimeData()->text()).toLocalFile());

    OpenVideo(QStringList(file));
}

//---------------------------------------------------------------------------
void MainForm::Timer1Timer()
{
    QString str;
    double t, period;
    int width, height;
    gtime_t time;
    char msg[256];
    
    GetVideoTime(t, period);
    GetVideoSize(width, height);

    time = timeadd(TimeStart, t);
    if (FileIndex >= 0) {
        str=QString("%1 (%2 x %3)").arg(time_str(time, 2)).arg(width).arg(height);
    }
    if (BtnSync->isDown()) {
        sprintf(msg, "%s\r\n", time_str(time, 2));
        strwrite(&StrTimeSync, (unsigned char *)msg, (int)strlen(msg));
    }
    ProgressBar->setValue(GetVideoPos()*1000);
    Message1->setText(str);

    Message2->setText(FileName);
    if (ProgressBar->value() >= 1000 && !BtnPlay->isEnabled()) {
        //NextVideo();
    }
}
//---------------------------------------------------------------------------
void MainForm::BtnSyncClick()
{
    if (BtnSync->isDown()) {
        stropen(&StrTimeSync, STR_TCPCLI, STR_MODE_RW, PATH_TIME_SYNC);
    }
    else {
        strclose(&StrTimeSync);
    }
}
//---------------------------------------------------------------------------
void MainForm::OpenVideo(const QStringList &file)
{
    MjpgPlayer->setMedia(QUrl::fromLocalFile(file.at(0))); //FIXME: load all media
    VideoType = VIDEO_TYPE_MJPEG;
    ReadNmea(file.at(0));
}
//---------------------------------------------------------------------------
void MainForm::ClearVideo(void)
{
    MjpgPlayer->setMedia(0);
    FileName = "";
}
//---------------------------------------------------------------------------
int MainForm::PlayVideo(void)
{
    MjpgPlayer->play();
    return 1;
}
//---------------------------------------------------------------------------
void MainForm::StopVideo(void)
{
    MjpgPlayer->stop();
}
//---------------------------------------------------------------------------
void MainForm::NextVideo(void)
{
    if (FileIndex >= Files.count()-1) return;
    OpenVideo(QStringList(Files.at(++FileIndex)));
    if (!BtnPlay->isEnabled()) PlayVideo();
}
//---------------------------------------------------------------------------
void MainForm::PrevVideo(void)
{
    if (FileIndex <= 0) return;
    OpenVideo(QStringList(Files.at(--FileIndex)));
    if (!BtnPlay->isEnabled()) PlayVideo();
}
//---------------------------------------------------------------------------
float MainForm::GetVideoPos(void)
{
    return MjpgPlayer->duration() == 0 ? 0.0 : ((double)MjpgPlayer->position()) / MjpgPlayer->duration();
}
//---------------------------------------------------------------------------
void MainForm::SetVideoPos(float pos)
{
    MjpgPlayer->setPosition(MjpgPlayer->duration() * pos);
}
//---------------------------------------------------------------------------
void MainForm::GetVideoTime(double &time, double &period)
{
    time = MjpgPlayer->position() / 1000.;
    period = MjpgPlayer->duration() / 1000.;
}
//---------------------------------------------------------------------------
void MainForm::GetVideoSize(int &width, int &height)
{
    width=MjpgPlayer->metaData(QMediaMetaData::Resolution).toSize().width();
    height=MjpgPlayer->metaData(QMediaMetaData::Resolution).toSize().height();
}
//---------------------------------------------------------------------------
void MainForm::LoadOptions(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    restoreGeometry(settings.value("window/size", 0).toByteArray());
    SyncAddr = settings.value("option/sync_addr","").toString();
    SyncPort = settings.value("option/sync_port",0).toInt();
}
//---------------------------------------------------------------------------
void MainForm::SaveOptions(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    settings.setValue("window/size", saveGeometry());
    settings.setValue("option/sync_addr", SyncAddr);
    settings.setValue("option/sync_port", SyncPort);

}
//---------------------------------------------------------------------------
void MainForm::BtnOptionClick()
{
/*    VideoPlayerOptDialog *videoPlayerOptDialog= new VideoPlayerOptDialog(this);

    videoPlayerOptDialog->exec();

    delete videoPlayerOptDialog;*/
}
//---------------------------------------------------------------------------



