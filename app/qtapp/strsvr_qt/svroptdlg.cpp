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
    lELocalDir->setCompleter(dirCompleter);

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnPosition, SIGNAL(clicked(bool)), this, SLOT(btnPosClicked()));
    connect(btnLogFile, SIGNAL(clicked(bool)), this, SLOT(btnLogFileClicked()));
    connect(cBNmeaReq, SIGNAL(clicked(bool)), this, SLOT(nmeaReqChecked()));
    connect(btnLocalDir, SIGNAL(clicked(bool)), this, SLOT(btnLocalDirClicked()));
    connect(cBStationId, SIGNAL(clicked(bool)), this, SLOT(stationIdChecked()));

}
//---------------------------------------------------------------------------
void SvrOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    sBDataTimeout->setValue(serverOptions[0]);
    sBReconnectInterval->setValue(serverOptions[1]);
    sBAveragePeriodRate->setValue(serverOptions[2]);
    sBServerBufferSize->setValue(serverOptions[3]);
    sBServerCycle->setValue(serverOptions[4]);
    cBRelayMessage->setCurrentIndex(RelayBack);
    sBProgressBar->setValue(progressBarRange);
    sBNmeaCycle->setValue(serverOptions[5]);
    sBFileSwapMargin->setValue(fileSwapMargin);
    if (norm(antennaPos, 3) > 0.0) {
        double pos[3];

        ecef2pos(antennaPos, pos);
        sBAntennaPos1->setValue(pos[0] * R2D);
        sBAntennaPos2->setValue(pos[1] * R2D);
        sBAntennaPos3->setValue(pos[2]);
    } else {
        sBAntennaPos1->setValue(0);
        sBAntennaPos2->setValue(0);
        sBAntennaPos3->setValue(0);
	}
    cBTraceLevel->setCurrentIndex(traceLevel);
    cBNmeaReq->setChecked(NmeaReq);
    lELocalDir->setText(localDirectory);
    lEProxyAddress->setText(proxyAddress);
    sBStationId->setValue(stationId);
    cBStationId->setChecked(StaSel);
    lEAntennaInfo->setText(antennaType);
    lEReceiverInfo->setText(receiverType);
    sBAntennaOffset1->setValue(antennaOffset[0]);
    sBAntennaOffset2->setValue(antennaOffset[1]);
    sBAntennaOffset3->setValue(antennaOffset[2]);
    lELogFile->setText(logFile);

	updateEnable();
}
//---------------------------------------------------------------------------
void SvrOptDialog::btnOkClicked()
{
	double pos[3];

    serverOptions[0] = sBDataTimeout->value();
    serverOptions[1] = sBReconnectInterval->value();
    serverOptions[2] = sBAveragePeriodRate->value();
    serverOptions[3] = sBServerBufferSize->value();
    serverOptions[4] = sBServerCycle->value();
    serverOptions[5] = sBNmeaCycle->value();
    fileSwapMargin = sBFileSwapMargin->value();
    RelayBack = cBRelayMessage->currentIndex();
    progressBarRange = sBProgressBar->value();
    pos[0] = sBAntennaPos1->value() * D2R;
    pos[1] = sBAntennaPos2->value() * D2R;
    pos[2] = sBAntennaPos3->value();

    if (norm(pos, 3) > 0.0)
        pos2ecef(pos, antennaPos);
    else
        for (int i = 0; i < 3; i++) antennaPos[i] = 0.0;

    traceLevel = cBTraceLevel->currentIndex();
    NmeaReq = cBNmeaReq->isChecked();
    localDirectory = lELocalDir->text();
    proxyAddress = lEProxyAddress->text();
    stationId = sBStationId->value();
    StaSel = cBStationId->isChecked();
    antennaType = lEAntennaInfo->text();
    receiverType = lEReceiverInfo->text();
    antennaOffset[0] = sBAntennaOffset1->value();
    antennaOffset[1] = sBAntennaOffset2->value();
    antennaOffset[2] = sBAntennaOffset3->value();

    accept();
}
//---------------------------------------------------------------------------
void SvrOptDialog::btnPosClicked()
{
    RefDialog *refDialog = new RefDialog(this);

    refDialog->RoverPosition[0] = sBAntennaPos1->value();
    refDialog->RoverPosition[1] = sBAntennaPos2->value();
    refDialog->RoverPosition[2] = sBAntennaPos3->value();
    refDialog->btnLoad->setEnabled(true);
    refDialog->stationPositionFile = stationPositionFile;
    refDialog->options=1;

    refDialog->exec();

    if (refDialog->result() != QDialog::Accepted) return;

    sBAntennaPos1->setValue(refDialog->position[0]);
    sBAntennaPos2->setValue(refDialog->position[1]);
    sBAntennaPos3->setValue(refDialog->position[2]);
    stationPositionFile = refDialog->stationPositionFile;
    logFile=lELogFile->text();

    delete refDialog;
}
//---------------------------------------------------------------------------
void SvrOptDialog::btnLocalDirClicked()
{
    QString dir = lELocalDir->text();

    dir = QFileDialog::getExistingDirectory(this, tr("Local Directory"), dir);
    lELocalDir->setText(dir);
}
//---------------------------------------------------------------------------
void SvrOptDialog::updateEnable(void)
{
    sBNmeaCycle->setEnabled(cBNmeaReq->isChecked());
    cBStationId->setEnabled(cBStationId->isChecked());
    sBAntennaPos1->setEnabled(cBStationId->isChecked() || cBNmeaReq->isChecked());
    sBAntennaPos2->setEnabled(cBStationId->isChecked() || cBNmeaReq->isChecked());
    sBAntennaPos3->setEnabled(cBStationId->isChecked() || cBNmeaReq->isChecked());
    btnPosition->setEnabled(cBStationId->isChecked() || cBNmeaReq->isChecked());
    sBAntennaOffset1->setEnabled(cBStationId->isChecked());
    sBAntennaOffset2->setEnabled(cBStationId->isChecked());
    sBAntennaOffset3->setEnabled(cBStationId->isChecked());
    lEAntennaInfo->setEnabled(cBStationId->isChecked());
    lEReceiverInfo->setEnabled(cBStationId->isChecked());
}
//---------------------------------------------------------------------------
void SvrOptDialog::nmeaReqChecked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void SvrOptDialog::stationIdChecked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void SvrOptDialog::btnLogFileClicked()
{
    lELogFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Log File"),QString(),tr("All (*.*)"))));
}
//---------------------------------------------------------------------------
