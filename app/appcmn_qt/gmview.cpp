//---------------------------------------------------------------------------
// gmview.c: google map view
//---------------------------------------------------------------------------
#include <QWebView>
#include <QWebElement>
#include <QWebFrame>
#include <QTimer>

#include "rtklib.h"
#include "gmview.h"


#define RTKLIB_GM_FILE "rtklib_gmap.htm"

//---------------------------------------------------------------------------
GoogleMapView::GoogleMapView(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnHome,SIGNAL(clicked(bool)),this,SLOT(BtnHomeClick()));

    QTimer::singleShot(0,this,SLOT(FormCreate()));
}
//---------------------------------------------------------------------------
void GoogleMapView::FormCreate()
{
    QString url,dir=".";

    dir=qApp->applicationDirPath(); // exe directory
    url="file://"+dir+"/"+RTKLIB_GM_FILE;

    WebBrowser->load(QUrl(url));
    WebBrowser->show();
}
//---------------------------------------------------------------------------
int GoogleMapView::GetState(void)
{
    QWebElement ele;
    int state;

    if (!WebBrowser->page()) return 0;
    if (!WebBrowser->page()->mainFrame()) return 0;

    QWebFrame *frame=WebBrowser->page()->mainFrame();

    ele=frame->findFirstElement("state");

    if (ele.isNull()) return 0;
    if (!ele.hasAttribute("value)")) return 0;

    state=ele.attribute("value").toInt();

    return state;
}
//---------------------------------------------------------------------------
void GoogleMapView::BtnCloseClick()
{
    close();
}
//---------------------------------------------------------------------------
void GoogleMapView::BtnHomeClick()
{
	ShowHome();
}
//---------------------------------------------------------------------------
void GoogleMapView::ShowHome(void)
{
    ExecFunc("ShowHome()");
}
//---------------------------------------------------------------------------
void GoogleMapView::ClearMark(void)
{
    ExecFunc("ClearMark()");
}
//---------------------------------------------------------------------------
void GoogleMapView::AddMark(double lat, double lon,
    const QString &title, const QString &msg)
{
    ExecFunc(QString("AddMark(%1,%2,\"%3\",\"%4\")").arg(lat,0,'f',9).arg(lon,0,'f',9).arg(title).arg(msg));
}
//---------------------------------------------------------------------------
void GoogleMapView::PosMark(double lat, double lon,
    const QString &title)
{
    ExecFunc(QString("PosMark(%1,%2,\"%3\")").arg(lat,0,'f',9).arg(lon,0,'f',9).arg(title));
}
//---------------------------------------------------------------------------
void GoogleMapView::HighlightMark(const QString &title)
{
    ExecFunc(QString("HighlightMark(\"%1\")").arg(title));
}
//---------------------------------------------------------------------------
void GoogleMapView::ExecFunc(const QString &func)
{
    if (!WebBrowser->page()) return;
    if (!WebBrowser->page()->mainFrame()) return;

    QWebFrame *frame=WebBrowser->page()->mainFrame();

    frame->evaluateJavaScript(func);}
//---------------------------------------------------------------------------
