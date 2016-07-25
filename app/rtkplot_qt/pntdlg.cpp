//---------------------------------------------------------------------------

#include <QShowEvent>
#include <QFileDialog>
#include <QFile>
#include <QDebug>

#include "rtklib.h"
#include "refdlg.h"
#include "pntdlg.h"
#include "plotmain.h"

extern Plot *plot;

//---------------------------------------------------------------------------
PntDialog::PntDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    QStringList labels;

    labels<<tr("Latitude (°)")<<tr("Longitude (°)")<<"Name";

    PntList->setColumnCount(3);
    PntList->setHorizontalHeaderLabels(labels);

    noUpdate=false;

    connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(close()));
    connect(BtnUpdate,SIGNAL(clicked(bool)),this,SLOT(BtnUpdateClick()));
    connect(BtnDel,SIGNAL(clicked(bool)),this,SLOT(BtnDelClick()));
    connect(BtnAdd,SIGNAL(clicked(bool)),this,SLOT(BtnAddClick()));
    connect(PntList,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(PntListSetEditText()));
    connect(PntList,SIGNAL(itemSelectionChanged()),this,SLOT(PntListClick()));
    connect(PntList,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this,SLOT(PntListDblClick(QTableWidgetItem*)));

}
//---------------------------------------------------------------------------
void PntDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    int width[]={120,120,90};
	
    FontScale=this->physicalDpiX();
    for (int i=0;i<3;i++) {
        PntList->setColumnWidth(i,width[i]*FontScale/96);
    }
}
//---------------------------------------------------------------------------
void PntDialog::BtnAddClick()
{
	double rr[3],pos[3]={0};

    if (PntList->rowCount()>=MAXWAYPNT) return;
    if (!plot->GetCenterPos(rr)) return;
	if (norm(rr,3)<=0.0) return;
	ecef2pos(rr,pos);

    noUpdate=true;
    PntList->setRowCount(PntList->rowCount()+1);
    PntList->setItem(PntList->rowCount()-1,0, new QTableWidgetItem(QString("%1").arg(pos[0]*R2D,0,'f',9)));
    PntList->setItem(PntList->rowCount()-1,1, new QTableWidgetItem(QString("%1").arg(pos[1]*R2D,0,'f',9)));
    PntList->setItem(PntList->rowCount()-1,2, new QTableWidgetItem(QString("Point%1").arg(PntList->rowCount(),2)));
    noUpdate=false;

    UpdatePoint();
}
//---------------------------------------------------------------------------
void PntDialog::BtnDelClick()
{
    QTableWidgetItem *sel=PntList->selectedItems().first();;
    if (!sel) return;
	
    noUpdate=true;
    for (int i=PntList->column(sel);i<PntList->rowCount();i++) {
        for (int j=0;j<PntList->columnCount();j++) {
            if (i+1>=PntList->rowCount()) PntList->setItem(i,j, new QTableWidgetItem(""));
            else PntList->setItem(i,j, new QTableWidgetItem(PntList->item(i+1,j)->text()));
		}
	}
    PntList->setRowCount(PntList->rowCount()-1);
    noUpdate=false;

    UpdatePoint();
}
//---------------------------------------------------------------------------
void PntDialog::BtnUpdateClick()
{
    UpdatePoint();
}
//---------------------------------------------------------------------------
void PntDialog::PntListSetEditText()
{
    UpdatePoint();
}

//---------------------------------------------------------------------------
void PntDialog::UpdatePoint()
{
    int n=0;

    if (noUpdate) return;

    for (int i=0;i<PntList->rowCount();i++) {
        if (!PntList->item(i,0)) continue;
        if (!PntList->item(i,1)) continue;
        if (!PntList->item(i,2)) continue;
        if (PntList->item(i,2)->text()=="") continue;
        plot->PntPos[n][0]=PntList->item(i,0)->text().toDouble();
        plot->PntPos[n][1]=PntList->item(i,1)->text().toDouble();
        plot->PntPos[n][2]=0.0;
        plot->PntName[n++]=PntList->item(i,2)->text();
    }
    plot->NWayPnt=n;

    plot->UpdatePlot();
}
//---------------------------------------------------------------------------
void PntDialog::SetPoint(void)
{
    noUpdate=true;
    PntList->setRowCount(plot->NWayPnt);
    for (int i=0;i<plot->NWayPnt;i++) {
        PntList->setItem(i,0,new QTableWidgetItem(QString::number(plot->PntPos[i][0],'f',9)));
        PntList->setItem(i,1,new QTableWidgetItem(QString::number(plot->PntPos[i][1],'f',9)));
        PntList->setItem(i,2,new QTableWidgetItem(plot->PntName[i]));
    }
    noUpdate=false;
}

//---------------------------------------------------------------------------
void PntDialog::PntListClick()
{
    QList<QTableWidgetItem*> selections= PntList->selectedItems();
    if (selections.isEmpty()) return;
    QTableWidgetItem * item=selections.first();
    if (!item) return;
    int sel=PntList->row(item);
    plot->SelWayPnt=sel<plot->NWayPnt?sel:-1;
    plot->UpdatePlot();
}
//---------------------------------------------------------------------------
void PntDialog::PntListDblClick(QTableWidgetItem *w)
{
    int sel=PntList->row(w);
    if (sel>=plot->NWayPnt) return;
    plot->SetTrkCent(plot->PntPos[sel][0],plot->PntPos[sel][1]);
}
//---------------------------------------------------------------------------
