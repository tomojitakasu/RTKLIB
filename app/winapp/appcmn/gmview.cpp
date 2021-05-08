//---------------------------------------------------------------------------
// gmview.c: map view
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <mshtml.h>
#include "gmview.h"

#define RTKLIB_GM      "rtklib_gmap.htm"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TGoogleMapView *GoogleMapView;
//---------------------------------------------------------------------------
__fastcall TGoogleMapView::TGoogleMapView(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::FormShow(TObject *Sender)
{
    UnicodeString url;
    AnsiString exe,dir=".",file;
    char *p,*q;
    
    exe=Application->ExeName; // exe directory
    p=exe.c_str();
    if ((q=strrchr(p,'\\'))) {
        dir=exe.SubString(1,q-p);
	}
    url="file://"+dir+"\\"+RTKLIB_GM;
    
    WebBrowser->Navigate(url.c_str());
}
//---------------------------------------------------------------------------
int __fastcall TGoogleMapView::GetState(void)
{
	IHTMLDocument3 *doc=NULL;
	IHTMLElement *ele1=NULL;
	VARIANT var;
	int state;
	
	if (!WebBrowser->Document) return 0;
	WebBrowser->Document->QueryInterface(IID_IHTMLDocument3,(void **)&doc);
	if (!doc) return 0;
    BSTR bstr1=SysAllocString(L"state");
	doc->getElementById(bstr1,&ele1);
    SysFreeString(bstr1);
	doc->Release();
	if (!ele1) return 0;
	
	VariantInit(&var);
    BSTR bstr2=SysAllocString(L"value");
	if (ele1->getAttribute(bstr2,0,&var)!=S_OK) {
        SysFreeString(bstr2);
		VariantClear(&var);
		return 0;
	}
    SysFreeString(bstr2);
	swscanf(var.bstrVal,L"%d",&state);
	VariantClear(&var);
	return state;
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::BtnCloseClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::BtnHomeClick(TObject *Sender)
{
	ShowHome();
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::ShowHome(void)
{
    ExecFunc("ShowHome()");
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
    if (lon>=180.0) lon-=360.0;
    ExecFunc(f.sprintf("AddMark(%.7f,%.7f,\"%s\",\"%s\")",lat,lon,title.c_str(),msg.c_str()));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::PosMark(double lat, double lon,
    AnsiString title)
{
    AnsiString f;
    ExecFunc(f.sprintf("PosMark(%.7f,%.7f,\"%s\")",lat,lon,title.c_str()));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::HighlightMark(AnsiString title)
{
    AnsiString f;
    ExecFunc(f.sprintf("HighlightMark(\"%s\")",title.c_str()));
}
//---------------------------------------------------------------------------
void __fastcall TGoogleMapView::ExecFunc(AnsiString func)
{
    IHTMLWindow2 *win;
    IHTMLDocument2 *doc=NULL;
    VARIANT var;
    HRESULT hr;
    wchar_t func_w[1024]={0};
    
    if (!WebBrowser->Document) return;
    WebBrowser->Document->QueryInterface(IID_IHTMLDocument2,(void **)&doc);
    if (!doc) return;
    hr=doc->get_parentWindow(&win);
    doc->Release();
    if (hr!=S_OK) return;
    
    VariantInit(&var);
    ::MultiByteToWideChar(CP_UTF8,0,func.c_str(),-1,func_w,512); 
    BSTR bstr1=SysAllocString(func_w);
    BSTR bstr2=SysAllocString(L"javascript");
    hr=win->execScript(bstr1,bstr2,&var);
    SysFreeString(bstr1);
    SysFreeString(bstr2);
    VariantClear(&var);
}
//---------------------------------------------------------------------------

