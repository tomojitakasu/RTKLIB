//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QCompleter>

#include "rtklib.h"
#include "refdlg.h"
#include "svroptdlg.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SvrOptDialog::SvrOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QCompleter *dirCompleter=new QCompleter(this);
    QFileSystemModel *dirModel=new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs|QDir::Drives|QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    LocalDir->setCompleter(dirCompleter);

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnPos,SIGNAL(clicked(bool)),this,SLOT(BtnPosClick()));
    connect(NmeaReqT,SIGNAL(clicked(bool)),this,SLOT(NmeaReqTClick()));
    connect(BtnLocalDir,SIGNAL(clicked(bool)),this,SLOT(BtnLocalDirClick()));
    connect(StaInfoSel,SIGNAL(clicked(bool)),this,SLOT(StaInfoSelClick()));

    // set validators to positive integer values
    SvrBuffSize->setValidator(new QIntValidator(0,0xffffffff));
    AvePeriodRate->setValidator(new QIntValidator(0,0xffffffff));
    SvrCycle->setValidator(new QIntValidator(0,0xffffffff));
    DataTimeout->setValidator(new QIntValidator(0,0xffffffff));
    ConnectInterval->setValidator(new QIntValidator(0,0xffffffff));
    NmeaCycle->setValidator(new QIntValidator(0,0xffffffff));
    StationId->setValidator(new QIntValidator(0,0xffffffff));

    AntPos1->setValidator(new QDoubleValidator(-90,90,9));
    AntPos2->setValidator(new QDoubleValidator(-180,180,9));
    AntPos3->setValidator(new QDoubleValidator(0,8000,3));

    AntOff1->setValidator(new QDoubleValidator());
    AntOff2->setValidator(new QDoubleValidator());
    AntOff3->setValidator(new QDoubleValidator());
}
//---------------------------------------------------------------------------
void SvrOptDialog::showEvent(QShowEvent* event)
{
    if (event->spontaneous()) return;

    DataTimeout->setText(QString::number(SvrOpt[0]));
    ConnectInterval->setText(QString::number(SvrOpt[1]));
    AvePeriodRate->setText(QString::number(SvrOpt[2]));
    SvrBuffSize->setText(QString::number(SvrOpt[3]));
    SvrCycle->setText(QString::number(SvrOpt[4]));
    NmeaCycle->setText(QString::number(SvrOpt[5]));
    FileSwapMarginE->setText(QString::number(FileSwapMargin));
	if (norm(AntPos,3)>0.0) {
        double pos[3];

        ecef2pos(AntPos,pos);
        AntPos1->setText(QString::number(pos[0]*R2D,'f',8));
        AntPos2->setText(QString::number(pos[1]*R2D,'f',8));
        AntPos3->setText(QString::number(pos[2],'f',3));
	}
	else {
        AntPos1->setText("0.00000000");
        AntPos2->setText("0.00000000");
        AntPos3->setText("0.000");
	}
    TraceLevelS->setCurrentIndex(TraceLevel);
    NmeaReqT->setChecked(NmeaReq);
    LocalDir->setText(LocalDirectory);
    ProxyAddr->setText(ProxyAddress);
    StationId->setText(QString::number(StaId));
    StaInfoSel->setChecked(StaSel);
    AntInfo->setText(AntType);
    RcvInfo->setText(RcvType);
    AntOff1->setText(QString::number(AntOff[0],'f',4));
    AntOff2->setText(QString::number(AntOff[1],'f',4));
    AntOff3->setText(QString::number(AntOff[2],'f',4));
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void SvrOptDialog::BtnOkClick()
{
	double pos[3];
    SvrOpt[0]=DataTimeout->text().toInt();
    SvrOpt[1]=ConnectInterval->text().toInt();
    SvrOpt[2]=AvePeriodRate->text().toInt();
    SvrOpt[3]=SvrBuffSize->text().toInt();
    SvrOpt[4]=SvrCycle->text().toInt();
    SvrOpt[5]=NmeaCycle->text().toInt();
    FileSwapMargin=FileSwapMarginE->text().toInt();
    pos[0]=AntPos1->text().toDouble()*D2R;
    pos[1]=AntPos2->text().toDouble()*D2R;
    pos[2]=AntPos3->text().toDouble();
	if (norm(pos,3)>0.0) {
		pos2ecef(pos,AntPos);
	}
	else {
		for (int i=0;i<3;i++) AntPos[i]=0.0;
	}
    TraceLevel=TraceLevelS->currentIndex();
    NmeaReq=NmeaReqT->isChecked();
    LocalDirectory=LocalDir->text();
    ProxyAddress=ProxyAddr->text();
    StaId=StationId->text().toInt();
    StaSel=StaInfoSel->isChecked();
    AntType=AntInfo->text();
    RcvType=RcvInfo->text();
    AntOff[0]=AntOff1->text().toDouble();
    AntOff[1]=AntOff2->text().toDouble();
    AntOff[2]=AntOff3->text().toDouble();

    accept();
}
//---------------------------------------------------------------------------
void SvrOptDialog::BtnPosClick()
{
    RefDialog *refDialog=new RefDialog(this);

    refDialog->RovPos[0]=AntPos1->text().toDouble();
    refDialog->RovPos[1]=AntPos2->text().toDouble();
    refDialog->RovPos[2]=AntPos3->text().toDouble();
    refDialog->BtnLoad->setEnabled(true);
    refDialog->StaPosFile=StaPosFile;

    refDialog->exec();

    if (refDialog->result()!=QDialog::Accepted) return;

    AntPos1->setText(QString::number(refDialog->Pos[0],'f',8));
    AntPos2->setText(QString::number(refDialog->Pos[1],'f',8));
    AntPos3->setText(QString::number(refDialog->Pos[2],'f',3));
    StaPosFile=refDialog->StaPosFile;

    delete refDialog;
}
//---------------------------------------------------------------------------
void SvrOptDialog::BtnLocalDirClick()
{
    QString dir=LocalDir->text();
    dir=QFileDialog::getExistingDirectory(this,tr("Local Directory"),dir);
    LocalDir->setText(dir);
}
//---------------------------------------------------------------------------
void SvrOptDialog::UpdateEnable(void)
{
    NmeaCycle->setEnabled(NmeaReqT->isChecked());
    StationId->setEnabled(StaInfoSel->isChecked());
    AntPos1->setEnabled(StaInfoSel->isChecked()||NmeaReqT->isChecked());
    AntPos2->setEnabled(StaInfoSel->isChecked()||NmeaReqT->isChecked());
    AntPos3->setEnabled(StaInfoSel->isChecked()||NmeaReqT->isChecked());
    BtnPos ->setEnabled(StaInfoSel->isChecked()||NmeaReqT->isChecked());
    AntOff1->setEnabled(StaInfoSel->isChecked());
    AntOff2->setEnabled(StaInfoSel->isChecked());
    AntOff3->setEnabled(StaInfoSel->isChecked());
    AntInfo->setEnabled(StaInfoSel->isChecked());
    RcvInfo->setEnabled(StaInfoSel->isChecked());
}
//---------------------------------------------------------------------------
void SvrOptDialog::NmeaReqTClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void SvrOptDialog::StaInfoSelClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
