//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <stdio.h>

#include <QShowEvent>
#include <QScreen>
#include <QLabel>
#include <QFileDialog>

#include "refdlg.h"
#include "rtklib.h"
//---------------------------------------------------------------------------
RefDialog::RefDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

	Pos[0]=Pos[1]=Pos[2]=RovPos[0]=RovPos[1]=RovPos[2]=0.0;

    for (int i=0;i<7;i++)
        for (int j=0;j<2;j++)
            StaList->setItem(i,j,new QTableWidgetItem());

    connect(StaList,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(StaListDblClick(int,int)));
    connect(BtnOK,SIGNAL(clicked(bool)),this,SLOT(BtnOKClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnFind,SIGNAL(clicked(bool)),this,SLOT(BtnFindClick()));
    connect(BtnLoad,SIGNAL(clicked(bool)),this,SLOT(BtnLoadClick()));
    connect(FindStr,SIGNAL(returnPressed()),this,SLOT(FindList()));
}
//---------------------------------------------------------------------------
void  RefDialog::showEvent(QShowEvent* event)
{
    int width[]={30,80,90,65,40,70,55};

    if (event->spontaneous()) return;

    FontScale=physicalDpiX();
	for (int i=0;i<7;i++) {
        StaList->setColumnWidth(i,width[i]*FontScale/96);
	}
    QStringList columns;
    columns<<tr("No")<<tr("Latitude(deg)")<<tr("Longitude(deg)")<<tr("Height(m)")<<tr("Id")<<tr("Name")<<tr("Dist(km)");
    StaList->setHorizontalHeaderLabels(columns);

    LoadList();

    StaList->sortItems(6);
}
//---------------------------------------------------------------------------
void RefDialog::StaListDblClick(int , int row)
{
    setResult(row);

    accept();
}
//---------------------------------------------------------------------------
void  RefDialog::BtnOKClick()
{
    setResult(StaList->currentRow());

    accept();
}
//---------------------------------------------------------------------------
void  RefDialog::BtnLoadClick()
{
    StaPosFile=QFileDialog::getOpenFileName(this,tr("Load Station List..."),StaPosFile);

	LoadList();
}
//---------------------------------------------------------------------------
void  RefDialog::BtnFindClick()
{
	FindList();
}
//---------------------------------------------------------------------------
void  RefDialog::FindList(void)
{
    QList<QTableWidgetItem*> sel=StaList->selectedItems();
    QString str=FindStr->text();
	
    QList<QTableWidgetItem*> f= StaList->findItems(str,Qt::MatchContains);

    if (f.empty()) return;

    StaList->setCurrentItem(f.first());
}
//---------------------------------------------------------------------------
void  RefDialog::LoadList(void)
{
    QByteArray buff;

    double pos[3];
    int n=0;
	
	// check format
    QFile fp(StaPosFile);
    fp.open(QIODevice::ReadOnly);
    buff=fp.readAll();
    if (buff.contains("%=SNX")){
		LoadSinex();
		return;
	}

    fp.seek(0);
    while (fp.canReadLine()) {
        buff=fp.readLine();
        buff=buff.mid(0,buff.indexOf('%')); /* remove comments */

        QList<QByteArray> tokens=buff.split(' ');

        if (tokens.size()!=5) continue;

        StaList->setRowCount(++n+1);

        for (int i=0;i<3;i++)
            pos[i]=tokens.at(i).toDouble();

        AddRef(n,pos,tokens.at(3),tokens.at(4));
	}
	if (n==0) {
        StaList->clear();
        StaList->setRowCount(1);
	}
	UpdateDist();
    setWindowTitle(StaPosFile);
}
//---------------------------------------------------------------------------
void  RefDialog::LoadSinex(void)
{
    int n=0,sol=0;
    double rr[3],pos[3];
    bool okay;
    QFile file(StaPosFile);
    QByteArray buff, code;
    if (!file.open(QIODevice::ReadOnly)) return;

    while (file.canReadLine()) { /* VERIFY */
        buff=file.readLine();

        if (buff.contains("+SOLUTION/ESTIMATE")) sol=1;
        if (buff.contains("-SOLUTION/ESTIMATE")) sol=0;

        if (!sol||buff.size()<68) continue;
        if (buff.mid(7,4)=="STAX") {
            rr[0]=buff.mid(47).toDouble(&okay);
            if (!okay) continue;
            code=buff.mid(14,4);
        }
        if (buff.mid(7,4)=="STAY") {
            rr[1]=buff.mid(47).toDouble(&okay);
            if (!okay) continue;
            if (buff.mid(14,4)!=code) continue;
        }
        if (buff.mid(7,4)=="STAZ") {
            rr[2]=buff.mid(47).toDouble(&okay);
            if (!okay) continue;
            if (buff.mid(14,4)!=code) continue;
            ecef2pos(rr,pos);
            pos[0]*=R2D;
            pos[1]*=R2D;
            StaList->setRowCount(++n);
            AddRef(n,pos,code,"");
        }
    };
	if (n==0) {
        StaList->clear();
        StaList->setRowCount(1);
	}

    UpdateDist();
    setWindowTitle(StaPosFile);
}
//---------------------------------------------------------------------------
void  RefDialog::AddRef(int n, double *pos, const QString code,
                                   const QString name)
{
    QString s;

    int row=StaList->rowCount();
    for (int i=0;i<7;i++)
        StaList->setItem(i,row-1,new QTableWidgetItem());

    StaList->item(0,row-1)->setText(QString::number(n));
    StaList->item(1,row-1)->setText(QString::number(pos[0],'f',9));
    StaList->item(2,row-1)->setText(QString::number(pos[1],'f',9));
    StaList->item(3,row-1)->setText(QString::number(pos[2],'f',4));
    StaList->item(4,row-1)->setText(code);
    StaList->item(5,row-1)->setText(name);
    StaList->item(6,row-1)->setText("");
}
//---------------------------------------------------------------------------
int  RefDialog::InputRef(void)
{
    bool ok;

    QList<QTableWidgetItem*> sel=StaList->selectedItems();
    int row=StaList->row(sel.first());
    Pos[0]=StaList->item(1,row)->text().toDouble(&ok);
    Pos[1]=StaList->item(2,row)->text().toDouble(&ok);
    Pos[2]=StaList->item(3,row)->text().toDouble(&ok);
    StaId  =StaList->item(4,row)->text();
    StaName=StaList->item(5,row)->text();
	return 1;
}
//---------------------------------------------------------------------------
void  RefDialog::UpdateDist(void)
{
	double pos[3],ru[3],rr[3];
    bool ok;

	for (int i=0;i<3;i++) pos[i]=RovPos[i];

	if (norm(pos,3)<=0.0) return;

	pos[0]*=D2R; pos[1]*=D2R; pos2ecef(pos,ru);

    for (int i=1;i<StaList->rowCount();i++) {
        if (StaList->item(1,i)->text()=="") continue;

        pos[0]=StaList->item(1,i)->text().toDouble(&ok)*D2R;
        pos[1]=StaList->item(2,i)->text().toDouble(&ok)*D2R;
        pos[2]=StaList->item(3,i)->text().toDouble(&ok);
		pos2ecef(pos,rr);
		for (int j=0;j<3;j++) rr[j]-=ru[j];

        StaList->item(6,i)->setText(QString::number(norm(rr,3)/1E3,'f',1));
	}
}
//---------------------------------------------------------------------------
