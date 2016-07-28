//---------------------------------------------------------------------------
// gmview.c: google map view
//---------------------------------------------------------------------------
#ifdef QWEBKIT
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#endif
#ifdef QWEBENGINE
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebChannel>
#include <QFile>
#endif
#include <QShowEvent>

#include "gmview.h"
#include "rtklib.h"

#define RTKLIB_GM_FILE "rtklib_gm.htm"

//---------------------------------------------------------------------------
GoogleMapView::GoogleMapView(QWidget *parent)
    : QDialog(parent)
{
    loaded=false;
    setupUi(this);

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnShrink,SIGNAL(clicked(bool)),this,SLOT(BtnShrinkClick()));
    connect(BtnExpand,SIGNAL(clicked(bool)),this,SLOT(BtnExpandClick()));
    connect(BtnFixCent,SIGNAL(clicked(bool)),this,SLOT(BtnFixCentClick()));
    connect(&Timer1,SIGNAL(timeout()),this,SLOT(Timer1Timer()));

#ifdef QWEBKIT
    WebBrowser = new QWebView(Panel2);
    QHBoxLayout *layout=new QHBoxLayout();
    layout->addWidget(WebBrowser);
    Panel2->setLayout(layout);
#endif
#ifdef QWEBENGINE
    WebBrowser = new QWebEngineView(Panel2);
    QHBoxLayout *layout=new QHBoxLayout();
    layout->addWidget(WebBrowser);
    Panel2->setLayout(layout);
    pageState=new GMPageState(this);

    connect(WebBrowser,SIGNAL(loadFinished(bool)),this,SLOT(PageLoaded(bool)));
#endif
	State=0;
	Lat=Lon=0.0;
	Zoom=2;
    FixCent=1;

    QTimer::singleShot(0,this,SLOT(FormCreate()));
}
//---------------------------------------------------------------------------
void GoogleMapView::FormCreate()
{
    QString dir;

    dir=qApp->applicationDirPath(); // exe directory
    dir=dir+"/"+RTKLIB_GM_FILE;

#ifdef QWEBKIT
    WebBrowser->load(QUrl::fromLocalFile(dir));
    WebBrowser->show();
    loaded=true;
#endif
#ifdef QWEBENGINE
    WebBrowser->load(QUrl::fromLocalFile(dir));
    QWebChannel *channel=new QWebChannel(this);
    channel->registerObject(QStringLiteral("state"),pageState);

    WebBrowser->page()->setWebChannel(channel);

    WebBrowser->show();
#endif
    Timer1.start(300);
}
//---------------------------------------------------------------------------
void GoogleMapView::BtnCloseClick()
{
    close();
}
//---------------------------------------------------------------------------
void GoogleMapView::PageLoaded(bool ok)
{
    if (!ok) return;

#ifdef QWEBENGINE
    QFile webchannel(":/html/qwebchannel.js");
    webchannel.open(QIODevice::ReadOnly);
    WebBrowser->page()->runJavaScript(webchannel.readAll());
    WebBrowser->page()->runJavaScript("new QWebChannel(qt.webChannelTransport,function(channel) {channel.objects.state.text=document.getElementById('state').value;});");
#endif
    loaded=true;
}
//---------------------------------------------------------------------------
void GoogleMapView::Timer1Timer()
{
	if (!GetState()) return;
	
    State=1;

	SetView(Lat,Lon,Zoom);
	
	AddMark(0.0,0.0,"SOL1","SOLUTION 1");
	AddMark(0.0,0.0,"SOL2","SOLUTION 2");

	HideMark(1);
	HideMark(2);

	for (int i=0;i<2;i++) MarkPos[i][0]=MarkPos[i][1]=0.0;

    Timer1.stop();
}
//---------------------------------------------------------------------------
void GoogleMapView::BtnShrinkClick()
{
    SetZoom(Zoom-1);
}
//---------------------------------------------------------------------------
void GoogleMapView::BtnExpandClick()
{
    SetZoom(Zoom+1);
}
//---------------------------------------------------------------------------
void GoogleMapView::BtnFixCentClick()
{
    FixCent=BtnFixCent->isChecked();
    if (FixCent) SetCent(Lat,Lon);
}
//---------------------------------------------------------------------------
void GoogleMapView::resizeEvent(QResizeEvent *)
{
    if (FixCent) SetCent(Lat,Lon);
}
//---------------------------------------------------------------------------
void GoogleMapView::SetView(double lat, double lon, int zoom)
{
	Lat=lat; Lon=lon; Zoom=zoom;
    ExecFunc(QString("SetView(%1,%2,%3)").arg(lat,0,'f',9).arg(lon,0,'f',9).arg(zoom));
}
//---------------------------------------------------------------------------
void GoogleMapView::SetCent(double lat, double lon)
{
	Lat=lat; Lon=lon;
    if (FixCent) ExecFunc(QString("SetCent(%1,%2)").arg(lat,0,'f',9).arg(lon,0,'f',9));
}
//---------------------------------------------------------------------------
void GoogleMapView::SetZoom(int zoom)
{
    if (zoom<2||zoom>21) return;
	Zoom=zoom;
    ExecFunc(QString("SetZoom(%1)").arg(zoom));
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
void GoogleMapView::SetMark(int index, const double *pos)
{
    QString title;
    title=QString("SOL%1").arg(index);
    ExecFunc(QString("PosMark(%1,%2,\"%3\")").arg(pos[0]*R2D,0,'f',9).arg(pos[1]*R2D,0,'f',9).arg(title));
	
	MarkPos[index-1][0]=pos[0]*R2D;
	MarkPos[index-1][1]=pos[1]*R2D;
}
//---------------------------------------------------------------------------
void GoogleMapView::ShowMark(int index)
{
    QString title;
    title=QString("SOL%1").arg(index);
    ExecFunc(QString("ShowMark(\"%1\")").arg(title));
}
//---------------------------------------------------------------------------
void GoogleMapView::HideMark(int index)
{
    QString title;
    title=QString("SOL%1").arg(index);
    ExecFunc(QString("HideMark(\"%1\")").arg(title));
}
//---------------------------------------------------------------------------
int GoogleMapView::GetState(void)
{
#ifdef QWEBKIT
    QWebElement ele;
    int state=0;

    if (!WebBrowser->page()) return 0;
    if (!WebBrowser->page()->mainFrame()) return 0;

    QWebFrame *frame=WebBrowser->page()->mainFrame();

    ele=frame->findFirstElement("#state");

    if (ele.isNull()) return 0;
    if (!ele.hasAttribute("value")) return 0;

    state=ele.attribute("value").toInt();

	return state;
#else
 #ifdef QWEBENGINE
    if (!loaded) return 0;
    return pageState->getText().toInt();
 #else
    return 0;
 #endif
#endif
}
//---------------------------------------------------------------------------
void GoogleMapView::ExecFunc(const QString &func)
{
#ifdef QWEBKIT
    if (!WebBrowser->page()) return;
    if (!WebBrowser->page()->mainFrame()) return;

    QWebFrame *frame=WebBrowser->page()->mainFrame();

    frame->evaluateJavaScript(func);
#else
 #ifdef QWEBENGINE
    if (!loaded) return;

    QWebEnginePage *page=WebBrowser->page();
    if (page==NULL) return;

    page->runJavaScript(func);
 #else
    Q_UNUSED(func)
 #endif
#endif
}
//---------------------------------------------------------------------------
void GoogleMapView::HighlightMark(const QString &title)
{
    ExecFunc(QString("HighlightMark(\"%1\")").arg(title));
}
//---------------------------------------------------------------------------
