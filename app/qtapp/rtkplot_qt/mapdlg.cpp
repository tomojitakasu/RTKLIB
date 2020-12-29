//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QMessageBox>

#include <stdio.h>
#include "rtklib.h"

#include "plotmain.h"
#include "mapdlg.h"

extern Plot *plot;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
MapAreaDialog::MapAreaDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(BtnCenter,SIGNAL(clicked(bool)),this,SLOT(BtnCenterClick()));
    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
    connect(BtnSave,SIGNAL(clicked(bool)),this,SLOT(BtnSaveClick()));
    connect(BtnUpdate,SIGNAL(clicked(bool)),this,SLOT(BtnUpdateClick()));
    connect(ScaleEq,SIGNAL(clicked(bool)),this,SLOT(ScaleEqClick()));
}
//---------------------------------------------------------------------------
void MapAreaDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

	UpdateField();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void MapAreaDialog::BtnSaveClick()
{
    QFile fp;
    QString data;
    QString file=plot->MapImageFile;

	if (file=="") return;

	file=file+".tag";

    if (QFile::exists(file)) {
        if (QMessageBox::question(this,file,tr("File exists. Overwrite it?"))!=QMessageBox::Yes) return;
	}
    if (!(fp.open(QIODevice::WriteOnly))) return;

    data=QString("%% map image tag file: rtkplot %1 %2\n\n").arg(VER_RTKLIB).arg(PATCH_LEVEL);
    data+=QString("scalex  = %1\n").arg(plot->MapScaleX,0,'g',6);
    data+=QString("scaley  = %1\n").arg(plot->MapScaleEq?plot->MapScaleX:plot->MapScaleY,0,'g',6);
    data+=QString("scaleeq = %1\n").arg(plot->MapScaleEq);
    data+=QString("lat     = %1\n").arg(plot->MapLat,0,'g',9);
    data+=QString("lon     = %1\n").arg(plot->MapLon,0,'g',9);

    fp.write(data.toLatin1());
};
//---------------------------------------------------------------------------
void MapAreaDialog::BtnCenterClick()
{
	double rr[3],pos[3];

    if (!plot->GetCenterPos(rr)) return;

	ecef2pos(rr,pos);
    Lat->setValue(pos[0]*R2D);
    Lon->setValue(pos[1]*R2D);

	UpdateMap();
}
//---------------------------------------------------------------------------
void MapAreaDialog::BtnUpdateClick()
{
	UpdateMap();
}
//---------------------------------------------------------------------------
void MapAreaDialog::BtnCloseClick()
{
    accept();
}
//---------------------------------------------------------------------------
void MapAreaDialog::ScaleEqClick()
{
	UpdateMap();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void MapAreaDialog::UpdateField(void)
{
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
void MapAreaDialog::UpdateMap(void)
{
    plot->MapScaleX=ScaleX->text().toDouble();
    plot->MapScaleY=ScaleY->text().toDouble();
    plot->MapLat=Lat->text().toDouble();
    plot->MapLon=Lon->text().toDouble();
    plot->MapScaleEq=ScaleEq->isChecked();
    plot->UpdatePlot();
}
//---------------------------------------------------------------------------
void MapAreaDialog::UpdateEnable(void)
{
    ScaleY      ->setEnabled(!ScaleEq->isChecked());
}
//---------------------------------------------------------------------------
