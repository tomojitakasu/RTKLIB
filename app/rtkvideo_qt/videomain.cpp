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
    Video_Width = Video_Height = FrameCount = FrameCount0 = FrameRate = 0;
    DevName = "";
    OutFile = "";
    Profile = 0;
    TcpPortEna = OutFileEna = OutTimeTag = FileSwap = 0;
    TcpPortNo = DEFAULT_TCP_PORT;

    setWindowTitle(QString("%1 ver. %2 %3").arg(PRGNAME).arg(VER_RTKLIB).arg(PATCH_LEVEL));
    videoOptDlg = new VideoOptDlg(this);

    connect(BtnStart, SIGNAL(clicked(bool)), this, SLOT(BtnStartClick()));
    connect(BtnStop, SIGNAL(clicked(bool)), this, SLOT(BtnStopClick()));
    connect(BtnExit, SIGNAL(clicked(bool)), this, SLOT(BtnExitClick()));
    connect(BtnOpt, SIGNAL(clicked(bool)), this, SLOT(BtnOptClick()));

    BtnStop->setEnabled(false);
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
        strinit(OutputStream+i);
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

    camera->setViewfinderSettings(videoSettings);
    camera->setCaptureMode(QCamera::CaptureVideo);

    if (TcpPortEna) {
        if (!stropen(OutputStream, STR_TCPSVR, STR_MODE_RW, qPrintable(QString::number(TcpPortNo)))) {
            Message3->setText(tr("tcp port open error"));
        }
    }
    if (OutFileEna && OutFile != "") {
        QString outfile = OutFile + (OutTimeTag ? "::T" : "") + swap[FileSwap];
        if (!stropen(OutputStream + 1, STR_FILE, STR_MODE_W, qPrintable(outfile))) {
            Message3->setText (tr("file open error"));
        }
    }

    cameraFrameGrabber = new CameraFrameGrabber();
    camera->setViewfinder(cameraFrameGrabber);

    connect(cameraFrameGrabber, SIGNAL(frameAvailable(QImage)), this, SLOT(SampleBufferSync(QImage)));

    StartTime = utc2gpst(timeget());
    StartTick = tickget();

    camera->start();
    return 1;
}
//---------------------------------------------------------------------------
void MainForm::CaptureStop(void)
{    
    camera->stop();
    
    for (int i = 0; i < 2; i++) {
        strclose(OutputStream + i);
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
    CaptureTick = tickget();
    FrameCount++;

    if (TcpPortEna || OutFileEna) {
        QImage sentFrame=videoFrame;

        QPainter c(&sentFrame);

        if (CaptionPos)
        {
            static const int aligns[] = {
                    Qt::AlignLeading, Qt::AlignCenter, Qt::AlignTrailing
            };

            str=QString("%1 T=%2 s TICK=%3 FRM=%4").arg(time_str(CaptureTime,3)).arg((CaptureTick-StartTick)*1e-3,0,'f',3).arg(CaptureTick).arg(FrameCount);

            c.setPen(CaptionColor);
            c.setFont(QFont(qApp->font().family(),CaptionSize));
            c.drawText(videoFrame.rect(), aligns[CaptionPos-1]|Qt::AlignTop, str_msg);
        }

        QBuffer buff;

        sentFrame.save(&buff, "JPEG");
        
        
        for (int i = 0; i < 2; i++) {
            strwrite(OutputStream + i, (unsigned char *)buff.data().constData(), buff.data().size());
        }
    }
    QPainter c(&videoFrame);

    c.setPen(color[strstat(OutputStream, str_msg)+1]);
    strsum(OutputStream, NULL, NULL, NULL, &outr);
    msg = QString("%1Mbps %2").arg(outr / 1e6, 4, 'f', 1).arg(str_msg);
    QRect rect = videoFrame.rect();
    rect.setHeight(rect.height() - c.boundingRect(videoFrame.rect(), "T").height() - 5); // decrease size by one line (+spacing)
    c.drawText(rect, Qt::AlignBottom|Qt::AlignLeft, msg);

    c.setPen(color[strstat(OutputStream + 1, str_msg) + 1]);
    strsum(OutputStream + 1, NULL, NULL, &outb, NULL);
    msg = QString("%1MB").arg(outb / 1e6, 4, 'f', 1);
    strstatx(OutputStream + 1, str_msg);
    if ((p = strstr(str_msg, "openpath=")) && (q = strchr(p + 9, '\n'))) {
        *q = '\0';
        msg += p + 9;
    }
    c.drawText(videoFrame.rect(), Qt::AlignBottom|Qt::AlignLeft, msg);

    c.setPen(CaptionColor);
    if (++Timer_Count * Time.elapsed() >= 1000) {
        FrameRate = FrameCount - FrameCount0;
        FrameCount0 = FrameCount;
        Timer_Count = 0;
        Time.restart();
    }
    msg = QString("%1 x %2 FRM=%3 FPS=%4").arg(videoFrame.width()).arg(videoFrame.height()).arg(FrameCount).arg(FrameRate, 2);
    c.drawText(videoFrame.rect(), Qt::AlignBottom|Qt::AlignRight, msg);
    
    time2str(utc2gpst(timeget()), str_msg, 1);
    c.drawText(videoFrame.rect(), Qt::AlignTop|Qt::AlignLeft, str_msg);
    
    msg = QString("T = %1 s TICK=%2").arg((CaptureTick - StartTick) * 1e-3, 0, 'f', 1).arg(CaptureTick);
    c.drawText(videoFrame.rect(), Qt::AlignTop|Qt::AlignRight, msg);

    // show image in monitor
    Disp->setPixmap(QPixmap::fromImage(videoFrame));
}
//---------------------------------------------------------------------------
void MainForm::LoadOptions(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    CaptionSize = settings.value("videoopt/captionsize", 24).toInt();
    CaptionColor = settings.value("videoopt/captioncolor", QColor(Qt::red)).value<QColor>();
    TcpPortEna = settings.value("videoopt/tcpportena", 0).toInt();
    TcpPortNo = settings.value("videoopt/tcpportno", 10033).toInt();
    OutFileEna = settings.value("videoopt/outfileena", 0).toInt();
    OutFile = settings.value("videoopt/outfile", "").toString();
    OutTimeTag = settings.value("videoopt/outtimetag", 0).toInt();
    FileSwap = settings.value("videoopt/fileswap", 0).toInt();
    CaptionPos = settings.value("videoopt/captionpos", 2).toInt();
    CodecQuality = settings.value("videoopt/codecquality", 90).toInt();
}
//---------------------------------------------------------------------------
void MainForm::SaveOptions(void)
{
    QSettings settings(IniFile, QSettings::IniFormat);

    settings.setValue("videoopt/capsize", CaptionSize);
    settings.setValue("videoopt/captioncolor", CaptionColor);
    settings.setValue("videoopt/tcpportena", TcpPortEna);
    settings.setValue("videoopt/tcpportno", TcpPortNo);
    settings.setValue("videoopt/outfileena", OutFileEna);
    settings.setValue("videoopt/outfile", OutFile);
    settings.setValue("videoopt/outtimetag", OutTimeTag);
    settings.setValue("videoopt/fileswap", FileSwap);
    settings.setValue("videoopt/captionpos", CaptionPos);
    settings.setValue("videoopt/codecquality", CodecQuality);

}
//---------------------------------------------------------------------------
void MainForm::cameraError(QCamera::Error )
{
    QMessageBox::warning(this, "Error", camera->errorString());
}
