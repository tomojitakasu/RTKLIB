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
    void btnExitClicked();
    void btnInputClicked();
    void btnOutputClicked();
    void btnStartClicked();
    void btnStopClicked();
    void serverStatusTimerTimeout();
    void btnOptionsClicked();
    void outputChanged();
    void inputChanged();
    void BtnCommandClicked();
    void btnAboutClicked();
    void btnStreamMonitorClicked();
    void streamMonitorTimerTimeout();
    void btnTaskIconClicked();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void menuExpandClicked();
    void menuStartClicked();
    void menuStopClicked();
    void menuExitClicked();
    void formCreated();
    void btnConvertClicked();
    void btnLogClicked();

protected:
    void closeEvent(QCloseEvent*);

private:
    QString iniFile;
    QString paths[MAXSTR][4], commands[MAXSTR][3], commandsTcp[MAXSTR][3];
    QString tcpHistory[MAXHIST], tcpMountpointHistory[MAXHIST];
    QString stationPositionFile, exeDirectory, localDirectory, swapInterval;
    QString proxyAddress, logFile;
    QString conversionMessage[MAXSTR - 1], conversionOptions[MAXSTR - 1], antennaType, receiverType;
    QString pathLog[MAXSTR];
    int conversionEnabled[MAXSTR - 1], ConversionInput[MAXSTR - 1], ConversionOutput[3], stationId, stationSelect;
    int traceLevel, serverOptions[6], commandsEnabled[MAXSTR][3], commandsEnabledTcp[MAXSTR][3], nmeaRequest, fileSwapMargin, relayBack, progressBarRange, pathEnabled[MAXSTR];
    double antennaPosition[3], antennaOffsets[3];
    gtime_t startTime, endTime;
    QSystemTrayIcon *trayIcon;
    SvrOptDialog *svrOptDialog;
    TcpOptDialog *tcpOptDialog;
    SerialOptDialog *serialOptDialog;
    FileOptDialog *fileOptDialog;
    FtpOptDialog * ftpOptDialog;
    StrMonDialog * strMonDialog;
    QTimer serverStatusTimer, streamMonitorTimer;

    void serialOptions(int index, int path);
    void tcpClientOptions(int index, int path);
    void tcpServerOptions(int index, int path);
    void ntripServerOptions(int index, int path);
    void ntripClientOptions(int index, int path);
    void ntripCasterOptions(int index, int path);
    void udpClientOptions(int index, int path);
    void udpServerOptions(int index, int path);
    void fileOptions(int index, int path);
    void showMessage(const QString &msg);
    void serverStart(void);
    void serverStop(void);
    void updateEnable(void);
    void setTrayIcon(int index);
    void loadOptions(void);
    void saveOptions(void);

public:
    explicit MainForm(QWidget *parent=0);
};
//---------------------------------------------------------------------------
#endif
