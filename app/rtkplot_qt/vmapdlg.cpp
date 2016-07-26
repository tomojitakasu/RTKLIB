//---------------------------------------------------------------------------

#include <QColorDialog>
#include <QShowEvent>

#include "vmapdlg.h"
#include "plotmain.h"

extern Plot *plot;

//---------------------------------------------------------------------------
QString color2String(const QColor &c);

//---------------------------------------------------------------------------
VecMapDialog::VecMapDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnColor1,SIGNAL(clicked(bool)),this,SLOT(BtnColor1Click()));
    connect(BtnColor2,SIGNAL(clicked(bool)),this,SLOT(BtnColor2Click()));
    connect(BtnColor3,SIGNAL(clicked(bool)),this,SLOT(BtnColor3Click()));
    connect(BtnColor4,SIGNAL(clicked(bool)),this,SLOT(BtnColor4Click()));
    connect(BtnColor5,SIGNAL(clicked(bool)),this,SLOT(BtnColor5Click()));
    connect(BtnColor6,SIGNAL(clicked(bool)),this,SLOT(BtnColor6Click()));
    connect(BtnColor7,SIGNAL(clicked(bool)),this,SLOT(BtnColor7Click()));
    connect(BtnColor8,SIGNAL(clicked(bool)),this,SLOT(BtnColor8Click()));
    connect(BtnColor9,SIGNAL(clicked(bool)),this,SLOT(BtnColor9Click()));
    connect(BtnColor10,SIGNAL(clicked(bool)),this,SLOT(BtnColor10Click()));
    connect(BtnColor11,SIGNAL(clicked(bool)),this,SLOT(BtnColor11Click()));
    connect(BtnColor12,SIGNAL(clicked(bool)),this,SLOT(BtnColor12Click()));
    connect(Vis1,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis2,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis3,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis4,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis5,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis6,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis7,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis8,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis9,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis10,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis11,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Vis12,SIGNAL(clicked(bool)),this,SLOT(VisClick()));
    connect(Layer1,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer2,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer3,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer4,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer5,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer6,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer7,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer8,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer9,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer10,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer11,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(Layer12,SIGNAL(clicked(bool)),this,SLOT(LayerClick()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnUp,SIGNAL(clicked(bool)),this,SLOT(BtnUpClick()));
    connect(BtnDown,SIGNAL(clicked(bool)),this,SLOT(BtnDownClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));

}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor1Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[0]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color1->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[0]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor2Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[1]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color2->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[1]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor3Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[2]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color3->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[2]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor4Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[3]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color4->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[3]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor5Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[4]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color5->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[4]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor6Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[5]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color6->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[5]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor7Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[6]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color7->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[6]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor8Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[7]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color8->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[7]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor9Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[8]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color9->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[8]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor10Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[9]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color10->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[9]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor11Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[10]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color11->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[10]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColor12Click()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(Colors[11]);

    if (dialog.exec()!=QDialog::Accepted) return;

    Color12->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(dialog.currentColor())));
    Colors[11]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void VecMapDialog::LayerClick()
{
    QRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
        layer[i]->setChecked((layer[i]==(QRadioButton *)sender()));
	}
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnUpClick()
{
    QRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
    QFrame *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
    QColor col;
	gisd_t *data;
	char name[256];
	int i,flag;
	
	for (i=0;i<MAXMAPLAYER;i++) {
        if (layer[i]->isChecked()) break;
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
    layer[i-1]->setChecked(true);
    layer[i]->setChecked(false);
	UpdateLayer();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnDownClick()
{
    QRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
    QFrame *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
    QColor col;
	gisd_t *data;
	char name[256];
	int i,flag;
	
	for (i=0;i<MAXMAPLAYER;i++) {
        if (layer[i]->isChecked()) break;
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
    layer[i+1]->setChecked(true);
    layer[i]->setChecked(false);
	UpdateLayer();
}
//---------------------------------------------------------------------------
void VecMapDialog::VisClick()
{
    QCheckBox *vis[]={
		Vis1,Vis2,Vis3,Vis4,Vis5,Vis6,Vis7,Vis8,Vis9,Vis10,Vis11,Vis12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
        if ((QCheckBox *)sender()==vis[i]) Gis.flag[i]=vis[i]->isChecked()?1:0;
	}
}
//---------------------------------------------------------------------------
void VecMapDialog::showEvent (QShowEvent *event)
{
    if (event->spontaneous()) return;

    QRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
    QFrame *color[]={
		Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
		Color10,Color11,Color12
	};
    Gis=plot->Gis;
	for (int i=0;i<MAXMAPLAYER;i++) {
        layer[i]->setChecked(false);
        color[i]->setStyleSheet(QString("QFrame {background-color: %1;}").arg(color2String(plot->MapColor[i])));
	}
	UpdateLayer();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnOkClick()
{
	for (int i=0;i<MAXMAPLAYER;i++) {
        plot->MapColor[i]=Colors[i];
	}

    plot->Gis=Gis;

    accept();
}
//---------------------------------------------------------------------------
void VecMapDialog::UpdateLayer(void)
{
    QRadioButton *layer[]={
		Layer1,Layer2,Layer3,Layer4,Layer5,Layer6,Layer7,Layer8,Layer9,
		Layer10,Layer11,Layer12
	};
    QCheckBox *vis[]={
		Vis1,Vis2,Vis3,Vis4,Vis5,Vis6,Vis7,Vis8,Vis9,Vis10,Vis11,Vis12
	};
	for (int i=0;i<MAXMAPLAYER;i++) {
        layer[i]->setText(Gis.name[i]);
        vis[i]->setChecked(Gis.flag[i]);
	}
}
//---------------------------------------------------------------------------

