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
void __fastcall TVecMapDialog::BtnColor1Click(TObject *Sender)
{
    ColorDialog->Color=Color1->Color;
    if (!ColorDialog->Execute()) return;
    Color1->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor2Click(TObject *Sender)
{
    ColorDialog->Color=Color2->Color;
    if (!ColorDialog->Execute()) return;
    Color2->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor3Click(TObject *Sender)
{
    ColorDialog->Color=Color3->Color;
    if (!ColorDialog->Execute()) return;
    Color3->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor4Click(TObject *Sender)
{
    ColorDialog->Color=Color4->Color;
    if (!ColorDialog->Execute()) return;
    Color4->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor5Click(TObject *Sender)
{
    ColorDialog->Color=Color5->Color;
    if (!ColorDialog->Execute()) return;
    Color5->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor6Click(TObject *Sender)
{
    ColorDialog->Color=Color6->Color;
    if (!ColorDialog->Execute()) return;
    Color6->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor7Click(TObject *Sender)
{
    ColorDialog->Color=Color7->Color;
    if (!ColorDialog->Execute()) return;
    Color7->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor8Click(TObject *Sender)
{
    ColorDialog->Color=Color8->Color;
    if (!ColorDialog->Execute()) return;
    Color8->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor9Click(TObject *Sender)
{
    ColorDialog->Color=Color9->Color;
    if (!ColorDialog->Execute()) return;
    Color9->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor10Click(TObject *Sender)
{
    ColorDialog->Color=Color10->Color;
    if (!ColorDialog->Execute()) return;
    Color10->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor11Click(TObject *Sender)
{
    ColorDialog->Color=Color11->Color;
    if (!ColorDialog->Execute()) return;
    Color11->Color=ColorDialog->Color;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnColor12Click(TObject *Sender)
{
    ColorDialog->Color=Color12->Color;
    if (!ColorDialog->Execute()) return;
    Color12->Color=ColorDialog->Color;
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
	//col=color[i-1]->Color;
	//color[i-1]->Color=color[i]->Color;
	//color[i]->Color=col;
	layer[i-1]->Checked=true;
	layer[i]->Checked=false;
	UpdateLayer();
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
	//col=color[i+1]->Color;
	//color[i+1]->Color=color[i]->Color;
	//color[i]->Color=col;
	layer[i+1]->Checked=true;
	layer[i]->Checked=false;
	UpdateLayer();
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
	Gis=Plot->Gis;
	for (int i=0;i<MAXMAPLAYER;i++) {
		layer[i]->Checked=false;
		color[i]->Color=Plot->MapColor[i];
	}
	UpdateLayer();
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::BtnOkClick(TObject *Sender)
{
	TPanel *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
		Plot->MapColor[i]=color[i]->Color;
	}
	Plot->Gis=Gis;
}
//---------------------------------------------------------------------------
void __fastcall TVecMapDialog::UpdateLayer(void)
{
	TRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
	TCheckBox *vis[]={
		Vis1,Vis2,Vis3,Vis4,Vis5,Vis6,Vis7,Vis8,Vis9,Vis10,Vis11,Vis12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
		layer[i]->Caption=Gis.name[i];
		vis[i]->Checked=Gis.flag[i];
	}
}
//---------------------------------------------------------------------------

