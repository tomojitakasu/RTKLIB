//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include "rtklib.h"
#pragma hdrstop

#include "plotmain.h"
#include "skydlg.h"
#include "confdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSkyImgDialog *SkyImgDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TSkyImgDialog::TSkyImgDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::FormShow(TObject *Sender)
{
	UpdateField();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::BtnSaveClick(TObject *Sender)
{
	FILE *fp;
	AnsiString file=Plot->SkyImageFile;
	if (file=="") return;
	UpdateSky();
	file=file+".tag";
	if ((fp=fopen(file.c_str(),"r"))) {
		fclose(fp);
		ConfDialog->Label1->Caption="File exists. Overwrite it?";
		ConfDialog->Label2->Caption=file;
		if (ConfDialog->ShowModal()!=mrOk) return;
	}
	if (!(fp=fopen(file.c_str(),"w"))) return;
	fprintf(fp,"%% sky image tag file: rtkplot %s\n\n",VER_RTKLIB);
	fprintf(fp,"centx   = %.6g\n",Plot->SkyCent[0]);
	fprintf(fp,"centy   = %.6g\n",Plot->SkyCent[1]);
	fprintf(fp,"scale   = %.6g\n",Plot->SkyScale  );
	fprintf(fp,"roll    = %.6g\n",Plot->SkyFov[0] );
	fprintf(fp,"pitch   = %.6g\n",Plot->SkyFov[1] );
	fprintf(fp,"yaw     = %.6g\n",Plot->SkyFov[2] );
	fprintf(fp,"destcorr= %d\n"  ,Plot->SkyDestCorr);
	fprintf(fp,"resample= %d\n"  ,Plot->SkyRes     );
	fprintf(fp,"flip    = %d\n"  ,Plot->SkyFlip    );
	fprintf(fp,"dest    = %.6g %.6g %.6g %.6g %.6g %.6g %.6g %.6g %.6g\n",
		Plot->SkyDest[1],Plot->SkyDest[2],Plot->SkyDest[3],Plot->SkyDest[4],
		Plot->SkyDest[5],Plot->SkyDest[6],Plot->SkyDest[7],Plot->SkyDest[8],
		Plot->SkyDest[9]);
	fprintf(fp,"elmask  = %d\n"  ,Plot->SkyElMask );
	fclose(fp);
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::BtnUpdateClick(TObject *Sender)
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::BtnCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::UpdateField(void)
{
	AnsiString s;
	Caption=Plot->SkyImageFile;
	SkySize1->Text=s.sprintf("%d",Plot->SkySize[0]);
	SkySize2->Text=s.sprintf("%d",Plot->SkySize[1]);
	SkyCent1->Text=s.sprintf("%.2f",Plot->SkyCent[0]);
	SkyCent2->Text=s.sprintf("%.2f",Plot->SkyCent[1]);
	SkyScale->Text=s.sprintf("%.2f",Plot->SkyScale);
	SkyFov1 ->Text=s.sprintf("%.2f",Plot->SkyFov[0]);
	SkyFov2 ->Text=s.sprintf("%.2f",Plot->SkyFov[1]);
	SkyFov3 ->Text=s.sprintf("%.2f",Plot->SkyFov[2]);
	SkyDest1->Text=s.sprintf("%.1f",Plot->SkyDest[1]);
	SkyDest2->Text=s.sprintf("%.1f",Plot->SkyDest[2]);
	SkyDest3->Text=s.sprintf("%.1f",Plot->SkyDest[3]);
	SkyDest4->Text=s.sprintf("%.1f",Plot->SkyDest[4]);
	SkyDest5->Text=s.sprintf("%.1f",Plot->SkyDest[5]);
	SkyDest6->Text=s.sprintf("%.1f",Plot->SkyDest[6]);
	SkyDest7->Text=s.sprintf("%.1f",Plot->SkyDest[7]);
	SkyDest8->Text=s.sprintf("%.1f",Plot->SkyDest[8]);
	SkyDest9->Text=s.sprintf("%.1f",Plot->SkyDest[9]);
	SkyElMask->Checked=Plot->SkyElMask;
	SkyDestCorr->Checked=Plot->SkyDestCorr;
	SkyRes->ItemIndex=Plot->SkyRes;
	SkyFlip->Checked=Plot->SkyFlip;
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::UpdateSky(void)
{
	Plot->SkyCent[0]=str2dbl(SkyCent1->Text);
	Plot->SkyCent[1]=str2dbl(SkyCent2->Text);
	Plot->SkyScale=str2dbl(SkyScale->Text);
	Plot->SkyFov[0]=str2dbl(SkyFov1->Text);
	Plot->SkyFov[1]=str2dbl(SkyFov2->Text);
	Plot->SkyFov[2]=str2dbl(SkyFov3->Text);
	Plot->SkyDest[1]=str2dbl(SkyDest1->Text);
	Plot->SkyDest[2]=str2dbl(SkyDest2->Text);
	Plot->SkyDest[3]=str2dbl(SkyDest3->Text);
	Plot->SkyDest[4]=str2dbl(SkyDest4->Text);
	Plot->SkyDest[5]=str2dbl(SkyDest5->Text);
	Plot->SkyDest[6]=str2dbl(SkyDest6->Text);
	Plot->SkyDest[7]=str2dbl(SkyDest7->Text);
	Plot->SkyDest[8]=str2dbl(SkyDest8->Text);
	Plot->SkyDest[9]=str2dbl(SkyDest9->Text);
	Plot->SkyElMask=SkyElMask->Checked;
	Plot->SkyDestCorr=SkyDestCorr->Checked;
	Plot->SkyRes=SkyRes->ItemIndex;
	Plot->SkyFlip=SkyFlip->Checked;
	Plot->UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::UpdateEnable(void)
{
	SkyDest1->Enabled=SkyDestCorr->Checked;
	SkyDest2->Enabled=SkyDestCorr->Checked;
	SkyDest3->Enabled=SkyDestCorr->Checked;
	SkyDest4->Enabled=SkyDestCorr->Checked;
	SkyDest5->Enabled=SkyDestCorr->Checked;
	SkyDest6->Enabled=SkyDestCorr->Checked;
	SkyDest7->Enabled=SkyDestCorr->Checked;
	SkyDest8->Enabled=SkyDestCorr->Checked;
	SkyDest9->Enabled=SkyDestCorr->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyFov2UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double fov=str2dbl(SkyFov2->Text);
    fov=floor(fov+0.5);
	if (Direction==updUp) fov+=1.0; else fov-=1.0;
	if (fov<=-180.0) fov+=360.0; else if (fov>180.0) fov-=360.0;
	SkyFov2->Text=s.sprintf("%.2f",fov);
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyFov1UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double fov=str2dbl(SkyFov1->Text);
    fov=floor(fov+0.5);
	if (Direction==updUp) fov+=1.0; else fov-=1.0;
	if (fov<=-180.0) fov+=360.0; else if (fov>180.0) fov-=360.0;
	SkyFov1->Text=s.sprintf("%.2f",fov);
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyFov3UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double fov=str2dbl(SkyFov3->Text);
    fov=floor(fov+0.5);
	if (Direction==updUp) fov+=1.0; else fov-=1.0;
	if (fov<=-180.0) fov+=360.0; else if (fov>180.0) fov-=360.0;
	SkyFov3->Text=s.sprintf("%.2f",fov);
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyCent1UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double cent=str2dbl(SkyCent1->Text);
    cent=floor(cent+0.5);
	if (Direction==updUp) cent+=1.0; else cent-=1.0;
	SkyCent1->Text=s.sprintf("%.2f",cent);
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyCent2UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double cent=str2dbl(SkyCent2->Text);
    cent=floor(cent+0.5);
	if (Direction==updUp) cent+=1.0; else cent-=1.0;
	SkyCent2->Text=s.sprintf("%.2f",cent);
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyScaleUpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double scale=str2dbl(SkyScale->Text);
    scale=floor(scale+0.5);
	if (Direction==updUp) scale+=1.0; else scale-=1.0;
	if (scale<1.0) scale=1.0;
	SkyScale->Text=s.sprintf("%.2f",scale);
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyElMaskMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyDestCorrMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
	UpdateSky();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyFlipMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::SkyResChange(TObject *Sender)
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void __fastcall TSkyImgDialog::BtnLoadClick(TObject *Sender)
{
    OpenTagDialog->Title="Open Tag";
    if (!OpenTagDialog->Execute()) return;
    Plot->ReadSkyTag(OpenTagDialog->FileName);
    UpdateField();
	Plot->UpdateSky();
}
//---------------------------------------------------------------------------

