//---------------------------------------------------------------------------
// geview.c: google earth view
//---------------------------------------------------------------------------

#include <QTimer>
#include <QStringList>
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>

#include "rtklib.h"
#include "geview.h"
#include "plotmain.h"

#define RTKPLOT_GE_FILE "rtkplot_ge.htm"

#define TIMEOUT_GE  5000   // timeout of GE load (ms)
#define MAXTRACKS   4096   // max number of track poitnts

#define INIT_RANGE  4.322  // initial range (km)
#define MIN_RANGE   0.01   // min range (km)
#define MAX_RANGE   20000.0 // max range (km)

#define TILT_ANGLE  70.0   // tilt angle (deg)

#define MIN(x,y)    ((x)<(y)?(x):(y))
#define MAX(x,y)    ((x)>(y)?(x):(y))
#define ATAN2(x,y)  ((x)*(x)+(y)*(y)>1E-12?atan2(x,y):0.0)

extern Plot *plot;

//---------------------------------------------------------------------------
 GoogleEarthView::GoogleEarthView(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    WebBrowser=new QWebView(this);

    setCentralWidget(WebBrowser);

    toolBar->addWidget(BtnOpt1);
    toolBar->addWidget(BtnOpt2);
    toolBar->addWidget(BtnOpt3);
    toolBar->addWidget(BtnOpt4);
    toolBar->addWidget(BtnOpt5);
    toolBar->addWidget(BtnOpt6);
    toolBar->addWidget(BtnOpt7);
    toolBar->addWidget(BtnOpt8);
    toolBar->addWidget(BtnOpt9);
    toolBar_2->addWidget(BtnFixCent);
    toolBar_2->addWidget(BtnEnaAlt);
    toolBar_2->addSeparator();
    toolBar_2->addWidget(BtnGENorm);
    toolBar_2->addWidget(BtnGETilt);
    toolBar_2->addWidget(BtnExpand);
    toolBar_2->addWidget(BtnShrink);
    toolBar_2->addSeparator();
    toolBar_2->addWidget(BtnRotL);
    toolBar_2->addWidget(BtnRotR);
    toolBar_2->addWidget(BtnHeading);
    toolBar_2->addWidget(BtnClose);

    State=0;
    Expand=Rotate=0;
    Lat=Lon=LatSet=LonSet=0.0;
    Range=RangeSet=0.0;
    Heading=HeadingSet=0.0;

    Clear();

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnEnaAlt,SIGNAL(clicked(bool)),this,SLOT(BtnEnaAltClick()));
    connect(BtnExpand,SIGNAL(pressed()),this,SLOT(BtnExpandPressed()));
    connect(BtnExpand,SIGNAL(released()),this,SLOT(BtnExpandReleased()));
    connect(BtnFixCent,SIGNAL(clicked(bool)),this,SLOT(BtnFixCentClick()));
    connect(BtnGENorm,SIGNAL(clicked(bool)),this,SLOT(BtnGENormClick()));
    connect(BtnGETilt,SIGNAL(clicked(bool)),this,SLOT(BtnGETiltClick()));
    connect(BtnHeading,SIGNAL(clicked(bool)),this,SLOT(BtnHeadingClick()));
    connect(BtnRotL,SIGNAL(pressed()),this,SLOT(BtnRotLPressed()));
    connect(BtnRotL,SIGNAL(released()),this,SLOT(BtnRotLReleased()));
    connect(BtnRotR,SIGNAL(pressed()),this,SLOT(BtnRotRPressed()));
    connect(BtnRotR,SIGNAL(released()),this,SLOT(BtnRotRReleased()));
    connect(BtnShrink,SIGNAL(pressed()),this,SLOT(BtnShrinkPressed()));
    connect(BtnShrink,SIGNAL(released()),this,SLOT(BtnShrinkReleased()));
    connect(&Timer1,SIGNAL(timeout()),this,SLOT(Timer1Timer()));
    connect(&Timer2,SIGNAL(timeout()),this,SLOT(Timer2Timer()));

    QTimer::singleShot(0,this,SLOT(FormCreate()));
}
//---------------------------------------------------------------------------
void  GoogleEarthView::FormCreate()
{
    QString url,dir=".";
    
    dir=qApp->applicationDirPath(); // exe directory
    url="file://"+dir+"/"+RTKPLOT_GE_FILE;
    
    WebBrowser->load(QUrl(url));
    WebBrowser->show();
}
//---------------------------------------------------------------------------
void  GoogleEarthView::Timer1Timer()
{
    QWebElement ele1,ele2;
    int state;
    
    if (!WebBrowser->page()) return;
    if (!WebBrowser->page()->mainFrame()) return;

    QWebFrame *frame=WebBrowser->page()->mainFrame();

    ele1=frame->findFirstElement("state");
    ele1=frame->findFirstElement("view");

    if (ele1.isNull()||ele2.isNull()) return;
    
    if (!ele1.hasAttribute("value")||
         ele2.hasAttribute("value")) return;

    QStringList tokens=ele2.attribute("value").split(',');

    if (tokens.size()!=4) return;

    state=ele1.attribute("value").toInt();
    Lat=tokens.at(0).toDouble();
    Lon=tokens.at(1).toDouble();
    Range=tokens.at(2).toDouble();
    Heading=tokens.at(3).toDouble();

    if (!State&&state) {
        State=state;
        UpdateOpts();
        if (LatSet!=0.0||LonSet!=0.0) {
            SetView(LatSet,LonSet,RangeSet,HeadingSet);
            plot->Refresh_GEView();
        }
    }
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnCloseClick()
{
    close();
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnOpt1Click()
{
    UpdateOpts();
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnHeadingClick()
{
    if (!BtnHeading->isChecked()) SetHeading(0.0);
    UpdateEnable();
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnFixCentClick()
{
    plot->Refresh_GEView();
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnEnaAltClick()
{
    UpdateOpts();
    plot->Refresh_GEView();
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnGENormClick()
{
    ExecFunc("SetTilt(0.0)");
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnGETiltClick()
{
    ExecFunc(QString("SetTilt(%1)").arg(TILT_ANGLE,0,'f',1));
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnShrinkPressed()
{
    Timer1.stop();
    Timer2.start();
    Expand=1;
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnShrinkReleased()
{
    Expand=0;
    Timer1.start();
    Timer2.stop();
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnExpandPressed()
{
    Timer1.stop();
    Timer2.start();
    Expand=-1;
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnExpandReleased()
{
    Expand=0;
    Timer1.start();
    Timer2.stop();
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnRotLPressed()
{
    Timer1.stop();
    Timer2.start();
    Rotate=1;
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnRotLReleased()
{
    Rotate=0;
    Timer1.start();
    Timer2.stop();
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnRotRPressed()
{
    Timer1.stop();
    Timer2.start();
    Rotate=-1;
}
//---------------------------------------------------------------------------
void  GoogleEarthView::BtnRotRReleased()
{
    Rotate=0;
    Timer1.start();
    Timer2.stop();
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void  GoogleEarthView::Timer2Timer()
{
    if (Expand) {
        if (Expand>0) Range=MIN(MAX_RANGE,Range*1.05);
        else          Range=MAX(MIN_RANGE,Range/1.05);
        SetRange(Range);
    }
    if (Rotate) {
        if (Rotate>0) Heading+=3.0;
        else          Heading-=3.0;
        if      (Heading> 180.0) Heading-=360.0;
        else if (Heading<-180.0) Heading+=360.0;
        SetHeading(Heading);
    }
}
//---------------------------------------------------------------------------
void  GoogleEarthView::Clear(void)
{
    MarkVis[0]=MarkVis[1]=TrackVis[0]=TrackVis[1]=0;
    MarkPos[0][0]=MarkPos[0][1]=0.0;
    MarkPos[1][0]=MarkPos[1][1]=0.0;
    ExecFunc("ClearTrack(1)");
    ExecFunc("ClearTrack(2)");
    ExecFunc("SetMark(1,0.0,0.0)");
    ExecFunc("SetMark(2,0.0,0.0)");
}
// --------------------------------------------------------------------------
void  GoogleEarthView::SetView(double lat, double lon, double range,
    double heading)
{
    if (range<=0.0) range=RangeSet<=0.0?INIT_RANGE:RangeSet;
    LatSet=lat; LonSet=lon; RangeSet=range; HeadingSet=heading;
    ExecFunc(QString("SetView(%1,%2,%3,%4)").arg(lat,0,'f',9).arg(lon,0,'f',9).arg(range,0,'f',3).arg(heading,0,'f',1));
}
// --------------------------------------------------------------------------
void  GoogleEarthView::SetCent(double lat, double lon)
{
    ExecFunc(QString("SetCent(%1,%2)").arg(lat,0,'f',9).arg(lon,0,'f',9));
}
// --------------------------------------------------------------------------
void  GoogleEarthView::SetRange(double range)
{
    ExecFunc(QString("SetRange(%1)").arg(range,0,'f',3));
}
/// --------------------------------------------------------------------------
void  GoogleEarthView::SetHeading(double angle)
{
    ExecFunc(QString("SetHeading(%1)").arg(angle,0,'f',2));
}
// --------------------------------------------------------------------------
void  GoogleEarthView::SetMark(int index, const double *pos)
{
    if (index<1||2<index) return;
    MarkPos[index-1][0]=pos[0]*R2D;
    MarkPos[index-1][1]=pos[1]*R2D;
    ExecFunc(QString("SetMark(%1,%2,%3,%4)").arg(index).arg(pos[0]*R2D,0,'f',9)
             .arg(pos[1]*R2D,0,'f',9).arg(pos[2],0,'f',3));
    
    if (BtnFixCent->isChecked()) {
        SetCent(pos[0]*R2D,pos[1]*R2D);
    }
}
// --------------------------------------------------------------------------
void  GoogleEarthView::ShowMark(int index)
{
    if (index<1||2<index) return;
    ExecFunc(QString("ShowMark(%1)").arg(index));
    MarkVis[index-1]=1;
    UpdateEnable();
}
// --------------------------------------------------------------------------
void  GoogleEarthView::HideMark(int index)
{
    if (index<1||2<index) return;
    ExecFunc(QString("HideMark(%1)").arg(index));
    MarkVis[index-1]=0;
    UpdateEnable();
}
// --------------------------------------------------------------------------
void  GoogleEarthView::ClearTrack(int index)
{
    if (index<1||2<index) return;
    ExecFunc(QString("ClearTrack(%1)").arg(index));
    TrackVis[index-1]=0;
    UpdateEnable();
}
// --------------------------------------------------------------------------
int  GoogleEarthView::UpdateTrack(int index, solbuf_t *sol)
{
    sol_t *data;
    double prev[3]={0},pos[3];
    int i,intv;
    
    if (index<1||2<index||!State||sol->n<=0) return 0;
    
    setCursor(Qt::WaitCursor);
    
    ClearTrack(index);
    
    intv=sol->n/MAXTRACKS+1; // interval to reduce points
    
    for (i=0;(data=getsol(sol,i))!=NULL;i++) {
        if (i%intv!=0) continue;
        ecef2pos(data->rr,pos);
        if (fabs(pos[0]-prev[0])<1E-8&&fabs(pos[1]-prev[1])<1E-8) continue;
        prev[0]=pos[0];
        prev[1]=pos[1];
        ExecFunc(QString("AddTrack(%d,%.9f,%.9f)").arg(index).arg(pos[0]*R2D,0,'f',9)
                 .arg(pos[1]*R2D,0,'f',9));
    }
    setCursor(Qt::ArrowCursor);
    UpdateEnable();
    return 1;
}
// --------------------------------------------------------------------------
void  GoogleEarthView::ShowTrack(int index)
{
    if (index<1||2<index) return;
    ExecFunc(QString("ShowTrack(%1)").arg(index));
    TrackVis[index-1]=1;
    UpdateEnable();
}
// --------------------------------------------------------------------------
void  GoogleEarthView::HideTrack(int index)
{
    if (index<1||2<index) return;
    ExecFunc(QString("HideTrack(%1)").arg(index));
    TrackVis[index-1]=0;
    UpdateEnable();
}
// ----------------------------------------------------------------------------
void  GoogleEarthView::UpdatePoint(void)
{
    double pos[3];
    int i;
    
    ExecFunc("ClearPoint()");
    
    for (i=0;i<plot->NWayPnt;i++) {
        ecef2pos(plot->PntPos[i],pos);
        ExecFunc(QString("AddPoint('%1',%2,%3,%4)").arg(plot->PntName[i])
                 .arg(pos[0]*R2D,0,'f',9).arg(pos[1]*R2D,0,'f',9).arg(pos[2],0,'f',2));
    }
}
// --------------------------------------------------------------------------
void  GoogleEarthView::ShowPoint(void)
{
    ExecFunc("ShowPoint()");
}
// --------------------------------------------------------------------------
void  GoogleEarthView::HidePoint(void)
{
    ExecFunc("HidePoint()");
}
//---------------------------------------------------------------------------
void  GoogleEarthView::SetOpts(const int *opts)
{
    QToolButton *btn[]={
        BtnOpt1,BtnOpt2,BtnOpt3,BtnOpt4,BtnOpt5,BtnOpt6,BtnOpt7,BtnOpt8,
        BtnOpt9,BtnEnaAlt,BtnHeading
    };
    for (int i=0;i<11;i++) {
        btn[i]->setChecked(opts[i]);
    }
}
//---------------------------------------------------------------------------
void  GoogleEarthView::GetOpts(int *opts)
{
    QToolButton *btn[]={
        BtnOpt1,BtnOpt2,BtnOpt3,BtnOpt4,BtnOpt5,BtnOpt6,BtnOpt7,BtnOpt8,
        BtnOpt9,BtnEnaAlt,BtnHeading
    };
    for (int i=0;i<11;i++) {
        opts[i]=btn[i]->isChecked();
    }
}
//---------------------------------------------------------------------------
void  GoogleEarthView::UpdateOpts(void)
{
    QString f;
    int opts[12];
    
    GetOpts(opts);
    ExecFunc(QString("SetOpts(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10)").arg(opts[0])
             .arg(opts[1]).arg(opts[2]).arg(opts[3]).arg(opts[4]).arg(opts[5]).arg(opts[6]).arg(opts[7]).arg(opts[8])
             .arg(opts[9]));
}
//---------------------------------------------------------------------------
void  GoogleEarthView::UpdateEnable(void)
{
    BtnFixCent->setEnabled(MarkVis[0]||MarkVis[1]);
    BtnEnaAlt ->setEnabled(MarkVis[0]||MarkVis[1]);
    BtnRotR   ->setEnabled(!BtnHeading->isChecked());
    BtnRotL   ->setEnabled(!BtnHeading->isChecked());
}
//---------------------------------------------------------------------------
void  GoogleEarthView::ExecFunc(const QString &func)
{
    if (!WebBrowser->page()) return;
    if (!WebBrowser->page()->mainFrame()) return;
    QWebFrame *frame=WebBrowser->page()->mainFrame();

    frame->evaluateJavaScript(func);
}
//---------------------------------------------------------------------------

void  GoogleEarthView::BtnOptClick()
{
    toolBar->setVisible(!toolBar->isVisible());
}
//---------------------------------------------------------------------------
