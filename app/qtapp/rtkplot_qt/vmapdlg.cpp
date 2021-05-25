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

    connect(Color1, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color2, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color3, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color4, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color5, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color6, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color7, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color8, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color9, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color10, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color11, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color12, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color1F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color2F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color3F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color4F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color5F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color6F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color7F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color8F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color9F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color10F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color11F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Color12F, SIGNAL(clicked(bool)), this, SLOT(BtnColorClick()));
    connect(Vis1, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis2, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis3, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis4, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis5, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis6, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis7, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis8, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis9, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis10, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis11, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Vis12, SIGNAL(clicked(bool)), this, SLOT(VisClick()));
    connect(Layer1, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer2, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer3, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer4, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer5, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer6, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer7, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer8, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer9, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer10, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer11, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(Layer12, SIGNAL(clicked(bool)), this, SLOT(LayerClick()));
    connect(BtnApply, SIGNAL(clicked(bool)), this, SLOT(BtnApplyClick()));
    connect(BtnUp, SIGNAL(clicked(bool)), this, SLOT(BtnUpClick()));
    connect(BtnDown, SIGNAL(clicked(bool)), this, SLOT(BtnDownClick()));
    connect(BtnClose, SIGNAL(clicked(bool)), this, SLOT(reject()));
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnColorClick()
{
    QPushButton *color[]={
        Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
        Color10,Color11,Color12,Color1F,Color2F,Color3F,Color4F,Color5F,
        Color6F,Color7F,Color8F,Color9F,Color10F,Color11F,Color12F,NULL
    };
    int i;

    for (i=0;color[i];i++) {
        if (color[i]==sender()) break;
    }
    if (color[i]) {
        QColorDialog dialog(this);

        dialog.setCurrentColor(Colors[i]);

        if (dialog.exec() != QDialog::Accepted) return;

        color[i]->setStyleSheet(QString("QPushButton {background-color: %1;}").arg(color2String(dialog.currentColor())));
        Colors[i] = dialog.currentColor();

        UpdateMap();
    }
}
//---------------------------------------------------------------------------
void VecMapDialog::LayerClick()
{
    QRadioButton *layer[] = {
        Layer1,	 Layer2,  Layer3, Layer4, Layer5, Layer6, Layer7, Layer8, Layer9,
        Layer10, Layer11, Layer12
	};

    for (int i = 0; i < MAXMAPLAYER; i++)
        layer[i]->setChecked((layer[i] == (QRadioButton *)sender()));

    UpdateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnUpClick()
{
    QRadioButton *layer[] = {
        Layer1,	 Layer2,  Layer3, Layer4, Layer5, Layer6, Layer7, Layer8, Layer9,
        Layer10, Layer11, Layer12
	};
    QPushButton *color[]={
               Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
               Color10,Color11,Color12
    };
    QString style;
    QPushButton *colorf[]={
        Color1F,Color2F,Color3F,Color4F,Color5F,Color6F,Color7F,Color8F,
        Color9F,Color10F,Color11F,Color12F
    };
	gisd_t *data;
	char name[256];
    int i, flag;

    for (i = 0; i < MAXMAPLAYER; i++)
        if (layer[i]->isChecked()) break;
    if (i == 0 || i >= MAXMAPLAYER) return;
    strcpy(name, Gis.name[i - 1]);
    strcpy(Gis.name[i - 1], Gis.name[i]);
    strcpy(Gis.name[i], name);
    flag = Gis.flag[i - 1];
    Gis.flag[i - 1] = Gis.flag[i];
    Gis.flag[i] = flag;
    data = Gis.data[i - 1];
    Gis.data[i - 1] = Gis.data[i];
    Gis.data[i] = data;
    style=color[i-1]->styleSheet();
    color[i-1]->setStyleSheet(color[i]->styleSheet());
    color[i]->setStyleSheet(style);
    style=colorf[i-1]->styleSheet();
    colorf[i-1]->setStyleSheet(colorf[i]->styleSheet());
    colorf[i]->setStyleSheet(style);
    layer[i - 1]->setChecked(true);
    layer[i]->setChecked(false);
    UpdateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnDownClick()
{
    QRadioButton *layer[] = {
        Layer1,	 Layer2,  Layer3, Layer4, Layer5, Layer6, Layer7, Layer8, Layer9,
        Layer10, Layer11, Layer12
	};
    QPushButton *color[]={
               Color1,Color2,Color3,Color4,Color5,Color6,Color7,Color8,Color9,
               Color10,Color11,Color12
    };
    QString style;
    QPushButton *colorf[]={
        Color1F,Color2F,Color3F,Color4F,Color5F,Color6F,Color7F,Color8F,
        Color9F,Color10F,Color11F,Color12F
    };
	gisd_t *data;
	char name[256];
    int i, flag;

    for (i = 0; i < MAXMAPLAYER; i++)
        if (layer[i]->isChecked()) break;
    if (i == MAXMAPLAYER - 1 || i >= MAXMAPLAYER) return;
    strcpy(name, Gis.name[i + 1]);
    strcpy(Gis.name[i + 1], Gis.name[i]);
    strcpy(Gis.name[i], name);
    flag = Gis.flag[i + 1];
    Gis.flag[i + 1] = Gis.flag[i];
    Gis.flag[i] = flag;
    data = Gis.data[i + 1];
    Gis.data[i + 1] = Gis.data[i];
    Gis.data[i] = data;
    style=color[i+1]->styleSheet();
    color[i+1]->setStyleSheet(color[i]->styleSheet());
    color[i]->setStyleSheet(style);
    style=colorf[i+1]->styleSheet();
    colorf[i+1]->setStyleSheet(colorf[i]->styleSheet());
    colorf[i]->setStyleSheet(style);

    layer[i + 1]->setChecked(true);
    layer[i]->setChecked(false);
    UpdateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::VisClick()
{
    QCheckBox *vis[] = {
        Vis1, Vis2, Vis3, Vis4, Vis5, Vis6, Vis7, Vis8, Vis9, Vis10, Vis11, Vis12
	};

    for (int i = 0; i < MAXMAPLAYER; i++)
        if (qobject_cast<QCheckBox *>(sender()) == vis[i]) Gis.flag[i] = vis[i]->isChecked() ? 1 : 0;

    UpdateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    QRadioButton *layer[] = {
        Layer1,	 Layer2,  Layer3, Layer4, Layer5, Layer6, Layer7, Layer8, Layer9,
        Layer10, Layer11, Layer12
	};
    QPushButton *color[] = {
        Color1,	 Color2,  Color3, Color4, Color5, Color6, Color7, Color8, Color9,
        Color10, Color11, Color12
	};
    QPushButton *colorf[]={
        Color1F,Color2F,Color3F,Color4F,Color5F,Color6F,Color7F,Color8F,
        Color9F,Color10F,Color11F,Color12F
    };
    Gis = plot->Gis;
    for (int i = 0; i < MAXMAPLAYER; i++) {
        layer[i]->setChecked(false);
        color[i]->setStyleSheet(QString("QPushButton {background-color: %1;}").arg(color2String(plot->MapColor[i])));
        colorf[i]->setStyleSheet(QString("QPushButton {background-color: %1;}").arg(color2String(plot->MapColor[i])));
	}
    UpdateMap();
}
//---------------------------------------------------------------------------
void VecMapDialog::BtnApplyClick()
{

    UpdateMap();

    accept();
}
//---------------------------------------------------------------------------
void VecMapDialog::UpdateMap(void)
{
    QRadioButton *layer[] = {
        Layer1,	 Layer2,  Layer3, Layer4, Layer5, Layer6, Layer7, Layer8, Layer9,
        Layer10, Layer11, Layer12
	};
    QCheckBox *vis[] = {
        Vis1, Vis2, Vis3, Vis4, Vis5, Vis6, Vis7, Vis8, Vis9, Vis10, Vis11, Vis12
	};

    for (int i = 0; i < MAXMAPLAYER; i++) {
        layer[i]->setText(Gis.name[i]);
        vis[i]->setChecked(Gis.flag[i]);
        plot->MapColor [i]=Colors[i];
        plot->MapColorF[i]=Colors[i+12];
    }
    plot->Gis = Gis;
}
//---------------------------------------------------------------------------
