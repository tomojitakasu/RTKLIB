//---------------------------------------------------------------------------
#ifndef geviewH
#define geviewH
//---------------------------------------------------------------------------
#include <QMainWindow>
#include <QTimer>

#include "ui_geview.h"

#include "rtklib.h"

#ifdef QWEBKIT
class QWebView;
#endif
//---------------------------------------------------------------------------
class GoogleEarthView : public QMainWindow, private Ui::GoogleEarthView
{
    Q_OBJECT

public slots:
    void FormCreate();
    void BtnGENormClick();
    void BtnGETiltClick();
    void BtnOpt1Click();
    void BtnHeadingClick();
    void BtnCloseClick();
    void BtnFixCentClick();
    void BtnEnaAltClick();
    void BtnShrinkPressed();
    void BtnShrinkReleased();
    void BtnExpandPressed();
    void BtnExpandReleased();
    void BtnRotLPressed();
    void BtnRotLReleased();
    void BtnRotRPressed();
    void BtnRotRReleased();
    void BtnOptClick();
    void Timer1Timer();
    void Timer2Timer();

private:
    int State,Expand,Rotate,MarkVis[2],TrackVis[2];
    double Lat,Lon,Range,Heading,LatSet,LonSet,RangeSet,HeadingSet;
    double MarkPos[2][2];
    QTimer Timer1,Timer2;
#ifdef QWEBKIT
    QWebView *WebBrowser;
#endif
    void UpdateOpts (void);
    void UpdateEnable(void);
    void ExecFunc   (const QString &func);

public:
    GoogleEarthView(QWidget *parent=NULL);

    void Init       (void);
    void Clear      (void);
    void SetView    (double lat, double lon, double range, double heading);
    void SetCent    (double lat, double lon);
    void SetRange   (double range);
    void SetHeading (double angle);
    void SetMark    (int index, const double *pos);
    void ShowMark   (int index);
    void HideMark   (int index);
    void ClearTrack (int index);
    int  UpdateTrack(int index, solbuf_t *sol);
    void ShowTrack  (int index);
    void HideTrack  (int index);
    void UpdatePoint(void);
    void ShowPoint  (void);
    void HidePoint  (void);
    void SetOpts    (const int *opts);
    void GetOpts    (int *opts);
};
//---------------------------------------------------------------------------
#endif
