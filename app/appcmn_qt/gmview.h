//---------------------------------------------------------------------------
#ifndef gmviewH
#define gmviewH
//---------------------------------------------------------------------------
#include <QDialog>
#include <QTimer>

#include "ui_gmview.h"

class QWebView;
class QResizeEvent;
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

protected:
    void resizeEvent(QResizeEvent*);

private:
	int State;
	double Lat,Lon,Zoom;
	double MarkPos[2][2];
    QTimer Timer1;
    QWebView *WebBrowser;
	
    void ExecFunc(const QString &func);

public:
    GoogleMapView(QWidget *parent=NULL);
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
