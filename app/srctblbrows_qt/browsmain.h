//---------------------------------------------------------------------------
#ifndef browsmainH
#define browsmainH
//---------------------------------------------------------------------------
#include <QMainWindow>

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
    void TypeChange();
    void BtnListClick();
    void AddressChange();
    void AddressKeyPress(QString);
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
    void TypeStrClick();
    void TypeCasClick();
    void TypeNetClick();
    void TypeSrcClick();
    void BtnMapClick();
    void TimerTimer();
    void Table0SelectCell(int ACol, int ARow);
    void BtnStaClick();
    void StaMaskClick();

private:
    QString AddrList,AddrCaster,SrcTable,IniFile;
	int FontScale;
    GoogleMapView *googleMapView;
    StaListDialog *staListDialog;
    QTimer *Timer;

    void UpdateCaster(void);
    void UpdateTable(void);
    void UpdateMap(void);
    void UpdateEnable(void);
    void ShowTable(void);
public:
    QStringList StaList;

    void ShowMsg(const QString &);
    MainForm(QWidget *parent=NULL);
};
//---------------------------------------------------------------------------
#endif
