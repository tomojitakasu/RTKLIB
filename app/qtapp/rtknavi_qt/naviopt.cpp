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
#include "freqdlg.h"

//---------------------------------------------------------------------------
#define MAXSTR      1024                /* max length of a string */

// receiver options table ---------------------------------------------------
static int strtype[] = {                  /* stream types */
    STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE, STR_NONE
};
static char strpath[8][MAXSTR] = { "" };        /* stream paths */
static int strfmt[] = {                         /* stream formats */
    STRFMT_RTCM3, STRFMT_RTCM3, STRFMT_SP3, SOLF_LLH, SOLF_NMEA, 0, 0, 0
};
static int svrcycle = 10;               /* server cycle (ms) */
static int timeout = 10000;             /* timeout time (ms) */
static int reconnect = 10000;           /* reconnect interval (ms) */
static int nmeacycle = 5000;            /* nmea request cycle (ms) */
static int fswapmargin = 30;            /* file swap marign (s) */
static int buffsize = 32768;            /* input buffer size (bytes) */
static int navmsgsel = 0;               /* navigation mesaage select */
static int nmeareq = 0;                 /* nmea request type (0:off,1:lat/lon,2:single) */
static double nmeapos[] = { 0, 0 };     /* nmea position (lat/lon) (deg) */
static char proxyaddr[MAXSTR] = "";     /* proxy address */

#define TIMOPT  "0:gpst,1:utc,2:jst,3:tow"
#define CONOPT  "0:dms,1:deg,2:xyz,3:enu,4:pyl"
#define FLGOPT  "0:off,1:std+2:age/ratio/ns"
#define ISTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,7:ntripcli,8:ftp,9:http"
#define OSTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr"
#define FMTOPT  "0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,10:nvs,11:binex,12:rt17,13:sbf,14:cmr,17:sp3"
#define NMEOPT  "0:off,1:latlon,2:single"
#define SOLOPT  "0:llh,1:xyz,2:enu,3:nmea"
#define MSGOPT  "0:all,1:rover,2:base,3:corr"

