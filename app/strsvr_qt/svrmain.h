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
class Console;
class SerialOptDialog;
class TcpOptDialog;
class FileOptDialog;
class FtpOptDialog;

//---------------------------------------------------------------------------
class MainForm : public QDialog, private Ui::MainForm
{
    Q_OBJECT
public slots:
    void BtnExitClick();
    void BtnInputClick();
    void BtnOutput1Click();
    void BtnOutput2Click();
    void BtnStartClick();
    void BtnStopClick();
    void Timer1Timer();
    void BtnOptClick();
    void Output1Change();
    void Output2Change();
    void InputChange();
    void BtnOutput3Click();
    void Output3Change();
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
    void BtnConv1Click();
    void BtnConv2Click();
    void BtnConv3Click();
protected:
    void closeEvent(QCloseEvent*);

private:
    QString IniFile;
    QString Paths[4][4],Cmds[2],CmdsTcp[2];
    QString TcpHistory[MAXHIST],TcpMntpHist[MAXHIST];
    QString StaPosFile,ExeDirectory,LocalDirectory,SwapInterval;
    QString ProxyAddress;
    QString ConvMsg[3],ConvOpt[3],AntType,RcvType;
	int ConvEna[3],ConvInp[3],ConvOut[3],StaId,StaSel;
	int TraceLevel,SvrOpt[6],CmdEna[2],CmdEnaTcp[2],NmeaReq,FileSwapMargin;
	double AntPos[3],AntOff[3];
	gtime_t StartTime,EndTime;
    QSystemTrayIcon *TrayIcon;
    SvrOptDialog *svrOptDialog;
    Console *console;
    TcpOptDialog *tcpOptDialog;
    SerialOptDialog *serialOptDialog;
    FileOptDialog *fileOptDialog;
    FtpOptDialog * ftpOptDialog;
    QTimer Timer1,Timer2;

    void SerialOpt(int index, int opt);
    void TcpOpt(int index, int opt);
    void FileOpt(int index, int opt);
    void FtpOpt(int index, int opt);
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
