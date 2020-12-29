//---------------------------------------------------------------------------

#include "plotmain.h"
#include "plotopt.h"
#include "refdlg.h"
#include "viewer.h"
#include "rtklib.h"

#include <QShowEvent>
#include <QColorDialog>
#include <QFontDialog>
#include <QFileDialog>
#include <QPoint>
#include <QDebug>
#include <QFileSystemModel>
#include <QCompleter>

QString color2String(const QColor &c);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
PlotOptDialog::PlotOptDialog(QWidget *parent)
   : QDialog(parent)
{
    setupUi(this);

    refDialog= new RefDialog(this);

    QCompleter *fileCompleter=new QCompleter(this);
    QFileSystemModel *fileModel=new QFileSystemModel(fileCompleter);
    fileModel->setRootPath("");
    fileCompleter->setModel(fileModel);
    TLEFile->setCompleter(fileCompleter);
    TLESatFile->setCompleter(fileCompleter);

    connect(BtnCancel,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(BtnColor1,SIGNAL(clicked(bool)),this,SLOT(BtnColor1Click()));
    connect(BtnColor2,SIGNAL(clicked(bool)),this,SLOT(BtnColor2Click()));
    connect(BtnColor3,SIGNAL(clicked(bool)),this,SLOT(BtnColor3Click()));
    connect(BtnColor4,SIGNAL(clicked(bool)),this,SLOT(BtnColor4Click()));
    connect(BtnFont,SIGNAL(clicked(bool)),this,SLOT(BtnFontClick()));
    connect(BtnOK,SIGNAL(clicked(bool)),this,SLOT(BtnOKClick()));
    connect(BtnRefPos,SIGNAL(clicked(bool)),this,SLOT(BtnRefPosClick()));
    connect(BtnTLEFile,SIGNAL(clicked(bool)),this,SLOT(BtnTLEFileClick()));
    connect(BtnTLESatFile,SIGNAL(clicked(bool)),this,SLOT(BtnTLESatFileClick()));
    connect(BtnTLESatView,SIGNAL(clicked(bool)),this,SLOT(BtnTLESatViewClick()));
    connect(BtnTLEView,SIGNAL(clicked(bool)),this,SLOT(BtnTLEViewClick()));
    connect(Origin,SIGNAL(currentIndexChanged(int)),this,SLOT(OriginChange()));
    connect(AutoScale,SIGNAL(currentIndexChanged(int)),this,SLOT(AutoScaleChange()));
    connect(RcvPos,SIGNAL(currentIndexChanged(int)),this,SLOT(RcvPosChange()));
    connect(MColor1,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor2,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor3,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor4,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor5,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor6,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor7,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor8,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor9,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor10,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor11,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
    connect(MColor12,SIGNAL(clicked(bool)),this,SLOT(MColorClick()));
}
//---------------------------------------------------------------------------
void PlotOptDialog::showEvent(QShowEvent *event)
{
    int i,marks[]={1,2,3,4,5,10,15,20};
    
    if (event->spontaneous()) return;

    TimeLabel  ->setCurrentIndex(plot->TimeLabel);
    LatLonFmt  ->setCurrentIndex(plot->LatLonFmt);
    AutoScale  ->setCurrentIndex(plot->AutoScale);
    ShowStats  ->setCurrentIndex(plot->ShowStats);
    ShowArrow  ->setCurrentIndex(plot->ShowArrow);
    ShowSlip   ->setCurrentIndex(plot->ShowSlip);
    ShowHalfC  ->setCurrentIndex(plot->ShowHalfC);
    ShowErr    ->setCurrentIndex(plot->ShowErr);
    ShowEph    ->setCurrentIndex(plot->ShowEph);
    ShowLabel  ->setCurrentIndex(plot->ShowLabel);
    ShowGLabel ->setCurrentIndex(plot->ShowGLabel);
    ShowScale  ->setCurrentIndex(plot->ShowScale);
    ShowCompass->setCurrentIndex(plot->ShowCompass);
    PlotStyle  ->setCurrentIndex(plot->PlotStyle);
    for (i=0;i<8;i++) {
        if (marks[i]==plot->MarkSize) MarkSize->setCurrentIndex(i);
    }
    for (int i=0;i<8;i++)
        for (int j=0;j<2;j++)
            MColor[j][i]=plot->MColor[j][i];
    for (int i=0;i<4;i++)
        CColor[i]=plot->CColor[i];

    MColor1 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[0][1])));
    MColor2 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[0][2])));
    MColor3 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[0][3])));
    MColor4 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[0][4])));
    MColor5 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[0][5])));
    MColor6 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[0][6])));
    MColor7 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[1][1])));
    MColor8 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[1][2])));
    MColor9 ->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[1][3])));
    MColor10->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[1][4])));
    MColor11->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[1][5])));
    MColor12->setStyleSheet(QString("background-color: %1;").arg(color2String(plot->MColor[1][6])));
    Color1 ->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->CColor[0])));
    Color2 ->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->CColor[1])));
    Color3 ->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->CColor[2])));
    Color4 ->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(plot->CColor[3])));
    
    FontOpt=plot->Font;
    UpdateFont();
    
    ElMask ->setCurrentText(QString::number(plot->ElMask));
    MaxDop ->setCurrentText(QString::number(plot->MaxDop));
    MaxMP  ->setCurrentText(QString::number(plot->MaxMP ));
    YRange ->setCurrentText(QString::number(plot->YRange));
    Origin ->setCurrentIndex(plot->Origin);
    RcvPos ->setCurrentIndex(plot->RcvPos);
    RefPos1->setText(QString::number(plot->OOPos[0]*R2D));
    RefPos2->setText(QString::number(plot->OOPos[1]*R2D));
    RefPos3->setText(QString::number(plot->OOPos[2]));
    NavSys1->setChecked(plot->NavSys&SYS_GPS);
    NavSys2->setChecked(plot->NavSys&SYS_GLO);
    NavSys3->setChecked(plot->NavSys&SYS_GAL);
    NavSys4->setChecked(plot->NavSys&SYS_QZS);
    NavSys5->setChecked(plot->NavSys&SYS_SBS);
    NavSys6->setChecked(plot->NavSys&SYS_CMP);
    NavSys7->setChecked(plot->NavSys&SYS_IRN);
    AnimCycle->setCurrentText(QString::number(plot->AnimCycle));
    RefCycle ->setText(QString::number(plot->RefCycle ));
    HideLowSat->setCurrentIndex(plot->HideLowSat);
    ElMaskP->setCurrentIndex(plot->ElMaskP);
    ExSats->setText(plot->ExSats);
    BuffSize->setText(QString::number(plot->RtBuffSize));
    QcCmd  ->setText(plot->QcCmd);
    RnxOpts->setText(plot->RnxOpts);
    TLEFile->setText(plot->TLEFile);
    TLESatFile->setText(plot->TLESatFile);
    
    UpdateEnable();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnOKClick()
{
    int marks[]={1,2,3,4,5,10,15,20};
    
    plot->TimeLabel  =TimeLabel  ->currentIndex();
    plot->LatLonFmt  =LatLonFmt  ->currentIndex();
    plot->AutoScale  =AutoScale  ->currentIndex();
    plot->ShowStats  =ShowStats  ->currentIndex();
    plot->ShowArrow  =ShowArrow  ->currentIndex();
    plot->ShowSlip   =ShowSlip   ->currentIndex();
    plot->ShowHalfC  =ShowHalfC  ->currentIndex();
    plot->ShowErr    =ShowErr    ->currentIndex();
    plot->ShowEph    =ShowEph    ->currentIndex();
    plot->ShowLabel  =ShowLabel  ->currentIndex();
    plot->ShowGLabel =ShowGLabel ->currentIndex();
    plot->ShowScale  =ShowScale  ->currentIndex();
    plot->ShowCompass=ShowCompass->currentIndex();
    plot->PlotStyle  =PlotStyle  ->currentIndex();
    plot->MarkSize=marks[MarkSize->currentIndex()];
    for (int i=0;i<8;i++)
        for (int j=0;j<2;j++)
            plot->MColor[j][i]=MColor[j][i];
    for (int i=0;i<4;i++)
        plot->CColor[i]=CColor[i];

    plot->Font=FontOpt;
    
    plot->ElMask=ElMask->currentText().toDouble();
    plot->MaxDop=MaxDop->currentText().toDouble();
    plot->MaxMP =MaxMP ->currentText().toDouble();
    plot->YRange=YRange->currentText().toDouble();
    plot->Origin=Origin->currentIndex();
    plot->RcvPos=RcvPos->currentIndex();
    plot->OOPos[0]=RefPos1->text().toDouble()*D2R;
    plot->OOPos[1]=RefPos2->text().toDouble()*D2R;
    plot->OOPos[2]=RefPos3->text().toDouble();
    plot->NavSys=(NavSys1->isChecked()?SYS_GPS:0)|
                 (NavSys2->isChecked()?SYS_GLO:0)|
                 (NavSys3->isChecked()?SYS_GAL:0)|
                 (NavSys4->isChecked()?SYS_QZS:0)|
                 (NavSys5->isChecked()?SYS_SBS:0)|
                 (NavSys6->isChecked()?SYS_CMP:0)|
                 (NavSys7->isChecked()?SYS_IRN:0);
    plot->AnimCycle=AnimCycle->currentText().toInt();
    plot->RefCycle =RefCycle ->text().toInt();
    plot->HideLowSat=HideLowSat->currentIndex();
    plot->ElMaskP=ElMaskP->currentIndex();
    plot->RtBuffSize=BuffSize->text().toDouble();
    plot->ExSats=ExSats->text();
    plot->QcCmd =QcCmd->text();
    plot->RnxOpts=RnxOpts->text();
    plot->TLEFile=TLEFile->text();
    plot->TLESatFile=TLESatFile->text();

    accept();
}
//---------------------------------------------------------------------------
void PlotOptDialog::MColorClick()
{
    QToolButton *button=dynamic_cast<QToolButton *>(QObject::sender());
    if (!button) return;

    QColorDialog dialog(this);
    QColor *current=&MColor[0][1];

    if (button==MColor1)  current=&MColor[0][1];
    if (button==MColor2)  current=&MColor[0][2];
    if (button==MColor3)  current=&MColor[0][3];
    if (button==MColor4)  current=&MColor[0][4];
    if (button==MColor5)  current=&MColor[0][5];
    if (button==MColor6)  current=&MColor[0][6];
    if (button==MColor7)  current=&MColor[1][1];
    if (button==MColor8)  current=&MColor[1][2];
    if (button==MColor9)  current=&MColor[1][3];
    if (button==MColor10) current=&MColor[1][4];
    if (button==MColor11) current=&MColor[1][5];
    if (button==MColor12) current=&MColor[1][6];
    dialog.setCurrentColor(*current);

    dialog.exec();
    if (dialog.result()!=QDialog::Accepted) return;
    button->setStyleSheet(QString("background-color: %1").arg(color2String(dialog.currentColor())));
    *current=dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnColor1Click()
{
    QColorDialog dialog(this);
    dialog.setCurrentColor(CColor[0]);
    dialog.exec();
    if (dialog.result()!=QDialog::Accepted) return;
    Color1->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    CColor[0]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnColor2Click()
{
    QColorDialog dialog(this);
    dialog.setCurrentColor(CColor[1]);
    dialog.exec();
    if (dialog.result()!=QDialog::Accepted) return;
    Color2->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    CColor[1]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnColor3Click()
{
    QColorDialog dialog(this);
    dialog.setCurrentColor(CColor[2]);
    dialog.exec();
    if (dialog.result()!=QDialog::Accepted) return;
    Color3->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    CColor[2]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnColor4Click()
{
    QColorDialog dialog(this);
    dialog.setCurrentColor(CColor[3]);
    dialog.exec();
    if (dialog.result()!=QDialog::Accepted) return;
    Color4->setStyleSheet(QString("QLabel {background-color: %1;}").arg(color2String(dialog.currentColor())));
    CColor[3]=dialog.currentColor();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnFontClick()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(FontOpt);
    dialog.exec();

    if (dialog.result()!=QDialog::Accepted) return;
    FontOpt=dialog.currentFont();
    UpdateFont();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnQcCmdClick()
{
    QcCmd->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Open"))));
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnTLEFileClick()
{
    TLEFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Open"),QString(),tr("Text Files (*.txt);;Position Files (*.pos *.snx);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnTLESatFileClick()
{
    TLESatFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this,tr("Open"),QString(),tr("Text Files (*.txt);;Position Files (*.pos *.snx);;All (*.*)"))));
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnRefPosClick()
{
    refDialog->RovPos[0]=RefPos1->text().toDouble();
    refDialog->RovPos[1]=RefPos2->text().toDouble();
    refDialog->RovPos[2]=RefPos3->text().toDouble();
    refDialog->move(pos().x()+size().width()/2-refDialog->size().width()/2,
        pos().y()+size().height()/2-refDialog->size().height()/2);

    refDialog->exec();

    if (refDialog->result()!=QDialog::Accepted) return;
    RefPos1->setText(QString::number(refDialog->Pos[0]));
    RefPos2->setText(QString::number(refDialog->Pos[1]));
    RefPos3->setText(QString::number(refDialog->Pos[2]));
}
//---------------------------------------------------------------------------
void PlotOptDialog::AutoScaleChange()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void PlotOptDialog::OriginChange()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void PlotOptDialog::UpdateFont(void)
{
    FontLabel->setFont(FontOpt);
    FontLabel->setText(FontOpt.family()+" "+QString::number(FontOpt.pointSize())+" pt");
}
//---------------------------------------------------------------------------
void PlotOptDialog::UpdateEnable(void)
{
    RefPos1    ->setEnabled(Origin->currentIndex()==5||RcvPos->currentIndex()==1);
    RefPos2    ->setEnabled(Origin->currentIndex()==5||RcvPos->currentIndex()==1);
    RefPos3    ->setEnabled(Origin->currentIndex()==5||RcvPos->currentIndex()==1);
    LabelRefPos->setEnabled(Origin->currentIndex()==5||Origin->currentIndex()==6||RcvPos->currentIndex()==1);
    BtnRefPos  ->setEnabled(Origin->currentIndex()==5||Origin->currentIndex()==6||RcvPos->currentIndex()==1);
}
//---------------------------------------------------------------------------
void PlotOptDialog::RcvPosChange()
{
    UpdateEnable();
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnTLEViewClick()
{
    TextViewer *viewer;
    QString file=TLEFile->text();
    
    if (file=="") return;
    viewer=new TextViewer(this);
    viewer->setWindowTitle(file);
    viewer->show();
    viewer->Read(file);
}
//---------------------------------------------------------------------------
void PlotOptDialog::BtnTLESatViewClick()
{
    TextViewer *viewer;
    QString file=TLESatFile->text();
    
    if (file=="") return;
    viewer=new TextViewer(this);
    viewer->setWindowTitle(file);
    viewer->show();
    viewer->Read(file);
}
//---------------------------------------------------------------------------

