//---------------------------------------------------------------------------
// gmview.c: google map view
//---------------------------------------------------------------------------
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebChannel>

#include <QFile>
#include <QShowEvent>

#include "gmview.h"
#include "rtklib.h"

#include "gm_template.h"

//---------------------------------------------------------------------------
GoogleMapView::GoogleMapView(QWidget *parent)
    : QDialog(parent)
{
    loaded = false;
    setupUi(this);

    connect(BtnClose, SIGNAL(clicked(bool)), this, SLOT(btnCloseClick()));
    connect(BtnShrink, SIGNAL(clicked(bool)), this, SLOT(btnShrinkClick()));
    connect(BtnExpand, SIGNAL(clicked(bool)), this, SLOT(btnExpandClick()));
    connect(BtnFixCent, SIGNAL(clicked(bool)), this, SLOT(btnFixCentClick()));
    connect(&loadTimer, SIGNAL(timeout()), this, SLOT(loadTimerExpired()));

    webBrowser = new QWebEngineView(MapPanel);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(webBrowser);
    MapPanel->setLayout(layout);
    pageState = new GMPageState(this);

    connect(webBrowser, SIGNAL(loadFinished(bool)), this, SLOT(pageLoaded(bool)));

    state = 0;
    latitude = longitude = 0.0;
    zoom = 2;
    fixCenter = true;
}

//---------------------------------------------------------------------------
int GoogleMapView::setApiKey(QString ApiKey)
{
    htmlPage.replace("_APIKEY_", ApiKey);

    webBrowser->setHtml(htmlPage);
    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("state"), pageState);

    webBrowser->page()->setWebChannel(channel);

    webBrowser->show();

    loadTimer.start(300);

    return 0;
}
//---------------------------------------------------------------------------
void GoogleMapView::btnCloseClick()
{
    trace(2,"gmview close\n");
    close();
}
//---------------------------------------------------------------------------
void GoogleMapView::pageLoaded(bool ok)
{
    if (!ok) return;

    QFile webchannel(":/html/qwebchannel.js");
    webchannel.open(QIODevice::ReadOnly);
    webBrowser->page()->runJavaScript(webchannel.readAll());
    webBrowser->page()->runJavaScript("new QWebChannel(qt.webChannelTransport,function(channel) {channel.objects.state.text=document.getElementById('state').value;});");

    loaded = true;
}
//---------------------------------------------------------------------------
void GoogleMapView::loadTimerExpired()
{
    if (!getState()) return;

    state = 1;

    setView(latitude, longitude, zoom);

    addMark(0.0, 0.0, "SOL1", tr("SOLUTION 1"));
    addMark(0.0, 0.0, "SOL2", tr("SOLUTION 2"));

    hideMark(1);
    hideMark(2);

    for (int i = 0; i < 2; i++) markPosition[i][0] = markPosition[i][1] = 0.0;

    loadTimer.stop();
}
//---------------------------------------------------------------------------
void GoogleMapView::btnShrinkClick()
{
    setZoom(zoom - 1);
}
//---------------------------------------------------------------------------
void GoogleMapView::btnExpandClick()
{
    setZoom(zoom + 1);
}
//---------------------------------------------------------------------------
void GoogleMapView::btnFixCentClick()
{
    fixCenter = BtnFixCent->isChecked();
    if (fixCenter) setCent(latitude, longitude);
}
//---------------------------------------------------------------------------
void GoogleMapView::resizeEvent(QResizeEvent *)
{
    if (fixCenter) setCent(latitude, longitude);
}
//---------------------------------------------------------------------------
void GoogleMapView::setView(double lat, double lon, int zoom)
{
    latitude = lat; longitude = lon; zoom = zoom;
    ExecFunc(QString("SetView(%1,%2,%3)").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(zoom));
}
//---------------------------------------------------------------------------
void GoogleMapView::setCent(double lat, double lon)
{
    latitude = lat; longitude = lon;
    if (fixCenter) ExecFunc(QString("SetCent(%1,%2)").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9));
}
//---------------------------------------------------------------------------
void GoogleMapView::setZoom(int zoom)
{
    if (zoom < 2 || zoom > 21) return;
    zoom = zoom;
    ExecFunc(QString("SetZoom(%1)").arg(zoom));
}
//---------------------------------------------------------------------------
void GoogleMapView::clearMark(void)
{
    ExecFunc("ClearMark()");
}
//---------------------------------------------------------------------------
void GoogleMapView::addMark(double lat, double lon,
                const QString &title, const QString &msg)
{
    ExecFunc(QString("AddMark(%1,%2,\"%3\",\"%4\")").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(title).arg(msg));
}
//---------------------------------------------------------------------------
void GoogleMapView::setMark(int index, const double *pos)
{
    QString title;

    title = QString("SOL%1").arg(index);
    ExecFunc(QString("PosMark(%1,%2,\"%3\")").arg(pos[0] * R2D, 0, 'f', 9).arg(pos[1] * R2D, 0, 'f', 9).arg(title));

    markPosition[index - 1][0] = pos[0] * R2D;
    markPosition[index - 1][1] = pos[1] * R2D;
}
//---------------------------------------------------------------------------
void GoogleMapView::showMark(int index)
{
    QString title;

    title = QString("SOL%1").arg(index);
    ExecFunc(QString("ShowMark(\"%1\")").arg(title));
}
//---------------------------------------------------------------------------
void GoogleMapView::hideMark(int index)
{
    QString title;

    title = QString("SOL%1").arg(index);
    ExecFunc(QString("HideMark(\"%1\")").arg(title));
}
//---------------------------------------------------------------------------
int GoogleMapView::getState(void)
{
    if (!loaded) return 0;
    return pageState->getText().toInt();
}
//---------------------------------------------------------------------------
void GoogleMapView::ExecFunc(const QString &func)
{
    if (!loaded) return;

    QWebEnginePage *page = webBrowser->page();
    if (page == NULL) return;

    page->runJavaScript(func);
}
//---------------------------------------------------------------------------
void GoogleMapView::highlightMark(const QString &title)
{
    ExecFunc(QString("HighlightMark(\"%1\")").arg(title));
}
//---------------------------------------------------------------------------
