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
#include "rtklib.h"

#define VIDEO_TYPE_NONE  0
#define VIDEO_TYPE_MEDIA 1
#define VIDEO_TYPE_MJPEG 2

#define MIN_WINDOW_WIDTH  320
#define MIN_WINDOW_HEIGHT 240

#define PRGNAME     "RTKVPLAYER_QT"

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
    VideoType = VIDEO_TYPE_NONE;
    FileName = "";
    Track = 0;

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
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    LoadOptions();
        
    setWindowTitle(QString("%1 ver. %2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *)
{
    StopVideo();
    ClearVideo();
    
    SaveOptions();
}
//---------------------------------------------------------------------------
void MainForm::BtnOpenClick()
{
    OpenVideo(QFileDialog::getOpenFileName(this, tr("Open Video"), QString(), tr("Video (*.avi *.mp4 *.mjpg);;All (*.*)")));

    SetVideoPos(0.0f);
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
void MainForm::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
//---------------------------------------------------------------------------
void MainForm::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/uri-list")) return;

    QString file = QUrl(event->mimeData()->text()).toLocalFile();

    OpenVideo(file);
}

//---------------------------------------------------------------------------
void MainForm::Timer1Timer()
{
    QString str;
    double time, period;
    int width, height;
    
    GetVideoTime(time, period);
    GetVideoSize(width, height);
    if (FileName != "") {
        str=QString("%1 / %2 s (%3 x %4)").arg(time,0,'f',1).arg(period,0,'f',1).arg(width).arg(height);
    }
    ProgressBar->setValue(GetVideoPos()*1000);
    Message1->setText(str);
    Message2->setText(FileName);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void MainForm::BtnPosStartClick()
{
    SetVideoPos(0.0f);
}
//---------------------------------------------------------------------------
void MainForm::OpenVideo(QString file)
{
    MjpgPlayer->setMedia(QUrl::fromLocalFile(file));
    FileName = file;
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
}
//---------------------------------------------------------------------------
void MainForm::SaveOptions(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    settings.setValue("window/size", saveGeometry());

}
//---------------------------------------------------------------------------




