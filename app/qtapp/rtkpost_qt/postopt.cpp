//---------------------------------------------------------------------------

#include "postmain.h"
#include "postopt.h"
#include "keydlg.h"
#include "viewer.h"
#include "refdlg.h"
#include "freqdlg.h"
#include "maskoptdlg.h"

#include <QShowEvent>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QCompleter>

extern MainForm *mainForm;

//---------------------------------------------------------------------------
OptDialog::OptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    widget->setVisible(false);

    int nglo = MAXPRNGLO, ngal = MAXPRNGAL, nqzs = MAXPRNQZS, ncmp = MAXPRNCMP;
    int nirn = MAXPRNIRN;

    QString label, s;
    cBFrequencies->clear();
    for (int i = 0; i < NFREQ; i++) {
        label="L1";
        for (int j=1;j<=i;j++) {
            label+=QString("+%1").arg(j+1);
        }
        cBFrequencies->addItem(label);
    }

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    lEStationPositionFile->setCompleter(fileCompleter);
    lEAntennaPcvFile->setCompleter(fileCompleter);
    lESatellitePcvFile->setCompleter(fileCompleter);
    lEDCBFile->setCompleter(fileCompleter);
    lEGeoidDataFile->setCompleter(fileCompleter);
    lEEOPFile->setCompleter(fileCompleter);
    lEBLQFile->setCompleter(fileCompleter);
    lEIonosphereFile->setCompleter(fileCompleter);

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(btnOkClicked()));
    connect(cBRoverAntennaPcv, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBReferenceAntennaPcv, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(btnAntennaPcvFile, SIGNAL(clicked(bool)), this, SLOT(btnAntennaPcvFileClicked()));
    connect(btnIonosphereFile, SIGNAL(clicked(bool)), this, SLOT(btnIonophereFileClicked()));
    connect(btnAntennaPcvView, SIGNAL(clicked(bool)), this, SLOT(btnAntennaPcvViewClicked()));
    connect(cBPositionMode, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBSolutionFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBAmbiguityResolutionGPS, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(btnLoad, SIGNAL(clicked(bool)), this, SLOT(btnLoadClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(btnSave, SIGNAL(clicked(bool)), this, SLOT(btnSaveClicked()));
    connect(cBFrequencies, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(btnReferencePosition, SIGNAL(clicked(bool)), this, SLOT(btnReferencePositionClicked()));
    connect(btnRoverPosition, SIGNAL(clicked(bool)), this, SLOT(btnRoverPositionClicked()));
    connect(btnStationPositionView, SIGNAL(clicked(bool)), this, SLOT(btnStationPositionViewClicked()));
    connect(btnStationPositionFile, SIGNAL(clicked(bool)), this, SLOT(btnStationPositionFileClicked()));
    connect(cBOutputHeight, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBReferencePositionType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBRoverPositionType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(btnSatellitePcvFile, SIGNAL(clicked(bool)), this, SLOT(btnSatellitePcvFileClicked()));
    connect(btnSatellitePcvView, SIGNAL(clicked(bool)), this, SLOT(btnSatelitePcvViewClicked()));
    connect(cBSatelliteEphemeris, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(btnGeoidDataFile, SIGNAL(clicked(bool)), this, SLOT(btnGeoidDataFileClicked()));
    connect(cBBaselineConstrain, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBNavSys1, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBNavSys2, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBNavSys3, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBNavSys4, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBNavSys5, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBNavSys6, SIGNAL(clicked(bool)), this, SLOT(updateEnable()));
    connect(cBIonosphericOption, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBTroposphericOption, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBDynamicModel, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBSatelliteEphemeris, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBRoverAntenna, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(cBReferenceAntenna, SIGNAL(currentIndexChanged(int)), this, SLOT(updateEnable()));
    connect(btnDCBFile, SIGNAL(clicked(bool)), this, SLOT(btnDCBFileClicked()));
    connect(btnDCBView, SIGNAL(clicked(bool)), this, SLOT(btnDCBViewClicked()));
    connect(btnHelp, SIGNAL(clicked(bool)), this, SLOT(btnHelpClicked()));
    connect(btnBLQFile, SIGNAL(clicked(bool)), this, SLOT(btnBLQFileClicked()));
    connect(btnBLQFileView, SIGNAL(clicked(bool)), this, SLOT(btnBLQFileViewClicked()));
    connect(btnEOPFile, SIGNAL(clicked(bool)), this, SLOT(btnEOPFileClicked()));
    connect(btnEOPView, SIGNAL(clicked(bool)), this, SLOT(btnEOPViewClicked()));
    connect(btnFrequencies, SIGNAL(clicked(bool)), this, SLOT(btnFrequenciesClicked()));
    connect(btnMask, SIGNAL(clicked(bool)), this, SLOT(btnMaskClicked()));

    if (nglo <= 0) cBNavSys2->setEnabled(false);
    if (ngal <= 0) cBNavSys3->setEnabled(false);
    if (nqzs <= 0) cBNavSys4->setEnabled(false);
    if (ncmp <= 0) cBNavSys6->setEnabled(false);
    if (nirn <= 0) cBNavSys7->setEnabled(false);

    updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    getOptions();
}
//---------------------------------------------------------------------------
void OptDialog::btnOkClicked()
{
	setOptions();

    accept();
}
//---------------------------------------------------------------------------
void OptDialog::btnLoadClicked()
{
    loadOptions(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Load Options"), QString(), tr("Options File (*.conf);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::btnSaveClicked()
{
    QString file;

    file = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Options"), QString(), tr("Options File (*.conf);;All (*.*)")));
    QFileInfo f(file);
    if (f.suffix() == "") file = file + ".conf";
	saveOptions(file);
}
//---------------------------------------------------------------------------
void OptDialog::btnStationPositionViewClicked()
{
    if (lEStationPositionFile->text() == "") return;
    TextViewer *viewer = new TextViewer(this);
    viewer->show();
    viewer->read(lEStationPositionFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::btnStationPositionFileClicked()
{
    lEStationPositionFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Station Position File"), QString(), tr("Position File (*.pos);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::roverPositionTypeChanged()
{
    QLineEdit *edit[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
	double pos[3];

    getPosition(roverPositionType, edit, pos);
    setPosition(cBRoverPositionType->currentIndex(), edit, pos);
    roverPositionType = cBRoverPositionType->currentIndex();

    updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::referencePositionTypeChanged()
{
    QLineEdit *edit[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
	double pos[3];

    getPosition(ReferencePositionType, edit, pos);
    setPosition(cBReferencePositionType->currentIndex(), edit, pos);
    ReferencePositionType = cBReferencePositionType->currentIndex();

    updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::btnRoverPositionClicked()
{
    QLineEdit *edit[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    double p[3], pos[3];

    getPosition(cBRoverPositionType->currentIndex(), edit, p);
    ecef2pos(p, pos);

    RefDialog *refDialog = new RefDialog(this);
    refDialog->RoverPosition[0] = pos[0] * R2D;
    refDialog->RoverPosition[1] = pos[1] * R2D;
    refDialog->position[2] = pos[2];
    refDialog->stationPositionFile = lEStationPositionFile->text();
    refDialog->move(this->pos().x() + width() / 2 - refDialog->width() / 2,
            this->pos().y() + height() / 2 - refDialog->height() / 2);

    refDialog->exec();
    if (refDialog->result() != QDialog::Accepted) return;

    pos[0] = refDialog->position[0] * D2R;
    pos[1] = refDialog->position[1] * D2R;
    pos[2] = refDialog->position[2];

    pos2ecef(pos, p);
    setPosition(cBRoverPositionType->currentIndex(), edit, p);

    delete refDialog;
}
//---------------------------------------------------------------------------
void OptDialog::btnReferencePositionClicked()
{
    QLineEdit *edit[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
    double p[3], pos[3];

    getPosition(cBReferencePositionType->currentIndex(), edit, p);
    ecef2pos(p, pos);

    RefDialog *refDialog = new RefDialog(this);
    refDialog->RoverPosition[0] = pos[0] * R2D;
    refDialog->RoverPosition[1] = pos[1] * R2D;
    refDialog->RoverPosition[2] = pos[2];
    refDialog->stationPositionFile = lEStationPositionFile->text();
    refDialog->move(this->pos().x() + width() / 2 - refDialog->width() / 2,
            this->pos().y() + height() / 2 - refDialog->height() / 2);

    refDialog->exec();
    if (refDialog->result() != QDialog::Accepted) return;

    pos[0] = refDialog->position[0] * D2R;
    pos[1] = refDialog->position[1] * D2R;
    pos[2] = refDialog->position[2];

    pos2ecef(pos, p);
    setPosition(cBReferencePositionType->currentIndex(), edit, p);

    delete refDialog;
}
//---------------------------------------------------------------------------
void OptDialog::btnSatelitePcvViewClicked()
{
    if (lESatellitePcvFile->text() == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();
    viewer->read(lESatellitePcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::btnSatellitePcvFileClicked()
{
    lESatellitePcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Satellite Antenna PCV File"), QString(), tr("PCV File (*.pcv *.atx);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::btnAntennaPcvViewClicked()
{
    if (lEAntennaPcvFile->text() == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();
    viewer->read(lEAntennaPcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::btnAntennaPcvFileClicked()
{
    lEAntennaPcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Receiver Antenna PCV File"), QString(), tr("APCV File (*.pcv *.atx);;All (*.*)"))));
    readAntennaList();
}
//---------------------------------------------------------------------------
void OptDialog::btnGeoidDataFileClicked()
{
    lEGeoidDataFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Geoid Data File"), QString(), tr("All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::btnDCBFileClicked()
{
    lEDCBFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("DCB Data File"), QString(), tr("DCB Data File (*.dcb *.DCB);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::btnDCBViewClicked()
{
    QString DCBFile_Text = lEDCBFile->text();

    if (DCBFile_Text == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();
	viewer->read(DCBFile_Text);
}
//---------------------------------------------------------------------------
void OptDialog::btnEOPFileClicked()
{
    lEEOPFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("EOP Date File"), QString(), tr("EOP Data File (*.eop *.erp);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::btnEOPViewClicked()
{
    QString EOPFile_Text = lEEOPFile->text();

    if (EOPFile_Text == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();
	viewer->read(EOPFile_Text);
}
//---------------------------------------------------------------------------
void OptDialog::btnBLQFileClicked()
{
    lEBLQFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Ocean Tide Loading BLQ File"), QString(), tr("OTL BLQ File (*.blq);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::btnBLQFileViewClicked()
{
    QString BLQFile_Text = lEBLQFile->text();

    if (BLQFile_Text == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();
	viewer->read(BLQFile_Text);
}
//---------------------------------------------------------------------------
void OptDialog::btnIonophereFileClicked()
{
    lEIonosphereFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Ionosphere DataFile"), QString(), tr("Ionosphere Data File (*.*i,*stec);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::getOptions(void)
{
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };

    cBPositionMode->setCurrentIndex(mainForm->positionMode);
    cBFrequencies->setCurrentIndex(mainForm->frequencies);
    cBSolution->setCurrentIndex(mainForm->solution);
    cBElevationMask->setCurrentText(QString::number(mainForm->elevationMask, 'f', 0));
    snrMask = mainForm->snrMask;
    cBDynamicModel->setCurrentIndex(mainForm->dynamicModel);
    cBTideCorr->setCurrentIndex(mainForm->tideCorrection);
    cBIonosphericOption->setCurrentIndex(mainForm->ionosphereOption);
    cBTroposphericOption->setCurrentIndex(mainForm->troposphereOption);
    cBSatelliteEphemeris->setCurrentIndex(mainForm->satelliteEphemeris);
    lEExcludedSatellites->setText(mainForm->excludedSatellites);
    cBNavSys1->setChecked(mainForm->navigationSystems & SYS_GPS);
    cBNavSys2->setChecked(mainForm->navigationSystems & SYS_GLO);
    cBNavSys3->setChecked(mainForm->navigationSystems & SYS_GAL);
    cBNavSys4->setChecked(mainForm->navigationSystems & SYS_QZS);
    cBNavSys5->setChecked(mainForm->navigationSystems & SYS_SBS);
    cBNavSys6->setChecked(mainForm->navigationSystems & SYS_CMP);
    cBNavSys7->setChecked(mainForm->navigationSystems & SYS_IRN);
    cBPosOption1->setChecked(mainForm->positionOption[0]);
    cBPosOption2->setChecked(mainForm->positionOption[1]);
    cBPosOption3->setChecked(mainForm->positionOption[2]);
    cBPosOption4->setChecked(mainForm->positionOption[3]);
    cBPosOption5->setChecked(mainForm->positionOption[4]);
    cBPosOption6->setChecked(mainForm->positionOption[5]);

    cBAmbiguityResolutionGPS->setCurrentIndex(mainForm->ambiguityResolutionGPS);
    cBAmbiguityResolutionGLO->setCurrentIndex(mainForm->ambiguityResolutionGLO);
    cBAmbiguityResolutionBDS->setCurrentIndex(mainForm->ambiguityResolutionBDS);
    sBValidThresAR->setValue(mainForm->validThresAR);
    sBThresAR2->setValue(mainForm->thresAR2);
    sBThresAR3->setValue(mainForm->thresAR3);
    sBOutCntResetAmb->setValue(mainForm->outputCntResetAmbiguity);
    sBFixCntHoldAmb->setValue(mainForm->fixCntHoldAmbiguity);
    sBLockCntFixAmb->setValue(mainForm->LockCntFixAmbiguity);
    sBElevationMaskAR->setValue(mainForm->elevationMaskAR);
    sBElevationMaskHold->setValue(mainForm->elevationMaskHold);
    sBMaxAgeDiff->setValue(mainForm->maxAgeDiff);
    sBRejectGdop->setValue(mainForm->rejectGdop);
    sBRejectThres->setValue(mainForm->rejectThres);
    sBSlipThres->setValue(mainForm->slipThres);
    sBARIter->setValue(mainForm->ARIter);
    sBNumIter->setValue(mainForm->numIter);
    sBBaselineLen->setValue(mainForm->baseLine[0]);
    sBBaselineSig->setValue(mainForm->baseLine[1]);
    cBBaselineConstrain->setChecked(mainForm->baseLineConstrain);

    cBSolutionFormat->setCurrentIndex(mainForm->solutionFormat);
    cBTimeFormat->setCurrentIndex(mainForm->timeFormat);
    sBTimeDecimal->setValue(mainForm->timeDecimal);
    cBLatLonFormat->setCurrentIndex(mainForm->latLonFormat);
    lEFieldSeperator->setText(mainForm->fieldSeperator);
    cBOutputHeader->setCurrentIndex(mainForm->outputHeader);
    cBOutputOptions->setCurrentIndex(mainForm->outputOptions);
    cBOutputVelocity->setCurrentIndex(mainForm->outputVelocity);
    cBOutputSingle->setCurrentIndex(mainForm->outputSingle);
    sBMaxSolutionStd->setValue(mainForm->maxSolutionStd);
    cBOutputDatum->setCurrentIndex(mainForm->outputDatum);
    cBOutputHeight->setCurrentIndex(mainForm->outputHeight);
    cBOutputGeoid->setCurrentIndex(mainForm->outputGeoid);
    cBSolutionStatic->setCurrentIndex(mainForm->solutionStatic);
    cBDebugTrace->setCurrentIndex(mainForm->debugTrace);
    cBDebugStatus->setCurrentIndex(mainForm->debugStatus);

    sBMeasurementErrorR1->setValue(mainForm->measurementErrorR1);
    sBMeasurementErrorR2->setValue(mainForm->measurementErrorR2);
    sBMeasurementError2->setValue(mainForm->measurementError2);
    sBMeasurementError3->setValue(mainForm->measurementError3);
    sBMeasurementError4->setValue(mainForm->measurementError4);
    sBMeasurementError5->setValue(mainForm->measurementError5);
    lESatelliteClockStability->setText(QString::number(mainForm->satelliteClockStability, 'E', 2));
    lEProcessNoise1->setText(QString::number(mainForm->processNoise1, 'E', 2));
    lEProcessNoise2->setText(QString::number(mainForm->processNoise2, 'E', 2));
    lEProcessNoise3->setText(QString::number(mainForm->processNoise3, 'E', 2));
    lEProcessNoise4->setText(QString::number(mainForm->processNoise4, 'E', 2));
    lEProcessNoise5->setText(QString::number(mainForm->processNoise5, 'E', 2));

    cBRoverAntennaPcv->setChecked(mainForm->roverAntennaPcv);
    cBReferenceAntennaPcv->setChecked(mainForm->referenceAntennaPcv);
    sBRoverAntennaE->setValue(mainForm->roverAntennaE);
    sBRoverAntennaN->setValue(mainForm->roverAntennaN);
    sBRoverAntennaU->setValue(mainForm->roverAntennaU);
    sBReferenceAntennaE->setValue(mainForm->referenceAntennaE);
    sBReferenceAntennaN->setValue(mainForm->referenceAntennaN);
    sBReferenceAntennaU->setValue(mainForm->referenceAntennaU);
    lEAntennaPcvFile->setText(mainForm->antennaPcvFile);

    lERnxOptions1->setText(mainForm->rnxOptions1);
    lERnxOptions2->setText(mainForm->rnxOptions2);
    lEPPPOptions->setText(mainForm->pppOptions);

    cBIntputReferenceObservation->setCurrentIndex(mainForm->intpolateReferenceObs);
    lESbasSat->setText(QString::number(mainForm->sbasSat));
    lESatellitePcvFile->setText(mainForm->satellitePcvFile);
    lEStationPositionFile->setText(mainForm->stationPositionFile);
    lEGeoidDataFile->setText(mainForm->geoidDataFile);
    lEEOPFile->setText(mainForm->eopFile);
    lEDCBFile->setText(mainForm->dcbFile);
    lEBLQFile->setText(mainForm->blqFile);
    lEIonosphereFile->setText(mainForm->ionosphereFile);
    cBRoverPositionType->setCurrentIndex(mainForm->roverPositionType);
    cBReferencePositionType->setCurrentIndex(mainForm->referencePositionType);
    roverPositionType = cBRoverPositionType->currentIndex();
    ReferencePositionType = cBReferencePositionType->currentIndex();
    setPosition(cBRoverPositionType->currentIndex(), editu, mainForm->roverPosition);
    setPosition(cBReferencePositionType->currentIndex(), editr, mainForm->referencePosition);
	readAntennaList();

    cBRoverAntenna->setCurrentText(mainForm->roverAntenna);
    cBReferenceAntenna->setCurrentText(mainForm->referenceAntenna);

    tERoverList->setPlainText(mainForm->roverList);
    tEBaseList->setPlainText(mainForm->baseList);

	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::setOptions(void)
{
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };

    mainForm->positionMode = cBPositionMode->currentIndex();
    mainForm->frequencies = cBFrequencies->currentIndex();
    mainForm->solution = cBSolution->currentIndex();
    mainForm->elevationMask = cBElevationMask->currentText().toDouble();
    mainForm->snrMask = snrMask;
    mainForm->dynamicModel = cBDynamicModel->currentIndex();
    mainForm->tideCorrection = cBTideCorr->currentIndex();
    mainForm->ionosphereOption = cBIonosphericOption->currentIndex();
    mainForm->troposphereOption = cBTroposphericOption->currentIndex();
    mainForm->satelliteEphemeris = cBSatelliteEphemeris->currentIndex();
    mainForm->excludedSatellites = lEExcludedSatellites->text();
    mainForm->navigationSystems = 0;
    if (cBNavSys1->isChecked()) mainForm->navigationSystems |= SYS_GPS;
    if (cBNavSys2->isChecked()) mainForm->navigationSystems |= SYS_GLO;
    if (cBNavSys3->isChecked()) mainForm->navigationSystems |= SYS_GAL;
    if (cBNavSys4->isChecked()) mainForm->navigationSystems |= SYS_QZS;
    if (cBNavSys5->isChecked()) mainForm->navigationSystems |= SYS_SBS;
    if (cBNavSys6->isChecked()) mainForm->navigationSystems |= SYS_CMP;
    if (cBNavSys7->isChecked()) mainForm->navigationSystems |= SYS_IRN;
    mainForm->positionOption[0] = cBPosOption1->isChecked();
    mainForm->positionOption[1] = cBPosOption2->isChecked();
    mainForm->positionOption[2] = cBPosOption3->isChecked();
    mainForm->positionOption[3] = cBPosOption4->isChecked();
    mainForm->positionOption[4] = cBPosOption5->isChecked();
    mainForm->positionOption[5] = cBPosOption6->isChecked();

    mainForm->ambiguityResolutionGPS = cBAmbiguityResolutionGPS->currentIndex();
    mainForm->ambiguityResolutionGLO = cBAmbiguityResolutionGLO->currentIndex();
    mainForm->ambiguityResolutionBDS = cBAmbiguityResolutionBDS->currentIndex();
    mainForm->validThresAR = sBValidThresAR->value();
    mainForm->thresAR2 = sBThresAR2->value();
    mainForm->thresAR3 = sBThresAR3->value();
    mainForm->outputCntResetAmbiguity = sBOutCntResetAmb->value();
    mainForm->fixCntHoldAmbiguity = sBFixCntHoldAmb->value();
    mainForm->outputCntResetAmbiguity = sBOutCntResetAmb->value();
    mainForm->LockCntFixAmbiguity = sBLockCntFixAmb->value();
    mainForm->elevationMaskAR = sBElevationMaskAR->value();
    mainForm->elevationMaskHold = sBElevationMaskHold->value();
    mainForm->maxAgeDiff = sBMaxAgeDiff->value();
    mainForm->rejectGdop = sBRejectGdop->value();
    mainForm->rejectThres = sBRejectThres->value();
    mainForm->slipThres = sBSlipThres->value();
    mainForm->ARIter = sBARIter->value();
    mainForm->numIter = sBNumIter->value();
    mainForm->baseLine[0] = sBBaselineLen->value();
    mainForm->baseLine[1] = sBBaselineSig->value();
    mainForm->baseLineConstrain = cBBaselineConstrain->isChecked();

    mainForm->solutionFormat = cBSolutionFormat->currentIndex();
    mainForm->timeFormat = cBTimeFormat->currentIndex();
    mainForm->timeDecimal = sBTimeDecimal->value();
    mainForm->latLonFormat = cBLatLonFormat->currentIndex();
    mainForm->fieldSeperator = lEFieldSeperator->text();
    mainForm->outputHeader = cBOutputHeader->currentIndex();
    mainForm->outputOptions = cBOutputOptions->currentIndex();
    mainForm->outputVelocity = cBOutputVelocity->currentIndex();
    mainForm->outputSingle = cBOutputSingle->currentIndex();
    mainForm->maxSolutionStd = sBMaxSolutionStd->value();
    mainForm->outputDatum = cBOutputDatum->currentIndex();
    mainForm->outputHeight = cBOutputHeight->currentIndex();
    mainForm->outputGeoid = cBOutputGeoid->currentIndex();
    mainForm->solutionStatic = cBSolutionStatic->currentIndex();
    mainForm->debugTrace = cBDebugTrace->currentIndex();
    mainForm->debugStatus = cBDebugStatus->currentIndex();

    mainForm->measurementErrorR1 = sBMeasurementErrorR1->value();
    mainForm->measurementErrorR2 = sBMeasurementErrorR2->value();
    mainForm->measurementError2 = sBMeasurementError2->value();
    mainForm->measurementError3 = sBMeasurementError3->value();
    mainForm->measurementError4 = sBMeasurementError4->value();
    mainForm->measurementError5 = sBMeasurementError5->value();
    mainForm->satelliteClockStability = lESatelliteClockStability->text().toDouble();
    mainForm->processNoise1 = lEProcessNoise1->text().toDouble();
    mainForm->processNoise2 = lEProcessNoise2->text().toDouble();
    mainForm->processNoise3 = lEProcessNoise3->text().toDouble();
    mainForm->processNoise4 = lEProcessNoise4->text().toDouble();
    mainForm->processNoise5 = lEProcessNoise5->text().toDouble();

    mainForm->roverAntennaPcv = cBRoverAntennaPcv->isChecked();
    mainForm->referenceAntennaPcv = cBReferenceAntennaPcv->isChecked();
    mainForm->roverAntenna = cBRoverAntenna->currentText();
    mainForm->referenceAntenna = cBReferenceAntenna->currentText();
    mainForm->roverAntennaE = sBRoverAntennaE->value();
    mainForm->roverAntennaN = sBRoverAntennaN->value();
    mainForm->roverAntennaU = sBRoverAntennaU->value();
    mainForm->referenceAntennaE = sBReferenceAntennaE->value();
    mainForm->referenceAntennaN = sBReferenceAntennaN->value();
    mainForm->referenceAntennaU = sBReferenceAntennaU->value();

    mainForm->rnxOptions1 = lERnxOptions1->text();
    mainForm->rnxOptions2 = lERnxOptions2->text();
    mainForm->pppOptions = lEPPPOptions->text();

    mainForm->intpolateReferenceObs = cBIntputReferenceObservation->currentIndex();
    mainForm->sbasSat = lESbasSat->text().toInt();
    mainForm->antennaPcvFile = lEAntennaPcvFile->text();
    mainForm->satellitePcvFile = lESatellitePcvFile->text();
    mainForm->stationPositionFile = lEStationPositionFile->text();
    mainForm->geoidDataFile = lEGeoidDataFile->text();
    mainForm->eopFile = lEEOPFile->text();
    mainForm->dcbFile = lEDCBFile->text();
    mainForm->blqFile = lEBLQFile->text();
    mainForm->ionosphereFile = lEIonosphereFile->text();
    mainForm->roverPositionType = cBRoverPositionType->currentIndex();
    mainForm->referencePositionType = cBReferencePositionType->currentIndex();
    getPosition(cBRoverPositionType->currentIndex(), editu, mainForm->roverPosition);
    getPosition(cBReferencePositionType->currentIndex(), editr, mainForm->referencePosition);

    mainForm->roverList = tERoverList->toPlainText();
    mainForm->baseList = tEBaseList->toPlainText();

	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::loadOptions(const QString &file)
{
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
    QString buff;
    char id[32];
    int sat;
    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;
    filopt_t filopt;

    memset(&filopt, 0, sizeof(filopt_t));

    resetsysopts();
    if (!loadopts(qPrintable(file), sysopts)) return;
    getsysopts(&prcopt, &solopt, &filopt);

    cBPositionMode->setCurrentIndex(prcopt.mode);
    cBFrequencies->setCurrentIndex(prcopt.nf > NFREQ - 1 ? NFREQ - 1 : prcopt.nf - 1);
    cBSolution->setCurrentIndex(prcopt.soltype);
    cBElevationMask->setCurrentText(QString::number(prcopt.elmin * R2D, 'f', 0));
    snrMask = prcopt.snrmask;
    cBDynamicModel->setCurrentIndex(prcopt.dynamics);
    cBTideCorr->setCurrentIndex(prcopt.tidecorr);
    cBIonosphericOption->setCurrentIndex(prcopt.ionoopt);
    cBTroposphericOption->setCurrentIndex(prcopt.tropopt);
    cBSatelliteEphemeris->setCurrentIndex(prcopt.sateph);
    lEExcludedSatellites->setText("");
    for (sat = 1; sat <= MAXSAT; sat++) {
        if (!prcopt.exsats[sat - 1]) continue;
        satno2id(sat, id);
        buff += QString("%1%2%3").arg(buff.isEmpty() ? "" : " ").arg(prcopt.exsats[sat - 1] == 2 ? "+" : "").arg(id);
    }
    lEExcludedSatellites->setText(buff);
    cBNavSys1->setChecked(prcopt.navsys & SYS_GPS);
    cBNavSys2->setChecked(prcopt.navsys & SYS_GLO);
    cBNavSys3->setChecked(prcopt.navsys & SYS_GAL);
    cBNavSys4->setChecked(prcopt.navsys & SYS_QZS);
    cBNavSys5->setChecked(prcopt.navsys & SYS_SBS);
    cBNavSys6->setChecked(prcopt.navsys & SYS_CMP);
    cBNavSys7->setChecked(prcopt.navsys & SYS_IRN);
    cBPosOption1->setChecked(prcopt.posopt[0]);
    cBPosOption2->setChecked(prcopt.posopt[1]);
    cBPosOption3->setChecked(prcopt.posopt[2]);
    cBPosOption4->setChecked(prcopt.posopt[3]);
    cBPosOption5->setChecked(prcopt.posopt[4]);
    cBPosOption6->setChecked(prcopt.posopt[5]);

    cBAmbiguityResolutionGPS->setCurrentIndex(prcopt.modear);
    cBAmbiguityResolutionGLO->setCurrentIndex(prcopt.glomodear);
    cBAmbiguityResolutionBDS->setCurrentIndex(prcopt.bdsmodear);
    sBValidThresAR->setValue(prcopt.thresar[0]);
    sBThresAR2->setValue(prcopt.thresar[1]);
    sBThresAR3->setValue(prcopt.thresar[2]);
    sBOutCntResetAmb->setValue(prcopt.maxout);
    sBFixCntHoldAmb->setValue(prcopt.minfix);
    sBLockCntFixAmb->setValue(prcopt.minlock);
    sBElevationMaskAR->setValue(prcopt.elmaskar * R2D);
    sBElevationMaskHold->setValue(prcopt.elmaskhold * R2D);
    sBMaxAgeDiff->setValue(prcopt.maxtdiff);
    sBRejectGdop->setValue(prcopt.maxgdop);
    sBRejectThres->setValue(prcopt.maxinno);
    sBSlipThres->setValue(prcopt.thresslip);
    sBARIter->setValue(prcopt.armaxiter);
    sBNumIter->setValue(prcopt.niter);
    sBBaselineLen->setValue(prcopt.baseline[0]);
    sBBaselineSig->setValue(prcopt.baseline[1]);
    cBBaselineConstrain->setChecked(prcopt.baseline[0] > 0.0);

    cBSolutionFormat->setCurrentIndex(solopt.posf);
    cBTimeFormat->setCurrentIndex(solopt.timef == 0 ? 0 : solopt.times + 1);
    sBTimeDecimal->setValue(solopt.timeu);
    cBLatLonFormat->setCurrentIndex(solopt.degf);
    lEFieldSeperator->setText(solopt.sep);
    cBOutputHeader->setCurrentIndex(solopt.outhead);
    cBOutputOptions->setCurrentIndex(solopt.outopt);
    cBOutputSingle->setCurrentIndex(prcopt.outsingle);
    sBMaxSolutionStd->setValue(solopt.maxsolstd);
    cBOutputDatum->setCurrentIndex(solopt.datum);
    cBOutputHeight->setCurrentIndex(solopt.height);
    cBOutputGeoid->setCurrentIndex(solopt.geoid);
    cBSolutionStatic->setCurrentIndex(solopt.solstatic);
    sBNmeaInterval1->setValue(solopt.nmeaintv[0]);
    sBNmeaInterval2->setValue(solopt.nmeaintv[1]);
    cBDebugTrace->setCurrentIndex(solopt.trace);
    cBDebugStatus->setCurrentIndex(solopt.sstat);

    sBMeasurementErrorR1->setValue(prcopt.eratio[0]);
    sBMeasurementErrorR2->setValue(prcopt.eratio[1]);
    sBMeasurementError2->setValue(prcopt.err[1]);
    sBMeasurementError3->setValue(prcopt.err[2]);
    sBMeasurementError4->setValue(prcopt.err[3]);
    sBMeasurementError5->setValue(prcopt.err[4]);
    lESatelliteClockStability->setText(QString::number(prcopt.sclkstab, 'E', 2));
    lEProcessNoise1->setText(QString::number(prcopt.prn[0], 'E', 2));
    lEProcessNoise2->setText(QString::number(prcopt.prn[1], 'E', 2));
    lEProcessNoise3->setText(QString::number(prcopt.prn[2], 'E', 2));
    lEProcessNoise4->setText(QString::number(prcopt.prn[3], 'E', 2));
    lEProcessNoise5->setText(QString::number(prcopt.prn[4], 'E', 2));

    cBRoverAntennaPcv->setChecked(*prcopt.anttype[0]);
    cBReferenceAntennaPcv->setChecked(*prcopt.anttype[1]);
    cBRoverAntenna->setCurrentText(prcopt.anttype[0]);
    cBReferenceAntenna->setCurrentText(prcopt.anttype[1]);
    sBRoverAntennaE->setValue(prcopt.antdel[0][0]);
    sBRoverAntennaN->setValue(prcopt.antdel[0][1]);
    sBRoverAntennaU->setValue(prcopt.antdel[0][2]);
    sBReferenceAntennaE->setValue(prcopt.antdel[1][0]);
    sBReferenceAntennaN->setValue(prcopt.antdel[1][1]);
    sBReferenceAntennaU->setValue(prcopt.antdel[1][2]);

    lERnxOptions1->setText(prcopt.rnxopt[0]);
    lERnxOptions2->setText(prcopt.rnxopt[1]);
    lEPPPOptions->setText(prcopt.pppopt);

    cBIntputReferenceObservation->setCurrentIndex(prcopt.intpref);
    lESbasSat->setText(QString::number(prcopt.sbassatsel));
    cBRoverPositionType->setCurrentIndex(prcopt.rovpos == 0 ? 0 : prcopt.rovpos + 2);
    cBReferencePositionType->setCurrentIndex(prcopt.refpos == 0 ? 0 : prcopt.refpos + 2);
    roverPositionType = cBRoverPositionType->currentIndex();
    ReferencePositionType = cBReferencePositionType->currentIndex();
    setPosition(cBRoverPositionType->currentIndex(), editu, prcopt.ru);
    setPosition(cBReferencePositionType->currentIndex(), editr, prcopt.rb);

    lESatellitePcvFile->setText(filopt.satantp);
    lEAntennaPcvFile->setText(filopt.rcvantp);
    lEStationPositionFile->setText(filopt.stapos);
    lEGeoidDataFile->setText(filopt.geoid);
    lEEOPFile->setText(filopt.eop);
    lEDCBFile->setText(filopt.dcb);
    lEBLQFile->setText(filopt.blq);
    lEIonosphereFile->setText(filopt.iono);

	readAntennaList();
	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::saveOptions(const QString &file)
{
    QString ExSats_Text = lEExcludedSatellites->text(), FieldSep_Text = lEFieldSeperator->text();
    QString RovAnt_Text = cBRoverAntenna->currentText(), RefAnt_Text = cBReferenceAntenna->currentText();
    QString SatPcvFile_Text = lESatellitePcvFile->text();
    QString AntPcvFile_Text = lEAntennaPcvFile->text();
    QString lEStationPositionFile_Text = lEStationPositionFile->text();
    QString GeoidDataFile_Text = lEGeoidDataFile->text();
    QString EOPFile_Text = lEEOPFile->text();
    QString DCBFile_Text = lEDCBFile->text();
    QString BLQFile_Text = lEBLQFile->text();
    QString IonoFile_Text = lEIonosphereFile->text();
    QString RnxOpts1_Text = lERnxOptions1->text();
    QString RnxOpts2_Text = lERnxOptions2->text();
    QString PPPOpts_Text = lEPPPOptions->text();
    QLineEdit *editu[] = { lERoverPosition1, lERoverPosition2, lERoverPosition3 };
    QLineEdit *editr[] = { lEReferencePosition1, lEReferencePosition2, lEReferencePosition3 };
    char buff[1024], *p, comment[256], s[64];
    int sat, ex;
    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;
    filopt_t filopt;

    memset(&filopt, 0, sizeof(filopt_t));

    prcopt.mode = cBPositionMode->currentIndex();
    prcopt.nf = cBFrequencies->currentIndex() + 1;
    prcopt.soltype = cBSolution->currentIndex();
    prcopt.elmin = cBElevationMask->currentText().toDouble() * D2R;
    prcopt.snrmask = snrMask;
    prcopt.dynamics = cBDynamicModel->currentIndex();
    prcopt.tidecorr = cBTideCorr->currentIndex();
    prcopt.ionoopt = cBIonosphericOption->currentIndex();
    prcopt.tropopt = cBTroposphericOption->currentIndex();
    prcopt.sateph = cBSatelliteEphemeris->currentIndex();
    if (lEExcludedSatellites->text() != "") {
        strcpy(buff, qPrintable(ExSats_Text));
        for (p = strtok(buff, " "); p; p = strtok(NULL, " ")) {
            if (*p == '+') {
                ex = 2; p++;
            } else {
                ex = 1;
            }
            if (!(sat = satid2no(p))) continue;
            prcopt.exsats[sat - 1] = ex;
        }
    }
    prcopt.navsys = (cBNavSys1->isChecked() ? SYS_GPS : 0) |
                    (cBNavSys2->isChecked() ? SYS_GLO : 0) |
                    (cBNavSys3->isChecked() ? SYS_GAL : 0) |
                    (cBNavSys4->isChecked() ? SYS_QZS : 0) |
                    (cBNavSys5->isChecked() ? SYS_SBS : 0) |
                    (cBNavSys6->isChecked() ? SYS_CMP : 0);
    prcopt.posopt[0] = cBPosOption1->isChecked();
    prcopt.posopt[1] = cBPosOption2->isChecked();
    prcopt.posopt[2] = cBPosOption3->isChecked();
    prcopt.posopt[3] = cBPosOption4->isChecked();
    prcopt.posopt[4] = cBPosOption5->isChecked();
    prcopt.posopt[5] = cBPosOption6->isChecked();
//	prcopt.mapfunc=MapFunc->currentIndex();

    prcopt.modear = cBAmbiguityResolutionGPS->currentIndex();
    prcopt.glomodear = cBAmbiguityResolutionGLO->currentIndex();
    prcopt.bdsmodear = cBAmbiguityResolutionBDS->currentIndex();
    prcopt.thresar[0] = sBValidThresAR->value();
    prcopt.thresar[1] = sBThresAR2->value();
    prcopt.thresar[2] = sBThresAR3->value();
    prcopt.maxout = sBOutCntResetAmb->value();
    prcopt.minfix = sBFixCntHoldAmb->value();
    prcopt.minlock = sBLockCntFixAmb->value();
    prcopt.elmaskar = sBElevationMaskAR->value() * D2R;
    prcopt.elmaskhold = sBElevationMaskHold->value() * D2R;
    prcopt.maxtdiff = sBMaxAgeDiff->value();
    prcopt.maxgdop = sBRejectGdop->value();
    prcopt.maxinno = sBRejectThres->value();
    prcopt.thresslip = sBSlipThres->value();
    prcopt.armaxiter = sBARIter->value();
    prcopt.niter = sBNumIter->value();
    if (prcopt.mode == PMODE_MOVEB && cBBaselineConstrain->isChecked()) {
        prcopt.baseline[0] = sBBaselineLen->value();
        prcopt.baseline[1] = sBBaselineSig->value();
    }
    solopt.posf = cBSolutionFormat->currentIndex();
    solopt.timef = cBTimeFormat->currentIndex() == 0 ? 0 : 1;
    solopt.times = cBTimeFormat->currentIndex() == 0 ? 0 : cBTimeFormat->currentIndex() - 1;
    solopt.timeu = sBTimeDecimal->value();
    solopt.degf = cBLatLonFormat->currentIndex();
    strcpy(solopt.sep, qPrintable(FieldSep_Text));
    solopt.outhead = cBOutputHeader->currentIndex();
    solopt.outopt = cBOutputOptions->currentIndex();
    prcopt.outsingle = cBOutputSingle->currentIndex();
    solopt.maxsolstd = sBMaxSolutionStd->value();
    solopt.datum = cBOutputDatum->currentIndex();
    solopt.height = cBOutputHeight->currentIndex();
    solopt.geoid = cBOutputGeoid->currentIndex();
    solopt.solstatic = cBSolutionFormat->currentIndex();
    solopt.nmeaintv[0] = sBNmeaInterval1->value();
    solopt.nmeaintv[1] = sBNmeaInterval2->value();
    solopt.trace = cBDebugTrace->currentIndex();
    solopt.sstat = cBDebugStatus->currentIndex();

    prcopt.eratio[0] = sBMeasurementErrorR1->value();
    prcopt.eratio[1] = sBMeasurementErrorR2->value();
    prcopt.err[1] = sBMeasurementError2->value();
    prcopt.err[2] = sBMeasurementError3->value();
    prcopt.err[3] = sBMeasurementError4->value();
    prcopt.err[4] = sBMeasurementError5->value();
    prcopt.sclkstab = lESatelliteClockStability->text().toDouble();
    prcopt.prn[0] = lEProcessNoise1->text().toDouble();
    prcopt.prn[1] = lEProcessNoise2->text().toDouble();
    prcopt.prn[2] = lEProcessNoise3->text().toDouble();
    prcopt.prn[3] = lEProcessNoise4->text().toDouble();
    prcopt.prn[4] = lEProcessNoise5->text().toDouble();

    if (cBRoverAntennaPcv->isChecked()) strcpy(prcopt.anttype[0], qPrintable(RovAnt_Text));
    if (cBReferenceAntennaPcv->isChecked()) strcpy(prcopt.anttype[1], qPrintable(RefAnt_Text));
    prcopt.antdel[0][0] = sBRoverAntennaE->value();
    prcopt.antdel[0][1] = sBRoverAntennaN->value();
    prcopt.antdel[0][2] = sBRoverAntennaU->value();
    prcopt.antdel[1][0] = sBReferenceAntennaE->value();
    prcopt.antdel[1][1] = sBReferenceAntennaN->value();
    prcopt.antdel[1][2] = sBReferenceAntennaU->value();

    prcopt.intpref = cBIntputReferenceObservation->currentIndex();
    prcopt.sbassatsel = lESbasSat->text().toInt();
    prcopt.rovpos = cBRoverPositionType->currentIndex() < 3 ? 0 : cBRoverPositionType->currentIndex() - 2;
    prcopt.refpos = cBReferencePositionType->currentIndex() < 3 ? 0 : cBReferencePositionType->currentIndex() - 2;
    if (prcopt.rovpos == 0) getPosition(cBRoverPositionType->currentIndex(), editu, prcopt.ru);
    if (prcopt.refpos == 0) getPosition(cBReferencePositionType->currentIndex(), editr, prcopt.rb);

    strcpy(prcopt.rnxopt[0], qPrintable(RnxOpts1_Text));
    strcpy(prcopt.rnxopt[1], qPrintable(RnxOpts2_Text));
    strcpy(prcopt.pppopt, qPrintable(PPPOpts_Text));

    strcpy(filopt.satantp, qPrintable(SatPcvFile_Text));
    strcpy(filopt.rcvantp, qPrintable(AntPcvFile_Text));
    strcpy(filopt.stapos, qPrintable(lEStationPositionFile_Text));
    strcpy(filopt.geoid, qPrintable(GeoidDataFile_Text));
    strcpy(filopt.eop, qPrintable(EOPFile_Text));
    strcpy(filopt.dcb, qPrintable(DCBFile_Text));
    strcpy(filopt.blq, qPrintable(BLQFile_Text));
    strcpy(filopt.iono, qPrintable(IonoFile_Text));

    time2str(utc2gpst(timeget()), s, 0);
    sprintf(comment, "rtkpost_qt options (%s, v.%s %s)", s, VER_RTKLIB, PATCH_LEVEL);
    setsysopts(&prcopt, &solopt, &filopt);
    if (!saveopts(qPrintable(file), "w", comment, sysopts)) return;
}
//---------------------------------------------------------------------------
void OptDialog::updateEnable(void)
{
    bool rel = PMODE_DGPS <= cBPositionMode->currentIndex() && cBPositionMode->currentIndex() <= PMODE_FIXED;
    bool rtk = PMODE_KINEMA <= cBPositionMode->currentIndex() && cBPositionMode->currentIndex() <= PMODE_FIXED;
    bool ppp = cBPositionMode->currentIndex() >= PMODE_PPP_KINEMA;
    bool ar = rtk || ppp;

    cBFrequencies->setEnabled(rel || ppp);
    cBSolution->setEnabled(rel || ppp);
    cBDynamicModel->setEnabled(rel);
    cBTideCorr->setEnabled(rel || ppp);
    //IonoOpt->setEnabled(!ppp);
    cBPosOption1->setEnabled(ppp);
    cBPosOption2->setEnabled(ppp);
    cBPosOption3->setEnabled(ppp);
    cBPosOption4->setEnabled(ppp);
    cBPosOption6->setEnabled(ppp);

    cBAmbiguityResolutionGPS->setEnabled(ar);
    cBAmbiguityResolutionGLO->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() > 0 && cBNavSys2->isChecked());
    cBAmbiguityResolutionBDS->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() > 0 && cBNavSys6->isChecked());
    sBValidThresAR->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1 && cBAmbiguityResolutionGPS->currentIndex() < 4);
    sBThresAR2->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 4);
    sBThresAR3->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 4);
    sBLockCntFixAmb->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1);
    sBElevationMaskAR->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() >= 1);
    sBOutCntResetAmb->setEnabled(ar || ppp);
    sBFixCntHoldAmb->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() == 3);
    sBElevationMaskHold->setEnabled(ar && cBAmbiguityResolutionGPS->currentIndex() == 3);
    sBSlipThres->setEnabled(rtk || ppp);
    sBMaxAgeDiff->setEnabled(rel);
    sBRejectThres->setEnabled(rel || ppp);
    sBARIter->setEnabled(ppp);
    sBNumIter->setEnabled(rel || ppp);
    cBBaselineConstrain->setEnabled(cBPositionMode->currentIndex() == PMODE_MOVEB);
    sBBaselineLen->setEnabled(cBBaselineConstrain->isChecked() && cBPositionMode->currentIndex() == PMODE_MOVEB);
    sBBaselineSig->setEnabled(cBBaselineConstrain->isChecked() && cBPositionMode->currentIndex() == PMODE_MOVEB);

    cBOutputHeader->setEnabled(cBSolutionFormat->currentIndex() < 3);
    cBOutputOptions->setEnabled(cBSolutionFormat->currentIndex() < 3);
    cBTimeFormat->setEnabled(cBSolutionFormat->currentIndex() < 3);
    sBTimeDecimal->setEnabled(cBSolutionFormat->currentIndex() < 3);
    cBLatLonFormat->setEnabled(cBSolutionFormat->currentIndex() == 0);
    lEFieldSeperator->setEnabled(cBSolutionFormat->currentIndex() < 3);
    cBOutputSingle->setEnabled(cBPositionMode->currentIndex() != 0);
    cBOutputDatum->setEnabled(cBSolutionFormat->currentIndex() == 0);
    cBOutputHeight->setEnabled(cBSolutionFormat->currentIndex() == 0);
    cBOutputGeoid->setEnabled(cBSolutionFormat->currentIndex() == 0 && cBOutputHeight->currentIndex() == 1);
    cBSolutionStatic->setEnabled(cBPositionMode->currentIndex() == PMODE_STATIC ||
                  cBPositionMode->currentIndex() == PMODE_PPP_STATIC);

    cBRoverAntennaPcv->setEnabled(rel || ppp);
    cBRoverAntenna->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked());
    sBRoverAntennaE->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    sBRoverAntennaN->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    sBRoverAntennaU->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    lblRoverAntennaD->setEnabled((rel || ppp) && cBRoverAntennaPcv->isChecked()&&cBRoverAntenna->currentText()!="*");
    cBReferenceAntennaPcv->setEnabled(rel);
    cBReferenceAntenna->setEnabled(rel && cBReferenceAntennaPcv->isChecked());
    sBReferenceAntennaE->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");
    sBReferenceAntennaN->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");
    sBReferenceAntennaU->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");
    lblReferenceAntennaD->setEnabled(rel && cBReferenceAntennaPcv->isChecked()&&cBReferenceAntenna->currentText()!="*");

    cBRoverPositionType->setEnabled(cBPositionMode->currentIndex() == PMODE_FIXED || cBPositionMode->currentIndex() == PMODE_PPP_FIXED);
    lERoverPosition1->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);
    lERoverPosition2->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);
    lERoverPosition3->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);
    btnRoverPosition->setEnabled(cBRoverPositionType->isEnabled() && cBRoverPositionType->currentIndex() <= 2);

    cBReferencePositionType->setEnabled(rel && cBPositionMode->currentIndex() != PMODE_MOVEB);
    lEReferencePosition1->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);
    lEReferencePosition2->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);
    lEReferencePosition3->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);
    btnReferencePosition->setEnabled(cBReferencePositionType->isEnabled() && cBReferencePositionType->currentIndex() <= 2);
}
//---------------------------------------------------------------------------
void OptDialog::getPosition(int type, QLineEdit **edit, double *pos)
{
    QString edit0_Text = edit[0]->text();
    QString edit1_Text = edit[1]->text();
    double p[3] = { 0 }, dms1[3] = { 0 }, dms2[3] = { 0 };

    if (type == 1) { /* lat/lon/height dms/m */
        QStringList tokens = edit0_Text.split(' ');
        if (tokens.size() == 3)
            for (int i = 0; i < 3; i++) dms1[i] = tokens.at(i).toDouble();
        tokens = edit1_Text.split(' ');
        if (tokens.size() == 3)
            for (int i = 0; i < 3; i++) dms2[i] = tokens.at(i).toDouble();
        p[0] = (dms1[0] < 0 ? -1 : 1) * (fabs(dms1[0]) + dms1[1] / 60 + dms1[2] / 3600) * D2R;
        p[1] = (dms2[0] < 0 ? -1 : 1) * (fabs(dms2[0]) + dms2[1] / 60 + dms2[2] / 3600) * D2R;
        p[2] = edit[2]->text().toDouble();
        pos2ecef(p, pos);
    } else if (type == 2) { /* x/y/z-ecef */
        pos[0] = edit[0]->text().toDouble();
        pos[1] = edit[1]->text().toDouble();
        pos[2] = edit[2]->text().toDouble();
    } else {
        p[0] = edit[0]->text().toDouble() * D2R;
        p[1] = edit[1]->text().toDouble() * D2R;
        p[2] = edit[2]->text().toDouble();
        pos2ecef(p, pos);
	}
}
//---------------------------------------------------------------------------
void OptDialog::setPosition(int type, QLineEdit **edit, double *pos)
{
    double p[3], dms1[3], dms2[3], s1, s2;

    if (type == 1) { /* lat/lon/height dms/m */
        ecef2pos(pos, p); s1 = p[0] < 0 ? -1 : 1; s2 = p[1] < 0 ? -1 : 1;
        p[0] = fabs(p[0]) * R2D + 1E-12; p[1] = fabs(p[1]) * R2D + 1E-12;
        dms1[0] = floor(p[0]); p[0] = (p[0] - dms1[0]) * 60.0;
        dms1[1] = floor(p[0]); dms1[2] = (p[0] - dms1[1]) * 60.0;
        dms2[0] = floor(p[1]); p[1] = (p[1] - dms2[0]) * 60.0;
        dms2[1] = floor(p[1]); dms2[2] = (p[1] - dms2[1]) * 60.0;
        edit[0]->setText(QString("%1 %2 %3").arg(s1 * dms1[0], 0, 'f', 0).arg(dms1[1], 2, 'f', 0).arg(dms1[2], 9, 'f', 6));
        edit[1]->setText(QString("%1 %2 %3").arg(s2 * dms2[0], 0, 'f', 0).arg(dms2[1], 2, 'f', 0).arg(dms2[2], 9, 'f', 6));
        edit[2]->setText(QString("%1").arg(p[2], 0, 'f', 4));
    } else if (type == 2) { /* x/y/z-ecef */
        edit[0]->setText(QString::number(pos[0], 'f', 4));
        edit[1]->setText(QString::number(pos[1], 'f', 4));
        edit[2]->setText(QString::number(pos[2], 'f', 4));
    } else {
        ecef2pos(pos, p);
        edit[0]->setText(QString::number(p[0] * R2D, 'f', 9));
        edit[1]->setText(QString::number(p[1] * R2D, 'f', 9));
        edit[2]->setText(QString::number(p[2], 'f', 4));
	}
}
//---------------------------------------------------------------------------
void OptDialog::readAntennaList(void)
{
    QString AntPcvFile_Text = lEAntennaPcvFile->text();
    pcvs_t pcvs = { 0, 0, 0 };
	char *p;

    if (!readpcv(qPrintable(AntPcvFile_Text), &pcvs)) return;

    cBRoverAntenna->clear();
    cBReferenceAntenna->clear();

    cBRoverAntenna->addItem(""); cBReferenceAntenna->addItem("");
    cBRoverAntenna->addItem("*"); cBReferenceAntenna->addItem("*");

    for (int i = 0; i < pcvs.n; i++) {
		if (pcvs.pcv[i].sat) continue;
        if ((p = strchr(pcvs.pcv[i].type, ' '))) *p = '\0';
        if (i > 0 && !strcmp(pcvs.pcv[i].type, pcvs.pcv[i - 1].type)) continue;
        cBRoverAntenna->addItem(pcvs.pcv[i].type);
        cBReferenceAntenna->addItem(pcvs.pcv[i].type);
    }

	free(pcvs.pcv);
}
//---------------------------------------------------------------------------
void OptDialog::btnHelpClicked()
{
    KeyDialog *keyDialog = new KeyDialog(this);

    keyDialog->Flag = 2;
    keyDialog->exec();

    delete keyDialog;
}
//---------------------------------------------------------------------------
void OptDialog::extEna0Clicked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::extEna1Clicked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::extEna2Clicked()
{
	updateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::btnMaskClicked()
{
    MaskOptDialog *maskOptDialog = new MaskOptDialog(this);

    maskOptDialog->Mask = snrMask;
    maskOptDialog->exec();
    if (maskOptDialog->result() != QDialog::Accepted) return;
    snrMask = maskOptDialog->Mask;

    delete  maskOptDialog;
}
//---------------------------------------------------------------------------
void OptDialog::NavSys6Click()
{
	updateEnable();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void OptDialog::btnFrequenciesClicked()
{
    FreqDialog *freqDialog = new FreqDialog(this);

    freqDialog->exec();

    delete freqDialog;
}
