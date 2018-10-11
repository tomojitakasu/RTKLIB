//---------------------------------------------------------------------------
// gmview.c: google map view
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <mshtml.h>
#include "rtklib.h"
#include "gmview.h"
#include "plotmain.h"

#define RTKLIB_GM_TEMP "rtkplot_gm.htm"
#define RTKLIB_GM_FILE "rtkplot_gm_a.htm"
#define URL_GM_API     "http://maps.google.com/maps/api/js"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SHDocVw_OCX"
#pragma resource "*.dfm"

TGoogleMapView *GoogleMapView;
//---------------------------------------------------------------------------
__fastcall TGoogleMapView::TGoogleMapView(TComponent* Owner)
    : TForm(Owner)
{
	State=0;
	Lat=Lon=0.0;
	Zoom=2;
    FixCent=1;
    WebCreate=0;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::FormShow(TObject *Sender)
{
    UnicodeString url;
    AnsiString exe,dir=".",infile,outfile;
    FILE *infp, *outfp;
    char *p,*q,*key=Plot->ApiKey.c_str(),buff[1024];
    
    if (WebCreate) {
        return;
    }
    exe=Application->ExeName; // exe directory
    p=exe.c_str();
    if ((q=strrchr(p,'\\'))) {
        dir=exe.SubString(1,q-p);
    }
    infile=dir+"\\"+RTKLIB_GM_TEMP;
    outfile=dir+"\\"+RTKLIB_GM_FILE;
    
    if (!(infp=fopen(infile.c_str(),"r"))) {
        return;
    }
    if (!(outfp=fopen(outfile.c_str(),"w"))) {
        fclose(infp);
        return;
    }
    while (fgets(buff,sizeof(buff),infp)) {
        
        if (*key&&(p=strstr(buff,URL_GM_API))) {
            p+=strlen(URL_GM_API);
            *p++='\0';
            fprintf(outfp,"%s?key=%s&%s",buff,key,p);
        }
        else {
            fputs(buff,outfp);
        }
    }
    fclose(infp);
    fclose(outfp);
    
    url="file://"+outfile;
    
    WebBrowser->Navigate(url.c_str());
    
    Timer1->Enabled=true;
    
    WebCreate=1;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::Timer1Timer(TObject *Sender)
{
	if (!GetState()) return;
	
	State=1;
	SetView(Lat,Lon,Zoom);
	AddMark(0.0,0.0,"SOL1","SOLUTION 1");
	AddMark(0.0,0.0,"SOL2","SOLUTION 2");
	HideMark(1);
	HideMark(2);
	for (int i=0;i<2;i++) MarkPos[i][0]=MarkPos[i][1]=0.0;
	Timer1->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::BtnCloseClick(TObject *Sender)
{
trace(2,"gmview close\n");
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::BtnShrinkClick(TObject *Sender)
{
	SetZoom(Zoom-1);
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::BtnExpandClick(TObject *Sender)
{
	SetZoom(Zoom+1);
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::BtnFixCentClick(TObject *Sender)
{
    FixCent=BtnFixCent->Down;
	if (FixCent) SetCent(Lat,Lon);
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::FormResize(TObject *Sender)
{
	if (FixCent) SetCent(Lat,Lon);
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::SetView(double lat, double lon, int zoom)
{
    AnsiString f;
	Lat=lat;
	Lon=lon;
	Zoom=zoom;
    ExecFunc(f.sprintf("SetView(%.9f,%.9f,%d)",lat,lon,zoom));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::SetCent(double lat, double lon)
{
    AnsiString f;
    Lat=lat;
    Lon=lon;
    if (FixCent) ExecFunc(f.sprintf("SetCent(%.9f,%.9f)",lat,lon));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::SetZoom(int zoom)
{
    AnsiString f;
    if (zoom<2||zoom>21) return;
	Zoom=zoom;
    ExecFunc(f.sprintf("SetZoom(%d)",zoom));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::ClearMark(void)
{
    ExecFunc("ClearMark()");
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::AddMark(double lat, double lon,
    AnsiString title, AnsiString msg)
{
    AnsiString f;
    ExecFunc(f.sprintf("AddMark(%.9f,%.9f,\"%s\",\"%s\")",lat,lon,title,msg));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::SetMark(int index, const double *pos)
{
    AnsiString f,title;
    title.sprintf("SOL%d",index);
    ExecFunc(f.sprintf("PosMark(%.9f,%.9f,\"%s\")",pos[0]*R2D,pos[1]*R2D,title));
	MarkPos[index-1][0]=pos[0]*R2D;
	MarkPos[index-1][1]=pos[1]*R2D;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::ShowMark(int index)
{
    AnsiString f,title;
    title.sprintf("SOL%d",index);
    ExecFunc(f.sprintf("ShowMark(\"%s\")",title));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::HideMark(int index)
{
    AnsiString f,title;
    title.sprintf("SOL%d",index);
    ExecFunc(f.sprintf("HideMark(\"%s\")",title));
}
//---------------------------------------------------------------------------
int __fastcall TGoogleMapView::GetState(void)
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
void __fastcall TGoogleMapView::ExecFunc(AnsiString func)
{
    IHTMLWindow2 *win;
    IHTMLDocument2 *doc=NULL;
    VARIANT var;
    HRESULT hr;
    wchar_t func_w[1024]={0};
    
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
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::FormCreate(TObject *Sender)
{
}
//---------------------------------------------------------------------------

