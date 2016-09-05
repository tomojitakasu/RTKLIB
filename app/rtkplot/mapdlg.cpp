//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include "rtklib.h"
#pragma hdrstop

#include "plotmain.h"
#include "mapdlg.h"
#include "confdlg.h"

#define INC_LATLON  0.000001
#define INC_SCALE   0.0001

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMapAreaDialog *MapAreaDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TMapAreaDialog::TMapAreaDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::FormShow(TObject *Sender)
{
	UpdateField();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::BtnSaveClick(TObject *Sender)
{
	FILE *fp;
	AnsiString file=Plot->MapImageFile;
	if (file=="") return;
	file=file+".tag";
	if ((fp=fopen(file.c_str(),"r"))) {
		fclose(fp);
		ConfDialog->Label1->Caption="File exists. Overwrite it?";
		ConfDialog->Label2->Caption=file;
		if (ConfDialog->ShowModal()!=mrOk) return;
	}
	if (!(fp=fopen(file.c_str(),"w"))) return;
	fprintf(fp,"%% map image tag file: rtkplot %s %s\n\n",VER_RTKLIB,PATCH_LEVEL);
	fprintf(fp,"scalex  = %.6g\n",Plot->MapScaleX );
	fprintf(fp,"scaley  = %.6g\n",Plot->MapScaleEq?Plot->MapScaleX:Plot->MapScaleY);
	fprintf(fp,"scaleeq = %d\n"  ,Plot->MapScaleEq);
	fprintf(fp,"lat     = %.9g\n",Plot->MapLat    );
	fprintf(fp,"lon     = %.9g\n",Plot->MapLon    );
	fclose(fp);
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::BtnCenterClick(TObject *Sender)
{
	AnsiString s;
	double rr[3],pos[3];
	if (!Plot->GetCenterPos(rr)) return;
	ecef2pos(rr,pos);
	Lat->Text=s.sprintf("%.7f",pos[0]*R2D);
	Lon->Text=s.sprintf("%.7f",pos[1]*R2D);
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::BtnUpdateClick(TObject *Sender)
{
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::ScaleEqClick(TObject *Sender)
{
	UpdateMap();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::ScaleXUpDownChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double scale=str2dbl(ScaleX->Text);
	if (Direction==updUp) scale+=INC_SCALE; else scale-=INC_SCALE;
	ScaleX->Text=s.sprintf("%.5f",scale);
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::ScaleYUpDownChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double scale=str2dbl(ScaleY->Text);
	if (Direction==updUp) scale+=INC_SCALE; else scale-=INC_SCALE;
	ScaleY->Text=s.sprintf("%.5f",scale);
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::LatUpDownChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double lat=str2dbl(Lat->Text);
	if (Direction==updUp) lat+=INC_LATLON; else lat-=INC_LATLON;
	Lat->Text=s.sprintf("%.7f",lat);
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::LonUpDownChangingEx(TObject *Sender,
      bool &AllowChange, short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double lon=str2dbl(Lon->Text);
	if (Direction==updUp) lon+=INC_LATLON; else lon-=INC_LATLON;
	Lon->Text=s.sprintf("%.7f",lon);
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::ScaleXKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN||Key==VK_RIGHT||Key==VK_LEFT) {
        ScaleXUpDownChangingEx(Sender,allowchange,0,Key==VK_UP||Key==VK_RIGHT?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::ScaleYKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN||Key==VK_RIGHT||Key==VK_LEFT) {
        ScaleYUpDownChangingEx(Sender,allowchange,0,Key==VK_UP||Key==VK_RIGHT?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::LatKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN||Key==VK_RIGHT||Key==VK_LEFT) {
        LatUpDownChangingEx(Sender,allowchange,0,Key==VK_UP||Key==VK_RIGHT?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::LonKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
    bool allowchange;
    if (Key==VK_UP||Key==VK_DOWN||Key==VK_RIGHT||Key==VK_LEFT) {
        LonUpDownChangingEx(Sender,allowchange,0,Key==VK_UP||Key==VK_RIGHT?updUp:updDown);
        Key=0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::UpdateField(void)
{
	AnsiString s;
	Caption=Plot->MapImageFile;
	MapSize1->Text=s.sprintf("%d",Plot->MapSize[0]);
	MapSize2->Text=s.sprintf("%d",Plot->MapSize[1]);
	ScaleX->Text=s.sprintf("%.5f",Plot->MapScaleX);
	ScaleY->Text=s.sprintf("%.5f",Plot->MapScaleY);
	Lat->Text=s.sprintf("%.7f",Plot->MapLat);
	Lon->Text=s.sprintf("%.7f",Plot->MapLon);
	ScaleEq->Checked=Plot->MapScaleEq;
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::UpdateMap(void)
{
	Plot->MapScaleX=str2dbl(ScaleX->Text);
	Plot->MapScaleY=str2dbl(ScaleY->Text);
	Plot->MapLat=str2dbl(Lat->Text);
	Plot->MapLon=str2dbl(Lon->Text);
	Plot->MapScaleEq=ScaleEq->Checked;
	Plot->UpdatePlot();
}
//---------------------------------------------------------------------------
void __fastcall TMapAreaDialog::UpdateEnable(void)
{
	ScaleY      ->Enabled=!ScaleEq->Checked;
	ScaleYUpDown->Enabled=!ScaleEq->Checked;
} 
//---------------------------------------------------------------------------

