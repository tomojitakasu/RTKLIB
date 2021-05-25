//---------------------------------------------------------------------------
#ifndef gmviewH
#define gmviewH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QTimer>

#include "ui_mapview.h"

#ifdef QWEBKIT
class QWebView;
#endif
#ifdef QWEBENGINE
class QWebEngineView;
class MapViewPageState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER text NOTIFY textChanged)
public:
    explicit MapViewPageState(QObject *parent = NULL): QObject(parent){}
    QString getText() {return text;}
signals:
    void textChanged(const QString &text);
private:
    QString text;
};
#endif

class QResizeEvent;
class QShowEvent;
class MapViewOptDialog;
//---------------------------------------------------------------------------
class MapView : public QDialog, private Ui::MapView
{
    Q_OBJECT

public slots:
    void BtnCloseClick();
    void Timer1Timer();
    void BtnZoomOutClick();
    void BtnZoomInClick();
    void BtnSyncClick();
    void PageLoaded(bool);

    void BtnOptClick();
    void MapSel1Click();
    void MapSel2Click();
    void Timer2Timer();

protected:
    void resizeEvent(QResizeEvent*);
     void showEvent(QShowEvent*);

private:
#ifdef QWEBKIT
    QWebView *WebBrowser;
#endif
#ifdef QWEBENGINE
    QWebEngineView *WebBrowser;
    MapViewPageState *pageState;
#endif
    int MarkState[2];
    double Lat,Lon;
    double MarkPos[2][2];
    QTimer Timer1, Timer2;
    bool loaded;

    MapViewOptDialog *mapViewOptDialog;

    void ShowMapLL(void);
    void ShowMapGM(void);
    void ShowMap(int map);
    void SetView(int map, double lat, double lon, int zoom);
    void AddMark(int map, int index, double lat, double lon, int state);
    void UpdateMap(void);
    void SelectMap(int map);
    int  GetState(int map);
    void ExecFunc(int map, const QString &func);

public:
    int MapSel;

    explicit MapView(QWidget *parent = NULL);
    void SetCent(double lat, double lon);
    void SetMark(int index, double lat, double lon);
    void ShowMark(int index);
    void HideMark(int index);
};
//---------------------------------------------------------------------------
#endif
