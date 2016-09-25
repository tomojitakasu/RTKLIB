//---------------------------------------------------------------------------
#include <vcl.h>
#include <FileCtrl.hpp>
#pragma hdrstop

#include "rtklib.h"
#include "refdlg.h"
#include "svroptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSvrOptDialog *SvrOptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TSvrOptDialog::TSvrOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::FormShow(TObject *Sender)
{
	double pos[3];
	AnsiString s;
	DataTimeout->Text=s.sprintf("%d",SvrOpt[0]);
	ConnectInterval->Text=s.sprintf("%d",SvrOpt[1]);
	AvePeriodRate->Text=s.sprintf("%d",SvrOpt[2]);
	SvrBuffSize->Text=s.sprintf("%d",SvrOpt[3]);
	SvrCycle->Text=s.sprintf("%d",SvrOpt[4]);
	ProgBarR->Text=s.sprintf("%d",ProgBarRange);
	RelayMsg->ItemIndex=RelayBack;
	NmeaCycle->Text=s.sprintf("%d",SvrOpt[5]);
	FileSwapMarginE->Text=s.sprintf("%d",FileSwapMargin);
	if (norm(AntPos,3)>0.0) {
		ecef2pos(AntPos,pos);
		AntPos1->Text=s.sprintf("%.8f",pos[0]*R2D);
		AntPos2->Text=s.sprintf("%.8f",pos[1]*R2D);
		AntPos3->Text=s.sprintf("%.3f",pos[2]);
	}
	else {
		AntPos1->Text="0.00000000";
		AntPos2->Text="0.00000000";
		AntPos3->Text="0.000";
	}
	TraceLevelS->ItemIndex=TraceLevel;
	NmeaReqT->Checked=NmeaReq;
	LocalDir->Text=LocalDirectory;
	ProxyAddr->Text=ProxyAddress;
	StationId->Text=s.sprintf("%d",StaId);
	StaInfoSel->Checked=StaSel;
	AntInfo->Text=AntType;
	RcvInfo->Text=RcvType;
	AntOff1->Text=s.sprintf("%.4f",AntOff[0]);
	AntOff2->Text=s.sprintf("%.4f",AntOff[1]);
	AntOff3->Text=s.sprintf("%.4f",AntOff[2]);
	SrcTblFileF->Text=SrcTblFile;
	LogFileF->Text=LogFile;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnOkClick(TObject *Sender)
{
	double pos[3];
	SvrOpt[0]=DataTimeout->Text.ToInt();
	SvrOpt[1]=ConnectInterval->Text.ToInt();
	SvrOpt[2]=AvePeriodRate->Text.ToInt();
	SvrOpt[3]=SvrBuffSize->Text.ToInt();
	SvrOpt[4]=SvrCycle->Text.ToInt();
	SvrOpt[5]=NmeaCycle->Text.ToInt();
	ProgBarRange=ProgBarR->Text.ToInt();
	FileSwapMargin=FileSwapMarginE->Text.ToInt();
	RelayBack=RelayMsg->ItemIndex;
	pos[0]=str2dbl(AntPos1->Text)*D2R;
	pos[1]=str2dbl(AntPos2->Text)*D2R;
	pos[2]=str2dbl(AntPos3->Text);
	if (norm(pos,3)>0.0) {
		pos2ecef(pos,AntPos);
	}
	else {
		for (int i=0;i<3;i++) AntPos[i]=0.0;
	}
	TraceLevel=TraceLevelS->ItemIndex;
	NmeaReq=NmeaReqT->Checked;
	LocalDirectory=LocalDir->Text;
	ProxyAddress=ProxyAddr->Text;
	StaId=(int)str2dbl(StationId->Text);
	StaSel=StaInfoSel->Checked;
	AntType=AntInfo->Text;
	RcvType=RcvInfo->Text;
	AntOff[0]=str2dbl(AntOff1->Text);
	AntOff[1]=str2dbl(AntOff2->Text);
	AntOff[2]=str2dbl(AntOff3->Text);
	SrcTblFile=SrcTblFileF->Text;
	LogFile=LogFileF->Text;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnPosClick(TObject *Sender)
{
	AnsiString s;
	RefDialog->RovPos[0]=str2dbl(AntPos1->Text);
	RefDialog->RovPos[1]=str2dbl(AntPos2->Text);
	RefDialog->RovPos[2]=str2dbl(AntPos3->Text);
	RefDialog->BtnLoad->Enabled=true;
	RefDialog->StaPosFile=StaPosFile;
	if (RefDialog->ShowModal()!=mrOk) return;
	AntPos1->Text=s.sprintf("%.8f",RefDialog->Pos[0]);
	AntPos2->Text=s.sprintf("%.8f",RefDialog->Pos[1]);
	AntPos3->Text=s.sprintf("%.3f",RefDialog->Pos[2]);
	StaPosFile=RefDialog->StaPosFile;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnLocalDirClick(TObject *Sender)
{
#ifdef TCPP
    AnsiString dir=LocalDir->Text;
    if (!SelectDirectory("Local Directory","",dir)) return;
    LocalDir->Text=dir;
#else
    UnicodeString dir=LocalDir->Text;
    TSelectDirExtOpts opt=TSelectDirExtOpts()<<sdNewUI<<sdNewFolder;
    if (!SelectDirectory(L"Local Directory",L"",dir,opt)) return;
    LocalDir->Text=dir;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::UpdateEnable(void)
{
	NmeaCycle->Enabled=NmeaReqT->Checked;
	StationId->Enabled=StaInfoSel->Checked;
	AntPos1->Enabled=StaInfoSel->Checked||NmeaReqT->Checked;
	AntPos2->Enabled=StaInfoSel->Checked||NmeaReqT->Checked;
	AntPos3->Enabled=StaInfoSel->Checked||NmeaReqT->Checked;
	BtnPos ->Enabled=StaInfoSel->Checked||NmeaReqT->Checked;
	AntOff1->Enabled=StaInfoSel->Checked;
	AntOff2->Enabled=StaInfoSel->Checked;
	AntOff3->Enabled=StaInfoSel->Checked;
	AntInfo->Enabled=StaInfoSel->Checked;
	RcvInfo->Enabled=StaInfoSel->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::NmeaReqTClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::StaInfoSelClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnSrcTblFileClick(TObject *Sender)
{
	OpenDialog->Title="NTRIP Source Table File";
	OpenDialog->FileName=SrcTblFileF->Text;
	if (!OpenDialog->Execute()) return;
	SrcTblFileF->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnLogFileClick(TObject *Sender)
{
	OpenDialog->Title="Log File";
	OpenDialog->FileName=LogFileF->Text;
	if (!OpenDialog->Execute()) return;
	LogFileF->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------