static opt_t rcvopts[] = {
    { "inpstr1-type",     3, (void *)&strtype[0],  ISTOPT  },
    { "inpstr2-type",     3, (void *)&strtype[1],  ISTOPT  },
    { "inpstr3-type",     3, (void *)&strtype[2],  ISTOPT  },
    { "inpstr1-path",     2, (void *)strpath [0],  ""      },
    { "inpstr2-path",     2, (void *)strpath [1],  ""      },
    { "inpstr3-path",     2, (void *)strpath [2],  ""      },
    { "inpstr1-format",   3, (void *)&strfmt [0],  FMTOPT  },
    { "inpstr2-format",   3, (void *)&strfmt [1],  FMTOPT  },
    { "inpstr3-format",   3, (void *)&strfmt [2],  FMTOPT  },
    { "inpstr2-nmeareq",  3, (void *)&nmeareq,     NMEOPT  },
    { "inpstr2-nmealat",  1, (void *)&nmeapos[0],  "deg"   },
    { "inpstr2-nmealon",  1, (void *)&nmeapos[1],  "deg"   },
    { "outstr1-type",     3, (void *)&strtype[3],  OSTOPT  },
    { "outstr2-type",     3, (void *)&strtype[4],  OSTOPT  },
    { "outstr1-path",     2, (void *)strpath [3],  ""      },
    { "outstr2-path",     2, (void *)strpath [4],  ""      },
    { "outstr1-format",   3, (void *)&strfmt [3],  SOLOPT  },
    { "outstr2-format",   3, (void *)&strfmt [4],  SOLOPT  },
    { "logstr1-type",     3, (void *)&strtype[5],  OSTOPT  },
    { "logstr2-type",     3, (void *)&strtype[6],  OSTOPT  },
    { "logstr3-type",     3, (void *)&strtype[7],  OSTOPT  },
    { "logstr1-path",     2, (void *)strpath [5],  ""      },
    { "logstr2-path",     2, (void *)strpath [6],  ""      },
    { "logstr3-path",     2, (void *)strpath [7],  ""      },

    { "misc-svrcycle",    0, (void *)&svrcycle,    "ms"    },
    { "misc-timeout",     0, (void *)&timeout,     "ms"    },
    { "misc-reconnect",   0, (void *)&reconnect,   "ms"    },
    { "misc-nmeacycle",   0, (void *)&nmeacycle,   "ms"    },
    { "misc-buffsize",    0, (void *)&buffsize,    "bytes" },
    { "misc-navmsgsel",   3, (void *)&navmsgsel,   MSGOPT  },
    { "misc-proxyaddr",   2, (void *)proxyaddr,    ""      },
    { "misc-fswapmargin", 0, (void *)&fswapmargin, "s"     },

    { "",		      0, NULL,		       ""      }
};
//---------------------------------------------------------------------------
OptDialog::OptDialog(QWidget *parent)
    : QDialog(parent)
{
    QString label;
    int nglo = MAXPRNGLO, ngal = MAXPRNGAL, nqzs = MAXPRNQZS;
    int ncmp = MAXPRNCMP, nirn = MAXPRNIRN;

    setupUi(this);

    PrcOpt = prcopt_default;
    SolOpt = solopt_default;

    textViewer = new TextViewer(this);
    freqDialog = new FreqDialog(this);

    UpdateEnable();

    Freq->clear();
    for (int i = 0; i < NFREQ; i++) {
        label="L1";
                for (int j=1;j<=i;j++) {
                    label+=QString("+%1").arg(j+1);
                }
        Freq->addItem(label);
	}
    if (nglo <= 0) NavSys2->setEnabled(false);
    if (ngal <= 0) NavSys3->setEnabled(false);
    if (nqzs <= 0) NavSys4->setEnabled(false);
    if (ncmp <= 0) NavSys6->setEnabled(false);
    if (nirn <= 0) NavSys7->setEnabled(false);

    QCompleter *fileCompleter = new QCompleter(this);
    QFileSystemModel *fileModel = new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    StaPosFile->setCompleter(fileCompleter);
    AntPcvFile->setCompleter(fileCompleter);
    SatPcvFile->setCompleter(fileCompleter);
    DCBFile->setCompleter(fileCompleter);
    GeoidDataFile->setCompleter(fileCompleter);
    EOPFile->setCompleter(fileCompleter);
    OLFile->setCompleter(fileCompleter);

    QCompleter *dirCompleter = new QCompleter(this);
    QFileSystemModel *dirModel = new QFileSystemModel(dirCompleter);
    dirModel->setRootPath("");
    dirModel->setFilter(QDir::AllDirs | QDir::Drives | QDir::NoDotAndDotDot);
    dirCompleter->setModel(dirModel);
    LocalDir->setCompleter(dirCompleter);

	UpdateEnable();

    connect(BtnAntPcvFile, SIGNAL(clicked(bool)), this, SLOT(BtnAntPcvFileClick()));
    connect(BtnAntPcvView, SIGNAL(clicked(bool)), this, SLOT(BtnAntPcvViewClick()));
    connect(BtnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(BtnDCBFile, SIGNAL(clicked(bool)), this, SLOT(BtnDCBFileClick()));
    connect(BtnEOPFile, SIGNAL(clicked(bool)), this, SLOT(BtnEOPFileClick()));
    connect(BtnEOPView, SIGNAL(clicked(bool)), this, SLOT(BtnEOPViewClick()));
    connect(BtnFont1, SIGNAL(clicked(bool)), this, SLOT(BtnFont1Click()));
    connect(BtnFont2, SIGNAL(clicked(bool)), this, SLOT(BtnFont2Click()));
    connect(BtnGeoidDataFile, SIGNAL(clicked(bool)), this, SLOT(BtnGeoidDataFileClick()));
    connect(BtnLoad, SIGNAL(clicked(bool)), this, SLOT(BtnLoadClick()));
    connect(BtnLocalDir, SIGNAL(clicked(bool)), this, SLOT(BtnLocalDirClick()));
    connect(BtnOk, SIGNAL(clicked(bool)), this, SLOT(BtnOkClick()));
//    connect(BtnOLFile,SIGNAL(clicked(bool)),this,SLOT(Btn));//Ocean Load
    connect(BtnRefPos, SIGNAL(clicked(bool)), this, SLOT(BtnRefPosClick()));
    connect(BtnRovPos, SIGNAL(clicked(bool)), this, SLOT(BtnRovPosClick()));
    connect(BtnSatPcvFile, SIGNAL(clicked(bool)), this, SLOT(BtnSatPcvFileClick()));
    connect(BtnSatPcvView, SIGNAL(clicked(bool)), this, SLOT(BtnSatPcvViewClick()));
    connect(BtnSave, SIGNAL(clicked(bool)), this, SLOT(BtnSaveClick()));
    connect(BtnSnrMask, SIGNAL(clicked(bool)), this, SLOT(BtnSnrMaskClick()));
    connect(BtnStaPosFile, SIGNAL(clicked(bool)), this, SLOT(BtnStaPosFileClick()));
    connect(BtnStaPosView, SIGNAL(clicked(bool)), this, SLOT(BtnStaPosViewClick()));
    connect(PosMode, SIGNAL(currentIndexChanged(int)), this, SLOT(PosModeChange(int)));
    connect(SolFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SolFormatChange(int)));
    connect(RefPosTypeP, SIGNAL(currentIndexChanged(int)), this, SLOT(RefPosTypePChange(int)));
    connect(RovPosTypeP, SIGNAL(currentIndexChanged(int)), this, SLOT(RovPosTypePChange(int)));
    connect(AmbRes, SIGNAL(currentIndexChanged(int)), this, SLOT(AmbResChange(int)));
    connect(RovAntPcv, SIGNAL(clicked(bool)), this, SLOT(RovAntPcvClick()));
    connect(RefAntPcv, SIGNAL(clicked(bool)), this, SLOT(RovAntPcvClick()));
    connect(OutputHeight, SIGNAL(currentIndexChanged(int)), this, SLOT(OutputHeightClick()));
    connect(NavSys2, SIGNAL(clicked(bool)), this, SLOT(NavSys2Click()));
    connect(NavSys6, SIGNAL(clicked(bool)), this, SLOT(NavSys6Click()));
    connect(BaselineConst, SIGNAL(clicked(bool)), this, SLOT(BaselineConstClick()));
    connect(BtnFreq, SIGNAL(clicked(bool)), this, SLOT(BtnFreqClick()));
    connect(RefAnt, SIGNAL(currentIndexChanged(int)), this, SLOT(RefAntClick()));
    connect(RovAnt, SIGNAL(currentIndexChanged(int)), this, SLOT(RovAntClick()));

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

    fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Load Options..."), QString(), tr("Options File (*.conf);;All (*.*)")));

    LoadOpt(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnSaveClick()
{
    QString file;

    file = QDir::toNativeSeparators(QFileDialog::getSaveFileName(this, tr("Save Options..."), QString(), tr("Options File (*.conf);;All (*.*)")));

    if (!file.contains('.')) file += ".conf";

    SaveOpt(file);
}
//---------------------------------------------------------------------------
void OptDialog::BtnStaPosViewClick()
{
    if (StaPosFile->text() == "") return;

    TextViewer *viewer = new TextViewer(this);
    viewer->show();

    viewer->Read(StaPosFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnStaPosFileClick()
{
    QString fileName;

    fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Station Position File"), QString(), tr("Position File (*.pos);;All (*.*)")));

    StaPosFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnSnrMaskClick()
{
    MaskOptDialog maskOptDialog(this);

    maskOptDialog.Mask = PrcOpt.snrmask;

    maskOptDialog.exec();
    if (maskOptDialog.result() != QDialog::Accepted) return;
    PrcOpt.snrmask = maskOptDialog.Mask;
}
//---------------------------------------------------------------------------
void OptDialog::RovPosTypePChange(int)
{
    QLineEdit *edit[] = { RovPos1, RovPos2, RovPos3 };
	double pos[3];

    GetPos(RovPosTypeF, edit, pos);
    SetPos(RovPosTypeP->currentIndex(), edit, pos);
    RovPosTypeF = RovPosTypeP->currentIndex();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RefPosTypePChange(int)
{
    QLineEdit *edit[] = { RefPos1, RefPos2, RefPos3 };
	double pos[3];

    GetPos(RefPosTypeF, edit, pos);
    SetPos(RefPosTypeP->currentIndex(), edit, pos);
    RefPosTypeF = RefPosTypeP->currentIndex();

	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::BtnRovPosClick()
{
    RefDialog refDialog(this);
    QLineEdit *edit[] = { RovPos1, RovPos2, RovPos3 };
    double p[3], posi[3];

    GetPos(RovPosTypeP->currentIndex(), edit, p);
    ecef2pos(p, posi);

    refDialog.RovPos[0] = posi[0] * R2D;
    refDialog.RovPos[1] = posi[1] * R2D;
    refDialog.Pos[2] = posi[2];
    refDialog.StaPosFile = StaPosFile->text();
    refDialog.move(pos().x() + size().width() / 2 - refDialog.size().width() / 2,
               pos().y() + size().height() / 2 - refDialog.size().height() / 2);

    refDialog.exec();
    if (refDialog.result() != QDialog::Accepted) return;

    posi[0] = refDialog.Pos[0] * D2R;
    posi[1] = refDialog.Pos[1] * D2R;
    posi[2] = refDialog.Pos[2];

    pos2ecef(posi, p);
    SetPos(RovPosTypeP->currentIndex(), edit, p);
}
//---------------------------------------------------------------------------
void OptDialog::BtnRefPosClick()
{
    RefDialog refDialog(this);
    QLineEdit *edit[] = { RefPos1, RefPos2, RefPos3 };
    double p[3], posi[3];

    GetPos(RefPosTypeP->currentIndex(), edit, p);
    ecef2pos(p, posi);
    refDialog.RovPos[0] = posi[0] * R2D;
    refDialog.RovPos[1] = posi[1] * R2D;
    refDialog.RovPos[2] = posi[2];
    refDialog.StaPosFile = StaPosFile->text();
    refDialog.move(pos().x() + size().width() / 2 - refDialog.size().width() / 2,
               pos().y() + size().height() / 2 - refDialog.size().height() / 2);

    refDialog.exec();
    if (refDialog.result() != QDialog::Accepted) return;

    posi[0] = refDialog.Pos[0] * D2R;
    posi[1] = refDialog.Pos[1] * D2R;
    posi[2] = refDialog.Pos[2];

    pos2ecef(posi, p);
    SetPos(RefPosTypeP->currentIndex(), edit, p);
}
//---------------------------------------------------------------------------
void OptDialog::BtnSatPcvViewClick()
{
    if (SatPcvFile->text() == "") return;

    textViewer->show();

    textViewer->Read(SatPcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnSatPcvFileClick()
{
    SatPcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Satellite Antenna PCV File"), QString(), tr("PCV File (*.pcv *.atx);Position File (*.pcv *.snx);All (*.*)"))));
}
//---------------------------------------------------------------------------
void OptDialog::BtnAntPcvViewClick()
{
    if (AntPcvFile->text() == "") return;

    textViewer->show();

    textViewer->Read(AntPcvFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnAntPcvFileClick()
{
    AntPcvFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Receiver Antenna PCV File"), QString(), tr("PCV File (*.pcv *.atx);Position File (*.pcv *.snx);All (*.*)"))));
    ReadAntList();
}
//---------------------------------------------------------------------------
void OptDialog::BtnGeoidDataFileClick()
{
    QString fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("Geoid Data File"), QString(), tr("All (*.*)")));

    GeoidDataFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnDCBFileClick()
{
    QString fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("DCB Data File"), QString(), tr("DCB (*.dcb)")));

    DCBFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnEOPFileClick()
{
    QString fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, tr("EOP Data File"), QString(), tr("EOP (*.erp)")));

    EOPFile->setText(fileName);
}
//---------------------------------------------------------------------------
void OptDialog::BtnEOPViewClick()
{
    if (EOPFile->text() == "") return;

    textViewer->show();

    textViewer->Read(EOPFile->text());
}
//---------------------------------------------------------------------------
void OptDialog::BtnLocalDirClick()
{
    QString dir = LocalDir->text();

    dir = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("FTP/HTTP Local Directory"), dir));
    LocalDir->setText(dir);
}
//---------------------------------------------------------------------------
void OptDialog::BtnFont1Click()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(FontLabel1->font());
    dialog.exec();

    FontLabel1->setFont(dialog.selectedFont());
    FontLabel1->setText(FontLabel1->font().family() + QString::number(FontLabel1->font().pointSize()) + " pt");
}
//---------------------------------------------------------------------------
void OptDialog::BtnFont2Click()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(FontLabel2->font());
    dialog.exec();

    FontLabel2->setFont(dialog.selectedFont());
    FontLabel2->setText(FontLabel2->font().family() + QString::number(FontLabel2->font().pointSize()) + " pt");
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
    QLineEdit *editu[] = { RovPos1, RovPos2, RovPos3 };
    QLineEdit *editr[] = { RefPos1, RefPos2, RefPos3 };

    PosMode->setCurrentIndex(PrcOpt.mode);
    Freq->setCurrentIndex(PrcOpt.nf - 1 > NFREQ - 1 ? NFREQ - 1 : PrcOpt.nf - 1);
    ElMask->setCurrentIndex(ElMask->findText(QString::number(PrcOpt.elmin * R2D)));
    DynamicModel->setCurrentIndex(PrcOpt.dynamics);
    TideCorr->setCurrentIndex(PrcOpt.tidecorr);
    IonoOpt->setCurrentIndex(PrcOpt.ionoopt);
    TropOpt->setCurrentIndex(PrcOpt.tropopt);
    SatEphem->setCurrentIndex(PrcOpt.sateph);
    AmbRes->setCurrentIndex(PrcOpt.modear);
    GloAmbRes->setCurrentIndex(PrcOpt.glomodear);
    BdsAmbRes->setCurrentIndex(PrcOpt.bdsmodear);
    ValidThresAR->setValue(PrcOpt.thresar[0]);
    OutCntResetAmb->setValue(PrcOpt.maxout);
    LockCntFixAmb->setValue(PrcOpt.minlock);
    FixCntHoldAmb->setValue(PrcOpt.minfix);
    ElMaskAR->setValue(PrcOpt.elmaskar * R2D);
    ElMaskHold->setValue(PrcOpt.elmaskhold * R2D);
    MaxAgeDiff->setValue(PrcOpt.maxtdiff);
    RejectGdop->setValue(PrcOpt.maxgdop);
    RejectThres->setValue(PrcOpt.maxinno);
    SlipThres->setValue(PrcOpt.thresslip);
    NumIter->setValue(PrcOpt.niter);
    SyncSol->setCurrentIndex(PrcOpt.syncsol);
    ExSatsE->setText(ExSats);
    NavSys1->setChecked(PrcOpt.navsys & SYS_GPS);
    NavSys2->setChecked(PrcOpt.navsys & SYS_GLO);
    NavSys3->setChecked(PrcOpt.navsys & SYS_GAL);
    NavSys4->setChecked(PrcOpt.navsys & SYS_QZS);
    NavSys5->setChecked(PrcOpt.navsys & SYS_SBS);
    NavSys6->setChecked(PrcOpt.navsys & SYS_CMP);
    NavSys7->setChecked(PrcOpt.navsys & SYS_IRN);
    PosOpt1->setChecked(PrcOpt.posopt[0]);
    PosOpt2->setChecked(PrcOpt.posopt[1]);
    PosOpt3->setChecked(PrcOpt.posopt[2]);
    PosOpt4->setChecked(PrcOpt.posopt[3]);
    PosOpt5->setChecked(PrcOpt.posopt[4]);

    SolFormat->setCurrentIndex(SolOpt.posf);
    TimeFormat->setCurrentIndex(SolOpt.timef == 0 ? 0 : SolOpt.times + 1);
    TimeDecimal->setValue(SolOpt.timeu);
    LatLonFormat->setCurrentIndex(SolOpt.degf);
    FieldSep->setText(SolOpt.sep);
    OutputHead->setCurrentIndex(SolOpt.outhead);
    OutputOpt->setCurrentIndex(SolOpt.outopt);
    OutputVel->setCurrentIndex(SolOpt.outvel);
    OutputSingle->setCurrentIndex(PrcOpt.outsingle);
    MaxSolStd->setValue(SolOpt.maxsolstd);
    OutputDatum->setCurrentIndex(SolOpt.datum);
    OutputHeight->setCurrentIndex(SolOpt.height);
    OutputGeoid->setCurrentIndex(SolOpt.geoid);
    NmeaIntv1->setValue(SolOpt.nmeaintv[0]);
    NmeaIntv2->setValue(SolOpt.nmeaintv[1]);
    DebugStatus->setCurrentIndex(DebugStatusF);
    DebugTrace->setCurrentIndex(DebugTraceF);

    BaselineConst->setChecked(BaselineC);
    BaselineLen->setValue(Baseline[0]);
    BaselineSig->setValue(Baseline[1]);

    MeasErrR1->setValue(PrcOpt.eratio[0]);
    MeasErrR2->setValue(PrcOpt.eratio[1]);
    MeasErr2->setValue(PrcOpt.err[1]);
    MeasErr3->setValue(PrcOpt.err[2]);
    MeasErr4->setValue(PrcOpt.err[3]);
    MeasErr5->setValue(PrcOpt.err[4]);
    PrNoise1->setText(QString::number(PrcOpt.prn[0]));
    PrNoise2->setText(QString::number(PrcOpt.prn[1]));
    PrNoise3->setText(QString::number(PrcOpt.prn[2]));
    PrNoise4->setText(QString::number(PrcOpt.prn[3]));
    PrNoise5->setText(QString::number(PrcOpt.prn[4]));
    SatClkStab->setText(QString::number(PrcOpt.sclkstab));
    MaxAveEp->setValue(PrcOpt.maxaveep);
    ChkInitRestart->setChecked(PrcOpt.initrst);

    RovPosTypeP->setCurrentIndex(RovPosTypeF);
    RefPosTypeP->setCurrentIndex(RefPosTypeF);
    RovAntPcv->setChecked(RovAntPcvF);
    RefAntPcv->setChecked(RefAntPcvF);
    RovAntE->setValue(RovAntDel[0]);
    RovAntN->setValue(RovAntDel[1]);
    RovAntU->setValue(RovAntDel[2]);
    RefAntE->setValue(RefAntDel[0]);
    RefAntN->setValue(RefAntDel[1]);
    RefAntU->setValue(RefAntDel[2]);
    SetPos(RovPosTypeP->currentIndex(), editu, RovPos);
    SetPos(RefPosTypeP->currentIndex(), editr, RefPos);

    SatPcvFile->setText(SatPcvFileF);
    AntPcvFile->setText(AntPcvFileF);
    StaPosFile->setText(StaPosFileF);
    GeoidDataFile->setText(GeoidDataFileF);
    DCBFile->setText(DCBFileF);
    EOPFile->setText(EOPFileF);
    LocalDir->setText(LocalDirectory);
	ReadAntList();

    RovAnt->setCurrentIndex(RovAnt->findText(RovAntF));
    RefAnt->setCurrentIndex(RefAnt->findText(RefAntF));

    SvrCycleE->setValue(SvrCycle);
    TimeoutTimeE->setValue(TimeoutTime);
    ReconTimeE->setValue(ReconTime);
    NmeaCycleE->setValue(NmeaCycle);
    FileSwapMarginE->setValue(FileSwapMargin);
    SvrBuffSizeE->setValue(SvrBuffSize);
    SolBuffSizeE->setValue(SolBuffSize);
    SavedSolE->setValue(SavedSol);
    NavSelectS->setCurrentIndex(NavSelect);
    SbasSatE->setValue(PrcOpt.sbassatsel);
    ProxyAddrE->setText(ProxyAddr);
    MoniPortE->setValue(MoniPort);
    SolBuffSizeE->setValue(SolBuffSize);
    PanelStackE->setCurrentIndex(PanelStack);

    FontLabel1->setFont(PanelFont);
    FontLabel1->setText(FontLabel1->font().family() + QString::number(FontLabel1->font().pointSize()) + "pt");
    FontLabel2->setFont(PosFont);
    FontLabel2->setText(FontLabel2->font().family() + QString::number(FontLabel2->font().pointSize()) + "pt");

	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SetOpt(void)
{
    QString FieldSep_Text = FieldSep->text();
    QLineEdit *editu[] = { RovPos1, RovPos2, RovPos3 };
    QLineEdit *editr[] = { RefPos1, RefPos2, RefPos3 };

    PrcOpt.mode = PosMode->currentIndex();
    PrcOpt.nf = Freq->currentIndex() + 1;
    PrcOpt.elmin = ElMask->currentText().toDouble() * D2R;
    PrcOpt.dynamics = DynamicModel->currentIndex();
    PrcOpt.tidecorr = TideCorr->currentIndex();
    PrcOpt.ionoopt = IonoOpt->currentIndex();
    PrcOpt.tropopt = TropOpt->currentIndex();
    PrcOpt.sateph = SatEphem->currentIndex();
    PrcOpt.modear = AmbRes->currentIndex();
    PrcOpt.glomodear = GloAmbRes->currentIndex();
    PrcOpt.bdsmodear = BdsAmbRes->currentIndex();
    PrcOpt.thresar[0] = ValidThresAR->value();
    PrcOpt.maxout = OutCntResetAmb->value();
    PrcOpt.minlock = LockCntFixAmb->value();
    PrcOpt.minfix = FixCntHoldAmb->value();
    PrcOpt.elmaskar = ElMaskAR->value() * D2R;
    PrcOpt.elmaskhold = ElMaskHold->value() * D2R;
    PrcOpt.maxtdiff = MaxAgeDiff->value();
    PrcOpt.maxgdop = RejectGdop->value();
    PrcOpt.maxinno = RejectThres->value();
    PrcOpt.thresslip = SlipThres->value();
    PrcOpt.niter = NumIter->value();
    PrcOpt.syncsol = SyncSol->currentIndex();
    ExSats = ExSatsE->text();
    PrcOpt.navsys = 0;

    if (NavSys1->isChecked()) PrcOpt.navsys |= SYS_GPS;
    if (NavSys2->isChecked()) PrcOpt.navsys |= SYS_GLO;
    if (NavSys3->isChecked()) PrcOpt.navsys |= SYS_GAL;
    if (NavSys4->isChecked()) PrcOpt.navsys |= SYS_QZS;
    if (NavSys5->isChecked()) PrcOpt.navsys |= SYS_SBS;
    if (NavSys6->isChecked()) PrcOpt.navsys |= SYS_CMP;
    if (NavSys7->isChecked()) PrcOpt.navsys |= SYS_IRN;
    PrcOpt.posopt[0] = PosOpt1->isChecked();
    PrcOpt.posopt[1] = PosOpt2->isChecked();
    PrcOpt.posopt[2] = PosOpt3->isChecked();
    PrcOpt.posopt[3] = PosOpt4->isChecked();
    PrcOpt.posopt[4] = PosOpt5->isChecked();

    SolOpt.posf = SolFormat->currentIndex();
    SolOpt.timef = TimeFormat->currentIndex() == 0 ? 0 : 1;
    SolOpt.times = TimeFormat->currentIndex() == 0 ? 0 : TimeFormat->currentIndex() - 1;
    SolOpt.timeu = static_cast<int>(TimeDecimal->value());
    SolOpt.degf = LatLonFormat->currentIndex();
    strcpy(SolOpt.sep, qPrintable(FieldSep_Text));
    SolOpt.outhead = OutputHead->currentIndex();
    SolOpt.outopt = OutputOpt->currentIndex();
    SolOpt.outvel = OutputVel->currentIndex();
    PrcOpt.outsingle = OutputSingle->currentIndex();
    SolOpt.maxsolstd = MaxSolStd->value();
    SolOpt.datum = OutputDatum->currentIndex();
    SolOpt.height = OutputHeight->currentIndex();
    SolOpt.geoid = OutputGeoid->currentIndex();
    SolOpt.nmeaintv[0] = NmeaIntv1->value();
    SolOpt.nmeaintv[1] = NmeaIntv2->value();
    DebugStatusF = DebugStatus->currentIndex();
    DebugTraceF = DebugTrace->currentIndex();

    BaselineC = BaselineConst->isChecked();
    Baseline[0] = BaselineLen->value();
    Baseline[1] = BaselineSig->value();

    PrcOpt.eratio[0] = MeasErrR1->value();
    PrcOpt.eratio[1] = MeasErrR2->value();
    PrcOpt.err[1] = MeasErr2->value();
    PrcOpt.err[2] = MeasErr3->value();
    PrcOpt.err[3] = MeasErr4->value();
    PrcOpt.err[4] = MeasErr5->value();
    PrcOpt.prn[0] = PrNoise1->text().toDouble();
    PrcOpt.prn[1] = PrNoise2->text().toDouble();
    PrcOpt.prn[2] = PrNoise3->text().toDouble();
    PrcOpt.prn[3] = PrNoise4->text().toDouble();
    PrcOpt.prn[4] = PrNoise5->text().toDouble();
    PrcOpt.sclkstab = SatClkStab->text().toDouble();
    PrcOpt.maxaveep = MaxAveEp->value();
    PrcOpt.initrst = ChkInitRestart->isChecked();

    RovPosTypeF = RovPosTypeP->currentIndex();
    RefPosTypeF = RefPosTypeP->currentIndex();
    RovAntPcvF = RovAntPcv->isChecked();
    RefAntPcvF = RefAntPcv->isChecked();
    RovAntF = RovAnt->currentText();
    RefAntF = RefAnt->currentText();
    RovAntDel[0] = RovAntE->value();
    RovAntDel[1] = RovAntN->value();
    RovAntDel[2] = RovAntU->value();
    RefAntDel[0] = RefAntE->value();
    RefAntDel[1] = RefAntN->value();
    RefAntDel[2] = RefAntU->value();
    GetPos(RovPosTypeP->currentIndex(), editu, RovPos);
    GetPos(RefPosTypeP->currentIndex(), editr, RefPos);

    SatPcvFileF = SatPcvFile->text();
    AntPcvFileF = AntPcvFile->text();
    StaPosFileF = StaPosFile->text();
    GeoidDataFileF = GeoidDataFile->text();
    DCBFileF = DCBFile->text();
    EOPFileF = EOPFile->text();
    LocalDirectory = LocalDir->text();

    SvrCycle = SvrCycleE->value();
    TimeoutTime = TimeoutTimeE->value();
    ReconTime = ReconTimeE->value();
    NmeaCycle = NmeaCycleE->value();
    FileSwapMargin = FileSwapMarginE->value();
    SvrBuffSize = SvrBuffSizeE->value();
    SolBuffSize = SolBuffSizeE->value();
    SavedSol = SavedSolE->value();
    NavSelect = NavSelectS->currentIndex();
    PrcOpt.sbassatsel = SbasSatE->value();
    ProxyAddr = ProxyAddrE->text();
    MoniPort = MoniPortE->value();
    PanelStack = PanelStackE->currentIndex();
    PanelFont = FontLabel1->font();
    PosFont = FontLabel2->font();

	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::LoadOpt(const QString &file)
{
    int itype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPCLI, STR_FILE, STR_FTP, STR_HTTP };
    int otype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPSVR, STR_NTRIPCAS, STR_FILE };
    QLineEdit *editu[] = { RovPos1, RovPos2, RovPos3 };
    QLineEdit *editr[] = { RefPos1, RefPos2, RefPos3 };
    QString buff;
    char id[32];
	int sat;
    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;
    filopt_t filopt;

    memset(&filopt, 0, sizeof(filopt_t));

	resetsysopts();
    if (!loadopts(qPrintable(file), sysopts) ||
        !loadopts(qPrintable(file), rcvopts)) return;
    getsysopts(&prcopt, &solopt, &filopt);

    for (int i = 0; i < 8; i++) {
        mainForm->StreamC[i] = strtype[i] != STR_NONE;
        mainForm->Stream[i] = STR_NONE;
        for (int j = 0; j < (i < 3 ? 7 : 5); j++) {
            if (strtype[i] != (i < 3 ? itype[j] : otype[j])) continue;
            mainForm->Stream[i] = j;
			break;
		}
        if (i < 5) mainForm->Format[i] = strfmt[i];

        if (strtype[i] == STR_SERIAL)
            mainForm->Paths[i][0] = strpath[i];
        else if (strtype[i] == STR_FILE)
            mainForm->Paths[i][2] = strpath[i];
        else if (strtype[i] <= STR_NTRIPCLI)
            mainForm->Paths[i][1] = strpath[i];
        else if (strtype[i] <= STR_HTTP)
            mainForm->Paths[i][3] = strpath[i];
	}
    mainForm->NmeaReq = nmeareq;
    mainForm->NmeaPos[0] = nmeapos[0];
    mainForm->NmeaPos[1] = nmeapos[1];

    SbasSatE->setValue(prcopt.sbassatsel);

    PosMode->setCurrentIndex(prcopt.mode);
    Freq->setCurrentIndex(prcopt.nf > NFREQ - 1 ? NFREQ - 1 : prcopt.nf - 1);
    Solution->setCurrentIndex(prcopt.soltype);
    ElMask->setCurrentIndex(ElMask->findText(QString::number(prcopt.elmin * R2D, 'f', 0)));
    DynamicModel->setCurrentIndex(prcopt.dynamics);
    TideCorr->setCurrentIndex(prcopt.tidecorr);
    IonoOpt->setCurrentIndex(prcopt.ionoopt);
    TropOpt->setCurrentIndex(prcopt.tropopt);
    SatEphem->setCurrentIndex(prcopt.sateph);
    ExSatsE->setText("");
    for (sat = 1; sat <= MAXSAT; sat++) {
        if (!prcopt.exsats[sat - 1]) continue;
        satno2id(sat, id);
        buff += QString(buff.isEmpty() ? "" : " ") + (prcopt.exsats[sat - 1] == 2 ? "+" : "") + id;
	}
    ExSatsE->setText(buff);
    NavSys1->setChecked(prcopt.navsys & SYS_GPS);
    NavSys2->setChecked(prcopt.navsys & SYS_GLO);
    NavSys3->setChecked(prcopt.navsys & SYS_GAL);
    NavSys4->setChecked(prcopt.navsys & SYS_QZS);
    NavSys5->setChecked(prcopt.navsys & SYS_SBS);
    NavSys6->setChecked(prcopt.navsys & SYS_CMP);
    NavSys7->setChecked(prcopt.navsys & SYS_IRN);
    PosOpt1->setChecked(prcopt.posopt[0]);
    PosOpt2->setChecked(prcopt.posopt[1]);
    PosOpt3->setChecked(prcopt.posopt[2]);
    PosOpt4->setChecked(prcopt.posopt[3]);
    PosOpt5->setChecked(prcopt.posopt[4]);

    AmbRes->setCurrentIndex(prcopt.modear);
    GloAmbRes->setCurrentIndex(prcopt.glomodear);
    BdsAmbRes->setCurrentIndex(prcopt.bdsmodear);
    ValidThresAR->setValue(prcopt.thresar[0]);
    OutCntResetAmb->setValue(prcopt.maxout);
    FixCntHoldAmb->setValue(prcopt.minfix);
    LockCntFixAmb->setValue(prcopt.minlock);
    ElMaskAR->setValue(prcopt.elmaskar * R2D);
    ElMaskHold->setValue(prcopt.elmaskhold * R2D);
    MaxAgeDiff->setValue(prcopt.maxtdiff);
    RejectGdop->setValue(prcopt.maxgdop);
    RejectThres->setValue(prcopt.maxinno);
    SlipThres->setValue(prcopt.thresslip);
    NumIter->setValue(prcopt.niter);
    SyncSol->setCurrentIndex(prcopt.syncsol);
    BaselineLen->setValue(prcopt.baseline[0]);
    BaselineSig->setValue(prcopt.baseline[1]);
    BaselineConst->setChecked(prcopt.baseline[0] > 0.0);

    SolFormat->setCurrentIndex(solopt.posf);
    TimeFormat->setCurrentIndex(solopt.timef == 0 ? 0 : solopt.times + 1);
    TimeDecimal->setValue(solopt.timeu);
    LatLonFormat->setCurrentIndex(solopt.degf);
    FieldSep->setText(solopt.sep);
    OutputHead->setCurrentIndex(solopt.outhead);
    OutputOpt->setCurrentIndex(solopt.outopt);
    OutputVel->setCurrentIndex(solopt.outvel);
    OutputSingle->setCurrentIndex(prcopt.outsingle);
    MaxSolStd->setValue(solopt.maxsolstd);
    OutputDatum->setCurrentIndex(solopt.datum);
    OutputHeight->setCurrentIndex(solopt.height);
    OutputGeoid->setCurrentIndex(solopt.geoid);
    NmeaIntv1->setValue(solopt.nmeaintv[0]);
    NmeaIntv2->setValue(solopt.nmeaintv[1]);
    DebugTrace->setCurrentIndex(solopt.trace);
    DebugStatus->setCurrentIndex(solopt.sstat);

    MeasErrR1->setValue(prcopt.eratio[0]);
    MeasErrR2->setValue(prcopt.eratio[1]);
    MeasErr2->setValue(prcopt.err[1]);
    MeasErr3->setValue(prcopt.err[2]);
    MeasErr4->setValue(prcopt.err[3]);
    MeasErr5->setValue(prcopt.err[4]);
    SatClkStab->setText(QString::number(prcopt.sclkstab));
    PrNoise1->setText(QString::number(prcopt.prn[0]));
    PrNoise2->setText(QString::number(prcopt.prn[1]));
    PrNoise3->setText(QString::number(prcopt.prn[2]));
    PrNoise4->setText(QString::number(prcopt.prn[3]));
    PrNoise5->setText(QString::number(prcopt.prn[4]));

    RovAntPcv->setChecked(*prcopt.anttype[0]);
    RefAntPcv->setChecked(*prcopt.anttype[1]);
    RovAnt->setCurrentIndex(RovAnt->findText(prcopt.anttype[0]));
    RefAnt->setCurrentIndex(RefAnt->findText(prcopt.anttype[1]));
    RovAntE->setValue(prcopt.antdel[0][0]);
    RovAntN->setValue(prcopt.antdel[0][1]);
    RovAntU->setValue(prcopt.antdel[0][2]);
    RefAntE->setValue(prcopt.antdel[1][0]);
    RefAntN->setValue(prcopt.antdel[1][1]);
    RefAntU->setValue(prcopt.antdel[1][2]);
    MaxAveEp->setValue(prcopt.maxaveep);

    RovPosTypeP->setCurrentIndex(0);
    RefPosTypeP->setCurrentIndex(0);
    if      (prcopt.refpos==POSOPT_RTCM  ) RefPosTypeP->setCurrentIndex(3);
    else if (prcopt.refpos==POSOPT_SINGLE) RefPosTypeP->setCurrentIndex(4);

    RovPosTypeF = RovPosTypeP->currentIndex();
    RefPosTypeF = RefPosTypeP->currentIndex();
    SetPos(RovPosTypeP->currentIndex(), editu, prcopt.ru);
    SetPos(RefPosTypeP->currentIndex(), editr, prcopt.rb);

    SatPcvFile->setText(filopt.satantp);
    AntPcvFile->setText(filopt.rcvantp);
    StaPosFile->setText(filopt.stapos);
    GeoidDataFile->setText(filopt.geoid);
    DCBFile->setText(filopt.dcb);
    LocalDir->setText(filopt.tempdir);

	ReadAntList();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::SaveOpt(const QString &file)
{
    QString ProxyAddrE_Text = ProxyAddrE->text();
    QString ExSatsE_Text = ExSatsE->text();
    QString FieldSep_Text = FieldSep->text();
    QString RovAnt_Text = RovAnt->currentText(), RefAnt_Text = RefAnt->currentText();
    QString SatPcvFile_Text = SatPcvFile->text();
    QString AntPcvFile_Text = AntPcvFile->text();
    QString StaPosFile_Text = StaPosFile->text();
    QString GeoidDataFile_Text = GeoidDataFile->text();
    QString DCBFile_Text = DCBFile->text();
    QString LocalDir_Text = LocalDir->text();
    int itype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPCLI, STR_FILE, STR_FTP, STR_HTTP };
    int otype[] = { STR_SERIAL, STR_TCPCLI, STR_TCPSVR, STR_NTRIPSVR, STR_NTRIPCAS, STR_FILE };
    QLineEdit *editu[] = { RovPos1, RovPos2, RovPos3 };
    QLineEdit *editr[] = { RefPos1, RefPos2, RefPos3 };
    char buff[1024], *p, *q, comment[256], s[64];
    int sat, ex;
    prcopt_t prcopt = prcopt_default;
    solopt_t solopt = solopt_default;
    filopt_t filopt;

    memset(&filopt, 0, sizeof(filopt_t));

    for (int i = 0; i < 8; i++) {
        strtype[i] = i < 3 ? itype[mainForm->Stream[i]] : otype[mainForm->Stream[i]];
        strfmt[i] = mainForm->Format[i];

        if (!mainForm->StreamC[i]) {
            strtype[i] = STR_NONE;
            strcpy(strpath[i], "");
        } else if (strtype[i] == STR_SERIAL) {
            strcpy(strpath[i], qPrintable(mainForm->Paths[i][0]));
        } else if (strtype[i] == STR_FILE) {
            strcpy(strpath[i], qPrintable(mainForm->Paths[i][2]));
        } else if (strtype[i] == STR_TCPSVR) {
            strcpy(buff,qPrintable(mainForm->Paths[i][1]));
            if ((p=strchr(buff,'/'))) *p='\0'; // TODO
            if ((p=strrchr(buff,':'))) {
                strcpy(strpath[i],p);
            }
            else {
                strcpy(strpath[i],"");
            }
        }
        else if (strtype[i]==STR_TCPCLI) {
            strcpy(buff,qPrintable(mainForm->Paths[i][1]));
            if ((p=strchr(buff,'/'))) *p='\0';
            if ((p=strrchr(buff,'@'))) {
                strcpy(strpath[i],p+1);
            }
            else {
                strcpy(strpath[i],buff);
            }
        }
        else if (strtype[i]==STR_NTRIPSVR) {
            strcpy(buff,qPrintable(mainForm->Paths[i][1]));
            if ((p=strchr(buff,':'))&&strchr(p+1,'@')) {
                strcpy(strpath[i],p);
            }
            else {
                strcpy(strpath[i],buff);
            }
        }
        else if (strtype[i]==STR_NTRIPCLI) {
            strcpy(buff,qPrintable(mainForm->Paths[i][1]));
            if ((p=strchr(buff,'/'))&&(q=strchr(p+1,':'))) *q='\0';
            strcpy(strpath[i],buff);
        }
        else if (strtype[i]==STR_NTRIPCAS) {
            strcpy(buff,qPrintable(mainForm->Paths[i][1]));
            if ((p=strchr(buff,'/'))&&(q=strchr(p+1,':'))) *q='\0';
            if ((p=strchr(buff,'@'))) {
                *(p+1)='\0';
                strcpy(strpath[i],buff);
            }
            if ((p=strchr(p?p+2:buff,':'))) {
                strcat(strpath[i],p);
            }
        } else if (strtype[i]==STR_FTP||strtype[i]==STR_HTTP)
        {
            strcpy(strpath[i], qPrintable(mainForm->Paths[i][3]));
		}
	}
    nmeareq = mainForm->NmeaReq;
    nmeapos[0] = mainForm->NmeaPos[0];
    nmeapos[1] = mainForm->NmeaPos[1];

    svrcycle = SvrCycleE->value();
    timeout = TimeoutTimeE->value();
    reconnect = ReconTimeE->value();
    nmeacycle = NmeaCycleE->value();
    buffsize = SvrBuffSizeE->value();
    navmsgsel = NavSelectS->currentIndex();
    strcpy(proxyaddr, qPrintable(ProxyAddrE_Text));
    fswapmargin = FileSwapMarginE->value();
    prcopt.sbassatsel = SbasSatE->value();

    prcopt.mode = PosMode->currentIndex();
    prcopt.nf = Freq->currentIndex() + 1;
    prcopt.soltype = Solution->currentIndex();
    prcopt.elmin = ElMask->currentText().toDouble() * D2R;
    prcopt.dynamics = DynamicModel->currentIndex();
    prcopt.tidecorr = TideCorr->currentIndex();
    prcopt.ionoopt = IonoOpt->currentIndex();
    prcopt.tropopt = TropOpt->currentIndex();
    prcopt.sateph = SatEphem->currentIndex();
    if (ExSatsE->text() != "") {
        strcpy(buff, qPrintable(ExSatsE_Text));
        for (p = strtok(buff, " "); p; p = strtok(NULL, " ")) {
            if (*p == '+') {
                ex = 2; p++;
            } else {
                ex = 1;
            }
            if (!(sat = satid2no(p))) continue;
            prcopt.exsats[sat - 1] = (unsigned char)ex;
		}
	}
    prcopt.navsys = (NavSys1->isChecked() ? SYS_GPS : 0) |
            (NavSys2->isChecked() ? SYS_GLO : 0) |
            (NavSys3->isChecked() ? SYS_GAL : 0) |
            (NavSys4->isChecked() ? SYS_QZS : 0) |
            (NavSys5->isChecked() ? SYS_SBS : 0) |
            (NavSys6->isChecked() ? SYS_CMP : 0) |
            (NavSys7->isChecked() ? SYS_IRN : 0);
    prcopt.posopt[0] = PosOpt1->isChecked();
    prcopt.posopt[1] = PosOpt2->isChecked();
    prcopt.posopt[2] = PosOpt3->isChecked();
    prcopt.posopt[3] = PosOpt4->isChecked();
    prcopt.posopt[4] = PosOpt5->isChecked();

    prcopt.modear = AmbRes->currentIndex();
    prcopt.glomodear = GloAmbRes->currentIndex();
    prcopt.bdsmodear = BdsAmbRes->currentIndex();
    prcopt.thresar[0] = ValidThresAR->value();
    prcopt.maxout = OutCntResetAmb->value();
    prcopt.minfix = FixCntHoldAmb->value();
    prcopt.minlock = LockCntFixAmb->value();
    prcopt.elmaskar = ElMaskAR->value() * D2R;
    prcopt.elmaskhold = ElMaskHold->value() * D2R;
    prcopt.maxtdiff = MaxAgeDiff->value();
    prcopt.maxgdop = RejectGdop->value();
    prcopt.maxinno = RejectThres->value();
    prcopt.thresslip = SlipThres->value();
    prcopt.niter = NumIter->value();
    prcopt.syncsol = SyncSol->currentIndex();
    if (prcopt.mode == PMODE_MOVEB && BaselineConst->isChecked()) {
        prcopt.baseline[0] = BaselineLen->value();
        prcopt.baseline[1] = BaselineSig->value();
	}
    solopt.posf = SolFormat->currentIndex();
    solopt.timef = TimeFormat->currentIndex() == 0 ? 0 : 1;
    solopt.times = TimeFormat->currentIndex() == 0 ? 0 : TimeFormat->currentIndex() - 1;
    solopt.timeu = TimeDecimal->value();
    solopt.degf = LatLonFormat->currentIndex();
    strcpy(solopt.sep, qPrintable(FieldSep_Text));
    solopt.outhead = OutputHead->currentIndex();
    solopt.outopt = OutputOpt->currentIndex();
    solopt.outvel =OutputVel->currentIndex();
    prcopt.outsingle = OutputSingle->currentIndex();
    solopt.maxsolstd = MaxSolStd->value();
    solopt.datum = OutputDatum->currentIndex();
    solopt.height = OutputHeight->currentIndex();
    solopt.geoid = OutputGeoid->currentIndex();
    solopt.nmeaintv[0] = NmeaIntv1->value();
    solopt.nmeaintv[1] = NmeaIntv2->value();
    solopt.trace = DebugTrace->currentIndex();
    solopt.sstat = DebugStatus->currentIndex();

    prcopt.eratio[0] = MeasErrR1->value();
    prcopt.eratio[1] = MeasErrR2->value();
    prcopt.err[1] = MeasErr2->value();
    prcopt.err[2] = MeasErr3->value();
    prcopt.err[3] = MeasErr4->value();
    prcopt.err[4] = MeasErr5->value();
    prcopt.sclkstab = SatClkStab->text().toDouble();
    prcopt.prn[0] = PrNoise1->text().toDouble();
    prcopt.prn[1] = PrNoise2->text().toDouble();
    prcopt.prn[2] = PrNoise3->text().toDouble();
    prcopt.prn[3] = PrNoise4->text().toDouble();
    prcopt.prn[4] = PrNoise5->text().toDouble();

    if (RovAntPcv->isChecked()) strcpy(prcopt.anttype[0], qPrintable(RovAnt_Text));
    if (RefAntPcv->isChecked()) strcpy(prcopt.anttype[1], qPrintable(RefAnt_Text));
    prcopt.antdel[0][0] = RovAntE->value();
    prcopt.antdel[0][1] = RovAntN->value();
    prcopt.antdel[0][2] = RovAntU->value();
    prcopt.antdel[1][0] = RefAntE->value();
    prcopt.antdel[1][1] = RefAntN->value();
    prcopt.antdel[1][2] = RefAntU->value();
    prcopt.maxaveep = MaxAveEp->value();
    prcopt.initrst = ChkInitRestart->isChecked();

    prcopt.rovpos = 4;
    prcopt.refpos = 4;
    if      (RefPosTypeP->currentIndex()==3) prcopt.refpos=POSOPT_RTCM;
    else if (RefPosTypeP->currentIndex()==4) prcopt.refpos=POSOPT_SINGLE;

    if (prcopt.rovpos == POSOPT_POS) GetPos(RovPosTypeP->currentIndex(), editu, prcopt.ru);
    if (prcopt.refpos == POSOPT_POS) GetPos(RefPosTypeP->currentIndex(), editr, prcopt.rb);

    strcpy(filopt.satantp, qPrintable(SatPcvFile_Text));
    strcpy(filopt.rcvantp, qPrintable(AntPcvFile_Text));
    strcpy(filopt.stapos, qPrintable(StaPosFile_Text));
    strcpy(filopt.geoid, qPrintable(GeoidDataFile_Text));
    strcpy(filopt.dcb, qPrintable(DCBFile_Text));
    strcpy(filopt.tempdir, qPrintable(LocalDir_Text));

    time2str(utc2gpst(timeget()), s, 0);
    sprintf(comment, qPrintable(tr("RTKNAVI options (%s, v.%s)")), s, VER_RTKLIB);
    setsysopts(&prcopt, &solopt, &filopt);
    if (!saveopts(qPrintable(file), "w", comment, sysopts) ||
        !saveopts(qPrintable(file), "a", "", rcvopts)) return;
}
//---------------------------------------------------------------------------
void OptDialog::UpdateEnable(void)
{
    bool rel = PMODE_DGPS <= PosMode->currentIndex() && PosMode->currentIndex() <= PMODE_FIXED;
    bool rtk = PMODE_KINEMA <= PosMode->currentIndex() && PosMode->currentIndex() <= PMODE_FIXED;
    bool ppp = PosMode->currentIndex() >= PMODE_PPP_KINEMA;
    bool ar = rtk || ppp;

    Freq->setEnabled(rel);
    Solution->setEnabled(false);
    DynamicModel->setEnabled(rel);
    TideCorr->setEnabled(rel || ppp);
    PosOpt1->setEnabled(ppp);
    PosOpt2->setEnabled(ppp);
    PosOpt3->setEnabled(ppp);
    PosOpt4->setEnabled(ppp);

    AmbRes->setEnabled(ar);
    GloAmbRes->setEnabled(ar && AmbRes->currentIndex() > 0 && NavSys2->isChecked());
    BdsAmbRes->setEnabled(ar && AmbRes->currentIndex() > 0 && NavSys6->isChecked());
    ValidThresAR->setEnabled(ar && AmbRes->currentIndex() >= 1 && AmbRes->currentIndex() < 4);
    ThresAR2->setEnabled(ar && AmbRes->currentIndex() >= 4);
    ThresAR3->setEnabled(ar && AmbRes->currentIndex() >= 4);
    LockCntFixAmb->setEnabled(ar && AmbRes->currentIndex() >= 1);
    ElMaskAR->setEnabled(ar && AmbRes->currentIndex() >= 1);
    OutCntResetAmb->setEnabled(ar || ppp);
    FixCntHoldAmb->setEnabled(ar && AmbRes->currentIndex() == 3);
    ElMaskHold->setEnabled(ar && AmbRes->currentIndex() == 3);
    SlipThres->setEnabled(ar || ppp);
    MaxAgeDiff->setEnabled(rel);
    RejectThres->setEnabled(rel || ppp);
    NumIter->setEnabled(rel || ppp);
    SyncSol->setEnabled(rel || ppp);
    BaselineConst->setEnabled(PosMode->currentIndex() == PMODE_MOVEB);
    BaselineLen->setEnabled(BaselineConst->isChecked() && PosMode->currentIndex() == PMODE_MOVEB);
    BaselineSig->setEnabled(BaselineConst->isChecked() && PosMode->currentIndex() == PMODE_MOVEB);

    OutputHead->setEnabled(SolFormat->currentIndex() != 3);
    OutputOpt->setEnabled(false);
    TimeFormat->setEnabled(SolFormat->currentIndex() != 3);
    TimeDecimal->setEnabled(SolFormat->currentIndex() != 3);
    LatLonFormat->setEnabled(SolFormat->currentIndex() == 0);
    FieldSep->setEnabled(SolFormat->currentIndex() != 3);
    OutputSingle->setEnabled(PosMode->currentIndex() != 0);
    OutputDatum->setEnabled(SolFormat->currentIndex() == 0);
    OutputHeight->setEnabled(SolFormat->currentIndex() == 0);
    OutputGeoid->setEnabled(SolFormat->currentIndex() == 0 && OutputHeight->currentIndex() == 1);

    RovAntPcv->setEnabled(rel || ppp);
    RovAnt->setEnabled((rel || ppp) && RovAntPcv->isChecked());
    RovAntE->setEnabled((rel || ppp) && RovAntPcv->isChecked()&&RovAnt->currentText()!="*");
    RovAntN->setEnabled((rel || ppp) && RovAntPcv->isChecked()&&RovAnt->currentText()!="*");
    RovAntU->setEnabled((rel || ppp) && RovAntPcv->isChecked()&&RovAnt->currentText()!="*");
    LabelRovAntD->setEnabled((rel || ppp) && RovAntPcv->isChecked()&&RovAnt->currentText()!="*");
    RefAntPcv->setEnabled(rel);
    RefAnt->setEnabled(rel && RefAntPcv->isChecked());
    RefAntE->setEnabled(rel && RefAntPcv->isChecked()&&RefAnt->currentText()!="*");
    RefAntN->setEnabled(rel && RefAntPcv->isChecked()&&RefAnt->currentText()!="*");
    RefAntU->setEnabled(rel && RefAntPcv->isChecked()&&RefAnt->currentText()!="*");
    LabelRefAntD->setEnabled(rel && RefAntPcv->isChecked()&&RefAnt->currentText()!="*");

    RovPosTypeP->setEnabled(PosMode->currentIndex() == PMODE_FIXED || PosMode->currentIndex() == PMODE_PPP_FIXED);
    RovPos1->setEnabled(RovPosTypeP->isEnabled() && RovPosTypeP->currentIndex() <= 2);
    RovPos2->setEnabled(RovPosTypeP->isEnabled() && RovPosTypeP->currentIndex() <= 2);
    RovPos3->setEnabled(RovPosTypeP->isEnabled() && RovPosTypeP->currentIndex() <= 2);
    BtnRovPos->setEnabled(RovPosTypeP->isEnabled() && RovPosTypeP->currentIndex() <= 2);

    RefPosTypeP->setEnabled(rel && PosMode->currentIndex() != PMODE_MOVEB);
    RefPos1->setEnabled(RefPosTypeP->isEnabled() && RefPosTypeP->currentIndex() <= 2);
    RefPos2->setEnabled(RefPosTypeP->isEnabled() && RefPosTypeP->currentIndex() <= 2);
    RefPos3->setEnabled(RefPosTypeP->isEnabled() && RefPosTypeP->currentIndex() <= 2);
    BtnRefPos->setEnabled(RefPosTypeP->isEnabled() && RefPosTypeP->currentIndex() <= 2);

    LabelMaxAveEp->setVisible(RefPosTypeP->currentIndex() == 4);
    MaxAveEp->setVisible(RefPosTypeP->currentIndex() == 4);
    ChkInitRestart->setVisible(RefPosTypeP->currentIndex() == 4);

    SbasSatE->setEnabled(PosMode->currentIndex() == 0);
}
//---------------------------------------------------------------------------
void OptDialog::GetPos(int type, QLineEdit **edit, double *pos)
{
    double p[3] = { 0 }, dms1[3] = { 0 }, dms2[3] = { 0 };

    if (type == 1) { /* lat/lon/height dms/m */
        QStringList tokens = edit[0]->text().split(" ");
        for (int i = 0; i < 3 || i < tokens.size(); i++)
            dms1[i] = tokens.at(i).toDouble();

        tokens = edit[1]->text().split(" ");
        for (int i = 0; i < 3 || i < tokens.size(); i++)
            dms2[i] = tokens.at(i).toDouble();

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
void OptDialog::SetPos(int type, QLineEdit **edit, double *pos)
{
    double p[3], dms1[3], dms2[3], s1, s2;

    if (type == 1) { /* lat/lon/height dms/m */
        ecef2pos(pos, p); s1 = p[0] < 0 ? -1 : 1; s2 = p[1] < 0 ? -1 : 1;
        p[0] = fabs(p[0]) * R2D + 1E-12; p[1] = fabs(p[1]) * R2D + 1E-12;
        dms1[0] = floor(p[0]); p[0] = (p[0] - dms1[0]) * 60.0;
        dms1[1] = floor(p[0]); dms1[2] = (p[0] - dms1[1]) * 60.0;
        dms2[0] = floor(p[1]); p[1] = (p[1] - dms2[0]) * 60.0;
        dms2[1] = floor(p[1]); dms2[2] = (p[1] - dms2[1]) * 60.0;
        edit[0]->setText(QString("%1 %2 %3").arg(s1 * dms1[0], 0, 'f', 0).arg(dms1[1], 2, 'f', 0, '0').arg(dms1[2], 9, 'f', 6, '0'));
        edit[1]->setText(QString("%1 %2 %3").arg(s2 * dms2[0], 0, 'f', 0).arg(dms2[1], 2, 'f', 0, '0').arg(dms2[2], 9, 'f', 6, '0'));
        edit[2]->setText(QString::number(p[2], 'f', 4));
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
void OptDialog::ReadAntList(void)
{
    QString AntPcvFile_Text = AntPcvFile->text();
    QStringList list;
    pcvs_t pcvs = { 0, 0, NULL };
	char *p;

    if (!readpcv(qPrintable(AntPcvFile_Text), &pcvs)) return;

    list.append("");
    list.append("*");

    for (int i = 0; i < pcvs.n; i++) {
		if (pcvs.pcv[i].sat) continue;
        if ((p = strchr(pcvs.pcv[i].type, ' '))) *p = '\0';
        if (i > 0 && !strcmp(pcvs.pcv[i].type, pcvs.pcv[i - 1].type)) continue;
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
void OptDialog::BtnFreqClick()
{
    freqDialog->exec();
}
//---------------------------------------------------------------------------
void OptDialog::RefAntClick()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void OptDialog::RovAntClick()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------



