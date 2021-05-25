//---------------------------------------------------------------------------
#ifndef svrmainH
#define svrmainH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QTimer>
#include <QSystemTrayIcon>

#include "ui_svrmain.h"

#include "rtklib.h"
#include "tcpoptdlg.h"

class QCloseEvent;
class SvrOptDialog;
class SerialOptDialog;
class TcpOptDialog;
class FileOptDialog;
class FtpOptDialog;
class StrMonDialog;

#define MAXSTR        7    // number of streams

//---------------------------------------------------------------------------
class MainForm : public QDialog, private Ui::MainForm
{
    Q_OBJECT

public slots:
    void BtnExitClick();
    void BtnInputClick();
    void BtnOutputClick();
    void BtnStartClick();
    void BtnStopClick();
    void Timer1Timer();
    void BtnOptClick();
    void OutputChange();
    void InputChange();
    void BtnCmdClick();
    void BtnAboutClick();
    void BtnStrMonClick();
    void Timer2Timer();
    void BtnTaskIconClick();
    void MenuExpandClick();
    void TrayIconActivated(QSystemTrayIcon::ActivationReason);
    void MenuStartClick();
    void MenuStopClick();
    void MenuExitClick();
    void FormCreate();
    void BtnConvClick();
    void BtnLogClick();

protected:
    void closeEvent(QCloseEvent*);

private:
    QString IniFile;
    QString Paths[MAXSTR][4], Cmds[MAXSTR][3], CmdsTcp[MAXSTR][3];
    QString TcpHistory[MAXHIST], TcpMntpHist[MAXHIST];
    QString StaPosFile, ExeDirectory, LocalDirectory, SwapInterval;
    QString ProxyAddress,LogFile;
    QString ConvMsg[MAXSTR - 1], ConvOpt[MAXSTR - 1], AntType, RcvType;
    QString PathLog[MAXSTR];
    int ConvEna[MAXSTR - 1], ConvInp[MAXSTR - 1], ConvOut[3], StaId, StaSel;
    int TraceLevel, SvrOpt[6], CmdEna[MAXSTR][3], CmdEnaTcp[MAXSTR][3], NmeaReq, FileSwapMargin, RelayBack, ProgBarRange, PathEna[MAXSTR];
    double AntPos[3], AntOff[3];
    gtime_t StartTime, EndTime;
    QSystemTrayIcon *TrayIcon;
    SvrOptDialog *svrOptDialog;
    TcpOptDialog *tcpOptDialog;
    SerialOptDialog *serialOptDialog;
    FileOptDialog *fileOptDialog;
    FtpOptDialog * ftpOptDialog;
    StrMonDialog * strMonDialog;
    QTimer Timer1,Timer2;

    void SerialOpt(int index, int path);
    void TcpCliOpt(int index, int path);
    void TcpSvrOpt(int index, int path);
    void NtripSvrOpt(int index, int path);
    void NtripCliOpt(int index, int path);
    void NtripCasOpt(int index, int path);
    void UdpCliOpt(int index, int path);
    void UdpSvrOpt(int index, int path);
    void FileOpt(int index, int path);
    void ShowMsg(const QString &msg);
    void SvrStart(void);
    void SvrStop(void);
    void UpdateEnable(void);
    void SetTrayIcon(int index);
    void LoadOpt(void);
    void SaveOpt(void);

public:
    explicit MainForm(QWidget *parent=0);
};
//---------------------------------------------------------------------------
#endif
