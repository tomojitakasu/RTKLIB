//---------------------------------------------------------------------------
#ifndef getmainH
#define getmainH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QSettings>
#include <QTimer>
#include <QPixmap>
#include <QSystemTrayIcon>

#include "rtklib.h"
#include "ui_getmain.h"

class TextViewer;
class DownloadThread;
class TimeDialog;
//---------------------------------------------------------------------------
class MainForm : public QWidget, public Ui::MainForm
{
     Q_OBJECT

protected:
    void  closeEvent(QCloseEvent *);

    void  FormCreate();

    void  dragEnterEvent(QDragEnterEvent *event);
    void  dropEvent(QDropEvent * event);

public slots:
    void  BtnExitClick();
    void  BtnOptsClick();
    void  BtnLogClick();
    void  BtnDownloadClick();
    void  DataTypeChange();
    void  BtnFileClick();
    void  DataListClick();
    void  BtnDirClick();
    void  LocalDirClick();
    void  BtnStasClick();
    void  BtnKeywordClick();
    void  BtnHelpClick();
    void  HidePasswdClick();
    void  TimerTimer();
    void  BtnTrayClick();
    void  TrayIconActivated(QSystemTrayIcon::ActivationReason);
    void  BtnTestClick();
    void  StaListClick();
    void  BtnAllClick();
    void  DirChange();
    void  DownloadFinished();
    void  BtnTime1Click();
    void  BtnTime2Click();

private:
    QStringList Types;
    QStringList Urls;
    QStringList Locals;
    QPixmap Images[8];
    QSystemTrayIcon TrayIcon;
    DownloadThread *thread;
    TextViewer *viewer;
    TimeDialog *timeDialog;

    void  LoadOpt(void);
    void  SaveOpt(void);
    void  UpdateType(void);
    void  UpdateMsg(void);
    void  UpdateStaList(void);
    void  UpdateEnable(void);
    void  PanelEnable(int ena);
    void  GetTime(gtime_t *ts, gtime_t *te, double *ti);
    int   SelectUrl(url_t *urls);
    int   SelectSta(char **stas);
    void  LoadUrl(QString file);
    void  LoadSta(QString file);
    int   ExecCmd(QString cmd);
    void  ReadHist(QSettings &, QString key, QComboBox *);
    void  WriteHist(QSettings &, QString key, QComboBox *);
    void  AddHist(QComboBox *combo);
	
public:
    QString IniFile;
    QString UrlFile;
    QString LogFile;
    QString Stations;
    QString ProxyAddr;
	int HoldErr;
	int HoldList;
	int NCol;
	int DateFormat;
	int TraceLevel;
	int LogAppend;
	int TimerCnt;
    QTimer Timer;

    explicit MainForm(QWidget* parent);
};
//---------------------------------------------------------------------------
#endif
