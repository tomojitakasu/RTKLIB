//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "naviopt.h"
#include "viewer.h"
#include "refdlg.h"
#include "navimain.h"
#include "maskoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOptDialog *OptDialog;

#define MAXSTR      1024                /* max length of a string */

//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
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
__fastcall TOptDialog::TOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	AnsiString label,s;
	int freq[]={1,2,5,6,7,8,9},nglo=MAXPRNGLO,ngal=MAXPRNGAL,nqzs=MAXPRNQZS;
	int ncmp=MAXPRNCMP,nirn=MAXPRNIRN;
	PrcOpt=prcopt_default;
	SolOpt=solopt_default;
	UpdateEnable();
	PosFont=new TFont;
	
	Freq->Items->Clear();
	for (int i=0;i<NFREQ;i++) {
		label=label+(i>0?"+":"")+s.sprintf("L%d",freq[i]);
		Freq->Items->Add(label);
	}
	if (nglo<=0) NavSys2->Enabled=false;
	if (ngal<=0) NavSys3->Enabled=false;
	if (nqzs<=0) NavSys4->Enabled=false;
	if (ncmp<=0) NavSys6->Enabled=false;
	if (nirn<=0) NavSys7->Enabled=false;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::FormShow(TObject *Sender)
{
	GetOpt();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnOkClick(TObject *Sender)
{
	SetOpt();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnLoadClick(TObject *Sender)
{
	OpenDialog->Title="Load Options";
	OpenDialog->FilterIndex=4;
	if (!OpenDialog->Execute()) return;
	LoadOpt(OpenDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSaveClick(TObject *Sender)
{
	AnsiString file;
	SaveDialog->Title="Save Options";
	SaveDialog->FilterIndex=2;
	if (!SaveDialog->Execute()) return;
	file=SaveDialog->FileName;
	if (!strrchr(file.c_str(),'.')) file=file+".conf";
	SaveOpt(file);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnStaPosViewClick(TObject *Sender)
{
	if (StaPosFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(StaPosFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnStaPosFileClick(TObject *Sender)
{
	OpenDialog->Title="Station Postion File";
	OpenDialog->FilterIndex=3;
	if (!OpenDialog->Execute()) return;
	StaPosFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSnrMaskClick(TObject *Sender)
{
	MaskOptDialog->Mask=PrcOpt.snrmask;
	if (MaskOptDialog->ShowModal()!=mrOk) return;
	PrcOpt.snrmask=MaskOptDialog->Mask;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovPosTypePChange(TObject *Sender)
{
	TEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double pos[3];
	GetPos(RovPosTypeF,edit,pos);
	SetPos(RovPosTypeP->ItemIndex,edit,pos);
	RovPosTypeF=RovPosTypeP->ItemIndex;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RefPosTypePChange(TObject *Sender)
{
	TEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double pos[3];
	GetPos(RefPosTypeF,edit,pos);
	SetPos(RefPosTypeP->ItemIndex,edit,pos);
	RefPosTypeF=RefPosTypeP->ItemIndex;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnRovPosClick(TObject *Sender)
{
	TEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double p[3],pos[3];
	GetPos(RovPosTypeP->ItemIndex,edit,p);
	ecef2pos(p,pos);
	RefDialog->RovPos[0]=pos[0]*R2D;
	RefDialog->RovPos[1]=pos[1]*R2D;
	RefDialog->Pos[2]=pos[2];
	RefDialog->StaPosFile=StaPosFile->Text;
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	pos[0]=RefDialog->Pos[0]*D2R;
	pos[1]=RefDialog->Pos[1]*D2R;
	pos[2]=RefDialog->Pos[2];
	pos2ecef(pos,p);
	SetPos(RovPosTypeP->ItemIndex,edit,p);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnRefPosClick(TObject *Sender)
{
	TEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double p[3],pos[3];
	GetPos(RefPosTypeP->ItemIndex,edit,p);
	ecef2pos(p,pos);
	RefDialog->RovPos[0]=pos[0]*R2D;
	RefDialog->RovPos[1]=pos[1]*R2D;
	RefDialog->RovPos[2]=pos[2];
	RefDialog->StaPosFile=StaPosFile->Text;
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	pos[0]=RefDialog->Pos[0]*D2R;
	pos[1]=RefDialog->Pos[1]*D2R;
	pos[2]=RefDialog->Pos[2];
	pos2ecef(pos,p);
	SetPos(RefPosTypeP->ItemIndex,edit,p);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSatPcvViewClick(TObject *Sender)
{
	if (SatPcvFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(SatPcvFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSatPcvFileClick(TObject *Sender)
{
	OpenDialog->Title="Satellite Antenna PCV File";
	OpenDialog->FilterIndex=2;
	if (!OpenDialog->Execute()) return;
	SatPcvFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnAntPcvViewClick(TObject *Sender)
{
	if (AntPcvFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(AntPcvFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnAntPcvFileClick(TObject *Sender)
{
	OpenDialog->Title="Receiver Antenna PCV File";
	OpenDialog->FilterIndex=2;
	if (!OpenDialog->Execute()) return;
	AntPcvFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnGeoidDataFileClick(TObject *Sender)
{
	OpenDialog->Title="Geoid Data File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	GeoidDataFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnDCBFileClick(TObject *Sender)
{
	OpenDialog->Title="DCB Data File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	DCBFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnEOPFileClick(TObject *Sender)
{
	OpenDialog->Title="EOP Data File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	EOPFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnEOPViewClick(TObject *Sender)
{
	if (EOPFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(EOPFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnLocalDirClick(TObject *Sender)
{
#ifdef TCPP
	AnsiString dir=LocalDir->Text;
	if (!SelectDirectory("FTP/HTTP Local Directory","",dir)) return;
	LocalDir->Text=dir;
#else
    UnicodeString dir=LocalDir->Text;
    TSelectDirExtOpts opt=TSelectDirExtOpts()<<sdNewUI<<sdNewFolder;
    if (!SelectDirectory(L"FTP/HTTP Local Directory",L"",dir,opt)) return;
    LocalDir->Text=dir;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnFontClick(TObject *Sender)
{
	AnsiString s;
	FontDialog->Font=FontLabel->Font;
	if (!FontDialog->Execute()) return;
	FontLabel->Font=FontDialog->Font;
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnTLESatFileClick(TObject *Sender)
{
	OpenDialog->Title="TLE Satellite Number File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	TLESatFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnTLEFileClick(TObject *Sender)
{
	OpenDialog->Title="TLE Data File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	TLEFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::FreqChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::NavSys2Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BaselineConstClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SolFormatChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::PosModeChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::AmbResChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovAntPcvClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SatClkCorrClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::IntpRefObsClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovPosClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RefPosClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::OutputHeightClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::NmeaReqCClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::DgpsCorrLChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::GetOpt(void)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	AnsiString s;
	
	PosMode		 ->ItemIndex=PrcOpt.mode;
	Freq		 ->ItemIndex=PrcOpt.nf-1>NFREQ-1?NFREQ-1:PrcOpt.nf-1;
	ElMask		 ->Text     =s.sprintf("%.0f",PrcOpt.elmin*R2D);
	DynamicModel ->ItemIndex=PrcOpt.dynamics;
	TideCorr	 ->ItemIndex=PrcOpt.tidecorr;
	IonoOpt		 ->ItemIndex=PrcOpt.ionoopt;
	TropOpt		 ->ItemIndex=PrcOpt.tropopt;
	SatEphem	 ->ItemIndex=PrcOpt.sateph;
	AmbRes		 ->ItemIndex=PrcOpt.modear;
	GloAmbRes	 ->ItemIndex=PrcOpt.glomodear;
	BdsAmbRes	 ->ItemIndex=PrcOpt.bdsmodear;
	ValidThresAR ->Text     =s.sprintf("%.1f",PrcOpt.thresar[0]);
	OutCntResetAmb->Text    =s.sprintf("%d",  PrcOpt.maxout);
	LockCntFixAmb->Text     =s.sprintf("%d",  PrcOpt.minlock);
	FixCntHoldAmb->Text     =s.sprintf("%d",  PrcOpt.minfix);
	ElMaskAR	 ->Text     =s.sprintf("%.0f",PrcOpt.elmaskar*R2D);
	ElMaskHold	 ->Text     =s.sprintf("%.0f",PrcOpt.elmaskhold*R2D);
	MaxAgeDiff	 ->Text     =s.sprintf("%.1f",PrcOpt.maxtdiff);
	RejectGdop   ->Text     =s.sprintf("%.1f",PrcOpt.maxgdop);
	RejectThres  ->Text     =s.sprintf("%.1f",PrcOpt.maxinno);
	SlipThres	 ->Text     =s.sprintf("%.3f",PrcOpt.thresslip);
	ARIter		 ->Text     =s.sprintf("%d",  PrcOpt.armaxiter);
	NumIter		 ->Text     =s.sprintf("%d",  PrcOpt.niter);
	SyncSol		 ->ItemIndex=PrcOpt.syncsol;
	ExSatsE		 ->Text     =ExSats;
	NavSys1		 ->Checked  =PrcOpt.navsys&SYS_GPS;
	NavSys2		 ->Checked  =PrcOpt.navsys&SYS_GLO;
	NavSys3		 ->Checked  =PrcOpt.navsys&SYS_GAL;
	NavSys4		 ->Checked  =PrcOpt.navsys&SYS_QZS;
	NavSys5		 ->Checked  =PrcOpt.navsys&SYS_SBS;
	NavSys6		 ->Checked  =PrcOpt.navsys&SYS_CMP;
	NavSys7		 ->Checked  =PrcOpt.navsys&SYS_IRN;
	PosOpt1		 ->Checked  =PrcOpt.posopt[0];
	PosOpt2		 ->Checked  =PrcOpt.posopt[1];
	PosOpt3		 ->Checked  =PrcOpt.posopt[2];
	PosOpt4		 ->Checked  =PrcOpt.posopt[3];
	PosOpt5		 ->Checked  =PrcOpt.posopt[4];
	
	SolFormat	 ->ItemIndex=SolOpt.posf;
	TimeFormat	 ->ItemIndex=SolOpt.timef==0?0:SolOpt.times+1;
	TimeDecimal	 ->Text     =s.sprintf("%d",SolOpt.timeu);
	LatLonFormat ->ItemIndex=SolOpt.degf;
	FieldSep	 ->Text     =SolOpt.sep;
	OutputHead	 ->ItemIndex=SolOpt.outhead;
	OutputOpt	 ->ItemIndex=SolOpt.outopt;
	OutputSingle ->ItemIndex=PrcOpt.outsingle;
	MaxSolStd	 ->Text		=s.sprintf("%.2g",SolOpt.maxsolstd);
	OutputDatum  ->ItemIndex=SolOpt.datum;
	OutputHeight ->ItemIndex=SolOpt.height;
	OutputGeoid  ->ItemIndex=SolOpt.geoid;
	NmeaIntv1    ->Text     =s.sprintf("%.2g",SolOpt.nmeaintv[0]);
	NmeaIntv2    ->Text     =s.sprintf("%.2g",SolOpt.nmeaintv[1]);
	DebugStatus	 ->ItemIndex=DebugStatusF;
	DebugTrace	 ->ItemIndex=DebugTraceF;
	
	BaselineConst->Checked  =BaselineC;
	BaselineLen->Text       =s.sprintf("%.3f",Baseline[0]);
	BaselineSig->Text       =s.sprintf("%.3f",Baseline[1]);
	
	MeasErrR1	 ->Text     =s.sprintf("%.1f",PrcOpt.eratio[0]);
	MeasErrR2	 ->Text     =s.sprintf("%.1f",PrcOpt.eratio[1]);
	MeasErr2	 ->Text     =s.sprintf("%.3f",PrcOpt.err[1]);
	MeasErr3	 ->Text     =s.sprintf("%.3f",PrcOpt.err[2]);
	MeasErr4	 ->Text     =s.sprintf("%.3f",PrcOpt.err[3]);
	MeasErr5	 ->Text     =s.sprintf("%.3f",PrcOpt.err[4]);
	PrNoise1	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[0]);
	PrNoise2	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[1]);
	PrNoise3	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[2]);
	PrNoise4	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[3]);
	PrNoise5	 ->Text     =s.sprintf("%.2E",PrcOpt.prn[4]);
	SatClkStab	 ->Text     =s.sprintf("%.2E",PrcOpt.sclkstab);
	MaxAveEp	 ->Text		=s.sprintf("%d",PrcOpt.maxaveep);
	ChkInitRestart->Checked =PrcOpt.initrst;
	
	RovPosTypeP	 ->ItemIndex=RovPosTypeF;
	RefPosTypeP	 ->ItemIndex=RefPosTypeF;
	RovAntPcv	 ->Checked  =RovAntPcvF;
	RefAntPcv	 ->Checked  =RefAntPcvF;
	RovAnt		 ->Text     =RovAntF;
	RefAnt		 ->Text     =RefAntF;
	RovAntE		 ->Text     =s.sprintf("%.4f",RovAntDel[0]);
	RovAntN		 ->Text     =s.sprintf("%.4f",RovAntDel[1]);
	RovAntU		 ->Text     =s.sprintf("%.4f",RovAntDel[2]);
	RefAntE		 ->Text     =s.sprintf("%.4f",RefAntDel[0]);
	RefAntN		 ->Text     =s.sprintf("%.4f",RefAntDel[1]);
	RefAntU		 ->Text     =s.sprintf("%.4f",RefAntDel[2]);
	SetPos(RovPosTypeP->ItemIndex,editu,RovPos);
	SetPos(RefPosTypeP->ItemIndex,editr,RefPos);
	
	SatPcvFile	 ->Text     =SatPcvFileF;
	AntPcvFile	 ->Text     =AntPcvFileF;
	StaPosFile	 ->Text     =StaPosFileF;
	GeoidDataFile->Text     =GeoidDataFileF;
	DCBFile      ->Text     =DCBFileF;
	EOPFile      ->Text     =EOPFileF;
	TLEFile      ->Text     =TLEFileF;
	TLESatFile   ->Text     =TLESatFileF;
	LocalDir	 ->Text     =LocalDirectory;
	ReadAntList();
	
	SvrCycleE	 ->Text     =s.sprintf("%d",SvrCycle);
	TimeoutTimeE ->Text     =s.sprintf("%d",TimeoutTime);
	ReconTimeE   ->Text     =s.sprintf("%d",ReconTime);
	NmeaCycleE   ->Text     =s.sprintf("%d",NmeaCycle);
	FileSwapMarginE->Text   =s.sprintf("%d",FileSwapMargin);
	SvrBuffSizeE ->Text     =s.sprintf("%d",SvrBuffSize);
	SolBuffSizeE ->Text     =s.sprintf("%d",SolBuffSize);
	SavedSolE    ->Text     =s.sprintf("%d",SavedSol);
	NavSelectS   ->ItemIndex=NavSelect;
	SbasSatE     ->Text     =s.sprintf("%d",PrcOpt.sbassatsel);
	ProxyAddrE   ->Text     =ProxyAddr;
	MoniPortE    ->Text     =s.sprintf("%d",MoniPort);
	SolBuffSizeE ->Text     =s.sprintf("%d",SolBuffSize);
	PanelStackE  ->ItemIndex=PanelStack;
	
	FontLabel->Font->Assign(PosFont);
	FontLabel->Caption=FontLabel->Font->Name+s.sprintf(" %dpt",FontLabel->Font->Size);
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SetOpt(void)
{
	AnsiString FieldSep_Text=FieldSep->Text;
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	
	PrcOpt.mode      =PosMode     ->ItemIndex;
	PrcOpt.nf        =Freq        ->ItemIndex+1;
	PrcOpt.elmin     =str2dbl(ElMask ->Text)*D2R;
	PrcOpt.dynamics  =DynamicModel->ItemIndex;
	PrcOpt.tidecorr  =TideCorr    ->ItemIndex;
	PrcOpt.ionoopt   =IonoOpt     ->ItemIndex;
	PrcOpt.tropopt   =TropOpt     ->ItemIndex;
	PrcOpt.sateph    =SatEphem    ->ItemIndex;
	PrcOpt.modear    =AmbRes      ->ItemIndex;
	PrcOpt.glomodear =GloAmbRes   ->ItemIndex;
	PrcOpt.bdsmodear =BdsAmbRes   ->ItemIndex;
	PrcOpt.thresar[0]=str2dbl(ValidThresAR->Text);
	PrcOpt.maxout    =OutCntResetAmb->Text.ToInt();
	PrcOpt.minlock   =LockCntFixAmb->Text.ToInt();
	PrcOpt.minfix    =FixCntHoldAmb->Text.ToInt();
	PrcOpt.elmaskar  =str2dbl(ElMaskAR   ->Text)*D2R;
	PrcOpt.elmaskhold=str2dbl(ElMaskHold ->Text)*D2R;
	PrcOpt.maxtdiff  =str2dbl(MaxAgeDiff ->Text);
	PrcOpt.maxgdop   =str2dbl(RejectGdop ->Text);
	PrcOpt.maxinno   =str2dbl(RejectThres->Text);
	PrcOpt.thresslip =str2dbl(SlipThres  ->Text);
	PrcOpt.armaxiter =ARIter      ->Text.ToInt();
	PrcOpt.niter     =NumIter     ->Text.ToInt();
	PrcOpt.syncsol   =SyncSol     ->ItemIndex;
	ExSats			 =ExSatsE	  ->Text;
	PrcOpt.navsys    =0;
	if (NavSys1->Checked) PrcOpt.navsys|=SYS_GPS;
	if (NavSys2->Checked) PrcOpt.navsys|=SYS_GLO;
	if (NavSys3->Checked) PrcOpt.navsys|=SYS_GAL;
	if (NavSys4->Checked) PrcOpt.navsys|=SYS_QZS;
	if (NavSys5->Checked) PrcOpt.navsys|=SYS_SBS;
	if (NavSys6->Checked) PrcOpt.navsys|=SYS_CMP;
	if (NavSys7->Checked) PrcOpt.navsys|=SYS_IRN;
	PrcOpt.posopt[0] =PosOpt1   ->Checked;
	PrcOpt.posopt[1] =PosOpt2   ->Checked;
	PrcOpt.posopt[2] =PosOpt3   ->Checked;
	PrcOpt.posopt[3] =PosOpt4   ->Checked;
	PrcOpt.posopt[4] =PosOpt5   ->Checked;
	PrcOpt.posopt[5] =PosOpt6   ->Checked;
	
	SolOpt.posf      =SolFormat   ->ItemIndex;
	SolOpt.timef     =TimeFormat->ItemIndex==0?0:1;
	SolOpt.times     =TimeFormat->ItemIndex==0?0:TimeFormat->ItemIndex-1;
	SolOpt.timeu     =(int)str2dbl(TimeDecimal->Text);
	SolOpt.degf      =LatLonFormat->ItemIndex;
	strcpy(SolOpt.sep,FieldSep_Text.c_str());
	SolOpt.outhead   =OutputHead  ->ItemIndex;
	SolOpt.outopt    =OutputOpt   ->ItemIndex;
	PrcOpt.outsingle =OutputSingle->ItemIndex;
	SolOpt.maxsolstd =str2dbl(MaxSolStd->Text);
	SolOpt.datum     =OutputDatum ->ItemIndex;
	SolOpt.height    =OutputHeight->ItemIndex;
	SolOpt.geoid     =OutputGeoid ->ItemIndex;
	SolOpt.nmeaintv[0]=str2dbl(NmeaIntv1->Text);
	SolOpt.nmeaintv[1]=str2dbl(NmeaIntv2->Text);
	DebugStatusF     =DebugStatus ->ItemIndex;
	DebugTraceF      =DebugTrace  ->ItemIndex;
	
	BaselineC        =BaselineConst->Checked;
	Baseline[0]      =str2dbl(BaselineLen->Text);
	Baseline[1]      =str2dbl(BaselineSig->Text);
	
	PrcOpt.eratio[0] =str2dbl(MeasErrR1 ->Text);
	PrcOpt.eratio[1] =str2dbl(MeasErrR2 ->Text);
	PrcOpt.err[1]    =str2dbl(MeasErr2  ->Text);
	PrcOpt.err[2]    =str2dbl(MeasErr3  ->Text);
	PrcOpt.err[3]    =str2dbl(MeasErr4  ->Text);
	PrcOpt.err[4]    =str2dbl(MeasErr5  ->Text);
	PrcOpt.prn[0]    =str2dbl(PrNoise1  ->Text);
	PrcOpt.prn[1]    =str2dbl(PrNoise2  ->Text);
	PrcOpt.prn[2]    =str2dbl(PrNoise3  ->Text);
	PrcOpt.prn[3]    =str2dbl(PrNoise4  ->Text);
	PrcOpt.prn[4]    =str2dbl(PrNoise5  ->Text);
	PrcOpt.sclkstab  =str2dbl(SatClkStab->Text);
	PrcOpt.maxaveep  =MaxAveEp->Text.ToInt();
	PrcOpt.initrst   =ChkInitRestart->Checked;
	
	RovPosTypeF      =RovPosTypeP ->ItemIndex;
	RefPosTypeF      =RefPosTypeP ->ItemIndex;
	RovAntPcvF       =RovAntPcv   ->Checked;
	RefAntPcvF       =RefAntPcv   ->Checked;
	RovAntF          =RovAnt      ->Text;
	RefAntF          =RefAnt      ->Text;
	RovAntDel[0]     =str2dbl(RovAntE   ->Text);
	RovAntDel[1]     =str2dbl(RovAntN   ->Text);
	RovAntDel[2]     =str2dbl(RovAntU   ->Text);
	RefAntDel[0]     =str2dbl(RefAntE   ->Text);
	RefAntDel[1]     =str2dbl(RefAntN   ->Text);
	RefAntDel[2]     =str2dbl(RefAntU   ->Text);
	GetPos(RovPosTypeP->ItemIndex,editu,RovPos);
	GetPos(RefPosTypeP->ItemIndex,editr,RefPos);
	
	SatPcvFileF      =SatPcvFile  ->Text;
	AntPcvFileF      =AntPcvFile  ->Text;
	StaPosFileF      =StaPosFile  ->Text;
	GeoidDataFileF   =GeoidDataFile->Text;
	DCBFileF         =DCBFile     ->Text;
	EOPFileF         =EOPFile     ->Text;
	TLEFileF         =TLEFile     ->Text;
	TLESatFileF      =TLESatFile  ->Text;
	LocalDirectory   =LocalDir    ->Text;
	
	SvrCycle	     =SvrCycleE   ->Text.ToInt();
	TimeoutTime      =TimeoutTimeE->Text.ToInt();
	ReconTime        =ReconTimeE  ->Text.ToInt();
	NmeaCycle	     =NmeaCycleE  ->Text.ToInt();
	FileSwapMargin   =FileSwapMarginE->Text.ToInt();
	SvrBuffSize      =SvrBuffSizeE->Text.ToInt();
	SolBuffSize      =SolBuffSizeE->Text.ToInt();
	SavedSol         =SavedSolE   ->Text.ToInt();
	NavSelect        =NavSelectS  ->ItemIndex;
	PrcOpt.sbassatsel=SbasSatE    ->Text.ToInt();
	ProxyAddr        =ProxyAddrE  ->Text;
	MoniPort         =MoniPortE   ->Text.ToInt();
	PanelStack       =PanelStackE ->ItemIndex;
	
	PosFont->Assign(FontLabel->Font);
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::LoadOpt(AnsiString file)
{
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPC_C,STR_FILE};
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	AnsiString s;
	char buff[1024]="",*p,id[32];
	int sat;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
	filopt_t filopt={""};
	
	resetsysopts();
	if (!loadopts(file.c_str(),sysopts)||
	    !loadopts(file.c_str(),rcvopts)) return;
	getsysopts(&prcopt,&solopt,&filopt);
	
	for (int i=0;i<8;i++) {
		MainForm->StreamC[i]=strtype[i]!=STR_NONE;
		MainForm->Stream[i]=STR_NONE;
		for (int j=0;j<(i<3?7:5);j++) {
			if (strtype[i]!=(i<3?itype[j]:otype[j])) continue;
			MainForm->Stream[i]=j;
			break;
		}
		if (i<5) MainForm->Format[i]=strfmt[i];
		
		if (strtype[i]==STR_SERIAL) {
			MainForm->Paths[i][0]=strpath[i];
		}
		else if (strtype[i]==STR_FILE) {
			MainForm->Paths[i][2]=strpath[i];
		}
		else if (strtype[i]<=STR_NTRIPCLI) {
			MainForm->Paths[i][1]=strpath[i];
		}
		else if (strtype[i]<=STR_HTTP) {
			MainForm->Paths[i][3]=strpath[i];
		}
	}
	MainForm->NmeaReq=nmeareq;
	MainForm->NmeaPos[0]=nmeapos[0];
	MainForm->NmeaPos[1]=nmeapos[1];
	
	SbasSatE     ->Text         =s.sprintf("%d",prcopt.sbassatsel);
	
	PosMode		 ->ItemIndex	=prcopt.mode;
	Freq		 ->ItemIndex	=prcopt.nf>NFREQ-1?NFREQ-1:prcopt.nf-1;
	Solution	 ->ItemIndex	=prcopt.soltype;
	ElMask		 ->Text			=s.sprintf("%.0f",prcopt.elmin*R2D);
	DynamicModel ->ItemIndex	=prcopt.dynamics;
	TideCorr	 ->ItemIndex	=prcopt.tidecorr;
	IonoOpt		 ->ItemIndex	=prcopt.ionoopt;
	TropOpt		 ->ItemIndex	=prcopt.tropopt;
	SatEphem	 ->ItemIndex	=prcopt.sateph;
	ExSatsE	     ->Text			="";
	for (sat=1,p=buff;sat<=MAXSAT;sat++) {
		if (!prcopt.exsats[sat-1]) continue;
		satno2id(sat,id);
		p+=sprintf(p,"%s%s%s",p==buff?"":" ",prcopt.exsats[sat-1]==2?"+":"",id);
	}
	ExSatsE		 ->Text			=buff;
	NavSys1	     ->Checked		=prcopt.navsys&SYS_GPS;
	NavSys2	     ->Checked		=prcopt.navsys&SYS_GLO;
	NavSys3	     ->Checked		=prcopt.navsys&SYS_GAL;
	NavSys4	     ->Checked		=prcopt.navsys&SYS_QZS;
	NavSys5	     ->Checked		=prcopt.navsys&SYS_SBS;
	NavSys6	     ->Checked		=prcopt.navsys&SYS_CMP;
	NavSys7	     ->Checked		=prcopt.navsys&SYS_IRN;
	PosOpt1		 ->Checked		=prcopt.posopt[0];
	PosOpt2		 ->Checked		=prcopt.posopt[1];
	PosOpt3		 ->Checked		=prcopt.posopt[2];
	PosOpt4		 ->Checked		=prcopt.posopt[3];
	PosOpt5		 ->Checked		=prcopt.posopt[4];
	PosOpt6		 ->Checked		=prcopt.posopt[5];
	
	AmbRes		 ->ItemIndex	=prcopt.modear;
	GloAmbRes	 ->ItemIndex	=prcopt.glomodear;
	BdsAmbRes	 ->ItemIndex	=prcopt.bdsmodear;
	ValidThresAR ->Text			=s.sprintf("%.1f",prcopt.thresar[0]);
	OutCntResetAmb->Text		=s.sprintf("%d"  ,prcopt.maxout   );
	FixCntHoldAmb->Text			=s.sprintf("%d"  ,prcopt.minfix   );
	LockCntFixAmb->Text			=s.sprintf("%d"  ,prcopt.minlock  );
	ElMaskAR	 ->Text			=s.sprintf("%.0f",prcopt.elmaskar*R2D);
	ElMaskHold	 ->Text			=s.sprintf("%.0f",prcopt.elmaskhold*R2D);
	MaxAgeDiff	 ->Text			=s.sprintf("%.1f",prcopt.maxtdiff );
	RejectGdop   ->Text			=s.sprintf("%.1f",prcopt.maxgdop  );
	RejectThres  ->Text			=s.sprintf("%.1f",prcopt.maxinno  );
	SlipThres	 ->Text			=s.sprintf("%.3f",prcopt.thresslip);
	ARIter		 ->Text			=s.sprintf("%d",  prcopt.armaxiter);
	NumIter		 ->Text			=s.sprintf("%d",  prcopt.niter    );
	SyncSol		 ->ItemIndex	=prcopt.syncsol;
	BaselineLen	 ->Text			=s.sprintf("%.3f",prcopt.baseline[0]);
	BaselineSig	 ->Text			=s.sprintf("%.3f",prcopt.baseline[1]);
	BaselineConst->Checked		=prcopt.baseline[0]>0.0;
	
	SolFormat	 ->ItemIndex	=solopt.posf;
	TimeFormat	 ->ItemIndex	=solopt.timef==0?0:solopt.times+1;
	TimeDecimal	 ->Text			=s.sprintf("%d",solopt.timeu);
	LatLonFormat ->ItemIndex	=solopt.degf;
	FieldSep	 ->Text			=solopt.sep;
	OutputHead	 ->ItemIndex	=solopt.outhead;
	OutputOpt	 ->ItemIndex	=solopt.outopt;
	OutputSingle ->ItemIndex	=prcopt.outsingle;
	MaxSolStd	 ->Text			=s.sprintf("%.2g",solopt.maxsolstd);
	OutputDatum  ->ItemIndex	=solopt.datum;
	OutputHeight ->ItemIndex	=solopt.height;
	OutputGeoid  ->ItemIndex	=solopt.geoid;
	NmeaIntv1	 ->Text			=s.sprintf("%.2g",solopt.nmeaintv[0]);
	NmeaIntv2	 ->Text			=s.sprintf("%.2g",solopt.nmeaintv[1]);
	DebugTrace	 ->ItemIndex	=solopt.trace;
	DebugStatus	 ->ItemIndex	=solopt.sstat;
	
	MeasErrR1	 ->Text			=s.sprintf("%.1f",prcopt.eratio[0]);
	MeasErrR2	 ->Text			=s.sprintf("%.3f",prcopt.eratio[1]);
	MeasErr2	 ->Text			=s.sprintf("%.3f",prcopt.err[1]);
	MeasErr3	 ->Text			=s.sprintf("%.3f",prcopt.err[2]);
	MeasErr4	 ->Text			=s.sprintf("%.3f",prcopt.err[3]);
	MeasErr5	 ->Text			=s.sprintf("%.3f",prcopt.err[4]);
	SatClkStab	 ->Text			=s.sprintf("%.2E",prcopt.sclkstab);
	PrNoise1	 ->Text			=s.sprintf("%.2E",prcopt.prn[0]);
	PrNoise2	 ->Text			=s.sprintf("%.2E",prcopt.prn[1]);
	PrNoise3	 ->Text			=s.sprintf("%.2E",prcopt.prn[2]);
	PrNoise4	 ->Text			=s.sprintf("%.2E",prcopt.prn[3]);
	PrNoise5	 ->Text			=s.sprintf("%.2E",prcopt.prn[4]);
	
	RovAntPcv	 ->Checked		=*prcopt.anttype[0];
	RefAntPcv	 ->Checked		=*prcopt.anttype[1];
	RovAnt		 ->Text			=prcopt.anttype[0];
	RefAnt		 ->Text			=prcopt.anttype[1];
	RovAntE		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][0]);
	RovAntN		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][1]);
	RovAntU		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][2]);
	RefAntE		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][0]);
	RefAntN		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][1]);
	RefAntU		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][2]);
	MaxAveEp	 ->Text			=s.sprintf("%d",prcopt.maxaveep);
	ChkInitRestart->Checked		=prcopt.initrst;
	
	RovPosTypeP	 ->ItemIndex	=0;
	RefPosTypeP	 ->ItemIndex	=0;
	if      (prcopt.refpos==POSOPT_RTCM  ) RefPosTypeP->ItemIndex=3;
	else if (prcopt.refpos==POSOPT_RAW   ) RefPosTypeP->ItemIndex=4;
	else if (prcopt.refpos==POSOPT_SINGLE) RefPosTypeP->ItemIndex=5;
	
	RovPosTypeF					=RovPosTypeP->ItemIndex;
	RefPosTypeF					=RefPosTypeP->ItemIndex;
	SetPos(RovPosTypeP->ItemIndex,editu,prcopt.ru);
	SetPos(RefPosTypeP->ItemIndex,editr,prcopt.rb);
	
	SatPcvFile ->Text			=filopt.satantp;
	AntPcvFile ->Text			=filopt.rcvantp;
	StaPosFile ->Text			=filopt.stapos;
	GeoidDataFile->Text			=filopt.geoid;
	DCBFile    ->Text			=filopt.dcb;
	LocalDir   ->Text			=filopt.tempdir;

	ReadAntList();
	UpdateEnable();
}
//---j------------------------------------------------------------------------
void __fastcall TOptDialog::SaveOpt(AnsiString file)
{
	AnsiString ProxyAddrE_Text=ProxyAddrE->Text;
	AnsiString ExSatsE_Text=ExSatsE->Text;
	AnsiString FieldSep_Text=FieldSep->Text;
	AnsiString RovAnt_Text=RovAnt->Text,RefAnt_Text=RefAnt->Text;
	AnsiString SatPcvFile_Text=SatPcvFile->Text;
	AnsiString AntPcvFile_Text=AntPcvFile->Text;
	AnsiString StaPosFile_Text=StaPosFile->Text;
	AnsiString GeoidDataFile_Text=GeoidDataFile->Text;
	AnsiString DCBFile_Text=DCBFile->Text;
	AnsiString LocalDir_Text=LocalDir->Text;
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_NTRIPC_C,STR_FILE};
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	char buff[1024],*p,*q,id[32],comment[256],s[64];
	int sat,ex;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
	filopt_t filopt={""};
	
	for (int i=0;i<8;i++) {
		strtype[i]=i<3?itype[MainForm->Stream[i]]:otype[MainForm->Stream[i]];
		strfmt[i]=MainForm->Format[i];
		
		if (!MainForm->StreamC[i]) {
			strtype[i]=STR_NONE;
			strcpy(strpath[i],"");
		}
		else if (strtype[i]==STR_SERIAL) {
			strcpy(strpath[i],MainForm->Paths[i][0].c_str());
		}
		else if (strtype[i]==STR_FILE) {
			strcpy(strpath[i],MainForm->Paths[i][2].c_str());
		}
		else if (strtype[i]==STR_TCPSVR) {
			strcpy(buff,MainForm->Paths[i][1].c_str());
			if ((p=strchr(buff,'/'))) *p='\0';
			if ((p=strrchr(buff,':'))) {
				strcpy(strpath[i],p);
			}
			else {
				strcpy(strpath[i],"");
			}
		}
		else if (strtype[i]==STR_TCPCLI) {
			strcpy(buff,MainForm->Paths[i][1].c_str());
			if ((p=strchr(buff,'/'))) *p='\0';
			if ((p=strrchr(buff,'@'))) {
				strcpy(strpath[i],p+1);
			}
			else {
				strcpy(strpath[i],buff);
			}
		}
		else if (strtype[i]==STR_NTRIPSVR) {
			strcpy(buff,MainForm->Paths[i][1].c_str());
			if ((p=strchr(buff,':'))&&strchr(p+1,'@')) {
				strcpy(strpath[i],p);
			}
			else {
				strcpy(strpath[i],buff);
			}
		}
		else if (strtype[i]==STR_NTRIPCLI) {
			strcpy(buff,MainForm->Paths[i][1].c_str());
			if ((p=strchr(buff,'/'))&&(q=strchr(p+1,':'))) *q='\0';
			strcpy(strpath[i],buff);
		}
		else if (strtype[i]==STR_NTRIPC_S||strtype[i]==STR_NTRIPC_C) {
			strcpy(buff,MainForm->Paths[i][1].c_str());
			if ((p=strchr(buff,'/'))&&(q=strchr(p+1,':'))) *q='\0';
			if ((p=strchr(buff,'@'))) {
				*(p+1)='\0';
				strcpy(strpath[i],buff);
			}
			if ((p=strchr(p?p+2:buff,':'))) {
				strcat(strpath[i],p);
			}
		}
		else if (strtype[i]==STR_FTP||strtype[i]==STR_HTTP) {
			strcpy(strpath[i],MainForm->Paths[i][3].c_str());
		}
	}
	nmeareq   =MainForm->NmeaReq;
	nmeapos[0]=MainForm->NmeaPos[0];
	nmeapos[1]=MainForm->NmeaPos[1];

	svrcycle    =SvrCycleE   ->Text.ToInt();
	timeout     =TimeoutTimeE->Text.ToInt();
	reconnect   =ReconTimeE  ->Text.ToInt();
	nmeacycle   =NmeaCycleE  ->Text.ToInt();
	buffsize    =SvrBuffSizeE->Text.ToInt();
	navmsgsel   =NavSelectS  ->ItemIndex;
	strcpy(proxyaddr,ProxyAddrE_Text.c_str());
	fswapmargin =FileSwapMarginE->Text.ToInt();
	prcopt.sbassatsel=SbasSatE->Text.ToInt();

	prcopt.mode		=PosMode	 ->ItemIndex;
	prcopt.nf		=Freq		 ->ItemIndex+1;
	prcopt.soltype	=Solution	 ->ItemIndex;
	prcopt.elmin	=str2dbl(ElMask	->Text)*D2R;
	prcopt.dynamics	=DynamicModel->ItemIndex;
	prcopt.tidecorr	=TideCorr	 ->ItemIndex;
	prcopt.ionoopt	=IonoOpt	 ->ItemIndex;
	prcopt.tropopt	=TropOpt	 ->ItemIndex;
	prcopt.sateph	=SatEphem	 ->ItemIndex;
	if (ExSatsE->Text!="") {
		strcpy(buff,ExSatsE_Text.c_str());
		for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
			if (*p=='+') {ex=2; p++;} else ex=1;
			if (!(sat=satid2no(p))) continue;
			prcopt.exsats[sat-1]=ex;
		}
	}
	prcopt.navsys	= (NavSys1->Checked?SYS_GPS:0)|
					  (NavSys2->Checked?SYS_GLO:0)|
					  (NavSys3->Checked?SYS_GAL:0)|
					  (NavSys4->Checked?SYS_QZS:0)|
					  (NavSys5->Checked?SYS_SBS:0)|
					  (NavSys6->Checked?SYS_CMP:0)|
					  (NavSys7->Checked?SYS_IRN:0);
	prcopt.posopt[0]=PosOpt1->Checked;
	prcopt.posopt[1]=PosOpt2->Checked;
	prcopt.posopt[2]=PosOpt3->Checked;
	prcopt.posopt[3]=PosOpt4->Checked;
	prcopt.posopt[4]=PosOpt5->Checked;
	prcopt.posopt[5]=PosOpt6->Checked;
	
	prcopt.modear	=AmbRes		->ItemIndex;
	prcopt.glomodear=GloAmbRes	->ItemIndex;
	prcopt.bdsmodear=BdsAmbRes	->ItemIndex;
	prcopt.thresar[0]=str2dbl(ValidThresAR->Text);
	prcopt.maxout	=str2dbl(OutCntResetAmb->Text);
	prcopt.minfix	=str2dbl(FixCntHoldAmb->Text);
	prcopt.minlock	=str2dbl(LockCntFixAmb->Text);
	prcopt.elmaskar	=str2dbl(ElMaskAR	->Text)*D2R;
	prcopt.elmaskhold=str2dbl(ElMaskHold->Text)*D2R;
	prcopt.maxtdiff	=str2dbl(MaxAgeDiff	->Text);
	prcopt.maxgdop	=str2dbl(RejectGdop ->Text);
	prcopt.maxinno	=str2dbl(RejectThres->Text);
	prcopt.thresslip=str2dbl(SlipThres	->Text);
	prcopt.armaxiter=str2dbl(ARIter		->Text);
	prcopt.niter	=str2dbl(NumIter	->Text);
	prcopt.syncsol	=SyncSol->ItemIndex;
	if (prcopt.mode==PMODE_MOVEB&&BaselineConst->Checked) {
		prcopt.baseline[0]=str2dbl(BaselineLen->Text);
		prcopt.baseline[1]=str2dbl(BaselineSig->Text);
	}
	solopt.posf		=SolFormat	->ItemIndex;
	solopt.timef	=TimeFormat	->ItemIndex==0?0:1;
	solopt.times	=TimeFormat	->ItemIndex==0?0:TimeFormat->ItemIndex-1;
	solopt.timeu	=str2dbl(TimeDecimal ->Text);
	solopt.degf		=LatLonFormat->ItemIndex;
	strcpy(solopt.sep,FieldSep_Text.c_str());
	solopt.outhead	=OutputHead	 ->ItemIndex;
	solopt.outopt	=OutputOpt	 ->ItemIndex;
	prcopt.outsingle=OutputSingle->ItemIndex;
	solopt.maxsolstd=str2dbl(MaxSolStd->Text);
	solopt.datum	=OutputDatum ->ItemIndex;
	solopt.height	=OutputHeight->ItemIndex;
	solopt.geoid	=OutputGeoid ->ItemIndex;
	solopt.nmeaintv[0]=str2dbl(NmeaIntv1->Text);
	solopt.nmeaintv[1]=str2dbl(NmeaIntv2->Text);
	solopt.trace	=DebugTrace	 ->ItemIndex;
	solopt.sstat	=DebugStatus ->ItemIndex;
	
	prcopt.eratio[0]=str2dbl(MeasErrR1->Text);
	prcopt.eratio[1]=str2dbl(MeasErrR2->Text);
	prcopt.err[1]	=str2dbl(MeasErr2->Text);
	prcopt.err[2]	=str2dbl(MeasErr3->Text);
	prcopt.err[3]	=str2dbl(MeasErr4->Text);
	prcopt.err[4]	=str2dbl(MeasErr5->Text);
	prcopt.sclkstab	=str2dbl(SatClkStab->Text);
	prcopt.prn[0]	=str2dbl(PrNoise1->Text);
	prcopt.prn[1]	=str2dbl(PrNoise2->Text);
	prcopt.prn[2]	=str2dbl(PrNoise3->Text);
	prcopt.prn[3]	=str2dbl(PrNoise4->Text);
	prcopt.prn[4]	=str2dbl(PrNoise5->Text);
	
	if (RovAntPcv->Checked) strcpy(prcopt.anttype[0],RovAnt_Text.c_str());
	if (RefAntPcv->Checked) strcpy(prcopt.anttype[1],RefAnt_Text.c_str());
	prcopt.antdel[0][0]=str2dbl(RovAntE->Text);
	prcopt.antdel[0][1]=str2dbl(RovAntN->Text);
	prcopt.antdel[0][2]=str2dbl(RovAntU->Text);
	prcopt.antdel[1][0]=str2dbl(RefAntE->Text);
	prcopt.antdel[1][1]=str2dbl(RefAntN->Text);
	prcopt.antdel[1][2]=str2dbl(RefAntU->Text);
	prcopt.maxaveep=MaxAveEp->Text.ToInt();
	prcopt.initrst=ChkInitRestart->Checked;
	
	prcopt.rovpos=POSOPT_POS;
	prcopt.refpos=POSOPT_POS;
	if      (RefPosTypeP->ItemIndex==3) prcopt.refpos=POSOPT_RTCM;
	else if (RefPosTypeP->ItemIndex==4) prcopt.refpos=POSOPT_RAW;
	else if (RefPosTypeP->ItemIndex==5) prcopt.refpos=POSOPT_SINGLE;
	
	if (prcopt.rovpos==POSOPT_POS) GetPos(RovPosTypeP->ItemIndex,editu,prcopt.ru);
	if (prcopt.refpos==POSOPT_POS) GetPos(RefPosTypeP->ItemIndex,editr,prcopt.rb);
	
	strcpy(filopt.satantp,SatPcvFile_Text.c_str());
	strcpy(filopt.rcvantp,AntPcvFile_Text.c_str());
	strcpy(filopt.stapos, StaPosFile_Text.c_str());
	strcpy(filopt.geoid,  GeoidDataFile_Text.c_str());
	strcpy(filopt.dcb,    DCBFile_Text.c_str());
	strcpy(filopt.tempdir,LocalDir_Text.c_str());
	
	time2str(utc2gpst(timeget()),s,0);
	sprintf(comment,"RTKNAVI options (%s, v.%s %s)",s,VER_RTKLIB,PATCH_LEVEL);
	setsysopts(&prcopt,&solopt,&filopt);
	if (!saveopts(file.c_str(),"w",comment,sysopts)||
		!saveopts(file.c_str(),"a","",rcvopts)) return;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::UpdateEnable(void)
{
	int rel=PMODE_DGPS<=PosMode->ItemIndex&&PosMode->ItemIndex<=PMODE_FIXED;
	int rtk=PMODE_KINEMA<=PosMode->ItemIndex&&PosMode->ItemIndex<=PMODE_FIXED;
	int ppp=PosMode->ItemIndex>=PMODE_PPP_KINEMA;
	int ar=rtk||ppp;
	
	Freq           ->Enabled=rel;
	Solution       ->Enabled=false;
	DynamicModel   ->Enabled=rel;
	TideCorr       ->Enabled=rel||ppp;
	PosOpt1        ->Enabled=ppp;
	PosOpt2        ->Enabled=ppp;
	PosOpt3        ->Enabled=ppp;
	PosOpt4        ->Enabled=ppp;
	PosOpt6        ->Enabled=ppp;
	
	AmbRes         ->Enabled=ar;
	GloAmbRes      ->Enabled=ar&&AmbRes->ItemIndex>0&&NavSys2->Checked;
	BdsAmbRes      ->Enabled=ar&&AmbRes->ItemIndex>0&&NavSys6->Checked;
	ValidThresAR   ->Enabled=ar&&AmbRes->ItemIndex>=1&&AmbRes->ItemIndex<4;
	ThresAR2       ->Enabled=ar&&AmbRes->ItemIndex>=4;
	ThresAR3       ->Enabled=ar&&AmbRes->ItemIndex>=4;
	LockCntFixAmb  ->Enabled=ar&&AmbRes->ItemIndex>=1;
	ElMaskAR       ->Enabled=ar&&AmbRes->ItemIndex>=1;
	OutCntResetAmb ->Enabled=ar||ppp;
	FixCntHoldAmb  ->Enabled=ar&&AmbRes->ItemIndex==3;
	ElMaskHold     ->Enabled=ar&&AmbRes->ItemIndex==3;
	SlipThres      ->Enabled=ar||ppp;
	MaxAgeDiff     ->Enabled=rel;
	RejectThres    ->Enabled=rel||ppp;
	ARIter         ->Enabled=ppp;
	NumIter        ->Enabled=rel||ppp;
	SyncSol        ->Enabled=rel||ppp;
	BaselineConst  ->Enabled=PosMode->ItemIndex==PMODE_MOVEB;
	BaselineLen    ->Enabled=BaselineConst->Checked&&PosMode->ItemIndex==PMODE_MOVEB;
	BaselineSig    ->Enabled=BaselineConst->Checked&&PosMode->ItemIndex==PMODE_MOVEB;
	
	OutputHead     ->Enabled=SolFormat->ItemIndex!=3;
	OutputOpt      ->Enabled=false;
	TimeFormat     ->Enabled=SolFormat->ItemIndex!=3;
	TimeDecimal    ->Enabled=SolFormat->ItemIndex!=3;
	LatLonFormat   ->Enabled=SolFormat->ItemIndex==0;
	FieldSep       ->Enabled=SolFormat->ItemIndex!=3;
	OutputSingle   ->Enabled=PosMode->ItemIndex!=0;
	OutputDatum    ->Enabled=SolFormat->ItemIndex==0;
	OutputHeight   ->Enabled=SolFormat->ItemIndex==0;
	OutputGeoid    ->Enabled=SolFormat->ItemIndex==0&&OutputHeight->ItemIndex==1;
	
	RovAntPcv      ->Enabled=rel||ppp;
	RovAnt         ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntE        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntN        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntU        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	LabelRovAntD   ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RefAntPcv      ->Enabled=rel;
	RefAnt         ->Enabled=rel&&RefAntPcv->Checked;
	RefAntE        ->Enabled=rel&&RefAntPcv->Checked;
	RefAntN        ->Enabled=rel&&RefAntPcv->Checked;
	RefAntU        ->Enabled=rel&&RefAntPcv->Checked;
	LabelRefAntD   ->Enabled=rel&&RefAntPcv->Checked;
	
	RovPosTypeP    ->Enabled=PosMode->ItemIndex==PMODE_FIXED||PosMode->ItemIndex==PMODE_PPP_FIXED;
	RovPos1        ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	RovPos2        ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	RovPos3        ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	BtnRovPos      ->Enabled=RovPosTypeP->Enabled&&RovPosTypeP->ItemIndex<=2;
	
	RefPosTypeP    ->Enabled=rel&&PosMode->ItemIndex!=PMODE_MOVEB;
	RefPos1        ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	RefPos2        ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	RefPos3        ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	BtnRefPos      ->Enabled=RefPosTypeP->Enabled&&RefPosTypeP->ItemIndex<=2;
	
	LabelMaxAveEp  ->Visible=RefPosTypeP->ItemIndex==5;
	MaxAveEp       ->Visible=RefPosTypeP->ItemIndex==5;
	ChkInitRestart ->Visible=RefPosTypeP->ItemIndex==5;
	
//	SbasSatE       ->Enabled=PosMode->ItemIndex==0;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::GetPos(int type, TEdit **edit, double *pos)
{
	AnsiString edit0_Text=edit[0]->Text;
	AnsiString edit1_Text=edit[1]->Text;
	double p[3]={0},dms1[3]={0},dms2[3]={0};
	
	if (type==1) { /* lat/lon/height dms/m */
		sscanf(edit0_Text.c_str(),"%lf %lf %lf",dms1,dms1+1,dms1+2);
		sscanf(edit1_Text.c_str(),"%lf %lf %lf",dms2,dms2+1,dms2+2);
		p[0]=(dms1[0]<0?-1:1)*(fabs(dms1[0])+dms1[1]/60+dms1[2]/3600)*D2R;
		p[1]=(dms2[0]<0?-1:1)*(fabs(dms2[0])+dms2[1]/60+dms2[2]/3600)*D2R;
		p[2]=str2dbl(edit[2]->Text);
		pos2ecef(p,pos);
	}
	else if (type==2) { /* x/y/z-ecef */
		pos[0]=str2dbl(edit[0]->Text);
		pos[1]=str2dbl(edit[1]->Text);
		pos[2]=str2dbl(edit[2]->Text);
	}
	else {
		p[0]=str2dbl(edit[0]->Text)*D2R;
		p[1]=str2dbl(edit[1]->Text)*D2R;
		p[2]=str2dbl(edit[2]->Text);
		pos2ecef(p,pos);
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SetPos(int type, TEdit **edit, double *pos)
{
	AnsiString s;
	double p[3],dms1[3],dms2[3],s1,s2;
	
	if (type==1) { /* lat/lon/height dms/m */
		ecef2pos(pos,p); s1=p[0]<0?-1:1; s2=p[1]<0?-1:1;
		p[0]=fabs(p[0])*R2D+1E-12; p[1]=fabs(p[1])*R2D+1E-12;
		dms1[0]=floor(p[0]); p[0]=(p[0]-dms1[0])*60.0;
		dms1[1]=floor(p[0]); dms1[2]=(p[0]-dms1[1])*60.0;
		dms2[0]=floor(p[1]); p[1]=(p[1]-dms2[0])*60.0;
		dms2[1]=floor(p[1]); dms2[2]=(p[1]-dms2[1])*60.0;
		edit[0]->Text=s.sprintf("%.0f %02.0f %09.6f",s1*dms1[0],dms1[1],dms1[2]);
		edit[1]->Text=s.sprintf("%.0f %02.0f %09.6f",s2*dms2[0],dms2[1],dms2[2]);
		edit[2]->Text=s.sprintf("%.4f",p[2]);
	}
	else if (type==2) { /* x/y/z-ecef */
		edit[0]->Text=s.sprintf("%.4f",pos[0]);
		edit[1]->Text=s.sprintf("%.4f",pos[1]);
		edit[2]->Text=s.sprintf("%.4f",pos[2]);
	}
	else {
		ecef2pos(pos,p);
		edit[0]->Text=s.sprintf("%.9f",p[0]*R2D);
		edit[1]->Text=s.sprintf("%.9f",p[1]*R2D);
		edit[2]->Text=s.sprintf("%.4f",p[2]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::ReadAntList(void)
{
	AnsiString AntPcvFile_Text=AntPcvFile->Text;
	TStringList *list;
	pcvs_t pcvs={0};
	char *p;
	
	if (!readpcv(AntPcvFile_Text.c_str(),&pcvs)) return;
	
	list=new TStringList;
	list->Add("");
	list->Add("*");
	
	for (int i=0;i<pcvs.n;i++) {
		if (pcvs.pcv[i].sat) continue;
		if ((p=strchr(pcvs.pcv[i].type,' '))) *p='\0';
		if (i>0&&!strcmp(pcvs.pcv[i].type,pcvs.pcv[i-1].type)) continue;
		list->Add(pcvs.pcv[i].type);
	}
	RovAnt->Items=list;
	RefAnt->Items=list;
	
	free(pcvs.pcv);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::NavSys6Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------

