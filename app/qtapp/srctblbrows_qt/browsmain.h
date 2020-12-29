//---------------------------------------------------------------------------
#ifndef browsmainH
#define browsmainH
//---------------------------------------------------------------------------
#include <QMainWindow>
#include<QFutureWatcher>

#include "ui_browsmain.h"


class QShowEvent;
class QCloseEvent;
class StaListDialog;
class GoogleMapView;
class QTimer;

//---------------------------------------------------------------------------
class MainForm : public QMainWindow, private Ui::MainForm
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);

public slots:
    void BtnUpdateClick();
    void BtnListClick();
    void AddressChange();
    void MenuOpenClick();
    void MenuSaveClick();
    void MenuQuitClick();
    void MenuUpdateCasterClick();
    void MenuUpdateTableClick();
    void MenuViewStrClick();
    void MenuViewCasClick();
    void MenuViewNetClick();
    void MenuViewSrcClick();
    void MenuAboutClick();
    void BtnMapClick();
    void TimerTimer();
    void Table0SelectCell(int ACol, int ARow);
    void BtnStaClick();
    void StaMaskClick();
    void UpdateCaster();
    void UpdateTable();
    void ShowMsg(const QString &);

private:
    QString AddrList,AddrCaster,SrcTable,IniFile;
	int FontScale;
    GoogleMapView *googleMapView;
    StaListDialog *staListDialog;
    QTimer *Timer;
    QFutureWatcher<char*> TableWatcher;
    QFutureWatcher<char*> CasterWatcher;

    void GetCaster(void);
    void GetTable(void);
    void UpdateMap(void);
    void UpdateEnable(void);
    void ShowTable(void);
public:
    QStringList StaList;

    explicit MainForm(QWidget *parent=NULL);
};
//---------------------------------------------------------------------------
#endif
