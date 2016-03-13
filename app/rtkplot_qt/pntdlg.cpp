//---------------------------------------------------------------------------

#include <QShowEvent>
#include <QFileDialog>
#include <QFile>

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

    Pos[0]=Pos[1]=Pos[2]=0.0;

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnDel,SIGNAL(clicked(bool)),this,SLOT(BtnDelClick()));
    connect(BtnLoad,SIGNAL(clicked(bool)),this,SLOT(BtnLoadClick()));
    connect(BtnSave,SIGNAL(clicked(bool)),this,SLOT(BtnSaveClick()));
    connect(BtnAdd,SIGNAL(clicked(bool)),this,SLOT(BtnAddClick()));
}
//---------------------------------------------------------------------------
void PntDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    double pos[3];
	int width[]={90,90,80,90};
	
    FontScale=this->physicalDpiX();
	for (int i=0;i<4;i++) {
        PntList->setColumnWidth(i,width[i]*FontScale/96);
	}
	
    for (int i=0;i<PntList->rowCount();i++) {
        if (i<plot->NWayPnt) {
            ecef2pos(plot->PntPos[i],pos);
            PntList->setItem(i,0, new QTableWidgetItem(QString("%1").arg(pos[0]*R2D,0,'f',9)));
            PntList->setItem(i,1, new QTableWidgetItem(QString("%1").arg(pos[1]*R2D,0,'f',9)));
            PntList->setItem(i,2, new QTableWidgetItem(QString("%1").arg(pos[2],0,'f',4)));
            PntList->setItem(i,3, new QTableWidgetItem(plot->PntName[i]));
		}
		else {
            for (int j=0;j<PntList->columnCount();j++) PntList->setItem(i,j, new QTableWidgetItem(""));
		}
	}
}
//---------------------------------------------------------------------------
void PntDialog::BtnOkClick()
{
	double pos[3]={0};
	int n=0;
    for (int i=0;i<PntList->rowCount();i++) {
        if (PntList->item(i,3)->text()=="") continue;
        pos[0]=PntList->item(i,0)->text().toDouble()*D2R;
        pos[1]=PntList->item(i,1)->text().toDouble()*D2R;
        pos[2]=PntList->item(i,2)->text().toDouble();
        pos2ecef(pos,plot->PntPos[n]);
        plot->PntName[n++]=PntList->item(i,3)->text();
	}
    plot->NWayPnt=n;

    accept();
}
//---------------------------------------------------------------------------
void PntDialog::BtnAddClick()
{
	int i;
	double rr[3],pos[3]={0};
    for (i=0;i<PntList->rowCount();i++) {
        if (PntList->item(i,3)->text()=="") break;
	}
    if (i>=PntList->rowCount()) return;
    if (!plot->GetCurrentPos(rr)) return;
	if (norm(rr,3)<=0.0) return;
	ecef2pos(rr,pos);
    PntList->setItem(i,0, new QTableWidgetItem(QString("%1").arg(pos[0]*R2D,0,'f',9)));
    PntList->setItem(i,1, new QTableWidgetItem(QString("%1").arg(pos[1]*R2D,0,'f',9)));
    PntList->setItem(i,2, new QTableWidgetItem(QString("%1").arg(pos[2],0,'f',4)));
    PntList->setItem(i,3, new QTableWidgetItem(QString("new point %1").arg(i+1)));
}
//---------------------------------------------------------------------------
void PntDialog::BtnDelClick()
{
    QTableWidgetItem *sel=PntList->selectedItems().first();;
    if (sel) return;
	
    for (int i=PntList->column(sel);i<PntList->rowCount();i++) {
        for (int j=0;j<PntList->columnCount();j++) {
            if (i+1>=PntList->rowCount()) PntList->setItem(i,j, new QTableWidgetItem(""));
            else PntList->setItem(i,j, new QTableWidgetItem(PntList->item(i+1,j)->text()));
		}
	}
}
//---------------------------------------------------------------------------
void PntDialog::BtnLoadClick()
{
    QString OpenDialog_FileName,s;
    QFile file;
    QByteArray buff,name;
	double pos[3];
	int i=0;
    OpenDialog_FileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Open")));

    if (!file.open(QIODevice::ReadOnly)) return;
    while (!file.atEnd()&&i<PntList->rowCount()) {
        buff=file.readLine();
        if (buff.at(0)=='#') continue;
        QList<QByteArray> tokens=buff.split(' ');
        if (tokens.size() <4) continue;
        for (int j=0;j<3;j++) pos[j]=tokens.at(j).toDouble();
        name=tokens.at(3);

        PntList->setItem(i,0, new QTableWidgetItem(QString("%1").arg(pos[0],0,'f',9)));
        PntList->setItem(i,1, new QTableWidgetItem(QString("%1").arg(pos[1],0,'f',9)));
        PntList->setItem(i,2, new QTableWidgetItem(QString("%1").arg(pos[2],0,'f',4)));
        PntList->setItem(i++,3, new QTableWidgetItem(QString(name)));
	}
    for (;i<PntList->rowCount();i++) {
        PntList->setItem(i,0, new QTableWidgetItem(""));
        PntList->setItem(i,1, new QTableWidgetItem(""));
        PntList->setItem(i,2, new QTableWidgetItem(""));
        PntList->setItem(i,3, new QTableWidgetItem(""));
	}
}
//---------------------------------------------------------------------------
void PntDialog::BtnSaveClick()
{
    QString SaveDialog_FileName=QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Save..")));
    QFile file(SaveDialog_FileName);

    if (!file.open(QIODevice::WriteOnly)) return;

    for (int i=0;i<PntList->rowCount();i++) {
        if (PntList->item(i,3)->text()=="") break;
        QString s=PntList->item(i,0)->text()+" "+PntList->item(i,1)->text()+" "+PntList->item(i,2)->text()+" "+PntList->item(i,3)->text()+"\n";
        file.write(s.toLatin1());
	}
}
//---------------------------------------------------------------------------
