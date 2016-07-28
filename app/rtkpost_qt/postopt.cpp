//---------------------------------------------------------------------------

#include "postmain.h"
#include "postopt.h"
#include "keydlg.h"
#include "viewer.h"
#include "refdlg.h"
#include "extopt.h"
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

    int nglo=MAXPRNGLO,ngal=MAXPRNGAL,nqzs=MAXPRNQZS,ncmp=MAXPRNCMP;
    int nirn=MAXPRNIRN;
    
#if 0
    QString label,s;
    Freq->Items->Clear();
    for (int i=0;i<NFREQ;i++) {
        label=label+(i>0?"+":"L")+s.sprintf("%d",freq[i]);
        Freq->Items->Add(label);
    }
#endif

    QCompleter *fileCompleter=new QCompleter(this);
    QFileSystemModel *fileModel=new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    StaPosFile->setCompleter(fileCompleter);
    AntPcvFile->setCompleter(fileCompleter);
    SatPcvFile->setCompleter(fileCompleter);
    DCBFile->setCompleter(fileCompleter);
    GeoidDataFile->setCompleter(fileCompleter);
    EOPFile->setCompleter(fileCompleter);
    BLQFile->setCompleter(fileCompleter);
    IonoFile->setCompleter(fileCompleter);

    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
    connect(RovAntPcv,SIGNAL(clicked(bool)),this,SLOT(RovAntPcvClick()));
    connect(RefAntPcv,SIGNAL(clicked(bool)),this,SLOT(RovAntPcvClick()));
    connect(BtnAntPcvFile,SIGNAL(clicked(bool)),this,SLOT(BtnAntPcvFileClick()));
    connect(BtnIonoFile,SIGNAL(clicked(bool)),this,SLOT(BtnIonoFileClick()));
    connect(BtnAntPcvView,SIGNAL(clicked(bool)),this,SLOT(BtnAntPcvViewClick()));
    connect(PosMode,SIGNAL(currentIndexChanged(int)),this,SLOT(PosModeChange()));
    connect(SolFormat,SIGNAL(currentIndexChanged(int)),this,SLOT(SolFormatChange()));
    connect(AmbRes,SIGNAL(currentIndexChanged(int)),this,SLOT(AmbResChange()));
    connect(BtnLoad,SIGNAL(clicked(bool)),this,SLOT(BtnLoadClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnSave,SIGNAL(clicked(bool)),this,SLOT(BtnSaveClick()));
    connect(Freq,SIGNAL(currentIndexChanged(int)),this,SLOT(FreqChange()));
    connect(BtnRefPos,SIGNAL(clicked(bool)),this,SLOT(BtnRefPosClick()));
    connect(BtnRovPos,SIGNAL(clicked(bool)),this,SLOT(BtnRovPosClick()));
    connect(BtnStaPosView,SIGNAL(clicked(bool)),this,SLOT(BtnStaPosViewClick()));
    connect(BtnStaPosFile,SIGNAL(clicked(bool)),this,SLOT(BtnSatPcvFileClick()));
    connect(OutputHeight,SIGNAL(currentIndexChanged(int)),this,SLOT(OutputHeightClick()));
    connect(RefPosType,SIGNAL(currentIndexChanged(int)),this,SLOT(RefPosTypeChange()));
    connect(RovPosType,SIGNAL(currentIndexChanged(int)),this,SLOT(RovPosTypeChange()));
    connect(BtnSatPcvFile,SIGNAL(clicked(bool)),this,SLOT(BtnSatPcvFileClick()));
    connect(BtnSatPcvView,SIGNAL(clicked(bool)),this,SLOT(BtnSatPcvViewClick()));
    connect(SatEphem,SIGNAL(currentIndexChanged(int)),this,SLOT(SatEphemChange()));
    connect(BtnGeoidDataFile,SIGNAL(clicked(bool)),this,SLOT(BtnGeoidDataFileClick()));
    connect(BaselineConst,SIGNAL(clicked(bool)),this,SLOT(BaselineConstClick()));
    connect(NavSys1,SIGNAL(clicked(bool)),this,SLOT(NavSys2Click()));
    connect(NavSys2,SIGNAL(clicked(bool)),this,SLOT(NavSys2Click()));
    connect(NavSys3,SIGNAL(clicked(bool)),this,SLOT(NavSys2Click()));
    connect(NavSys4,SIGNAL(clicked(bool)),this,SLOT(NavSys2Click()));
    connect(NavSys5,SIGNAL(clicked(bool)),this,SLOT(NavSys2Click()));
    connect(NavSys6,SIGNAL(clicked(bool)),this,SLOT(NavSys2Click()));
    connect(IonoOpt,SIGNAL(currentIndexChanged(int)),this,SLOT(IonoOptChange()));
    connect(TropOpt,SIGNAL(currentIndexChanged(int)),this,SLOT(TropOptChange()));
    connect(DynamicModel,SIGNAL(currentIndexChanged(int)),this,SLOT(DynamicModelChange()));
    connect(SatEphem,SIGNAL(currentIndexChanged(int)),this,SLOT(SatEphemChange()));
    connect(RovAnt,SIGNAL(currentIndexChanged(int)),this,SLOT(RovAntClick()));
    connect(RefAnt,SIGNAL(currentIndexChanged(int)),this,SLOT(RefAntClick()));
    connect(BtnDCBFile,SIGNAL(clicked(bool)),this,SLOT(BtnDCBFileClick()));
    connect(BtnDCBView,SIGNAL(clicked(bool)),this,SLOT(BtnDCBViewClick()));
    connect(BtnHelp,SIGNAL(clicked(bool)),this,SLOT(BtnHelpClick()));
    connect(BtnBLQFile,SIGNAL(clicked(bool)),this,SLOT(BtnBLQFileClick()));
    connect(BtnBLQFileView,SIGNAL(clicked(bool)),this,SLOT(BtnBLQFileViewClick()));
    connect(BtnEOPFile,SIGNAL(clicked(bool)),this,SLOT(BtnEOPFileClick()));
    connect(BtnEOPView,SIGNAL(clicked(bool)),this,SLOT(BtnEOPViewClick()));
    connect(BtnExtOpt,SIGNAL(clicked(bool)),this,SLOT(BtnExtOptClick()));
    connect(BtnMask,SIGNAL(clicked(bool)),this,SLOT(BtnMaskClick()));

    if (nglo<=0) NavSys2->setEnabled(false);
    if (ngal<=0) NavSys3->setEnabled(false);
    if (nqzs<=0) NavSys4->setEnabled(false);
    if (ncmp<=0) NavSys6->setEnabled(false);
    if (nirn<=0) NavSys7->setEnabled(false);

    UpdateEnable();        
}
//---------------------------------------------------------------------------
void OptDialog::showEvent(QShowEvent *event)
{
    if (event->spontaneous()) return;

    GetOpt();
}
//---------------------------------------------------------------------------
void OptDialog::BtnOkClick()
{
	SetOpt();

    accept();
}
//---------------------------------------------------------------------------
void OptDialog::BtnLoadClick()
{
    LoadOpt(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Load Options"),QString(),tr("Options File (*.conf);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::BtnSaveClick()
{
    QString file;
    file=QDir::toNativeSeparators(QFileDialog::getSaveFileName(this,tr("Save Options"),QString(),tr("Options File (*.conf);;All (*.*)")));
    QFileInfo f(file);
    if (f.suffix()=="") file=file+".conf";
	SaveOpt(file);
}
//---------------------------------------------------------------------------
void OptDialog::BtnStaPosViewClick()
{
    if (StaPosFile->text()=="") return;
    TextViewer *viewer=new TextViewer(this);
    viewer->show();
    viewer->Read(StaPosFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnStaPosFileClick()
{
    StaPosFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Station Postion File"),QString(),tr("Position File (*.pos);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::RovPosTypeChange()
{
    QLineEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double pos[3];

    GetPos(RovPosTypeP,edit,pos);
    SetPos(RovPosType->currentIndex(),edit,pos);
    RovPosTypeP=RovPosType->currentIndex();

    UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RefPosTypeChange()
{
    QLineEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double pos[3];

    GetPos(RefPosTypeP,edit,pos);
    SetPos(RefPosType->currentIndex(),edit,pos);
    RefPosTypeP=RefPosType->currentIndex();

    UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::BtnRovPosClick()
{
    QLineEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double p[3],pos[3];

    GetPos(RovPosType->currentIndex(),edit,p);
	ecef2pos(p,pos);

    RefDialog *refDialog=new RefDialog(this);
    refDialog->RovPos[0]=pos[0]*R2D;
    refDialog->RovPos[1]=pos[1]*R2D;
    refDialog->Pos[2]=pos[2];
    refDialog->StaPosFile=StaPosFile->text();
    refDialog->move(this->pos().x()+width()/2-refDialog->width()/2,
                this->pos().y()+height()/2-refDialog->height()/2);

    refDialog->exec();
    if (refDialog->result()!=QDialog::Accepted) return;

    pos[0]=refDialog->Pos[0]*D2R;
    pos[1]=refDialog->Pos[1]*D2R;
    pos[2]=refDialog->Pos[2];

    pos2ecef(pos,p);
    SetPos(RovPosType->currentIndex(),edit,p);

    delete refDialog;
}
//---------------------------------------------------------------------------
void OptDialog::BtnRefPosClick()
{
    QLineEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double p[3],pos[3];

    GetPos(RefPosType->currentIndex(),edit,p);
	ecef2pos(p,pos);

    RefDialog *refDialog=new RefDialog(this);
    refDialog->RovPos[0]=pos[0]*R2D;
    refDialog->RovPos[1]=pos[1]*R2D;
    refDialog->RovPos[2]=pos[2];
    refDialog->StaPosFile=StaPosFile->text();
    refDialog->move(this->pos().x()+width()/2-refDialog->width()/2,
                this->pos().y()+height()/2-refDialog->height()/2);

    refDialog->exec();
    if (refDialog->result()!=QDialog::Accepted) return;

    pos[0]=refDialog->Pos[0]*D2R;
    pos[1]=refDialog->Pos[1]*D2R;
    pos[2]=refDialog->Pos[2];

    pos2ecef(pos,p);
    SetPos(RefPosType->currentIndex(),edit,p);

    delete refDialog;
}
//---------------------------------------------------------------------------
void OptDialog::BtnSatPcvViewClick()
{
    if (SatPcvFile->text()=="") return;

    TextViewer *viewer=new TextViewer(this);
    viewer->show();
    viewer->Read(SatPcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnSatPcvFileClick()
{
    SatPcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Satellite Antenna PCV File"),QString(),tr("PCV File (*.pcv *.atx);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::BtnAntPcvViewClick()
{
    if (AntPcvFile->text()=="") return;

    TextViewer *viewer=new TextViewer(this);
    viewer->show();
    viewer->Read(AntPcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnAntPcvFileClick()
{
    AntPcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Receiver Antenna PCV File"),QString(),tr("APCV File (*.pcv *.atx);;All (*.*)"))));
    ReadAntList();
}
//---------------------------------------------------------------------------
void OptDialog::BtnGeoidDataFileClick()
{
    GeoidDataFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Geoid Data File"),QString(),tr("All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::BtnDCBFileClick()
{
    DCBFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("DCB Data File"),QString(),tr("DCB Data File (*.dcb *.DCB);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::BtnDCBViewClick()
{
    QString DCBFile_Text=DCBFile->text();
    if (DCBFile->text()=="") return;

    TextViewer *viewer=new TextViewer(this);
    viewer->show();
	viewer->Read(DCBFile_Text);
}
//---------------------------------------------------------------------------
void OptDialog::BtnEOPFileClick()
{
    EOPFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("EOP Date File"),QString(),tr("EOP Data File (*.eop *.erp);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::BtnEOPViewClick()
{
    QString EOPFile_Text=EOPFile->text();
    if (EOPFile->text()=="") return;

    TextViewer *viewer=new TextViewer(this);
    viewer->show();
	viewer->Read(EOPFile_Text);
}
//---------------------------------------------------------------------------
void OptDialog::BtnBLQFileClick()
{
    BLQFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Ocean Tide Loading BLQ File"),QString(),tr("OTL BLQ File (*.blq);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::BtnBLQFileViewClick()
{
    QString BLQFile_Text=BLQFile->text();
    if (BLQFile->text()=="") return;

    TextViewer *viewer=new TextViewer(this);
    viewer->show();
	viewer->Read(BLQFile_Text);
}
//---------------------------------------------------------------------------
void OptDialog::BtnIonoFileClick()
{
    IonoFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Ionosphere DataFile"),QString(),tr("Ionosphere Data File (*.*i,*stec);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::FreqChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::IonoOptChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::TropOptChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::DynamicModelChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SatEphemChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SolFormatChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::PosModeChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SatEphemClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::NavSys2Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::AmbResChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RovAntPcvClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::NetRSCorrClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SatClkCorrClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RovPosClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RefPosClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SbasCorrClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::OutputHeightClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::BaselineConstClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RovAntClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RefAntClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::GetOpt(void)
{
    QLineEdit *editu[]={RovPos1,RovPos2,RovPos3};
    QLineEdit *editr[]={RefPos1,RefPos2,RefPos3};

    PosMode		 ->setCurrentIndex(mainForm->PosMode);
    Freq		 ->setCurrentIndex(mainForm->Freq);
    Solution	 ->setCurrentIndex(mainForm->Solution);
    ElMask		 ->setCurrentText(QString::number(mainForm->ElMask,'f',0));
    SnrMask						=mainForm->SnrMask;
    DynamicModel ->setCurrentIndex(mainForm->DynamicModel);
    TideCorr	 ->setCurrentIndex(mainForm->TideCorr);
    IonoOpt		 ->setCurrentIndex(mainForm->IonoOpt);
    TropOpt		 ->setCurrentIndex(mainForm->TropOpt);
    SatEphem	 ->setCurrentIndex(mainForm->SatEphem);
    ExSats	     ->setText(mainForm->ExSats);
    NavSys1	     ->setChecked(mainForm->NavSys&SYS_GPS);
    NavSys2	     ->setChecked(mainForm->NavSys&SYS_GLO);
    NavSys3	     ->setChecked(mainForm->NavSys&SYS_GAL);
    NavSys4	     ->setChecked(mainForm->NavSys&SYS_QZS);
    NavSys5	     ->setChecked(mainForm->NavSys&SYS_SBS);
    NavSys6	     ->setChecked(mainForm->NavSys&SYS_CMP);
    NavSys7	     ->setChecked(mainForm->NavSys&SYS_IRN);
    PosOpt1	     ->setChecked(mainForm->PosOpt[0]);
    PosOpt2	     ->setChecked(mainForm->PosOpt[1]);
    PosOpt3	     ->setChecked(mainForm->PosOpt[2]);
    PosOpt4	     ->setChecked(mainForm->PosOpt[3]);
    PosOpt5	     ->setChecked(mainForm->PosOpt[4]);
    PosOpt6	     ->setChecked(mainForm->PosOpt[5]);
//	MapFunc	     ->setCurrentIndex(mainForm->MapFunc);
	
    AmbRes		 ->setCurrentIndex(mainForm->AmbRes);
    GloAmbRes	 ->setCurrentIndex(mainForm->GloAmbRes);
    BdsAmbRes	 ->setCurrentIndex(mainForm->BdsAmbRes);
    ValidThresAR ->setText(QString::number(mainForm->ValidThresAR,'g',3));
    ThresAR2     ->setText(QString::number(mainForm->ThresAR2,'g',8));
    ThresAR3     ->setText(QString::number(mainForm->ThresAR3,'g',3));
    OutCntResetAmb->setText(QString::number(mainForm->OutCntResetAmb));
    FixCntHoldAmb->setText(QString::number(mainForm->FixCntHoldAmb));
    LockCntFixAmb->setText(QString::number(mainForm->LockCntFixAmb));
    ElMaskAR	 ->setText(QString::number(mainForm->ElMaskAR,'f',0));
    ElMaskHold	 ->setText(QString::number(mainForm->ElMaskHold,'f',0));
    MaxAgeDiff	 ->setText(QString::number(mainForm->MaxAgeDiff,'f',1));
    RejectGdop   ->setText(QString::number(mainForm->RejectGdop,'f',1));
    RejectThres  ->setText(QString::number(mainForm->RejectThres,'f',1));
    SlipThres	 ->setText(QString::number(mainForm->SlipThres,'f',3));
    ARIter		 ->setText(QString::number(mainForm->ARIter));
    NumIter		 ->setText(QString::number(mainForm->NumIter));
    BaselineLen	 ->setText(QString::number(mainForm->BaseLine[0],'f',3));
    BaselineSig	 ->setText(QString::number(mainForm->BaseLine[1],'f',3));
    BaselineConst->setChecked(mainForm->BaseLineConst);
	
    SolFormat	 ->setCurrentIndex(mainForm->SolFormat);
    TimeFormat	 ->setCurrentIndex(mainForm->TimeFormat);
    TimeDecimal	 ->setText(QString::number(mainForm->TimeDecimal));
    LatLonFormat ->setCurrentIndex(mainForm->LatLonFormat);
    FieldSep	 ->setText(mainForm->FieldSep);
    OutputHead	 ->setCurrentIndex(mainForm->OutputHead);
    OutputOpt	 ->setCurrentIndex(mainForm->OutputOpt);
    OutputDatum  ->setCurrentIndex(mainForm->OutputDatum);
    OutputHeight ->setCurrentIndex(mainForm->OutputHeight);
    OutputGeoid  ->setCurrentIndex(mainForm->OutputGeoid);
    SolStatic    ->setCurrentIndex(mainForm->SolStatic);
    DebugTrace	 ->setCurrentIndex(mainForm->DebugTrace);
    DebugStatus	 ->setCurrentIndex(mainForm->DebugStatus);
	
    MeasErrR1	 ->setText(QString::number(mainForm->MeasErrR1,'f',1));
    MeasErrR2	 ->setText(QString::number(mainForm->MeasErrR2,'f',1));
    MeasErr2	 ->setText(QString::number(mainForm->MeasErr2,'f',3));
    MeasErr3	 ->setText(QString::number(mainForm->MeasErr3,'f',3));
    MeasErr4	 ->setText(QString::number(mainForm->MeasErr4,'f',3));
    MeasErr5	 ->setText(QString::number(mainForm->MeasErr5,'f',3));
    SatClkStab	 ->setText(QString::number(mainForm->SatClkStab,'E',2));
    PrNoise1	 ->setText(QString::number(mainForm->PrNoise1,'E',2));
    PrNoise2	 ->setText(QString::number(mainForm->PrNoise2,'E',2));
    PrNoise3	 ->setText(QString::number(mainForm->PrNoise3,'E',2));
    PrNoise4	 ->setText(QString::number(mainForm->PrNoise4,'E',2));
    PrNoise5	 ->setText(QString::number(mainForm->PrNoise5,'E',2));
	
    RovAntPcv	 ->setChecked(mainForm->RovAntPcv);
    RefAntPcv	 ->setChecked(mainForm->RefAntPcv);
    RovAntE		 ->setText(QString::number(mainForm->RovAntE,'f',4));
    RovAntN		 ->setText(QString::number(mainForm->RovAntN,'f',4));
    RovAntU		 ->setText(QString::number(mainForm->RovAntU,'f',4));
    RefAntE		 ->setText(QString::number(mainForm->RefAntE,'f',4));
    RefAntN		 ->setText(QString::number(mainForm->RefAntN,'f',4));
    RefAntU		 ->setText(QString::number(mainForm->RefAntU,'f',4));
    AntPcvFile	 ->setText(mainForm->AntPcvFile);
	
    RnxOpts1	 ->setText(mainForm->RnxOpts1);
    RnxOpts2	 ->setText(mainForm->RnxOpts2);
    PPPOpts		 ->setText(mainForm->PPPOpts);
	
    IntpRefObs	 ->setCurrentIndex(mainForm->IntpRefObs);
    SbasSat		 ->setText(QString::number(mainForm->SbasSat));
    SatPcvFile   ->setText(mainForm->SatPcvFile);
    StaPosFile	 ->setText(mainForm->StaPosFile);
    GeoidDataFile->setText(mainForm->GeoidDataFile);
    EOPFile		 ->setText(mainForm->EOPFile);
    DCBFile		 ->setText(mainForm->DCBFile);
    BLQFile		 ->setText(mainForm->BLQFile);
    IonoFile	 ->setText(mainForm->IonoFile);
    RovPosType	 ->setCurrentIndex(mainForm->RovPosType);
    RefPosType	 ->setCurrentIndex(mainForm->RefPosType);
    RovPosTypeP					=RovPosType->currentIndex();
    RefPosTypeP					=RefPosType->currentIndex();
    SetPos(RovPosType->currentIndex(),editu,mainForm->RovPos);
    SetPos(RefPosType->currentIndex(),editr,mainForm->RefPos);
	ReadAntList();
	
    RovAnt		 ->setCurrentText(mainForm->RovAnt);
    RefAnt		 ->setCurrentText(mainForm->RefAnt);

    RovList		 ->setPlainText(mainForm->RovList);
    BaseList	 ->setPlainText(mainForm->BaseList);
	
    ExtErr						=mainForm->ExtErr;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SetOpt(void)
{
    QLineEdit *editu[]={RovPos1,RovPos2,RovPos3};
    QLineEdit *editr[]={RefPos1,RefPos2,RefPos3};
	
    mainForm->PosMode		=PosMode	->currentIndex();
    mainForm->Freq			=Freq		->currentIndex();
    mainForm->Solution		=Solution   ->currentIndex();
    mainForm->ElMask		=ElMask	->currentText().toDouble();
    mainForm->SnrMask		=SnrMask;
    mainForm->DynamicModel	=DynamicModel->currentIndex();
    mainForm->TideCorr		=TideCorr	->currentIndex();
    mainForm->IonoOpt	  	=IonoOpt	->currentIndex();
    mainForm->TropOpt	  	=TropOpt	->currentIndex();
    mainForm->SatEphem	  	=SatEphem	->currentIndex();
    mainForm->ExSats	  	=ExSats		->text();
    mainForm->NavSys	  	=0;
    if (NavSys1->isChecked()) mainForm->NavSys|=SYS_GPS;
    if (NavSys2->isChecked()) mainForm->NavSys|=SYS_GLO;
    if (NavSys3->isChecked()) mainForm->NavSys|=SYS_GAL;
    if (NavSys4->isChecked()) mainForm->NavSys|=SYS_QZS;
    if (NavSys5->isChecked()) mainForm->NavSys|=SYS_SBS;
    if (NavSys6->isChecked()) mainForm->NavSys|=SYS_CMP;
    if (NavSys7->isChecked()) mainForm->NavSys|=SYS_IRN;
    mainForm->PosOpt[0]	  	=PosOpt1	->isChecked();
    mainForm->PosOpt[1]	  	=PosOpt2	->isChecked();
    mainForm->PosOpt[2]	  	=PosOpt3	->isChecked();
    mainForm->PosOpt[3]	  	=PosOpt4	->isChecked();
    mainForm->PosOpt[4]	  	=PosOpt5	->isChecked();
    mainForm->PosOpt[5]	  	=PosOpt6	->isChecked();
//	mainForm->MapFunc		=MapFunc	->ItemIndex;
	
    mainForm->AmbRes	  	=AmbRes		->currentIndex();
    mainForm->GloAmbRes	  	=GloAmbRes	->currentIndex();
    mainForm->BdsAmbRes	  	=BdsAmbRes	->currentIndex();
    mainForm->ValidThresAR	=ValidThresAR->text().toDouble();
    mainForm->ThresAR2		=ThresAR2->text().toDouble();
    mainForm->ThresAR3		=ThresAR3->text().toDouble();
    mainForm->OutCntResetAmb=OutCntResetAmb->text().toInt();
    mainForm->FixCntHoldAmb =FixCntHoldAmb->text().toInt();
    mainForm->OutCntResetAmb=OutCntResetAmb->text().toInt();
    mainForm->LockCntFixAmb	=LockCntFixAmb->text().toInt();
    mainForm->ElMaskAR	  	=ElMaskAR   ->text().toInt();
    mainForm->ElMaskHold  	=ElMaskHold ->text().toInt();
    mainForm->MaxAgeDiff  	=MaxAgeDiff ->text().toDouble();
    mainForm->RejectGdop 	=RejectGdop ->text().toDouble();
    mainForm->RejectThres 	=RejectThres->text().toDouble();
    mainForm->SlipThres   	=SlipThres  ->text().toDouble();
    mainForm->ARIter	  	=ARIter		  ->text().toInt();
    mainForm->NumIter	  	=NumIter	  ->text().toInt();
    mainForm->BaseLine[0]  	=BaselineLen->text().toDouble();
    mainForm->BaseLine[1]  	=BaselineSig->text().toDouble();
    mainForm->BaseLineConst	=BaselineConst->isChecked();
	
    mainForm->SolFormat   	=SolFormat  ->currentIndex();
    mainForm->TimeFormat  	=TimeFormat ->currentIndex();
    mainForm->TimeDecimal  	=TimeDecimal->text().toDouble();
    mainForm->LatLonFormat	=LatLonFormat->currentIndex();
    mainForm->FieldSep	  	=FieldSep   ->text();
    mainForm->OutputHead  	=OutputHead ->currentIndex();
    mainForm->OutputOpt   	=OutputOpt  ->currentIndex();
    mainForm->OutputDatum 	=OutputDatum->currentIndex();
    mainForm->OutputHeight	=OutputHeight->currentIndex();
    mainForm->OutputGeoid 	=OutputGeoid->currentIndex();
    mainForm->SolStatic	 	=SolStatic  ->currentIndex();
    mainForm->DebugTrace  	=DebugTrace ->currentIndex();
    mainForm->DebugStatus  	=DebugStatus->currentIndex();
	
    mainForm->MeasErrR1	  =MeasErrR1  ->text().toDouble();
    mainForm->MeasErrR2	  =MeasErrR2  ->text().toDouble();
    mainForm->MeasErr2	  =MeasErr2   ->text().toDouble();
    mainForm->MeasErr3	  =MeasErr3   ->text().toDouble();
    mainForm->MeasErr4	  =MeasErr4   ->text().toDouble();
    mainForm->MeasErr5	  =MeasErr5   ->text().toDouble();
    mainForm->SatClkStab  =SatClkStab ->text().toDouble();
    mainForm->PrNoise1	  =PrNoise1   ->text().toDouble();
    mainForm->PrNoise2	  =PrNoise2   ->text().toDouble();
    mainForm->PrNoise3	  =PrNoise3   ->text().toDouble();
    mainForm->PrNoise4	  =PrNoise4   ->text().toDouble();
    mainForm->PrNoise5	  =PrNoise5   ->text().toDouble();
	
    mainForm->RovAntPcv   =RovAntPcv	->isChecked();
    mainForm->RefAntPcv   =RefAntPcv	->isChecked();
    mainForm->RovAnt	  =RovAnt		->currentText();
    mainForm->RefAnt	  =RefAnt		->currentText();
    mainForm->RovAntE	  =RovAntE	->text().toDouble();
    mainForm->RovAntN	  =RovAntN	->text().toDouble();
    mainForm->RovAntU	  =RovAntU	->text().toDouble();
    mainForm->RefAntE	  =RefAntE	->text().toDouble();
    mainForm->RefAntN	  =RefAntN	->text().toDouble();
    mainForm->RefAntU	  =RefAntU	->text().toDouble();
	
    mainForm->RnxOpts1	  =RnxOpts1		->text();
    mainForm->RnxOpts2	  =RnxOpts2		->text();
    mainForm->PPPOpts	  =PPPOpts		->text();
	
    mainForm->IntpRefObs  =IntpRefObs	->currentIndex();
    mainForm->SbasSat     =SbasSat		->text().toInt();
    mainForm->AntPcvFile  =AntPcvFile	->text();
    mainForm->SatPcvFile  =SatPcvFile	->text();
    mainForm->StaPosFile  =StaPosFile	->text();
    mainForm->GeoidDataFile=GeoidDataFile->text();
    mainForm->EOPFile     =EOPFile		->text();
    mainForm->DCBFile     =DCBFile		->text();
    mainForm->BLQFile     =BLQFile		->text();
    mainForm->IonoFile    =IonoFile		->text();
    mainForm->RovPosType  =RovPosType	->currentIndex();
    mainForm->RefPosType  =RefPosType	->currentIndex();
    GetPos(RovPosType->currentIndex(),editu,mainForm->RovPos);
    GetPos(RefPosType->currentIndex(),editr,mainForm->RefPos);
	
    mainForm->RovList	  =RovList		->toPlainText();
    mainForm->BaseList	  =BaseList		->toPlainText();
	
    mainForm->ExtErr	  =ExtErr;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::LoadOpt(const QString &file)
{
    QLineEdit *editu[]={RovPos1,RovPos2,RovPos3};
    QLineEdit *editr[]={RefPos1,RefPos2,RefPos3};
    QString buff;
    char id[32];
	int sat;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
    filopt_t filopt;
	
    memset(&filopt,0,sizeof(filopt_t));

	resetsysopts();
    if (!loadopts(qPrintable(file),sysopts)) return;
	getsysopts(&prcopt,&solopt,&filopt);
	
    PosMode		 ->setCurrentIndex(prcopt.mode);
    Freq		 ->setCurrentIndex(prcopt.nf>NFREQ-1?NFREQ-1:prcopt.nf-1);
    Solution	 ->setCurrentIndex(prcopt.soltype);
    ElMask		 ->setCurrentText(QString::number(prcopt.elmin*R2D,'f',0));
	SnrMask						=prcopt.snrmask;
    DynamicModel ->setCurrentIndex(prcopt.dynamics);
    TideCorr	 ->setCurrentIndex(prcopt.tidecorr);
    IonoOpt		 ->setCurrentIndex(prcopt.ionoopt);
    TropOpt		 ->setCurrentIndex(prcopt.tropopt);
    SatEphem	 ->setCurrentIndex(prcopt.sateph);
    ExSats	     ->setText("");
    for (sat=1;sat<=MAXSAT;sat++) {
		if (!prcopt.exsats[sat-1]) continue;
		satno2id(sat,id);
        buff+=QString("%1%2%3").arg(buff.isEmpty()?"":" ").arg(prcopt.exsats[sat-1]==2?"+":"").arg(id);
	}
    ExSats		 ->setText(buff);
    NavSys1	     ->setChecked(prcopt.navsys&SYS_GPS);
    NavSys2	     ->setChecked(prcopt.navsys&SYS_GLO);
    NavSys3	     ->setChecked(prcopt.navsys&SYS_GAL);
    NavSys4	     ->setChecked(prcopt.navsys&SYS_QZS);
    NavSys5	     ->setChecked(prcopt.navsys&SYS_SBS);
    NavSys6	     ->setChecked(prcopt.navsys&SYS_CMP);
    PosOpt1	     ->setChecked(prcopt.posopt[0]);
    PosOpt2	     ->setChecked(prcopt.posopt[1]);
    PosOpt3	     ->setChecked(prcopt.posopt[2]);
    PosOpt4	     ->setChecked(prcopt.posopt[3]);
    PosOpt5	     ->setChecked(prcopt.posopt[4]);
    PosOpt6	     ->setChecked(prcopt.posopt[5]);
//	MapFunc	     ->ItemIndex	=prcopt.mapfunc;
	
    AmbRes		 ->setCurrentIndex(prcopt.modear);
    GloAmbRes	 ->setCurrentIndex(prcopt.glomodear);
    BdsAmbRes	 ->setCurrentIndex(prcopt.bdsmodear);
    ValidThresAR ->setText(QString::number(prcopt.thresar[0],'g',3));
    ThresAR2	 ->setText(QString::number(prcopt.thresar[1],'g',9));
    ThresAR3	 ->setText(QString::number(prcopt.thresar[2],'g',3));
    OutCntResetAmb->setText(QString::number(prcopt.maxout));
    FixCntHoldAmb->setText(QString::number(prcopt.minfix));
    LockCntFixAmb  ->setText(QString::number(prcopt.minlock));
    ElMaskAR	 ->setText(QString::number(prcopt.elmaskar*R2D,'f',0));
    ElMaskHold	 ->setText(QString::number(prcopt.elmaskhold*R2D,'f',0));
    MaxAgeDiff	 ->setText(QString::number(prcopt.maxtdiff,'f',1));
    RejectGdop   ->setText(QString::number(prcopt.maxgdop,'f',1));
    RejectThres  ->setText(QString::number(prcopt.maxinno,'f',1));
    SlipThres	 ->setText(QString::number(prcopt.thresslip,'f',3));
    ARIter		 ->setText(QString::number(prcopt.armaxiter));
    NumIter		 ->setText(QString::number(prcopt.niter));
    BaselineLen	 ->setText(QString::number(prcopt.baseline[0],'f',3));
    BaselineSig	 ->setText(QString::number(prcopt.baseline[1],'f',3));
    BaselineConst->setChecked(prcopt.baseline[0]>0.0);
	
    SolFormat	 ->setCurrentIndex(solopt.posf);
    TimeFormat	 ->setCurrentIndex(solopt.timef==0?0:solopt.times+1);
    TimeDecimal	 ->setText(QString::number(solopt.timeu));
    LatLonFormat ->setCurrentIndex(solopt.degf);
    FieldSep	 ->setText(solopt.sep);
    OutputHead	 ->setCurrentIndex(solopt.outhead);
    OutputOpt	 ->setCurrentIndex(solopt.outopt);
    OutputDatum  ->setCurrentIndex(solopt.datum);
    OutputHeight ->setCurrentIndex(solopt.height);
    OutputGeoid  ->setCurrentIndex(solopt.geoid);
    SolStatic    ->setCurrentIndex(solopt.solstatic);
    NmeaIntv1	 ->setText(QString::number(solopt.nmeaintv[0],'g',2));
    NmeaIntv2	 ->setText(QString::number(solopt.nmeaintv[1],'g',2));
    DebugTrace	 ->setCurrentIndex(solopt.trace);
    DebugStatus	 ->setCurrentIndex(solopt.sstat);
	
    MeasErrR1	 ->setText(QString::number(prcopt.eratio[0],'f',1));
    MeasErrR2	 ->setText(QString::number(prcopt.eratio[1],'f',1));
    MeasErr2	 ->setText(QString::number(prcopt.err[1],'f',3));
    MeasErr3	 ->setText(QString::number(prcopt.err[2],'f',3));
    MeasErr4	 ->setText(QString::number(prcopt.err[3],'f',3));
    MeasErr5	 ->setText(QString::number(prcopt.err[4],'f',3));
    SatClkStab	 ->setText(QString::number(prcopt.sclkstab,'E',2));
    PrNoise1	 ->setText(QString::number(prcopt.prn[0],'E',2));
    PrNoise2	 ->setText(QString::number(prcopt.prn[1],'E',2));
    PrNoise3	 ->setText(QString::number(prcopt.prn[2],'E',2));
    PrNoise4	 ->setText(QString::number(prcopt.prn[3],'E',2));
    PrNoise5	 ->setText(QString::number(prcopt.prn[4],'E',2));
	
    RovAntPcv	 ->setChecked(*prcopt.anttype[0]);
    RefAntPcv	 ->setChecked(*prcopt.anttype[1]);
    RovAnt		 ->setCurrentText(prcopt.anttype[0]);
    RefAnt		 ->setCurrentText(prcopt.anttype[1]);
    RovAntE		 ->setText(QString::number(prcopt.antdel[0][0],'f',4));
    RovAntN		 ->setText(QString::number(prcopt.antdel[0][1],'f',4));
    RovAntU		 ->setText(QString::number(prcopt.antdel[0][2],'f',4));
    RefAntE		 ->setText(QString::number(prcopt.antdel[1][0],'f',4));
    RefAntN		 ->setText(QString::number(prcopt.antdel[1][1],'f',4));
    RefAntU		 ->setText(QString::number(prcopt.antdel[1][2],'f',4));
	
    RnxOpts1	 ->setText(prcopt.rnxopt[0]);
    RnxOpts2	 ->setText(prcopt.rnxopt[1]);
    PPPOpts		 ->setText(prcopt.pppopt);
	
    IntpRefObs	 ->setCurrentIndex(prcopt.intpref);
    SbasSat		 ->setText(QString::number(prcopt.sbassatsel));
    RovPosType	 ->setCurrentIndex(prcopt.rovpos==0?0:prcopt.rovpos+2);
    RefPosType	 ->setCurrentIndex(prcopt.refpos==0?0:prcopt.refpos+2);
    RovPosTypeP					=RovPosType->currentIndex();
    RefPosTypeP					=RefPosType->currentIndex();
    SetPos(RovPosType->currentIndex(),editu,prcopt.ru);
    SetPos(RefPosType->currentIndex(),editr,prcopt.rb);
	
    SatPcvFile ->setText(filopt.satantp);
    AntPcvFile ->setText(filopt.rcvantp);
    StaPosFile ->setText(filopt.stapos);
    GeoidDataFile->setText(filopt.geoid);
    EOPFile	   ->setText(filopt.eop);
    DCBFile	   ->setText(filopt.dcb);
    BLQFile	   ->setText(filopt.blq);
    IonoFile   ->setText(filopt.iono);
	
	ReadAntList();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SaveOpt(const QString &file)
{
    QString ExSats_Text=ExSats->text(),FieldSep_Text=FieldSep->text();
    QString RovAnt_Text=RovAnt->currentText(),RefAnt_Text=RefAnt->currentText();
    QString SatPcvFile_Text=SatPcvFile->text();
    QString AntPcvFile_Text=AntPcvFile->text();
    QString StaPosFile_Text=StaPosFile->text();
    QString GeoidDataFile_Text=GeoidDataFile->text();
    QString EOPFile_Text=EOPFile->text();
    QString DCBFile_Text=DCBFile->text();
    QString BLQFile_Text=BLQFile->text();
    QString IonoFile_Text=IonoFile->text();
    QString RnxOpts1_Text=RnxOpts1->text();
    QString RnxOpts2_Text=RnxOpts2->text();
    QString PPPOpts_Text=PPPOpts->text();
    QLineEdit *editu[]={RovPos1,RovPos2,RovPos3};
    QLineEdit *editr[]={RefPos1,RefPos2,RefPos3};
    char buff[1024],*p,comment[256],s[64];
	int sat,ex;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
    filopt_t filopt;
	
    memset(&filopt,0,sizeof(filopt_t));

    prcopt.mode		=PosMode	 ->currentIndex();
    prcopt.nf		=Freq		 ->currentIndex()+1;
    prcopt.soltype	=Solution	 ->currentIndex();
    prcopt.elmin	=ElMask	->currentText().toDouble()*D2R;
	prcopt.snrmask	=SnrMask;
    prcopt.dynamics	=DynamicModel->currentIndex();
    prcopt.tidecorr	=TideCorr	 ->currentIndex();
    prcopt.ionoopt	=IonoOpt	 ->currentIndex();
    prcopt.tropopt	=TropOpt	 ->currentIndex();
    prcopt.sateph	=SatEphem	 ->currentIndex();
    if (ExSats->text()!="") {
        strcpy(buff,qPrintable(ExSats_Text));
		for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
			if (*p=='+') {ex=2; p++;} else ex=1;
			if (!(sat=satid2no(p))) continue;
			prcopt.exsats[sat-1]=ex;
		}
	}
    prcopt.navsys	= (NavSys1->isChecked()?SYS_GPS:0)|
                      (NavSys2->isChecked()?SYS_GLO:0)|
                      (NavSys3->isChecked()?SYS_GAL:0)|
                      (NavSys4->isChecked()?SYS_QZS:0)|
                      (NavSys5->isChecked()?SYS_SBS:0)|
                      (NavSys6->isChecked()?SYS_CMP:0);
    prcopt.posopt[0]=PosOpt1	->isChecked();
    prcopt.posopt[1]=PosOpt2	->isChecked();
    prcopt.posopt[2]=PosOpt3	->isChecked();
    prcopt.posopt[3]=PosOpt4	->isChecked();
    prcopt.posopt[4]=PosOpt5	->isChecked();
    prcopt.posopt[5]=PosOpt6	->isChecked();
//	prcopt.mapfunc	=MapFunc	->ItemIndex;
	
    prcopt.modear	=AmbRes		->currentIndex();
    prcopt.glomodear=GloAmbRes	->currentIndex();
    prcopt.bdsmodear=BdsAmbRes	->currentIndex();
    prcopt.thresar[0]=ValidThresAR->text().toDouble();
    prcopt.thresar[1]=ThresAR2->text().toDouble();
    prcopt.thresar[2]=ThresAR3->text().toDouble();
    prcopt.maxout	=OutCntResetAmb->text().toDouble();
    prcopt.minfix	=FixCntHoldAmb->text().toDouble();
    prcopt.minlock	=LockCntFixAmb->text().toDouble();
    prcopt.elmaskar	=ElMaskAR	->text().toDouble()*D2R;
    prcopt.elmaskhold=ElMaskHold->text().toDouble()*D2R;
    prcopt.maxtdiff	=MaxAgeDiff	->text().toDouble();
    prcopt.maxgdop	=RejectGdop ->text().toDouble();
    prcopt.maxinno	=RejectThres->text().toDouble();
    prcopt.thresslip=SlipThres	->text().toDouble();
    prcopt.armaxiter=ARIter		->text().toDouble();
    prcopt.niter	=NumIter	->text().toDouble();
    if (prcopt.mode==PMODE_MOVEB&&BaselineConst->isChecked()) {
        prcopt.baseline[0]=BaselineLen->text().toDouble();
        prcopt.baseline[1]=BaselineSig->text().toDouble();
	}
    solopt.posf		=SolFormat	->currentIndex();
    solopt.timef	=TimeFormat	->currentIndex()==0?0:1;
    solopt.times	=TimeFormat	->currentIndex()==0?0:TimeFormat->currentIndex()-1;
    solopt.timeu	=TimeDecimal ->text().toDouble();
    solopt.degf		=LatLonFormat->currentIndex();
    strcpy(solopt.sep,qPrintable(FieldSep_Text));
    solopt.outhead	=OutputHead	 ->currentIndex();
    solopt.outopt	=OutputOpt	 ->currentIndex();
    solopt.datum	=OutputDatum ->currentIndex();
    solopt.height	=OutputHeight->currentIndex();
    solopt.geoid	=OutputGeoid ->currentIndex();
    solopt.solstatic=SolStatic   ->currentIndex();
    solopt.nmeaintv[0]=NmeaIntv1->text().toDouble();
    solopt.nmeaintv[1]=NmeaIntv2->text().toDouble();
    solopt.trace	=DebugTrace	 ->currentIndex();
    solopt.sstat	=DebugStatus ->currentIndex();
	
    prcopt.eratio[0]=MeasErrR1->text().toDouble();
    prcopt.eratio[1]=MeasErrR2->text().toDouble();
    prcopt.err[1]	=MeasErr2->text().toDouble();
    prcopt.err[2]	=MeasErr3->text().toDouble();
    prcopt.err[3]	=MeasErr4->text().toDouble();
    prcopt.err[4]	=MeasErr5->text().toDouble();
    prcopt.sclkstab	=SatClkStab->text().toDouble();
    prcopt.prn[0]	=PrNoise1->text().toDouble();
    prcopt.prn[1]	=PrNoise2->text().toDouble();
    prcopt.prn[2]	=PrNoise3->text().toDouble();
    prcopt.prn[3]	=PrNoise4->text().toDouble();
    prcopt.prn[4]	=PrNoise5->text().toDouble();
	
    if (RovAntPcv->isChecked()) strcpy(prcopt.anttype[0],qPrintable(RovAnt_Text));
    if (RefAntPcv->isChecked()) strcpy(prcopt.anttype[1],qPrintable(RefAnt_Text));
    prcopt.antdel[0][0]=RovAntE->text().toDouble();
    prcopt.antdel[0][1]=RovAntN->text().toDouble();
    prcopt.antdel[0][2]=RovAntU->text().toDouble();
    prcopt.antdel[1][0]=RefAntE->text().toDouble();
    prcopt.antdel[1][1]=RefAntN->text().toDouble();
    prcopt.antdel[1][2]=RefAntU->text().toDouble();
	
    prcopt.intpref	=IntpRefObs->currentIndex();
    prcopt.sbassatsel=SbasSat->text().toInt();
    prcopt.rovpos=RovPosType->currentIndex()<3?0:RovPosType->currentIndex()-2;
    prcopt.refpos=RefPosType->currentIndex()<3?0:RefPosType->currentIndex()-2;
    if (prcopt.rovpos==0) GetPos(RovPosType->currentIndex(),editu,prcopt.ru);
    if (prcopt.refpos==0) GetPos(RefPosType->currentIndex(),editr,prcopt.rb);
	
    strcpy(prcopt.rnxopt[0],qPrintable(RnxOpts1_Text));
    strcpy(prcopt.rnxopt[1],qPrintable(RnxOpts2_Text));
    strcpy(prcopt.pppopt,qPrintable(PPPOpts_Text));
	
    strcpy(filopt.satantp,qPrintable(SatPcvFile_Text));
    strcpy(filopt.rcvantp,qPrintable(AntPcvFile_Text));
    strcpy(filopt.stapos, qPrintable(StaPosFile_Text));
    strcpy(filopt.geoid,  qPrintable(GeoidDataFile_Text));
    strcpy(filopt.eop,    qPrintable(EOPFile_Text));
    strcpy(filopt.dcb,    qPrintable(DCBFile_Text));
    strcpy(filopt.blq,    qPrintable(BLQFile_Text));
    strcpy(filopt.iono,   qPrintable(IonoFile_Text));
	
	time2str(utc2gpst(timeget()),s,0);
    sprintf(comment,"rtkpost_qt options (%s, v.%s %s)",s,VER_RTKLIB,PATCH_LEVEL);
	setsysopts(&prcopt,&solopt,&filopt);
    if (!saveopts(qPrintable(file),"w",comment,sysopts)) return;
}
//---------------------------------------------------------------------------
void OptDialog::UpdateEnable(void)
{
    bool rel=PMODE_DGPS<=PosMode->currentIndex()&&PosMode->currentIndex()<=PMODE_FIXED;
    bool rtk=PMODE_KINEMA<=PosMode->currentIndex()&&PosMode->currentIndex()<=PMODE_FIXED;
    bool ppp=PosMode->currentIndex()>=PMODE_PPP_KINEMA;
    bool ar=rtk||ppp;
	
    Freq           ->setEnabled(rel||ppp);
    Solution       ->setEnabled(rel||ppp);
    DynamicModel   ->setEnabled(rel);
    TideCorr       ->setEnabled(rel||ppp);
    //IonoOpt        ->setEnabled(!ppp);
    PosOpt1        ->setEnabled(ppp);
    PosOpt2        ->setEnabled(ppp);
    PosOpt3        ->setEnabled(ppp);
    PosOpt4        ->setEnabled(ppp);
    PosOpt6        ->setEnabled(ppp);
	
    AmbRes         ->setEnabled(ar);
    GloAmbRes      ->setEnabled(ar&&AmbRes->currentIndex()>0&&NavSys2->isChecked());
    BdsAmbRes      ->setEnabled(ar&&AmbRes->currentIndex()>0&&NavSys6->isChecked());
    ValidThresAR   ->setEnabled(ar&&AmbRes->currentIndex()>=1&&AmbRes->currentIndex()<4);
    ThresAR2	   ->setEnabled(ar&&AmbRes->currentIndex()>=4);
    ThresAR3	   ->setEnabled(ar&&AmbRes->currentIndex()>=4);
    LockCntFixAmb  ->setEnabled(ar&&AmbRes->currentIndex()>=1);
    ElMaskAR       ->setEnabled(ar&&AmbRes->currentIndex()>=1);
    OutCntResetAmb ->setEnabled(ar||ppp);
    FixCntHoldAmb  ->setEnabled(ar&&AmbRes->currentIndex()==3);
    ElMaskHold     ->setEnabled(ar&&AmbRes->currentIndex()==3);
    SlipThres      ->setEnabled(rtk||ppp);
    MaxAgeDiff     ->setEnabled(rel);
    RejectThres    ->setEnabled(rel||ppp);
    ARIter         ->setEnabled(ppp);
    NumIter        ->setEnabled(rel||ppp);
    BaselineConst  ->setEnabled(PosMode->currentIndex()==PMODE_MOVEB);
    BaselineLen    ->setEnabled(BaselineConst->isChecked()&&PosMode->currentIndex()==PMODE_MOVEB);
    BaselineSig    ->setEnabled(BaselineConst->isChecked()&&PosMode->currentIndex()==PMODE_MOVEB);
	
    OutputHead     ->setEnabled(SolFormat->currentIndex()<3);
    OutputOpt      ->setEnabled(SolFormat->currentIndex()<3);
    TimeFormat     ->setEnabled(SolFormat->currentIndex()<3);
    TimeDecimal    ->setEnabled(SolFormat->currentIndex()<3);
    LatLonFormat   ->setEnabled(SolFormat->currentIndex()==0);
    FieldSep       ->setEnabled(SolFormat->currentIndex()<3);
    OutputDatum    ->setEnabled(SolFormat->currentIndex()==0);
    OutputHeight   ->setEnabled(SolFormat->currentIndex()==0);
    OutputGeoid    ->setEnabled(SolFormat->currentIndex()==0&&OutputHeight->currentIndex()==1);
    SolStatic      ->setEnabled(PosMode->currentIndex()==PMODE_STATIC||
                             PosMode->currentIndex()==PMODE_PPP_STATIC);
	
    RovAntPcv      ->setEnabled(rel||ppp);
    RovAnt         ->setEnabled((rel||ppp)&&RovAntPcv->isChecked());
    RovAntE        ->setEnabled((rel||ppp)&&RovAntPcv->isChecked());
    RovAntN        ->setEnabled((rel||ppp)&&RovAntPcv->isChecked());
    RovAntU        ->setEnabled((rel||ppp)&&RovAntPcv->isChecked());
    LabelRovAntD   ->setEnabled((rel||ppp)&&RovAntPcv->isChecked());
    RefAntPcv      ->setEnabled(rel);
    RefAnt         ->setEnabled(rel&&RefAntPcv->isChecked());
    RefAntE        ->setEnabled(rel&&RefAntPcv->isChecked());
    RefAntN        ->setEnabled(rel&&RefAntPcv->isChecked());
    RefAntU        ->setEnabled(rel&&RefAntPcv->isChecked());
    LabelRefAntD   ->setEnabled(rel&&RefAntPcv->isChecked());
	
    RovPosType     ->setEnabled(PosMode->currentIndex()==PMODE_FIXED||PosMode->currentIndex()==PMODE_PPP_FIXED);
    RovPos1        ->setEnabled(RovPosType->isEnabled()&&RovPosType->currentIndex()<=2);
    RovPos2        ->setEnabled(RovPosType->isEnabled()&&RovPosType->currentIndex()<=2);
    RovPos3        ->setEnabled(RovPosType->isEnabled()&&RovPosType->currentIndex()<=2);
    BtnRovPos      ->setEnabled(RovPosType->isEnabled()&&RovPosType->currentIndex()<=2);
	
    RefPosType     ->setEnabled(rel&&PosMode->currentIndex()!=PMODE_MOVEB);
    RefPos1        ->setEnabled(RefPosType->isEnabled()&&RefPosType->currentIndex()<=2);
    RefPos2        ->setEnabled(RefPosType->isEnabled()&&RefPosType->currentIndex()<=2);
    RefPos3        ->setEnabled(RefPosType->isEnabled()&&RefPosType->currentIndex()<=2);
    BtnRefPos      ->setEnabled(RefPosType->isEnabled()&&RefPosType->currentIndex()<=2);
}
//---------------------------------------------------------------------------
void OptDialog::GetPos(int type, QLineEdit **edit, double *pos)
{
    QString edit0_Text=edit[0]->text();
    QString edit1_Text=edit[1]->text();
	double p[3]={0},dms1[3]={0},dms2[3]={0};
	
	if (type==1) { /* lat/lon/height dms/m */
        QStringList tokens=edit0_Text.split(' ');
        if (tokens.size()==3)
            for (int i=0;i<3;i++) dms1[i]=tokens.at(i).toDouble();
        tokens=edit1_Text.split(' ');
        if (tokens.size()==3)
            for (int i=0;i<3;i++) dms2[i]=tokens.at(i).toDouble();
		p[0]=(dms1[0]<0?-1:1)*(fabs(dms1[0])+dms1[1]/60+dms1[2]/3600)*D2R;
		p[1]=(dms2[0]<0?-1:1)*(fabs(dms2[0])+dms2[1]/60+dms2[2]/3600)*D2R;
        p[2]=edit[2]->text().toDouble();
		pos2ecef(p,pos);
	}
	else if (type==2) { /* x/y/z-ecef */
        pos[0]=edit[0]->text().toDouble();
        pos[1]=edit[1]->text().toDouble();
        pos[2]=edit[2]->text().toDouble();
	}
	else {
        p[0]=edit[0]->text().toDouble()*D2R;
        p[1]=edit[1]->text().toDouble()*D2R;
        p[2]=edit[2]->text().toDouble();
		pos2ecef(p,pos);
	}
}
//---------------------------------------------------------------------------
void OptDialog::SetPos(int type, QLineEdit **edit, double *pos)
{
	double p[3],dms1[3],dms2[3],s1,s2;
	
	if (type==1) { /* lat/lon/height dms/m */
		ecef2pos(pos,p); s1=p[0]<0?-1:1; s2=p[1]<0?-1:1;
		p[0]=fabs(p[0])*R2D+1E-12; p[1]=fabs(p[1])*R2D+1E-12;
		dms1[0]=floor(p[0]); p[0]=(p[0]-dms1[0])*60.0;
		dms1[1]=floor(p[0]); dms1[2]=(p[0]-dms1[1])*60.0;
		dms2[0]=floor(p[1]); p[1]=(p[1]-dms2[0])*60.0;
		dms2[1]=floor(p[1]); dms2[2]=(p[1]-dms2[1])*60.0;
        edit[0]->setText(QString("%1 %2 %3").arg(s1*dms1[0],0,'f',0).arg(dms1[1],2,'f',0).arg(dms1[2],9,'f',6));
        edit[1]->setText(QString("%1 %2 %3").arg(s2*dms2[0],0,'f',0).arg(dms2[1],2,'f',0).arg(dms2[2],9,'f',6));
        edit[2]->setText(QString("%1").arg(p[2],0,'f',4));
	}
	else if (type==2) { /* x/y/z-ecef */
        edit[0]->setText(QString::number(pos[0],'f',4));
        edit[1]->setText(QString::number(pos[1],'f',4));
        edit[2]->setText(QString::number(pos[2],'f',4));
	}
	else {
		ecef2pos(pos,p);
        edit[0]->setText(QString::number(p[0]*R2D,'f',9));
        edit[1]->setText(QString::number(p[1]*R2D,'f',9));
        edit[2]->setText(QString::number(p[2],'f',4));
	}
}
//---------------------------------------------------------------------------
void OptDialog::ReadAntList(void)
{
    QString AntPcvFile_Text=AntPcvFile->text();
    pcvs_t pcvs={0,0,0};
	char *p;
	
    if (!readpcv(qPrintable(AntPcvFile_Text),&pcvs)) return;
	
    RovAnt->clear();
    RefAnt->clear();

    RovAnt->addItem("");RefAnt->addItem("");
    RovAnt->addItem("*");RefAnt->addItem("*");

	for (int i=0;i<pcvs.n;i++) {
		if (pcvs.pcv[i].sat) continue;
		if ((p=strchr(pcvs.pcv[i].type,' '))) *p='\0';
		if (i>0&&!strcmp(pcvs.pcv[i].type,pcvs.pcv[i-1].type)) continue;
        RovAnt->addItem(pcvs.pcv[i].type);
        RefAnt->addItem(pcvs.pcv[i].type);
    }
	
	free(pcvs.pcv);
}
//---------------------------------------------------------------------------
void OptDialog::BtnHelpClick()
{
    KeyDialog *keyDialog=new KeyDialog(this);
    keyDialog->Flag=2;
    keyDialog->exec();

    delete keyDialog;
}
//---------------------------------------------------------------------------
void OptDialog::ExtEna0Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::ExtEna1Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::ExtEna2Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::BtnExtOptClick()
{
    ExtOptDialog *extOptDialog= new ExtOptDialog(this);
    extOptDialog->exec();

    delete extOptDialog;
}
//---------------------------------------------------------------------------
void OptDialog::BtnMaskClick()
{
    MaskOptDialog *maskOptDialog= new MaskOptDialog(this);
    maskOptDialog->Mask=SnrMask;
    maskOptDialog->exec();
    if (maskOptDialog->result()!=QDialog::Accepted) return;
    SnrMask=maskOptDialog->Mask;

    delete  maskOptDialog;
}
//---------------------------------------------------------------------------
void OptDialog::NavSys6Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------


