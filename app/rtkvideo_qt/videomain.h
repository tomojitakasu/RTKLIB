//---------------------------------------------------------------------------
#ifndef vmainH
#define vmainH

#include <QDialog>
#include <QCamera>
#include <QVideoProbe>
#include <QMediaRecorder>
#include <QVideoFrame>
#include <QTime>

#include "ui_videomain.h"
#include "videoopt.h"

#include "rtklib.h"

class QShowEvent;
class QCloseEvent;
class CameraFrameGrabber;

//---------------------------------------------------------------------------
class MainForm : public QDialog, protected Ui::MainForm
{
    Q_OBJECT
public slots:
    void BtnStopClick();
    void BtnOptClick();
    void BtnExitClick();
    void BtnStartClick();
    void cameraError(QCamera::Error value);
    void SampleBufferSync(QImage);
protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
private:
    QCamera *camera;
    CameraFrameGrabber *cameraFrameGrabber;
    stream_t OutStr[2];
    gtime_t StartTime;
    gtime_t CaptureTime;
    lock_t DeviceLock;
    int Video_Width, Video_Height, FRM, FRM0, FPS;
    QString IniFile;
    QTime Time;

    int  CaptureStart(void);
    void CaptureStop(void);
    void LoadOptions(void);
    void SaveOptions(void);
    void DrawText(QPainter *c, int x, int y,
        QString str, int size, QColor color, int ha, int va);
public:
    QString DevName, OutFile;
    VideoOptDlg *videoOptDlg;
    int Profile, CapSizeEna, CapWidth, CapHeight, Annotation;
    int TcpPortEna, TcpPortNo, OutFileEna, OutTimeTag, FileSwap;
    
    MainForm(QWidget *parent = 0);
};
//---------------------------------------------------------------------------
#endif
