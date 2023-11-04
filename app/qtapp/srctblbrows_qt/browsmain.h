//---------------------------------------------------------------------------
#ifndef browsmainH
#define browsmainH
//---------------------------------------------------------------------------
#include <QMainWindow>
#include <QFutureWatcher>

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
    void btnUpdateClicked();
    void btnListClicked();
    void addressChanged();
    void menuOpenClicked();
    void menuSaveClicked();
    void menuQuitClicked();
    void menuUpdateCasterClicked();
    void menuUpdateTableClicked();
    void menuViewStrClicked();
    void menuViewCasterClicked();
    void menuViewNetClicked();
    void menuViewSourceClicked();
    void menuAboutClicked();
    void btnMapClicked();
    void loadTimerExpired();
    void Table0SelectCell(int ARow, int ACol);
    void btnStatsionClicked();
    void stationMaskClicked();
    void updateCaster();
    void updateTable();
    void showMsg(const QString &);

private:
    QString addressList, addressCaster, sourceTable, iniFile;
    float fontScale;
#ifdef QWEBENGINE
    GoogleMapView *mapView;
#endif
    StaListDialog *staListDialog;
    QTimer *loadTimer;
    QFutureWatcher<char*> tableWatcher;
    QFutureWatcher<char*> casterWatcher;

    void getCaster(void);
    void getTable(void);
    void updateMap(void);
    void updateEnable(void);
    void showTable(void);
public:
    QStringList stationList;

    explicit MainForm(QWidget *parent = NULL);
};
//---------------------------------------------------------------------------
#endif
