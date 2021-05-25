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
class LaunchOptDlg;

//---------------------------------------------------------------------------
class MainForm : public QDialog, private Ui::MainForm
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);

public slots:
    void BtnPlotClick();
    void BtnConvClick();
    void BtnStrClick();
    void BtnPostClick();
    void BtnNtripClick();
    void BtnNaviClick();
    void BtnGetClick();
    void BtnVideoClick();
    void BtnTrayClick();
    void BtnOptionClick();
    void TrayIconActivated(QSystemTrayIcon::ActivationReason);
    void MenuExpandClick();

private:
    QString IniFile;
    QSystemTrayIcon TrayIcon;
    QMenu *trayMenu;
    LaunchOptDlg *launchOptDlg;
    int Tray;
	
    int ExecCmd(const QString &cmd, const QStringList &opt);
    void UpdatePanel();

public:
    int Option, Minimize;
    explicit MainForm(QWidget *parent = 0);
};
//---------------------------------------------------------------------------
#endif
