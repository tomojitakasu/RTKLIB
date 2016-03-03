//---------------------------------------------------------------------------
// gmview.c: google map view
//---------------------------------------------------------------------------
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>

#include "rtklib.h"
#include "gmview.h"

#define RTKLIB_GM_FILE "rtkplot_gm.htm"

//---------------------------------------------------------------------------
GoogleMapView::GoogleMapView(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnShrink,SIGNAL(clicked(bool)),this,SLOT(BtnShrinkClick()));
    connect(BtnExpand,SIGNAL(clicked(bool)),this,SLOT(BtnExpandClick()));
    connect(BtnFixCent,SIGNAL(clicked(bool)),this,SLOT(BtnFixCentClick()));
    connect(&Timer1,SIGNAL(timeout()),this,SLOT(Timer1Timer()));

    WebBrowser = new QWebView(Panel2);
    QHBoxLayout *layout=new QHBoxLayout();
    layout->addWidget(WebBrowser);
    Panel2->setLayout(layout);


	State=0;
	Lat=Lon=0.0;
	Zoom=2;

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

    Timer1.start(300);
}
//---------------------------------------------------------------------------
void GoogleMapView::BtnCloseClick()
{
    close();
}
//---------------------------------------------------------------------------
void GoogleMapView::Timer1Timer()
{
	if (!GetState()) return;
	
	SetView(Lat,Lon,Zoom);
	
	AddMark(0.0,0.0,"SOL1","SOLUTION 1");
	AddMark(0.0,0.0,"SOL2","SOLUTION 2");

	HideMark(1);
	HideMark(2);

	for (int i=0;i<2;i++) MarkPos[i][0]=MarkPos[i][1]=0.0;

    Timer1.stop();
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
    ExecFunc(QString("SetCent(%1,%2)").arg(lat,0,'f',9).arg(lon,0,'f',9));
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
	
    if (BtnFixCent->isChecked()) {
		SetCent(pos[0]*R2D,pos[1]*R2D);
    }
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
    QWebElement ele;
    int state;

    if (!WebBrowser->page()) return 0;
    if (!WebBrowser->page()->mainFrame()) return 0;

    QWebFrame *frame=WebBrowser->page()->mainFrame();

    ele=frame->findFirstElement("#state");

    if (ele.isNull()) return 0;
    if (!ele.hasAttribute("value")) return 0;

    state=ele.attribute("value").toInt();

	return state;
}
//---------------------------------------------------------------------------
void GoogleMapView::ExecFunc(const QString &func)
{
    if (!WebBrowser->page()) return;
    if (!WebBrowser->page()->mainFrame()) return;

    QWebFrame *frame=WebBrowser->page()->mainFrame();

    frame->evaluateJavaScript(func);
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
    if (BtnFixCent->isChecked()&&MarkPos[0][0]!=0.0&&MarkPos[0][1]!=0.0) {
		SetCent(MarkPos[0][0],MarkPos[0][1]);
	}
}
//---------------------------------------------------------------------------
void GoogleMapView::resizeEvent(QResizeEvent *)
{
    if (BtnFixCent->isChecked()&&MarkPos[0][0]!=0.0&&MarkPos[0][1]!=0.0) {
		SetCent(MarkPos[0][0],MarkPos[0][1]);
	}
}
//---------------------------------------------------------------------------
