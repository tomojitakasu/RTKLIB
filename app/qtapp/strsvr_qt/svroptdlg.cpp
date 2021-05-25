//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QCompleter>
#include <QFileDialog>

#include "rtklib.h"
#include "refdlg.h"
#include "svroptdlg.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SvrOptDialog::SvrOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    QCompleter *dirCompleter = new QCompleter(this);
    QFileSystemModel *dirModel = new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    LocalDir->setCompleter(dirCompleter);

    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnPos, SIGNAL(clicked(bool)), this, SLOT(BtnPosClick()));
    connect(BtnLogFile, SIGNAL(clicked(bool)), this, SLOT(BtnLogFileClick()));
    connect(NmeaReqT, SIGNAL(clicked(bool)), this, SLOT(NmeaReqTClick()));
    connect(BtnLocalDir, SIGNAL(clicked(bool)), this, SLOT(BtnLocalDirClick()));
    connect(StaInfoSel, SIGNAL(clicked(bool)), this, SLOT(StaInfoSelClick()));

}
//---------------------------------------------------------------------------
void SvrOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    DataTimeout->setValue(SvrOpt[0]);
    ConnectInterval->setValue(SvrOpt[1]);
    AvePeriodRate->setValue(SvrOpt[2]);
    SvrBuffSize->setValue(SvrOpt[3]);
    SvrCycle->setValue(SvrOpt[4]);
    RelayMsg->setCurrentIndex(RelayBack);
    ProgBarR->setValue(ProgBarRange);
    NmeaCycle->setValue(SvrOpt[5]);
    FileSwapMarginE->setValue(FileSwapMargin);
    if (norm(AntPos, 3) > 0.0) {
        double pos[3];

        ecef2pos(AntPos, pos);
        AntPos1->setValue(pos[0] * R2D);
        AntPos2->setValue(pos[1] * R2D);
        AntPos3->setValue(pos[2]);
    } else {
        AntPos1->setValue(0);
        AntPos2->setValue(0);
        AntPos3->setValue(0);
	}
    TraceLevelS->setCurrentIndex(TraceLevel);
    NmeaReqT->setChecked(NmeaReq);
    LocalDir->setText(LocalDirectory);
    ProxyAddr->setText(ProxyAddress);
    StationId->setValue(StaId);
    StaInfoSel->setChecked(StaSel);
    AntInfo->setText(AntType);
    RcvInfo->setText(RcvType);
    AntOff1->setValue(AntOff[0]);
    AntOff2->setValue(AntOff[1]);
    AntOff3->setValue(AntOff[2]);
    LogFileF->setText(LogFile);

	UpdateEnable();
}
//---------------------------------------------------------------------------
void SvrOptDialog::BtnOkClick()
{
	double pos[3];

    SvrOpt[0] = DataTimeout->value();
    SvrOpt[1] = ConnectInterval->value();
    SvrOpt[2] = AvePeriodRate->value();
    SvrOpt[3] = SvrBuffSize->value();
    SvrOpt[4] = SvrCycle->value();
    SvrOpt[5] = NmeaCycle->value();
    FileSwapMargin = FileSwapMarginE->value();
    RelayBack = RelayMsg->currentIndex();
    ProgBarRange = ProgBarR->value();
    pos[0] = AntPos1->value() * D2R;
    pos[1] = AntPos2->value() * D2R;
    pos[2] = AntPos3->value();

    if (norm(pos, 3) > 0.0)
        pos2ecef(pos, AntPos);
    else
        for (int i = 0; i < 3; i++) AntPos[i] = 0.0;

    TraceLevel = TraceLevelS->currentIndex();
    NmeaReq = NmeaReqT->isChecked();
    LocalDirectory = LocalDir->text();
    ProxyAddress = ProxyAddr->text();
    StaId = StationId->value();
    StaSel = StaInfoSel->isChecked();
    AntType = AntInfo->text();
    RcvType = RcvInfo->text();
    AntOff[0] = AntOff1->value();
    AntOff[1] = AntOff2->value();
    AntOff[2] = AntOff3->value();

    accept();
}
//---------------------------------------------------------------------------
void SvrOptDialog::BtnPosClick()
{
    RefDialog *refDialog = new RefDialog(this);

    refDialog->RovPos[0] = AntPos1->value();
    refDialog->RovPos[1] = AntPos2->value();
    refDialog->RovPos[2] = AntPos3->value();
    refDialog->BtnLoad->setEnabled(true);
    refDialog->StaPosFile = StaPosFile;
    refDialog->Opt=1;

    refDialog->exec();

    if (refDialog->result() != QDialog::Accepted) return;

    AntPos1->setValue(refDialog->Pos[0]);
    AntPos2->setValue(refDialog->Pos[1]);
    AntPos3->setValue(refDialog->Pos[2]);
    StaPosFile = refDialog->StaPosFile;
    LogFile=LogFileF->text();

    delete refDialog;
}
//---------------------------------------------------------------------------
void SvrOptDialog::BtnLocalDirClick()
{
    QString dir = LocalDir->text();

    dir = QFileDialog::getExistingDirectory(this, tr("Local Directory"), dir);
    LocalDir->setText(dir);
}
//---------------------------------------------------------------------------
void SvrOptDialog::UpdateEnable(void)
{
    NmeaCycle->setEnabled(NmeaReqT->isChecked());
    StationId->setEnabled(StaInfoSel->isChecked());
    AntPos1->setEnabled(StaInfoSel->isChecked() || NmeaReqT->isChecked());
    AntPos2->setEnabled(StaInfoSel->isChecked() || NmeaReqT->isChecked());
    AntPos3->setEnabled(StaInfoSel->isChecked() || NmeaReqT->isChecked());
    BtnPos->setEnabled(StaInfoSel->isChecked() || NmeaReqT->isChecked());
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
void SvrOptDialog::BtnLogFileClick()
{
    LogFileF->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Log File"),QString(),tr("All (*.*)"))));
}
//---------------------------------------------------------------------------
