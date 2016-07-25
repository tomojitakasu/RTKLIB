//---------------------------------------------------------------------------
#ifndef gmviewH
#define gmviewH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QTimer>

#include "ui_gmview.h"

#ifdef QWEBKIT
class QWebView;
#endif
#ifdef QWEBENGINE
class QWebEngineView;
class GMPageState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER text NOTIFY textChanged)
public:
    explicit GMPageState(QObject *parent=NULL): QObject(parent){}
    QString getText() {return text;}
signals:
    void textChanged(const QString &text);
private:
    QString text;
};
#endif
class QResizeEvent;
class QShowEvent;
//---------------------------------------------------------------------------
class GoogleMapView : public QDialog, private Ui::GoogleMapView
{
    Q_OBJECT

public slots:
    void FormCreate();
    void Timer1Timer();
    void BtnShrinkClick();
    void BtnExpandClick();
    void BtnFixCentClick();
    void BtnCloseClick();
    void PageLoaded(bool);

protected:
    void resizeEvent(QResizeEvent*);

private:
	int State;
	double Lat,Lon,Zoom;
	double MarkPos[2][2];
    QTimer Timer1;
    bool loaded;
#ifdef QWEBKIT
    QWebView *WebBrowser;
#endif
#ifdef QWEBENGINE
    QWebEngineView *WebBrowser;
    GMPageState *pageState;
#endif
    void ExecFunc(const QString &func);

public:
    int FixCent;

    explicit GoogleMapView(QWidget *parent=NULL);
    int  GetState(void);
    void SetView(double lat, double lon, int zoom);
    void SetCent(double lat, double lon);
    void SetZoom(int zoom);
    void ClearMark(void);
    void AddMark(double lat, double lon, const QString &title, const QString &msg);
    void SetMark(int index, const double *pos);
    void ShowMark(int index);
    void HideMark(int index);
    void HighlightMark(const QString &title);
};
//---------------------------------------------------------------------------
#endif
