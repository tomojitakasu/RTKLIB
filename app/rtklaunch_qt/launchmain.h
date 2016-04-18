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
    void BtnTrayClick();
    void TrayIconActivated(QSystemTrayIcon::ActivationReason);
    void MenuExpandClick();

private:
    QString IniFile;
    QSystemTrayIcon TrayIcon;
    QMenu *trayMenu;
    int Tray;
	
    int ExecCmd(const QString &cmd);
public:
    explicit MainForm(QWidget *parent=0);
};
//---------------------------------------------------------------------------
#endif
