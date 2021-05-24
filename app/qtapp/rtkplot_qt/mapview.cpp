//---------------------------------------------------------------------------
// gmview.c: map view
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
#include <QFile>
#include <QTextStream>
#include "mapviewopt.h"
#include "plotmain.h"
#include "mapview.h"
#include "rtklib.h"


// instance of Plot --------------------------------------------------------
extern Plot *plot;

#define RTKLIB_GM_TEMP "rtkplot_gm.htm"
#define RTKLIB_GM_FILE "rtkplot_gm_a.htm"
#define RTKLIB_LL_TEMP "rtkplot_ll.htm"
#define RTKLIB_LL_FILE "rtkplot_ll_a.htm"
#define URL_GM_API     "http://maps.google.com/maps/api/js"
#define MAP_OPACITY    0.8
#define INIT_ZOOM      12  // initial zoom level

//---------------------------------------------------------------------------
MapView::MapView(QWidget *parent)
    : QDialog(parent)
{
    loaded = false;
    setupUi(this);

    MapSel=0;
    Lat=Lon=0.0;
    for (int i=0;i<2;i++) {
        MapState[0]=MapState[1]=0;
        MarkState[0]=MarkState[1]=0;
        MarkPos[i][0]=MarkPos[i][1]=0.0;
    }
    mapViewOptDialog = new MapViewOptDialog(this);

    connect(BtnClose, SIGNAL(clicked(bool)), this, SLOT(BtnCloseClick()));
    connect(BtnOpt, SIGNAL(clicked(bool)), this, SLOT(BtnOptClick()));
    connect(BtnShrink, SIGNAL(clicked(bool)), this, SLOT(BtnZoomOutClick()));
    connect(BtnExpand, SIGNAL(clicked(bool)), this, SLOT(BtnZoomInClick()));
    connect(BtnSync, SIGNAL(clicked(bool)), this, SLOT(BtnSyncClick()));
    connect(&Timer1, SIGNAL(timeout()), this, SLOT(Timer1Timer()));
    connect(&Timer2, SIGNAL(timeout()), this, SLOT(Timer2Timer()));
    connect(MapSel1, SIGNAL(toggled(bool)), this, SLOT(MapSel1Click()));
    connect(MapSel2, SIGNAL(toggled(bool)), this, SLOT(MapSel2Click()));

#ifdef QWEBKIT
    WebBrowser = new QWebView(Panel2);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(WebBrowser);
    Panel2->setLayout(layout);
#endif
#ifdef QWEBENGINE
    WebBrowser = new QWebEngineView(Panel2);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(WebBrowser);
    Panel2->setLayout(layout);
    pageState = new MapViewPageState(this);

    connect(WebBrowser, SIGNAL(loadFinished(bool)), this, SLOT(PageLoaded(bool)));
#endif
}
//---------------------------------------------------------------------------
void MapView::showEvent(QShowEvent*)
{
    MapSel1->setChecked(!MapSel);
    MapSel2->setChecked(MapSel);
    SelectMap(MapSel);
    ShowMap(MapSel);
}
//---------------------------------------------------------------------------
void MapView::BtnCloseClick()
{
    close();
}
//---------------------------------------------------------------------------
void MapView::MapSel1Click()
{
    SelectMap(0);
}
//---------------------------------------------------------------------------
void MapView::MapSel2Click()
{
    SelectMap(1);
}
//---------------------------------------------------------------------------
void MapView::BtnOptClick()
{
    mapViewOptDialog->move(x()+width()/2-mapViewOptDialog->width()/2,
                           y()+height()/2-mapViewOptDialog->height()/2);

    mapViewOptDialog->ApiKey=plot->ApiKey;
    for (int i=0;i<6;i++) for (int j=0;j<3;j++) {
        mapViewOptDialog->MapStrs[i][j]=plot->MapStrs[i][j];
    }
    if (mapViewOptDialog->exec()!=QDialog::Accepted) return;

    plot->ApiKey=mapViewOptDialog->ApiKey;
    for (int i=0;i<6;i++) for (int j=0;j<3;j++) {
        plot->MapStrs[i][j]=mapViewOptDialog->MapStrs[i][j];
    }
    MapState[0]=MapState[1]=0;
    ShowMap(MapSel);
}
//---------------------------------------------------------------------------
void MapView::PageLoaded(bool ok)
{
    if (!ok) return;

#ifdef QWEBENGINE
    QFile webchannel(":/html/qwebchannel.js");
    webchannel.open(QIODevice::ReadOnly);
    WebBrowser->page()->runJavaScript(webchannel.readAll());
    WebBrowser->page()->runJavaScript("new QWebChannel(qt.webChannelTransport,function(channel) {channel.objects.state.text=document.getElementById('state').value;});");
#endif
    loaded = true;
}
//---------------------------------------------------------------------------
void MapView::BtnZoomOutClick()
{
    ExecFunc(MapSel,"ZoomOut()");
}
//---------------------------------------------------------------------------
void MapView::BtnZoomInClick()
{
    ExecFunc(MapSel,"ZoomIn()");
}
//---------------------------------------------------------------------------
void MapView::BtnSyncClick()
{
    if (BtnSync->isChecked()) {
        SetCent(Lat,Lon);
    }
}
//---------------------------------------------------------------------------
void MapView::resizeEvent(QResizeEvent *)
{
    if (BtnSync->isChecked()) SetCent(Lat, Lon);
}
//---------------------------------------------------------------------------
void MapView::ShowMap(int map)
{
    if (map==0&&!MapState[0]) {
        ShowMapLL();
    }
    else if (map==1&&!MapState[1]) {
        ShowMapGM();
    }
    else {
        UpdateMap();
    }
}
//---------------------------------------------------------------------------
void MapView::ShowMapLL(void)
{
    QString dir, ifile,ofile;
    int i, j;

    dir = qApp->applicationDirPath(); // exe directory

    ifile=dir+"/"+RTKLIB_LL_TEMP;
    ofile=dir+"/"+RTKLIB_LL_FILE;

    QFile ifp(ifile), ofp(ofile);

    if (!ifp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    if (!ofp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ifp.close();
        return;
    }
    QTextStream out(&ofp);
    while (!ifp.atEnd()) {
        QString buff = ifp.readLine();
        out<<buff;
        if (!buff.contains("// start map tiles")) continue;
        for (i=0,j=1;i<6;i++) {
            if (plot->MapStrs[i][0]=="") continue;
            QString title=plot->MapStrs[i][0];
            QString url  =plot->MapStrs[i][1];
            QString attr =plot->MapStrs[i][2];

            out << QString("var tile%1 = L.tileLayer('%2', {\n").arg(j).arg(url);
            out << QString("  attribution: \"<a href='%1' target='_blank'>%2</a>\",\n")
                    .arg(attr).arg(title);
            out << QString("  opacity: %1});\n").arg(MAP_OPACITY,0,'f',1);
            j++;
        }
        out << "var basemaps = {";
        for (i=0,j=1;i<6;i++) {
            if (plot->MapStrs[i][0]=="") continue;
            QString title=plot->MapStrs[i][0];
            out << QString("%1\"%2\":tile%3").arg((j==1)?"":",").arg(title).arg(j);
            j++;
        }
        out << "};\n";
    }
    ifp.close();
    ofp.close();

#ifdef QWEBKIT
    WebBrowser->load(QUrl::fromLocalFile(ofile));
    WebBrowser->show();
    loaded = true;
#endif
#ifdef QWEBENGINE
    WebBrowser->load(QUrl::fromLocalFile(ofile));
    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("state"), pageState);

    WebBrowser->page()->setWebChannel(channel);

    WebBrowser->show();
#endif

    Timer1.start();
}
//---------------------------------------------------------------------------
void MapView::Timer1Timer()
{
    if (!GetState(0)) return;

    MapState[0]=1;
    MapState[1]=0;
    SetView(0,Lat,Lon,INIT_ZOOM);
    AddMark(0,1,MarkPos[0][0],MarkPos[0][1],MarkState[0]);
    AddMark(0,2,MarkPos[1][0],MarkPos[1][1],MarkState[1]);

    Timer1.stop();
}
void MapView::ShowMapGM(void)
{
    QString dir,ifile,ofile;
    int p;

    dir = qApp->applicationDirPath(); // exe directory

    ifile=dir+"/"+RTKLIB_LL_TEMP;
    ofile=dir+"/"+RTKLIB_LL_FILE;

    QFile ifp(ifile), ofp(ofile);

    if (!ifp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    if (!ofp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ifp.close();
        return;
    }
    QTextStream out(&ofp);
    while (!ifp.atEnd()) {
        QString buff = ifp.readLine();

        if ((!plot->ApiKey.isEmpty())&&(p=buff.indexOf(QLatin1String(URL_GM_API)))!=-1) {
            p+=strlen(URL_GM_API);
            buff.insert(p, QString("?key=%1").arg(plot->ApiKey));
        }

        out<<buff;
    }
    ifp.close();
    ofp.close();

#ifdef QWEBKIT
    WebBrowser->load(QUrl::fromLocalFile(ofile));
    WebBrowser->show();
    loaded = true;
#endif
#ifdef QWEBENGINE
    WebBrowser->load(QUrl::fromLocalFile(ofile));
    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("state"), pageState);

    WebBrowser->page()->setWebChannel(channel);

    WebBrowser->show();
#endif

    Timer2.start();

}
//---------------------------------------------------------------------------
void MapView::Timer2Timer()
{
    if (!GetState(1)) return;
    MapState[1]=1;
    MapState[0]=0;
    SetView(1,Lat,Lon,INIT_ZOOM);
        AddMark(1,1,MarkPos[0][0],MarkPos[0][1],MarkState[0]);
        AddMark(1,2,MarkPos[1][0],MarkPos[1][1],MarkState[1]);
        Timer2.stop();
}

