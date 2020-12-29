//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "plotmain.h"
#include "vmapdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TVecMapDialog *VecMapDialog;

//---------------------------------------------------------------------------
__fastcall TVecMapDialog::TVecMapDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::Layer1Click(TObject *Sender)
{
	TRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
		layer[i]->Checked=(layer[i]==(TRadioButton *)Sender);
	}
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnUpClick(TObject *Sender)
{
	TRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
	TPanel *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
	TPanel *colorf[]={
		Color1F,Color2F,Color3F,Color4F,Color5F,Color6F,Color7F,Color8F,
		Color9F,Color10F,Color11F,Color12F
	};
	TColor col;
	gisd_t *data;
	char name[256];
	int i,flag;
	
	for (i=0;i<MAXMAPLAYER;i++) {
		if (layer[i]->Checked) break;
	}
	if (i==0||i>=MAXMAPLAYER) return;
	strcpy(name,Gis.name[i-1]);
	strcpy(Gis.name[i-1],Gis.name[i]);
	strcpy(Gis.name[i],name);
	flag=Gis.flag[i-1];
	Gis.flag[i-1]=Gis.flag[i];
	Gis.flag[i]=flag;
	data=Gis.data[i-1];
	Gis.data[i-1]=Gis.data[i];
	Gis.data[i]=data;
	col=color[i-1]->Color;
	color[i-1]->Color=color[i]->Color;
	color[i]->Color=col;
	col=colorf[i-1]->Color;
	colorf[i-1]->Color=colorf[i]->Color;
	colorf[i]->Color=col;
	layer[i-1]->Checked=true;
	layer[i]->Checked=false;
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnDownClick(TObject *Sender)
{
	TRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
	TPanel *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
	TPanel *colorf[]={
		Color1F,Color2F,Color3F,Color4F,Color5F,Color6F,Color7F,Color8F,
		Color9F,Color10F,Color11F,Color12F
	};
	TColor col;
	gisd_t *data;
	char name[256];
	int i,flag;
	
	for (i=0;i<MAXMAPLAYER;i++) {
		if (layer[i]->Checked) break;
	}
	if (i==MAXMAPLAYER-1||i>=MAXMAPLAYER) return;
	strcpy(name,Gis.name[i+1]);
	strcpy(Gis.name[i+1],Gis.name[i]);
	strcpy(Gis.name[i],name);
	flag=Gis.flag[i+1];
	Gis.flag[i+1]=Gis.flag[i];
	Gis.flag[i]=flag;
	data=Gis.data[i+1];
	Gis.data[i+1]=Gis.data[i];
	Gis.data[i]=data;
	col=color[i+1]->Color;
	color[i+1]->Color=color[i]->Color;
	color[i]->Color=col;
	col=colorf[i+1]->Color;
	colorf[i+1]->Color=colorf[i]->Color;
	colorf[i]->Color=col;
	layer[i+1]->Checked=true;
	layer[i]->Checked=false;
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnDeleteClick(TObject *Sender)
{
	;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::Vis1Click(TObject *Sender)
{
	TCheckBox *vis[]={
		Vis1,Vis2,Vis3,Vis4,Vis5,Vis6,Vis7,Vis8,Vis9,Vis10,Vis11,Vis12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
		if ((TCheckBox *)Sender==vis[i]) Gis.flag[i]=vis[i]->Checked?1:0;
	}
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::FormShow(TObject *Sender)
{
	TRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
	TPanel *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
	TPanel *colorf[]={
		Color1F,Color2F,Color3F,Color4F,Color5F,Color6F,Color7F,Color8F,
		Color9F,Color10F,Color11F,Color12F
	};
	Gis=Plot->Gis;
	for (int i=0;i<MAXMAPLAYER;i++) {
		layer[i]->Checked=false;
		color [i]->Color=Plot->MapColor [i];
		colorf[i]->Color=Plot->MapColorF[i];
	}
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnApplyClick(TObject *Sender)
{
	UpdateMap();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::ColorClick(TObject *Sender)
{
	TPanel *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12,Color1F,Color2F,Color3F,Color4F,Color5F,
		Color6F,Color7F,Color8F,Color9F,Color10F,Color11F,Color12F,NULL
	};
	int i;

	for (i=0;color[i];i++) {
		if (color[i]==(TPanel *)Sender) break;
	}
	if (color[i]) {
        ColorDialog->Color=color[i]->Color;
        if (!ColorDialog->Execute()) return;
        color[i]->Color=ColorDialog->Color;
	    UpdateMap();
	}
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::UpdateMap(void)
{
	TRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
	TCheckBox *vis[]={
		Vis1,Vis2,Vis3,Vis4,Vis5,Vis6,Vis7,Vis8,Vis9,Vis10,Vis11,Vis12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
	}
	TPanel *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
	TPanel *colorf[]={
		Color1F,Color2F,Color3F,Color4F,Color5F,Color6F,Color7F,Color8F,
		Color9F,Color10F,Color11F,Color12F
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
		layer[i]->Caption=Gis.name[i];
		vis[i]->Checked=Gis.flag[i];
		Plot->MapColor [i]=color [i]->Color;
		Plot->MapColorF[i]=colorf[i]->Color;
	}
	Plot->Gis=Gis;
	Plot->UpdatePlot();
}
//---------------------------------------------------------------------------

