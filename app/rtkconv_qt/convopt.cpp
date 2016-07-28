//---------------------------------------------------------------------------
#include <QShowEvent>

#include "convmain.h"
#include "convopt.h"
#include "codeopt.h"

extern MainWindow *mainWindow;
//---------------------------------------------------------------------------
ConvOptDialog::ConvOptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    codeOptDialog = new CodeOptDialog(this,this);

	int glo=MAXPRNGLO,gal=MAXPRNGAL,qzs=MAXPRNQZS,cmp=MAXPRNCMP;
    if (glo<=0) Nav2->setEnabled(false);
    if (gal<=0) Nav3->setEnabled(false);
    if (qzs<=0) Nav4->setEnabled(false);
    if (cmp<=0) Nav6->setEnabled(false);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(BtnMask,SIGNAL(clicked(bool)),this,SLOT(BtnMaskClick()));
    connect(AutoPos,SIGNAL(clicked(bool)),this,SLOT(AutoPosClick()));
    connect(RnxFile,SIGNAL(clicked(bool)),this,SLOT(RnxFileClick()));
    connect(RnxVer,SIGNAL(currentIndexChanged(int)),this,SLOT(RnxVerChange()));

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
    Ant2->setText(mainWindow->Ant[2]);
    AppPos0->setText(QString::number(mainWindow->AppPos[0],'f',4));
    AppPos1->setText(QString::number(mainWindow->AppPos[1],'f',4));
    AppPos2->setText(QString::number(mainWindow->AppPos[2],'f',4));
    AntDel0->setText(QString::number(mainWindow->AntDel[0],'f',4));
    AntDel1->setText(QString::number(mainWindow->AntDel[1],'f',4));
    AntDel2->setText(QString::number(mainWindow->AntDel[2],'f',4));
    Comment0->setText(mainWindow->Comment[0]);
    Comment1->setText(mainWindow->Comment[1]);
    RcvOption->setText(mainWindow->RcvOption);
    for (int i=0;i<6;i++) CodeMask[i]=mainWindow->CodeMask[i];
    AutoPos->setChecked(mainWindow->AutoPos);
    ScanObs->setChecked(mainWindow->ScanObs);
    OutIono->setChecked(mainWindow->OutIono);
    OutTime->setChecked(mainWindow->OutTime);
    OutLeaps->setChecked(mainWindow->OutLeaps);

    Nav1->setChecked(mainWindow->NavSys&SYS_GPS);
    Nav2->setChecked(mainWindow->NavSys&SYS_GLO);
    Nav3->setChecked(mainWindow->NavSys&SYS_GAL);
    Nav4->setChecked(mainWindow->NavSys&SYS_QZS);
    Nav5->setChecked(mainWindow->NavSys&SYS_SBS);
    Nav6->setChecked(mainWindow->NavSys&SYS_CMP);
    Nav7->setChecked(mainWindow->NavSys&SYS_IRN);
    Obs1->setChecked(mainWindow->ObsType&OBSTYPE_PR);
    Obs2->setChecked(mainWindow->ObsType&OBSTYPE_CP);
    Obs3->setChecked(mainWindow->ObsType&OBSTYPE_DOP);
    Obs4->setChecked(mainWindow->ObsType&OBSTYPE_SNR);
    Freq1->setChecked(mainWindow->FreqType&FREQTYPE_L1);
    Freq2->setChecked(mainWindow->FreqType&FREQTYPE_L2);
    Freq3->setChecked(mainWindow->FreqType&FREQTYPE_L5);
    Freq4->setChecked(mainWindow->FreqType&FREQTYPE_L6);
    Freq5->setChecked(mainWindow->FreqType&FREQTYPE_L7);
    Freq6->setChecked(mainWindow->FreqType&FREQTYPE_L8);
    Freq7->setChecked(mainWindow->FreqType&FREQTYPE_L9);
    ExSats->setText(mainWindow->ExSats);
    TraceLevel->setCurrentIndex(mainWindow->TraceLevel);
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void ConvOptDialog::BtnOkClick()
{
    mainWindow->RnxVer=RnxVer->currentIndex();
    mainWindow->RnxFile=RnxFile->isChecked();
    mainWindow->RnxCode=RnxCode->text();
    mainWindow->RunBy=RunBy->text();
    mainWindow->Marker=Marker->text();
    mainWindow->MarkerNo=MarkerNo->text();
    mainWindow->MarkerType=MarkerType->text();
    mainWindow->Name[0]=Name0->text();
    mainWindow->Name[1]=Name1->text();
    mainWindow->Rec[0]=Rec0->text();
    mainWindow->Rec[1]=Rec1->text();
    mainWindow->Rec[2]=Rec2->text();
    mainWindow->Ant[0]=Ant0->text();
    mainWindow->Ant[1]=Ant1->text();
    mainWindow->Ant[2]=Ant2->text();
    mainWindow->AppPos[0]=AppPos0->text().toDouble();
    mainWindow->AppPos[1]=AppPos1->text().toDouble();
    mainWindow->AppPos[2]=AppPos2->text().toDouble();
    mainWindow->AntDel[0]=AntDel0->text().toDouble();
    mainWindow->AntDel[1]=AntDel1->text().toDouble();
    mainWindow->AntDel[2]=AntDel2->text().toDouble();
    mainWindow->Comment[0]=Comment0->text();
    mainWindow->Comment[1]=Comment1->text();
    mainWindow->RcvOption=RcvOption->text();
    for (int i=0;i<6;i++) mainWindow->CodeMask[i]=CodeMask[i];
    mainWindow->AutoPos=AutoPos->isChecked();
    mainWindow->ScanObs=ScanObs->isChecked();
    mainWindow->OutIono=OutIono->isChecked();
    mainWindow->OutTime=OutTime->isChecked();
    mainWindow->OutLeaps=OutLeaps->isChecked();
	
	int navsys=0,obstype=0,freqtype=0;
    if (Nav1->isChecked()) navsys|=SYS_GPS;
    if (Nav2->isChecked()) navsys|=SYS_GLO;
    if (Nav3->isChecked()) navsys|=SYS_GAL;
    if (Nav4->isChecked()) navsys|=SYS_QZS;
    if (Nav5->isChecked()) navsys|=SYS_SBS;
    if (Nav6->isChecked()) navsys|=SYS_CMP;
    if (Nav7->isChecked()) navsys|=SYS_IRN;
    if (Obs1->isChecked()) obstype|=OBSTYPE_PR;
    if (Obs2->isChecked()) obstype|=OBSTYPE_CP;
    if (Obs3->isChecked()) obstype|=OBSTYPE_DOP;
    if (Obs4->isChecked()) obstype|=OBSTYPE_SNR;
    if (Freq1->isChecked()) freqtype|=FREQTYPE_L1;
    if (Freq2->isChecked()) freqtype|=FREQTYPE_L2;
    if (Freq3->isChecked()) freqtype|=FREQTYPE_L5;
    if (Freq4->isChecked()) freqtype|=FREQTYPE_L6;
    if (Freq5->isChecked()) freqtype|=FREQTYPE_L7;
    if (Freq6->isChecked()) freqtype|=FREQTYPE_L8;
    if (Freq7->isChecked()) freqtype|=FREQTYPE_L9;
    mainWindow->NavSys=navsys;
    mainWindow->ObsType=obstype;
    mainWindow->FreqType=freqtype;
    mainWindow->ExSats=ExSats->text();
    mainWindow->TraceLevel=TraceLevel->currentIndex();

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
//---------------------------------------------------------------------------
void ConvOptDialog::BtnMaskClick()
{    
    codeOptDialog->NavSys=0;
    codeOptDialog->FreqType=0;
    if (Nav1->isChecked()) codeOptDialog->NavSys|=SYS_GPS;
    if (Nav2->isChecked()) codeOptDialog->NavSys|=SYS_GLO;
    if (Nav3->isChecked()) codeOptDialog->NavSys|=SYS_GAL;
    if (Nav4->isChecked()) codeOptDialog->NavSys|=SYS_QZS;
    if (Nav5->isChecked()) codeOptDialog->NavSys|=SYS_SBS;
    if (Nav6->isChecked()) codeOptDialog->NavSys|=SYS_CMP;
    if (Nav7->isChecked()) codeOptDialog->NavSys|=SYS_IRN;
    if (Freq1->isChecked()) codeOptDialog->FreqType|=FREQTYPE_L1;
    if (Freq2->isChecked()) codeOptDialog->FreqType|=FREQTYPE_L2;
    if (Freq3->isChecked()) codeOptDialog->FreqType|=FREQTYPE_L5;
    if (Freq4->isChecked()) codeOptDialog->FreqType|=FREQTYPE_L6;
    if (Freq5->isChecked()) codeOptDialog->FreqType|=FREQTYPE_L7;
    if (Freq6->isChecked()) codeOptDialog->FreqType|=FREQTYPE_L8;
    if (Freq7->isChecked()) codeOptDialog->FreqType|=FREQTYPE_L9;

    codeOptDialog->show();
}
//---------------------------------------------------------------------------
void ConvOptDialog::UpdateEnable(void)
{
//	Freq4->setEnabled(RnxVer->currentIndex()>0);
//	Freq5->setEnabled(RnxVer->currentIndex()>0);
//	Freq6->setEnabled(RnxVer->currentIndex()>0);
    AppPos0->setEnabled(AutoPos->isChecked());
    AppPos1->setEnabled(AutoPos->isChecked());
    AppPos2->setEnabled(AutoPos->isChecked());
}
//---------------------------------------------------------------------------

