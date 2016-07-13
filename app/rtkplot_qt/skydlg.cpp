//---------------------------------------------------------------------------
#include <QShowEvent>
#include <QMessageBox>
#include <QFileDialog>

#include <stdio.h>
#include "rtklib.h"

#include "plotmain.h"
#include "skydlg.h"

extern Plot *plot;

//---------------------------------------------------------------------------
 SkyImgDialog::SkyImgDialog(QWidget *parent)
    : QDialog(parent)
{
     setupUi(this);

     connect(BtnClose,SIGNAL(clicked(bool)),this,SLOT(BtnCloseClick()));
     connect(BtnGenMask,SIGNAL(clicked(bool)),this,SLOT(BtnGenMaskClick()));
     connect(BtnLoad,SIGNAL(clicked(bool)),this,SLOT(BtnLoadClick()));
     connect(BtnSave,SIGNAL(clicked(bool)),this,SLOT(BtnSaveClick()));
     connect(BtnUpdate,SIGNAL(clicked(bool)),this,SLOT(BtnUpdateClick()));
     connect(SkyRes,SIGNAL(currentIndexChanged(int)),this,SLOT(SkyResChange()));
     connect(SkyElMask,SIGNAL(clicked(bool)),this,SLOT(SkyElMaskClicked()));
     connect(SkyDestCorr,SIGNAL(clicked(bool)),this,SLOT(SkyDestCorrClicked()));
     connect(SkyFlip,SIGNAL(clicked(bool)),this,SLOT(SkyFlipClicked()));
     connect(SkyBinarize,SIGNAL(clicked(bool)),this,SLOT(SkyBinarizeClicked()));
}
//---------------------------------------------------------------------------
void  SkyImgDialog::showEvent(QShowEvent* event)
{
    if (event->spontaneous()) return;

	UpdateField();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::BtnSaveClick()
{
    QFile fp;
    QString file=plot->SkyImageFile;
	if (file=="") return;
	UpdateSky();
	file=file+".tag";
    fp.setFileName(file);
    if (QFile::exists(file)) {
        if (QMessageBox::question(this,file,tr("File exists. Overwrite it?"))!=QMessageBox::Yes) return;
	}
    if (!fp.open(QIODevice::WriteOnly)) return;

    QString data;
    data=QString("%% sky image tag file: rtkplot %1 %2\n\n").arg(VER_RTKLIB).arg(PATCH_LEVEL);
    data+=QString("centx   = %1\n").arg(plot->SkyCent[0],0,'g',6);
    data+=QString("centy   = %1\n").arg(plot->SkyCent[1],0,'g',6);
    data+=QString("scale   = %1\n").arg(plot->SkyScale,0,'g',6);
    data+=QString("roll    = %1\n").arg(plot->SkyFov[0],0,'g',6);
    data+=QString("pitch   = %1\n").arg(plot->SkyFov[1],0,'g',6);
    data+=QString("yaw     = %1\n").arg(plot->SkyFov[2],0,'g',6);
    data+=QString("destcorr= %1\n").arg(plot->SkyDestCorr);
    data+=QString("resample= %1\n").arg(plot->SkyRes);
    data+=QString("flip    = %1\n").arg(plot->SkyFlip);
    data+=QString("dest    = %1 %2 %3 %4 %5 %6 %7 %8 %9s\n")
        .arg(plot->SkyDest[1],0,'g',6).arg(plot->SkyDest[2],0,'g',6).arg(plot->SkyDest[3],0,'g',6).arg(plot->SkyDest[4],0,'g',6)
        .arg(plot->SkyDest[5],0,'g',6).arg(plot->SkyDest[6],0,'g',6).arg(plot->SkyDest[7],0,'g',6).arg(plot->SkyDest[8],0,'g',6)
        .arg(plot->SkyDest[9],0,'g',6);
    data+=QString("elmask  = %1\n").arg(plot->SkyElMask);
    data+=QString("binarize= %1\n").arg(plot->SkyBinarize);
    data+=QString("binthr1 = %1\n").arg(plot->SkyBinThres1,0,'f',2);
    data+=QString("binthr2 = %1f\n").arg(plot->SkyBinThres2,0,'f',2);
    fp.write(data.toLatin1());
}
//---------------------------------------------------------------------------
void  SkyImgDialog::BtnUpdateClick()
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::BtnCloseClick()
{
    accept();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::UpdateField(void)
{
    setWindowTitle(plot->SkyImageFile);

    SkySize1->setValue(plot->SkySize[0]);
    SkySize2->setValue(plot->SkySize[1]);
    SkyCent1->setValue(plot->SkyCent[0]);
    SkyCent2->setValue(plot->SkyCent[1]);
    SkyScale->setValue(plot->SkyScale);
    SkyFov1 ->setValue(plot->SkyFov[0]);
    SkyFov2 ->setValue(plot->SkyFov[1]);
    SkyFov3 ->setValue(plot->SkyFov[2]);
    SkyDest1->setText(QString::number(plot->SkyDest[1],'f',1));
    SkyDest2->setText(QString::number(plot->SkyDest[2],'f',1));
    SkyDest3->setText(QString::number(plot->SkyDest[3],'f',1));
    SkyDest4->setText(QString::number(plot->SkyDest[4],'f',1));
    SkyDest5->setText(QString::number(plot->SkyDest[5],'f',1));
    SkyDest6->setText(QString::number(plot->SkyDest[6],'f',1));
    SkyDest7->setText(QString::number(plot->SkyDest[7],'f',1));
    SkyDest8->setText(QString::number(plot->SkyDest[8],'f',1));
    SkyDest9->setText(QString::number(plot->SkyDest[9],'f',1));
    SkyElMask->setChecked(plot->SkyElMask);
    SkyDestCorr->setChecked(plot->SkyDestCorr);
    SkyRes->setCurrentIndex(plot->SkyRes);
    SkyFlip->setChecked(plot->SkyFlip);
    SkyBinarize->setChecked(plot->SkyBinarize);
    SkyBinThres1->setValue(plot->SkyBinThres1);
    SkyBinThres2->setValue(plot->SkyBinThres2);
}
//---------------------------------------------------------------------------
void  SkyImgDialog::UpdateSky(void)
{
    plot->SkyCent[0]=SkyCent1->text().toDouble();
    plot->SkyCent[1]=SkyCent2->text().toDouble();
    plot->SkyScale=SkyScale->text().toDouble();
    plot->SkyFov[0]=SkyFov1->text().toDouble();
    plot->SkyFov[1]=SkyFov2->text().toDouble();
    plot->SkyFov[2]=SkyFov3->text().toDouble();
    plot->SkyDest[1]=SkyDest1->text().toDouble();
    plot->SkyDest[2]=SkyDest2->text().toDouble();
    plot->SkyDest[3]=SkyDest3->text().toDouble();
    plot->SkyDest[4]=SkyDest4->text().toDouble();
    plot->SkyDest[5]=SkyDest5->text().toDouble();
    plot->SkyDest[6]=SkyDest6->text().toDouble();
    plot->SkyDest[7]=SkyDest7->text().toDouble();
    plot->SkyDest[8]=SkyDest8->text().toDouble();
    plot->SkyDest[9]=SkyDest9->text().toDouble();
    plot->SkyElMask=SkyElMask->isChecked();
    plot->SkyDestCorr=SkyDestCorr->isChecked();
    plot->SkyRes=SkyRes->currentIndex();
    plot->SkyFlip=SkyFlip->isChecked();
    plot->SkyBinarize=SkyBinarize->isChecked();
    plot->SkyBinThres1=SkyBinThres1->text().toDouble();
    plot->SkyBinThres2=SkyBinThres2->text().toDouble();

    plot->UpdateSky();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::UpdateEnable(void)
{
    SkyDest1->setEnabled(SkyDestCorr->isChecked());
    SkyDest2->setEnabled(SkyDestCorr->isChecked());
    SkyDest3->setEnabled(SkyDestCorr->isChecked());
    SkyDest4->setEnabled(SkyDestCorr->isChecked());
    SkyDest5->setEnabled(SkyDestCorr->isChecked());
    SkyDest6->setEnabled(SkyDestCorr->isChecked());
    SkyDest7->setEnabled(SkyDestCorr->isChecked());
    SkyDest8->setEnabled(SkyDestCorr->isChecked());
    SkyDest9->setEnabled(SkyDestCorr->isChecked());
    SkyBinThres1->setEnabled(SkyBinarize->isChecked());
    SkyBinThres2->setEnabled(SkyBinarize->isChecked());
    BtnGenMask->setEnabled(SkyBinarize->isChecked());
}
//---------------------------------------------------------------------------
void  SkyImgDialog::SkyElMaskClicked()
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::SkyDestCorrClicked()
{
	UpdateSky();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::SkyFlipClicked()
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::SkyResChange()
{
	UpdateSky();
}
//---------------------------------------------------------------------------
void  SkyImgDialog::BtnLoadClick()
{

    plot->ReadSkyTag(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Open Tag"),QString(),"Tag File (*.tag);;All (*.*)")));
    UpdateField();
    plot->UpdateSky();
}
//---------------------------------------------------------------------------

void  SkyImgDialog::BtnGenMaskClick()
{
    QImage &bm=plot->SkyImageR;
    double r,ca,sa,el,el0;
    int x,y,w,h,az,n;
    
    w=bm.width(); h=bm.height();
    if (w<=0||h<=0) return;
    
    for (az=0;az<=360;az++) {
        ca=cos(az*D2R);
        sa=sin(az*D2R);
        for (el=90.0,n=0,el0=0.0;el>=0.0;el-=0.1) {
            r=(1.0-el/90.0)*plot->SkyScaleR;
            x=(int)floor(w/2.0+sa*r+0.5);
            y=(int)floor(h/2.0+ca*r+0.5);
            if (x<0||x>=w||y<0||y>=h) continue;
            QRgb pix=bm.pixel(x,y);
            if (qRed(pix)<255&&qGreen(pix)<255&&qBlue(pix)<255) {
                if (++n==1) el0=el;
                if (n>=5) break;
            }
            else n=0;
        }
        plot->ElMaskData[az]=el0==90.0?0.0:el0*D2R;
    }
    plot->UpdateSky();
}
//---------------------------------------------------------------------------

void  SkyImgDialog::SkyBinarizeClicked()
{
	UpdateSky();
	UpdateEnable();
}
//---------------------------------------------------------------------------
