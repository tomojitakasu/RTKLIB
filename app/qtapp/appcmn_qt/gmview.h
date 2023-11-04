//---------------------------------------------------------------------------
#ifndef gmviewH
#define gmviewH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QTimer>

#include "ui_gmview.h"

class QWebEngineView;
class GMPageState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER text NOTIFY textChanged)
public:
    explicit GMPageState(QObject *parent = NULL): QObject(parent){}
    QString getText() {return text;}
signals:
    void textChanged(const QString &text);
private:
    QString text;
};

class QResizeEvent;
class QShowEvent;
//---------------------------------------------------------------------------
class GoogleMapView : public QDialog, private Ui::GoogleMapView
{
    Q_OBJECT

public slots:
    void loadTimerExpired();
    void btnShrinkClick();
    void btnExpandClick();
    void btnFixCentClick();
    void btnCloseClick();
    void pageLoaded(bool);

protected:
    void resizeEvent(QResizeEvent*);

private:
    int state;
    double latitude, longitude;
    int zoom;
    double markPosition[2][2];
    QTimer loadTimer;
    bool loaded;
    QWebEngineView *webBrowser;
    GMPageState *pageState;
    void ExecFunc(const QString &func);

public:
    bool fixCenter;

    explicit GoogleMapView(QWidget *parent = NULL);

    int setApiKey(QString key);
    int  getState(void);
    void setView(double latitude, double longitude, int zoom);
    void setCent(double latitude, double longitude);
    void setZoom(int zoom);
    void clearMark(void);
    void addMark(double latitude, double longitude, const QString &title, const QString &msg);
    void setMark(int index, const double *pos);
    void showMark(int index);
    void hideMark(int index);
    void highlightMark(const QString &title);
};
//---------------------------------------------------------------------------
#endif