//---------------------------------------------------------------------------
void MapView::SetView(int map, double lat, double lon, int zoom)
{
    ExecFunc(map, QString("SetView(%1,%2,%3)").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(zoom));
}

//---------------------------------------------------------------------------
void MapView::AddMark(int map, int index, double lat, double lon,
                int state)
{
    QString func = QString("AddMark(%1,%2,'SOL%3','SOLUTION %4')").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(index).arg(index);

   ExecFunc(map,func);
   if (state) func = QString("ShowMark('SOL%1')").arg(index);
   else       func = QString("HideMark('SOL%1')").arg(index);
   ExecFunc(map,func);
}
//---------------------------------------------------------------------------
void MapView::UpdateMap(void)
{
    SetCent(Lat,Lon);
    for (int i=0;i<2;i++) {
        SetMark(i+1,MarkPos[i][0],MarkPos[i][1]);
        if (MarkState[i]) ShowMark(i+1); else HideMark(i+1);
    }
}
//---------------------------------------------------------------------------
void MapView::SelectMap(int map)
{
    MapSel=map;
    ShowMap(map);
}

//---------------------------------------------------------------------------
void MapView::SetCent(double lat, double lon)
{
    QString func=QString("SetCent(%1,%2)").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9);
    Lat = lat; Lon = lon;

    if (BtnSync->isChecked()) {
        ExecFunc(MapSel,func);
    }
}
//---------------------------------------------------------------------------
void MapView::SetMark(int index, double lat, double lon)
{
    QString func = QString("PosMark(%1,%2,'SOL%3')").arg(lat, 0, 'f', 9).arg(lon, 0, 'f', 9).arg(index);

    MarkPos[index-1][0]=lat;
    MarkPos[index-1][1]=lon;

    ExecFunc(MapSel,func);

}
//---------------------------------------------------------------------------
void MapView::ShowMark(int index)
{
    QString func = QString("ShowMark('SOL%1')").arg(index);

    MarkState[index-1]=1;
    ExecFunc(MapSel,func);
}
//---------------------------------------------------------------------------
void MapView::HideMark(int index)
{
    QString func = QString("HideMark('SOL%1')").arg(index);

    MarkState[index-1]=0;
    ExecFunc(MapSel,func);
}
//---------------------------------------------------------------------------
int MapView::GetState(int map)
{
    Q_UNUSED(map)
#ifdef QWEBKIT
    QWebElement ele;
    int state = 0;

    if (!WebBrowser->page()) return 0;
    if (!WebBrowser->page()->mainFrame()) return 0;

    QWebFrame *frame = WebBrowser->page()->mainFrame();

    qDebug() << frame;

    ele = frame->findFirstElement("#state");

    if (ele.isNull()) return 0;
    if (!ele.hasAttribute("value")) return 0;

    state = ele.attribute("value").toInt();

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
void MapView::ExecFunc(int map, const QString &func)
{
    Q_UNUSED(map)
#ifdef QWEBKIT
    if (!WebBrowser->page()) return;
    if (!WebBrowser->page()->mainFrame()) return;

    QWebFrame *frame = WebBrowser->page()->mainFrame();

    frame->evaluateJavaScript(func);
#else
#ifdef QWEBENGINE
    if (!loaded) return;

    QWebEnginePage *page = WebBrowser->page();
    if (page == NULL) return;

    page->runJavaScript(func);
#else
    Q_UNUSED(func)
#endif
#endif
}
//---------------------------------------------------------------------------
