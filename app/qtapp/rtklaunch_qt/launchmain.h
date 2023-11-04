//---------------------------------------------------------------------------
#ifndef launchmainH
#define launchmainH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QSystemTrayIcon>
#include <QMenu>

#include "ui_launchmain.h"

class QCloseEvent;
class QCloseEvent;
class LaunchOptDialog;

//---------------------------------------------------------------------------
class MainForm : public QDialog, private Ui::MainForm
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);

public slots:
    void btnPlotClicked();
    void btnConvClicked();
    void btnStrClicked();
    void btnPostClicked();
    void btnNtripClicked();
    void btnNaviClicked();
    void btnGetClicked();
    void btnVideoClicked();
    void btnTrayClicked();
    void btnOptionClicked();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void menuExpandClicked();

private:
    QString iniFile;
    QSystemTrayIcon trayIcon;
    QMenu *trayMenu;
    LaunchOptDialog *launchOptDlg;
    bool tray;
	
    int execCmd(const QString &cmd, const QStringList &opt);
    void updatePanel();

public:
    int option, minimize;
    explicit MainForm(QWidget *parent = 0);
};
//---------------------------------------------------------------------------
#endif
