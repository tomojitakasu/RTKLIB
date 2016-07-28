//---------------------------------------------------------------------------
#include <QFileDialog>
#include <QLineEdit>
#include <QPoint>
#include <QFontDialog>
#include <QFont>
#include <QShowEvent>
#include <QFileSystemModel>
#include <QCompleter>

#include "rtklib.h"
#include "naviopt.h"
#include "viewer.h"
#include "refdlg.h"
#include "navimain.h"
#include "maskoptdlg.h"

//---------------------------------------------------------------------------
#define MAXSTR      1024                /* max length of a string */

// receiver options table ---------------------------------------------------
static int strtype[]={                  /* stream types */
    STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE
};
static char strpath[8][MAXSTR]={""};    /* stream paths */
static int strfmt[]={                   /* stream formats */
    STRFMT_RTCM3,STRFMT_RTCM3,STRFMT_SP3,SOLF_LLH,SOLF_NMEA,0,0,0
};
static int svrcycle     =10;            /* server cycle (ms) */
static int timeout      =10000;         /* timeout time (ms) */
static int reconnect    =10000;         /* reconnect interval (ms) */
static int nmeacycle    =5000;          /* nmea request cycle (ms) */
static int fswapmargin  =30;            /* file swap marign (s) */
static int buffsize     =32768;         /* input buffer size (bytes) */
static int navmsgsel    =0;             /* navigation mesaage select */
static int nmeareq      =0;             /* nmea request type (0:off,1:lat/lon,2:single) */
static double nmeapos[] ={0,0};         /* nmea position (lat/lon) (deg) */
static char proxyaddr[MAXSTR]="";       /* proxy address */

#define TIMOPT  "0:gpst,1:utc,2:jst,3:tow"
#define CONOPT  "0:dms,1:deg,2:xyz,3:enu,4:pyl"
#define FLGOPT  "0:off,1:std+2:age/ratio/ns"
#define ISTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,7:ntripcli,8:ftp,9:http"
#define OSTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr"
#define FMTOPT  "0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,10:nvs,11:binex,12:rt17,13:sbf,14:cmr,17:sp3"
#define NMEOPT  "0:off,1:latlon,2:single"
#define SOLOPT  "0:llh,1:xyz,2:enu,3:nmea"
#define MSGOPT  "0:all,1:rover,2:base,3:corr"

static opt_t rcvopts[]={
    {"inpstr1-type",    3,  (void *)&strtype[0],         ISTOPT },
    {"inpstr2-type",    3,  (void *)&strtype[1],         ISTOPT },
    {"inpstr3-type",    3,  (void *)&strtype[2],         ISTOPT },
    {"inpstr1-path",    2,  (void *)strpath [0],         ""     },
    {"inpstr2-path",    2,  (void *)strpath [1],         ""     },
    {"inpstr3-path",    2,  (void *)strpath [2],         ""     },
    {"inpstr1-format",  3,  (void *)&strfmt [0],         FMTOPT },
    {"inpstr2-format",  3,  (void *)&strfmt [1],         FMTOPT },
    {"inpstr3-format",  3,  (void *)&strfmt [2],         FMTOPT },
    {"inpstr2-nmeareq", 3,  (void *)&nmeareq,            NMEOPT },
    {"inpstr2-nmealat", 1,  (void *)&nmeapos[0],         "deg"  },
    {"inpstr2-nmealon", 1,  (void *)&nmeapos[1],         "deg"  },
    {"outstr1-type",    3,  (void *)&strtype[3],         OSTOPT },
    {"outstr2-type",    3,  (void *)&strtype[4],         OSTOPT },
    {"outstr1-path",    2,  (void *)strpath [3],         ""     },
    {"outstr2-path",    2,  (void *)strpath [4],         ""     },
    {"outstr1-format",  3,  (void *)&strfmt [3],         SOLOPT },
    {"outstr2-format",  3,  (void *)&strfmt [4],         SOLOPT },
    {"logstr1-type",    3,  (void *)&strtype[5],         OSTOPT },
    {"logstr2-type",    3,  (void *)&strtype[6],         OSTOPT },
    {"logstr3-type",    3,  (void *)&strtype[7],         OSTOPT },
    {"logstr1-path",    2,  (void *)strpath [5],         ""     },
    {"logstr2-path",    2,  (void *)strpath [6],         ""     },
    {"logstr3-path",    2,  (void *)strpath [7],         ""     },
    
    {"misc-svrcycle",   0,  (void *)&svrcycle,           "ms"   },
    {"misc-timeout",    0,  (void *)&timeout,            "ms"   },
    {"misc-reconnect",  0,  (void *)&reconnect,          "ms"   },
    {"misc-nmeacycle",  0,  (void *)&nmeacycle,          "ms"   },
    {"misc-buffsize",   0,  (void *)&buffsize,           "bytes"},
    {"misc-navmsgsel",  3,  (void *)&navmsgsel,          MSGOPT },
    {"misc-proxyaddr",  2,  (void *)proxyaddr,           ""     },
    {"misc-fswapmargin",0,  (void *)&fswapmargin,        "s"    },
    
    {"",0,NULL,""}
};
//---------------------------------------------------------------------------
OptDialog::OptDialog(QWidget* parent)
    : QDialog(parent)
{
    QString label;
    int freq[]={1,2,5,6,7,8,9},nglo=MAXPRNGLO,ngal=MAXPRNGAL,nqzs=MAXPRNQZS;
    int ncmp=MAXPRNCMP,nirn=MAXPRNIRN;

    setupUi(this);

	PrcOpt=prcopt_default;
	SolOpt=solopt_default;

    textViewer =new TextViewer(this);

    UpdateEnable();
	
    Freq->clear();
	for (int i=0;i<NFREQ;i++) {
        label=label+(i>0?"+":"")+QString("L%1").arg(freq[i]);
        Freq->addItem(label);
	}
    if (nglo<=0) NavSys2->setEnabled(false);
    if (ngal<=0) NavSys3->setEnabled(false);
    if (nqzs<=0) NavSys4->setEnabled(false);
    if (ncmp<=0) NavSys6->setEnabled(false);
    if (nirn<=0) NavSys7->setEnabled(false);

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
    OLFile->setCompleter(fileCompleter);
    TLEFile->setCompleter(fileCompleter);
    TLESatFile->setCompleter(fileCompleter);

    QCompleter *dirCompleter=new QCompleter(this);
    QFileSystemModel *dirModel=new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs|QDir::Drives|QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    LocalDir->setCompleter(dirCompleter);

	UpdateEnable();

    connect(BtnAntPcvFile,SIGNAL(clicked(bool)),this,SLOT(BtnAntPcvFileClick()));
    connect(BtnAntPcvView,SIGNAL(clicked(bool)),this,SLOT(BtnAntPcvViewClick()));
    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnDCBFile,SIGNAL(clicked(bool)),this,SLOT(BtnDCBFileClick()));
    connect(BtnEOPFile,SIGNAL(clicked(bool)),this,SLOT(BtnEOPFileClick()));
    connect(BtnEOPView,SIGNAL(clicked(bool)),this,SLOT(BtnEOPViewClick()));
    connect(BtnFont,SIGNAL(clicked(bool)),this,SLOT(BtnFontClick()));
    connect(BtnGeoidDataFile,SIGNAL(clicked(bool)),this,SLOT(BtnGeoidDataFileClick()));
    connect(BtnLoad,SIGNAL(clicked(bool)),this,SLOT(BtnLoadClick()));
    connect(BtnLocalDir,SIGNAL(clicked(bool)),this,SLOT(BtnLocalDirClick()));
    connect(BtnOk,SIGNAL(clicked(bool)),this,SLOT(BtnOkClick()));
