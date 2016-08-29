//---------------------------------------------------------------------------
// geview.c: google earth view
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <mshtml.h>
#include "rtklib.h"
#include "geview.h"
#include "plotmain.h"

#define RTKPLOT_GE_FILE L"rtkplot_ge.htm"

#define TIMEOUT_GE  5000   // timeout of GE load (ms)
#define MAXTRACKS   4096   // max number of track poitnts

#define INIT_RANGE  4.322  // initial range (km)
#define MIN_RANGE   0.01   // min range (km)
#define MAX_RANGE   20000.0 // max range (km)

#define TILT_ANGLE  70.0   // tilt angle (deg)

#define MIN(x,y)    ((x)<(y)?(x):(y))
#define MAX(x,y)    ((x)>(y)?(x):(y))
#define ATAN2(x,y)  ((x)*(x)+(y)*(y)>1E-12?atan2(x,y):0.0)

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SHDocVw_OCX"
#pragma resource "*.dfm"

TGoogleEarthView *GoogleEarthView;
//---------------------------------------------------------------------------
__fastcall TGoogleEarthView::TGoogleEarthView(TComponent* Owner)
    : TForm(Owner)
{
    State=Expand=Rotate=0;
    Lat=Lon=Range=Heading=0.0;
    FixCent=1;
    Clear();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::FormCreate(TObject *Sender)
{
    UnicodeString url,exe,dir=L".";
    wchar_t *p,*q;
    
    exe=Application->ExeName; // exe directory
    p=exe.c_str();
    if ((q=wcsrchr(p,L'\\'))) {
        dir=exe.SubString(1,q-p);
    }
    url=L"file://"+dir+L"\\"+RTKPLOT_GE_FILE;
    
    WebBrowser->Navigate(url.c_str());
    
    Timer1->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::Timer1Timer(TObject *Sender)
{
    if (!GetState()) return;
    
    State=1;
    UpdateOpts();
    SetView(Lat,Lon,Range,Heading);
    Timer1->Enabled=false;
    Plot->Refresh_GEView();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnCloseClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnOpt1Click(TObject *Sender)
{
    UpdateOpts();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnHeadingClick(TObject *Sender)
{
    if (!BtnHeading->Down) SetHeading(0.0);
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnFixCentClick(TObject *Sender)
{
    FixCent=BtnFixCent->Down;
    if (FixCent) SetCent(Lat,Lon);
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnEnaAltClick(TObject *Sender)
{
    UpdateOpts();
    Plot->Refresh_GEView();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnGENormClick(TObject *Sender)
{
    ExecFunc("SetTilt(0.0)");
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnGETiltClick(TObject *Sender)
{
    AnsiString f;
    ExecFunc(f.sprintf("SetTilt(%.1f)",TILT_ANGLE));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnShrinkMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Timer2->Enabled=true;
    Expand=1;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnShrinkMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Expand=0;
    Timer2->Enabled=false;
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnExpandMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Timer2->Enabled=true;
    Expand=-1;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnExpandMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Expand=0;
    Timer2->Enabled=false;
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnRotLMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Timer2->Enabled=true;
    Rotate=1;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnRotLMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Rotate=0;
    Timer2->Enabled=false;
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnRotRMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Timer2->Enabled=true;
    Rotate=-1;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnRotRMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
    Rotate=0;
    Timer2->Enabled=false;
    ExecFunc("UpdateState()");
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::Panel2Gesture(TObject *Sender,
	const TGestureEventInfo &EventInfo, bool &Handled)
{
	;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::BtnOptClick(TObject *Sender)
{
	Panel8->Visible=!Panel8->Visible;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::FormResize(TObject *Sender)
{
	if (FixCent) SetCent(Lat,Lon);
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::Timer2Timer(TObject *Sender)
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
void __fastcall TGoogleEarthView::Clear(void)
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
void __fastcall TGoogleEarthView::SetView(double lat, double lon, double range,
    double heading)
{
    AnsiString f;
    if (range<=0.0) range=INIT_RANGE;
    Lat=lat;
    Lon=lon;
    Range=range;
    Heading=heading;
    ExecFunc(f.sprintf("SetView(%.9f,%.9f,%.3f,%.1f)",lat,lon,range,heading));
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::SetCent(double lat, double lon)
{
    AnsiString f;
    Lat=lat;
    Lon=lon;
    if (FixCent) ExecFunc(f.sprintf("SetCent(%.9f,%.9f)",lat,lon));
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::SetRange(double range)
{
    AnsiString f;
    if (range<=0.0) range=INIT_RANGE;
    Range=range;
    ExecFunc(f.sprintf("SetRange(%.3f)",range));
}
/// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::SetHeading(double angle)
{
    AnsiString f;
    Heading=angle;
    ExecFunc(f.sprintf("SetHeading(%.2f)",angle));
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::SetMark(int index, const double *pos)
{
    AnsiString f;
    if (index<1||2<index) return;
    ExecFunc(f.sprintf("SetMark(%d,%.9f,%.9f,%.3f)",index,pos[0]*R2D,
             pos[1]*R2D,pos[2]));
    MarkPos[index-1][0]=pos[0]*R2D;
    MarkPos[index-1][1]=pos[1]*R2D;
    SetCent(Lat,Lon);
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::ShowMark(int index)
{
    AnsiString f;
    if (index<1||2<index) return;
    ExecFunc(f.sprintf("ShowMark(%d)",index));
    MarkVis[index-1]=1;
    UpdateEnable();
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::HideMark(int index)
{
    AnsiString f;
    if (index<1||2<index) return;
    ExecFunc(f.sprintf("HideMark(%d)",index));
    MarkVis[index-1]=0;
    UpdateEnable();
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::ClearTrack(int index)
{
    AnsiString f;
    if (index<1||2<index) return;
    ExecFunc(f.sprintf("ClearTrack(%d)",index));
    TrackVis[index-1]=0;
    UpdateEnable();
}
// --------------------------------------------------------------------------
int __fastcall TGoogleEarthView::UpdateTrack(int index, solbuf_t *sol)
{
    AnsiString f;
    sol_t *data;
    double prev[3]={0},pos[3];
    int i,j,intv;
    
    if (index<1||2<index||!State||sol->n<=0) return 0;
    
    Screen->Cursor=crHourGlass;
    
    ClearTrack(index);
    
    intv=sol->n/MAXTRACKS+1; // interval to reduce points
    
    for (i=0;data=getsol(sol,i);i++) {
        if (i%intv!=0) continue;
        ecef2pos(data->rr,pos);
        if (fabs(pos[0]-prev[0])<1E-8&&fabs(pos[1]-prev[1])<1E-8) continue;
        prev[0]=pos[0];
        prev[1]=pos[1];
        ExecFunc(f.sprintf("AddTrack(%d,%.9f,%.9f)",index,pos[0]*R2D,
                 pos[1]*R2D));
    }
    Screen->Cursor=crDefault;
    UpdateEnable();
    return 1;
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::ShowTrack(int index)
{
    AnsiString f;
    if (index<1||2<index) return;
    ExecFunc(f.sprintf("ShowTrack(%d)",index));
    TrackVis[index-1]=1;
    UpdateEnable();
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::HideTrack(int index)
{
    AnsiString f;
    if (index<1||2<index) return;
    ExecFunc(f.sprintf("HideTrack(%d)",index));
    TrackVis[index-1]=0;
    UpdateEnable();
}
// ----------------------------------------------------------------------------
void __fastcall TGoogleEarthView::UpdatePoint(void)
{
    AnsiString f;
    double pos[3];
    int i;
    
    ExecFunc("ClearPoint()");
    
    for (i=0;i<Plot->NWayPnt;i++) {
        ecef2pos(Plot->PntPos[i],pos);
        ExecFunc(f.sprintf("AddPoint('%s',%.9f,%.9f,%.2f)",Plot->PntName[i],
                 pos[0]*R2D,pos[1]*R2D,pos[2]));
    }
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::ShowPoint(void)
{
    ExecFunc("ShowPoint()");
}
// --------------------------------------------------------------------------
void __fastcall TGoogleEarthView::HidePoint(void)
{
    ExecFunc("HidePoint()");
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::SetOpts(const int *opts)
{
    TSpeedButton *btn[]={
        BtnOpt1,BtnOpt2,BtnOpt3,BtnOpt4,BtnOpt5,BtnOpt6,BtnOpt7,BtnOpt8,
        BtnOpt9,BtnEnaAlt,BtnHeading
    };
    for (int i=0;i<11;i++) {
        btn[i]->Down=opts[i];
    }
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::GetOpts(int *opts)
{
    TSpeedButton *btn[]={
        BtnOpt1,BtnOpt2,BtnOpt3,BtnOpt4,BtnOpt5,BtnOpt6,BtnOpt7,BtnOpt8,
        BtnOpt9,BtnEnaAlt,BtnHeading
    };
    for (int i=0;i<11;i++) {
        opts[i]=btn[i]->Down;
    }
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::UpdateOpts(void)
{
    AnsiString f;
    int opts[12];
    
    GetOpts(opts);
    ExecFunc(f.sprintf("SetOpts(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",opts[0],
             opts[1],opts[2],opts[3],opts[4],opts[5],opts[6],opts[7],opts[8],
             opts[9]));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::UpdateEnable(void)
{
    BtnEnaAlt ->Enabled=MarkVis[0]||MarkVis[1];
    BtnRotR   ->Enabled=!BtnHeading->Down;
    BtnRotL   ->Enabled=!BtnHeading->Down;
}
//---------------------------------------------------------------------------
int __fastcall TGoogleEarthView::GetState(void)
{
    IHTMLDocument3 *doc=NULL;
    IHTMLElement *ele1=NULL;
    VARIANT var;
    int state=0;
    
    if (!WebBrowser->Document) return 0;
    WebBrowser->Document->QueryInterface(IID_IHTMLDocument3,(void **)&doc);
    if (!doc) return 0;
    doc->getElementById(L"state",&ele1);
    doc->Release();
    if (!ele1) return 0;
    
    VariantInit(&var);
    if (ele1->getAttribute(L"value",0,&var)!=S_OK) {
        VariantClear(&var);
        return 0;
    }
    swscanf(var.bstrVal,L"%d",&state);
    VariantClear(&var);
    return state;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleEarthView::ExecFunc(AnsiString func)
{
    IHTMLWindow2 *win;
    IHTMLDocument2 *doc=NULL;
    VARIANT var;
    HRESULT hr;
    wchar_t func_w[256]={0};
    
    if (!State||!WebBrowser->Document) return;
    WebBrowser->Document->QueryInterface(IID_IHTMLDocument2,(void **)&doc);
    if (!doc) return;
    hr=doc->get_parentWindow(&win);
    doc->Release();
    if (hr!=S_OK) return;
    
    VariantInit(&var);
    ::MultiByteToWideChar(CP_UTF8,0,func.c_str(),-1,func_w,512);
    hr=win->execScript(func_w,L"javascript",&var);
    VariantClear(&var);
#if 1 // for debug
    trace(2,"GE: %s\n",func.c_str());
#endif
}
//---------------------------------------------------------------------------


