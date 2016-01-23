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
	fprintf(fp,"%% sky image tag file: rtkplot %s %s\n\n",VER_RTKLIB,PATCH_LEVEL);
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
	fprintf(fp,"binarize= %d\n"  ,Plot->SkyBinarize);
	fprintf(fp,"binthr1 = %.2f\n",Plot->SkyBinThres1);
	fprintf(fp,"binthr2 = %.2f\n",Plot->SkyBinThres2);
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
	SkyBinarize->Checked=Plot->SkyBinarize;
	SkyBinThres1->Text=s.sprintf("%.2f",Plot->SkyBinThres1);
	SkyBinThres2->Text=s.sprintf("%.2f",Plot->SkyBinThres2);
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
	Plot->SkyBinarize=SkyBinarize->Checked;
	Plot->SkyBinThres1=str2dbl(SkyBinThres1->Text);
	Plot->SkyBinThres2=str2dbl(SkyBinThres2->Text);
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
	SkyBinThres1->Enabled=SkyBinarize->Checked;
	SkyBinThres2->Enabled=SkyBinarize->Checked;
	SkyBinThres1UpDown->Enabled=SkyBinarize->Checked;
	SkyBinThres2UpDown->Enabled=SkyBinarize->Checked;
	BtnGenMask->Enabled=SkyBinarize->Checked;
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

void __fastcall TSkyImgDialog::BtnGenMaskClick(TObject *Sender)
{
    BITMAP bm;
    BYTE *img,*pix;
    double r,ca,sa,el,el0;
    int x,y,w,h,b,az,n;
    
    if (!GetObject(Plot->SkyImageR->Handle,sizeof(bm),&bm)) return;
    w=bm.bmWidth; h=bm.bmHeight; b=bm.bmWidthBytes;
    if (w<=0||h<=0||b<w*3) return;
    img=(BYTE *)bm.bmBits;
    
    for (az=0;az<=360;az++) {
        ca=cos(az*D2R);
        sa=sin(az*D2R);
        for (el=90.0,n=0,el0=0.0;el>=0.0;el-=0.1) {
            r=(1.0-el/90.0)*Plot->SkyScaleR;
            x=(int)floor(w/2.0+sa*r+0.5);
            y=(int)floor(h/2.0+ca*r+0.5);
            if (x<0||x>=w||y<0||y>=h) continue;
            pix=img+x*3+y*b;
            if (pix[0]<255&&pix[1]<255&&pix[2]<255) {
                if (++n==1) el0=el;
                if (n>=5) break;
            }
            else n=0;
        }
        Plot->ElMaskData[az]=el0==90.0?0.0:el0*D2R;
    }
	Plot->UpdateSky();
}
//---------------------------------------------------------------------------

void __fastcall TSkyImgDialog::SkyBinThres1UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double thres=str2dbl(SkyBinThres1->Text);
	if (Direction==updUp) thres+=0.01; else thres-=0.01;
	if (thres<=0.0) thres=0.0; else if (thres>1.0) thres=1.0;
	SkyBinThres1->Text=s.sprintf("%.2f",thres);
	UpdateSky();
}
//---------------------------------------------------------------------------

void __fastcall TSkyImgDialog::SkyBinThres2UpDownChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction)
{
	AnsiString s;
	double thres=str2dbl(SkyBinThres2->Text);
	if (Direction==updUp) thres+=0.01; else thres-=0.01;
	if (thres<=0.0) thres=0.0; else if (thres>1.0) thres=1.0;
	SkyBinThres2->Text=s.sprintf("%.2f",thres);
	UpdateSky();
}
//---------------------------------------------------------------------------

void __fastcall TSkyImgDialog::SkyBinarizeMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
	UpdateSky();
	UpdateEnable();
}
//---------------------------------------------------------------------------