//    connect(BtnOLFile,SIGNAL(clicked(bool)),this,SLOT(Btn));//Ocean Load
    connect(BtnRefPos,SIGNAL(clicked(bool)),this,SLOT(BtnRefPosClick()));
    connect(BtnRovPos,SIGNAL(clicked(bool)),this,SLOT(BtnRovPosClick()));
    connect(BtnSatPcvFile,SIGNAL(clicked(bool)),this,SLOT(BtnSatPcvFileClick()));
    connect(BtnSatPcvView,SIGNAL(clicked(bool)),this,SLOT(BtnSatPcvViewClick()));
    connect(BtnSave,SIGNAL(clicked(bool)),this,SLOT(BtnSaveClick()));
    connect(BtnSnrMask,SIGNAL(clicked(bool)),this,SLOT(BtnSnrMaskClick()));
    connect(BtnStaPosFile,SIGNAL(clicked(bool)),this,SLOT(BtnStaPosFileClick()));
    connect(BtnStaPosView,SIGNAL(clicked(bool)),this,SLOT(BtnStaPosViewClick()));
    connect(BtnTLEFile,SIGNAL(clicked(bool)),this,SLOT(BtnTLEFileClick()));
    connect(BtnTLESatFile,SIGNAL(clicked(bool)),this,SLOT(BtnTLESatFileClick()));
    connect(PosMode,SIGNAL(currentIndexChanged(int)),this,SLOT(PosModeChange(int)));
    connect(SolFormat,SIGNAL(currentIndexChanged(int)),this,SLOT(SolFormatChange(int)));
    connect(RefPosTypeP,SIGNAL(currentIndexChanged(int)),this,SLOT(RefPosTypePChange(int)));
    connect(RovPosTypeP,SIGNAL(currentIndexChanged(int)),this,SLOT(RovPosTypePChange(int)));
    connect(AmbRes,SIGNAL(currentIndexChanged(int)),this,SLOT(AmbResChange(int)));
    connect(RovAntPcv,SIGNAL(clicked(bool)),this,SLOT(RovAntPcvClick()));
    connect(RefAntPcv,SIGNAL(clicked(bool)),this,SLOT(RovAntPcvClick()));
    connect(OutputHeight,SIGNAL(currentIndexChanged(int)),this,SLOT(OutputHeightClick()));
    connect(NavSys2,SIGNAL(clicked(bool)),this,SLOT(NavSys2Click()));
    connect(NavSys6,SIGNAL(clicked(bool)),this,SLOT(NavSys6Click()));
    connect(BaselineConst,SIGNAL(clicked(bool)),this,SLOT(BaselineConstClick()));
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
    QString fileName;
    fileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Load Options..."),QString(),tr("Options File (*.conf);;All (*.*)")));

    LoadOpt(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnSaveClick()
{
    QString file;

    file=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Save Options..."),QString(),tr("Options File (*.conf);;All (*.*)")));

    if (!file.contains('.')) file+=".conf";

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
    QString fileName;

    fileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Station Postion File"),QString(),tr("Position File (*.pos);;All (*.*)")));

    StaPosFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnSnrMaskClick()
{
    MaskOptDialog maskOptDialog(this);
    maskOptDialog.Mask=PrcOpt.snrmask;

    maskOptDialog.exec();
    if (maskOptDialog.result()!=QDialog::Accepted) return;
    PrcOpt.snrmask=maskOptDialog.Mask;
}
//---------------------------------------------------------------------------
void OptDialog::RovPosTypePChange(int)
{
    QLineEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double pos[3];

    GetPos(RovPosTypeF,edit,pos);
    SetPos(RovPosTypeP->currentIndex(),edit,pos);
    RovPosTypeF=RovPosTypeP->currentIndex();
	UpdateEnable();

}
//---------------------------------------------------------------------------
void OptDialog::RefPosTypePChange(int)
{
    QLineEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double pos[3];

	GetPos(RefPosTypeF,edit,pos);
    SetPos(RefPosTypeP->currentIndex(),edit,pos);
    RefPosTypeF=RefPosTypeP->currentIndex();

	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::BtnRovPosClick()
{
    RefDialog refDialog(this);
    QLineEdit *edit[]={RovPos1,RovPos2,RovPos3};
    double p[3],posi[3];

    GetPos(RovPosTypeP->currentIndex(),edit,p);
    ecef2pos(p,posi);

    refDialog.RovPos[0]=posi[0]*R2D;
    refDialog.RovPos[1]=posi[1]*R2D;
    refDialog.Pos[2]=posi[2];
    refDialog.StaPosFile=StaPosFile->text();
    refDialog.move(pos().x()+size().width()/2-refDialog.size().width()/2,
        pos().y()+size().height()/2-refDialog.size().height()/2);

    refDialog.exec();
    if (refDialog.result()!=QDialog::Accepted) return;

    posi[0]=refDialog.Pos[0]*D2R;
    posi[1]=refDialog.Pos[1]*D2R;
    posi[2]=refDialog.Pos[2];

    pos2ecef(posi,p);
    SetPos(RovPosTypeP->currentIndex(),edit,p);
}
//---------------------------------------------------------------------------
void OptDialog::BtnRefPosClick()
{
    RefDialog refDialog(this);
    QLineEdit *edit[]={RefPos1,RefPos2,RefPos3};
    double p[3],posi[3];

    GetPos(RefPosTypeP->currentIndex(),edit,p);
    ecef2pos(p,posi);
    refDialog.RovPos[0]=posi[0]*R2D;
    refDialog.RovPos[1]=posi[1]*R2D;
    refDialog.RovPos[2]=posi[2];
    refDialog.StaPosFile=StaPosFile->text();
    refDialog.move(pos().x()+size().width()/2-refDialog.size().width()/2,
        pos().y()+size().height()/2-refDialog.size().height()/2);

    refDialog.exec();
    if (refDialog.result()!=QDialog::Accepted) return;

    posi[0]=refDialog.Pos[0]*D2R;
    posi[1]=refDialog.Pos[1]*D2R;
    posi[2]=refDialog.Pos[2];

    pos2ecef(posi,p);
    SetPos(RefPosTypeP->currentIndex(),edit,p);
}
//---------------------------------------------------------------------------
void OptDialog::BtnSatPcvViewClick()
{
    if (SatPcvFile->text()=="") return;

    textViewer->show();

    textViewer->Read(SatPcvFile->text());
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

    textViewer->show();

    textViewer->Read(AntPcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnAntPcvFileClick()
{
    AntPcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Receiver Antenna PCV File"),QString(),tr("PCV File (*.pcv *.atx);;All (*.*)"))));
    ReadAntList();
}
//---------------------------------------------------------------------------
void OptDialog::BtnGeoidDataFileClick()
{
    QString fileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Geoid Data File"),QString(),tr("All (*.*)")));
    GeoidDataFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnDCBFileClick()
{
    QString fileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("DCB Data File"),QString(),tr("DCB (*.dcb)")));
    DCBFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnEOPFileClick()
{
    QString fileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("EOP Data File"),QString(),tr("EOP (*.erp)")));
    EOPFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnEOPViewClick()
{
    if (EOPFile->text()=="") return;

    textViewer->show();

    textViewer->Read(EOPFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnLocalDirClick()
{
    QString dir=LocalDir->text();
    dir=QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,tr("FTP/HTTP Local Directory"),dir));
    LocalDir->setText(dir);
}
//---------------------------------------------------------------------------
void OptDialog::BtnFontClick()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(FontLabel->font());
    dialog.exec();

    FontLabel->setFont(dialog.selectedFont());
    FontLabel->setText(FontLabel->font().family()+QString::number(FontLabel->font().pointSize())+" pt");
}
//---------------------------------------------------------------------------
void OptDialog::BtnTLESatFileClick()
{
    QString fileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("TLE Satellite Number File"),QString(),tr("All (*.*)")));

    TLESatFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnTLEFileClick()
{
    QString fileName=QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("TLE Data File"),QString(),tr("All (*.*)")));

    TLEFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::FreqChange()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::NavSys2Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::BaselineConstClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SolFormatChange(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::PosModeChange(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::AmbResChange(int)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RovAntPcvClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::OutputHeightClick()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::GetOpt(void)
{
    QLineEdit *editu[]={RovPos1,RovPos2,RovPos3};
    QLineEdit *editr[]={RefPos1,RefPos2,RefPos3};
	
    PosMode		 ->setCurrentIndex(PrcOpt.mode);
    Freq		 ->setCurrentIndex(PrcOpt.nf-1>NFREQ-1?NFREQ-1:PrcOpt.nf-1);
    ElMask		 ->setCurrentIndex(ElMask->findText(QString::number(PrcOpt.elmin*R2D)));
    DynamicModel ->setCurrentIndex(PrcOpt.dynamics);
    TideCorr	 ->setCurrentIndex(PrcOpt.tidecorr);
    IonoOpt		 ->setCurrentIndex(PrcOpt.ionoopt);
    TropOpt		 ->setCurrentIndex(PrcOpt.tropopt);
    SatEphem	 ->setCurrentIndex(PrcOpt.sateph);
    AmbRes		 ->setCurrentIndex(PrcOpt.modear);
    GloAmbRes	 ->setCurrentIndex(PrcOpt.glomodear);
    BdsAmbRes	 ->setCurrentIndex(PrcOpt.bdsmodear);
    ValidThresAR ->setText(QString::number(PrcOpt.thresar[0]));
    OutCntResetAmb->setText(QString::number(PrcOpt.maxout));
    LockCntFixAmb->setText(QString::number(PrcOpt.minlock));
    FixCntHoldAmb->setText(QString::number(PrcOpt.minfix));
    ElMaskAR	 ->setText(QString::number(PrcOpt.elmaskar*R2D));
    ElMaskHold	 ->setText(QString::number(PrcOpt.elmaskhold*R2D));
    MaxAgeDiff	 ->setText(QString::number(PrcOpt.maxtdiff));
    RejectGdop   ->setText(QString::number(PrcOpt.maxgdop));
    RejectThres  ->setText(QString::number(PrcOpt.maxinno));
    SlipThres	 ->setText(QString::number(PrcOpt.thresslip));
    NumIter		 ->setText(QString::number(PrcOpt.niter));
    SyncSol		 ->setCurrentIndex(PrcOpt.syncsol);
    ExSatsE		 ->setText(ExSats);
    NavSys1		 ->setChecked(PrcOpt.navsys&SYS_GPS);
    NavSys2		 ->setChecked(PrcOpt.navsys&SYS_GLO);
    NavSys3		 ->setChecked(PrcOpt.navsys&SYS_GAL);
    NavSys4		 ->setChecked(PrcOpt.navsys&SYS_QZS);
    NavSys5		 ->setChecked(PrcOpt.navsys&SYS_SBS);
    NavSys6		 ->setChecked(PrcOpt.navsys&SYS_CMP);
    NavSys7		 ->setChecked(PrcOpt.navsys&SYS_IRN);
    PosOpt1		 ->setChecked(PrcOpt.posopt[0]);
    PosOpt2		 ->setChecked(PrcOpt.posopt[1]);
    PosOpt3		 ->setChecked(PrcOpt.posopt[2]);
    PosOpt4		 ->setChecked(PrcOpt.posopt[3]);
    PosOpt5		 ->setChecked(PrcOpt.posopt[4]);
	
    SolFormat	 ->setCurrentIndex(SolOpt.posf);
    TimeFormat	 ->setCurrentIndex(SolOpt.timef==0?0:SolOpt.times+1);
    TimeDecimal	 ->setText(QString::number(SolOpt.timeu));
    LatLonFormat ->setCurrentIndex(SolOpt.degf);
    FieldSep	 ->setText(SolOpt.sep);
    OutputHead	 ->setCurrentIndex(SolOpt.outhead);
    OutputOpt	 ->setCurrentIndex(SolOpt.outopt);
    OutputDatum  ->setCurrentIndex(SolOpt.datum);
    OutputHeight ->setCurrentIndex(SolOpt.height);
    OutputGeoid  ->setCurrentIndex(SolOpt.geoid);
    NmeaIntv1    ->setText(QString::number(SolOpt.nmeaintv[0]));
    NmeaIntv2    ->setText(QString::number(SolOpt.nmeaintv[1]));
    DebugStatus	 ->setCurrentIndex(DebugStatusF);
    DebugTrace	 ->setCurrentIndex(DebugTraceF);
	
    BaselineConst->setChecked(BaselineC);
    BaselineLen->setText(QString::number(Baseline[0]));
    BaselineSig->setText(QString::number(Baseline[1]));
	
    MeasErrR1	 ->setText(QString::number(PrcOpt.eratio[0]));
    MeasErrR2	 ->setText(QString::number(PrcOpt.eratio[1]));
    MeasErr2	 ->setText(QString::number(PrcOpt.err[1]));
    MeasErr3	 ->setText(QString::number(PrcOpt.err[2]));
    MeasErr4	 ->setText(QString::number(PrcOpt.err[3]));
    MeasErr5	 ->setText(QString::number(PrcOpt.err[4]));
    PrNoise1	 ->setText(QString::number(PrcOpt.prn[0]));
    PrNoise2	 ->setText(QString::number(PrcOpt.prn[1]));
    PrNoise3	 ->setText(QString::number(PrcOpt.prn[2]));
    PrNoise4	 ->setText(QString::number(PrcOpt.prn[3]));
    PrNoise5	 ->setText(QString::number(PrcOpt.prn[4]));
    SatClkStab	 ->setText(QString::number(PrcOpt.sclkstab));
    MaxAveEp	 ->setValue(PrcOpt.maxaveep);
    ChkInitRestart->setChecked(PrcOpt.initrst);
	
    RovPosTypeP	 ->setCurrentIndex(RovPosTypeF);
    RefPosTypeP	 ->setCurrentIndex(RefPosTypeF);
    RovAntPcv	 ->setChecked(RovAntPcvF);
    RefAntPcv	 ->setChecked(RefAntPcvF);
    RovAntE		 ->setText(QString::number(RovAntDel[0]));
    RovAntN		 ->setText(QString::number(RovAntDel[1]));
    RovAntU		 ->setText(QString::number(RovAntDel[2]));
    RefAntE		 ->setText(QString::number(RefAntDel[0]));
    RefAntN		 ->setText(QString::number(RefAntDel[1]));
    RefAntU		 ->setText(QString::number(RefAntDel[2]));
    SetPos(RovPosTypeP->currentIndex(),editu,RovPos);
    SetPos(RefPosTypeP->currentIndex(),editr,RefPos);
	
    SatPcvFile	 ->setText(SatPcvFileF);
    AntPcvFile	 ->setText(AntPcvFileF);
    StaPosFile	 ->setText(StaPosFileF);
    GeoidDataFile->setText(GeoidDataFileF);
    DCBFile      ->setText(DCBFileF);
    EOPFile      ->setText(EOPFileF);
    TLEFile      ->setText(TLEFileF);
    TLESatFile   ->setText(TLESatFileF);
    LocalDir	 ->setText(LocalDirectory);
	ReadAntList();

    RovAnt		 ->setCurrentIndex(RovAnt->findText(RovAntF));
    RefAnt		 ->setCurrentIndex(RefAnt->findText(RefAntF));

    SvrCycleE	 ->setText(QString::number(SvrCycle));
    TimeoutTimeE ->setText(QString::number(TimeoutTime));
    ReconTimeE   ->setText(QString::number(ReconTime));
    NmeaCycleE   ->setText(QString::number(NmeaCycle));
    FileSwapMarginE->setText(QString::number(FileSwapMargin));
    SvrBuffSizeE ->setText(QString::number(SvrBuffSize));
    SolBuffSizeE ->setText(QString::number(SolBuffSize));
    SavedSolE    ->setText(QString::number(SavedSol));
    NavSelectS   ->setCurrentIndex(NavSelect);
    SbasSatE     ->setText(QString::number(PrcOpt.sbassatsel));
    ProxyAddrE   ->setText(ProxyAddr);
    MoniPortE    ->setText(QString::number(MoniPort));
    SolBuffSizeE ->setText(QString::number(SolBuffSize));
    PanelStackE  ->setCurrentIndex(PanelStack);
	
    FontLabel->setFont(PosFont);
    FontLabel->setText(FontLabel->font().family()+QString::number(FontLabel->font().pointSize())+"pt");

	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SetOpt(void)
{
    QString FieldSep_Text=FieldSep->text();
    QLineEdit *editu[]={RovPos1,RovPos2,RovPos3};
    QLineEdit *editr[]={RefPos1,RefPos2,RefPos3};
	
    PrcOpt.mode      =PosMode     ->currentIndex();
    PrcOpt.nf        =Freq        ->currentIndex()+1;
    PrcOpt.elmin     =ElMask      ->currentText().toDouble()*D2R;
    PrcOpt.dynamics  =DynamicModel->currentIndex();
    PrcOpt.tidecorr  =TideCorr    ->currentIndex();
    PrcOpt.ionoopt   =IonoOpt     ->currentIndex();
    PrcOpt.tropopt   =TropOpt     ->currentIndex();
    PrcOpt.sateph    =SatEphem    ->currentIndex();
    PrcOpt.modear    =AmbRes      ->currentIndex();
    PrcOpt.glomodear =GloAmbRes   ->currentIndex();
    PrcOpt.bdsmodear =BdsAmbRes   ->currentIndex();
    PrcOpt.thresar[0]=ValidThresAR->text().toDouble();
    PrcOpt.maxout    =OutCntResetAmb->text().toInt();
    PrcOpt.minlock   =LockCntFixAmb->text().toInt();
    PrcOpt.minfix    =FixCntHoldAmb->text().toInt();
    PrcOpt.elmaskar  =ElMaskAR   ->text().toDouble()*D2R;
    PrcOpt.elmaskhold=ElMaskHold ->text().toDouble()*D2R;
    PrcOpt.maxtdiff  =MaxAgeDiff ->text().toDouble();
    PrcOpt.maxgdop   =RejectGdop ->text().toDouble();
    PrcOpt.maxinno   =RejectThres->text().toDouble();
    PrcOpt.thresslip =SlipThres  ->text().toDouble();
    PrcOpt.niter     =NumIter     ->text().toInt();
    PrcOpt.syncsol   =SyncSol     ->currentIndex();
    ExSats			 =ExSatsE	  ->text();
	PrcOpt.navsys    =0;

    if (NavSys1->isChecked()) PrcOpt.navsys|=SYS_GPS;
    if (NavSys2->isChecked()) PrcOpt.navsys|=SYS_GLO;
    if (NavSys3->isChecked()) PrcOpt.navsys|=SYS_GAL;
    if (NavSys4->isChecked()) PrcOpt.navsys|=SYS_QZS;
    if (NavSys5->isChecked()) PrcOpt.navsys|=SYS_SBS;
    if (NavSys6->isChecked()) PrcOpt.navsys|=SYS_CMP;
    if (NavSys7->isChecked()) PrcOpt.navsys|=SYS_IRN;
    PrcOpt.posopt[0] =PosOpt1   ->isChecked();
    PrcOpt.posopt[1] =PosOpt2   ->isChecked();
    PrcOpt.posopt[2] =PosOpt3   ->isChecked();
    PrcOpt.posopt[3] =PosOpt4   ->isChecked();
    PrcOpt.posopt[4] =PosOpt5   ->isChecked();
	
    SolOpt.posf      =SolFormat   ->currentIndex();
    SolOpt.timef     =TimeFormat->currentIndex()==0?0:1;
    SolOpt.times     =TimeFormat->currentIndex()==0?0:TimeFormat->currentIndex()-1;
    SolOpt.timeu     =static_cast<int>(TimeDecimal->text().toDouble());
    SolOpt.degf      =LatLonFormat->currentIndex();
    strcpy(SolOpt.sep,qPrintable(FieldSep_Text));
    SolOpt.outhead   =OutputHead  ->currentIndex();
    SolOpt.outopt    =OutputOpt   ->currentIndex();
    SolOpt.datum     =OutputDatum ->currentIndex();
    SolOpt.height    =OutputHeight->currentIndex();
    SolOpt.geoid     =OutputGeoid ->currentIndex();
    SolOpt.nmeaintv[0]=NmeaIntv1->text().toDouble();
    SolOpt.nmeaintv[1]=NmeaIntv2->text().toDouble();
    DebugStatusF     =DebugStatus ->currentIndex();
    DebugTraceF      =DebugTrace  ->currentIndex();
	
    BaselineC        =BaselineConst->isChecked();
    Baseline[0]      =BaselineLen->text().toDouble();
    Baseline[1]      =BaselineSig->text().toDouble();
	
    PrcOpt.eratio[0] =MeasErrR1 ->text().toDouble();
    PrcOpt.eratio[1] =MeasErrR2 ->text().toDouble();
    PrcOpt.err[1]    =MeasErr2  ->text().toDouble();
    PrcOpt.err[2]    =MeasErr3  ->text().toDouble();
    PrcOpt.err[3]    =MeasErr4  ->text().toDouble();
    PrcOpt.err[4]    =MeasErr5  ->text().toDouble();
    PrcOpt.prn[0]    =PrNoise1  ->text().toDouble();
    PrcOpt.prn[1]    =PrNoise2  ->text().toDouble();
    PrcOpt.prn[2]    =PrNoise3  ->text().toDouble();
    PrcOpt.prn[3]    =PrNoise4  ->text().toDouble();
    PrcOpt.prn[4]    =PrNoise5  ->text().toDouble();
    PrcOpt.sclkstab  =SatClkStab->text().toDouble();
    PrcOpt.maxaveep  =MaxAveEp  ->text().toInt();
    PrcOpt.initrst   =ChkInitRestart->isChecked();
	
    RovPosTypeF      =RovPosTypeP ->currentIndex();
    RefPosTypeF      =RefPosTypeP ->currentIndex();
    RovAntPcvF       =RovAntPcv   ->isChecked();
    RefAntPcvF       =RefAntPcv   ->isChecked();
    RovAntF          =RovAnt      ->currentText();
    RefAntF          =RefAnt      ->currentText();
    RovAntDel[0]     =RovAntE   ->text().toDouble();
    RovAntDel[1]     =RovAntN   ->text().toDouble();
    RovAntDel[2]     =RovAntU   ->text().toDouble();
    RefAntDel[0]     =RefAntE   ->text().toDouble();
    RefAntDel[1]     =RefAntN   ->text().toDouble();
    RefAntDel[2]     =RefAntU   ->text().toDouble();
    GetPos(RovPosTypeP->currentIndex(),editu,RovPos);
    GetPos(RefPosTypeP->currentIndex(),editr,RefPos);
	
    SatPcvFileF      =SatPcvFile  ->text();
    AntPcvFileF      =AntPcvFile  ->text();
    StaPosFileF      =StaPosFile  ->text();
    GeoidDataFileF   =GeoidDataFile->text();
    DCBFileF         =DCBFile     ->text();
    EOPFileF         =EOPFile     ->text();
    TLEFileF         =TLEFile     ->text();
    TLESatFileF      =TLESatFile  ->text();
    LocalDirectory   =LocalDir    ->text();
	
    SvrCycle	     =SvrCycleE   ->text().toInt();
    TimeoutTime      =TimeoutTimeE->text().toInt();
    ReconTime        =ReconTimeE  ->text().toInt();
    NmeaCycle	     =NmeaCycleE  ->text().toInt();
    FileSwapMargin   =FileSwapMarginE->text().toInt();
    SvrBuffSize      =SvrBuffSizeE->text().toInt();
    SolBuffSize      =SolBuffSizeE->text().toInt();
    SavedSol         =SavedSolE   ->text().toInt();
    NavSelect        =NavSelectS  ->currentIndex();
    PrcOpt.sbassatsel=SbasSatE    ->text().toInt();
    ProxyAddr        =ProxyAddrE  ->text();
    MoniPort         =MoniPortE   ->text().toInt();
    PanelStack       =PanelStackE ->currentIndex();
    PosFont=FontLabel->font();

	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::LoadOpt(const QString &file)
{
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
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
    if (!loadopts(qPrintable(file),sysopts)||
        !loadopts(qPrintable(file),rcvopts)) return;
	getsysopts(&prcopt,&solopt,&filopt);
	
	for (int i=0;i<8;i++) {
        mainForm->StreamC[i]=strtype[i]!=STR_NONE;
        mainForm->Stream[i]=STR_NONE;
		for (int j=0;j<(i<3?7:5);j++) {
			if (strtype[i]!=(i<3?itype[j]:otype[j])) continue;
            mainForm->Stream[i]=j;
			break;
		}
        if (i<5) mainForm->Format[i]=strfmt[i];
		
		if (strtype[i]==STR_SERIAL) {
            mainForm->Paths[i][0]=strpath[i];
		}
		else if (strtype[i]==STR_FILE) {
            mainForm->Paths[i][2]=strpath[i];
		}
		else if (strtype[i]<=STR_NTRIPCLI) {
            mainForm->Paths[i][1]=strpath[i];
		}
		else if (strtype[i]<=STR_HTTP) {
            mainForm->Paths[i][3]=strpath[i];
		}
	}
    mainForm->NmeaReq=nmeareq;
    mainForm->NmeaPos[0]=nmeapos[0];
    mainForm->NmeaPos[1]=nmeapos[1];
	
    SbasSatE     ->setText(QString::number(prcopt.sbassatsel));
	
    PosMode		 ->setCurrentIndex(prcopt.mode);
    Freq		 ->setCurrentIndex(prcopt.nf>NFREQ-1?NFREQ-1:prcopt.nf-1);
    Solution	 ->setCurrentIndex(prcopt.soltype);
    ElMask		 ->setCurrentIndex(ElMask->findText(QString::number(prcopt.elmin*R2D,'f',0)));
    DynamicModel ->setCurrentIndex(prcopt.dynamics);
    TideCorr	 ->setCurrentIndex(prcopt.tidecorr);
    IonoOpt		 ->setCurrentIndex(prcopt.ionoopt);
    TropOpt		 ->setCurrentIndex(prcopt.tropopt);
    SatEphem	 ->setCurrentIndex(prcopt.sateph);
    ExSatsE	     ->setText("");
    for (sat=1;sat<=MAXSAT;sat++) {
		if (!prcopt.exsats[sat-1]) continue;
		satno2id(sat,id);
        buff+=QString(buff.isEmpty()?"":" ")+(prcopt.exsats[sat-1]==2?"+":"")+id;
	}
    ExSatsE		 ->setText(buff);
    NavSys1	     ->setChecked(prcopt.navsys&SYS_GPS);
    NavSys2	     ->setChecked(prcopt.navsys&SYS_GLO);
    NavSys3	     ->setChecked(prcopt.navsys&SYS_GAL);
    NavSys4	     ->setChecked(prcopt.navsys&SYS_QZS);
    NavSys5	     ->setChecked(prcopt.navsys&SYS_SBS);
    NavSys6	     ->setChecked(prcopt.navsys&SYS_CMP);
    NavSys7	     ->setChecked(prcopt.navsys&SYS_IRN);
    PosOpt1		 ->setChecked(prcopt.posopt[0]);
    PosOpt2		 ->setChecked(prcopt.posopt[1]);
    PosOpt3		 ->setChecked(prcopt.posopt[2]);
    PosOpt4		 ->setChecked(prcopt.posopt[3]);
    PosOpt5		 ->setChecked(prcopt.posopt[4]);
	
    AmbRes		 ->setCurrentIndex(prcopt.modear);
    GloAmbRes	 ->setCurrentIndex(prcopt.glomodear);
    BdsAmbRes	 ->setCurrentIndex(prcopt.bdsmodear);
    ValidThresAR ->setText(QString::number(prcopt.thresar[0],'f',1));
    OutCntResetAmb->setText(QString::number(prcopt.maxout   ));
    FixCntHoldAmb->setText(QString::number(prcopt.minfix   ));
    LockCntFixAmb->setText(QString::number(prcopt.minlock  ));
    ElMaskAR	 ->setText(QString::number(prcopt.elmaskar*R2D,'f',0));
    ElMaskHold	 ->setText(QString::number(prcopt.elmaskhold*R2D,'f',0));
    MaxAgeDiff	 ->setText(QString::number(prcopt.maxtdiff ,'f',1));
    RejectGdop   ->setText(QString::number(prcopt.maxgdop  ,'f',1));
    RejectThres  ->setText(QString::number(prcopt.maxinno  ,'f',1));
    SlipThres	 ->setText(QString::number(prcopt.thresslip,'f',3));
    NumIter		 ->setText(QString::number(prcopt.niter    ));
    SyncSol		 ->setCurrentIndex(prcopt.syncsol);
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
    NmeaIntv1	 ->setText(QString::number(solopt.nmeaintv[0],'g',2));
    NmeaIntv2	 ->setText(QString::number(solopt.nmeaintv[1],'g',2));
    DebugTrace	 ->setCurrentIndex(solopt.trace);
    DebugStatus	 ->setCurrentIndex(solopt.sstat);
	
    MeasErrR1	 ->setText(QString::number(prcopt.eratio[0],'f',1));
    MeasErrR2	 ->setText(QString::number(prcopt.eratio[1],'f',3));
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
    RovAnt		 ->setCurrentIndex(RovAnt->findText(prcopt.anttype[0]));
    RefAnt		 ->setCurrentIndex(RefAnt->findText(prcopt.anttype[1]));
    RovAntE		 ->setText(QString::number(prcopt.antdel[0][0],'f',4));
    RovAntN		 ->setText(QString::number(prcopt.antdel[0][1],'f',4));
    RovAntU		 ->setText(QString::number(prcopt.antdel[0][2],'f',4));
    RefAntE		 ->setText(QString::number(prcopt.antdel[1][0],'f',4));
    RefAntN		 ->setText(QString::number(prcopt.antdel[1][1],'f',4));
    RefAntU		 ->setText(QString::number(prcopt.antdel[1][2],'f',4));
    MaxAveEp	 ->setValue(prcopt.maxaveep);
	
    ChkInitRestart->setChecked(prcopt.initrst);

    RovPosTypeP	 ->setCurrentIndex(0);
    RefPosTypeP	 ->setCurrentIndex(0);
    if      (prcopt.refpos==POSOPT_RTCM  ) RefPosTypeP->setCurrentIndex(3);
    else if (prcopt.refpos==POSOPT_RAW   ) RefPosTypeP->setCurrentIndex(4);
    else if (prcopt.refpos==POSOPT_SINGLE) RefPosTypeP->setCurrentIndex(5);

    RovPosTypeF					=RovPosTypeP->currentIndex();
    RefPosTypeF					=RefPosTypeP->currentIndex();
    SetPos(RovPosTypeP->currentIndex(),editu,prcopt.ru);
    SetPos(RefPosTypeP->currentIndex(),editr,prcopt.rb);
	
    SatPcvFile ->setText(filopt.satantp);
    AntPcvFile ->setText(filopt.rcvantp);
    StaPosFile ->setText(filopt.stapos);
    GeoidDataFile->setText(filopt.geoid);
    DCBFile    ->setText(filopt.dcb);
    LocalDir   ->setText(filopt.tempdir);
	
	ReadAntList();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SaveOpt(const QString &file)
{
    QString ProxyAddrE_Text=ProxyAddrE->text();
    QString ExSatsE_Text=ExSatsE->text();
    QString FieldSep_Text=FieldSep->text();
    QString RovAnt_Text=RovAnt->currentText(),RefAnt_Text=RefAnt->currentText();
    QString SatPcvFile_Text=SatPcvFile->text();
    QString AntPcvFile_Text=AntPcvFile->text();
    QString StaPosFile_Text=StaPosFile->text();
    QString GeoidDataFile_Text=GeoidDataFile->text();
    QString DCBFile_Text=DCBFile->text();
    QString LocalDir_Text=LocalDir->text();
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    QLineEdit *editu[]={RovPos1,RovPos2,RovPos3};
    QLineEdit *editr[]={RefPos1,RefPos2,RefPos3};
    char buff[1024],*p,comment[256],s[64];
	int sat,ex;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
    filopt_t filopt;
	
    memset(&filopt,0,sizeof(filopt_t));

	for (int i=0;i<8;i++) {
        strtype[i]=i<3?itype[mainForm->Stream[i]]:otype[mainForm->Stream[i]];
        strfmt[i]=mainForm->Format[i];
		
        if (!mainForm->StreamC[i]) {
			strtype[i]=STR_NONE;
			strcpy(strpath[i],"");
		}
		else if (strtype[i]==STR_SERIAL) {
            strcpy(strpath[i],qPrintable(mainForm->Paths[i][0]));
		}
		else if (strtype[i]==STR_FILE) {
            strcpy(strpath[i],qPrintable(mainForm->Paths[i][2]));
		}
		else if (strtype[i]<=STR_NTRIPCLI) {
            strcpy(strpath[i],qPrintable(mainForm->Paths[i][1]));
		}
		else if (strtype[i]<=STR_HTTP) {
            strcpy(strpath[i],qPrintable(mainForm->Paths[i][3]));
		}
	}
    nmeareq   =mainForm->NmeaReq;
    nmeapos[0]=mainForm->NmeaPos[0];
    nmeapos[1]=mainForm->NmeaPos[1];

    svrcycle    =SvrCycleE   ->text().toInt();
    timeout     =TimeoutTimeE->text().toInt();
    reconnect   =ReconTimeE  ->text().toInt();
    nmeacycle   =NmeaCycleE  ->text().toInt();
    buffsize    =SvrBuffSizeE->text().toInt();
    navmsgsel   =NavSelectS  ->currentIndex();
    strcpy(proxyaddr,qPrintable(ProxyAddrE_Text));
    fswapmargin =FileSwapMarginE->text().toInt();
    prcopt.sbassatsel=SbasSatE->text().toInt();

    prcopt.mode		=PosMode	 ->currentIndex();
    prcopt.nf		=Freq		 ->currentIndex()+1;
    prcopt.soltype	=Solution	 ->currentIndex();
    prcopt.elmin	=ElMask	->currentText().toDouble()*D2R;
    prcopt.dynamics	=DynamicModel->currentIndex();
    prcopt.tidecorr	=TideCorr	 ->currentIndex();
    prcopt.ionoopt	=IonoOpt	 ->currentIndex();
    prcopt.tropopt	=TropOpt	 ->currentIndex();
    prcopt.sateph	=SatEphem	 ->currentIndex();
    if (ExSatsE->text()!="") {
        strcpy(buff,qPrintable(ExSatsE_Text));
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
                      (NavSys6->isChecked()?SYS_CMP:0)|
                      (NavSys7->isChecked()?SYS_IRN:0);
    prcopt.posopt[0]=PosOpt1->isChecked();
    prcopt.posopt[1]=PosOpt2->isChecked();
    prcopt.posopt[2]=PosOpt3->isChecked();
    prcopt.posopt[3]=PosOpt4->isChecked();
    prcopt.posopt[4]=PosOpt5->isChecked();
	
    prcopt.modear	=AmbRes		->currentIndex();
    prcopt.glomodear=GloAmbRes	->currentIndex();
    prcopt.bdsmodear=BdsAmbRes	->currentIndex();
    prcopt.thresar[0]=ValidThresAR->text().toDouble();
    prcopt.maxout	=OutCntResetAmb->text().toInt();
    prcopt.minfix	=FixCntHoldAmb->text().toInt();
    prcopt.minlock	=LockCntFixAmb->text().toInt();
    prcopt.elmaskar	=ElMaskAR	->text().toDouble()*D2R;
    prcopt.elmaskhold=ElMaskHold->text().toDouble()*D2R;
    prcopt.maxtdiff	=MaxAgeDiff	->text().toDouble();
    prcopt.maxgdop	=RejectGdop ->text().toDouble();
    prcopt.maxinno	=RejectThres->text().toDouble();
    prcopt.thresslip=SlipThres	->text().toDouble();
    prcopt.niter	=NumIter	->text().toInt();
    prcopt.syncsol	=SyncSol->currentIndex();
    if (prcopt.mode==PMODE_MOVEB&&BaselineConst->isChecked()) {
        prcopt.baseline[0]=BaselineLen->text().toDouble();
        prcopt.baseline[1]=BaselineSig->text().toDouble();
	}
    solopt.posf		=SolFormat	->currentIndex();
    solopt.timef	=TimeFormat	->currentIndex()==0?0:1;
    solopt.times	=TimeFormat	->currentIndex()==0?0:TimeFormat->currentIndex()-1;
    solopt.timeu	=TimeDecimal ->text().toInt();
    solopt.degf		=LatLonFormat->currentIndex();
    strcpy(solopt.sep,qPrintable(FieldSep_Text));
    solopt.outhead	=OutputHead	 ->currentIndex();
    solopt.outopt	=OutputOpt	 ->currentIndex();
    solopt.datum	=OutputDatum ->currentIndex();
    solopt.height	=OutputHeight->currentIndex();
    solopt.geoid	=OutputGeoid ->currentIndex();
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
    prcopt.maxaveep=MaxAveEp->text().toInt();
    prcopt.initrst=ChkInitRestart->isChecked();
	
    prcopt.rovpos=POSOPT_POS;
    prcopt.refpos=POSOPT_POS;
    if      (RefPosTypeP->currentIndex()==3) prcopt.refpos=POSOPT_RTCM;
    else if (RefPosTypeP->currentIndex()==4) prcopt.refpos=POSOPT_RAW;
    else if (RefPosTypeP->currentIndex()==5) prcopt.refpos=POSOPT_SINGLE;
	
    if (prcopt.rovpos==POSOPT_POS) GetPos(RovPosTypeP->currentIndex(),editu,prcopt.ru);
    if (prcopt.refpos==POSOPT_POS) GetPos(RefPosTypeP->currentIndex(),editr,prcopt.rb);
	
    strcpy(filopt.satantp,qPrintable(SatPcvFile_Text));
    strcpy(filopt.rcvantp,qPrintable(AntPcvFile_Text));
    strcpy(filopt.stapos, qPrintable(StaPosFile_Text));
    strcpy(filopt.geoid,  qPrintable(GeoidDataFile_Text));
    strcpy(filopt.dcb,    qPrintable(DCBFile_Text));
    strcpy(filopt.tempdir,qPrintable(LocalDir_Text));
	
	time2str(utc2gpst(timeget()),s,0);
    sprintf(comment,qPrintable(tr("RTKNAVI options (%s, v.%s)")),s,VER_RTKLIB);
	setsysopts(&prcopt,&solopt,&filopt);
    if (!saveopts(qPrintable(file),"w",comment,sysopts)||
        !saveopts(qPrintable(file),"a","",rcvopts)) return;
}
//---------------------------------------------------------------------------
void OptDialog::UpdateEnable(void)
{
    bool rel=PMODE_DGPS<=PosMode->currentIndex()&&PosMode->currentIndex()<=PMODE_FIXED;
    bool rtk=PMODE_KINEMA<=PosMode->currentIndex()&&PosMode->currentIndex()<=PMODE_FIXED;
    bool ppp=PosMode->currentIndex()>=PMODE_PPP_KINEMA;
    bool ar=rtk||ppp;
	
    Freq           ->setEnabled(rel);
    Solution       ->setEnabled(false);
    DynamicModel   ->setEnabled(rel);
    TideCorr       ->setEnabled(rel||ppp);
    PosOpt1        ->setEnabled(ppp);
    PosOpt2        ->setEnabled(ppp);
    PosOpt3        ->setEnabled(ppp);
    PosOpt4        ->setEnabled(ppp);
	
    AmbRes         ->setEnabled(ar);
    GloAmbRes      ->setEnabled(ar&&AmbRes->currentIndex()>0&&NavSys2->isChecked());
    BdsAmbRes      ->setEnabled(ar&&AmbRes->currentIndex()>0&&NavSys6->isChecked());
    ValidThresAR   ->setEnabled(ar&&AmbRes->currentIndex()>=1&&AmbRes->currentIndex()<4);
    ThresAR2       ->setEnabled(ar&&AmbRes->currentIndex()>=4);
    ThresAR3       ->setEnabled(ar&&AmbRes->currentIndex()>=4);
    LockCntFixAmb  ->setEnabled(ar&&AmbRes->currentIndex()>=1);
    ElMaskAR       ->setEnabled(ar&&AmbRes->currentIndex()>=1);
    OutCntResetAmb ->setEnabled(ar||ppp);
    FixCntHoldAmb  ->setEnabled(ar&&AmbRes->currentIndex()==3);
    ElMaskHold     ->setEnabled(ar&&AmbRes->currentIndex()==3);
    SlipThres      ->setEnabled(ar||ppp);
    MaxAgeDiff     ->setEnabled(rel);
    RejectThres    ->setEnabled(rel||ppp);
    NumIter        ->setEnabled(rel||ppp);
    SyncSol        ->setEnabled(rel||ppp);
    BaselineConst  ->setEnabled(PosMode->currentIndex()==PMODE_MOVEB);
    BaselineLen    ->setEnabled(BaselineConst->isChecked()&&PosMode->currentIndex()==PMODE_MOVEB);
    BaselineSig    ->setEnabled(BaselineConst->isChecked()&&PosMode->currentIndex()==PMODE_MOVEB);
	
    OutputHead     ->setEnabled(SolFormat->currentIndex()!=3);
    OutputOpt      ->setEnabled(false);
    TimeFormat     ->setEnabled(SolFormat->currentIndex()!=3);
    TimeDecimal    ->setEnabled(SolFormat->currentIndex()!=3);
    LatLonFormat   ->setEnabled(SolFormat->currentIndex()==0);
    FieldSep       ->setEnabled(SolFormat->currentIndex()!=3);
    OutputDatum    ->setEnabled(SolFormat->currentIndex()==0);
    OutputHeight   ->setEnabled(SolFormat->currentIndex()==0);
    OutputGeoid    ->setEnabled(SolFormat->currentIndex()==0&&OutputHeight->currentIndex()==1);
	
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
	
    RovPosTypeP    ->setEnabled(PosMode->currentIndex()==PMODE_FIXED||PosMode->currentIndex()==PMODE_PPP_FIXED);
    RovPos1        ->setEnabled(RovPosTypeP->isEnabled()&&RovPosTypeP->currentIndex()<=2);
    RovPos2        ->setEnabled(RovPosTypeP->isEnabled()&&RovPosTypeP->currentIndex()<=2);
    RovPos3        ->setEnabled(RovPosTypeP->isEnabled()&&RovPosTypeP->currentIndex()<=2);
    BtnRovPos      ->setEnabled(RovPosTypeP->isEnabled()&&RovPosTypeP->currentIndex()<=2);
	
    RefPosTypeP    ->setEnabled(rel&&PosMode->currentIndex()!=PMODE_MOVEB);
    RefPos1        ->setEnabled(RefPosTypeP->isEnabled()&&RefPosTypeP->currentIndex()<=2);
    RefPos2        ->setEnabled(RefPosTypeP->isEnabled()&&RefPosTypeP->currentIndex()<=2);
    RefPos3        ->setEnabled(RefPosTypeP->isEnabled()&&RefPosTypeP->currentIndex()<=2);
    BtnRefPos      ->setEnabled(RefPosTypeP->isEnabled()&&RefPosTypeP->currentIndex()<=2);
	
    LabelMaxAveEp  ->setVisible(RefPosTypeP->currentIndex()==5);
    MaxAveEp       ->setVisible(RefPosTypeP->currentIndex()==5);
    ChkInitRestart ->setVisible(RefPosTypeP->currentIndex()==5);

    SbasSatE       ->setEnabled(PosMode->currentIndex()==0);
}
//---------------------------------------------------------------------------
void OptDialog::GetPos(int type, QLineEdit **edit, double *pos)
{
	double p[3]={0},dms1[3]={0},dms2[3]={0};
	
	if (type==1) { /* lat/lon/height dms/m */
        QStringList tokens=edit[0]->text().split(" ");
        for (int i=0;i<3||i<tokens.size();i++)
            dms1[i]=tokens.at(i).toDouble();

        tokens=edit[1]->text().split(" ");
        for (int i=0;i<3||i<tokens.size();i++)
            dms2[i]=tokens.at(i).toDouble();

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
        edit[0]->setText(QString("%1 %2 %3").arg(s1*dms1[0],0,'f',0).arg(dms1[1],2,'f',0,'0').arg(dms1[2],9,'f',6,'0'));
        edit[1]->setText(QString("%1 %2 %3").arg(s2*dms2[0],0,'f',0).arg(dms2[1],2,'f',0,'0').arg(dms2[2],9,'f',6,'0'));
        edit[2]->setText(QString::number(p[2],'f',4));
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
    QStringList list;
    pcvs_t pcvs={0,0,NULL};
	char *p;
	
    if (!readpcv(qPrintable(AntPcvFile_Text),&pcvs)) return;
	
    list.append("");
    list.append("*");
	
	for (int i=0;i<pcvs.n;i++) {
		if (pcvs.pcv[i].sat) continue;
		if ((p=strchr(pcvs.pcv[i].type,' '))) *p='\0';
		if (i>0&&!strcmp(pcvs.pcv[i].type,pcvs.pcv[i-1].type)) continue;
        list.append(pcvs.pcv[i].type);
	}
    RovAnt->clear();
    RefAnt->clear();
    RovAnt->addItems(list);
    RefAnt->addItems(list);
	
	free(pcvs.pcv);
}
//---------------------------------------------------------------------------

void OptDialog::NavSys6Click()
{
	UpdateEnable();
}
//---------------------------------------------------------------------------

