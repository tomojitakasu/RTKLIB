//---------------------------------------------------------------------------
// ported to Qt by Jens Reimann
#include <stdio.h>

#include <QShowEvent>
#include <QScreen>
#include <QLabel>
#include <QFileDialog>
#include <QDebug>

#include "refdlg.h"
#include "rtklib.h"

static const QChar degreeChar(0260);           // character code of degree (UTF-8)

//---------------------------------------------------------------------------
RefDialog::RefDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    Pos[0]=Pos[1]=Pos[2]=RovPos[0]=RovPos[1]=RovPos[2]=0.0;

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

    QStringList columns;
    columns<<tr("No")<<tr("Latitude(%1)").arg(degreeChar)<<tr("Longitude(%1)").arg(degreeChar)<<tr("Height(m)")<<tr("Id")<<tr("Name")<<tr("Dist(km)");
    StaList->setColumnCount(columns.size());
    StaList->setRowCount(2);

    for (int i=0;i<columns.size();i++)
        for (int j=0;j<2;j++)
            StaList->setItem(i,j,new QTableWidgetItem(""));

    FontScale=2*physicalDpiX();    
    for (int i=0;i<columns.size();i++) {
        StaList->setColumnWidth(i,width[i]*FontScale/96);
    }

    StaList->setHorizontalHeaderLabels(columns);

    LoadList();

    StaList->sortItems(6);
}
//---------------------------------------------------------------------------
void RefDialog::StaListDblClick(int , int row)
{
    Pos[0]=StaList->item(row,1)->text().toDouble();
    Pos[1]=StaList->item(row,2)->text().toDouble();
    Pos[2]=StaList->item(row,3)->text().toDouble();

    accept();
}
//---------------------------------------------------------------------------
void  RefDialog::BtnOKClick()
{
    int row=StaList->currentRow();
    Pos[0]=StaList->item(row,1)->text().toDouble();
    Pos[1]=StaList->item(row,2)->text().toDouble();
    Pos[2]=StaList->item(row,3)->text().toDouble();

    accept();
}
//---------------------------------------------------------------------------
void  RefDialog::BtnLoadClick()
{
    StaPosFile=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Load Station List..."),StaPosFile,tr("Position File (*.pos *.snx);;All (*.*)")));

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

    StaList->setRowCount(0);

	// check format
    QFile fp(StaPosFile);
    if (!fp.open(QIODevice::ReadOnly)) return;

    buff=fp.readAll();
    if (buff.contains("%=SNX")){
		LoadSinex();
		return;
	}

    fp.seek(0);
    while (!fp.atEnd()) {
        buff=fp.readLine();
        buff=buff.mid(0,buff.indexOf('%')); /* remove comments */

        QList<QByteArray> tokens=buff.simplified().split(' ');

        if (tokens.size()!=5) continue;

        StaList->setRowCount(++n);

        for (int i=0;i<3;i++)
            pos[i]=tokens.at(i).toDouble();

        AddRef(n,pos,tokens.at(3),tokens.at(4));
	}
	if (n==0) {
        StaList->setRowCount(0);
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

    while (!file.atEnd()) {
        buff=file.readLine();

        if (buff.contains("+SOLUTION/ESTIMATE")) sol=1;
        if (buff.contains("-SOLUTION/ESTIMATE")) sol=0;

        if (!sol||buff.size()<68) continue;

        if (buff.mid(7,4)=="STAX") {
            rr[0]=buff.mid(47,21).toDouble(&okay);
            if (!okay) continue;
            code=buff.mid(14,4);
        }
        if (buff.mid(7,4)=="STAY") {
            rr[1]=buff.mid(47,21).toDouble(&okay);
            if (!okay) continue;
            if (buff.mid(14,4)!=code) continue;
        }
        if (buff.mid(7,4)=="STAZ") {
            rr[2]=buff.mid(47,21).toDouble(&okay);
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
        StaList->setRowCount(0);
    }

    UpdateDist();
    setWindowTitle(StaPosFile);
}
//---------------------------------------------------------------------------
void  RefDialog::AddRef(int n, double *pos, const QString code,
                                   const QString name)
{
    int row=StaList->rowCount();
    for (int i=0;i<7;i++)
        StaList->setItem(row-1,i, new QTableWidgetItem());

    StaList->setItem(row-1, 0, new QTableWidgetItem(QString::number(n)));
    StaList->setItem(row-1, 1, new QTableWidgetItem(QString::number(pos[0],'f',9)));
    StaList->setItem(row-1, 2, new QTableWidgetItem(QString::number(pos[1],'f',9)));
    StaList->setItem(row-1, 3, new QTableWidgetItem(QString::number(pos[2],'f',4)));
    StaList->setItem(row-1, 4, new QTableWidgetItem(code));
    StaList->setItem(row-1, 5, new QTableWidgetItem(name));
    StaList->setItem(row-1, 6, new QTableWidgetItem(""));
}
//---------------------------------------------------------------------------
int  RefDialog::InputRef(void)
{
    bool ok;

    QList<QTableWidgetItem*> sel=StaList->selectedItems();
    int row=StaList->row(sel.first());
    Pos[0]=StaList->item(row, 1)->text().toDouble(&ok);
    Pos[1]=StaList->item(row, 2)->text().toDouble(&ok);
    Pos[2]=StaList->item(row, 3)->text().toDouble(&ok);
    StaId  =StaList->item(row, 4)->text();
    StaName=StaList->item(row, 5)->text();
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
        if (StaList->item(i,1)->text()=="") continue;

        pos[0]=StaList->item(i,1)->text().toDouble(&ok)*D2R;
        pos[1]=StaList->item(i,2)->text().toDouble(&ok)*D2R;
        pos[2]=StaList->item(i,3)->text().toDouble(&ok);
		pos2ecef(pos,rr);
		for (int j=0;j<3;j++) rr[j]-=ru[j];

        StaList->setItem(i,6, new QTableWidgetItem(QString::number(norm(rr,3)/1E3,'f',1)));
	}
}
//---------------------------------------------------------------------------
