//---------------------------------------------------------------------------
#ifndef gmviewH
#define gmviewH
//---------------------------------------------------------------------------
#include <QDialog>

#include "ui_gmview.h"

class QWebView;

//---------------------------------------------------------------------------
class GoogleMapView : public QDialog, private Ui::GoogleMapView
{
    Q_OBJECT
public slots:
    void BtnCloseClick();
    void BtnHomeClick();
    void FormCreate();


private:
    void ExecFunc(const QString &func);
    QWebView *WebBrowser;

public:
    GoogleMapView(QWidget *parent=NULL);

    void ShowHome(void);
    int  GetState(void);
    void ClearMark(void);
    void AddMark(double lat, double lon, const QString &title, const QString &msg);
    void PosMark(double lat, double lon, const QString &title);
    void HighlightMark(const QString &title);
};
//---------------------------------------------------------------------------
#endif
