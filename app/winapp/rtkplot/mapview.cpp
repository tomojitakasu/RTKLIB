//---------------------------------------------------------------------------
// mapview.c: map view
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <mshtml.h>
#include "rtklib.h"
#include "mapview.h"
#include "mapviewopt.h"
#include "plotmain.h"

#define RTKLIB_GM_TEMP "rtkplot_gm.htm"
#define RTKLIB_GM_FILE "rtkplot_gm_a.htm"
#define RTKLIB_LL_TEMP "rtkplot_ll.htm"
#define RTKLIB_LL_FILE "rtkplot_ll_a.htm"
#define URL_GM_API     "http://maps.google.com/maps/api/js"
#define MAP_OPACITY    0.8
#define INIT_ZOOM      12  // initial zoom level

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SHDocVw_OCX"
#pragma resource "*.dfm"

TMapView *MapView;
//---------------------------------------------------------------------------
__fastcall TMapView::TMapView(TComponent* Owner)
    : TForm(Owner)
{
    MapSel=0;
	Lat=Lon=0.0;
	for (int i=0;i<2;i++) {
        MapState[0]=MapState[1]=0;
        MarkState[0]=MarkState[1]=0;
        MarkPos[i][0]=MarkPos[i][1]=0.0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapView::FormShow(TObject *Sender)
{
    MapSel1->Checked=!MapSel;
    MapSel2->Checked=MapSel;
    SelectMap(MapSel);
    ShowMap(MapSel);
}
//---------------------------------------------------------------------------
void __fastcall TMapView::BtnCloseClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMapView::MapSel1Click(TObject *Sender)
{
    SelectMap(0);
}
//---------------------------------------------------------------------------
void __fastcall TMapView::MapSel2Click(TObject *Sender)
{
    SelectMap(1);
}
//---------------------------------------------------------------------------
void __fastcall TMapView::BtnOptClick(TObject *Sender)
{
    MapViewOptDialog->Top=Top+Height/2-MapViewOptDialog->Height/2;
    MapViewOptDialog->Left=Left+Width/2-MapViewOptDialog->Width/2;
    
    MapViewOptDialog->ApiKey=Plot->ApiKey;
    for (int i=0;i<6;i++) for (int j=0;j<3;j++) {
        MapViewOptDialog->MapStrs[i][j]=Plot->MapStrs[i][j];
    }
    if (MapViewOptDialog->ShowModal()!=mrOk) return;
    
    Plot->ApiKey=MapViewOptDialog->ApiKey;
    for (int i=0;i<6;i++) for (int j=0;j<3;j++) {
        Plot->MapStrs[i][j]=MapViewOptDialog->MapStrs[i][j];
    }
    MapState[0]=MapState[1]=0;
    ShowMap(MapSel);
}
//---------------------------------------------------------------------------
void __fastcall TMapView::BtnZoomOutClick(TObject *Sender)
{
	ExecFunc(MapSel,"ZoomOut()");
}
//---------------------------------------------------------------------------
void __fastcall TMapView::BtnZoomInClick(TObject *Sender)
{
	ExecFunc(MapSel,"ZoomIn()");
}
//---------------------------------------------------------------------------
void __fastcall TMapView::BtnSyncClick(TObject *Sender)
{
	if (BtnSync->Down) {
		SetCent(Lat,Lon);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMapView::FormResize(TObject *Sender)
{
	if (BtnSync->Down) {
        SetCent(Lat,Lon);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMapView::ShowMap(int map)
{
    if (!map&&!MapState[0]) {
        ShowMapLL();
    }
    else if (map&&!MapState[1]) {
        ShowMapGM();
    }
    else {
        UpdateMap();
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapView::ShowMapLL(void)
{
    AnsiString exe,dir=".",ifile,ofile;
    FILE *ifp,*ofp;
    char *p,*q,buff[1024];
    int i,j;
    
    exe=Application->ExeName; // exe directory
    p=exe.c_str();
    if ((q=strrchr(p,'\\'))) {
        dir=exe.SubString(1,q-p);
    }
    ifile=dir+"\\"+RTKLIB_LL_TEMP;
    ofile=dir+"\\"+RTKLIB_LL_FILE;
    
    if (!(ifp=fopen(ifile.c_str(),"r"))) {
        return;
    }
    if (!(ofp=fopen(ofile.c_str(),"w"))) {
        fclose(ifp);
        return;
    }
    while (fgets(buff,sizeof(buff),ifp)) {
        fputs(buff,ofp);
        if (!strstr(buff,"// start map tiles")) continue;
        for (i=0,j=1;i<6;i++) {
            if (Plot->MapStrs[i][0]=="") continue;
            UTF8String title=Plot->MapStrs[i][0];
            UTF8String url  =Plot->MapStrs[i][1];
            UTF8String attr =Plot->MapStrs[i][2];

            fprintf(ofp,"var tile%d = L.tileLayer('%s', {\n",j,url.c_str());
            fprintf(ofp,"  attribution: \"<a href='%s' target='_blank'>%s</a>\",\n",
                    attr.c_str(),title.c_str());
            fprintf(ofp,"  opacity: %.1f});\n",MAP_OPACITY);
            j++;
        }
        fprintf(ofp,"var basemaps = {");
        for (i=0,j=1;i<6;i++) {
            if (Plot->MapStrs[i][0]=="") continue;
            UTF8String title=Plot->MapStrs[i][0];
            fprintf(ofp,"%s\"%s\":tile%d",(j==1)?"":",",title.c_str(),j);
            j++;
        }
        fprintf(ofp,"};\n");
    }
    fclose(ifp);
    fclose(ofp);

    UnicodeString url="file://"+ofile;
    Browser1->Navigate(url.c_str());
    Timer1->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TMapView::Timer1Timer(TObject *Sender)
{
    if (!GetState(0)) return;
    MapState[0]=1;
    SetView(0,Lat,Lon,INIT_ZOOM);
	AddMark(0,1,MarkPos[0][0],MarkPos[0][1],MarkState[0]);
	AddMark(0,2,MarkPos[1][0],MarkPos[1][1],MarkState[1]);
	Timer1->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TMapView::ShowMapGM(void)
{
    AnsiString exe,dir=".",ifile,ofile;
    FILE *ifp, *ofp;
    char *p,*q,*key=Plot->ApiKey.c_str(),buff[1024];
    
    exe=Application->ExeName; // exe directory
    p=exe.c_str();
    if ((q=strrchr(p,'\\'))) {
        dir=exe.SubString(1,q-p);
    }
    ifile=dir+"\\"+RTKLIB_GM_TEMP;
    ofile=dir+"\\"+RTKLIB_GM_FILE;
    
    if (!(ifp=fopen(ifile.c_str(),"r"))) {
        return;
    }
    if (!(ofp=fopen(ofile.c_str(),"w"))) {
        fclose(ifp);
        return;
    }
    while (fgets(buff,sizeof(buff),ifp)) {
        if (*key&&(p=strstr(buff,URL_GM_API))) {
            p+=strlen(URL_GM_API);
            *p++='\0';
            fprintf(ofp,"%s?key=%s&%s",buff,key,p);
        }
        else {
            fputs(buff,ofp);
        }
    }
    fclose(ifp);
    fclose(ofp);
     
    UnicodeString url="file://"+ofile;
    Browser2->Navigate(url.c_str());
    Timer2->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TMapView::Timer2Timer(TObject *Sender)
{
    if (!GetState(1)) return;
    MapState[1]=1;
    SetView(1,Lat,Lon,INIT_ZOOM);
	AddMark(1,1,MarkPos[0][0],MarkPos[0][1],MarkState[0]);
	AddMark(1,2,MarkPos[1][0],MarkPos[1][1],MarkState[1]);
	Timer2->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TMapView::SetView(int map, double lat, double lon, int zoom)
{
    UTF8String func;

    ExecFunc(map,func.sprintf("SetView(%.9f,%.9f,%d)",lat,lon,zoom));
}
//---------------------------------------------------------------------------
void __fastcall TMapView::AddMark(int map, int index, double lat, double lon,
    int state)
{
    UTF8String func;

    func.sprintf("AddMark(%.9f,%.9f,'SOL%d','SOLUTION %d')",lat,lon,index,index);
    ExecFunc(map,func);
    if (state) func.sprintf("ShowMark('SOL%d')",index);
    else       func.sprintf("HideMark('SOL%d')",index);
    ExecFunc(map,func);
}
//---------------------------------------------------------------------------
void __fastcall TMapView::UpdateMap(void)
{
    SetCent(Lat,Lon);
    for (int i=0;i<2;i++) {
        SetMark(i+1,MarkPos[i][0],MarkPos[i][1]);
        if (MarkState[i]) ShowMark(i+1); else HideMark(i+1);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapView::SelectMap(int map)
{
    if (!map) {
        Panel22->Visible=false;
        Panel21->Visible=true;
    }
    else {
        Panel21->Visible=false;
        Panel22->Visible=true;
    }
    MapSel=map;
    ShowMap(map);
}
//---------------------------------------------------------------------------
void __fastcall TMapView::SetCent(double lat, double lon)
{
    UTF8String func;
    
    Lat=lat;
    Lon=lon;
    if (BtnSync->Down) {
        ExecFunc(MapSel,func.sprintf("SetCent(%.9f,%.9f)",lat,lon));
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapView::SetMark(int index, double lat, double lon)
{
    UTF8String func;
        
    MarkPos[index-1][0]=lat;
    MarkPos[index-1][1]=lon;
    func.sprintf("PosMark(%.9f,%.9f,'SOL%d')",lat,lon,index);
    ExecFunc(MapSel,func);
}
//---------------------------------------------------------------------------
void __fastcall TMapView::ShowMark(int index)
{
    UTF8String func;
     
    MarkState[index-1]=1;
    ExecFunc(MapSel,func.sprintf("ShowMark('SOL%d')",index));
}
//---------------------------------------------------------------------------
void __fastcall TMapView::HideMark(int index)
{
    UTF8String func;
    
	MarkState[index-1]=0;
    ExecFunc(MapSel,func.sprintf("HideMark('SOL%d')",index));
}
//---------------------------------------------------------------------------
int __fastcall TMapView::GetState(int map)
{
    TCppWebBrowser *browser[]={Browser1,Browser2};
	IHTMLDocument3 *doc=NULL;
	IHTMLElement *ele1=NULL;
	VARIANT var;
	int state=0;
	
    if (!browser[map]->Document) return 0;
	browser[map]->Document->QueryInterface(IID_IHTMLDocument3,(void **)&doc);
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
int __fastcall TMapView::ExecFunc(int map, UTF8String func)
{
    TCppWebBrowser *browser[]={Browser1,Browser2};
    IHTMLWindow2 *win;
    IHTMLDocument2 *doc=NULL;
    VARIANT var;
    HRESULT hr;
    wchar_t func_w[1024]={0};
    
    if (!browser[map]->Document) return 0;
    browser[map]->Document->QueryInterface(IID_IHTMLDocument2,(void **)&doc);
    if (!doc) return 0;
    hr=doc->get_parentWindow(&win);
    doc->Release();
    if (hr!=S_OK) return 0;
    
    VariantInit(&var);
    ::MultiByteToWideChar(CP_UTF8,0,func.c_str(),-1,func_w,512); 
    BSTR bstr1=SysAllocString(func_w);
    BSTR bstr2=SysAllocString(L"javascript");
    hr=win->execScript(bstr1,bstr2,&var);
    SysFreeString(bstr2);
    VariantClear(&var);
    return 1;
}
//---------------------------------------------------------------------------


