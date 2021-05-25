//---------------------------------------------------------------------------
#include <QShowEvent>

#include "convmain.h"
#include "convopt.h"
#include "codeopt.h"
#include "freqdlg.h"
#include "glofcndlg.h"

extern MainWindow *mainWindow;
//---------------------------------------------------------------------------
ConvOptDialog::ConvOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    codeOptDialog = new CodeOptDialog(this, this);
    gloFcnDialog = new GloFcnDialog(this);
    freqDialog = new FreqDialog(this);

    int glo = MAXPRNGLO, gal = MAXPRNGAL, qzs = MAXPRNQZS, cmp = MAXPRNCMP, irn=MAXPRNIRN;;
    if (glo <= 0) Nav2->setEnabled(false);
    if (gal <= 0) Nav3->setEnabled(false);
    if (qzs <= 0) Nav4->setEnabled(false);
    if (cmp <= 0) Nav6->setEnabled(false);
    if (irn <= 0) Nav7->setEnabled(false);

    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
    connect(BtnMask, SIGNAL(clicked(bool)), this, SLOT(BtnMaskClick()));
    connect(AutoPos, SIGNAL(clicked(bool)), this, SLOT(AutoPosClick()));
    connect(RnxFile, SIGNAL(clicked(bool)), this, SLOT(RnxFileClick()));
    connect(RnxVer, SIGNAL(currentIndexChanged(int)), this, SLOT(RnxVerChange()));
    connect(BtnFcn, SIGNAL(clicked(bool)), this, SLOT(BtnFcnClick()));
    connect(BtnFreq, SIGNAL(clicked(bool)), this, SLOT(BtnFreqClick()));

	UpdateEnable();
}
//---------------------------------------------------------------------------
void ConvOptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    RnxVer->setCurrentIndex(mainWindow->RnxVer);
    RnxFile->setChecked(mainWindow->RnxFile);
    RnxCode->setText(mainWindow->RnxCode);
    RunBy->setText(mainWindow->RunBy);
    Marker->setText(mainWindow->Marker);
    MarkerNo->setText(mainWindow->MarkerNo);
    MarkerType->setText(mainWindow->MarkerType);
    Name0->setText(mainWindow->Name[0]);
    Name1->setText(mainWindow->Name[1]);
    Rec0->setText(mainWindow->Rec[0]);
    Rec1->setText(mainWindow->Rec[1]);
    Rec2->setText(mainWindow->Rec[2]);
    Ant0->setText(mainWindow->Ant[0]);
    Ant1->setText(mainWindow->Ant[1]);
    AppPos0->setValue(mainWindow->AppPos[0]);
    AppPos1->setValue(mainWindow->AppPos[1]);
    AppPos2->setValue(mainWindow->AppPos[2]);
    AntDel0->setValue(mainWindow->AntDel[0]);
    AntDel1->setValue(mainWindow->AntDel[1]);
    AntDel2->setValue(mainWindow->AntDel[2]);
    Comment0->setText(mainWindow->Comment[0]);
    Comment1->setText(mainWindow->Comment[1]);
    RcvOption->setText(mainWindow->RcvOption);

    for (int i = 0; i < 7; i++) CodeMask[i] = mainWindow->CodeMask[i];

    AutoPos->setChecked(mainWindow->AutoPos);
    PhaseShift->setChecked(mainWindow->PhaseShift);
    HalfCyc->setChecked(mainWindow->HalfCyc);
    OutIono->setChecked(mainWindow->OutIono);
    OutTime->setChecked(mainWindow->OutTime);
    OutLeaps->setChecked(mainWindow->OutLeaps);
    gloFcnDialog->EnaGloFcn=mainWindow->EnaGloFcn;
    for (int i=0;i<27;i++) {
        gloFcnDialog->GloFcn[i]=mainWindow->GloFcn[i];
    }

    Nav1->setChecked(mainWindow->NavSys & SYS_GPS);
    Nav2->setChecked(mainWindow->NavSys & SYS_GLO);
    Nav3->setChecked(mainWindow->NavSys & SYS_GAL);
    Nav4->setChecked(mainWindow->NavSys & SYS_QZS);
    Nav5->setChecked(mainWindow->NavSys & SYS_SBS);
    Nav6->setChecked(mainWindow->NavSys & SYS_CMP);
    Nav7->setChecked(mainWindow->NavSys & SYS_IRN);
    Obs1->setChecked(mainWindow->ObsType & OBSTYPE_PR);
    Obs2->setChecked(mainWindow->ObsType & OBSTYPE_CP);
    Obs3->setChecked(mainWindow->ObsType & OBSTYPE_DOP);
    Obs4->setChecked(mainWindow->ObsType & OBSTYPE_SNR);
    Freq1->setChecked(mainWindow->FreqType & FREQTYPE_L1);
    Freq2->setChecked(mainWindow->FreqType & FREQTYPE_L2);
    Freq3->setChecked(mainWindow->FreqType & FREQTYPE_L3);
    Freq4->setChecked(mainWindow->FreqType & FREQTYPE_L4);
    Freq5->setChecked(mainWindow->FreqType & FREQTYPE_L5);

    ExSats->setText(mainWindow->ExSats);
    TraceLevel->setCurrentIndex(mainWindow->TraceLevel);
    ChkSepNav->setChecked(mainWindow->SepNav);
    TimeTol->setValue(mainWindow->TimeTol);

	UpdateEnable();
}
//---------------------------------------------------------------------------
void ConvOptDialog::BtnOkClick()
{
    mainWindow->RnxVer = RnxVer->currentIndex();
    mainWindow->RnxFile = RnxFile->isChecked();
    mainWindow->RnxCode = RnxCode->text();
    mainWindow->RunBy = RunBy->text();
    mainWindow->Marker = Marker->text();
    mainWindow->MarkerNo = MarkerNo->text();
    mainWindow->MarkerType = MarkerType->text();
    mainWindow->Name[0] = Name0->text();
    mainWindow->Name[1] = Name1->text();
    mainWindow->Rec[0] = Rec0->text();
    mainWindow->Rec[1] = Rec1->text();
    mainWindow->Rec[2] = Rec2->text();
    mainWindow->Ant[0] = Ant0->text();
    mainWindow->Ant[1] = Ant1->text();
    mainWindow->AppPos[0] = AppPos0->value();
    mainWindow->AppPos[1] = AppPos1->value();
    mainWindow->AppPos[2] = AppPos2->value();
    mainWindow->AntDel[0] = AntDel0->value();
    mainWindow->AntDel[1] = AntDel1->value();
    mainWindow->AntDel[2] = AntDel2->value();
    mainWindow->Comment[0] = Comment0->text();
    mainWindow->Comment[1] = Comment1->text();
    mainWindow->RcvOption = RcvOption->text();

    for (int i = 0; i < 7; i++) mainWindow->CodeMask[i] = CodeMask[i];

    mainWindow->AutoPos = AutoPos->isChecked();
    mainWindow->PhaseShift = PhaseShift->isChecked();
    mainWindow->HalfCyc = HalfCyc->isChecked();
    mainWindow->OutIono = OutIono->isChecked();
    mainWindow->OutTime = OutTime->isChecked();
    mainWindow->OutLeaps = OutLeaps->isChecked();
    mainWindow->EnaGloFcn=gloFcnDialog->EnaGloFcn;
    for (int i=0;i<27;i++) {
        mainWindow->GloFcn[i]=gloFcnDialog->GloFcn[i];
    }

    int navsys = 0, obstype = 0, freqtype = 0;

    if (Nav1->isChecked()) navsys |= SYS_GPS;
    if (Nav2->isChecked()) navsys |= SYS_GLO;
    if (Nav3->isChecked()) navsys |= SYS_GAL;
    if (Nav4->isChecked()) navsys |= SYS_QZS;
    if (Nav5->isChecked()) navsys |= SYS_SBS;
    if (Nav6->isChecked()) navsys |= SYS_CMP;
    if (Nav7->isChecked()) navsys |= SYS_IRN;

    if (Obs1->isChecked()) obstype |= OBSTYPE_PR;
    if (Obs2->isChecked()) obstype |= OBSTYPE_CP;
    if (Obs3->isChecked()) obstype |= OBSTYPE_DOP;
    if (Obs4->isChecked()) obstype |= OBSTYPE_SNR;

    if (Freq1->isChecked()) freqtype |= FREQTYPE_L1;
    if (Freq2->isChecked()) freqtype |= FREQTYPE_L2;
    if (Freq3->isChecked()) freqtype |= FREQTYPE_L3;
    if (Freq4->isChecked()) freqtype |= FREQTYPE_L4;
    if (Freq5->isChecked()) freqtype |= FREQTYPE_L5;

    mainWindow->NavSys = navsys;
    mainWindow->ObsType = obstype;
    mainWindow->FreqType = freqtype;
    mainWindow->ExSats = ExSats->text();
    mainWindow->TraceLevel = TraceLevel->currentIndex();
    mainWindow->SepNav=ChkSepNav->isChecked();
    mainWindow->TimeTol=TimeTol->value();

    accept();
}
//---------------------------------------------------------------------------
void ConvOptDialog::RnxFileClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ConvOptDialog::RnxVerChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ConvOptDialog::AutoPosClick()
{
	UpdateEnable();
}
void ConvOptDialog::BtnFreqClick()
{
    freqDialog->exec();
}
//---------------------------------------------------------------------------
void ConvOptDialog::BtnMaskClick()
{
    codeOptDialog->NavSys = 0;
    codeOptDialog->FreqType = 0;

    if (Nav1->isChecked()) codeOptDialog->NavSys |= SYS_GPS;
    if (Nav2->isChecked()) codeOptDialog->NavSys |= SYS_GLO;
    if (Nav3->isChecked()) codeOptDialog->NavSys |= SYS_GAL;
    if (Nav4->isChecked()) codeOptDialog->NavSys |= SYS_QZS;
    if (Nav5->isChecked()) codeOptDialog->NavSys |= SYS_SBS;
    if (Nav6->isChecked()) codeOptDialog->NavSys |= SYS_CMP;
    if (Nav7->isChecked()) codeOptDialog->NavSys |= SYS_IRN;

    if (Freq1->isChecked()) codeOptDialog->FreqType |= FREQTYPE_L1;
    if (Freq2->isChecked()) codeOptDialog->FreqType |= FREQTYPE_L2;
    if (Freq3->isChecked()) codeOptDialog->FreqType |= FREQTYPE_L3;
    if (Freq4->isChecked()) codeOptDialog->FreqType |= FREQTYPE_L4;
    if (Freq5->isChecked()) codeOptDialog->FreqType |= FREQTYPE_L5;

    codeOptDialog->show();
}
//---------------------------------------------------------------------------
void ConvOptDialog::UpdateEnable(void)
{
    AppPos0->setEnabled(AutoPos->isChecked());
    AppPos1->setEnabled(AutoPos->isChecked());
    AppPos2->setEnabled(AutoPos->isChecked());
    ChkSepNav->setEnabled(RnxVer->currentIndex()>=3);
    Label13->setEnabled(mainWindow->TimeIntF->isChecked());
    TimeTol->setEnabled(mainWindow->TimeIntF->isChecked());
    Nav3->setEnabled(RnxVer->currentIndex()>=1);
    Nav4->setEnabled(RnxVer->currentIndex()>=5);
    Nav6->setEnabled(RnxVer->currentIndex()==2||RnxVer->currentIndex()>=4);
    Nav7->setEnabled(RnxVer->currentIndex()>=6);
    Freq3->setEnabled(RnxVer->currentIndex()>=1);
    Freq4->setEnabled(RnxVer->currentIndex()>=1);
    Freq5->setEnabled(RnxVer->currentIndex()>=1);
    PhaseShift->setEnabled(RnxVer->currentIndex()>=4);
}
//---------------------------------------------------------------------------
void ConvOptDialog::BtnFcnClick()
{
    gloFcnDialog->exec();
}
//---------------------------------------------------------------------------
