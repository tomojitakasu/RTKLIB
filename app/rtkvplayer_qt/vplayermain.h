//---------------------------------------------------------------------------

#ifndef vplayerH
#define vplayerH
//---------------------------------------------------------------------------

#include <QDialog>
#include <QMediaPlayer>
#include <QTimer>

#include "ui_vplayermain.h"

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
    void Timer1Timer();
    void BtnPosStartClick();

protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);

    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
    QMediaPlayer *MjpgPlayer;
    QString FileName;
    int VideoType, Track;
    QString IniFile;
    QTimer Timer1;
    void OpenVideo(QString file);
    void ClearVideo(void);
    int  PlayVideo(void);
    void StopVideo(void);
    void SetVideoPos(float pos);
    float GetVideoPos(void);
    void GetVideoTime(double &time, double &period);
    void GetVideoSize(int &width, int &height);
    void LoadOptions(void);
    void SaveOptions(void);
    
public:
    MainForm(QWidget *parent = 0);
};
//---------------------------------------------------------------------------
#endif
