//---------------------------------------------------------------------------
// rtkvideo : video capture
//
//          Copyright (C) 2016 by T.TAKASU, All rights reserved.
//          ported to Qt by Jens Reimann
//
// version : $Revision:$ $Date:$
// history : 2016/09/25 1.0 new
//---------------------------------------------------------------------------

#include <QShowEvent>
#include <QCloseEvent>
#include <QCameraInfo>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QBuffer>
#include <QDebug>

#include "videomain.h"
#include "videoopt.h"
#include "cameraframegrabber.h"

#define DEFAULT_TCP_PORT 10033
#define DEFAULT_CAP_WIDTH 500
#define DEFAULT_CAP_HEIGHT 500

#define STR_BUF_SIZE 4096000

#define MIN_WINDOW_WIDTH  320
#define MIN_WINDOW_HEIGHT 240

#define PRGNAME     "RTKVIDEO_QT"

#define MIN(x,y)    ((x)<(y)?(x):(y))

extern "C" {
extern int showmsg(const char *, ...)  {return 0;}
extern void settime(gtime_t) {}
extern void settspan(gtime_t, gtime_t) {}
}

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

    camera = NULL;
    StartTime.time = 0;
    CaptureTime.time = 0;
    Video_Width = Video_Height = FRM = FRM0 = FPS = 0;
    DevName = "";
    OutFile = "";
    Profile = CapSizeEna = Annotation = 0;
    TcpPortEna = OutFileEna = OutTimeTag = FileSwap = 0;
    CapWidth = DEFAULT_CAP_WIDTH;
    CapHeight = DEFAULT_CAP_HEIGHT;
    TcpPortNo = DEFAULT_TCP_PORT;

    setWindowTitle(QString("%1 ver. %2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    videoOptDlg = new VideoOptDlg(this);

    connect(BtnStart, SIGNAL(clicked(bool)), this, SLOT(BtnStartClick()));
    connect(BtnStop, SIGNAL(clicked(bool)), this, SLOT(BtnStopClick()));
    connect(BtnExit, SIGNAL(clicked(bool)), this, SLOT(BtnExitClick()));
    connect(BtnOpt, SIGNAL(clicked(bool)), this, SLOT(BtnOptClick()));

    Time.start();
}
//---------------------------------------------------------------------------
void MainForm::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    const int str_opt[8] = {0, 0, 0, STR_BUF_SIZE, 0};
    
    LoadOptions();
    
    DevName = QCameraInfo::defaultCamera().deviceName();
    camera = new QCamera(DevName.toLatin1(),this);
    
    strsetopt(str_opt);
    
    for (int i = 0; i < 2; i++) {
        strinit(OutStr+i);
    }
    initlock(&DeviceLock);
}
//---------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent *)
{
    if (camera && camera->status()==QCamera::ActiveStatus) {
        camera->stop();
    }
    SaveOptions();
    
}
//---------------------------------------------------------------------------
void MainForm::BtnStartClick()
{
    if (!CaptureStart()) return;
    
    BtnStart->setEnabled(false);
    BtnStop ->setEnabled(true);
    BtnOpt  ->setEnabled(false);
    BtnExit ->setEnabled(false);
}
//---------------------------------------------------------------------------
void MainForm::BtnStopClick()
{
    CaptureStop();
    
    BtnStart->setEnabled(true);
    BtnStop ->setEnabled(false);
    BtnOpt  ->setEnabled(true);
    BtnExit ->setEnabled(true);
}
//---------------------------------------------------------------------------
void MainForm::BtnOptClick()
{    
    if (videoOptDlg->exec() != QDialog::Accepted) return;
}
//---------------------------------------------------------------------------
void MainForm::BtnExitClick()
{
    close();
}
//---------------------------------------------------------------------------
int MainForm::CaptureStart(void)
{
    const char *swap[] = {
        "", "::S=0.05", "::S=0.1", "::S=0.25", "::S=0.5", "::S=1", "::S=2"
    };
    
   camera = new QCamera(DevName.toLatin1());

   if (!camera->isAvailable()) return -1;

   camera->setCaptureMode(QCamera::CaptureVideo);

   if (TcpPortEna) {
        if (!stropen(OutStr, STR_TCPSVR, STR_MODE_RW, qPrintable(QString::number(TcpPortNo)))) {
            Message3->setText(tr("tcp port open error"));
        }
    }
    if (OutFileEna && OutFile != "") {
        QString outfile = OutFile + (OutTimeTag ? "::T" : "") + swap[FileSwap];
        if (!stropen(OutStr + 1, STR_FILE, STR_MODE_W, qPrintable(outfile))) {
            Message3->setText (tr("file open error"));
        }
    }

    cameraFrameGrabber = new CameraFrameGrabber();
    camera->setViewfinder(cameraFrameGrabber);

    connect(cameraFrameGrabber, SIGNAL(frameAvailable(QImage)), this, SLOT(SampleBufferSync(QImage)));

    StartTime = utc2gpst(timeget());
    camera->start();
    return 1;
}
//---------------------------------------------------------------------------
void MainForm::CaptureStop(void)
{    
    camera->stop();
    
    for (int i = 0; i < 2; i++) {
        strclose(OutStr + i);
    }
}
//---------------------------------------------------------------------------
void MainForm::SampleBufferSync(QImage videoFrame)
{
    static int Timer_Count = 0;

    QColor color[] = {
        Qt::red, QColor(0xFF,0x40,0x40), QColor(0xFF,0xFF,0xA5), Qt::green, QColor(0xff,0x00,0xff)
    };
    QString msg;
    char str_msg[4096],*p,*q;
    int outb = 0, outr = 0;

    QString str;
        
    CaptureTime = utc2gpst(timeget());
    FRM++;

    if (CapSizeEna) {
        videoFrame=videoFrame.scaled(CapWidth, CapHeight);
     }
    QPainter c(&videoFrame);
    c.setPen(Qt::yellow);

    if (TcpPortEna || OutFileEna) {
        
        if (Annotation) {
            str=QString("%1 FRM=%2").arg(time_str(CaptureTime,1)).arg(FRM);
            c.drawText(10, 10, str);
        }
        QBuffer buff;

        videoFrame.save(&buff, "JPEG");
        
        
        for (int i = 0; i < 2; i++) {
            strwrite(OutStr + i, (unsigned char *)buff.data().constData(), buff.data().size());
        }
    }
        
    c.setPen(color[strstat(OutStr, str_msg)+1]);
    strsum(OutStr, NULL, NULL, NULL, &outr);
    msg = QString("%1Mbps %2").arg(outr / 1e6, 0, 'f', 1).arg(str_msg);
    QRect rect = videoFrame.rect();
    rect.setHeight(rect.height() - c.boundingRect(videoFrame.rect(), "T").height() - 5); // decrease size by one line (+spacing)
    c.drawText(rect, Qt::AlignBottom|Qt::AlignLeft, msg);

    c.setPen(color[strstat(OutStr + 1, str_msg) + 1]);
    strsum(OutStr + 1, NULL, NULL, &outb, NULL);
    msg = QString("%1MB").arg(outb / 1e6, 0, 'f', 1);
    strstatx(OutStr + 1, str_msg);
    if ((p = strstr(str_msg, "openpath=")) && (q = strchr(p + 9, '\n'))) {
        *q = '\0';
        msg += p + 9;
    }
    c.drawText(videoFrame.rect(), Qt::AlignBottom|Qt::AlignLeft, msg);

    c.setPen(Qt::yellow);
    if (++Timer_Count * Time.elapsed() >= 1000) {
        FPS = FRM - FRM0;
        FRM0 = FRM;
        Timer_Count = 0;
        Time.restart();
    }
    msg = QString("%1 x %2 FRM=%3 FPS=%4").arg(Video_Width).arg(Video_Height).arg(FRM).arg(FPS, 2);
    c.drawText(videoFrame.rect(), Qt::AlignBottom|Qt::AlignRight, msg);
    
    time2str(utc2gpst(timeget()), str_msg, 1);
    c.drawText(videoFrame.rect(), Qt::AlignTop|Qt::AlignLeft, str_msg);
    
    msg = QString("TIME = %1 s").arg(!StartTime.time ? 0.0 : timediff(CaptureTime, StartTime), 0, 'f', 1);
    c.drawText(videoFrame.rect(), Qt::AlignTop|Qt::AlignRight, msg);

    // show image in monitor
    Disp->setPixmap(QPixmap::fromImage(videoFrame));
}
//---------------------------------------------------------------------------
void MainForm::LoadOptions(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    CapSizeEna = settings.value("videoopt/capsizeena", 0).toInt();
    CapWidth = settings.value("videoopt/capwidth", 500).toInt();
    CapHeight = settings.value("videoopt/capheight", 500).toInt();
    TcpPortEna = settings.value("videoopt/tcpportena", 0).toInt();
    TcpPortNo = settings.value("videoopt/tcpportno", 10033).toInt();
    OutFileEna = settings.value("videoopt/outfileena", 0).toInt();
    OutFile = settings.value("videoopt/outfile", "").toString();
    OutTimeTag = settings.value("videoopt/outtimetag", 0).toInt();
    FileSwap = settings.value("videoopt/fileswap", 0).toInt();
}
//---------------------------------------------------------------------------
void MainForm::SaveOptions(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    settings.setValue("videoopt/capsizeena", CapSizeEna);
    settings.setValue("videoopt/capwidth", CapWidth);
    settings.setValue("videoopt/capheight", CapHeight);
    settings.setValue("videoopt/tcpportena", TcpPortEna);
    settings.setValue("videoopt/tcpportno", TcpPortNo);
    settings.setValue("videoopt/outfileena", OutFileEna);
    settings.setValue("videoopt/outfile", OutFile);
    settings.setValue("videoopt/outtimetag", OutTimeTag);
    settings.setValue("videoopt/fileswap", FileSwap);
}
//---------------------------------------------------------------------------
void MainForm::cameraError(QCamera::Error )
{
    QMessageBox::warning(this, "Error", camera->errorString());
}
