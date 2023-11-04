//---------------------------------------------------------------------------
#include <stdio.h>
#include "rtklib.h"

#include <QShowEvent>
#include <QKeyEvent>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>

#include "plotmain.h"
#include "mapoptdlg.h"

extern Plot *plot;

#define INC_LATLON  0.000001
#define INC_SCALE   0.0001

//---------------------------------------------------------------------------
MapOptDialog::MapOptDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(BtnSave, SIGNAL(clicked(bool)), this, SLOT(BtnSaveClick()));
    connect(BtnClose, SIGNAL(clicked(bool)), this, SLOT(close()));
}
//---------------------------------------------------------------------------
void MapOptDialog::showEvent(QShowEvent*)
{
	UpdateField();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void MapOptDialog::BtnSaveClick()
{
    QString file=plot->MapImageFile;
	if (file=="") return;
	file=file+".tag";

    QFile fp(file);
    if (!fp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (QMessageBox::question(this, file, tr("File exists. Overwrite it?")) != QMessageBox::Yes) return;
	}
    QTextStream out(&fp);
    out << QString("%% map image tag file: rtkplot %1 %2\n\n").arg(VER_RTKLIB).arg(PATCH_LEVEL);
    out << QString("scalex  = %1\n").arg(plot->MapScaleX,0,'g',6);
    out << QString("scaley  = %1\n").arg(plot->MapScaleEq?plot->MapScaleX:plot->MapScaleY,0,'g',6);
    out << QString("scaleeq = %1\n").arg(plot->MapScaleEq);
    out << QString("lat     = %1\n").arg(plot->MapLat,0,'g',9);
    out << QString("lon     = %1\n").arg(plot->MapLon,0,'g',9);

}
//---------------------------------------------------------------------------
void MapOptDialog::BtnCenterClick()
{
    QString s;
	double rr[3],pos[3];
    if (!plot->GetCenterPos(rr)) return;
	ecef2pos(rr,pos);
    Lat->setValue(pos[0]*R2D);
    Lon->setValue(pos[1]*R2D);
	UpdateMap();
}
//---------------------------------------------------------------------------
void MapOptDialog::BtnUpdateClick()
{
	UpdateMap();
}
//---------------------------------------------------------------------------
void MapOptDialog::BtnCloseClick()
{
    close();
}
//---------------------------------------------------------------------------
void MapOptDialog::ScaleEqClick()
{
	UpdateMap();
	UpdateEnable();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void MapOptDialog::UpdateField(void)
{
    QString s;
    setWindowTitle(plot->MapImageFile);
    MapSize1->setValue(plot->MapSize[0]);
    MapSize2->setValue(plot->MapSize[1]);
    ScaleX->setValue(plot->MapScaleX);
    ScaleY->setValue(plot->MapScaleY);
    Lat->setValue(plot->MapLat);
    Lon->setValue(plot->MapLon);
    ScaleEq->setChecked(plot->MapScaleEq);
}
//---------------------------------------------------------------------------
void MapOptDialog::UpdateMap(void)
{
    plot->MapScaleX=ScaleX->value();
    plot->MapScaleY=ScaleY->value();
    plot->MapLat=Lat->value();
    plot->MapLon=Lon->value();
    plot->MapScaleEq=ScaleEq->isChecked();
    plot->UpdatePlot();
}
//---------------------------------------------------------------------------
void MapOptDialog::UpdateEnable(void)
{
    ScaleY      ->setEnabled(!ScaleEq->isChecked());
} 
//---------------------------------------------------------------------------

