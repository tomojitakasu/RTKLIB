//---------------------------------------------------------------------------

#ifndef vplayerH
#define vplayerH
//---------------------------------------------------------------------------

#include <QDialog>
#include <QMediaPlayer>
#include <QTimer>

#include "ui_vplayermain.h"
#include "rtklib.h"

class QShowEvent;
class QCloseEvent;

//---------------------------------------------------------------------------
class MainForm : public QDialog, protected Ui::MainForm
{
    Q_OBJECT
public slots:
    void BtnOpenClick();
    void BtnPlayClick();
    void BtnStopClick();
    void BtnClearClick();
    void BtnExitClick();
    void BtnNextClick();
    void BtnPrevClick();
    void Timer1Timer();
    void BtnSyncClick();
    void BtnPosStartClick();
    void BtnOptionsClick();

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);

    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
    gtime_t TimeStart;
    stream_t StrTimeSync;
    QMediaPlayer *MjpgPlayer;
    QStringList Files;
    int VideoType, Track, NStrBuff, FileIndex;
    char StrBuff[1024];
    QString FileName;
    QString IniFile;
    QTimer Timer1;
    void OpenVideo(const QStringList &file);
    void ClearVideo(void);
    int  PlayVideo(void);
    void StopVideo(void);
    void SetVideoPos(float pos);
    float GetVideoPos(void);
    void GetVideoTime(double &time, double &period);
    void GetVideoSize(int &width, int &height);
    void LoadOptions(void);
    void SaveOptions(void);
    void NextVideo();
    void PrevVideo();
    void BtnOptionClick();
    void ReadNmea(QString file);
public:
    MainForm(QWidget *parent = 0);

    double MjpgRate;
    QString SyncAddr;
    int SyncPort;
};
//---------------------------------------------------------------------------
#endif
